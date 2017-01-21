#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>

#include <stm32746g_discovery_lcd.h>
#include <ff.h>
#include <rtc.h>

#include "app.h"

#include "log.h"

// The file was made possible by our sponsor StackOverflow! Please visit:
// https://stackoverflow.com/questions/1516370/wrapper-printf-function-that-filters-according-to-user-preferences

//SD logical drive path ("0:/" if all goes well)
extern char SD_Path[4]; // Not my magic number!

/* ------------------------------------------------------------------------- */

// Poor mans toilet sign, because logging an error to file while logging an error to file is not pretty.
static bool bPrintingToFile = false;

// Common buffer. Reduces stack usage.
static char cBuffer[log_BUFFER_SIZE];

/* ------------------------------------------------------------------------- */

void vLogBothF( const char *format, ... )
{
va_list args;

    va_start( args, format );
    vsprintf( cBuffer, format, args );
    va_end( args );

    vLogBoth( cBuffer );
}

/* ------------------------------------------------------------------------- */

void vLogBoth( const char *string )
{
    vLogFile( string );
    vLogLcd( string );
}

/* ------------------------------------------------------------------------- */

void vLogFileF( const char *format, ... )
{
va_list args;

    va_start( args, format );
    vsprintf( cBuffer, format, args );
    va_end( args );

    vLogFile( cBuffer );
}

/* ------------------------------------------------------------------------- */

void vLogFile( const char *string )
{
FRESULT eFResult = FR_OK;
FATFS xSDFatFs;
FIL xFile;
int32_t lResult = 0;
bPrintingToFile = true;

    log_ASSERT(BSP_SD_IsDetected() == SD_PRESENT, "SD card removed.");

    // Sometimes the SD just doesn't wanna mount on the first try.
    while( lResult++ < app_SD_ATTEMPTS )
    {
        // opt = true -> force mount now
        eFResult = f_mount( &xSDFatFs, SD_Path, true );
        if( eFResult == FR_OK )
        {
            break;
        }
        HAL_Delay( app_SD_DELAY );
    }
    log_ASSERT( eFResult == FR_OK, "SD mount failed 10 times. Is it FAT formatted?" );


    eFResult = f_open( &xFile, log_FILENAME, FA_WRITE | FA_OPEN_ALWAYS );
    log_ASSERT( eFResult == FR_OK, "Failed to open log file " log_FILENAME );

    eFResult = f_lseek( &xFile, f_size( &xFile ));
    log_ASSERT( eFResult == FR_OK, "Failed to find end of log file." );

    lResult = f_puts( string, &xFile );
    log_ASSERT( lResult != EOF, "Failed to write to logfile." );
    lResult = f_putc( '\n', &xFile );
    log_ASSERT( lResult != EOF, "Failed to write to logfile." );

    eFResult = f_sync( &xFile );
    log_ASSERT( eFResult == FR_OK, "Log file sync failed." );

    eFResult = f_close( &xFile );
    log_ASSERT( eFResult == FR_OK, "Log file close failed." );

    // NULL = unmount, opt is unused in unmount.
    eFResult = f_mount(NULL, SD_Path, false );
    log_ASSERT( eFResult == FR_OK, "Could not unmount SD." );

    bPrintingToFile = false;
}

/* ------------------------------------------------------------------------- */

void vLogLcdF( const char *format, ... )
{
va_list args;

    if( format == NULL)
    {
        vLogLcd(NULL);
        return;
    }

    va_start( args, format );
    vsprintf( cBuffer, format, args );
    va_end( args );

    vLogLcd( cBuffer );
}

/* ------------------------------------------------------------------------- */

void vLogLcd( const char *string )
{
static uint16_t ulLine = 0;

    if( string == NULL)
    {
        BSP_LCD_Clear(LCD_COLOR_TRANSPARENT);
        ulLine = 0;
        return;
    }

    uint32_t oldColor = BSP_LCD_GetTextColor();
    BSP_LCD_SetTextColor( BSP_LCD_GetBackColor());
    BSP_LCD_FillRect( 0, BSP_LCD_GetFont()->Height * ulLine, (uint16_t) BSP_LCD_GetXSize(), BSP_LCD_GetFont()->Height );
    BSP_LCD_SetTextColor( oldColor );

    BSP_LCD_DisplayStringAtLine( ulLine++, (uint8_t *) string );

    if( ulLine >= ( BSP_LCD_GetYSize() / BSP_LCD_GetFont()->Height ))
    {
        ulLine = 0;
    }

    BSP_LCD_SetTextColor( BSP_LCD_GetBackColor());
    BSP_LCD_FillRect( 0, BSP_LCD_GetFont()->Height * ulLine, (uint16_t) BSP_LCD_GetXSize(), BSP_LCD_GetFont()->Height );
    BSP_LCD_SetTextColor( oldColor );
}

/* ------------------------------------------------------------------------- */

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"

void vLogCrash( const char *message )
{
    BSP_LCD_SetTextColor(LCD_COLOR_RED);

    if( !bPrintingToFile )
    {
        vLogLcd( "FATAL ERROR - NOT LOGGED TO FILE" );
        vLogLcd( message );
    }
    else
    {
        vLogBoth( "FATAL ERROR" );
        vLogBoth( message );
    }

    while( true )
    {
        BSP_LED_Toggle( LED1 );
        HAL_Delay( 250 );
    }
}

#pragma clang diagnostic pop

/* ------------------------------------------------------------------------- */

const char* pcLogTimestamp( void )
{
static char strTimestamp[log_TIMESTAMP_SIZE];
RTC_TimeTypeDef xTime;
RTC_DateTypeDef xDate;

    HAL_RTC_GetDate( &hrtc, &xDate, RTC_FORMAT_BIN );
    HAL_RTC_GetTime( &hrtc, &xTime, RTC_FORMAT_BIN );

    sprintf(strTimestamp, "%04d/%02d/%02d %02d:%02d:%02d",
            lod_Y2K + xDate.Year, xDate.Month, xDate.Date, xTime.Hours, xTime.Minutes, xTime.Seconds);

    return strTimestamp;
}

/* ------------------------------------------------------------------------- */
