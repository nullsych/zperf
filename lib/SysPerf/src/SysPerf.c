/**
 * @file   SysPerf.c
 * @brief  Contain main implementation of SysPerf functionality.
 * @author kvant6ubl
 * @date   May 6, 2024
 *
 */

/*******************************************************************************************************************
 *                                             RTOS-SPECIFIC HEADERS                                                *
 *******************************************************************************************************************/
#include <stdlib.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(SysPerf, LOG_LEVEL_DBG);

/*******************************************************************************************************************
*                                             SYSPERF HEADERS                                                      *
*******************************************************************************************************************/
#include "SysPerf.h"
#include "SysPerf_History.h"
#include "SysPerf_Keywords.h"
#include "SysPerf_DataTypes.h"
#include "SysPerf_RTOS_Wrapper.h"
#include "SysPerf_SetupData.h"
#include "SysPerf_Cfg.cfg"

/*******************************************************************************************************************
*                                    MAIN SYSPERF STATISTIC ENTITIES                                               *
*******************************************************************************************************************/

/**
 * @brief Intermediate storage for uptime statistic, reseted within 1 trace cycle (SP-cycle).
 * @details All collected data later copied to TraceCycle_UptimeStatistic.
 */
SP_LOCAL sp_UptimeStatistic_t       Hook_UptimeStatistic;

/**
 * @brief Intermediate storage for Tasks (Threads) runtime statistic, reseted within 1 trace cycle (SP-cycle).
 * @details All collected data later copied to TraceCycle_TasksStatistics.
 */
SP_LOCAL sp_TaskRuntime             Hook_TasksStatistics[SP_TASK_COUNT];

#if (SP_CFG_ISR_RUNTIME_SUPPORTED == SP_CFG_ON)
   #if (SP_CFG_ISR_RUNTIME_DISTINGUISH == SP_CFG_ON)
      /**
       * @brief Intermediate storage for ISRs runtime statistic, reseted within 1 trace cycle (SP-cycle).
       * @details All collected data later copied to TraceCycle_IrqsStatistics.
       */
      SP_LOCAL sp_IsrRuntime        Hook_IrqsStatistics[SP_ISR_COUNT];
   #elif (SP_CFG_ISR_TOTAL_RUNTIME_ONLY == SP_CFG_ON)
      SP_LOCAL sp_IsrRuntime        Hook_IsrTotalStatistic;
   #endif /* SP_CFG_ISR_RUNTIME_DISTINGUISH */
#endif /* SP_CFG_ISR_RUNTIME_SUPPORTED */

/**
 * @brief Idle Tasks (Threads) uniue identifiers.
 */
SP_LOCAL uint32                     Idle_TaskIds[SP_IDLE_COUNT];

/**
 * @brief Store a copy of uptime statistic when SysPerf_CycleBackup() routine occurs.
 * @details Used to trace all collected information to avoid race condition in rescheduling points.
 */
SP_LOCAL sp_UptimeStatistic_t       TraceCycle_UptimeStatistic;

/**
 * @brief Store a copy of Tasks (Threads) runtime statistic when SysPerf_CycleBackup() routine occurs.
 * @details Used to trace all collected information to avoid race condition in rescheduling points.
 */
SP_LOCAL sp_TaskRuntime             TraceCycle_TasksStatistics[SP_TASK_COUNT];

/*******************************************************************************************************************
*                                             SYSPERF LOCAL API                                                    *
*******************************************************************************************************************/
SP_LOCAL uint32 SysPerf_GetTaskRuntime(uint8 taskIterator, uint32 timeStamp);
SP_LOCAL uint64 SysPerf_GetTimestampDelta(uint64, uint64);
SP_LOCAL uint32 SysPerf_GetSystemRuntime(void);
SP_LOCAL uint32 SysPerf_GetIdleRuntime(void);

SP_LOCAL void SysPerf_CycleBackup(void);

SP_LOCAL void SysPerf_IsrLock(unsigned int *key);
SP_LOCAL void SysPerf_IsrUnlock(unsigned int key);
SP_LOCAL void SysPerf_ResetRuntimeStatistic(void);

