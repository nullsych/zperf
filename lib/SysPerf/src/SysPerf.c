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
SP_LOCAL_INLINE void SysPerf_UpdateTaskStat(uint8 taskId, uint32 timestamp, uint32 taskRuntime);
/*******************************************************************************************************************
 *                                          SYSPERF STAT LOCALS                                                    *
 *******************************************************************************************************************/
SP_LOCAL sp_UptimeStat_t UptimeStatistic;
SP_LOCAL sp_TaskRuntime_t TasksRuntime[SYSPERF_TASK_NUM];
// SP_LOCAL sp_IsrRuntime_t IsrRuntime[SYSPERF_ISR_NUM];

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
      TasksRuntime[taskId].taskTimestamp = timestamp;
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

      SysPerf_UpdateTaskStat(taskId, timestamp, taskRuntime);

      // if (taskRuntime > TasksRuntime[taskId].maxRuntime)
      // {
      //    TasksRuntime[taskId].maxRuntime = taskRuntime;
      // }

      // TasksRuntime[taskId].taskTimestamp = timestamp;
      // TasksRuntime[taskId].totalRuntime += taskRuntime;
      // TasksRuntime[taskId].taskSwitches++;

      UptimeStatistic.TotalTasksRuntime[taskId] += taskRuntime;
   }
}

/**
 * @brief Trace Tasks & ISRs runtime statistic per 1 second (epoch). This function need to be called each 1 second.
 */
void SysPerf_EpochOverallStatistic(void)
{
   SP_TRACE_INFO("# #####################   1 Second System Perfomance   ####################### #");
   SP_TRACE_INFO("#                                                                              #");
   SP_TRACE_INFO("#   Task Id         Total runtime         Max runtime        Total Switches    #");

   for (uint8 taskId = 0; taskId < SYSPERF_TASK_NUM; taskId++)
   {
      SP_TRACE_INFO("#     %2u           %10u            %10u          %10u        #",
                    taskId, UptimeStatistic.TotalTasksRuntime[taskId],
                    TasksRuntime[taskId].maxRuntime, TasksRuntime[taskId].taskSwitches);
   }
   SP_TRACE_INFO("# ############################################################################ #\n");

   // TODO: reset stat!!!
}

SP_LOCAL_INLINE void SysPerf_UpdateTaskStat(uint8 taskId, uint32 timestamp, uint32 taskRuntime)
{
   sp_TaskRuntime_t *task = &TasksRuntime[taskId];

   if (taskRuntime > task->maxRuntime)
   {
      task->maxRuntime = taskRuntime;
   }

   task->taskTimestamp = timestamp;
   task->totalRuntime += taskRuntime;
   task->taskSwitches++;
}

SP_LOCAL uint32 SysPerf_GetTaskRuntime(uint8 taskId, uint32 timeStamp)
{
   uint32 taskRuntime, prevTaskTimeStamp;
   prevTaskTimeStamp = TasksRuntime[taskId].taskTimestamp;
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
