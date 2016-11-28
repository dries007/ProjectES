/**
 * App main
 * Because main.c/h is in use by HAL
 *
 * @author Dries007
 */

#include <ff.h>
#include "app_main.h"

void app_pre_init();
void app_init();
void app_post_init();
void app_main_thread();

void lcdTask(void const * argument);
void ledTask(void const * argument);

extern void Error_Handler(void);

void app_pre_init()
{
    SCB_EnableICache();
    SCB_EnableDCache();
}

void app_init()
{
    BSP_LED_Init(LED1);
    BSP_PB_Init(BUTTON_KEY, BUTTON_MODE_GPIO);
    BSP_LCD_Init(0, 0, 0);

    BSP_LCD_LayerDefaultInit(0, LCD_FB_START_ADDRESS);
    BSP_LCD_LayerDefaultInit(1, LCD_FB_START_ADDRESS+(BSP_LCD_GetXSize()*BSP_LCD_GetYSize()*4));
    BSP_LCD_SelectLayer(0);
    BSP_LCD_SetFont(&LCD_DEFAULT_FONT);
    BSP_LCD_SetBackColor(0);
    BSP_LCD_Clear(0);
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);

    BSP_LCD_SelectLayer(1);
    BSP_LCD_SetFont(&LCD_DEFAULT_FONT);
    BSP_LCD_SetBackColor(0);
    BSP_LCD_Clear(0);
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);

    BSP_LCD_DisplayStringAt(0, 0, (uint8_t *) "Booting...", CENTER_MODE);
    BSP_LCD_DisplayStringAt(0, BSP_LCD_GetFont()->Height, (uint8_t *) "DHCP takes a while...", CENTER_MODE);

    while (!BSP_SD_IsDetected())
    {
        BSP_LCD_DisplayStringAt(0, (uint16_t) (BSP_LCD_GetFont()->Height * 2), (uint8_t*) "Please insert SD Card...", CENTER_MODE);
        HAL_Delay(10);
    }
    BSP_LCD_ClearStringLine(2);

    configASSERT_EQ(BSP_SD_Init(), MSD_OK);

    MX_FATFS_Init();

    BSP_LCD_DisplayStringAt(0, (uint16_t) (BSP_LCD_GetFont()->Height * 2), (uint8_t*) SD_Path, CENTER_MODE);

    FRESULT res;
    FATFS fs;
    DIR dir;
    FILINFO fno;

    res = f_mount(&fs, "", 1); configASSERT_EQ(res, FR_OK);
    res = f_opendir(&dir, ""); configASSERT_EQ(res, FR_OK);

    for (uint32_t count = 0; true; count++)
    {
        res = f_readdir(&dir, &fno);
        configASSERT_EQ(res, FR_OK);
        if (fno.fname[0] == 0) break;

        BSP_LCD_DisplayStringAtLine((uint16_t) (count + 3), (uint8_t *) fno.fname);
    }


    FIL file;
    res = f_open(&file, "TEST.TXT", FA_READ | FA_WRITE | FA_OPEN_ALWAYS);
    configASSERT_EQ(res, FR_OK);

    res = f_lseek(&file, f_size(&file));
    configASSERT_EQ(res, FR_OK);

    unsigned int wr = 0;
    res = f_write(&file, "OMG!", 4, &wr);
    configASSERT_EQ(res, FR_OK);

//    res = f_sync(&file);
//    configASSERT_EQ(res, FR_OK);

    res = f_close(&file);
    configASSERT_EQ(res, FR_OK);

    res = f_mount(NULL, "", 1);
    configASSERT_EQ(res, FR_OK);

    char str[128];
    sprintf(str, "OMG! Wr: %d Size: %d", wr, (int) f_size(&file));
    BSP_LCD_DisplayStringAt(0, (uint16_t) (BSP_LCD_GetYSize() / 2) - BSP_LCD_GetFont()->Height, (uint8_t*) str, CENTER_MODE);
}

void app_post_init()
{
    osThreadDef(lcdBG, lcdTask, osPriorityAboveNormal, 0, configMINIMAL_STACK_SIZE);
    osThreadCreate(osThread(lcdBG), NULL);

    osThreadDef(ledToggle, ledTask, osPriorityAboveNormal, 0, configMINIMAL_STACK_SIZE);
    osThreadCreate(osThread(ledToggle), NULL);
}

void app_main_thread()
{
    uint32_t previousWakeTime = osKernelSysTick();

    BSP_LCD_Clear(0);

    while (true)
    {
        if (BSP_PB_GetState(BUTTON_KEY))
        {
            HAL_NVIC_SystemReset();
        }
        osDelayUntil(&previousWakeTime, 10);
    }
}

void lcdTask(void const * argument)
{
    uint32_t previousWakeTime = osKernelSysTick();
    ColorHSV hsv = {000, 1.00, 0.75};
    ColorRGB rgb;

    while (true)
    {
        hsv.h += 1;
        if (hsv.h >= 360) hsv.h -= 360;
        rgb = hsv2rgb(hsv);

        LTDC->BCCR = ((uint8_t) (rgb.r * 0xFF) << 16 | (uint8_t) (rgb.g * 0xFF) << 8 | (uint8_t) (rgb.b * 0xFF));

        osDelayUntil(&previousWakeTime, 10);
    }
}

void ledTask(void const * argument)
{
    uint32_t previousWakeTime = osKernelSysTick();
    while (true)
    {
        BSP_LED_Toggle(LED1);

        osDelayUntil(&previousWakeTime, 100);
    }
}