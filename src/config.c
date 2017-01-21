#include <string.h>
#include <stdlib.h>
#include <limits.h>

#include <ff.h>

#include <lwip/ip4_addr.h>

#include "config.h"
#include "ini.h"
#include "log.h"


//Single config instance
ConfigType xConfigInstance;

/* ------------------------------------------------------------------------- */

// SD logical drive path ("0:/" if all goes well)
extern char SD_Path[4]; // Not my magic constant!

/* ------------------------------------------------------------------------- */

// ini parser handler
static int prvHandler( void *user, const char *section, const char *name, const char *value );

/* ------------------------------------------------------------------------- */

void vConfigLoad( void )
{
FRESULT eFResult;
FATFS xSDFatFs;
FIL xFile;
int32_t lReturn;

    // opt = true -> force mount now
    eFResult = f_mount( &xSDFatFs, SD_Path, true );
    log_ASSERT( eFResult == FR_OK, "SD mount failed. Is it FAT formatted?" );

    eFResult = f_open( &xFile, config_FILENAME, FA_READ );

    if( eFResult == FR_NO_FILE )
    {
        vLogLcd( "No " config_FILENAME " file found. Creating default." );

        eFResult = f_open( &xFile, config_FILENAME, FA_WRITE | FA_CREATE_NEW );
        log_ASSERT( eFResult == FR_OK, "Failed to open file for writing." );

        lReturn = f_puts( "; Logger config\n"
                                  "; Windows ini file format\n"
                                  "; All section and properties are lowercase.\n"
                                  "\n"
                                  "[general]\n"
                                  "interval = 300\n"
                                  "id = 0\n"
                                  "\n"
                                  "[server]\n"
                                  "port = 41752\n"
                                  "\n"
                                  "[push]\n"
                                  "ip = 255.255.255.255\n"
                                  "port = 0\n", &xFile );
        log_ASSERT( lReturn != EOF, "Example config file write failed." );

        eFResult = f_sync( &xFile );
        log_ASSERT( eFResult == FR_OK, "Example config file sync failed." );

        eFResult = f_close( &xFile );
        log_ASSERT( eFResult == FR_OK, "Example config file close failed." );

        vLogCrash( "Example config file written. Please reset when ready." );
    }
    log_ASSERT( eFResult == FR_OK, "Config file open failed." );

    lReturn = ini_parse_stream((ini_reader) f_gets, &xFile, prvHandler, &xConfigInstance );
    log_ASSERT( lReturn >= 0, "Can't load config file "
            config_FILENAME )

    vLogBoth( "Configuration:" );
    vLogBothF( "- Interval: %d seconds", xConfigInstance.lInterval );
    vLogBothF( "- Port: %d", xConfigInstance.usPort );

    log_ASSERT( xConfigInstance.lInterval > 1, "Configuration: Interval <= 0 or error parsing." )
    log_ASSERT( xConfigInstance.lInterval <= config_INTERVAL_MAX,
                "Configuration: Interval > " config_XSTR( config_INTERVAL_MAX ) );

    log_ASSERT( xConfigInstance.lId > 1, "Configuration: Id <= 0 or error parsing." )

    log_ASSERT( xConfigInstance.usPort > config_PORT_MIN,
                "Configuration: Server Port <= " config_XSTR( config_PORT_MIN ) " or error parsing." )
    log_ASSERT( xConfigInstance.usPort <= config_PORT_MAX,
                "Configuration: Server Port > " config_XSTR( config_PORT_MAX ) );

    if( xConfigInstance.usPushPort != 0 )
    {
        log_ASSERT( xConfigInstance.usPushPort > config_PORT_MIN,
                    "Configuration: Push Port <= " config_XSTR( config_PORT_MIN ) " or error parsing." )
        log_ASSERT( xConfigInstance.usPushPort <= config_PORT_MAX,
                    "Configuration: Push Port > " config_XSTR( config_PORT_MAX ) );
        vLogBothF( "- Push to: %d.%d.%d.%d:%d",
                   ip4_addr1( &xConfigInstance.usPushIp ),
                   ip4_addr2( &xConfigInstance.usPushIp ),
                   ip4_addr3( &xConfigInstance.usPushIp ),
                   ip4_addr4( &xConfigInstance.usPushIp ),
                   xConfigInstance.usPort );
    }
    else
    {
        vLogBothF( "- Push is disabled" );
    }

    // No need to close or sync the cfg. it was read only.

    // NULL = unmount, opt is unused in unmount.
    eFResult = f_mount(NULL, SD_Path, false );
    log_ASSERT( eFResult == FR_OK, "Could not unmount SD." );
}

/* ------------------------------------------------------------------------- */

int prvHandler( void *user, const char *section, const char *name, const char *value )
{
int b1 = 0, b2 = 0, b3 = 0, b4 = 0, s; // scanf doesn't like int32_t for some reason.
ConfigType *pConfig = (ConfigType *) user;

    if( strcmp( section, "general" ) == 0 && strcmp( name, "interval" ) == 0 )
    {
        pConfig->lInterval = atoi( value );
    }
    else if( strcmp( section, "general" ) == 0 && strcmp( name, "id" ) == 0 )
    {
        pConfig->lId = atoi( value );
    }
    else if( strcmp( section, "server" ) == 0 && strcmp( name, "port" ) == 0 )
    {
        pConfig->usPort = (uint16_t) atoi( value );
    }
    else if( strcmp( section, "push" ) == 0 && strcmp( name, "ip" ) == 0 )
    {
        s = sscanf( value, "%d.%d.%d.%d", &b1, &b2, &b3, &b4 );
        log_ASSERT( s == 4, "Config: Push IP can't be parsed." )
        if( b1 < 0 || b1 > UCHAR_MAX || b2 < 0 || b2 > UCHAR_MAX ||
                b3 < 0 || b3 > UCHAR_MAX || b4 < 0 || b4 > UCHAR_MAX )
        {
            vLogCrash( "Config: Push IP error." );
        }
        ip4_addr_set_u32( &pConfig->usPushIp, config_BYTES2IP(b1, b2, b3, b4));
    }
    else if( strcmp( section, "push" ) == 0 && strcmp( name, "port" ) == 0 )
    {
        pConfig->usPushPort = (uint16_t) atoi( value );
    }
    else
    {
        return 0;
    }

    return 1;
}

/* ------------------------------------------------------------------------- */
