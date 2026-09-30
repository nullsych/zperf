/**
 * @file   zperf_history.c
 * @brief
 * @author nullsych
 * @date   Jule 9, 2024
 *
 */
/* clang-format off */
/*******************************************************************************************************************
 *                                             RTOS-SPECIFIC HEADERS                                               *
 ******************************************************************************************************************/
#include <stdlib.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(zperf_history, LOG_LEVEL_DBG);

/*******************************************************************************************************************
 *                                                zperf HEADERS                                                  *
 ******************************************************************************************************************/
#include "zperf_internal.h"
#include "zperf_history.h"
#include "wrapper.h"

/*******************************************************************************************************************
 *                                              STATISTIC ENTITIES                                                 *
 ******************************************************************************************************************/

/**
 * @brief Main history uptime statistic.
 * @details Store a system 'footprint' for last microseconds.
 * Could be used for runtime analysis in cases of thread starvation or sporadically resets.
 */
ZP_LOCAL zp_hook_history     history_handler;

/**
 * @brief Index for history_handler's circular buffer.
 */
ZP_LOCAL uint32_t            history_index  = 0U;

/**
 * @brief Size for history_handler's circular buffer.
 */
ZP_LOCAL uint32_t            history_size = 0U;

/*******************************************************************************************************************
*                                        HISTORY GLOBAL API IMPLEMENTATION                                         *
*******************************************************************************************************************/
/* clang-format on */

ZP_HOOK_CALLED uint8_t history_get_last_state(void)
{
	return history_handler.entities[history_index].state;
}

ZP_HOOK_CALLED uint32_t history_get_last_timestamp(void)
{
	return history_handler.entities[history_index].timestamp;
}

ZP_HOOK_CALLED uint32_t history_get_last_task(void)
{
	return history_handler.entities[history_index].task_id;
}

ZP_HOOK_CALLED void history_init(void)
{
	history_index = 0U;
	history_size = 0U;
}

ZP_HOOK_CALLED void history_dump_byte_stream(void)
{
	// TODO: add crc32 calculation
	// TODO: dump byte-stream from history
}

ZP_HOOK_CALLED void history_clear(void)
{
	for (uint32_t idx = 0; idx < history_size; idx++) {
		history_handler.entities[idx].task_id = 0;
		history_handler.entities[idx].state = 0;
		history_handler.entities[idx].timestamp = 0;
	}
}

ZP_HOOK_CALLED void history_update(uint32_t identifier, uint32_t timestamp, uint8_t state)
{
	zp_hook *slot = &history_handler.entities[history_index];

	if (slot != NULL) {
		slot->state = state;
		slot->task_id = identifier;
		slot->timestamp = timestamp;

		history_index++;
		history_index %= ZP_HISTORY_SIZE;
		history_size =
			(history_size < ZP_HISTORY_SIZE) ? (history_size + 1) : ZP_HISTORY_SIZE;
	}
}

static const char *state_to_str(uint8_t state)
{
	switch (state) {
	case TASK_STATE_ENTER:
		return "IN"; // task switch in
	case TASK_STATE_EXIT:
		return "OUT"; // task switch out
	case ISR_STATE_ENTER:
		return "ISR_IN"; // isr switch in
	case ISR_STATE_EXIT:
		return "ISR_OUT"; // isr switch out
	case IDLE_STATE_RUNNING:
		return "IDLE";
	default:
		return "UNKNOWN";
	}
}

static const char *task_name_safe(uint32_t task_id)
{
	if (task_id == 0) {
		return "ISR";
	}
	const char *name = wr_get_task_name(task_id);
	return (name != NULL) ? name : "?";
}

ZP_USER_CALLED void history_trace_collected(void)
{
	ZP_TRACE_INFO("# ############### Collected History ############### #");
	ZP_TRACE_INFO("#                                                   #");
	ZP_TRACE_INFO("# Task Name           State    Timestamp            #");

	for (uint32_t idx = 0; idx < history_size; idx++) {
		ZP_TRACE_INFO("# %-12s   %10s   %10u            #",
			      task_name_safe(history_handler.entities[idx].task_id),
			      state_to_str(history_handler.entities[idx].state),
			      history_handler.entities[idx].timestamp);
	}
	ZP_TRACE_INFO("# ################################################# #\n");
}
