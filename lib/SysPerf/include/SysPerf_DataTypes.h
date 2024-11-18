/**
 * @file   SysPerf_DataTypes.h
 * @brief
 * @author kvant6ubl
 * @date   Jule 9, 2024
 *
 */

#ifndef SYS_PERF_DATATYPES_H
#define SYS_PERF_DATATYPES_H

#include "SysPerf.h"
#include "SysPerf_Keywords.h"

typedef SP_PTR2TASK                   sp_TaskPtr;
typedef SP_PRIO                       sp_TaskPrio;
typedef SP_STACKSIZE                  sp_TaskStackSize;
typedef SP_TASK_ID                    sp_TaskId;

#if (SP_CFG_ISR_RUNTIME_DISTINGUISH == SP_CFG_OFF)
/* Common ISR signature if case when above feature is not present in RTOS */
#define SP_ISR_ID_SIGN                (0xFFFC)
#else
typedef SP_ISR_ID                     sp_IsrId;
#endif /* SP_CFG_ISR_RUNTIME_DISTINGUISH */

/* Runtime Unit's (Task's/ISR's) state mask */
typedef enum
{
    /* Written when new Task (Thread) take precedence and start works */
    TASK_STATE_ENTER        = 0x00,

    /* Written when current Task (Thread) finished it work, and new one Task takes precedence */
    TASK_STATE_EXIT         = 0x01,

    /* Written when Pre/Post hook for the Task (Thread) was done successfully */
    TASK_STATE_HOOK_SUCCESS = 0x02,

    /* Written when invalid Task (Thread) identifier obtained */
    TASK_STATE_INVALID_ID   = 0x03,

#if (SP_CFG_ISR_RUNTIME_SUPPORTED == SP_CFG_ON)
    /* Written when new ISR take precedence and start works */
    ISR_STATE_ENTER         = 0x04,

    /* Written when current ISR finished it work, and new one Task/ISR takes precedence */
    ISR_STATE_EXIT          = 0x05,

    #if (SP_CFG_ISR_RUNTIME_DISTINGUISH == SP_CFG_ON)
    /* Written when Pre/Post hook for the ISR was done successfully */
    ISR_STATE_HOOK_SUCCESS  = 0x06,

    /* Written when invalid ISR identifier obtained */
    ISR_STATE_INVALID_ID    = 0x07,
    #endif /* SP_CFG_ISR_RUNTIME_DISTINGUISH */

#endif /* SP_CFG_ISR_RUNTIME_SUPPORTED */

#if (SP_CFG_RTOS_HAS_IDLE_SWITCHES == SP_CFG_ON)
    /* Written when Idle Tasks (Thread) start works */
    IDLE_STATE_ENTER        = 0x08,

    /* Written when current Idle Tasks (Thread) finished it work, and new one Task/ISR takes precedence */
    IDLE_STATE_EXIT         = 0x09,
#else
    /* Written when CPU is in Idle in general */
    IDLE_STATE_RUNNING      = 0x10,
#endif /* SP_CFG_RTOS_HAS_IDLE_SWITCHES */

} sp_RuntimeStateMask;

/* Hook statistic */
typedef struct SP_PACKED
{
    uint64 timestamp;
    uint32 identifier;
    uint8  stateMask;
} sp_Hook;

/* Start-up configuration info */
typedef struct SP_PACKED
{
    sp_TaskStackSize stackSize;
    sp_TaskPrio priority;
    uint8 taskIterator;
    char *taskName;
} sp_TaskConfigInfo;

/* Task (Thread) runtime information */
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
    uint64 isrTimestamp;
    uint32 totalRuntime;
    uint32 maxRuntime;
    uint32 isrSwitches;
} sp_IsrRuntime;

/* Total system uptime statistic */
typedef struct SP_PACKED
{
    uint32 TasksRuntime[SP_TASK_COUNT];
    uint32 TasksSwitches[SP_TASK_COUNT];
    uint32 MaxTasksRuntime[SP_TASK_COUNT];
    uint32 IsrRuntime[SP_TASK_COUNT];
    uint32 IdleRuntime;
    uint32 timestamp;
} sp_UptimeStatistic_t;

/* Start-up configuration info handler for Tasks (Threads) with unique identifier */
typedef struct SP_PACKED
{
   uint32 taskId;
   sp_TaskConfigInfo data;
} sp_TaskConfigInfoHandler;

#endif /* SYS_PERF_DATATYPES_H */
