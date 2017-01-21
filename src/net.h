#ifndef PROJECT_NET_H
#define PROJECT_NET_H

#define net_BUFFER_SIZE 1000

/* ------------------------------------------------------------------------- */

/**
 * Sets up the UDP server
 */
void vNetInit( void );

/**
 * Push string to UDP server if configured to do so
 * @param string
 */
void vNetPush( const char *string );

#endif //PROJECT_NET_H
