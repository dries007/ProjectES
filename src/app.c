#include <stm32746g_discovery.h>
#include <stm32746g_discovery_lcd.h>
#include <stm32746g_discovery_sd.h>

#include <rtc.h>
#include <i2c.h>

#include <lwip.h>

#include <fatfs.h>
#include <stm32f7xx_hal_rtc.h>

#include "config.h"
#include "log.h"
#include "temp.h"
#include "net.h"

#include "app.h"

/* ------------------------------------------------------------------------- */

// Initializes all BSP requirements.
static void prvBSPInit( void );

// Waits for, and mounts SD card.
static void prvWaitForSD( void );

// Initialize and syc RTCs
static void prvInitRTC( void );

/* ------------------------------------------------------------------------- */

static uint32_t ulNextLog = 0;

/* ------------------------------------------------------------------------- */

void vAppPreInit( void )
{
    SCB_EnableICache();
    SCB_EnableDCache();
}

/* ------------------------------------------------------------------------- */

void vAppPostInit( void )
{
    prvBSPInit();
    prvWaitForSD();
    prvInitRTC();

    vConfigLoad();

    vNetInit();

    vNetPush( "BOOT" );
}

/* ------------------------------------------------------------------------- */

void vAppLoop( void )
{
    MX_LWIP_Process();

    // fixme: 24 days overflow.
    if( ulNextLog < HAL_GetTick())
    {
        vTempLog();
        vNetPush( pcTempGetLastLog());
        ulNextLog = HAL_GetTick() + ( xConfigInstance.lInterval * app_MS_IN_SEC );
    }
}

/* ------------------------------------------------------------------------- */

static void prvBSPInit( void )
{
    BSP_LED_Init( LED1 );
    BSP_PB_Init( BUTTON_KEY, BUTTON_MODE_GPIO );
    BSP_LCD_Init( 0, 0, 0 );

    BSP_LCD_LayerDefaultInit( 0, LCD_FB_START_ADDRESS);
//    BSP_LCD_LayerDefaultInit( 1, LCD_FB_START_ADDRESS + ( BSP_LCD_GetXSize() * BSP_LCD_GetYSize() * 4 ));

    BSP_LCD_SelectLayer( 0 );
    BSP_LCD_SetFont( &Font12 );
    BSP_LCD_SetBackColor( 0 );
    BSP_LCD_Clear(LCD_COLOR_TRANSPARENT);
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);

//    BSP_LCD_SelectLayer( 1 );
//    BSP_LCD_SetFont( &Font12 );
//    BSP_LCD_SetBackColor( 0 );
//    BSP_LCD_Clear(LCD_COLOR_TRANSPARENT);
//    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);

    vLogLcd(NULL);
    vLogLcd( "Booting..." );
    vLogLcdF( "LCD %d x %d. %d log lines available.", BSP_LCD_GetXSize(), BSP_LCD_GetYSize(),
              BSP_LCD_GetYSize() / BSP_LCD_GetFont()->Height );
}

/* ------------------------------------------------------------------------- */

