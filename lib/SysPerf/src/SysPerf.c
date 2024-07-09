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

/*******************************************************************************************************************
 *                                    MAIN SYSPERF STATISTIC ENTITIES                                              *
 *******************************************************************************************************************/
SP_LOCAL sp_TotalStat_t Hook_UptimeStatistic;
SP_LOCAL sp_TaskRuntime Hook_TasksStatistics[SP_TASK_COUNT];

SP_LOCAL sp_TotalStat_t TraceCycle_UptimeStatistic;
SP_LOCAL sp_TaskRuntime TraceCycle_TasksStatistics[SP_TASK_COUNT];

/*******************************************************************************************************************
 *                                             SYSPERF LOCAL API                                                   *
 *******************************************************************************************************************/
SP_LOCAL uint32 SysPerf_GetTaskRuntime(uint8 taskId, uint32 timeStamp);
SP_LOCAL uint32 SysPerf_GetUptimeTimestamp(void);
SP_LOCAL uint8 SysPerf_GetTaskId(void);
SP_LOCAL void SysPerf_CycleBackup(void);
SP_LOCAL void SysPerf_IsrLock(unsigned int *key);
SP_LOCAL void SysPerf_IsrUnlock(unsigned int key);
SP_LOCAL void SysPerf_ResetRuntimeStatistic(void);
SP_LOCAL void SysPerf_TraceCycleStatistic(void);
SP_LOCAL void SysPerf_TraceUptimeStatistic(void);
SP_LOCAL void SysPerf_TraceCpuUsage(void);
SP_LOCAL_INLINE void SysPerf_UpdateTaskStatistic(uint8 taskId, uint32 timestamp, uint32 taskRuntime);
SP_LOCAL_INLINE void SysPerf_UpdateUptimeStatistic(uint8 taskId, uint32 taskRuntime, sp_TaskRuntime *task);
SP_LOCAL_INLINE void SysPerf_ResetTraceCycleStatistic(uint8 taskId);
SP_LOCAL_INLINE void SysPerf_ResetHooksStatistic(uint8 taskId);

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
   SysPerf_TraceUptimeStatistic();
   // SysPerf_TraceCpuUsage();
   History_TraceCollected();
   SysPerf_ResetRuntimeStatistic();
}

/*******************************************************************************************************************
 *                                       SYSPERF HOOKS IMPLEMENTATION                                              *
 *******************************************************************************************************************/

SP_RTOS_CALLED void SysPerf_PreTaskHook(void)
{
   uint8 taskId;
   uint32 timestamp;

   taskId = SysPerf_GetTaskId();

   if (TASK_IS_VALID(taskId))
   {
      timestamp = SysPerf_GetUptimeTimestamp();
      Hook_TasksStatistics[taskId].taskTimestamp = timestamp;
      History_Update(taskId, timestamp, TASK_STATE_ENTER, PRE_TASK_HOOK_EV);
   }
}

