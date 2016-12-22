//
// Created by dries on 3/12/16.
//

#include "tmp.h"
#include "app_main.h"
#include "../bsp/stm32746g_discovery_lcd.h"

void lcdDebug(char *buffer)
{
    static int line = 0;
    BSP_LCD_DisplayStringAtLine((uint16_t) line ++, (uint8_t *) buffer);
}

/*

#if (USBH_DEBUG_LEVEL > 0)
#define  USBH_UsrLog(...)   { char buffer[256]; sprintf(buffer, __VA_ARGS__); buffer[255] = 0; lcdDebug(buffer); }
#else
#define USBH_UsrLog(...)
#endif


#if (USBH_DEBUG_LEVEL > 1)

#define  USBH_ErrLog(...)   { char buffer[256]; sprintf(buffer, __VA_ARGS__); buffer[255] = 0; lcdDebug(buffer); }
#else
#define USBH_ErrLog(...)
#endif


#if (USBH_DEBUG_LEVEL > 2)
#define  USBH_DbgLog(...)   { char buffer[256]; sprintf(buffer, __VA_ARGS__); buffer[255] = 0; lcdDebug(buffer); }
#else
#define USBH_DbgLog(...)
#endif

 */