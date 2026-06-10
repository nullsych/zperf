/**
 * @file   nrf_perf.c
 * @brief  Contain main implementation of nrf_perf functionality.
 * @author nullsych
 * @date   May 6, 2024
 *
 */

/*******************************************************************************************************************
 *                                             RTOS-SPECIFIC HEADERS                                                *
 *******************************************************************************************************************/
#include <stdlib.h>
#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(nrf_perf, LOG_LEVEL_DBG);

/*******************************************************************************************************************
*                                             nrf_perf HEADERS                                                      *
*******************************************************************************************************************/
#include "nrf_perf.h"
#include "nrf_perf_history.h"
#include "wrapper.h"
#include "nrf_perf_cfg.cfg"

/*******************************************************************************************************************
*                                    MAIN nrf_perf STATISTIC ENTITIES                                               *
*******************************************************************************************************************/

/**
 * @brief Intermediate storage for uptime statistic, reset within 1 trace cycle (SP-cycle).
 * @details All collected data later copied to TraceCycle_UptimeStatistic.
 */
SPK_LOCAL sp_UptimeStatistic_t       Hook_UptimeStatistic;

/**
 * @brief Intermediate storage for Tasks (Threads) runtime statistic, reset within 1 trace cycle (SP-cycle).
 * @details All collected data later copied to TraceCycle_TasksStatistics.
 */
SPK_LOCAL sp_TaskRuntime             Hook_TasksStatistics[SP_TASK_COUNT];


/**
 * @brief Idle task runtime metadata.
 */
SPK_LOCAL uint8_t                    Idle_TaskIterator = SP_UNINIT;
SPK_LOCAL uint32_t                   Idle_TaskId       = SP_UNINIT;

/**
 * @brief Store a copy of uptime statistic when nrf_perf_cycle_backup() routine occurs.
 * @details Used to trace all collected information to avoid race condition in rescheduling points.
 */
SPK_LOCAL sp_UptimeStatistic_t       TraceCycle_UptimeStatistic;

/**
 * @brief Store a copy of Tasks (Threads) runtime statistic when nrf_perf_cycle_backup() routine occurs.
 * @details Used to trace all collected information to avoid race condition in rescheduling points.
 */
SPK_LOCAL sp_TaskRuntime             TraceCycle_TasksStatistics[SP_TASK_COUNT];

/*******************************************************************************************************************
*                                             nrf_perf LOCAL API                                                    *
*******************************************************************************************************************/
SPK_LOCAL uint32_t nrf_perf_get_task_runtime(uint8_t taskIterator, uint32_t timeStamp);
SPK_LOCAL uint32_t nrf_perf_get_timestamp_delta(uint32_t, uint32_t);
SPK_LOCAL uint32_t nrf_perf_get_system_runtime(void);
SPK_LOCAL uint32_t nrf_perf_get_idle_runtime(void);

SPK_LOCAL void nrf_perf_cycle_backup(void);

SPK_LOCAL void nrf_perf_isr_lock(unsigned int *key);
SPK_LOCAL void nrf_perf_isr_unlock(unsigned int key);
SPK_LOCAL void nrf_perf_reset_runtime_statistic(void);

SPK_LOCAL void nrf_perf_trace_cycle_statistic(uint32_t *);
SPK_LOCAL void nrf_perf_trace_cpu_usage(uint32_t *);
SPK_LOCAL void nrf_perf_trace_uptime_statistic(void);

SPK_LOCAL_INLINE void nrf_perf_update_isrs_statistic(uint8_t isrIterator, uint32_t timestamp, uint32_t isrRuntime);
SPK_LOCAL_INLINE void nrf_perf_update_task_statistic(uint8_t taskIterator, uint32_t timestamp, uint32_t taskRuntime);
SPK_LOCAL_INLINE void nrf_perf_update_idle_statistic(uint32_t timestamp);
SPK_LOCAL_INLINE void nrf_perf_update_uptime_statistic(uint8_t taskIterator, uint32_t taskRuntime, sp_TaskRuntime *task);

SPK_LOCAL_INLINE void nrf_perf_reset_trace_cycle_statistic(uint8_t taskIterator);
SPK_LOCAL_INLINE void nrf_perf_reset_hooks_statistic(uint8_t taskIterator);

/*******************************************************************************************************************
*                                          USER API IMPLEMENTATION                                                 *
*******************************************************************************************************************/

/**
 * @brief Trace Tasks (Threads) & ISRs runtime statistic per SP-cycle.
 *
 */
