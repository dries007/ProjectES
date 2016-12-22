//
// Created by dries on 3/12/16.
//

#ifndef PROJECT_TMP_H
#define PROJECT_TMP_H

void lcdDebug(char* buffer);

#define  lcdDebugf(...)   { char buffer[256]; sprintf(buffer, __VA_ARGS__); buffer[255] = 0; lcdDebug(buffer); }

#endif //PROJECT_TMP_H
