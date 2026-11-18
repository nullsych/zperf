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

/**
 * @brief Index of the most recently written entry.
 * @details history_index points to the next slot to write, so the last entry is one step back.
 */
ZP_LOCAL_INLINE uint32_t history_last_index(void)
{
	return (history_index + ZP_HISTORY_SIZE - 1U) % ZP_HISTORY_SIZE;
}

ZP_HOOK_CALLED uint8_t history_get_last_state(void)
{
	if (history_size == 0U) {
		return HISTORY_STATE_NONE;
	}

	return history_handler.entities[history_last_index()].state;
}

ZP_HOOK_CALLED uint32_t history_get_last_timestamp(void)
{
	if (history_size == 0U) {
		return 0U;
	}

	return history_handler.entities[history_last_index()].timestamp;
}

ZP_HOOK_CALLED uint32_t history_get_last_task(void)
{
	if (history_size == 0U) {
		return 0U;
	}

	return history_handler.entities[history_last_index()].task_id;
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
	/* Called from a user thread while hooks may write the buffer, so reset it atomically */
	unsigned int key = ZP_ISR_LOCK();

	history_index = 0U;
	history_size = 0U;

	ZP_ISR_UNLOCK(key);
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
	/* Once the buffer has wrapped, the oldest entry sits at history_index */
	uint32_t start = (history_size < ZP_HISTORY_SIZE) ? 0U : history_index;

	ZP_TRACE_INFO("# ############ Collected History ############ #");
	ZP_TRACE_INFO("#                                             #");
	ZP_TRACE_INFO("# %-20s  %-7s  %11s  #", "Task Name", "State", "Timestamp");

	for (uint32_t i = 0; i < history_size; i++) {
		const zp_hook *entry = &history_handler.entities[(start + i) % ZP_HISTORY_SIZE];

		/* Names longer than the column are truncated to keep the table aligned */
		ZP_TRACE_INFO("# %-20.20s  %-7s  %11u  #", task_name_safe(entry->task_id),
			      state_to_str(entry->state), entry->timestamp);
	}
	ZP_TRACE_INFO("# ########################################### #\n");
}