SP_LOCAL void SysPerf_TraceCycleStatistic(void);
SP_LOCAL void SysPerf_TraceUptimeStatistic(void);
SP_LOCAL void SysPerf_TraceCpuUsage(void);

SP_LOCAL_INLINE void SysPerf_UpdateISRsStatistic(uint8 isrIterator, uint64 timestamp, uint32 isrRuntime);
SP_LOCAL_INLINE void SysPerf_UpdateTaskStatistic(uint8 taskIterator, uint64 timestamp, uint32 taskRuntime);
SP_LOCAL_INLINE void SysPerf_UpdateUptimeStatistic(uint8 taskIterator, uint32 taskRuntime, sp_TaskRuntime *task);
SP_LOCAL_INLINE void SysPerf_ResetTraceCycleStatistic(uint8 taskIterator);
SP_LOCAL_INLINE void SysPerf_ResetHooksStatistic(uint8 taskIterator);

/*******************************************************************************************************************
*                                          USER API IMPLEMENTATION                                                 *
*******************************************************************************************************************/

/**
 * @brief Trace Tasks (Threads) & ISRs runtime statistic per SP-cycle.
 * //TODO: rewr later: This function needs to be called each 1 second.
 */
USER_CALLED void SysPerf_RuntimeOverallStatistic(void)
{
   SysPerf_CycleBackup();

#if (SP_CFG_TRACE_MAIN_STAT == SP_CFG_ON)
   SysPerf_TraceCycleStatistic();
#endif

#if (SP_CFG_TRACE_UPTIME == SP_CFG_ON)
   SysPerf_TraceUptimeStatistic();
#endif

#if (SP_CFG_TRACE_CPU_LOAD == SP_CFG_ON)
   SysPerf_TraceCpuUsage();
#endif

#if (SP_CFG_TRACE_HISTORY == SP_CFG_ON)
   SysPerf_History_TraceCollected();
#endif

   SysPerf_ResetRuntimeStatistic();
}

/*******************************************************************************************************************
*                                       SYSPERF HOOKS IMPLEMENTATION                                               *
*******************************************************************************************************************/

RTOS_CALLED void SysPerf_PreTaskHook(void)
{
   uint8  stateMask = 0U;
   uint8  taskIterator;
   uint32 taskId;
   uint64 timestamp = 0U;

   taskId = SysPerf_GetTaskId();
   taskIterator = SysPerf_GetTaskIterator(taskId);
   stateMask |= TASK_STATE_ENTER;

   if (TASK_IS_VALID(taskIterator))
   {
      timestamp = SysPerf_GetUptimeTimestamp();
      Hook_TasksStatistics[taskIterator].taskTimestamp = timestamp;
      stateMask |= TASK_STATE_HOOK_SUCCESS;
      // SP_TRACE_INFO("### SysPerf_PreTaskHook: data successfully written for %s thread", SysPerf_StartupData_GetTaskName(taskIterator));
   }
   else
   {
      stateMask |= TASK_STATE_INVALID_ID;
      // TODO: add collecting diagnostic data to history
      // SP_TRACE_WARN("### SysPerf_PreTaskHook | failed to write stat | taskIterator - %u | taskId - 0x%x", taskIterator, taskId);
   }
   SysPerf_History_Update(taskId, timestamp, stateMask);
}

RTOS_CALLED void SysPerf_PostTaskHook(void)
{
   uint8  stateMask = 0U;
   uint8  taskIterator;
   uint32 taskId;
   uint32 taskRuntime;
   uint64 timestamp = 0U;

   taskId = SysPerf_GetTaskId();
   taskIterator = SysPerf_GetTaskIterator(taskId);
   stateMask |= TASK_STATE_EXIT;

   if (TASK_IS_VALID(taskIterator))
   {
      timestamp = SysPerf_GetUptimeTimestamp();
      taskRuntime = SysPerf_GetTaskRuntime(taskIterator, timestamp);
      SysPerf_UpdateTaskStatistic(taskIterator, timestamp, taskRuntime);
      stateMask |= TASK_STATE_HOOK_SUCCESS;
   }
   else
   {
      stateMask |= TASK_STATE_INVALID_ID;
      // TODO: add collecting diagnostic data to history
      // SP_TRACE_WARN("### SysPerf_PostTaskHook | failed to write stat, taskIterator: %u, taskId 0x%x", taskIterator, taskId);
   }
   SysPerf_History_Update(taskId, timestamp, stateMask);
}

