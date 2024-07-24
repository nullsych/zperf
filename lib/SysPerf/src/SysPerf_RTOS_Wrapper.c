/**
 * @file   SysPerf_RTOS_Wrapper.c
 * @brief  Implementation of wrappers that encapsulate RTOS-specific API.
 * @author kvant6ubl
 * @date   Jul 18, 2024
 *
 */

/*******************************************************************************************************************
 *                                             RTOS-SPECIFIC HEADERS                                                *
 *******************************************************************************************************************/
#include <zephyr/kernel.h>
#include <zephyr/debug/thread_analyzer.h>
#include <zephyr/tracing/tracing.h>
#include <stdint-gcc.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(SysPerf_RTOS_Wrapper, LOG_LEVEL_DBG);

/*******************************************************************************************************************
*                                                SYSPERF HEADERS                                                   *
*******************************************************************************************************************/
#include "SysPerf_RTOS_Map.h"
#include "SysPerf_SetupData.h"

/*******************************************************************************************************************
*                                            WAPPERS API IMPLEMENTATION                                            *
********************************************************************************************************************/

/**
 * @brief Get unique ID (RTOS specific) of the current task.
 */
SP_HOOK_CALLED uint32 SysPerf_GetTaskId(void)
{
   /**
    * @note Recomendation from Zephyr RTOS: Can't use k_current_get() as thread base might be incorrect
    */
   return (uint32)k_sched_current_thread_query();
}

SP_RTOS_CALLED static void SysPerf_LocalCallback(const struct k_thread *thread, void *user_data)
{
   static uint8 taskIterator = 0U;

   SP_TRACE_INFO("thread name  : %s", thread->name);
   SP_TRACE_INFO("thread zid   : 0x%x", (uint32)thread);
   SP_TRACE_INFO("thread prio  : %u", (uint8)thread->base.prio);
   SP_TRACE_INFO("thread stack : %u", (uint32)thread->stack_info.size);
   SP_TRACE_INFO("iterator     : %u\n", taskIterator);

   SysPerf_StartupData_Init(taskIterator, (uint32)thread, thread->name, (uint8)thread->base.prio, (uint32)thread->stack_info.size);
   taskIterator++;
}

/**
 * @brief Saves initialized tasks configuration data after scheduler started it's job.
 */
SP_HOOK_CALLED void SysPerf_SaveTasksCfg(void)
{
   k_thread_foreach(SysPerf_LocalCallback, NULL);
}

/**
 * @brief Get task index (SysPerf specific) based on taskId.
 */
SP_HOOK_CALLED uint8 SysPerf_GetTaskIterator(uint32 taskId)
{
    return SysPerf_StartupData_GetTaskIterator(taskId);
}

SP_HOOK_CALLED uint64 SysPerf_GetUptimeTimestamp(void)
{
   uint64 timeStamp;
   timeStamp = k_uptime_ticks();
   timeStamp = k_ticks_to_us_near64(timeStamp);
   return timeStamp; /* in uS */
}
