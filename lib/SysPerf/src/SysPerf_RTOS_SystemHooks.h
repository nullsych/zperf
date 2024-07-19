/**
 * @file   SysPerf_RTOS_SystemHooks.h
 * @brief  Implement RTOS-specific hook functions with invoked SysPerf's API.
 * @author kvant6ubl
 * @date   May 6, 2024
 *
 * @details Use next template to define RTOS-specific hook function's body:
 *
 * Os_Hook_Function (...)
 * {
 *    SysPerf_<Hook>(...);
 * }
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

void sys_trace_thread_switched_in_user(void)
{
   SysPerf_PreTaskHook();
}

void sys_trace_thread_switched_out_user(void)
{
   SysPerf_PostTaskHook();
}

void sys_trace_isr_enter_user(void)
{
   SysPerf_PreISRHook();
}

void sys_trace_isr_exit_user(void)
{
   SysPerf_PostISRHook();
}

void sys_trace_idle_user(void)
{
   SysPerf_IdleTaskRunning();
}

#endif /* SYSPERF_RTOS_WRAPPER_H */
