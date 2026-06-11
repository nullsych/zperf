/**
 * @file   zperf_constants.h
 * @brief
 * @author nullsych
 * @date   Nov 19, 2024
 *
 */

#ifndef ZPERF_CONSTANTS_H
#define ZPERF_CONSTANTS_H

#include "rtos_map.h"

#define ZP_TASK_COUNT    MAP_RTOS_OVERALL_TASKS_COUNT /* Overall number of Tasks in the system */
#define ZP_IDLE_COUNT    MAP_RTOS_IDLE_TASK_COUNT     /* Total number of Idle Tasks            */
#define ZP_TASK_NAME_LEN MAP_RTOS_TASK_MAX_NAME_LEN   /* Maximum length for Task's name        */
#define ZP_TIMESTAMP_TOP MAP_RTOS_UPTIME_TOP
#define ZP_UNINIT        (0x0E)

#endif /* ZPERF_CONSTANTS_H */
