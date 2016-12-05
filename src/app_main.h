//
// Created by dries on 24/11/16.
//

#ifndef PROJECT_APP_MAIN_H
#define PROJECT_APP_MAIN_H

#include "FreeRTOSConfig.h"
#include "ffconf.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <main.h>

#include <stm32f7xx_hal.h>
#include <stm32f7xx_hal_ltdc.h>
#include <stm32746g_discovery.h>
#include <stm32746g_discovery_lcd.h>
#include <stm32746g_discovery_ts.h>
#include <stm32746g_discovery_sd.h>
#include <stm32746g_discovery_eeprom.h>
#include <usbh_core.h>
#include <usbh_msc.h>

#include <fatfs.h>
#include <ff.h>

#include <arm_math.h>

#include <cmsis_os.h>

#include "color.h"

#define RTC_I2C_ADDR 0x68
#define EEPROM_I2C_ADDR 0x57

#endif //PROJECT_APP_MAIN_H
