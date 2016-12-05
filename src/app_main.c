/**
 * App main
 * Because main.c/h is in use by HAL
 *
 * @author Dries007
 */

#include "app_main.h"

void app_pre_init();
void app_init();
void app_post_init();
void app_main_thread();

void lcdTask(void const * argument);
void ledTask(void const * argument);

extern void Error_Handler(void);

extern I2C_HandleTypeDef hi2c1;

void usb_test()
{
    FRESULT res;
    FATFS fs;
    DIR dir;
    FILINFO fno;

    lcdDebug(USBH_Path);

    osDelay(5000);
    res = f_mount(&fs, USBH_Path, 0); configASSERT_EQ(res, FR_OK);
    osDelay(5000);
    res = f_opendir(&dir, USBH_Path); configASSERT_EQ(res, FR_OK);
    osDelay(10);

    for (uint32_t count = 0; true; count++)
    {
        res = f_readdir(&dir, &fno);
        configASSERT_EQ(res, FR_OK);
        if (fno.fname[0] == 0) break;

        BSP_LCD_DisplayStringAt(0, (uint16_t) (count * BSP_LCD_GetFont()->Height), (uint8_t *) fno.fname, RIGHT_MODE);
    }
    osDelay(10);

    FIL file;
    char fname[64];
    strcat(fname, USBH_Path);
    strcat(fname, "TEST.TXT");

    res = f_open(&file, fname, FA_READ | FA_WRITE | FA_OPEN_ALWAYS); configASSERT_EQ(res, FR_OK);
    osDelay(10);
    res = f_lseek(&file, f_size(&file)); configASSERT_EQ(res, FR_OK);
    osDelay(10);
    int wr = 0;
    wr = f_printf(&file, "TEST!\t\n");
    configASSERT_EQ(wr, EOF);
    osDelay(10);
    res = f_sync(&file); configASSERT_EQ(res, FR_OK);
    osDelay(10);
    res = f_close(&file); configASSERT_EQ(res, FR_OK);
    osDelay(10);
    res = f_mount(NULL, SD_Path, 1); configASSERT_EQ(res, FR_OK);
}

void sd_test()
{
    BSP_LCD_Clear(0);

    FRESULT res;
    FATFS fs;
    DIR dir;
    FILINFO fno;

    osDelay(10);
    res = f_mount(&fs, SD_Path, 1); configASSERT_EQ(res, FR_OK);
    osDelay(10);
    res = f_opendir(&dir, SD_Path); configASSERT_EQ(res, FR_OK);
    osDelay(10);

    for (uint32_t count = 0; true; count++)
    {
        res = f_readdir(&dir, &fno);
        configASSERT_EQ(res, FR_OK);
        if (fno.fname[0] == 0) break;

        BSP_LCD_DisplayStringAt(0, (uint16_t) (count * BSP_LCD_GetFont()->Height), (uint8_t *) fno.fname, LEFT_MODE);
    }
    osDelay(10);

    FIL file;
    char fname[64] = "";
    strcat(fname, SD_Path);
    strcat(fname, "TEST.TXT");

    res = f_open(&file, fname, FA_READ | FA_WRITE | FA_OPEN_ALWAYS); configASSERT_EQ(res, FR_OK);
    osDelay(10);
    res = f_lseek(&file, f_size(&file)); configASSERT_EQ(res, FR_OK);
    osDelay(10);

    int wr = 0;
    wr = f_printf(&file, "TEST!\t\n");
    configASSERT_NEQ(wr, EOF);

    osDelay(10);
    res = f_sync(&file); configASSERT_EQ(res, FR_OK);
    osDelay(10);
    res = f_close(&file); configASSERT_EQ(res, FR_OK);
    osDelay(10);
    res = f_mount(NULL, SD_Path, 1); configASSERT_EQ(res, FR_OK);
}

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
    BSP_SD_Init();

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

    MX_FATFS_Init();

//    USB_HC_Init()
//
//    while (!BSP_SD_IsDetected())
//    {
//        BSP_LCD_DisplayStringAt(0, (uint16_t) (BSP_LCD_GetFont()->Height * 2), (uint8_t*) "Please insert SD Card...", CENTER_MODE);
//        HAL_Delay(10);
//    }
//    BSP_LCD_ClearStringLine(2);
//
//    configASSERT_EQ(BSP_SD_Init(), MSD_OK);
//
//    MX_FATFS_Init();
//
//    BSP_LCD_DisplayStringAt(0, (uint16_t) (BSP_LCD_GetFont()->Height * 2), (uint8_t*) SD_Path, CENTER_MODE);
//    BSP_LCD_DisplayStringAt(0, (uint16_t) (BSP_LCD_GetFont()->Height * 3), (uint8_t*) USBH_Path, CENTER_MODE);

}

void app_post_init()
{
    osThreadDef(lcdBG, lcdTask, osPriorityAboveNormal, 0, configMINIMAL_STACK_SIZE);
    osThreadCreate(osThread(lcdBG), NULL);
//
//    osThreadDef(ledToggle, ledTask, osPriorityAboveNormal, 0, configMINIMAL_STACK_SIZE);
//    osThreadCreate(osThread(ledToggle), NULL);
}

void app_main_thread()
{
    uint32_t previousWakeTime = osKernelSysTick();

    BSP_LCD_Clear(0);

//    sd_test();
//    usb_test();

    HAL_StatusTypeDef status;

    status = HAL_I2C_Init(&hi2c1);
    configASSERT_EQ(status, HAL_OK);

    char data[0x14];
    data[0x13] = '\0';

    status = HAL_I2C_IsDeviceReady(&hi2c1, RTC_I2C_ADDR, 300, 300);
    configASSERT_EQ(status, HAL_OK);

    status = HAL_I2C_Mem_Read(&hi2c1, RTC_I2C_ADDR, 0x00, 1, (uint8_t *) data, 0x12, 300);
    configASSERT_EQ(status, HAL_OK);

    lcdDebug(data);

    BSP_LCD_Clear(0);

    while (true)
    {
        osDelayUntil(&previousWakeTime, 100);
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

        osDelayUntil(&previousWakeTime, 25);
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