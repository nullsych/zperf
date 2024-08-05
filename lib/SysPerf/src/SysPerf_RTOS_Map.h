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
 *                                             SYSPERF HEADERS                                                     *
 ******************************************************************************************************************/
#include "SysPerf_Cfg.cfg"

/*******************************************************************************************************************
*                                             RTOS-SPECIFIC LOCKS                                                  *
*******************************************************************************************************************/
/** @details: Map specific routines to lock/unlock all ISRs **/
// #ifdef ...
#define SP_ISR_LOCK                                 irq_lock
#define SP_ISR_UNLOCK                               irq_unlock


/*******************************************************************************************************************
*                                             RTOS-SPECIFIC TRACES                                                 *
*******************************************************************************************************************/
/** @details: Map specific functions with printf() format to trace SysPerf* statistics **/
#define SP_TRACE_INFO                               LOG_INF
#define SP_TRACE_WARN                               LOG_WRN
#define SP_TRACE_ERRO                               LOG_ERR


/*******************************************************************************************************************
*                                           RTOS-SPECIFIC CONTANTS                                                 *
*******************************************************************************************************************/
/** @details: Map RTOS-specific constants **/

/** @brief Number of user's Tasks (Threads), runnings in system. */
#define MAP_RTOS_USER_TASKS_COUNT                   (3U)

#if (SP_CFG_RTOS_HAS_SYSTEM_TASKS == SP_CFG_ON)
/** @brief In Zephyr RTOS, depending on project configuration (prg.conf) there could be
 *         muitiple system threads. In the current project there are 5 system threads:
 *         "Main", "sysworkq", "MPSL Work", "BT_LW_WQ" and "logging".
 *
 *         To obtain an actual number of running threads you could use an appropriate trace-functionality, since many RTOSes have so.
 *         For example, in Zephyr RTOS it's:
 *
 *              void k_thread_foreach(k_thread_user_cb_t user_cb, void *user_data)
 *
 *         It iterates over all the threads in the system, and calls the user_cb function for each thread,
 *         which could be used once to calculate a number of your running threads, includes both system's and user's (MAP_RTOS_USER_TASKS_COUNT) created threads.
 */
#define RTOS_SYS_TASK_COUNT                         (5U)
#else
#define RTOS_SYS_TASK_COUNT                         (0U)
#endif

/** @brief Total number of Idle Tasks (Threads) in the system. */
#define MAP_RTOS_IDLE_TASK_COUNT                    (1U)

/** @brief Total number of Tasks (Threads) in the system. */
#define MAP_RTOS_OVERALL_TASKS_COUNT                (RTOS_SYS_TASK_COUNT + MAP_RTOS_USER_TASKS_COUNT + MAP_RTOS_IDLE_TASK_COUNT)

/** @brief Total number of ISRs in the system. */
#if (SP_CFG_ISR_RUNTIME_SUPPORTED == SP_CFG_ON)
#if (SP_CFG_ISR_TOTAL_RUNTIME_ONLY == SP_CFG_ON)
    #define MAP_RTOS_ISRS_COUNT                     (1u)
#else
    #define MAP_RTOS_ISRS_COUNT                     ()
#endif /* SP_CFG_ISR_TOTAL_RUNTIME_ONLY */
#endif /* SP_CFG_ISR_RUNTIME_SUPPORTED */

/** @brief Zephyr timer top value. */
#define MAP_RTOS_UPTIME_TOP                         (0xFFFFFFFF)

/** @brief Zephyr length for Task's (Thread's) name. */
#define MAP_RTOS_TASK_MAX_NAME_LEN                  (CONFIG_THREAD_MAX_NAME_LEN)


/*******************************************************************************************************************
*                                             RTOS-SPECIFIC TYPES                                                  *
*******************************************************************************************************************/
/** @details: Map RTOS-speficif standart, Tasks (Threads) & ISRs types **/

typedef uint64_t                                    uint64;
typedef uint32_t                                    uint32;
typedef uint16_t                                    uint16;
typedef uint8_t                                     uint8;
typedef int64_t                                     sint64;
typedef int32_t                                     sint32;
typedef int16_t                                     sint16;
typedef int8_t                                      sint8;

/** @brief Pointer to user/system running Task (Thread) */
#define MAP_RTOS_PTR2TASK                           struct k_thread *

/** @brief Pointer to user/system running Interrupt */
#define MAP_RTOS_PTR2ISR

/** @brief Allocated RTOS's running units priority */
#define MAP_RTOS_PRIO                               sint8

/** @brief Allocated stack space, measured in bytes */
#define MAP_RTOS_STACKSIZE                          uint32

/** @brief Unique identifier for RTOS's running units e.g. Tasks (Threads), ISRs etc. */
#define MAP_RTOS_UNIQ_ID                            uint32

#endif /* SYSPERF_RTOS_MAP_H */