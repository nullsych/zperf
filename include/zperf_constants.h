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
/* Marks an unset task iterator. Must stay outside of the valid range [0, ZP_TASK_COUNT) */
#define ZP_UNINIT        (0xFFU)

/* Task iterators are uint8_t and 0xFF is reserved for ZP_UNINIT */
_Static_assert(ZP_TASK_COUNT < ZP_UNINIT,
	       "zperf supports up to 254 tasks: reduce ZPERF_USER/SYSTEM_TASKS_COUNT");

#endif /* ZPERF_CONSTANTS_H */
