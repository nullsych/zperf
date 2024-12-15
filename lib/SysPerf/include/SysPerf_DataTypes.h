/**
 * @file   SysPerf_DataTypes.h
 * @brief
 * @author kvant6ubl
 * @date   Jule 9, 2024
 *
 */

#ifndef SYS_PERF_DATATYPES_H
#define SYS_PERF_DATATYPES_H

#include "SysPerf_RTOS_Map.h"
#include "SysPerf_Constants.h"
#include "SysPerf_Keywords.h"

typedef MAP_RTOS_PTR2TASK                   sp_TaskPtr;
typedef MAP_RTOS_PRIO                       sp_TaskPrio;
typedef MAP_RTOS_STACKSIZE                  sp_TaskStackSize;
typedef MAP_RTOS_UNIQ_ID                    sp_TaskId;
typedef MAP_RTOS_UNIQ_ID                    sp_IsrId;

/* Runtime Unit's (Task's/ISR's) state mask */
typedef enum
{
    /* Written when new Task (Thread) take precedence and start works */
    TASK_STATE_ENTER        = 0x00,

    /* Written when current Task (Thread) finished it work, and new one Task takes precedence */
    TASK_STATE_EXIT         = 0x01,

    /* Written when new ISR take precedence and start works */
    ISR_STATE_ENTER         = 0x02,

    /* Written when current ISR finished it work, and new one Task/ISR takes precedence */
    ISR_STATE_EXIT          = 0x03,

    /* Written when Idle Tasks (Thread) is running */
    IDLE_STATE_RUNNING      = 0x04

} sp_RuntimeStateMask;

/* Hook statistic */
typedef struct SPK_PACKED
{
    uint32_t timestamp;
    uint32_t identifier;
    uint8_t  state;
} sp_Hook;

/* Task (Thread) runtime information */
typedef struct SPK_PACKED
{
    uint32_t taskTimestamp;
    uint32_t totalRuntime;
    uint32_t maxRuntime;
    uint32_t taskSwitches;
} sp_TaskRuntime;

/* Runtime information of a ISR */
typedef struct SPK_PACKED
{
    uint32_t isrTimestamp;
    uint32_t totalRuntime;
    uint32_t maxRuntime;
    uint32_t isrSwitches;
} sp_IsrRuntime;

/* Total system uptime statistic */
typedef struct SPK_PACKED
{
    uint32_t TasksRuntime[SP_TASK_COUNT];
    uint32_t TasksSwitches[SP_TASK_COUNT];
    uint32_t MaxTasksRuntime[SP_TASK_COUNT];
    uint32_t IsrRuntime[SP_TASK_COUNT];
    uint32_t IdleRuntime;
    uint32_t timestamp;
} sp_UptimeStatistic_t;

#endif /* SYS_PERF_DATATYPES_H */
