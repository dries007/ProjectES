//
// Created by dries on 27/11/16.
//

#include "os.h"

#define MAX_TASKS 64

Task tasks[MAX_TASKS];
__IO bool triggeredTasks = false;

void OS_Crash()
{
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
    while (1);
#pragma clang diagnostic pop
}

void OS_Main()
{
    // State machine logic here
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
    while (1)
    {
        HAL_Delay(1000);
    }
#pragma clang diagnostic pop
}

bool OS_Add_Task(Task task)
{
    task._used = true;
    task._next = HAL_GetTick();

    for (uint32_t i = 0; i < MAX_TASKS; i++)
    {
        if (!tasks[i]._used)
        {
            tasks[i] = task;
            return true;
        }
    }
    return false;
}

void OS_Run_Tasks()
{
    for (uint32_t i = 0; i < MAX_TASKS; i++)
    {
        Task* task = &tasks[i];
        if (!task->_used || task->_running) continue;
        if (task->_next > HAL_GetTick()) continue;

        task->_running = true;
        int32_t next = task->run();
        task->_running = false;
        if (next >= 0)
        {
            task->_next = HAL_GetTick() + (uint32_t) next;
        }
        else
        {
            task->_used = false;
        }
    }
}
//
//void HAL_SYSTICK_Callback()
//{
//    triggeredTasks = true;
//}
//
//void HAL_Delay(__IO uint32_t Delay)
//{
//    uint32_t end = HAL_GetTick() + Delay;
//    do
//    {
//        if (triggeredTasks)
//        {
//            OS_Run_Tasks();
//        }
//    }
//    while(HAL_GetTick() < end);
//}
