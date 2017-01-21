#ifndef PROJECT_APP_H
#define PROJECT_APP_H

#include <stdbool.h>

#define app_I2C_ATTEMPTS 300
#define app_I2C_TIMEOUT 300
#define app_I2C_ADDR_RTC ( 0x68 << 1 )

#define app_MS_IN_SEC 1000

#define app_RTC_CFG_ADDR 0x0E
#define app_RTC_CFG_LEN 2

#define app_RTC_TEMP_ADDR 0x11
#define app_RTC_TEMP_LEN 2

// If the bit 'bits-1' is 1, subtract (1 < <bits)
#define app_2CPLM2INT(val, bits) (((val & (1 << (bits - 1))) != 0) ? val - (1 << bits) : val)
#define app_RTC_TEMP2SHORT( a ) app_2CPLM2INT((( a[0] << 2 ) | ( a[1] >> 6 )), 10)

#define app_RTC_CFG_EOSC( a ) (( a[0] & ( 1 << 7 )) != ( 1 << 7 ))
#define app_RTC_CFG_OSF( a ) (( a[1] & ( 1 << 7 )) == ( 1 << 7 ))

#define app_RTC_TIME_ADDR 0x00
#define app_RTC_TIME_LEN 7

#define app_SD_ATTEMPTS 10
#define app_SD_DELAY 10

// Cut off bit 7
#define app_RTC_TIME_SEC( a ) ((uint8_t)(a[0] & 0x7F))
// Cut off bit 7
#define app_RTC_TIME_MIN( a ) ((uint8_t)(a[1] & 0x7F))
// Cut off bits 7 & 6
#define app_RTC_TIME_HOUR( a ) ((uint8_t)(a[2] & 0x3F))
// Leave lower 3 bits
#define app_RTC_TIME_WEEKDAY( a ) ((uint8_t)(a[3] & 0x07))
// Cut off bits 7 & 6
#define app_RTC_TIME_DATE( a ) ((uint8_t)(a[4] & 0x3F))
// Cut off bits 7, 6 & 5
#define app_RTC_TIME_MONTH( a ) ((uint8_t)(a[5] & 0x1F))
#define app_RTC_TIME_YEAR( a ) ((uint8_t)(a[6]))

// Thanks https://stackoverflow.com/questions/15763259/unix-timestamp-to-fat-timestamp
#define app_FAT_TIMESTAMP(Y, M, D, h, m, s) \
    ((uint32_t)(((Y + 20) << 25) | (M << 21) | (D << 16) | (h << 11) | (m << 5) | ( s >> 1)))


/* ------------------------------------------------------------------------- */

/**
 * Called before initialization of any perhiperals.
 */
void vAppPreInit( void );

/**
 * Called after initialization of all configured peripherals.
 */
void vAppPostInit( void );

/**
 * Called in (generated) main.c's infinite loop.
 */
void vAppLoop( void );

/**
 * @return FAT format timestamp
 */
uint32_t ulAppGetFATTimestamp( void );

#endif //PROJECT_APP_H
