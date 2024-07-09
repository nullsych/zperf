/**
 * @file   SysPerf_DataTypes.h
 * @brief
 * @author kvant6ubl
 * @date   Jule 9, 2024
 *
 */

#ifndef SYSPERFDATATYPES_H
#define SYSPERFDATATYPES_H

#include "SysPerf.h"

/*******************************************************************************************************************
*                                                 DATA TYPES                                                       *
*******************************************************************************************************************/
typedef SP_PTR2TASK                   sp_TaskPtr;
typedef SP_PRIO                       sp_TaskPrio;
typedef SP_STACKSIZE                  sp_TaskStackSize;

// #ifndef __packed
// #define __packed        __attribute__((__packed__))
// #endif

// __packed

/* Tasks Identifiers */
typedef enum
{
    TaskLED0 = 0,
    TaskBLE = 1,
    TaskSysPerf = 2,
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

#endif /* SYSPERFDATATYPES_H */
