#ifndef __SYSPERF_RTOS_WRAPPER_H__
#define __SYSPERF_RTOS_WRAPPER_H__

/* Place RTOS-speficif headers */
#include <zephyr.h>
#include <debug/thread_analyzer.h>
#include <tracing/tracing.h>

#include "SysPerf.h"

/* Implement RTOS-speficif hook functions with wrapped SysPerf ones */

/* 
Template:

    Hook_Function (...)
    {
        SysPerf__PreTaskHook(...);
    }
*/

void sys_trace_thread_switched_in_user(struct k_thread *thread)
{
    SysPerf__PreTaskHook((sp_TaskPtr)thread);
}

void sys_trace_thread_switched_out_user(struct k_thread *thread)
{
    SysPerf__PostTaskHook((sp_TaskPtr)thread);
}

#endif /* __SYSPERF_RTOS_WRAPPER_H__ */