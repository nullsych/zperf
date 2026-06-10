/**
 * @file   nrf_perf_history.c
 * @brief
 * @author nullsych
 * @date   Jule 9, 2024
 *
 */

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
SPK_LOCAL sp_HookHistory      HistoryHandler;

/**
 * @brief Index for HistoryHandler's circular buffer.
 */
SPK_LOCAL uint32_t            HistoryIndex  = 0U;

/**
 * @brief Size for HistoryHandler's circular buffer.
 */
SPK_LOCAL uint32_t            HistorySize = 0U;

/*******************************************************************************************************************
*                                        HISTORY GLOBAL API IMPLEMENTATION                                         *
*******************************************************************************************************************/

HOOK_CALLED uint8_t nrf_perf_history_get_last_state(void)
{
   return HistoryHandler.entities[HistoryIndex].state;
}

HOOK_CALLED uint32_t nrf_perf_history_get_last_timestamp(void)
{
   return HistoryHandler.entities[HistoryIndex].timestamp;
}

HOOK_CALLED uint8_t nrf_perf_history_get_last_task(void)
{
   return HistoryHandler.entities[HistoryIndex].identifier;
}

HOOK_CALLED void nrf_perf_history_init(void)
{
   HistoryIndex = 0U;
   HistorySize  = 0U;
}

HOOK_CALLED void nrf_perf_history_dump_byte_stream(void)
{
   // TODO: add crc32 calculation
   // TODO: dump byte-stream from history in UART
}

HOOK_CALLED void nrf_perf_history_clear(void)
{
   for (uint32_t idx = 0; idx < HistorySize; idx++)
   {
      HistoryHandler.entities[idx].identifier = 0;
      HistoryHandler.entities[idx].state = 0;
      HistoryHandler.entities[idx].timestamp = 0;
   }
}

HOOK_CALLED void nrf_perf_history_update(uint32_t identifier, uint32_t timestamp, uint8_t state)
{
   sp_Hook *currentEntity = &HistoryHandler.entities[HistoryIndex];

   if (currentEntity != NULL)
   {
      currentEntity->state = state;
      currentEntity->identifier = identifier;
      currentEntity->timestamp = timestamp;

      HistoryIndex++;
      HistoryIndex %= SP_HISTORY_SIZE;
      HistorySize = (HistorySize < SP_HISTORY_SIZE) ? (HistorySize + 1) : SP_HISTORY_SIZE;
   }
}

USER_CALLED void nrf_perf_history_trace_collected(void)
{
   SP_TRACE_INFO("# ############### Collected History ############### #");
   SP_TRACE_INFO("#                                                   #");
   SP_TRACE_INFO("#    Task Id             Mask         Timestamp    #");

   for (uint32_t idx = 0; idx < HistorySize; idx++)
   {
      SP_TRACE_INFO("%s %8u %17u %18u %5s",
                    "#", HistoryHandler.entities[idx].identifier, HistoryHandler.entities[idx].state,
                    HistoryHandler.entities[idx].timestamp, "#");
   }
   SP_TRACE_INFO("# ################################################# #\n");
}
