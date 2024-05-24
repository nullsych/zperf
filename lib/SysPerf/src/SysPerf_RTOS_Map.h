/**
 * @file   SysPerf_RTOS_Map.h
 * @brief  Map RTOS-specific functionality: logs, constants, etc.
 * @author kvant6ubl
 * @date   May 17, 2024
 *
 */

#ifndef SYSPERF_RTOS_MAP_H
#define SYSPERF_RTOS_MAP_H

/*******************************************************************************************************************
*                                             RTOS-SPECIFIC HEADERS                                                *
*******************************************************************************************************************/
/** @details: Include necessary headers **/
#include <zephyr.h>
#include <logging/log.h>


/*******************************************************************************************************************
*                                             RTOS-SPECIFIC LOCKS                                                 *
*******************************************************************************************************************/
/** @details: Map specific routines to lock/unlock all ISRs **/
// #ifdef ...
// #define SP_CRITICAL_SECTION_BEGIN(key)           (key = irq_lock())
// #define SP_CRITICAL_SECTION_END(key)             (irq_unlock(key))
#define SP_ISR_LOCK                         irq_lock
#define SP_ISR_UNLOCK                       irq_unlock


/*******************************************************************************************************************
*                                             RTOS-SPECIFIC TRACES                                                 *
*******************************************************************************************************************/
/** @details: Map specific functions with printf() format to trace SysPerf* statistics **/
#define SP_TRACE_INFO                       LOG_INF
#define SP_TRACE_WARN                       LOG_WRN
#define SP_TRACE_ERRO                       LOG_ERR


/*******************************************************************************************************************
*                                           RTOS-SPECIFIC CONTANTS                                                 *
*******************************************************************************************************************/
/** @details: Map RTOS-specific constants **/
#define MAP_RTOS_TASKS_COUNT                (3u)                        /* Number of tasks, runnings in system    */
#define MAP_RTOS_ISRS_COUNT                 (0u)                        /* Number of ISRs, runnings in system     */
#define MAP_RTOS_UPTIME_TOP                 (0xFFFFFFul)                /* Zephyr timer top value                 */
#define MAP_RTOS_TASK_MAX_NAME_LEN          CONFIG_THREAD_MAX_NAME_LEN  /* Zephyr tasks max len of name           */


/*******************************************************************************************************************
*                                             RTOS-SPECIFIC TYPES                                                  *
*******************************************************************************************************************/
/** @details: Map RTOS-speficif standart, Tasks & ISRs types **/
#define MAP_RTOS_PTR2TASK                   struct k_thread *
#define MAP_RTOS_PTR2ISR
#define MAP_RTOS_PRIO                       int
#define MAP_RTOS_STACKSIZE                  int

typedef uint64_t                            uint64;
typedef uint32_t                            uint32;
typedef uint16_t                            uint16;
typedef uint8_t                             uint8;
typedef int64_t                             sint64;
typedef int32_t                             sint32;
typedef int16_t                             sint16;
typedef int8_t                              sint8;

#endif /* SYSPERF_RTOS_MAP_H */