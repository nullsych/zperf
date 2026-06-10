/**
 * @file   RTOS_Map.h
 * @brief  Map RTOS-specific functionality: logs, constants, etc.
 * @author nullsych
 * @date   May 17, 2024
 *
 */

#ifndef NRF_PERF_RTOS_MAP_H
#define NRF_PERF_RTOS_MAP_H

/*******************************************************************************************************************
*                                             RTOS-SPECIFIC HEADERS                                                *
*******************************************************************************************************************/
/** @details: Include necessary headers **/
#include <stdint.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

/*******************************************************************************************************************
 *                                             nrf_perf HEADERS                                                     *
 ******************************************************************************************************************/
#include "nrf_perf_cfg.cfg"

/*******************************************************************************************************************
*                                             RTOS-SPECIFIC LOCKS                                                  *
*******************************************************************************************************************/
/** @details: Map specific routines to lock/unlock all ISRs, since it needed for critical sections **/
#define MAP_RTOS_ISR_LOCK                           irq_lock
#define MAP_RTOS_ISR_UNLOCK                         irq_unlock


/*******************************************************************************************************************
*                                             RTOS-SPECIFIC TRACES                                                 *
*******************************************************************************************************************/
/** @details: Map specific functions with printf()-compatible format to trace nrf_perf* statistics **/
#define MAP_RTOS_TRACE_INFO                         LOG_INF
#define MAP_RTOS_TRACE_WARN                         LOG_WRN
#define MAP_RTOS_TRACE_ERRO                         LOG_ERR


/*******************************************************************************************************************
*                                           RTOS-SPECIFIC CONSTANTS                                                *
*******************************************************************************************************************/
/** @details: Map RTOS-specific constants **/

/** @brief Number of user's Tasks (Threads), runnings in system. */
#ifdef CONFIG_NRF_PERF_USER_TASKS_COUNT
#define MAP_RTOS_USER_TASKS_COUNT                   (CONFIG_NRF_PERF_USER_TASKS_COUNT)
#else
#define MAP_RTOS_USER_TASKS_COUNT                   (3U)
#endif

#if (NP_CFG_RTOS_HAS_SYSTEM_TASKS == NP_CFG_ON)
/** @brief Zephyr RTOS, like many other RTOSs, has an implicit task creation mechanism.
 *         That means, unfortunately you are not be able to determine all threads
 *         (e.g. system threads) at compile time, because it's created inside RTOS sources.
 *         And to do so, you need to make a modifications in RTOS.
 *
 *         Depending on project configuration (prg.conf) there could be mutiple system
 *         threads regarding BLE, logging etc. With my current configuration there
 *         are 5 system threads:
 *
 *              "Main", "sysworkq", "MPSL Work", "BT_LW_WQ" and "logging".
 *
 *         To obtain an actual number of running threads you could use an appropriate
 *         trace-functionality, since many RTOSes have so. For example, in Zephyr RTOS it's:
 *
 *              void k_thread_foreach(k_thread_user_cb_t user_cb, void *user_data)
 *
 *         Since we use this function in backend (nrf_perf_RTOS_Wrapper) at the start of the application,
 *         you can get the actual number of running tasks and it's data, and then change RTOS_SYS_TASK_COUNT.
 *         It iterates over all the threads in the system, and calls the user_cb function for each thread,
 *         which could be used once to calculate a number of your running threads, includes both system's and user's (MAP_RTOS_USER_TASKS_COUNT) created threads.
 */
#ifdef CONFIG_NRF_PERF_SYSTEM_TASKS_COUNT
#define RTOS_SYS_TASK_COUNT                         (CONFIG_NRF_PERF_SYSTEM_TASKS_COUNT)
#else
#define RTOS_SYS_TASK_COUNT                         (5U)
#endif
#else
#define RTOS_SYS_TASK_COUNT                         (0U)
#endif

/** @brief Total number of Idle Tasks (Threads) in the system. Zephyr RTOS has only one idle task */
#define MAP_RTOS_IDLE_TASK_COUNT                    (1U)

/** @brief Total number of Tasks (Threads) in the system. */
#define MAP_RTOS_OVERALL_TASKS_COUNT                (RTOS_SYS_TASK_COUNT + MAP_RTOS_USER_TASKS_COUNT + MAP_RTOS_IDLE_TASK_COUNT)

/** @brief Zephyr timer top value. */
#define MAP_RTOS_UPTIME_TOP                         (UINT32_MAX)

/** @brief Zephyr length for Task's (Thread's) name. */
#define MAP_RTOS_TASK_MAX_NAME_LEN                  (CONFIG_THREAD_MAX_NAME_LEN)


/*******************************************************************************************************************
*                                             RTOS-SPECIFIC TYPES                                                  *
*******************************************************************************************************************/

/** @brief Pointer to user/system running Task (Thread) */
#define MAP_RTOS_PTR2TASK                           const struct k_thread *

/** @brief Pointer to user/system running Interrupt */
#define MAP_RTOS_PTR2ISR

/** @brief Allocated RTOS's running units priority */
#define MAP_RTOS_PRIO                               int8_t

/** @brief Allocated stack space, measured in bytes */
#define MAP_RTOS_STACKSIZE                          uint32_t

/** @brief Unique identifier for RTOS's running units e.g. Tasks (Threads), ISRs etc. */
#define MAP_RTOS_UNIQ_ID                            uint32_t

#endif /* NRF_PERF_RTOS_MAP_H */
