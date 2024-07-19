/**
 * @file   SysPerf_RTOS_Wrapper.h
 * @brief  Implementation of wrappers that encapsulate RTOS-specific API.
 * @author kvant6ubl
 * @date   Jul 18, 2024
 *
 */

#ifndef SYSPERF_RTOS_WRAPPER_H
#define SYSPERF_RTOS_WRAPPER_H

/* Place RTOS-specific headers */
#include <zephyr/kernel.h>
#include <zephyr/debug/thread_analyzer.h>
#include <zephyr/tracing/tracing.h>
#include <stdint-gcc.h>

#include "SysPerf.h"

/**
 * @brief Get unique ID (RTOS specific) of the current task.
 */
unsigned int SysPerf_GetTaskId(void)
{
   /* Recomendation from Zephyr RTOS: Can't use k_current_get() as thread base might be incorrect */
   return (unsigned int)k_sched_current_thread_query();
}

/**
 * @brief Get current task index (SysPerf specific), embedded in task configuration
 */
uint8 SysPerf_GetTaskIterator(void)
{
   return (uint8)k_thread_custom_data_get();
}

uint64 SysPerf_GetUptimeTimestamp(void)
{
   uint64 timeStamp;
   timeStamp = k_uptime_ticks();
   timeStamp = k_ticks_to_us_near64(timeStamp);
   return timeStamp; /* in uS */
}

#endif /* SYSPERF_RTOS_WRAPPER_H */