USER_CALLED void nrf_perf_runtime_overall_statistic(void)
{
   uint32_t timeslice;
   uint32_t currTimestamp;
   static uint32_t lastTimestamp = 0U;

   nrf_perf_cycle_backup();

   /* Calculate timeslice */
   currTimestamp = nrf_perf_get_uptime_timestamp();
   timeslice = nrf_perf_get_timestamp_delta(lastTimestamp, currTimestamp);
   lastTimestamp = currTimestamp;

#if (SP_CFG_TRACE_MAIN_STAT == SP_CFG_ON)
   nrf_perf_trace_cycle_statistic(&timeslice);
#endif

#if (SP_CFG_TRACE_UPTIME == SP_CFG_ON)
   nrf_perf_trace_uptime_statistic();
#endif

#if (SP_CFG_TRACE_CPU_LOAD == SP_CFG_ON)
   nrf_perf_trace_cpu_usage(&timeslice);
#endif

#if (SP_CFG_TRACE_HISTORY == SP_CFG_ON)
   nrf_perf_history_trace_collected();
#endif

   nrf_perf_reset_runtime_statistic();
   nrf_perf_history_clear();
}

/*******************************************************************************************************************
*                                       nrf_perf HOOKS IMPLEMENTATION                                               *
*******************************************************************************************************************/

RTOS_CALLED void nrf_perf_pre_task_hook(void)
{
   uint8_t  taskIterator;
   uint32_t taskId;
   uint32_t timestamp = 0U;

   timestamp = nrf_perf_get_uptime_timestamp();
   taskId = nrf_perf_get_task_id();
   taskIterator = nrf_perf_get_task_iterator(taskId);


   if (TASK_IS_VALID(taskIterator))
   {
      Hook_TasksStatistics[taskIterator].taskTimestamp = timestamp;

      if (IDLE_STATE_RUNNING == nrf_perf_history_get_last_state())
      {
         nrf_perf_update_idle_statistic(timestamp);
      }

      nrf_perf_history_update(taskId, timestamp, TASK_STATE_ENTER);
   }
   else
   {
      // TODO: add collecting diagnostic data to history
   }
}

RTOS_CALLED void nrf_perf_post_task_hook(void)
{
   uint8_t  taskIterator;
   uint32_t taskId;
   uint32_t taskRuntime;
   uint32_t timestamp = 0U;

   taskId = nrf_perf_get_task_id();
   taskIterator = nrf_perf_get_task_iterator(taskId);

   if (TASK_IS_VALID(taskIterator))
   {
      timestamp = nrf_perf_get_uptime_timestamp();
      taskRuntime = nrf_perf_get_task_runtime(taskIterator, timestamp);
      nrf_perf_update_task_statistic(taskIterator, timestamp, taskRuntime);

      nrf_perf_history_update(taskId, timestamp, TASK_STATE_EXIT);
   }
   else
   {
      // TODO: add collecting diagnostic data to history
   }
}

RTOS_CALLED void nrf_perf_pre_isr_hook(void)
{
   uint32_t timestamp;

   timestamp = nrf_perf_get_uptime_timestamp();

   if (IDLE_STATE_RUNNING == nrf_perf_history_get_last_state())
   {
      nrf_perf_update_idle_statistic(timestamp);
   }

   nrf_perf_history_update(0x000, timestamp, ISR_STATE_ENTER);

// TODO: add nrf_perf_get_isr_id support
/*
   uint32_t isr_num = nrf_perf_get_isr_id();

   if (isr_num != 0)
   {
      SP_TRACE_INFO("Currently running in ISR number: %u\n", isr_num);
   }
   else
   {
      SP_TRACE_INFO("Currently running in thread context.");
   }
*/
}

RTOS_CALLED void nrf_perf_post_isr_hook(void)
{
   uint32_t timestamp;

   timestamp = nrf_perf_get_uptime_timestamp();

   nrf_perf_history_update(0x000, timestamp, ISR_STATE_EXIT);
}

RTOS_CALLED void nrf_perf_idle_task_running(void)
{
   static bool idleNotDetermined = true;

   if (idleNotDetermined)
   {
      Idle_TaskId = nrf_perf_get_task_id();
      Idle_TaskIterator = nrf_perf_get_task_iterator(Idle_TaskId);
      idleNotDetermined = false;
   }

   if (Idle_TaskIterator != SP_UNINIT && Idle_TaskId != SP_UNINIT)
   {
      nrf_perf_history_update(Idle_TaskId, nrf_perf_get_uptime_timestamp(), IDLE_STATE_RUNNING);
   }
}