static void prvInitRTC( void )
{
HAL_StatusTypeDef eHALStatus;
RTC_TimeTypeDef xTime;
RTC_DateTypeDef xDate;
uint8_t ucBuffer[app_RTC_TIME_LEN];

    // Verify external RTC integrity

    // 300 attempts with a 300 tick timeout
    eHALStatus = HAL_I2C_IsDeviceReady( &hi2c1, app_I2C_ADDR_RTC, app_I2C_ATTEMPTS, app_I2C_TIMEOUT );
    log_ASSERT( eHALStatus == HAL_OK, "External RTC: Device not ready. Is it attached?" )

    // Read 2 bytes from 0x0E (config buffer address) with a 300 tick timeout
    eHALStatus = HAL_I2C_Mem_Read( &hi2c1, app_I2C_ADDR_RTC, app_RTC_CFG_ADDR, I2C_MEMADD_SIZE_8BIT, ucBuffer, app_RTC_CFG_LEN, app_I2C_TIMEOUT );
    log_ASSERT( eHALStatus == HAL_OK, "External RTC: Unable to read status." )

    // TODO: ideally show a time set thing, or use NTP is possible to fetch time.
    if( !app_RTC_CFG_EOSC(ucBuffer) || app_RTC_CFG_OSF(ucBuffer) )
    {
        vLogCrash( "RTC Error: EOSC was 0 or OSF was 1!" );
    }

    // external RTC -> internal RTC

    // Read 6 bytes from 0x00 (start of time & date) with 300 tick timeout
    eHALStatus = HAL_I2C_Mem_Read( &hi2c1, app_I2C_ADDR_RTC, app_RTC_TIME_ADDR, I2C_MEMADD_SIZE_8BIT, ucBuffer, app_RTC_TIME_LEN, app_I2C_TIMEOUT );
    log_ASSERT( eHALStatus == HAL_OK, "External RTC: Unable to read time & date." )

    xTime.SubSeconds = 0x00;
    xTime.Seconds = RTC_Bcd2ToByte( app_RTC_TIME_SEC( ucBuffer ) );
    xTime.Minutes = RTC_Bcd2ToByte( app_RTC_TIME_MIN( ucBuffer ) );
    xTime.Hours = RTC_Bcd2ToByte( app_RTC_TIME_HOUR( ucBuffer ) );
    xTime.TimeFormat = RTC_HOURFORMAT_24;
    xTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    xTime.StoreOperation = RTC_STOREOPERATION_RESET;

    eHALStatus = HAL_RTC_SetTime( &hrtc, &xTime, RTC_FORMAT_BIN );
    log_ASSERT( eHALStatus == HAL_OK, "External RTC: Unable to sync internal RTC (time)." )

    xDate.WeekDay = RTC_Bcd2ToByte( app_RTC_TIME_WEEKDAY( ucBuffer ) );
    xDate.Date = RTC_Bcd2ToByte( app_RTC_TIME_DATE( ucBuffer ) );
    xDate.Month = RTC_Bcd2ToByte( app_RTC_TIME_MONTH( ucBuffer ) );
    xDate.Year = RTC_Bcd2ToByte( app_RTC_TIME_YEAR( ucBuffer ) );

    eHALStatus = HAL_RTC_SetDate( &hrtc, &xDate, RTC_FORMAT_BIN );
    log_ASSERT( eHALStatus == HAL_OK, "External RTC: Unable to sync internal RTC (date)." )

    vLogBothF( "UTC Time at startup: %s", pcLogTimestamp( ) );
}

/* ------------------------------------------------------------------------- */

static void prvWaitForSD( void )
{
uint8_t ucAttempts = 0;
uint8_t ucStatus;
uint8_t ucPrevStatus = 0;

    do
    {
        ucStatus = BSP_SD_Init();
        switch( ucStatus )
        {
            case MSD_ERROR_SD_NOT_PRESENT:
                if( ucStatus != ucPrevStatus )
                {
                    vLogLcd( "Please insert SD card..." );
                }
                break;
            case MSD_ERROR:
                if( ucStatus != ucPrevStatus )
                {
                    vLogLcd( "SD error. Trying again..." );
                }
                ucAttempts++;
            case MSD_OK:
            default:break;
        }
        ucPrevStatus = ucStatus;
        HAL_Delay( app_SD_DELAY );
    }
    while( ucStatus != MSD_OK && ucAttempts < app_SD_ATTEMPTS );
    log_ASSERT( ucStatus == MSD_OK, "SD error after " config_XSTR(app_SD_ATTEMPTS) " attempts. Reset required." )

    // If the SD was present at boot, this already happened, and retSD is 0.
    if( retSD != 0 )
    {
        MX_FATFS_Init();
    }
    log_ASSERT( retSD == MSD_OK, "Unable to initialize FatFS" )
}

/* ------------------------------------------------------------------------- */

uint32_t ulAppGetFATTimestamp( void )
{
RTC_TimeTypeDef xTime;
RTC_DateTypeDef xDate;

    HAL_RTC_GetDate( &hrtc, &xDate, RTC_FORMAT_BIN );
    HAL_RTC_GetTime( &hrtc, &xTime, RTC_FORMAT_BIN );

    return app_FAT_TIMESTAMP(xDate.Year, xDate.Month, xDate.Date, xTime.Hours, xTime.Minutes, xTime.Seconds);
}

/* ------------------------------------------------------------------------- */