RTOS_CALLED void SysPerf_PreISRHook(void)
{
   uint64 timestamp;
   timestamp = SysPerf_GetUptimeTimestamp();
   Hook_IsrTotalStatistic.isrTimestamp = timestamp;

#if (SP_CFG_ISR_RUNTIME_DISTINGUISH == SP_CFG_OFF)
   SysPerf_History_Update(SP_ISR_ID_SIGN, timestamp, ISR_STATE_ENTER);
#endif /* SP_CFG_ISR_RUNTIME_DISTINGUISH */
}

RTOS_CALLED void SysPerf_PostISRHook(void)
{
   uint32 isrRuntime;
   uint64 timestamp;

   timestamp = SysPerf_GetUptimeTimestamp();
   isrRuntime = (uint32)SysPerf_GetTimestampDelta(timestamp, Hook_IsrTotalStatistic.isrTimestamp);
   SysPerf_UpdateISRsStatistic(0U, timestamp, isrRuntime);

#if (SP_CFG_ISR_RUNTIME_DISTINGUISH == SP_CFG_OFF)
   SysPerf_History_Update(SP_ISR_ID_SIGN, timestamp, ISR_STATE_EXIT);
#endif /* SP_CFG_ISR_RUNTIME_DISTINGUISH */
}

RTOS_CALLED void SysPerf_IdleTaskRunning(void)
{
   uint64 timestamp;
   uint8  stateMask = 0U;
   static uint32 taskId = 0x00000000;
   static bool unfilled = true;

   timestamp = SysPerf_GetUptimeTimestamp();

#if (SP_IDLE_COUNT > 1U)
   static uint8 taskIterator = 0U;
   static uint32 prevtaskId = 0U;
#endif

   if (unfilled)
   {
      taskId = SysPerf_GetTaskId();
#if (SP_IDLE_COUNT == 1U)
      Idle_TaskIds[0] = taskId;
      unfilled = false;
#elif (SP_IDLE_COUNT > 1U)
      if (taskId != prevtaskId)
      {
         Idle_TaskIds[taskIterator] = taskId;
         taskIterator++;
         prevtaskId = taskId;

         if (taskIterator == SP_IDLE_COUNT)
         {
            unfilled = false;
         }
      }
#endif /* SP_IDLE_COUNT */
   }

   stateMask |= IDLE_STATE_RUNNING;
   SysPerf_History_Update(SysPerf_GetTaskId(), timestamp, stateMask);
}

/*******************************************************************************************************************
*                                       USER API IMPLEMENTATION                                                    *
*******************************************************************************************************************/

USER_CALLED void SysPerf_PreSchedulerInit(void)
{
   SysPerf_History_Init();
   SysPerf_ResetRuntimeStatistic();
}

USER_CALLED void SysPerf_PostSchedulerInit(void)
{
   SysPerf_InitRTUConfiguration();
}

/*******************************************************************************************************************
*                                         LOCAL API IMPLEMENTATION                                                 *
*******************************************************************************************************************/

SP_LOCAL uint64 SysPerf_GetTimestampDelta(uint64 t1, uint64 t2)
{
   return (uint64)((t2 > t1) ? (t2 - t1) : (t1 - t2));
}

/**
 * @brief Calculate total runtime across all Tasks (Threads) running in the system (with Idle task(s))
 */
SP_LOCAL uint32 SysPerf_GetSystemRuntime(void)
{
   uint8 taskIterator;
   uint32 systemTotalRuntime = 0U;

   for (taskIterator = 0; taskIterator < SP_TASK_COUNT; taskIterator++)
   {
      systemTotalRuntime += TraceCycle_TasksStatistics[taskIterator].totalRuntime;
   }

   return systemTotalRuntime;
}

/**
 * @brief Calculate separately Idle task(s) total runtime
 */
