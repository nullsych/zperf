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
#define SP_RTOS_CALLED
#define SP_USER_CALLED

/*******************************************************************************************************************
*                                          SYSPERF CONSTANTS                                                       *
*******************************************************************************************************************/
#define SP_TASK_COUNT                       MAP_RTOS_TASKS_COUNT        /* Total number of Tasks in the system    */
#define SP_ISR_COUNT                        MAP_RTOS_ISRS_COUNT         /* Total number of ISRs in the system     */
#define SP_TASKNAME_MAX_LEN                 MAP_RTOS_TASK_MAX_NAME_LEN  /* Maximum length for Task's name         */

#define SP_HISTORY_SIZE                     (1024U)  /* Total number of hook-history entities, static allocated   */
#define SP_CPU_USAGE_THR                    (50U)    /* Threshold for CPU utilization accross all cores           */

/*******************************************************************************************************************
*                                                SYSPERF MACROS                                                    *
*******************************************************************************************************************/
#define IS_TASK_VALID(taskId)               (taskId < SP_TASK_COUNT)


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
    TaskBLE = 1,
    TaskSysPerf = 3,
    SP_TASK_NUM = SP_TASK_COUNT
} sp_TaskId;

/* Task states */
typedef enum
{
    SP_TASK_ENTER = 0,
    SP_TASK_EXIT  = 1,
    SP_TASK_INTERRUPTED = 2,
} sp_TaskState;

/* ISRs Identifiers */
typedef enum
{
    SP_ISR_NUM = SP_ISR_COUNT
} sp_IsrId;

/* Hook statistic */
typedef struct
{
    uint32 timestamp;
    uint8 taskId;
    uint8 state;
} sp_Hook;

/* Hook history statistic */
typedef struct
{
    sp_Hook history[SP_HISTORY_SIZE];
    uint32 idx;
    uint32 size;
} sp_HookHistory;

/* Configuration information of a task */
typedef struct
{
    sp_TaskPrio priority;
    sp_TaskStackSize stackSize;
} sp_TaskConfigInfo;

/* Task runtime information */
typedef struct
{
    uint32 totalRuntime;
    uint32 maxRuntime;
    uint32 taskTimestamp;
    uint32 taskSwitches;
    const char taskName[SP_TASKNAME_MAX_LEN];
} sp_TaskRuntime;

/* Runtime information of a ISR */
typedef struct
{
    uint32 runtime;
    uint32 maxRuntime;
    uint32 isrSwitches;
} sp_IsrRuntime;

/* Uptime statistic */
typedef struct
{
    uint32 TotalTasksRuntime[SP_TASK_NUM];
    uint32 TotalTasksSwitches[SP_TASK_NUM];
    uint32 MaxTasksRuntime[SP_TASK_NUM];
    uint32 TotalIsrRuntime[SP_ISR_NUM];
} sp_UptimeStat_t;

/* TODO: History runtime information */


/*******************************************************************************************************************
*                                            SYSPERF API                                                           *
*******************************************************************************************************************/
/* SysPerf Hooks Declarations for RTOS-only usage */
SP_RTOS_CALLED void SysPerf_PreTaskHook(sp_TaskPtr);
SP_RTOS_CALLED void SysPerf_PostTaskHook(sp_TaskPtr);
SP_RTOS_CALLED void SysPerf_PreISRHook(void);
SP_RTOS_CALLED void SysPerf_PostISRHook(void);
SP_RTOS_CALLED void SysPerf_IdleRaskRunning(void);

/* SysPerf API for user's code */
SP_USER_CALLED extern void SysPerf_RuntimeOverallStatistic(void);
SP_USER_CALLED extern void SysPerf__PreSchedulerInit(void);

#endif /* SYSPERF_H */