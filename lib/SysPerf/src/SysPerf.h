#ifndef __SYSPERF_H__
#define __SYSPERF_H__

/*******************************************************************************************************************
*                                             RTOS-SPECIFIC HEADERS                                                *
*******************************************************************************************************************/
/** @details: Include necessary headers **/
#include <zephyr.h>
#include <logging/log.h>


/*******************************************************************************************************************
*                                             RTOS-SPECIFIC TRACES                                                 *
*******************************************************************************************************************/
/** @details: Map specific functions with printf() format to trace SysPerf* statistics **/
#define SP_TRACE_INFO LOG_INF
#define SP_TRACE_WARN LOG_WRN
#define SP_TRACE_ERRO LOG_ERR


/*******************************************************************************************************************
*                                           RTOS-SPECIFIC CONTANTS                                                 *
*******************************************************************************************************************/
/** @details: Map RTOS-specific constants **/
#define MAP_RTOS_TASKS_COUNT                (2u)                        /* Number of tasks, runnings in system */
#define MAP_RTOS_ISRS_COUNT                 (0u)                        /* Number of ISRs, runnings in system */
#define MAP_RTOS_UPTIME_TOP                 (0xFFFFFFul)                /* Zephyr timer top value */
#define MAP_RTOS_TASK_MAX_NAME_LEN          CONFIG_THREAD_MAX_NAME_LEN  /* Zephyr tasks max len of name */


/*******************************************************************************************************************
*                                             RTOS-SPECIFIC TYPES                                                  *
*******************************************************************************************************************/
/** @details: Map RTOS-speficif standart, Tasks & ISRs types **/
#define MAP_RTOS_PTR2TASK                   struct k_thread *
#define MAP_RTOS_PTR2ISR
#define MAP_RTOS_PRIO                       int
#define MAP_RTOS_STACKSIZE                  int
typedef uint64_t                            uint64;
typedef uint32_t                            uint32;
typedef uint16_t                            uint16;
typedef uint8_t                             uint8;
typedef int64_t                             sint64;
typedef int32_t                             sint32;
typedef int16_t                             sint16;
typedef int8_t                              sint8;

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
/* ***************************************************************************************************************** */



/*******************************************************************************************************************
*                                            SYSPERF DATA TYPES                                                    *
*******************************************************************************************************************/
/* SysPerf types Declarations */
typedef MAP_RTOS_PTR2TASK                   sp_TaskPtr;
typedef MAP_RTOS_PRIO                       sp_TaskPrio;
typedef MAP_RTOS_STACKSIZE                  sp_TaskStackSize;
typedef uint32_t                            sp_TaskRuntime;

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

/* Epoch main runtime statistic */
typedef struct
{
    sp_TaskRuntime_t TasksRuntime[SYSPERF_TASK_NUM];
    sp_IsrRuntime_t IsrRuntime[SYSPERF_ISR_NUM];
} sp_EpochStat_t;

/* Uptime statistic */
typedef struct
{
    uint32 TotalTasksRuntime[SYSPERF_TASK_NUM];
    uint32 TotalIsrRuntime[SYSPERF_ISR_NUM];
} sp_UptimeStat_t;


/*******************************************************************************************************************
*                                            SYSPERF API                                                           *
*******************************************************************************************************************/
/* SysPerf Hooks Declarations for RTOS-only usage */
void SysPerf__PreSchedulerInit(void);
void SysPerf__PreTaskHook(sp_TaskPtr);
void SysPerf__PostTaskHook(sp_TaskPtr);
void SysPerf__PreISRHook();
void SysPerf__PostISRHook();
void SysPerf__IdleRaskRunning();
void SysPerf__EpochOverallStatistic(void);

#endif /* __SYSPERF_H__ */