/*******************************************************************************************************************
*                                       USER API IMPLEMENTATION                                                    *
*******************************************************************************************************************/

USER_CALLED void nrf_perf_pre_scheduler_init(void)
{
   nrf_perf_history_init();
   nrf_perf_reset_runtime_statistic();
}

USER_CALLED void nrf_perf_post_scheduler_init(void)
{
   nrf_perf_init_configuration();
}

USER_CALLED void nrf_perf_trace_history(void)
{
   nrf_perf_history_trace_collected();
}

/*******************************************************************************************************************
*                                         LOCAL API IMPLEMENTATION                                                 *
*******************************************************************************************************************/

SPK_LOCAL uint32_t nrf_perf_get_timestamp_delta(uint32_t timestamp1, uint32_t timestamp2)
{
   uint32_t delta;
   if (timestamp2 >= timestamp1)
   {
      delta = timestamp2 - timestamp1;
   }
   else
   {
      /* In case of type overflow */
      delta = (SP_TIMESTAMP_TOP - timestamp1) + timestamp2 + 1;
   }

   return delta;
}

/**
 * @brief Calculate total runtime across all Tasks (Threads) running in the system (with Idle task(s))
 */
SPK_LOCAL uint32_t nrf_perf_get_system_runtime(void)
{
   uint8_t taskIterator;
   uint32_t systemTotalRuntime = 0U;

   for (taskIterator = 0; taskIterator < SP_TASK_COUNT; taskIterator++)
   {
      systemTotalRuntime += TraceCycle_TasksStatistics[taskIterator].totalRuntime;
   }

   return systemTotalRuntime;
}

/**
 * @brief Calculate separately Idle task(s) total runtime
 */
SPK_LOCAL uint32_t nrf_perf_get_idle_runtime(void)
{
   uint32_t idleTotalRuntime = 0U;

   if (SP_UNINIT != Idle_TaskIterator)
   {
      idleTotalRuntime = TraceCycle_TasksStatistics[Idle_TaskIterator].totalRuntime;
   }

   return idleTotalRuntime;
}

SPK_LOCAL void nrf_perf_trace_cpu_usage(uint32_t *timeslice)
{
   /* System Tasks (Threads) total runtime per timeslice */
   uint32_t systemTotalRuntime;
   /* Percentage with application total runtime per timeslice */
   uint8_t cpuLoadPercent;
   /* Percentage with idle total runtime per timeslice */
   uint8_t idleTimePercent;
   /* Percentage for os resource runtime (scheduling etc) */
   uint8_t osTimePercent;
   /* Idle task(s) total runtime per timeslice */
   uint32_t idleTotalRuntime;
   /* Os total runtime per timeslice */
   uint32_t osTotalRuntime;

   /* Calculate separately Idle task(s) total runtime */
   idleTotalRuntime = nrf_perf_get_idle_runtime();

   /* Calculate total runtime across all Tasks (Threads) running in the system, except Idle task */
   systemTotalRuntime = nrf_perf_get_system_runtime() - idleTotalRuntime;

   /* Calculate RTOS's resources total runtime */
   osTotalRuntime = (*timeslice) - (systemTotalRuntime + idleTotalRuntime);

   /* Calculate percentages */
   idleTimePercent = (uint8_t)GET_CPU_PERCENT((*timeslice), idleTotalRuntime);
   cpuLoadPercent = (uint8_t)GET_CPU_PERCENT((*timeslice), systemTotalRuntime);
   osTimePercent = (uint8_t)GET_CPU_PERCENT((*timeslice), osTotalRuntime);

   SP_TRACE_INFO("# ################# CPU LOAD %d%% ### IDLE %d%% ### KERNEL %d%% ################## #\n", cpuLoadPercent, idleTimePercent, osTimePercent);
}

SPK_LOCAL_INLINE void nrf_perf_update_task_statistic(uint8_t taskIterator, uint32_t timestamp, uint32_t taskRuntime)
{
   sp_TaskRuntime *taskPtr = &Hook_TasksStatistics[taskIterator];

   if (taskRuntime > taskPtr->maxRuntime)
   {
      taskPtr->maxRuntime = taskRuntime;
   }

   taskPtr->taskTimestamp = timestamp;
   taskPtr->totalRuntime += taskRuntime;
   taskPtr->taskSwitches++;

   nrf_perf_update_uptime_statistic(taskIterator, taskRuntime, taskPtr);
}