SP_LOCAL uint32 SysPerf_GetIdleRuntime(void)
{
   uint8 taskIterator;
   uint32 idleTotalRuntime = 0U;

   for (uint8 i = 0; i < SP_IDLE_COUNT; i++)
   {
      taskIterator = SysPerf_GetTaskIterator(Idle_TaskIds[i]);
      idleTotalRuntime += TraceCycle_TasksStatistics[taskIterator].totalRuntime;
   }

   return idleTotalRuntime;
}

SP_LOCAL void SysPerf_TraceCpuUsage(void)
{
   uint32 timeslice;
   uint64 currTimestamp;
   static uint64 lastTimestamp = 0U;

   /* System Tasks (Threads) total runtime per timeslice */
   uint32 systemTotalRuntime;
   /* Percentage with application total runtime per timeslice */
   uint32 cpuLoadPercent;
   /* Percentage with idle total runtime per timeslice */
   uint32 idleTimePercent;
   /* Percentage for os resource runtime (scheduling etc) */
   uint32 osTimePercent;
   /* Idle task(s) total runtime per timeslice */
   uint32 idleTotalRuntime;
   /* Os total runtime per timeslice */
   uint32 osTotalRuntime;

   /* Calculate timeslice */
   currTimestamp = SysPerf_GetUptimeTimestamp();
   timeslice = (uint32)SysPerf_GetTimestampDelta(lastTimestamp, currTimestamp);
   lastTimestamp = currTimestamp;

   /* Calculate total runtime across all Tasks (Threads) running in the system (with Idle task(s)) */
   systemTotalRuntime = SysPerf_GetSystemRuntime();

   /* Calculate RTOS's resources total runtime */
   osTotalRuntime = timeslice - systemTotalRuntime;

   /* Calculate separately Idle task(s) total runtime */
   idleTotalRuntime = SysPerf_GetIdleRuntime();

   /* Calculate percentages */
   idleTimePercent = GET_CPU_PERCENT(timeslice, idleTotalRuntime);
   cpuLoadPercent = GET_CPU_PERCENT(timeslice, (systemTotalRuntime - idleTotalRuntime));
   osTimePercent = GET_CPU_PERCENT(timeslice, osTotalRuntime);

   SP_TRACE_INFO("# ################# CPU LOAD %d%% ### IDLE %d%% ### KERNEL %d%% ################## #\n", cpuLoadPercent, idleTimePercent, osTimePercent);
}

SP_LOCAL_INLINE void SysPerf_UpdateTaskStatistic(uint8 taskIterator, uint64 timestamp, uint32 taskRuntime)
{
   sp_TaskRuntime *taskPtr = &Hook_TasksStatistics[taskIterator];

   if (taskRuntime > taskPtr->maxRuntime)
   {
      taskPtr->maxRuntime = taskRuntime;
   }

   taskPtr->taskTimestamp = timestamp;
   taskPtr->totalRuntime += taskRuntime;
   taskPtr->taskSwitches++;

   SysPerf_UpdateUptimeStatistic(taskIterator, taskRuntime, taskPtr);
}

SP_LOCAL_INLINE void SysPerf_UpdateISRsStatistic(uint8 isrIterator, uint64 timestamp, uint32 isrRuntime)
{
#if (SP_CFG_ISR_RUNTIME_DISTINGUISH == SP_CFG_ON)
   // TODO: implement
   sp_IsrRuntime *isrPtr = &Hook_TasksStatistics[taskIterator];
#elif (SP_CFG_ISR_TOTAL_RUNTIME_ONLY == SP_CFG_ON)
   sp_IsrRuntime *isrPtr = &Hook_IsrTotalStatistic;
#endif

   if (isrRuntime > isrPtr->maxRuntime)
   {
      isrPtr->maxRuntime = isrRuntime;
   }

   isrPtr->isrTimestamp = timestamp;
   isrPtr->totalRuntime += isrRuntime;
   isrPtr->isrSwitches++;

   // SysPerf_UpdateUptimeStatistic(taskIterator, taskRuntime, taskPtr);
}

