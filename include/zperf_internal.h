/**
 * @file   zperf_internal.h
 * @brief  Internal macros and RTOS hook declarations (not for user code).
 * @author nullsych
 * @date   May 6, 2024
 *
 */

#ifndef ZPERF_INTERNAL_H
#define ZPERF_INTERNAL_H

#include "zperf.h"
#include "rtos_map.h"
#include "zperf_data_types.h"
#include "zperf_constants.h"

/*******************************************************************************************************************
 *                                                   MACROS
 *******************************************************************************************************************/
#define ZP_ISR_LOCK   MAP_RTOS_ISR_LOCK
#define ZP_ISR_UNLOCK MAP_RTOS_ISR_UNLOCK

#define ZP_TRACE_INFO MAP_RTOS_TRACE_INFO
#define ZP_TRACE_WARN MAP_RTOS_TRACE_WARN
#define ZP_TRACE_ERRO MAP_RTOS_TRACE_ERRO

#define ZP_ISR_ID_SIGN (0xFFFC) /* Common ISR signature */

#define TASK_IS_VALID(task_iterator) ((task_iterator < ZP_TASK_COUNT) && (task_iterator >= 0))
/* 64-bit math: total_rt * 100 overflows uint32_t for windows longer than ~43 s */
#define GET_CPU_PERCENT(time_slice, total_rt)                                                      \
	(((time_slice) == 0U) ? 0U                                                                 \
			      : (uint32_t)(((uint64_t)(total_rt) * 100U) / (uint64_t)(time_slice)))

/*******************************************************************************************************************
 *                                            RTOS HOOKS
 *******************************************************************************************************************/
ZP_RTOS_CALLED void pre_task_hook(void);
ZP_RTOS_CALLED void post_task_hook(void);
ZP_RTOS_CALLED void pre_isr_hook(void);
ZP_RTOS_CALLED void post_isr_hook(void);
ZP_RTOS_CALLED void idle_task_running(void);

/* Called by the RTOS wrapper while enumerating threads, with IRQs locked */
ZP_RTOS_CALLED uint8_t zperf_task_register(zp_task_id task_id);

#endif /* ZPERF_INTERNAL_H */
