/**
 * @file   nrf_perf_history.c
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
LOG_MODULE_REGISTER(nrf_perf_history, LOG_LEVEL_DBG);

/*******************************************************************************************************************
 *                                                nrf_perf HEADERS                                                  *
 ******************************************************************************************************************/
#include "nrf_perf.h"
#include "nrf_perf_history.h"

/*******************************************************************************************************************
 *                                              STATISTIC ENTITIES                                                 *
 ******************************************************************************************************************/

/**
 * @brief Main history uptime statistic.
 * @details Store a system 'footprint' for last microseconds.
 * Could be used for runtime analysis in cases of thread starvation or sporadically resets.
 */
NP_LOCAL np_hook_history     history_handler;

/**
 * @brief Index for history_handler's circular buffer.
 */
NP_LOCAL uint32_t            history_index  = 0U;

/**
 * @brief Size for history_handler's circular buffer.
 */
NP_LOCAL uint32_t            history_size = 0U;

/*******************************************************************************************************************
*                                        HISTORY GLOBAL API IMPLEMENTATION                                         *
*******************************************************************************************************************/
/* clang-format on */

NP_HOOK_CALLED uint8_t nrf_perf_history_get_last_state(void)
{
	return history_handler.entities[history_index].state;
}

NP_HOOK_CALLED uint32_t nrf_perf_history_get_last_timestamp(void)
{
	return history_handler.entities[history_index].timestamp;
}

NP_HOOK_CALLED uint8_t nrf_perf_history_get_last_task(void)
{
	return history_handler.entities[history_index].identifier;
}

NP_HOOK_CALLED void nrf_perf_history_init(void)
{
	history_index = 0U;
	history_size = 0U;
}

NP_HOOK_CALLED void nrf_perf_history_dump_byte_stream(void)
{
	// TODO: add crc32 calculation
	// TODO: dump byte-stream from history
}

NP_HOOK_CALLED void nrf_perf_history_clear(void)
{
	for (uint32_t idx = 0; idx < history_size; idx++) {
		history_handler.entities[idx].identifier = 0;
		history_handler.entities[idx].state = 0;
		history_handler.entities[idx].timestamp = 0;
	}
}

NP_HOOK_CALLED void nrf_perf_history_update(uint32_t identifier, uint32_t timestamp, uint8_t state)
{
	np_hook *currentEntity = &history_handler.entities[history_index];

	if (currentEntity != NULL) {
		currentEntity->state = state;
		currentEntity->identifier = identifier;
		currentEntity->timestamp = timestamp;

		history_index++;
		history_index %= NP_HISTORY_SIZE;
		history_size =
			(history_size < NP_HISTORY_SIZE) ? (history_size + 1) : NP_HISTORY_SIZE;
	}
}

NP_USER_CALLED void nrf_perf_history_trace_collected(void)
{
	NP_TRACE_INFO("# ############### Collected History ############### #");
	NP_TRACE_INFO("#                                                   #");
	NP_TRACE_INFO("#    Task Id             Mask         Timestamp    #");

	for (uint32_t idx = 0; idx < history_size; idx++) {
		NP_TRACE_INFO("%s %8u %17u %18u %5s", "#", history_handler.entities[idx].identifier,
			      history_handler.entities[idx].state,
			      history_handler.entities[idx].timestamp, "#");
	}
	NP_TRACE_INFO("# ################################################# #\n");
}