SP_LOCAL_INLINE void SysPerf_UpdateUptimeStatistic(uint8 taskIterator, uint32 taskRuntime, sp_TaskRuntime *taskPtr)
{
   Hook_UptimeStatistic.TasksRuntime[taskIterator] += taskRuntime;
   Hook_UptimeStatistic.MaxTasksRuntime[taskIterator] = taskPtr->maxRuntime;
   Hook_UptimeStatistic.TasksSwitches[taskIterator] += taskPtr->taskSwitches;
}

SP_LOCAL void SysPerf_IsrLock(unsigned int *key)
{
   *key = SP_ISR_LOCK();
}

SP_LOCAL void SysPerf_IsrUnlock(unsigned int key)
{
   SP_ISR_UNLOCK(key);
}

SP_LOCAL void SysPerf_TraceUptimeStatistic(void)
{
   // TODO: same for SysPerf_TraceCycleStatistic
}

SP_LOCAL void SysPerf_CycleBackup(void)
{
   unsigned int key;
   SysPerf_IsrLock(&key);
   memcpy(&TraceCycle_TasksStatistics, &Hook_TasksStatistics, sizeof(Hook_TasksStatistics));
   memcpy(&TraceCycle_UptimeStatistic, &Hook_UptimeStatistic, sizeof(Hook_UptimeStatistic));
   SysPerf_IsrUnlock(key);
}

SP_LOCAL void SysPerf_TraceCycleStatistic(void)
{
   uint64 timeslice;
   uint64 currTimestamp;
   static uint64 lastTimestamp;

   currTimestamp = SysPerf_GetUptimeTimestamp();
   timeslice = SysPerf_GetTimestampDelta(lastTimestamp, currTimestamp);
   lastTimestamp = currTimestamp;

   SP_TRACE_INFO("# #####################   1 Second System Perfomance   ####################### #");
   SP_TRACE_INFO("#                                                                              #");
   SP_TRACE_INFO("#   Task Id         Total runtime         Max runtime        Total Switches    #");

   for (uint8 taskIterator = 0; taskIterator < SP_TASK_COUNT; taskIterator++)
   {
      SP_TRACE_INFO("%s %6u %22u %19u %17u %10s",
                    "#", taskIterator, TraceCycle_TasksStatistics[taskIterator].totalRuntime,
                    TraceCycle_TasksStatistics[taskIterator].maxRuntime, TraceCycle_TasksStatistics[taskIterator].taskSwitches, "#");
   }
   SP_TRACE_INFO("# ############################################################################ #");
   SP_TRACE_INFO("# ### Calculations Timeslice: %llu\n", timeslice);
}

SP_LOCAL void SysPerf_ResetRuntimeStatistic(void)
{
   for (uint8 taskIterator = 0; taskIterator < SP_TASK_COUNT; taskIterator++)
   {
      SysPerf_ResetTraceCycleStatistic(taskIterator);
      SysPerf_ResetHooksStatistic(taskIterator);
   }
}

SP_LOCAL_INLINE void SysPerf_ResetTraceCycleStatistic(uint8 taskIterator)
{
   TraceCycle_TasksStatistics[taskIterator].maxRuntime = 0u;
   TraceCycle_TasksStatistics[taskIterator].maxRuntime = 0u;
   TraceCycle_TasksStatistics[taskIterator].taskSwitches = 0u;
   TraceCycle_TasksStatistics[taskIterator].totalRuntime = 0u;
}

SP_LOCAL_INLINE void SysPerf_ResetHooksStatistic(uint8 taskIterator)
{
   Hook_TasksStatistics[taskIterator].maxRuntime = 0u;
   Hook_TasksStatistics[taskIterator].maxRuntime = 0u;
   Hook_TasksStatistics[taskIterator].taskSwitches = 0u;
   Hook_TasksStatistics[taskIterator].totalRuntime = 0u;
}

SP_LOCAL uint32 SysPerf_GetTaskRuntime(uint8 taskIterator, uint32 timeStamp)
{
   uint32 taskRuntime;
   uint32 prevTaskTimeStamp;
   prevTaskTimeStamp = Hook_TasksStatistics[taskIterator].taskTimestamp;
   taskRuntime = (timeStamp > prevTaskTimeStamp) ? (timeStamp - prevTaskTimeStamp) : (prevTaskTimeStamp - timeStamp);
   return taskRuntime;
}