SP_RTOS_CALLED void SysPerf_PostTaskHook(void)
{
   uint32 timestamp;
   uint32 taskRuntime;
   uint8 taskId;

   taskId = SysPerf_GetTaskId();

   if (TASK_IS_VALID(taskId))
   {
      if (TASK_STATE_INTERRUPTED != History_GetLastEntityState(taskId))
      {
         timestamp = SysPerf_GetUptimeTimestamp();
         taskRuntime = SysPerf_GetTaskRuntime(taskId, timestamp);
         SysPerf_UpdateTaskStatistic(taskId, timestamp, taskRuntime);
         // History_Update(taskId, timestamp, TASK_STATE_EXIT, POST_TASK_HOOK_EV);
      }
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

}

SP_USER_CALLED void SysPerf_PreSchedulerInit(void)
{
   History_Init();
   SysPerf_ResetRuntimeStatistic();
}

/*******************************************************************************************************************
 *                                         LOCAL API IMPLEMENTATION                                                *
 *******************************************************************************************************************/

SP_LOCAL uint32 SysPerf_GetTimestampDelta(uint32 t1, uint32 t2)
{
   return (uint32)((t2 > t1) ? (t2 - t1) : (t2 + (SP_TIMESTAMP_TOP - t1)));
}

SP_LOCAL void SysPerf_TraceCpuUsage(void)
{

}

SP_LOCAL_INLINE void SysPerf_UpdateTaskStatistic(uint8 taskId, uint32 timestamp, uint32 taskRuntime)
{
   sp_TaskRuntime *taskPtr = &Hook_TasksStatistics[taskId];

   if (taskRuntime > taskPtr->maxRuntime)
   {
      taskPtr->maxRuntime = taskRuntime;
   }

   taskPtr->taskTimestamp = timestamp;
   taskPtr->totalRuntime += taskRuntime;
   taskPtr->taskSwitches++;

   SysPerf_UpdateUptimeStatistic(taskId, taskRuntime, taskPtr);
}

SP_LOCAL_INLINE void SysPerf_UpdateUptimeStatistic(uint8 taskId, uint32 taskRuntime, sp_TaskRuntime *taskPtr)
{
   Hook_UptimeStatistic.TotalTasksRuntime[taskId] += taskRuntime;
   Hook_UptimeStatistic.MaxTasksRuntime[taskId] = taskPtr->maxRuntime;
   Hook_UptimeStatistic.TotalTasksSwitches[taskId] += taskPtr->taskSwitches;
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
   SP_TRACE_INFO("# #####################   1 Second System Perfomance   ####################### #");
   SP_TRACE_INFO("#                                                                              #");
   SP_TRACE_INFO("#   Task Id         Total runtime         Max runtime        Total Switches    #");

   for (uint8 taskId = 0; taskId < SP_TASK_NUM; taskId++)
   {
      SP_TRACE_INFO("%s %6u %22u %19u %17u %10s",
                    "#", taskId, TraceCycle_TasksStatistics[taskId].totalRuntime,
                    TraceCycle_TasksStatistics[taskId].maxRuntime, TraceCycle_TasksStatistics[taskId].taskSwitches, "#");
   }
   SP_TRACE_INFO("# ############################################################################ #\n");
}

SP_LOCAL void SysPerf_ResetRuntimeStatistic(void)
{
   for (uint8 taskId = 0; taskId < SP_TASK_NUM; taskId++)
   {
      SysPerf_ResetTraceCycleStatistic(taskId);
      SysPerf_ResetHooksStatistic(taskId);
   }
}

SP_LOCAL_INLINE void SysPerf_ResetTraceCycleStatistic(uint8 taskId)
{
   TraceCycle_TasksStatistics[taskId].maxRuntime = 0u;
   TraceCycle_TasksStatistics[taskId].maxRuntime = 0u;
   TraceCycle_TasksStatistics[taskId].taskSwitches = 0u;
   TraceCycle_TasksStatistics[taskId].totalRuntime = 0u;
}

SP_LOCAL_INLINE void SysPerf_ResetHooksStatistic(uint8 taskId)
{
   Hook_TasksStatistics[taskId].maxRuntime = 0u;
   Hook_TasksStatistics[taskId].maxRuntime = 0u;
   Hook_TasksStatistics[taskId].taskSwitches = 0u;
   Hook_TasksStatistics[taskId].totalRuntime = 0u;
}

SP_LOCAL uint32 SysPerf_GetTaskRuntime(uint8 taskId, uint32 timeStamp)
{
   uint32 taskRuntime, prevTaskTimeStamp;
   prevTaskTimeStamp = Hook_TasksStatistics[taskId].taskTimestamp;
   taskRuntime = (timeStamp > prevTaskTimeStamp) ? (timeStamp - prevTaskTimeStamp) : (timeStamp + (SP_TIMESTAMP_TOP - prevTaskTimeStamp));
   return taskRuntime;
}

SP_LOCAL uint32 SysPerf_GetUptimeTimestamp(void)
{
   uint32 timeStamp;
   timeStamp = k_uptime_ticks();
   timeStamp = k_ticks_to_us_near32((uint32)timeStamp);
   return timeStamp; /* in uS*/
}

SP_LOCAL uint8 SysPerf_GetTaskId()
{
   return (uint8)k_thread_custom_data_get();
}
