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
#include "History.h"
#include "SysPerf_DataTypes.h"
#include "SysPerf_RTOS_Wrapper.h"

/*******************************************************************************************************************
 *                                    MAIN SYSPERF STATISTIC ENTITIES                                              *
 *******************************************************************************************************************/

/**
 * @brief Intermediate storage for uptime statistic, reseted within 1 trace cycle.
 * @details All collected data later copied to TraceCycle_UptimeStatistic.
 */
SP_LOCAL sp_UptimeStat_t Hook_UptimeStatistic;

/**
 * @brief Intermediate storage for tasks runtime statistic, reseted within 1 trace cycle.
 * @details All collected data later copied to TraceCycle_TasksStatistics.
 */
SP_LOCAL sp_TaskRuntime Hook_TasksStatistics[SP_TASK_COUNT];

/**
 * @brief Store configuration information of all tasks, used as reference.
 */
SP_LOCAL sp_TaskConfigInfo Tasks_ConfigInfo[SP_TASK_COUNT];

/**
 * @brief Store a copy of uptime statistic when SysPerf_CycleBackup() routine occurs.
 * @details Used to trace all collected information to avoid race condition in rescheduling points.
 */
SP_LOCAL sp_UptimeStat_t TraceCycle_UptimeStatistic;

/**
 * @brief Store a copy of tasks runtime statistic when SysPerf_CycleBackup() routine occurs.
 * @details Used to trace all collected information to avoid race condition in rescheduling points.
 */
SP_LOCAL sp_TaskRuntime TraceCycle_TasksStatistics[SP_TASK_COUNT];

/*******************************************************************************************************************
 *                                             SYSPERF LOCAL API                                                   *
 *******************************************************************************************************************/
SP_LOCAL uint32 SysPerf_GetTaskRuntime(uint8 taskIterator, uint32 timeStamp);
SP_LOCAL uint64 SysPerf_GetTimestampDelta(uint64, uint64);
SP_LOCAL bool SysPerf_CheckTaskId(uint8 taskIterator, unsigned int *taskId);

SP_LOCAL void SysPerf_CycleBackup(void);

SP_LOCAL void SysPerf_IsrLock(unsigned int *key);
SP_LOCAL void SysPerf_IsrUnlock(unsigned int key);
SP_LOCAL void SysPerf_ResetRuntimeStatistic(void);

SP_LOCAL void SysPerf_TraceCycleStatistic(void);
SP_LOCAL void SysPerf_TraceUptimeStatistic(void);
SP_LOCAL void SysPerf_TraceCpuUsage(void);

SP_LOCAL_INLINE void SysPerf_UpdateTaskStatistic(uint8 taskIterator, uint64 timestamp, uint32 taskRuntime);
SP_LOCAL_INLINE void SysPerf_UpdateUptimeStatistic(uint8 taskIterator, uint32 taskRuntime, sp_TaskRuntime *task);
SP_LOCAL_INLINE void SysPerf_ResetTraceCycleStatistic(uint8 taskIterator);
SP_LOCAL_INLINE void SysPerf_ResetHooksStatistic(uint8 taskIterator);

/*******************************************************************************************************************
 *                                          USER API IMPLEMENTATION                                                *
 *******************************************************************************************************************/

/**
 * @brief Trace Tasks & ISRs runtime statistic per 1 second (cycle). This function need to be called each 1 second.
 */
SP_USER_CALLED void SysPerf_RuntimeOverallStatistic(void)
{
   SysPerf_CycleBackup();

   SysPerf_TraceCycleStatistic();
   // SysPerf_TraceUptimeStatistic();
   // SysPerf_TraceCpuUsage();
   // History_TraceCollected();
   SysPerf_ResetRuntimeStatistic();
}

/*******************************************************************************************************************
 *                                       SYSPERF HOOKS IMPLEMENTATION                                              *
 *******************************************************************************************************************/

SP_RTOS_CALLED void SysPerf_PreTaskHook(void)
{
   uint8 taskIterator;
   uint64 timestamp;
   unsigned int taskId;

   taskIterator = SysPerf_GetTaskIterator();

   // if (TASK_IS_VALID(taskIterator) && SysPerf_CheckTaskId(taskIterator))
   if (TASK_IS_VALID(taskIterator))
   {
      if (SysPerf_CheckTaskId(taskIterator, &taskId))
      {
         timestamp = SysPerf_GetUptimeTimestamp();
         Hook_TasksStatistics[taskIterator].taskTimestamp = timestamp;
         History_Update(taskIterator, timestamp, TASK_STATE_ENTER, PRE_TASK_HOOK_EV);
      }
      else
      {
         // SP_TRACE_INFO("### SysPerf_PreTaskHook -> invalid taskId: 0x%x", taskId);
      }
   }
   else
   {
      SP_TRACE_INFO("### SysPerf_PreTaskHook -> invalid taskIterator: %d", taskIterator);
   }
}

