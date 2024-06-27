/**
 * @file   SysPerf_RTOS_Wrapper.h
 * @brief  Implement RTOS-specific hook functions with wrapped SysPerf ones
 * @author kvant6ubl
 * @date   May 6, 2024
 *
 * Use next template to define RTOS-specific hook function's body:
 *
 * Os_Hook_Function (...)
 * {
 *    SysPerf__Hook(...);
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
   /* code */
}

void sys_trace_idle_user(void)
{
   SysPerf_IdleTaskRunning();
}

#endif /* SYSPERF_RTOS_WRAPPER_H */