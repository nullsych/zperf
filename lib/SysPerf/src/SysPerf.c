/*******************************************************************************************************************
 *                                             RTOS-SPECIFIC HEADERS                                                *
 *******************************************************************************************************************/
#include <zephyr.h>
// #define LOG_LEVEL LOG_LEVEL_DBG
// #define LOG_MODULE_NAME SysPerf

// #ifdef LOG_LEVEL
// #undef LOG_LEVEL
// #define LOG_LEVEL LOG_LEVEL_DBG
// #endif
#include <logging/log.h>
LOG_MODULE_REGISTER(SysPerf, LOG_LEVEL_DBG);

/*******************************************************************************************************************
 *                                             SYSPERF HEADERS                                                      *
 *******************************************************************************************************************/
#include "SysPerf.h"

/*******************************************************************************************************************
 *                                             SYSPERF LOCAL API                                                    *
 *******************************************************************************************************************/
static uint32 SysPerf__GetTaskRuntime(uint8 taskId, uint32 timeStamp);
static uint32 SysPerf__GetUptimeTimestamp(void);
static uint8 SysPerf__GetTaskId(void);

static sp_EpochStat_t EpochStatistic;
static sp_UptimeStat_t UptimeStatistic;

/*******************************************************************************************************************
 *                                       SYSPERF HOOKS IMPLEMENTATION                                               *
 *******************************************************************************************************************/

void SysPerf__PreTaskHook(sp_TaskPtr task)
{
   uint32 preTaskTimestamp_uS;
   uint8 taskId;

   taskId = SysPerf__GetTaskId();

   // SP_TRACE_INFO("Task Name: %s\n", task->name);
   // SP_TRACE_INFO("TaskId: %d\n", taskId);

   if (taskId < SYSPERF_TASK_NUM)
   {
      preTaskTimestamp_uS = SysPerf__GetUptimeTimestamp();
      EpochStatistic.TasksRuntime[taskId].taskTimestamp = preTaskTimestamp_uS;
   }
   /*
       k_thread_runtime_stats_t rt_stats_thread;
       k_thread_runtime_stats_get(k_current_get(), &rt_stats_thread);
       printk("Cycles: %llu\n", rt_stats_thread.execution_cycles);
   */
}

void SysPerf__PostTaskHook(sp_TaskPtr task)
{
   uint32 postTaskTimestamp_uS, taskRuntime_uS;
   uint8 taskId;

   taskId = SysPerf__GetTaskId();

   // SP_TRACE_INFO("Task Name: %s\n", task->name);
   // SP_TRACE_INFO("TaskId: %d\n", taskId);

   if (taskId < SYSPERF_TASK_NUM)
   {
      postTaskTimestamp_uS = SysPerf__GetUptimeTimestamp();
      taskRuntime_uS = SysPerf__GetTaskRuntime(taskId, postTaskTimestamp_uS);

      EpochStatistic.TasksRuntime[taskId].taskTimestamp = postTaskTimestamp_uS;
      EpochStatistic.TasksRuntime[taskId].totalRuntime += taskRuntime_uS;
      EpochStatistic.TasksRuntime[taskId].taskSwitches++;

      // SP_TRACE_INFO("# ====== Task name: %s, Timestamp = %u [uS], Runtime = %u [uS]\n", task->name, postTaskTimestamp_uS, taskRuntime_uS);

      UptimeStatistic.TotalTasksRuntime[taskId] += taskRuntime_uS;
   }
}

static uint32 SysPerf__GetTaskRuntime(uint8 taskId, uint32 timeStamp)
{
   uint32 taskRuntime, prevTaskTimeStamp;
   prevTaskTimeStamp = EpochStatistic.TasksRuntime[taskId].taskTimestamp;
   taskRuntime = (timeStamp > prevTaskTimeStamp) ? (timeStamp - prevTaskTimeStamp) : (timeStamp + (MAP_RTOS_UPTIME_TOP - prevTaskTimeStamp));
   return taskRuntime;
}

static uint32 SysPerf__GetUptimeTimestamp(void)
{
   uint32 timeStamp;
   timeStamp = k_uptime_ticks();
   timeStamp = k_ticks_to_us_near32((uint32)timeStamp);
   return timeStamp; /* in uS*/
}

static uint8 SysPerf__GetTaskId()
{
   return (uint8)k_thread_custom_data_get();
}

/**
 * @brief Trace Tasks & ISRs runtime statistic per 1 second (epoch). This function need to be called each 1 second.
 */
void SysPerf__EpochOverallStatistic(void)
{
   uint32 timeStamp;

   timeStamp = SysPerf__GetUptimeTimestamp();

   SP_TRACE_INFO("# #####################   1 Second System Perfomance   ####################### #");
   SP_TRACE_INFO("#   Task Id         Total runtime         Max runtime        Total Switches    #");

   // SP_TRACE_INFO("#     %3u                %6u         Max runtime        Total Switches    #");

   for (uint8 taskId = 0; taskId < 10; taskId++)
   {
      // SP_TRACE_INFO("#      %-4u      #       %14u      #       %12u      #          %8u        #", 

      // SP_TRACE_INFO("# "
                  //   11, 123, 524,
                  //   565);

                  //   taskId, EpochStatistic.TasksRuntime[taskId].totalRuntime,
                  //   EpochStatistic.TasksRuntime[taskId].totalRuntime,
                  //   EpochStatistic.TasksRuntime[taskId].taskSwitches);
   }
   SP_TRACE_INFO("# ############################################################################ #\n");
}