SP_RTOS_CALLED void SysPerf_PostTaskHook(void)
{
   uint8 taskIterator;
   uint64 timestamp;
   uint32 taskRuntime;
   unsigned int taskId;

   taskIterator = SysPerf_GetTaskIterator();

   // if (TASK_IS_VALID(taskIterator) && SysPerf_CheckTaskId(taskIterator))
   if (TASK_IS_VALID(taskIterator))
   {
      // if (TASK_STATE_INTERRUPTED != History_GetLastEntityState(taskIterator))
      if (SysPerf_CheckTaskId(taskIterator, &taskId))
      {
         timestamp = SysPerf_GetUptimeTimestamp();
         taskRuntime = SysPerf_GetTaskRuntime(taskIterator, timestamp);
         SysPerf_UpdateTaskStatistic(taskIterator, timestamp, taskRuntime);
         History_Update(taskIterator, timestamp, TASK_STATE_EXIT, POST_TASK_HOOK_EV);
      }
      else
      {
         // SP_TRACE_INFO("### SysPerf_PostTaskHook -> invalid taskId: 0x%x", taskId);
      }
   }
   else
   {
      SP_TRACE_INFO("### SysPerf_PostTaskHook -> invalid taskIterator: %d", taskIterator);
   }
}

SP_RTOS_CALLED void SysPerf_PreISRHook(void)
{
   // TODO: add support to measure ISR runtime
}

SP_RTOS_CALLED void SysPerf_PostISRHook(void)
{
   // TODO: add support to measure ISR runtime
}

SP_RTOS_CALLED void SysPerf_IdleTaskRunning(void)
{
   // SP_TRACE_INFO("###SysPerf_IdleTaskRunning: %s", __func__);
}

/*******************************************************************************************************************
 *                                       USER API IMPLEMENTATION                                                   *
 *******************************************************************************************************************/

SP_USER_CALLED void SysPerf_InitTaskData(uint8 taskIterator, unsigned int taskId, const char *taskName, uint8 taskPrio, uint32 taskStackSize)
{
   if (TASK_IS_VALID(taskIterator))
   {
      Tasks_ConfigInfo[taskIterator].priority = taskPrio;
      Tasks_ConfigInfo[taskIterator].stackSize = taskStackSize;
      Tasks_ConfigInfo[taskIterator].taskName = taskName;
      Tasks_ConfigInfo[taskIterator].taskId = taskId;
   }
}

SP_USER_CALLED void SysPerf_PreSchedulerInit(void)
{
   History_Init();
   SysPerf_ResetRuntimeStatistic();
}

/*******************************************************************************************************************
 *                                         LOCAL API IMPLEMENTATION                                                *
 *******************************************************************************************************************/

SP_LOCAL uint64 SysPerf_GetTimestampDelta(uint64 t1, uint64 t2)
{
   return (uint64)((t2 > t1) ? (t2 - t1) : (t1 - t2));
}

SP_LOCAL void SysPerf_TraceCpuUsage(void)
{
   uint32 timeslice;
   uint64 currTimestamp;
   static uint64 lastTimestamp;

   /* Percentage with application total runtime per timeslice */
   uint32 cpuLoad;
   /* Percentage with idle total runtime per timeslice */
   uint32 idleTime;
   /* Application tasks total runtime per timeslice */
   uint32 appTotalRuntime = 0U;
   /* Idle task(s) total runtime per timeslice */
   uint32 idleTotalRuntime = 0U;

   currTimestamp = SysPerf_GetUptimeTimestamp();
   timeslice = (uint32)SysPerf_GetTimestampDelta(lastTimestamp, currTimestamp);
   lastTimestamp = currTimestamp;

   for (uint8 taskIterator = 0; taskIterator < SP_TASK_NUM; taskIterator++)
   {
      appTotalRuntime += TraceCycle_TasksStatistics[taskIterator].totalRuntime;
   }

   cpuLoad = GET_CPULOAD(timeslice, appTotalRuntime);
   idleTotalRuntime = timeslice - appTotalRuntime; // TODO: rewrite with SysPerf_IdleTaskRunning routine calculations
   idleTime = GET_IDLETIME(timeslice, idleTotalRuntime);
   SP_TRACE_INFO("# ######### CPU LOAD %d%% ### IDLE %d%% ######### #\n", cpuLoad, idleTime);
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

   for (uint8 taskIterator = 0; taskIterator < SP_TASK_NUM; taskIterator++)
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
   for (uint8 taskIterator = 0; taskIterator < SP_TASK_NUM; taskIterator++)
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

SP_LOCAL bool SysPerf_CheckTaskId(uint8 taskIterator, unsigned int *taskId)
{
   *taskId = SysPerf_GetTaskId();
   return (bool)(Tasks_ConfigInfo[taskIterator].taskId == (*taskId));
}

