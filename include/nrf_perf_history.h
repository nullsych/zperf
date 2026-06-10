/**
 * @file   nrf_perf_history.h
 * @brief
 * @author nullsych
 * @date   May 6, 2024
 *
 */

#ifndef NRF_PERF_HISTORY_H
#define NRF_PERF_HISTORY_H

#include "nrf_perf_keywords.h"
#include "nrf_perf_data_types.h"

/*******************************************************************************************************************
 *                                              HISTORY CONSTANTS                 *
 *******************************************************************************************************************/
#ifdef CONFIG_NRF_PERF_HISTORY_SIZE
#define NP_HISTORY_SIZE (CONFIG_NRF_PERF_HISTORY_SIZE)
#else
#define NP_HISTORY_SIZE (8U)
#endif

#ifdef CONFIG_NRF_PERF_CPU_USAGE_THRESHOLD
#define NP_CPU_USAGE_THR (CONFIG_NRF_PERF_CPU_USAGE_THRESHOLD)
#else
#define NP_CPU_USAGE_THR (50U)
#endif

/* History statistic */
typedef struct NP_PACKED {
	np_hook entities[NP_HISTORY_SIZE];
	uint32_t crc32;
} np_hook_history;

/*******************************************************************************************************************
 *                                            HISTORY GLOBAL API                 *
 ********************************************************************************************************************/

NP_HOOK_CALLED NP_GLOBAL void nrf_perf_history_init(void);
NP_HOOK_CALLED NP_GLOBAL void nrf_perf_history_update(uint32_t, uint32_t, uint8_t);
NP_HOOK_CALLED NP_GLOBAL void nrf_perf_history_clear(void);
NP_HOOK_CALLED NP_GLOBAL void nrf_perf_history_dump_byte_stream(void);
NP_USER_CALLED NP_GLOBAL void nrf_perf_history_trace_collected(void);

NP_HOOK_CALLED NP_GLOBAL uint8_t nrf_perf_history_get_last_state(void);
NP_HOOK_CALLED NP_GLOBAL uint8_t nrf_perf_history_get_last_task(void);
NP_HOOK_CALLED NP_GLOBAL uint32_t nrf_perf_history_get_last_timestamp(void);

#endif /* NRF_PERF_HISTORY_H */
