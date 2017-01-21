#include <lwip.h>
#include <string.h>
#include <ctype.h>
#include <rtc.h>

#include "config.h"
#include "log.h"
#include "temp.h"

#include "net.h"

extern struct netif *netif_default;

/* ------------------------------------------------------------------------- */

static struct udp_pcb *xUdpPcb;

/* ------------------------------------------------------------------------- */

// Initializes all BSP requirements.
static void
prvUDPCallback( void *pvArg, struct udp_pcb *pxPcb, struct pbuf *pxBuf, const ip_addr_t *pxAddr, u16_t usPort );

/* ------------------------------------------------------------------------- */

void vNetInit( void )
{
err_t ucReturn;

    vLogLcd( "Waiting for DHCP..." );

    while( ip4_netif_get_local_ip( netif_default )->addr == 0 )
    {
        MX_LWIP_Process();
    }

    {
        const ip_addr_t *ipAddr = ip4_netif_get_local_ip( netif_default );
        vLogBothF( "IP address: %d.%d.%d.%d", ip4_addr1( ipAddr ), ip4_addr2( ipAddr ), ip4_addr3( ipAddr ),
                   ip4_addr4( ipAddr ));
    }

    xUdpPcb = udp_new();
    log_ASSERT( xUdpPcb != NULL, "UDP init: Could not make UDP PCB." )

    ucReturn = udp_bind( xUdpPcb, IP_ADDR_ANY, xConfigInstance.usPort );
    log_ASSERT( ucReturn == ERR_OK, "Error binding to UDP port." )

    udp_recv( xUdpPcb, prvUDPCallback, NULL);
}

/* ------------------------------------------------------------------------- */

static void
prvUDPCallback( void *pvArg, struct udp_pcb *pxPcb, struct pbuf *pxBuf, const ip_addr_t *pxAddr, u16_t usPort )
{
static char cBuffer[net_BUFFER_SIZE];
err_t ucReturn;
uint16_t usIndex = 0;
uint16_t usToCopy;
struct pbuf *pxI;
uint16_t usCopyLen;
RTC_TimeTypeDef xTime;
RTC_DateTypeDef xDate;

    LWIP_UNUSED_ARG( pvArg );
    if( pxBuf == NULL)
    {
        return;
    }

    { // Almost copied from pbuf_take. Copies all packets into the buffer.
        usToCopy = pxBuf->tot_len;
        if( usToCopy >= net_BUFFER_SIZE - 1 )
        {
            usToCopy = net_BUFFER_SIZE - 1;
        }

        for( pxI = pxBuf; usToCopy != 0; pxI = pxI->next )
        {
            usCopyLen = usToCopy;
            if( usCopyLen > pxI->len ) // Not yet last packet.
            {
                usCopyLen = pxI->len;
            }
            memcpy( cBuffer + usIndex, pxI->payload, usCopyLen );
            usToCopy -= usCopyLen;
            usIndex += usCopyLen;
        }
        // Crude trim. Meant for newline's and spaces appended to the command string.
        usIndex = pxBuf->tot_len;
        do
        {
            cBuffer[usIndex--] = 0;
        }
        while( usIndex > 0 && !isprint( cBuffer[usIndex] ));
    }

    // Command parser

    if( strcmp( cBuffer, "last" ) == 0 )
    {
        strcpy( cBuffer, "LAST: " );
        strcat( cBuffer, pcTempGetLastLog());
        strcat( cBuffer, "\n" );
    }
    else if( strcmp( cBuffer, "log" ) == 0 )
    {
        vTempLog();
        strcpy( cBuffer, "LOG: " );
        strcat( cBuffer, pcTempGetLastLog());
        strcat( cBuffer, "\n" );
    }
    else if( strcmp( cBuffer, "config" ) == 0 )
    {
        sprintf( cBuffer, "ID: %d Interval: %d Push: %d.%d.%d.%d:%d\n",
                 (int) xConfigInstance.lId,
                 (int) xConfigInstance.lInterval,
                 ip4_addr1( &xConfigInstance.usPushIp ),
                 ip4_addr2( &xConfigInstance.usPushIp ),
                 ip4_addr3( &xConfigInstance.usPushIp ),
                 ip4_addr4( &xConfigInstance.usPushIp ),
                 xConfigInstance.usPort );
    }
    else if( strcmp( cBuffer, "time" ) == 0 )
    {
        HAL_RTC_GetDate( &hrtc, &xDate, RTC_FORMAT_BIN);
        HAL_RTC_GetTime( &hrtc, &xTime, RTC_FORMAT_BIN);

        strcpy(cBuffer, "TIME: ");
        strcat(cBuffer, pcLogTimestamp());
        strcat(cBuffer, "\n");
    }
    else
    {
        strcpy( cBuffer, "ERROR: Unknown command.\n" );
    }

    { // Common send logic
        pbuf_free( pxBuf );

        pxBuf = pbuf_alloc( PBUF_TRANSPORT, (u16_t) ( strlen( cBuffer )), PBUF_RAM );
        log_ASSERT( pxBuf != NULL, "UDP reply: Could not allocate buffer." )

        ucReturn = pbuf_take( pxBuf, cBuffer, (u16_t) strlen( cBuffer ));
        log_ASSERT( ucReturn == ERR_OK, "UDP reply: Could not copy data." )

        ucReturn = udp_sendto( pxPcb, pxBuf, pxAddr, usPort );
        log_ASSERT( ucReturn == ERR_OK, "UDP reply: Could not send reply." )

        pbuf_free( pxBuf );
    }
}

/* ------------------------------------------------------------------------- */

void vNetPush( const char *string )
{
static char cBuffer[net_BUFFER_SIZE];
struct udp_pcb *xUdpPcb;
struct pbuf *pxBuf;
err_t ucReturn;

    if( xConfigInstance.usPushPort == 0 )
    {
        vLogLcd( "No push" );
        return;
    }

    sprintf( cBuffer, "PUSH %d: ", (int) xConfigInstance.lId );
    strcat( cBuffer, string );

    xUdpPcb = udp_new();
    log_ASSERT( xUdpPcb != NULL, "UDP push: Could not make UDP PCB." )

    ucReturn = udp_connect( xUdpPcb, &xConfigInstance.usPushIp, xConfigInstance.usPushPort );
    log_ASSERT( ucReturn == ERR_OK, "UDP push: Could not copy data." )

    pxBuf = pbuf_alloc( PBUF_TRANSPORT, (u16_t) ( strlen( cBuffer )), PBUF_RAM );
    log_ASSERT( pxBuf != NULL, "UDP push: Could not allocate buffer." )

    ucReturn = pbuf_take( pxBuf, cBuffer, (u16_t) strlen( cBuffer ));
    log_ASSERT( ucReturn == ERR_OK, "UDP push: Could not copy data." )

    ucReturn = udp_send( xUdpPcb, pxBuf );
    log_ASSERT( ucReturn == ERR_OK, "UDP push: Could not send data." )

    pbuf_free( pxBuf );
    udp_disconnect( xUdpPcb );
    udp_remove( xUdpPcb );
}

/* ------------------------------------------------------------------------- */
