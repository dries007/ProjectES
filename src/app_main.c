/**
 * App main
 * Because main.c/h is in use by HAL
 *
 * @author Dries007
 */

#include "app_main.h"
#include "tmp.h"

// Windows: YOU SUCK.
#include "../cubemx/Drivers/STM32F7xx_HAL_Driver/Inc/stm32f7xx_hal_i2c.h"
#include "../cubemx/Middlewares/Third_Party/FatFs/src/ff.h"
#include "../cubemx/Inc/fatfs.h"
#include "../cubemx/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS/cmsis_os.h"
#include "../cubemx/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM7/r0p1/portmacro.h"
#include "../cubemx/Inc/FreeRTOSConfig.h"
#include "../cubemx/Middlewares/Third_Party/FreeRTOS/Source/include/task.h"
#include "../bsp/stm32746g_discovery_lcd.h"
#include "../cubemx/Drivers/CMSIS/Include/core_cm7.h"
#include "../bsp/stm32746g_discovery_sd.h"
#include "../cubemx/Drivers/STM32F7xx_HAL_Driver/Inc/stm32f7xx_hal.h"
#include "../cubemx/Drivers/CMSIS/Device/ST/STM32F7xx/Include/stm32f746xx.h"
#include "../cubemx/Inc/i2c.h"

void app_pre_init();
void app_init();
void app_post_init();
void app_main_thread();

void lcdTask(void const * argument);

extern void Error_Handler(void);

extern I2C_HandleTypeDef hi2c1;

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

//    BSP_LCD_DisplayStringAt(0, BSP_LCD_GetFont()->Height, (uint8_t *) "DHCP takes a while...", CENTER_MODE);

    BSP_SD_Init(); // MSD_OK
    BSP_SD_IsDetected();

    while (!BSP_SD_IsDetected())
    {
        BSP_LCD_DisplayStringAt(0, (uint16_t) (BSP_LCD_GetFont()->Height * 2), (uint8_t*) "Please insert SD Card...", CENTER_MODE);
        HAL_Delay(10);
    }
    BSP_LCD_ClearStringLine(2);

//    BSP_LCD_DisplayStringAt(0, (uint16_t) (BSP_LCD_GetFont()->Height * 2), (uint8_t*) SD_Path, CENTER_MODE);
//    BSP_LCD_DisplayStringAt(0, (uint16_t) (BSP_LCD_GetFont()->Height * 3), (uint8_t*) USBH_Path, CENTER_MODE);
}

void app_post_init()
{
    osThreadDef(lcdBG, lcdTask, osPriorityAboveNormal, 0, configMINIMAL_STACK_SIZE);
    osThreadCreate(osThread(lcdBG), NULL);

//    osThreadDef(ledToggle, ledTask, osPriorityAboveNormal, 0, configMINIMAL_STACK_SIZE);
//    osThreadCreate(osThread(ledToggle), NULL);
}

void app_main_thread()
{
    uint32_t previousWakeTime = osKernelSysTick();

    BSP_LCD_Clear(0);

    MX_I2C1_Init();

    lcdDebug("RTC test");

//    sd_test();
//    usb_test();
//
    HAL_StatusTypeDef status;

    status = HAL_I2C_Init(&hi2c1);
    configASSERT_EQ(status, HAL_OK);

    HAL_I2C_StateTypeDef state = HAL_I2C_GetState(&hi2c1);
    lcdDebugf("STATE: 0x%02x", state);

    HAL_I2C_ModeTypeDef mode = HAL_I2C_GetMode(&hi2c1);
    lcdDebugf("MODE: 0x%02x", mode);

    uint32_t error = HAL_I2C_GetError(&hi2c1);
    lcdDebugf("ERROR: 0x%08x", (unsigned int) error);

    uint8_t data[0x14];
    for (int i = 0; i < 0x14; i++) data[i] = 0;

    status = HAL_I2C_IsDeviceReady(&hi2c1, RTC_I2C_ADDR, 3000, 3000);
    configASSERT_EQ(status, HAL_OK);

    status = HAL_I2C_Mem_Read(&hi2c1, RTC_I2C_ADDR, 0x00, I2C_MEMADD_SIZE_16BIT, (uint8_t *) data, 0x12, 3000);
    configASSERT_EQ(status, HAL_OK);

    lcdDebugf("0: 0x%02x", data[0]);
    lcdDebugf("1: 0x%02x", data[1]);
    lcdDebugf("2: 0x%02x", data[2]);
    lcdDebugf("3: 0x%02x", data[3]);
    lcdDebugf("4: 0x%02x", data[4]);
    lcdDebugf("5: 0x%02x", data[5]);
    lcdDebugf("6: 0x%02x", data[6]);
    lcdDebugf("7: 0x%02x", data[7]);
    lcdDebugf("8: 0x%02x", data[8]);
    lcdDebugf("9: 0x%02x", data[9]);
    lcdDebugf("a: 0x%02x", data[10]);
    lcdDebugf("b: 0x%02x", data[11]);
    lcdDebugf("c: 0x%02x", data[12]);
    lcdDebugf("d: 0x%02x", data[13]);
    lcdDebugf("e: 0x%02x", data[14]);
    lcdDebugf("f: 0x%02x", data[15]);

    lcdDebug((char*)data);

//    BSP_LCD_Clear(0);

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