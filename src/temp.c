#include <stm32746g_discovery.h>

#include <rtc.h>
#include <i2c.h>
#include <string.h>

#include "temp.h"
#include "app.h"

#include "log.h"

/* ------------------------------------------------------------------------- */

// Read the RTC's temp sensor
static double prvReadRTCTemp( void );

/* ------------------------------------------------------------------------- */

// Common buffer. Remembers last log stored
static char cBuffer[log_BUFFER_SIZE] = "Error: No data yet.";

/* ------------------------------------------------------------------------- */

void vTempLog( void )
{
RTC_TimeTypeDef xTime;
RTC_DateTypeDef xDate;
double dTempRTC;

    HAL_RTC_GetDate( &hrtc, &xDate, RTC_FORMAT_BIN);
    HAL_RTC_GetTime( &hrtc, &xTime, RTC_FORMAT_BIN);

    dTempRTC = prvReadRTCTemp();

    strcpy(cBuffer, pcLogTimestamp());
    // I guess this works. YEY pointers!
    sprintf(cBuffer + strlen(cBuffer), " RTC: %.2lf", dTempRTC);

    vLogBoth( cBuffer );
}

/* ------------------------------------------------------------------------- */

double prvReadRTCTemp( void )
{
HAL_StatusTypeDef eHALStatus;
uint8_t ucBuffer[2];

    eHALStatus = HAL_I2C_IsDeviceReady( &hi2c1, app_I2C_ADDR_RTC, app_I2C_ATTEMPTS, app_I2C_TIMEOUT );
    log_ASSERT( eHALStatus == HAL_OK, "RTC Temp: Device not ready. Is it attached?" )

    // Read 2 bytes from 0x11 (temperature buffer address) with a 300 tick timeout
    eHALStatus = HAL_I2C_Mem_Read( &hi2c1, app_I2C_ADDR_RTC, app_RTC_TEMP_ADDR, I2C_MEMADD_SIZE_8BIT,
                                   ucBuffer, app_RTC_TEMP_LEN, app_I2C_TIMEOUT );
    log_ASSERT( eHALStatus == HAL_OK, "RTC Temp: Unable to read temp buffer." )
    return (double) app_RTC_TEMP2SHORT(ucBuffer) * 0.25;
}

/* ------------------------------------------------------------------------- */

const char *pcTempGetLastLog( void )
{
    return cBuffer;
}

/* ------------------------------------------------------------------------- */
