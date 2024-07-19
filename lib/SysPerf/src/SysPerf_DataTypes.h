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
typedef SP_TASK_ID                    sp_TaskId;

/* SysPerf Task indexes */
typedef enum
{
    TaskLED0    = 0x00,
    TaskBLE     = 0x01,
    TaskSysPerf = 0x02,
    SP_TASK_NUM = SP_TASK_COUNT
} sp_TaskIterator;

/* Os Events */
typedef enum
{
    PRE_TASK_HOOK_EV   = 0x00,
    POST_TASK_HOOK_EV  = 0x01,
    PRE_ISR_HOOK_EV    = 0x02,
    POST_ISR_HOOK_EV   = 0x03,
    POST_IDLE_HOOK_EV  = 0x04
} sp_OsEvent;

/* Task states */
typedef enum
{
    TASK_STATE_ENTER       = 0x00,
    TASK_STATE_EXIT        = 0x01,
    TASK_STATE_INTERRUPTED = 0x02
} sp_TaskState;

/* SysPerf ISR indexes */
// TODO: add support to measure ISR runtime
typedef enum
{
    SP_ISR_NUM = SP_ISR_COUNT
} sp_IsrId;

/* Hook statistic */
typedef struct SP_PACKED
{
    uint64 timestamp;
    uint8 taskIterator;
    uint8 state;
} sp_Hook;

/* History handler */
typedef struct SP_PACKED
{
    sp_Hook entities[SP_HISTORY_SIZE];
    uint32 idx;
    uint32 size;
} sp_HistoryHandler;

/* Hook history statistic */
typedef struct SP_PACKED
{
    sp_HistoryHandler history;
    uint8 lastOsEvent;
} sp_HookHistory;

/* Configuration information of a task, filed be before scheduler start */
typedef struct SP_PACKED
{
    sp_TaskId taskId;
    sp_TaskPrio priority;
    sp_TaskStackSize stackSize;
    const char *taskName;
    // char taskName[SP_TASKNAME_MAX_LEN];
} sp_TaskConfigInfo;

/* Task runtime information */
typedef struct SP_PACKED
{
    uint64 taskTimestamp;
    uint32 totalRuntime;
    uint32 maxRuntime;
    uint32 taskSwitches;
} sp_TaskRuntime;

/* Runtime information of a ISR */
typedef struct SP_PACKED
{
    uint32 runtime;
    uint32 maxRuntime;
    uint32 isrSwitches;
} sp_IsrRuntime;

/* Total statistic */
typedef struct SP_PACKED
{
    uint32 TasksRuntime[SP_TASK_COUNT];
    uint32 TasksSwitches[SP_TASK_COUNT];
    uint32 MaxTasksRuntime[SP_TASK_COUNT];
    uint32 IsrRuntime[SP_TASK_COUNT];
    uint32 IdleRuntime;
    uint32 timestamp;
} sp_UptimeStat_t;

#endif /* SYSPERFDATATYPES_H */
