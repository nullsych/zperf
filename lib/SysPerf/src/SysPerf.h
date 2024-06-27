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
*                                               MAPS & CONSTANTS                                                   *
*******************************************************************************************************************/
#define SP_TASK_COUNT                       MAP_RTOS_TASKS_COUNT        /* Total number of Tasks in the system    */
#define SP_ISR_COUNT                        MAP_RTOS_ISRS_COUNT         /* Total number of ISRs in the system     */
#define SP_TASKNAME_MAX_LEN                 MAP_RTOS_TASK_MAX_NAME_LEN  /* Maximum length for Task's name         */
#define SP_TIMESTAMP_TOP                    MAP_RTOS_UPTIME_TOP
#define SP_HISTORY_SIZE                     (1024U)  /* Total number of hook-history entities, static allocated   */
#define SP_CPU_USAGE_THR                    (50U)    /* Threshold for CPU utilization accross all cores           */

/*******************************************************************************************************************
*                                                   MACROS                                                         *
*******************************************************************************************************************/
#define IS_TASK_VALID(taskId)               (taskId < SP_TASK_COUNT)

/*******************************************************************************************************************
*                                                 DATA TYPES                                                       *
*******************************************************************************************************************/
typedef MAP_RTOS_PTR2TASK                   sp_TaskPtr;
typedef MAP_RTOS_PRIO                       sp_TaskPrio;
typedef MAP_RTOS_STACKSIZE                  sp_TaskStackSize;

/* Tasks Identifiers */
typedef enum
{
    TaskLED0 = 0,
    TaskBLE = 1,
    TaskSysPerf = 2,
    TaskIdle = 3,
    SP_TASK_NUM = SP_TASK_COUNT
} sp_TaskId;

/* Os Events */
typedef enum
{
    PRE_TASK_HOOK_EV   = 0,
    POST_TASK_HOOK_EV  = 1,
    PRE_ISR_HOOK_EV    = 2,
    POST_ISR_HOOK_EV   = 3,
    POST_IDLE_HOOK_EV  = 4,
} sp_OsEvent;

/* Task states */
typedef enum
{
    TASK_STATE_ENTER       = 0,
    TASK_STATE_EXIT        = 1,
    TASK_STATE_INTERRUPTED = 2
} sp_TaskState;

/* ISRs Identifiers */
// TODO: add support to measure ISR runtime
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

/* History handler */
typedef struct
{
    sp_Hook entities[SP_HISTORY_SIZE];
    uint32 idx;
    uint32 size;
} sp_HistoryHandler;

/* Hook history statistic */
typedef struct
{
    sp_HistoryHandler history;
    uint8 lastOsEvent;
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

/* Total statistic */
typedef struct
{
    uint32 TotalTasksRuntime[SP_TASK_COUNT];
    uint32 TotalTasksSwitches[SP_TASK_COUNT];
    uint32 MaxTasksRuntime[SP_TASK_COUNT];
    uint32 TotalIsrRuntime[SP_TASK_COUNT];
    uint32 TotalIdleRuntime;
    uint32 statisticTracedTimestamp;
    bool IdleWasRunning;
} sp_TotalStat_t;


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