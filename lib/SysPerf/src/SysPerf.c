/* Place RTOS-speficif headers */
#include <zephyr.h>
#include <debug/thread_analyzer.h>
#include <tracing/tracing.h>
#include <tracing_user.h>

#include "SysPerf.h"

/* SysPerf Hooks Implementation */
void SysPerf__PreTaskHook(sp_TaskPtr task)
{
   SP_TRACE("# ====== SysPerf__PreTaskHook: Task %s\n", task->name);
}

void SysPerf__PostTaskHook(sp_TaskPtr task)
{
   SP_TRACE("# ====== SysPerf__PostTaskHook: Task %s\n", task->name);
}
