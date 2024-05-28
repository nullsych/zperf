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
 *                                          SYSPERF STAT LOCALS                                                    *
 *******************************************************************************************************************/
SP_LOCAL sp_UptimeStat_t Hook_UptimeStatistic;
SP_LOCAL sp_TaskRuntime Hook_TasksStatistics[SP_TASK_COUNT];

SP_LOCAL sp_UptimeStat_t TraceCycle_UptimeStatistic;
SP_LOCAL sp_TaskRuntime TraceCycle_TasksStatistics[SP_TASK_COUNT];

SP_LOCAL sp_HookHistory Hook_History;

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
SP_LOCAL_INLINE void SysPerf_UpdateTaskStatistic(uint8 taskId, uint32 timestamp, uint32 taskRuntime);
SP_LOCAL_INLINE void SysPerf_UpdateUptimeStatistic(uint8 taskId, uint32 taskRuntime, sp_TaskRuntime *task);
SP_LOCAL_INLINE void SysPerf_ResetTraceCycleStatistic(uint8 taskId);
SP_LOCAL_INLINE void SysPerf_ResetHooksStatistic(uint8 taskId);
SP_LOCAL_INLINE uint8 SysPerf_HistoryInit(void);
SP_LOCAL_INLINE uint8 SysPerf_HistoryUpdate(uint8, uint8, uint8);
SP_LOCAL_INLINE uint8 SysPerf_HistoryGetLastState(void);
SP_LOCAL_INLINE uint8 SysPerf_HistoryGetLastTask(void);
SP_LOCAL_INLINE uint32 SysPerf_HistoryGetLastTimestamp(void);


/*******************************************************************************************************************
 *                                       SYSPERF HOOKS IMPLEMENTATION                                              *
 *******************************************************************************************************************/

SP_RTOS_CALLED void SysPerf_PreTaskHook(sp_TaskPtr task)
{
   uint32 timestamp;
   uint8 taskId;

   taskId = SysPerf_GetTaskId();

   if (IS_TASK_VALID(taskId))
   {
      timestamp = SysPerf_GetUptimeTimestamp();
      Hook_TasksStatistics[taskId].taskTimestamp = timestamp;
      Hook_History.history[taskId].state = SP_TASK_ENTER;
   }
   /*
       k_thread_runtime_stats_t rt_stats_thread;
       k_thread_runtime_stats_get(k_current_get(), &rt_stats_thread);
       printk("Cycles: %llu\n", rt_stats_thread.execution_cycles);
   */
}

SP_RTOS_CALLED void SysPerf_PostTaskHook(sp_TaskPtr task)
{
   uint32 timestamp;
   uint32 taskRuntime;
   uint8 taskId;

   taskId = SysPerf_GetTaskId();

   if (IS_TASK_VALID(taskId))
   {
      if (SP_TASK_INTERRUPTED != Hook_History.history[taskId].state)
      {
         timestamp = SysPerf_GetUptimeTimestamp();
         taskRuntime = SysPerf_GetTaskRuntime(taskId, timestamp);
         SysPerf_UpdateTaskStatistic(taskId, timestamp, taskRuntime);
         Hook_History.history[taskId].state = SP_TASK_EXIT;
      }
   }
}

SP_RTOS_CALLED void SysPerf_PreISRHook(void)
{
   uint32 timestamp;
   uint32 taskRuntime;
   uint8 taskId;

   taskId = SysPerf_HistoryGetLastTask();

   /* trigger if last task was interrupted */
   if ((IS_TASK_VALID(taskId)) && (SP_TASK_ENTER == Hook_History.history[taskId].state))
   {
      Hook_History.history[taskId].state = SP_TASK_INTERRUPTED;

      timestamp = SysPerf_GetUptimeTimestamp();
      taskRuntime = SysPerf_GetTaskRuntime(taskId, timestamp);

      SysPerf_UpdateTaskStatistic(taskId, timestamp, taskRuntime);
   }

   // TODO: measure ISR also
}

SP_RTOS_CALLED void SysPerf_PostISRHook(void)
{
   // TODO: measure ISR also
}

SP_RTOS_CALLED void SysPerf_IdleRaskRunning(void)
{
   // TODO: measure Idle also
}

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
   SysPerf_ResetRuntimeStatistic();
}

/*******************************************************************************************************************
 *                                         LOCAL API IMPLEMENTATION                                                *
 *******************************************************************************************************************/

SP_LOCAL_INLINE void SysPerf_UpdateTaskStatistic(uint8 taskId, uint32 timestamp, uint32 taskRuntime)
{
   sp_TaskRuntime *task = &Hook_TasksStatistics[taskId];

   if (taskRuntime > task->maxRuntime)
   {
      task->maxRuntime = taskRuntime;
   }

   task->taskTimestamp = timestamp;
   task->totalRuntime += taskRuntime;
   task->taskSwitches++;

   SysPerf_UpdateUptimeStatistic(taskId, taskRuntime, task);
}

SP_LOCAL_INLINE void SysPerf_UpdateUptimeStatistic(uint8 taskId, uint32 taskRuntime, sp_TaskRuntime *task)
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

SP_LOCAL_INLINE uint8 SysPerf_HistoryGetLastState(void)
{
   return Hook_History.history[Hook_History.idx].state;
}

SP_LOCAL_INLINE uint32 SysPerf_HistoryGetLastTimestamp(void)
{
   return Hook_History.history[Hook_History.idx].timestamp;
}

SP_LOCAL_INLINE uint8 SysPerf_HistoryGetLastTask(void)
{
   return Hook_History.history[Hook_History.idx].taskId;
}

SP_LOCAL_INLINE uint8 SysPerf_HistoryInit(void)
{
   Hook_History.idx = 0U;
   Hook_History.size = 0U;
}

SP_LOCAL uint8 SysPerf_HistoryUpdate(uint8 state, uint8 taskId, uint8 timestamp)
{
   Hook_History.history[Hook_History.idx].state = state;
   Hook_History.history[Hook_History.idx].taskId = taskId;
   Hook_History.history[Hook_History.idx].timestamp = timestamp;

   Hook_History.idx++;
   Hook_History.idx %= SP_HISTORY_SIZE;
   Hook_History.size = (Hook_History.size < SP_HISTORY_SIZE) ? (Hook_History.size + 1) : SP_HISTORY_SIZE;
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
