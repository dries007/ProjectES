//
// Created by dries on 27/11/16.
//

#ifndef PROJECT_OS_H
#define PROJECT_OS_H

#include <stm32f7xx_hal.h>
#include <stdbool.h>

typedef struct strTask
{
    const char* name;
    int32_t (*run)(void);

    // Internal values
    uint32_t _next;
    bool _used;
    bool _running;
} Task;

void OS_Main();
bool OS_Add_Task(Task task);

#endif //PROJECT_OS_H
