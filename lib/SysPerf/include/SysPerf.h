/**
 * @file   SysPerf.h
 * @brief  Contain main implementation of SysPerf functionality.
 * @author kvant6ubl
 * @date   May 6, 2024
 *
 */

#ifndef SYSPERF_H
#define SYSPERF_H

#include "SysPerf_RTOS_Map.h"
#include "SysPerf_Keywords.h"

/*******************************************************************************************************************
*                                               MAPS                                                               *
*******************************************************************************************************************/
#define SP_TASK_COUNT                       MAP_RTOS_OVERALL_TASKS_COUNT /* Overall number of Tasks in the system */
#define SP_IDLE_COUNT                       MAP_RTOS_IDLE_TASK_COUNT     /* Total number of Idle Tasks            */
#define SP_ISR_COUNT                        MAP_RTOS_ISRS_COUNT          /* Total number of ISRs in the system    */
#define SP_TASKNAME_MAX_LEN                 MAP_RTOS_TASK_MAX_NAME_LEN   /* Maximum length for Task's name        */
#define SP_TIMESTAMP_TOP                    MAP_RTOS_UPTIME_TOP

#define SP_PTR2TASK                         MAP_RTOS_PTR2TASK
#define SP_PRIO                             MAP_RTOS_PRIO
#define SP_STACKSIZE                        MAP_RTOS_STACKSIZE

#define SP_TASK_ID                          MAP_RTOS_UNIQ_ID
#define SP_ISR_ID                           MAP_RTOS_UNIQ_ID

#define SP_ISR_LOCK                         MAP_RTOS_ISR_LOCK
#define SP_ISR_UNLOCK                       MAP_RTOS_ISR_UNLOCK

#define SP_TRACE_INFO                       MAP_RTOS_TRACE_INFO
#define SP_TRACE_WARN                       MAP_RTOS_TRACE_WARN
#define SP_TRACE_ERRO                       MAP_RTOS_TRACE_ERRO


/*******************************************************************************************************************
*                                                   MACROS                                                         *
*******************************************************************************************************************/
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
USER_CALLED extern void SysPerf_RuntimeOverallStatistic(void);
USER_CALLED extern void SysPerf_PreSchedulerInit(void);
USER_CALLED extern void SysPerf_PostSchedulerInit(void);

#endif /* SYSPERF_H */