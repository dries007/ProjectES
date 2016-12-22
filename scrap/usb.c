
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

