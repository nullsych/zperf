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
#include "SysPerf_DataTypes.h"
#include "SysPerf_Keywords.h"
#include "SysPerf.h"

/*******************************************************************************************************************
*                                            WRAPPER API IMPLEMENTATION                                            *
********************************************************************************************************************/

/**
 * @brief Get unique taskId (zephyr thread ID) of the current Task (Thread).
 */
RTOS_WRAPPER uint32_t SysPerf_GetTaskId(void)
{
   /**
    * @note Recommendation from Zephyr RTOS: Can't use k_current_get() as thread base might be incorrect.
    */
   return (uint32_t)k_sched_current_thread_query();
}

/**
 * @brief Get task iterator (SysPerf specific) based on taskId (zephyr thread ID).
 */
RTOS_WRAPPER uint8_t SysPerf_GetTaskIterator(uint32_t taskId)
{
   k_tid_t id = (k_tid_t)(taskId);
   return (uint8_t)(uintptr_t)id->custom_data;
}

/**
 * @brief Get task name based on taskId (zephyr thread ID).
 */
RTOS_WRAPPER const char* SysPerf_GetTaskName(uint32_t taskId)
{
   return k_thread_name_get((k_tid_t)taskId);
}

/**
 * @brief Get unique ID of the current ISR.
 */
RTOS_WRAPPER uint32_t SysPerf_GetIsrId(void)
{
   return (uint32_t)__get_IPSR();
}

SPK_LOCAL RTOS_WRAPPER void SysPerf_EnumerateCallback(sp_TaskPtr thread, void *user_data)
{
   static uint8_t taskIterator = 0;

   /* Here k_thread_custom_data_set() is not used cause id doesn't allowed to write it during k_thread_foreach */
   // k_thread_custom_data_set((void *)(uintptr_t)taskIterator);

   /* Instead of this we use also not the best option by casting away a const qualification on thread object of pointer type. */
   k_tid_t mod_thread = (k_tid_t)(thread);

   /* And assign a taskIterator to it*/
   mod_thread->custom_data = (void *)(uintptr_t)taskIterator;

   /* Print config data */
   SP_TRACE_INFO("Iterating %d thread in the system. Thread data: ", (uint8_t)(uintptr_t)mod_thread->custom_data);
   SP_TRACE_INFO("      Name        : %s", mod_thread->name);
   SP_TRACE_INFO("      Priority    : %d", mod_thread->base.prio);
   SP_TRACE_INFO("      Stack size  : %d", mod_thread->stack_info.size);
   SP_TRACE_INFO("      Custom data : %d", (uint8_t)(uintptr_t)mod_thread->custom_data);

   taskIterator++;
}

/**
 * @brief Saves RTU's (Runtime Unit, Tasks/ISRs) configuration.
 */
RTOS_WRAPPER void SysPerf_InitRTUConfiguration(void)
{
   /**
    * @note Assign pointer to the local callback function to save cfg
    */
   k_thread_foreach(SysPerf_EnumerateCallback, NULL);
}

RTOS_WRAPPER uint32_t SysPerf_GetUptimeTimestamp(void)
{
   uint32_t timeStamp;
   timeStamp = k_uptime_ticks();
   timeStamp = k_ticks_to_us_near32(timeStamp);
   return timeStamp; /* in uS */
}
