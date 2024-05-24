/**
 * @file   SysPerf_RTOS_Wrapper.h
 * @brief  Implement RTOS-speficif hook functions with wrapped SysPerf ones
 * @author kvant6ubl
 * @date   May 6, 2024
 *
 * Use next template to define RTOS-speficif hook function's body:
 *
 * Os_Hook_Function (...)
 * {
 *    SysPerf__Hook(...);
 * }
 *
 */

#ifndef SYSPERF_RTOS_WRAPPER_H
#define SYSPERF_RTOS_WRAPPER_H

/* Place RTOS-speficif headers */
#include <zephyr.h>
#include <debug/thread_analyzer.h>
#include <tracing/tracing.h>
#include <stdint-gcc.h>

#include "SysPerf.h"

void sys_trace_thread_switched_in_user(struct k_thread *thread)
{
    SysPerf_PreTaskHook((sp_TaskPtr)thread);
}

void sys_trace_thread_switched_out_user(struct k_thread *thread)
{
    SysPerf_PostTaskHook((sp_TaskPtr)thread);
}

#endif /* SYSPERF_RTOS_WRAPPER_H */