SPK_LOCAL_INLINE void nrf_perf_update_idle_statistic(uint32_t timestamp)
{
   uint32_t idleRuntime;
   idleRuntime = nrf_perf_get_task_runtime(Idle_TaskIterator, timestamp);
   nrf_perf_update_task_statistic(Idle_TaskIterator, timestamp, idleRuntime);
}

SPK_LOCAL_INLINE void nrf_perf_update_isrs_statistic(uint8_t isrIterator, uint32_t timestamp, uint32_t isrRuntime)
{

}

SPK_LOCAL_INLINE void nrf_perf_update_uptime_statistic(uint8_t taskIterator, uint32_t taskRuntime, sp_TaskRuntime *taskPtr)
{
   Hook_UptimeStatistic.TasksRuntime[taskIterator] += taskRuntime;
   Hook_UptimeStatistic.MaxTasksRuntime[taskIterator] = taskPtr->maxRuntime;
   Hook_UptimeStatistic.TasksSwitches[taskIterator] += taskPtr->taskSwitches;
}

SPK_LOCAL void nrf_perf_isr_lock(unsigned int *key)
{
   *key = SP_ISR_LOCK();
}

SPK_LOCAL void nrf_perf_isr_unlock(unsigned int key)
{
   SP_ISR_UNLOCK(key);
}

SPK_LOCAL void nrf_perf_trace_uptime_statistic(void)
{
   // TODO: same for nrf_perf_trace_cycle_statistic
}

SPK_LOCAL void nrf_perf_cycle_backup(void)
{
   unsigned int key;
   nrf_perf_isr_lock(&key);
   memcpy(&TraceCycle_TasksStatistics, &Hook_TasksStatistics, sizeof(Hook_TasksStatistics));
   memcpy(&TraceCycle_UptimeStatistic, &Hook_UptimeStatistic, sizeof(Hook_UptimeStatistic));
   nrf_perf_isr_unlock(key);
}

SPK_LOCAL void nrf_perf_trace_cycle_statistic(uint32_t *timeslice)
{
   SP_TRACE_INFO("# #####################   1 Second System Performance   ###################### #");
   SP_TRACE_INFO("#                                                                              #");
   SP_TRACE_INFO("#   Task Id         Total runtime         Max runtime        Total Switches    #");

   // TODO: add support of formatting names using k_thread_name_get((k_tid_t)thread);
   for (uint8_t taskIterator = 0; taskIterator < SP_TASK_COUNT; taskIterator++)
   {
      SP_TRACE_INFO("%s %6u %22u %19u %17u %10s",
                    "#", taskIterator, TraceCycle_TasksStatistics[taskIterator].totalRuntime,
                    TraceCycle_TasksStatistics[taskIterator].maxRuntime, TraceCycle_TasksStatistics[taskIterator].taskSwitches, "#");
   }
   SP_TRACE_INFO("# ############################################################################ #");
   SP_TRACE_INFO("# ### Calculations Timeslice: %u\n", (*timeslice));
}

SPK_LOCAL void nrf_perf_reset_runtime_statistic(void)
{
   for (uint8_t taskIterator = 0; taskIterator < SP_TASK_COUNT; taskIterator++)
   {
      nrf_perf_reset_trace_cycle_statistic(taskIterator);
      nrf_perf_reset_hooks_statistic(taskIterator);
   }
}

SPK_LOCAL_INLINE void nrf_perf_reset_trace_cycle_statistic(uint8_t taskIterator)
{
   TraceCycle_TasksStatistics[taskIterator].maxRuntime = 0u;
   TraceCycle_TasksStatistics[taskIterator].maxRuntime = 0u;
   TraceCycle_TasksStatistics[taskIterator].taskSwitches = 0u;
   TraceCycle_TasksStatistics[taskIterator].totalRuntime = 0u;
}

SPK_LOCAL_INLINE void nrf_perf_reset_hooks_statistic(uint8_t taskIterator)
{
   Hook_TasksStatistics[taskIterator].maxRuntime = 0u;
   Hook_TasksStatistics[taskIterator].maxRuntime = 0u;
   Hook_TasksStatistics[taskIterator].taskSwitches = 0u;
   Hook_TasksStatistics[taskIterator].totalRuntime = 0u;
}

SPK_LOCAL uint32_t nrf_perf_get_task_runtime(uint8_t taskIterator, uint32_t timeStamp)
{
   uint32_t taskRuntime;
   uint32_t prevTaskTimeStamp;
   prevTaskTimeStamp = Hook_TasksStatistics[taskIterator].taskTimestamp;
   taskRuntime = nrf_perf_get_timestamp_delta(prevTaskTimeStamp, timeStamp);
   return taskRuntime;
}
