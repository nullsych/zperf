/**
 * @file   nrf_perf.h
 * @brief  Contain main implementation of nrf_perf functionality.
 * @author nullsych
 * @date   May 6, 2024
 *
 */

#ifndef NRF_PERF_H
#define NRF_PERF_H

#include "rtos_map.h"
#include "nrf_perf_data_types.h"
#include "nrf_perf_constants.h"
#include "nrf_perf_keywords.h"

/*******************************************************************************************************************
*                                                   MACROS                                                         *
*******************************************************************************************************************/
#define SP_ISR_LOCK                         MAP_RTOS_ISR_LOCK
#define SP_ISR_UNLOCK                       MAP_RTOS_ISR_UNLOCK

#define SP_TRACE_INFO                       MAP_RTOS_TRACE_INFO
#define SP_TRACE_WARN                       MAP_RTOS_TRACE_WARN
#define SP_TRACE_ERRO                       MAP_RTOS_TRACE_ERRO

#define SP_ISR_ID_SIGN                      (0xFFFC)                                      /* Common ISR signature */

#define TASK_IS_VALID(taskIterator)         ((taskIterator < SP_TASK_COUNT) && (taskIterator >= 0))
#define GET_CPU_PERCENT(timeslice, totalRT) ((totalRT * 100U) / timeslice)


/*******************************************************************************************************************
*                                            nrf_perf GLOBAL API                                                    *
*******************************************************************************************************************/
/* nrf_perf Hooks Declarations for RTOS-only usage */
RTOS_CALLED void nrf_perf_pre_task_hook(void);
RTOS_CALLED void nrf_perf_post_task_hook(void);
RTOS_CALLED void nrf_perf_pre_isr_hook(void);
RTOS_CALLED void nrf_perf_post_isr_hook(void);
RTOS_CALLED void nrf_perf_idle_task_running(void);

/* nrf_perf API for user's code */
USER_CALLED extern void nrf_perf_trace_history(void);
USER_CALLED extern void nrf_perf_runtime_overall_statistic(void);
USER_CALLED extern void nrf_perf_pre_scheduler_init(void);
USER_CALLED extern void nrf_perf_post_scheduler_init(void);

#endif /* NRF_PERF_H */