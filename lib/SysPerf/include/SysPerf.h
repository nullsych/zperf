/**
 * @file   SysPerf.h
 * @brief  Contain main implementation of SysPerf functionality.
 * @author kvant6ubl
 * @date   May 6, 2024
 *
 */

#ifndef SYSPERF_H
#define SYSPERF_H

#include "RTOS_Map.h"
#include "SysPerf_DataTypes.h"
#include "SysPerf_Constants.h"
#include "SysPerf_Keywords.h"

/*******************************************************************************************************************
*                                                   MACROS                                                         *
*******************************************************************************************************************/
#define SP_ISR_LOCK                         MAP_RTOS_ISR_LOCK
#define SP_ISR_UNLOCK                       MAP_RTOS_ISR_UNLOCK

#define SP_TRACE_INFO                       MAP_RTOS_TRACE_INFO
#define SP_TRACE_WARN                       MAP_RTOS_TRACE_WARN
#define SP_TRACE_ERRO                       MAP_RTOS_TRACE_ERRO

#define SP_ISR_ID_SIGN                      (0xFFFC)                                      /* Common ISR signature */

#define TASK_IS_VALID(taskIterator)         ((taskIterator < SP_TASK_COUNT) && (taskIterator >= 0))
#define GET_CPU_PERCENT(timeslice, totalRT) ((totalRT * 100U) / timeslice)


/*******************************************************************************************************************
*                                            SYSPERF GLOBAL API                                                    *
*******************************************************************************************************************/
/* SysPerf Hooks Declarations for RTOS-only usage */
RTOS_CALLED void SysPerf_PreTaskHook(void);
RTOS_CALLED void SysPerf_PostTaskHook(void);
RTOS_CALLED void SysPerf_PreISRHook(void);
RTOS_CALLED void SysPerf_PostISRHook(void);
RTOS_CALLED void SysPerf_IdleTaskRunning(void);

/* SysPerf API for user's code */
USER_CALLED extern void SysPerf_TraceHistory(void);
USER_CALLED extern void SysPerf_RuntimeOverallStatistic(void);
USER_CALLED extern void SysPerf_PreSchedulerInit(void);
USER_CALLED extern void SysPerf_PostSchedulerInit(void);

#endif /* SYSPERF_H */