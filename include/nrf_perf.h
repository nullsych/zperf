/**
 * @file   nrf_perf.h
 * @brief  Contain main declarations of nrf_perf.
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
#define NP_ISR_LOCK                         MAP_RTOS_ISR_LOCK
#define NP_ISR_UNLOCK                       MAP_RTOS_ISR_UNLOCK

#define NP_TRACE_INFO                       MAP_RTOS_TRACE_INFO
#define NP_TRACE_WARN                       MAP_RTOS_TRACE_WARN
#define NP_TRACE_ERRO                       MAP_RTOS_TRACE_ERRO

#define NP_ISR_ID_SIGN                      (0xFFFC)                                      /* Common ISR signature */

#define TASK_IS_VALID(task_iterator)         ((task_iterator < NP_TASK_COUNT) && (task_iterator >= 0))
#define GET_CPU_PERCENT(time_slice, total_rt) ((total_rt * 100U) / time_slice)


/*******************************************************************************************************************
*                                            nrf_perf GLOBAL API                                                    *
*******************************************************************************************************************/
/* Hooks Declarations for RTOS-only usage */
NP_RTOS_CALLED void nrf_perf_pre_task_hook(void);
NP_RTOS_CALLED void nrf_perf_post_task_hook(void);
NP_RTOS_CALLED void nrf_perf_pre_isr_hook(void);
NP_RTOS_CALLED void nrf_perf_post_isr_hook(void);
NP_RTOS_CALLED void nrf_perf_idle_task_running(void);

/* API for user's code */
NP_USER_CALLED extern void nrf_perf_trace_history(void);
NP_USER_CALLED extern void nrf_perf_runtime_overall_statistic(void);
NP_USER_CALLED extern void nrf_perf_pre_scheduler_init(void);
NP_USER_CALLED extern void nrf_perf_post_scheduler_init(void);

#endif /* NRF_PERF_H */