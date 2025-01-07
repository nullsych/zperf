/**
 * @file   SysPerf_Constants.h
 * @brief
 * @author kvant6ubl
 * @date   Nov 19, 2024
 *
 */

#ifndef SYS_PERF_CONSTANTS_H
#define SYS_PERF_CONSTANTS_H

#include "RTOS_Map.h"

#define SP_TASK_COUNT                       MAP_RTOS_OVERALL_TASKS_COUNT /* Overall number of Tasks in the system */
#define SP_IDLE_COUNT                       MAP_RTOS_IDLE_TASK_COUNT     /* Total number of Idle Tasks            */
#define SP_TASK_NAME_LEN                    MAP_RTOS_TASK_MAX_NAME_LEN   /* Maximum length for Task's name        */
#define SP_TIMESTAMP_TOP                    MAP_RTOS_UPTIME_TOP
#define SP_UNINIT                           (0x0E)

#endif /* SYS_PERF_CONSTANTS_H */
