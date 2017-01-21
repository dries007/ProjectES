#ifndef PROJECT_TEMP_H
#define PROJECT_TEMP_H

/**
 * Make new temp log entry.
 */
void vTempLog( void );

/**
 * @return last log entry's data. Never NULL.
 */
const char *pcTempGetLastLog( void );

#endif //PROJECT_TEMP_H
