
#include <stm32f7xx_hal.h>
#include <stm32f7xx_hal.h>
#include <stm32f7xx_hal_ltdc.h>
#include <stm32746g_discovery.h>
#include <stm32746g_discovery_lcd.h>
#include <stm32746g_discovery_ts.h>

typedef struct {
    double r;       // percent
    double g;       // percent
    double b;       // percent
} ColorRGB;

typedef struct {
    double h;       // angle in degrees
    double s;       // percent
    double v;       // percent
} ColorHSV;

extern RNG_HandleTypeDef hrng;
extern LTDC_HandleTypeDef hltdc;
TS_StateTypeDef tsState;

ColorRGB hsv2rgb(ColorHSV in)
{
    double hh, p, q, t, ff;
    long   i;
    ColorRGB out;

    hh = in.h;
    if(hh >= 360.0) hh = 0.0;
    hh /= 60.0;
    i = (long)hh;
    ff = hh - i;
    p = in.v * (1.0 - in.s);
    q = in.v * (1.0 - (in.s * ff));
    t = in.v * (1.0 - (in.s * (1.0 - ff)));

    switch(i) {
        case 0:
            out.r = in.v;
            out.g = t;
            out.b = p;
            break;
        case 1:
            out.r = q;
            out.g = in.v;
            out.b = p;
            break;
        case 2:
            out.r = p;
            out.g = in.v;
            out.b = t;
            break;

        case 3:
            out.r = p;
            out.g = q;
            out.b = in.v;
            break;
        case 4:
            out.r = t;
            out.g = p;
            out.b = in.v;
            break;
        case 5:
        default:
            out.r = in.v;
            out.g = p;
            out.b = q;
            break;
    }
    return out;
}

int main(void)
{

    BSP_LED_Init(LED1);
    BSP_PB_Init(BUTTON_KEY, BUTTON_MODE_GPIO);

    BSP_LCD_Init(0xAA, 0x25, 0);
    BSP_LCD_LayerDefaultInit(LTDC_ACTIVE_LAYER, SDRAM_DEVICE_ADDR);
    BSP_LCD_SelectLayer(LTDC_ACTIVE_LAYER);
    BSP_LCD_SetFont(&LCD_DEFAULT_FONT);
    BSP_LCD_SetBackColor(LCD_COLOR_TRANSPARENT);
    BSP_LCD_Clear(LCD_COLOR_TRANSPARENT);
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);

    const uint16_t sizeX = (const uint16_t) BSP_LCD_GetXSize();
    const uint16_t sizeY = (const uint16_t) BSP_LCD_GetYSize();

    const uint16_t centerX = (const uint16_t) (sizeX / 2);
    const uint16_t centerY = (const uint16_t) (sizeY / 2);

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
    if (BSP_TS_Init(sizeX, sizeY) != TS_OK) while (1);
