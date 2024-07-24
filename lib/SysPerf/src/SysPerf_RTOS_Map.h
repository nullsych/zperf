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
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

/*******************************************************************************************************************
 *                                             SYSPERF HEADERS                                                      *
 *******************************************************************************************************************/
#include "SysPerf_Cfg.cfg"

/*******************************************************************************************************************
*                                             RTOS-SPECIFIC LOCKS                                                 *
*******************************************************************************************************************/
/** @details: Map specific routines to lock/unlock all ISRs **/
// #ifdef ...
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

/** @brief Number of user's tasks, runnings in system */
#define MAP_RTOS_USER_TASKS_COUNT           (3U)

#if (RTOS_HAS_SYSTEM_TASKS == SP_F_ON)
/** @brief Main, sysworkq, MPSL Work, BT_LW_WQ and logging task */
#define RTOS_SYS_TASK_COUNT                 (5U)
#else
#define RTOS_SYS_TASK_COUNT                 (0U)
#endif

/** @brief Total number of Idle Tasks in the system */
#define MAP_RTOS_IDLE_TASK_COUNT            (1U)

/** @brief Total number of Tasks in the system */
#define MAP_RTOS_OVERALL_TASKS_COUNT        (RTOS_SYS_TASK_COUNT + MAP_RTOS_USER_TASKS_COUNT + MAP_RTOS_IDLE_TASK_COUNT)

/** @brief Total number of ISRs in the system */
#define MAP_RTOS_ISRS_COUNT                 (0u) // TODO: add support to calculate ISR runtime

/** @brief Zephyr timer top value */
#define MAP_RTOS_UPTIME_TOP                 (0xFFFFFFFF)

/** @brief Zephyr length for Task's name */
#define MAP_RTOS_TASK_MAX_NAME_LEN          CONFIG_THREAD_MAX_NAME_LEN


/*******************************************************************************************************************
*                                             RTOS-SPECIFIC TYPES                                                  *
*******************************************************************************************************************/
/** @details: Map RTOS-speficif standart, Tasks & ISRs types **/
#define MAP_RTOS_PTR2TASK                   struct k_thread *
#define MAP_RTOS_PTR2ISR
#define MAP_RTOS_PRIO                       int8_t
#define MAP_RTOS_STACKSIZE                  uint32_t
#define MAP_RTOS_UNIQ_ID                    uint32_t

typedef uint64_t                            uint64;
typedef uint32_t                            uint32;
typedef uint16_t                            uint16;
typedef uint8_t                             uint8;
typedef int64_t                             sint64;
typedef int32_t                             sint32;
typedef int16_t                             sint16;
typedef int8_t                              sint8;

#endif /* SYSPERF_RTOS_MAP_H */