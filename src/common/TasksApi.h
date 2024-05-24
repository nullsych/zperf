/**
 * @file   TasksApi.h
 * @brief  Contain creating/deleting user-defined tasks API.
 * @author kvant6ubl
 * @date   May 6, 2024
 *
 */

#ifndef SRC_COMMON_TASKS_API_H
#define SRC_COMMON_TASKS_API_H

#include <zephyr.h>
#include <zephyr/types.h>

#define TASK_LOCAL static

void Led_createTask(void);
void Ble_createTask(void);
void SysPerf_createTask(void);
// void ApplTasks__startThreadAnalyzer(void);

/*
static const char *now_str(void)
{
    static char buf[16]; // ...HH:MM:SS.MMM
    uint32_t now = k_uptime_get_32();
    unsigned int ms = now % MSEC_PER_SEC;
    unsigned int s;
    unsigned int min;
    unsigned int h;

    now /= MSEC_PER_SEC;
    s = now % 60U;
    now /= 60U;
    min = now % 60U;
    now /= 60U;
    h = now;

    snprintf(buf, sizeof(buf), "%u:%02u:%02u.%03u",
         h, min, s, ms);
    return buf;
}
*/

#endif /* SRC_COMMON_TASKS_API_H */