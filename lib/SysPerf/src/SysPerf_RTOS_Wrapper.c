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
// #include <zephyr/arch/arm/irq.h>
#include <zephyr/debug/thread_analyzer.h>
#include <zephyr/tracing/tracing.h>
#include <stdint-gcc.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(SysPerf_RTOS_Wrapper, LOG_LEVEL_DBG);

/*******************************************************************************************************************
*                                                SYSPERF HEADERS                                                   *
*******************************************************************************************************************/
#include "SysPerf_DataTypes.h"
#include "SysPerf_SetupData.h"
#include "SysPerf_Keywords.h"

/*******************************************************************************************************************
*                                            WRAPPER API IMPLEMENTATION                                            *
********************************************************************************************************************/

/**
 * @brief Get unique ID (RTOS specific) of the current Task (Thread).
 */
RTOS_WRAPPER uint32_t SysPerf_GetTaskId(void)
{
   /**
    * @note Recommendation from Zephyr RTOS: Can't use k_current_get() as thread base might be incorrect
    */
   return (uint32_t)k_sched_current_thread_query();
}

/**
 * @brief Get unique ID of the current ISR.
 */
RTOS_WRAPPER uint32_t SysPerf_GetIsrId(void)
{
   return (uint32_t)__get_IPSR();
}

SPK_LOCAL RTOS_WRAPPER void SysPerf_LocalCallback(sp_TaskPtr thread, void *user_data)
{
   static uint8_t taskIterator = 0U;

   SP_TRACE_INFO("+ k_thread_foreach -> thread name  : %s", thread->name);
   SP_TRACE_INFO("+ k_thread_foreach -> thread zid   : 0x%x", (uint32_t)thread);
   SP_TRACE_INFO("+ k_thread_foreach -> thread prio  : %u", (uint8_t)thread->base.prio);
   SP_TRACE_INFO("+ k_thread_foreach -> thread stack : %u", (uint32_t)thread->stack_info.size);
   SP_TRACE_INFO("+ k_thread_foreach -> iterator     : %u\n", taskIterator);

   SysPerf_StartupData_Init(taskIterator, (uint32_t)thread, thread->name, (uint8_t)thread->base.prio, (uint32_t)thread->stack_info.size);
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
   k_thread_foreach(SysPerf_LocalCallback, NULL);
}

/**
 * @brief Get task index (SysPerf specific) based on taskId.
 */
RTOS_WRAPPER uint8_t SysPerf_GetTaskIterator(uint32_t taskId)
{
   return SysPerf_StartupData_GetTaskIterator(taskId);
}

RTOS_WRAPPER uint32_t SysPerf_GetUptimeTimestamp(void)
{
   uint32_t timeStamp;
   timeStamp = k_uptime_ticks();
   timeStamp = k_ticks_to_us_near32(timeStamp);
   return timeStamp; /* in uS */
}
