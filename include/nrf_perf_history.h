/**
 * @file   nrf_perf_History.h
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
*                                              HISTORY CONSTANTS                                                   *
*******************************************************************************************************************/
#ifdef CONFIG_NRF_PERF_HISTORY_SIZE
#define SP_HISTORY_SIZE                     (CONFIG_NRF_PERF_HISTORY_SIZE)
#else
#define SP_HISTORY_SIZE                     (8U)
#endif

#ifdef CONFIG_NRF_PERF_CPU_USAGE_THRESHOLD
#define SP_CPU_USAGE_THR                    (CONFIG_NRF_PERF_CPU_USAGE_THRESHOLD)
#else
#define SP_CPU_USAGE_THR                    (50U)
#endif

/* History statistic */
typedef struct SPK_PACKED
{
    sp_Hook entities[SP_HISTORY_SIZE];
    uint32_t  crc32;
} sp_HookHistory;

/*******************************************************************************************************************
*                                            HISTORY GLOBAL API                                                    *
********************************************************************************************************************/

HOOK_CALLED SPK_GLOBAL void         nrf_perf_history_init(void);
HOOK_CALLED SPK_GLOBAL void         nrf_perf_history_update(uint32_t, uint32_t, uint8_t);
HOOK_CALLED SPK_GLOBAL void         nrf_perf_history_clear(void);
HOOK_CALLED SPK_GLOBAL void         nrf_perf_history_dump_byte_stream(void);
USER_CALLED SPK_GLOBAL void         nrf_perf_history_trace_collected(void);

HOOK_CALLED SPK_GLOBAL uint8_t      nrf_perf_history_get_last_state(void);
HOOK_CALLED SPK_GLOBAL uint8_t      nrf_perf_history_get_last_task(void);
HOOK_CALLED SPK_GLOBAL uint32_t     nrf_perf_history_get_last_timestamp(void);

#endif /* NRF_PERF_HISTORY_H */