#pragma clang diagnostic pop

    uint32_t i = 0;
    ColorHSV hsvFg = {0, 1, 0.5};
    ColorHSV hsvBg = {0, 1, 0.5};

    ColorRGB rgb;
    uint32_t rnd;
    uint32_t prevRnd;
    uint8_t r, g, b;

    HAL_RNG_GenerateRandomNumber(&hrng, &prevRnd);

    int16_t x1, y1;
    int16_t x2, y2;
    uint8_t rad;

    x1 = 0;
    y1 = 0;

    uint32_t delay = 10;
    char buff[150];

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
    while (1)
    {
        if (BSP_PB_GetState(BUTTON_KEY))
        {
            while (BSP_PB_GetState(BUTTON_KEY))
            {
                delay = (delay + 1) % 101;
                sprintf(buff, "%4d", delay);
                BSP_LCD_DisplayStringAtLine(0, buff);
                HAL_Delay(15);
                if (delay == 0) while (BSP_PB_GetState(BUTTON_KEY)) HAL_Delay(10);
            }
            BSP_LCD_Clear(LCD_COLOR_TRANSPARENT);
        }

        hsvFg.h += 3;
        if (hsvFg.h >= 360) hsvFg.h -= 360;
        rgb = hsv2rgb(hsvFg);
        r = (uint8_t) (rgb.r * 0xFF);
        g = (uint8_t) (rgb.g * 0xFF);
        b = (uint8_t) (rgb.b * 0xFF);
        BSP_LCD_SetTextColor((uint32_t) ((0xFF << 24) | (r << 16) | (g << 8) | (b)));

        BSP_TS_GetState(&tsState);

        if (tsState.touchDetected)
        {
//            BSP_TS_Get_GestureId(&tsState);
            switch (tsState.gestureId)
            {
                case GEST_ID_MOVE_UP: BSP_LCD_DisplayStringAtLine(1, "UP"); break;
                case GEST_ID_MOVE_RIGHT: BSP_LCD_DisplayStringAtLine(1, "RIGHT"); break;
                case GEST_ID_MOVE_DOWN: BSP_LCD_DisplayStringAtLine(1, "DOWN"); break;
                case GEST_ID_MOVE_LEFT: BSP_LCD_DisplayStringAtLine(1, "LEFT"); break;
                case GEST_ID_ZOOM_IN: BSP_LCD_DisplayStringAtLine(1, "Z in"); break;
                case GEST_ID_ZOOM_OUT: BSP_LCD_DisplayStringAtLine(1, "Z out"); break;
//                default: BSP_LCD_DisplayStringAtLine(1, "--"); break;
            }
        }

        HAL_RNG_GenerateRandomNumber(&hrng, &rnd);
        for (i = 0; i < tsState.touchDetected; i++)
        {
//            rad = (uint8_t) (((rnd >> i) & 0x0F) + 1);
            x1 = tsState.touchX[i];
            y1 = tsState.touchY[i];

            delay = tsState.touchWeight[i];


            if (x1 <= delay) x1 = (int16_t) (delay + 2);
            else if (x1 >= (sizeX - delay)) x1 = (int16_t) (sizeX - delay - 2);

            if (y1 <= delay) y1 = (int16_t) (delay + 2);
            else if (y1 >= (sizeY - delay)) y1 = (int16_t) (sizeY - delay - 2);

            BSP_LCD_FillCircle((uint16_t) x1, (uint16_t) y1, (uint16_t) (delay + 1));
        }


//
//        HAL_RNG_GenerateRandomNumber(&hrng, &rnd);
//
//        rad = (uint8_t) (4 + (rnd % 8));
//        x1 = (int16_t) ((rnd >> 8) % sizeX);
//        y1 = (int16_t) ((rnd >> 24) % sizeY);
//
//        if (x1 < rad) x1 = rad;
//        else if (x1 + rad > sizeX) x1 = sizeX - rad;
//
//        if (y1 < rad) y1 = rad;
//        else if (y1 + rad > sizeY) y1 = sizeY - rad;
//
//        BSP_LCD_FillCircle((uint16_t) x1, (uint16_t) y1, rad);

//        x2 = (int16_t) (x1 + ((int8_t) (rnd)) / 16);
//        y2 = (int16_t) (y1 + ((int8_t) ((rnd >> 4)) / 16));

//        x2 = (int16_t) (x1 + (8.0 * cos((rnd % 360) * 0.0174532925)));
//        y2 = (int16_t) (y1 + (8.0 * sin((rnd % 360) * 0.0174532925)));
//
//        if (x2 <= -centerX) x2 = (int16_t) (-centerX + 1);
//        else if (x2 >= centerX) x2 = (int16_t) (centerX - 1);
//
//        if (y2 <= -centerY) y2 = (int16_t) (-centerY + 1);
//        else if (y2 >= centerX) y2 = (int16_t) (centerX - 1);
//
//        BSP_LCD_DrawLine((uint16_t) centerX + x1, (uint16_t) centerY + y1, (uint16_t) centerX + x2, (uint16_t) centerY + y2);
//        x1 = x2;
//        y1 = y2;

//        HAL_Delay(delay);

        sprintf(buff, "%4d", delay);
        BSP_LCD_DisplayStringAtLine(0, buff);
//
//        if ((i++ >> 4) & 1) BSP_LED_Toggle(LED1);
//
//
//        HAL_RNG_GenerateRandomNumber(&hrng, &rnd);
//        BSP_LCD_DrawLine((prevRnd >> 16) % sizeX, (prevRnd & 0xFFFF) % sizeY, (rnd >> 16) % sizeX, (rnd & 0xFFFF) % sizeY);
//        prevRnd = rnd;
//
//        HAL_Delay(delay);
    }
#pragma clang diagnostic pop
}