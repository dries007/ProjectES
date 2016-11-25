/**
 * App main
 * Because main.c/h is in use by HAL
 *
 * @author Dries007
 */


#include "app_main.h"

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
    init();

    BSP_LED_Init(LED1);
    BSP_PB_Init(BUTTON_KEY, BUTTON_MODE_GPIO);

    BSP_LCD_Init();
    BSP_LCD_LayerDefaultInit(LTDC_ACTIVE_LAYER, LCD_FRAME_BUFFER);
    BSP_LCD_SelectLayer(LTDC_ACTIVE_LAYER);
    BSP_LCD_SetFont(&LCD_DEFAULT_FONT);
    BSP_LCD_SetBackColor(LCD_COLOR_TRANSPARENT);
    BSP_LCD_Clear(LCD_COLOR_TRANSPARENT);
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);

    const uint16_t sizeX = (const uint16_t) BSP_LCD_GetXSize();
    const uint16_t sizeY = (const uint16_t) BSP_LCD_GetYSize();

    uint32_t i = 0;
    ColorHSV hsvFg = {0, 1, 0.5};
    ColorHSV hsvBg = {0, 1, 0.5};

    ColorRGB rgb;
    uint32_t rnd;
    uint32_t prevRnd;
    uint8_t r, g, b;

    HAL_RNG_GenerateRandomNumber(&hrng, &prevRnd);

    uint32_t delay = 100;
    char buff[150];

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
    while (1)
    {
//        BSP_LCD_SelectLayer(0);
//        hsvBg.h += 1;
//        if (hsvBg.h >= 360) hsvBg.h -= 360;
//        rgb = hsv2rgb(hsvBg);
//        r = (uint8_t) (rgb.r * 0xFF);
//        g = (uint8_t) (rgb.g * 0xFF);
//        b = (uint8_t) (rgb.b * 0xFF);
//        BSP_LCD_SetBackColor((0xAA << 24) | (r << 16) | (g << 8) | (b));
//        BSP_LCD_Clear((0xAA << 24) | (r << 16) | (g << 8) | (b));

//        BSP_LCD_SelectLayer(1);
        hsvFg.h += 3;
        if (hsvFg.h >= 360) hsvFg.h -= 360;
        rgb = hsv2rgb(hsvFg);
        r = (uint8_t) (rgb.r * 0xFF);
        g = (uint8_t) (rgb.g * 0xFF);
        b = (uint8_t) (rgb.b * 0xFF);
        BSP_LCD_SetTextColor((0xAA << 24) | (r << 16) | (g << 8) | (b));

        HAL_RNG_GenerateRandomNumber(&hrng, &rnd);
        BSP_LCD_DrawLine((prevRnd >> 16) % sizeX, (prevRnd & 0xFFFF) % sizeY, (rnd >> 16) % sizeX, (rnd & 0xFFFF) % sizeY);
        prevRnd = rnd;

        sprintf(buff, "%4d", delay);
        BSP_LCD_DisplayStringAtLine(0, buff);

        if ((i++ >> 4) & 1) BSP_LED_Toggle(LED1);

        if (BSP_PB_GetState(BUTTON_KEY))
        {
            while (BSP_PB_GetState(BUTTON_KEY))
            {
                delay = (delay + 1) % 251;
                sprintf(buff, "%4d", delay);
                BSP_LCD_DisplayStringAtLine(0, buff);
                HAL_Delay(15);
                if (delay == 0) while (BSP_PB_GetState(BUTTON_KEY)) HAL_Delay(10);
            }
            BSP_LCD_Clear(LCD_COLOR_BLACK);
        }
        HAL_Delay(delay);

        BSP_LCD_SetBackColor(LCD_COLOR_TRANSPARENT);
    }
#pragma clang diagnostic pop
}
