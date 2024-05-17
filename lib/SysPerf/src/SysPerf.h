/**
 * @file   SysPerf.h
 * @brief  Contain main implementation of SysPerf functionality.
 * @author kvant6ubl
 * @date   May 6, 2024
 *
 */

#ifndef __SYSPERF_H__
#define __SYSPERF_H__

#include "SysPerf_RTOS_Map.h"

/*******************************************************************************************************************
*                                            SYSPERF KEYWORDS                                                      *
*******************************************************************************************************************/
#define SP_LOCAL                            static
#define SP_LOCAL_INLINE                     static inline


/*******************************************************************************************************************
*                                            SYSPERF DATA TYPES                                                    *
*******************************************************************************************************************/

typedef MAP_RTOS_PTR2TASK                   sp_TaskPtr;
typedef MAP_RTOS_PRIO                       sp_TaskPrio;
typedef MAP_RTOS_STACKSIZE                  sp_TaskStackSize;

/* Tasks Identifiers */
typedef enum
{
    TaskLED0 = 0,
    TaskIdle = 1,
    SYSPERF_TASK_NUM = MAP_RTOS_TASKS_COUNT
} sp_TaskId;

/* ISRs Identifiers */
typedef enum
{
    SYSPERF_ISR_NUM = MAP_RTOS_ISRS_COUNT
} sp_IsrId;

/* Configuration information of a task */
typedef struct
{
    sp_TaskPrio priority;
    sp_TaskStackSize stackSize;
} sp_TaskConfigInfo_t;

/* Runtime information of a task per 1 epoch */
typedef struct
{
    uint32 totalRuntime;
    uint32 maxRuntime;
    uint32 taskTimestamp;
    uint32 taskSwitches;
    char taskName[MAP_RTOS_TASK_MAX_NAME_LEN];
} sp_TaskRuntime_t;

/* Runtime information of a ISR */
typedef struct
{
    uint32 runtime;
    uint32 maxRuntime;
    uint32 isrSwitches;
} sp_IsrRuntime_t;

/* Hook statistic */
typedef struct
{
    uint32 timestamp;
    uint32 taskRuntime;
    uint8 taskId;
} sp_HookStat_t;

/* Uptime statistic */
typedef struct
{
    uint32 TotalTasksRuntime[SYSPERF_TASK_NUM];
    uint32 TotalTasksSwitches[SYSPERF_TASK_NUM];
    uint32 MaxTasksRuntime[SYSPERF_TASK_NUM];
    uint32 TotalIsrRuntime[SYSPERF_ISR_NUM];
} sp_UptimeStat_t;

/* TODO: History runtime information */


/*******************************************************************************************************************
*                                            SYSPERF API                                                           *
*******************************************************************************************************************/
/* SysPerf Hooks Declarations for RTOS-only usage */
void SysPerf__PreSchedulerInit(void);
void SysPerf_PreTaskHook(sp_TaskPtr);
void SysPerf_PostTaskHook(sp_TaskPtr);
void SysPerf__PreISRHook();
void SysPerf__PostISRHook();
void SysPerf__IdleRaskRunning();

/* SysPerf API for user's code */
extern void SysPerf_RuntimeOverallStatistic(void);

#endif /* __SYSPERF_H__ */