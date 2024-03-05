#ifndef __SYSPERF_H__
#define __SYSPERF_H__

/* ------------------------------------ Place RTOS-speficif headers ------------------------------------ */
#include <zephyr.h>
#include <debug/thread_analyzer.h>
#include <tracing/tracing.h>
/* ---------------------------------------------- End -------------------------------------------------- */


/* --------------------------- Map RTOS-speficif printf format function -------------------------------- */
#define SP_TRACE printk
/* ---------------------------------------------- End -------------------------------------------------- */


/* ------------------------------ Map RTOS-speficif Tasks & ISRs types --------------------------------- */
#define PTR2TASK struct k_thread *
#define PTR2ISR
/* ---------------------------------------------- End -------------------------------------------------- */


/* --------------------------------- Map RTOS-speficif constants --------------------------------------- */
#define SYSPERF_TASKS_COUNT (0u)
/* ---------------------------------------------- End -------------------------------------------------- */

/* SysPerf types Declarations */
typedef PTR2TASK sp_TaskPtr;

/* SysPerf Hooks Declarations */
void SysPerf__PreTaskHook(sp_TaskPtr);
void SysPerf__PostTaskHook(sp_TaskPtr);
void SysPerf__PreISRHook();
void SysPerf__PostISRHook();
void SysPerf__IdleRaskRunning();

#endif /* __SYSPERF_H__ */