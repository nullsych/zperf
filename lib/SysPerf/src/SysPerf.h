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

/*******************************************************************************************************************
*                                            SYSPERF KEYWORDS                                                      *
*******************************************************************************************************************/
#define SP_LOCAL                            static
#define SP_LOCAL_INLINE                     static inline
#define SP_PACKED                           __packed
#define SP_ALIGNED(x)                       __aligned(x)

#define SP_RTOS_CALLED                      /* API used within RTOS hooks */
#define SP_USER_CALLED                      /* API used within user application code */
#define SP_HOOK_CALLED                      /* API used within SysPerf Hooks */

/*******************************************************************************************************************
*                                               MAPS & CONSTANTS                                                   *
*******************************************************************************************************************/
#define SP_TASK_COUNT                       MAP_RTOS_TASKS_COUNT        /* Total number of Tasks in the system    */
#define SP_ISR_COUNT                        MAP_RTOS_ISRS_COUNT         /* Total number of ISRs in the system     */
#define SP_TASKNAME_MAX_LEN                 MAP_RTOS_TASK_MAX_NAME_LEN  /* Maximum length for Task's name         */
#define SP_TIMESTAMP_TOP                    MAP_RTOS_UPTIME_TOP

#define SP_PTR2TASK                         MAP_RTOS_PTR2TASK
#define SP_PRIO                             MAP_RTOS_PRIO
#define SP_STACKSIZE                        MAP_RTOS_STACKSIZE
#define SP_TASK_ID                          MAP_RTOS_UNIQ_ID

#define SP_HISTORY_SIZE                     (8U)   /* Total number of hook-history entities, static allocated   */
#define SP_CPU_USAGE_THR                    (50U)  /* Threshold for CPU utilization accross all cores           */

/*******************************************************************************************************************
*                                                   MACROS                                                         *
*******************************************************************************************************************/
#define TASK_IS_VALID(taskIterator)         (taskIterator < SP_TASK_COUNT)
#define GET_CPULOAD(timeslice, totalRT)     ((totalRT * 100U) / timeslice)
#define GET_IDLETIME(timeslice, idleRT)     ((idleRT  * 100U) / timeslice)


/*******************************************************************************************************************
*                                            SYSPERF GLOBAL API                                                    *
*******************************************************************************************************************/
/* SysPerf Hooks Declarations for RTOS-only usage */
SP_RTOS_CALLED void SysPerf_PreTaskHook(void);
SP_RTOS_CALLED void SysPerf_PostTaskHook(void);
SP_RTOS_CALLED void SysPerf_PreISRHook(void);
SP_RTOS_CALLED void SysPerf_PostISRHook(void);
SP_RTOS_CALLED void SysPerf_IdleTaskRunning(void);

/* SysPerf API for user's code */
SP_USER_CALLED extern void SysPerf_RuntimeOverallStatistic(void);
SP_USER_CALLED extern void SysPerf_PreSchedulerInit(void);

#endif /* SYSPERF_H */