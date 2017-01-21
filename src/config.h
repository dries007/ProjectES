#ifndef PROJECT_CONFIG_H
#define PROJECT_CONFIG_H

#include <stdbool.h>
#include <stdint.h>

// Sometimes C is weird. This is one of those times.
#define config_STR( x ) #x
#define config_XSTR( x ) config_STR( x )

#define config_BYTES2IP(a, b, c, d) ( b1 | ( b2 << 8 ) | ( b3 << 16 ) | ( b4 << 24 ))

#define config_FILENAME "LOGGER.INI"
#define config_INTERVAL_MAX 18000

#define config_PORT_MIN 1024
#define config_PORT_MAX 65534

/* ------------------------------------------------------------------------- */

typedef struct
{
    int32_t lInterval;
    int32_t lId;
    uint16_t usPort;
    struct ip4_addr usPushIp;
    uint16_t usPushPort;
} ConfigType;

/* ------------------------------------------------------------------------- */

// The app configuration
extern ConfigType xConfigInstance;

/* ------------------------------------------------------------------------- */

/**
 * Load the config file.
 * Will terminate and write default if it doesn't exist.
 */
void vConfigLoad( void );

#endif //PROJECT_CONFIG_H
