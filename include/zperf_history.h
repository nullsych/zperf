/**
 * @file   zperf_history.h
 * @brief
 * @author nullsych
 * @date   May 6, 2024
 *
 */

#ifndef ZPERF_HISTORY_H
#define ZPERF_HISTORY_H

#include <stdbool.h>

#include "zperf_keywords.h"
#include "zperf_data_types.h"

/*******************************************************************************************************************
 *                                              HISTORY CONSTANTS
 *******************************************************************************************************************/
#ifdef CONFIG_ZPERF_HISTORY_SIZE
#define ZP_HISTORY_SIZE (CONFIG_ZPERF_HISTORY_SIZE)
#else
#define ZP_HISTORY_SIZE (25U)
#endif

#ifdef CONFIG_ZPERF_CPU_USAGE_THRESHOLD
#define ZP_CPU_USAGE_THR (CONFIG_ZPERF_CPU_USAGE_THRESHOLD)
#else
#define ZP_CPU_USAGE_THR (50U)
#endif

/* History statistic */
typedef struct ZP_PACKED {
	zp_hook entities[ZP_HISTORY_SIZE];
	uint32_t crc32;
} zp_hook_history;

/*******************************************************************************************************************
 *                                            HISTORY GLOBAL API
 ********************************************************************************************************************/

ZP_HOOK_CALLED ZP_GLOBAL void history_init(void);
ZP_HOOK_CALLED ZP_GLOBAL void history_update(zp_task_id, uint32_t, uint8_t);
ZP_HOOK_CALLED ZP_GLOBAL void history_clear(void);
ZP_HOOK_CALLED ZP_GLOBAL void history_dump_byte_stream(void);
ZP_USER_CALLED ZP_GLOBAL void history_trace_collected(void);
ZP_HOOK_CALLED ZP_GLOBAL void history_snapshot_take(bool clear);
ZP_USER_CALLED ZP_GLOBAL void history_trace_snapshot(void);

ZP_HOOK_CALLED ZP_GLOBAL uint8_t history_get_last_state(void);
ZP_HOOK_CALLED ZP_GLOBAL zp_task_id history_get_last_task(void);
ZP_HOOK_CALLED ZP_GLOBAL uint32_t history_get_last_timestamp(void);

#endif /* ZPERF_HISTORY_H */
