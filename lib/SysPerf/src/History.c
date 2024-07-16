/**
 * @file   History.c
 * @brief
 * @author kvant6ubl
 * @date   Jule 9, 2024
 *
 */

/*******************************************************************************************************************
 *                                             RTOS-SPECIFIC HEADERS                                                *
 *******************************************************************************************************************/
#include <stdlib.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(SysPerf_History, LOG_LEVEL_DBG);

/*******************************************************************************************************************
 *                                                SYSPERF HEADERS                                                  *
 *******************************************************************************************************************/
#include "SysPerf.h"
#include "SysPerf_DataTypes.h"
#include "History.h"

/*******************************************************************************************************************
 *                                              STATISTIC ENTITIES                                                 *
 *******************************************************************************************************************/

SP_LOCAL sp_HookHistory Hook_History;


/*******************************************************************************************************************
*                                        HISTORY GLOBAL API IMPLEMENTATION                                         *
*******************************************************************************************************************/

SP_HOOK_CALLED uint8 History_GetLastEntityState(uint8 taskIterator)
{
    return Hook_History.history.entities[taskIterator].state;
}

SP_HOOK_CALLED uint8 History_GetLastState(void)
{
   return Hook_History.history.entities[Hook_History.history.idx].state;
}

SP_HOOK_CALLED uint32 History_GetLastTimestamp(void)
{
   return Hook_History.history.entities[Hook_History.history.idx].timestamp;
}

SP_HOOK_CALLED uint8 History_GetLastTask(void)
{
   return Hook_History.history.entities[Hook_History.history.idx].taskIterator;
}

SP_HOOK_CALLED void History_Init(void)
{
   Hook_History.history.idx = 0U;
   Hook_History.history.size = 0U;
}

SP_HOOK_CALLED void History_Update(uint8 taskIterator, uint64 timestamp, uint8 state, uint8 lastOsEvent)
{
   sp_Hook *currentEntity = &Hook_History.history.entities[Hook_History.history.idx];

   if (currentEntity != NULL)
   {
      currentEntity->state = state;
      currentEntity->taskIterator = taskIterator;
      currentEntity->timestamp = timestamp;

      Hook_History.lastOsEvent = lastOsEvent;
      Hook_History.history.idx++;
      Hook_History.history.idx %= SP_HISTORY_SIZE;
      Hook_History.history.size = (Hook_History.history.size < SP_HISTORY_SIZE) ? (Hook_History.history.size + 1) : SP_HISTORY_SIZE;
   }
}

SP_USER_CALLED void History_TraceCollected(void)
{
   SP_TRACE_INFO("# ############### Collected History ############### #");
   SP_TRACE_INFO("#                                                   #");
   SP_TRACE_INFO("#    Task Id             State         Timestamp    #");

   for (uint32 idx = 0; idx < Hook_History.history.size; idx++)
   {
      SP_TRACE_INFO("%s %8u %17u %18u %5s",
                    "#", Hook_History.history.entities[idx].taskIterator, Hook_History.history.entities[idx].state,
                    Hook_History.history.entities[idx].timestamp, "#");
   }
   SP_TRACE_INFO("# ################################################# #\n");
}