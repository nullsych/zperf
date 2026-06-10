/**
 * @file   nrf_perf_Constants.h
 * @brief
 * @author nullsych
 * @date   Nov 19, 2024
 *
 */

#ifndef NRF_PERF_CONSTANTS_H
#define NRF_PERF_CONSTANTS_H

#include "rtos_map.h"

#define NP_TASK_COUNT                       MAP_RTOS_OVERALL_TASKS_COUNT /* Overall number of Tasks in the system */
#define NP_IDLE_COUNT                       MAP_RTOS_IDLE_TASK_COUNT     /* Total number of Idle Tasks            */
#define NP_TASK_NAME_LEN                    MAP_RTOS_TASK_MAX_NAME_LEN   /* Maximum length for Task's name        */
#define NP_TIMESTAMP_TOP                    MAP_RTOS_UPTIME_TOP
#define NP_UNINIT                           (0x0E)

#endif /* NRF_PERF_CONSTANTS_H */
