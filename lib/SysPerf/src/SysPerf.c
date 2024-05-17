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
#include <zephyr.h>
#include <stdlib.h>
#include <logging/log.h>
LOG_MODULE_REGISTER(SysPerf, LOG_LEVEL_DBG);

/*******************************************************************************************************************
 *                                             SYSPERF HEADERS                                                      *
 *******************************************************************************************************************/
#include "SysPerf.h"

/*******************************************************************************************************************
 *                                             SYSPERF LOCAL API                                                    *
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
SP_LOCAL_INLINE void SysPerf_UpdateTaskStatistic(uint8 taskId, uint32 timestamp, uint32 taskRuntime);
SP_LOCAL_INLINE void SysPerf_UpdateUptimeStatistic(uint8 taskId, uint32 taskRuntime, sp_TaskRuntime_t *task);
SP_LOCAL_INLINE void SysPerf_ResetTraceCycleStatistic(uint8 taskId);
SP_LOCAL_INLINE void SysPerf_ResetHooksStatistic(uint8 taskId);

/*******************************************************************************************************************
 *                                          SYSPERF STAT LOCALS                                                    *
 *******************************************************************************************************************/
SP_LOCAL sp_UptimeStat_t Hook_UptimeStatistic;
SP_LOCAL sp_TaskRuntime_t Hook_TasksStatistics[SYSPERF_TASK_NUM];

SP_LOCAL sp_UptimeStat_t TraceCycle_UptimeStatistic;
SP_LOCAL sp_TaskRuntime_t TraceCycle_TasksStatistics[SYSPERF_TASK_NUM];

/*******************************************************************************************************************
 *                                       SYSPERF HOOKS IMPLEMENTATION                                               *
 *******************************************************************************************************************/

void SysPerf_PreTaskHook(sp_TaskPtr task)
{
   uint32 timestamp;
   uint8 taskId;

   taskId = SysPerf_GetTaskId();

   if (taskId < SYSPERF_TASK_NUM)
   {
      timestamp = SysPerf_GetUptimeTimestamp();
      Hook_TasksStatistics[taskId].taskTimestamp = timestamp;
   }
   /*
       k_thread_runtime_stats_t rt_stats_thread;
       k_thread_runtime_stats_get(k_current_get(), &rt_stats_thread);
       printk("Cycles: %llu\n", rt_stats_thread.execution_cycles);
   */
}

void SysPerf_PostTaskHook(sp_TaskPtr task)
{
   uint32 timestamp, taskRuntime;
   uint8 taskId;

   taskId = SysPerf_GetTaskId();

   if (taskId < SYSPERF_TASK_NUM)
   {
      timestamp = SysPerf_GetUptimeTimestamp();
      taskRuntime = SysPerf_GetTaskRuntime(taskId, timestamp);

      SysPerf_UpdateTaskStatistic(taskId, timestamp, taskRuntime);
   }
}

/*******************************************************************************************************************
 *                                       SYSPERF API IMPLEMENTATION                                                *
 *******************************************************************************************************************/

/**
 * @brief Trace Tasks & ISRs runtime statistic per 1 second (cycle). This function need to be called each 1 second.
 */
void SysPerf_RuntimeOverallStatistic(void)
{
   SysPerf_CycleBackup();

   SysPerf_TraceCycleStatistic();
   SysPerf_TraceUptimeStatistic();

   SysPerf_ResetRuntimeStatistic();
}

SP_LOCAL_INLINE void SysPerf_UpdateTaskStatistic(uint8 taskId, uint32 timestamp, uint32 taskRuntime)
{
   sp_TaskRuntime_t *task = &Hook_TasksStatistics[taskId];

   if (taskRuntime > task->maxRuntime)
   {
      task->maxRuntime = taskRuntime;
   }

   task->taskTimestamp = timestamp;
   task->totalRuntime += taskRuntime;
   task->taskSwitches++;

   SysPerf_UpdateUptimeStatistic(taskId, taskRuntime, task);
}

SP_LOCAL_INLINE void SysPerf_UpdateUptimeStatistic(uint8 taskId, uint32 taskRuntime, sp_TaskRuntime_t *task)
{
   Hook_UptimeStatistic.TotalTasksRuntime[taskId] += taskRuntime;
   Hook_UptimeStatistic.MaxTasksRuntime[taskId] = task->maxRuntime;
   Hook_UptimeStatistic.TotalTasksSwitches[taskId] += task->taskSwitches;
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

   for (uint8 taskId = 0; taskId < SYSPERF_TASK_NUM; taskId++)
   {
      SP_TRACE_INFO("#     %2u             %10u          %10u        %10u          #",
                    taskId, TraceCycle_TasksStatistics[taskId].totalRuntime,
                    TraceCycle_TasksStatistics[taskId].maxRuntime, TraceCycle_TasksStatistics[taskId].taskSwitches);
   }
   SP_TRACE_INFO("# ############################################################################ #\n");
}

SP_LOCAL void SysPerf_ResetRuntimeStatistic(void)
{
   for (uint8 taskId = 0; taskId < SYSPERF_TASK_NUM; taskId++)
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
   taskRuntime = (timeStamp > prevTaskTimeStamp) ? (timeStamp - prevTaskTimeStamp) : (timeStamp + (MAP_RTOS_UPTIME_TOP - prevTaskTimeStamp));
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
