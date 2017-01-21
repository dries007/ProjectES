#ifndef PROJECT_LOG_H
#define PROJECT_LOG_H

// And the #1 prize for most original filename goes to:
#define log_FILENAME "LOGGER.LOG"

// Way too big, just in case. Its a global, so it doesn't take up stack.
#define log_BUFFER_SIZE 1024
// YYYY/MM/DD hh:mm:ss + null = 20
#define log_TIMESTAMP_SIZE 20

#define lod_Y2K 2000

#define log_ASSERT( x, msg ) if ( !( x ) ) { vLogCrash( msg ); }

/* ------------------------------------------------------------------------- */

/**
 * printf for LCD & File
 */
void vLogBothF( const char *format, ... );

/**
 * puts for LCD & File
 */
void vLogBoth( const char *string );

/**
 * printf for LCD
 */
void vLogLcdF( const char *format, ... );

/**
 * puts for LCD
 */
void vLogLcd( const char *string );

/**
 * printf for File
 */
void vLogFileF( const char *format, ... );

/**
 * puts for LCD
 */
void vLogFile( const char *string );

/**
 * Displays the message on the LCD.
 * Contains infinite loop which blinks the LED forever.
 * Used by assert macro.
 * @param message
 */
void vLogCrash( const char *message );

/**
 * Get timestamp. Static buffer is returned.
 */
const char * pcLogTimestamp( void );

#endif //PROJECT_LOG_H
