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
 * @brief Intermediate storage for uptime statistic, reset within 1 trace cycle (window cycle).
 * @details All collected data later copied to window_uptime_statistic.
 */
NP_LOCAL np_uptime_statistic_t       hook_uptime_statistic;

/**
 * @brief Intermediate storage for Tasks (Threads) runtime statistic, reset within 1 trace cycle (window cycle).
 * @details All collected data later copied to window_tasks_statistic.
 */
NP_LOCAL np_task_runtime             hook_tasks_statistics[NP_TASK_COUNT];


/**
 * @brief Idle task runtime metadata.
 */
NP_LOCAL uint8_t                    idle_task_iterator = NP_UNINIT;
NP_LOCAL uint32_t                   idle_task_id       = NP_UNINIT;

/**
 * @brief Store a copy of uptime statistic when nrf_perf_cycle_backup() routine occurs.
 * @details Used to trace all collected information to avoid race condition in rescheduling points.
 */
NP_LOCAL np_uptime_statistic_t       window_uptime_statistic;

/**
 * @brief Store a copy of Tasks (Threads) runtime statistic when nrf_perf_cycle_backup() routine occurs.
 * @details Used to trace all collected information to avoid race condition in rescheduling points.
 */
NP_LOCAL np_task_runtime             window_tasks_statistic[NP_TASK_COUNT];

/*******************************************************************************************************************
*                                             nrf_perf LOCAL API                                                    *
*******************************************************************************************************************/
NP_LOCAL uint32_t nrf_perf_get_task_runtime(uint8_t task_iterator, uint32_t timestamp);
NP_LOCAL uint32_t nrf_perf_get_timestamp_delta(uint32_t, uint32_t);
NP_LOCAL uint32_t nrf_perf_get_system_runtime(void);
NP_LOCAL uint32_t nrf_perf_get_idle_runtime(void);

NP_LOCAL void nrf_perf_cycle_backup(void);

NP_LOCAL void nrf_perf_isr_lock(unsigned int *key);
NP_LOCAL void nrf_perf_isr_unlock(unsigned int key);
NP_LOCAL void nrf_perf_reset_runtime_statistic(void);

NP_LOCAL void nrf_perf_trace_cycle_statistic(uint32_t *);
NP_LOCAL void nrf_perf_trace_cpu_usage(uint32_t *);
NP_LOCAL void nrf_perf_trace_uptime_statistic(void);

NP_LOCAL_INLINE void nrf_perf_update_isrs_statistic(uint8_t isrIterator, uint32_t timestamp, uint32_t isrRuntime);
NP_LOCAL_INLINE void nrf_perf_update_task_statistic(uint8_t task_iterator, uint32_t timestamp, uint32_t task_runtime);
NP_LOCAL_INLINE void nrf_perf_update_idle_statistic(uint32_t timestamp);
NP_LOCAL_INLINE void nrf_perf_update_uptime_statistic(uint8_t task_iterator, uint32_t task_runtime, np_task_runtime *task);

NP_LOCAL_INLINE void nrf_perf_reset_trace_cycle_statistic(uint8_t task_iterator);
NP_LOCAL_INLINE void nrf_perf_reset_hooks_statistic(uint8_t task_iterator);

/*******************************************************************************************************************
*                                          USER API IMPLEMENTATION                                                 *
*******************************************************************************************************************/

/**
 * @brief Trace Tasks (Threads) & ISRs runtime statistic per window cycle.
 *
 */
NP_USER_CALLED void nrf_perf_runtime_overall_statistic(void)
{
   uint32_t time_slice = 0;
   uint32_t curr_timestamp = 0;
   static uint32_t last_timestamp = 0U;

   nrf_perf_cycle_backup();

   /* Calculate time_slice */
   curr_timestamp = nrf_perf_get_uptime_timestamp();
   time_slice = nrf_perf_get_timestamp_delta(last_timestamp, curr_timestamp);
   last_timestamp = curr_timestamp;

#if (NP_CFG_TRACE_MAIN_STAT == NP_CFG_ON)
   nrf_perf_trace_cycle_statistic(&time_slice);
#endif

#if (NP_CFG_TRACE_UPTIME == NP_CFG_ON)
   nrf_perf_trace_uptime_statistic();
#endif

#if (NP_CFG_TRACE_CPU_LOAD == NP_CFG_ON)
   nrf_perf_trace_cpu_usage(&time_slice);
#endif

#if (NP_CFG_TRACE_HISTORY == NP_CFG_ON)
   nrf_perf_history_trace_collected();
#endif

   nrf_perf_reset_runtime_statistic();
   nrf_perf_history_clear();
}

/*******************************************************************************************************************
*                                       nrf_perf HOOKS IMPLEMENTATION                                               *
*******************************************************************************************************************/

NP_RTOS_CALLED void nrf_perf_pre_task_hook(void)
{
   uint32_t timestamp = nrf_perf_get_uptime_timestamp();
   uint32_t task_id = nrf_perf_get_task_id();
   uint8_t task_iterator = nrf_perf_get_task_iterator(task_id);

   if (TASK_IS_VALID(task_iterator))
   {
      hook_tasks_statistics[task_iterator].task_timestamp = timestamp;

      if (IDLE_STATE_RUNNING == nrf_perf_history_get_last_state())
      {
         nrf_perf_update_idle_statistic(timestamp);
      }

      nrf_perf_history_update(task_id, timestamp, TASK_STATE_ENTER);
   }
   else
   {
      // TODO: add collecting diagnostic data to history
   }
}

NP_RTOS_CALLED void nrf_perf_post_task_hook(void)
{
   uint8_t  task_iterator = 0;
   uint32_t task_id = 0;
   uint32_t task_runtime = 0;
   uint32_t timestamp = 0;

   task_id = nrf_perf_get_task_id();
   task_iterator = nrf_perf_get_task_iterator(task_id);

   if (TASK_IS_VALID(task_iterator))
   {
      timestamp = nrf_perf_get_uptime_timestamp();
      task_runtime = nrf_perf_get_task_runtime(task_iterator, timestamp);
      nrf_perf_update_task_statistic(task_iterator, timestamp, task_runtime);

      nrf_perf_history_update(task_id, timestamp, TASK_STATE_EXIT);
   }
   else
   {
      // TODO: add collecting diagnostic data to history
   }
}

NP_RTOS_CALLED void nrf_perf_pre_isr_hook(void)
{
   uint32_t timestamp = 0;

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
      NP_TRACE_INFO("Currently running in ISR number: %u\n", isr_num);
   }
   else
   {
      NP_TRACE_INFO("Currently running in thread context.");
   }
*/
}

NP_RTOS_CALLED void nrf_perf_post_isr_hook(void)
{
   uint32_t timestamp = nrf_perf_get_uptime_timestamp();
   nrf_perf_history_update(0x000, timestamp, ISR_STATE_EXIT);
}

NP_RTOS_CALLED void nrf_perf_idle_task_running(void)
{
   static bool idle_not_determined = true;

   if (idle_not_determined)
   {
      idle_task_id = nrf_perf_get_task_id();
      idle_task_iterator = nrf_perf_get_task_iterator(idle_task_id);
      idle_not_determined = false;
   }

   if (idle_task_iterator != NP_UNINIT && idle_task_id != NP_UNINIT)
   {
      nrf_perf_history_update(idle_task_id, nrf_perf_get_uptime_timestamp(), IDLE_STATE_RUNNING);
   }
}

/*******************************************************************************************************************
*                                       USER API IMPLEMENTATION                                                    *
*******************************************************************************************************************/

NP_USER_CALLED void nrf_perf_pre_scheduler_init(void)
{
   nrf_perf_history_init();
   nrf_perf_reset_runtime_statistic();
}

NP_USER_CALLED void nrf_perf_post_scheduler_init(void)
{
   nrf_perf_init_configuration();
}

NP_USER_CALLED void nrf_perf_trace_history(void)
{
   nrf_perf_history_trace_collected();
}

/*******************************************************************************************************************
*                                         LOCAL API IMPLEMENTATION                                                 *
*******************************************************************************************************************/

NP_LOCAL uint32_t nrf_perf_get_timestamp_delta(uint32_t timestamp1, uint32_t timestamp2)
{
   uint32_t delta = 0;

   if (timestamp2 >= timestamp1)
   {
      delta = timestamp2 - timestamp1;
   }
   else
   {
      /* In case of type overflow */
      delta = (NP_TIMESTAMP_TOP - timestamp1) + timestamp2 + 1;
   }

   return delta;
}

/**
 * @brief Calculate total runtime across all Tasks (Threads) running in the system (with Idle task(s))
 */
NP_LOCAL uint32_t nrf_perf_get_system_runtime(void)
{
   uint8_t task_iterator = 0;
   uint32_t system_total_runtime = 0U;

   for (task_iterator = 0; task_iterator < NP_TASK_COUNT; task_iterator++)
   {
      system_total_runtime += window_tasks_statistic[task_iterator].total_runtime;
   }

   return system_total_runtime;
}

/**
 * @brief Calculate separately Idle task(s) total runtime
 */
NP_LOCAL uint32_t nrf_perf_get_idle_runtime(void)
{
   uint32_t idle_total_runtime = 0U;

   if (NP_UNINIT != idle_task_iterator)
   {
      idle_total_runtime = window_tasks_statistic[idle_task_iterator].total_runtime;
   }

   return idle_total_runtime;
}

NP_LOCAL void nrf_perf_trace_cpu_usage(uint32_t *time_slice)
{
   /* System Tasks (Threads) total runtime per time_slice */
   uint32_t system_total_runtime;
   /* Percentage with application total runtime per time_slice */
   uint8_t cpu_load_per;
   /* Percentage with idle total runtime per time_slice */
   uint8_t idle_time_per;
   /* Percentage for os resource runtime (scheduling etc) */
   uint8_t os_per;
   /* Idle task(s) total runtime per time_slice */
   uint32_t idle_total_runtime;
   /* Os total runtime per time_slice */
   uint32_t os_total_runtime;

   /* Calculate separately Idle task(s) total runtime */
   idle_total_runtime = nrf_perf_get_idle_runtime();

   /* Calculate total runtime across all Tasks (Threads) running in the system, except Idle task */
   system_total_runtime = nrf_perf_get_system_runtime() - idle_total_runtime;

   /* Calculate RTOS's resources total runtime */
   os_total_runtime = (*time_slice) - (system_total_runtime + idle_total_runtime);

   /* Calculate percentages */
   idle_time_per = (uint8_t)GET_CPU_PERCENT((*time_slice), idle_total_runtime);
   cpu_load_per = (uint8_t)GET_CPU_PERCENT((*time_slice), system_total_runtime);
   os_per = (uint8_t)GET_CPU_PERCENT((*time_slice), os_total_runtime);

   NP_TRACE_INFO("# ################# CPU LOAD %d%% ### IDLE %d%% ### KERNEL %d%% ################## #\n", cpu_load_per, idle_time_per, os_per);
}

NP_LOCAL_INLINE void nrf_perf_update_task_statistic(uint8_t task_iterator, uint32_t timestamp, uint32_t task_runtime)
{
   np_task_runtime *task_ptr = &hook_tasks_statistics[task_iterator];

   if (task_runtime > task_ptr->max_runtime)
   {
      task_ptr->max_runtime = task_runtime;
   }

   task_ptr->task_timestamp = timestamp;
   task_ptr->total_runtime += task_runtime;
   task_ptr->task_switches++;

   nrf_perf_update_uptime_statistic(task_iterator, task_runtime, task_ptr);
}

NP_LOCAL_INLINE void nrf_perf_update_idle_statistic(uint32_t timestamp)
{
   uint32_t idle_runtime = nrf_perf_get_task_runtime(idle_task_iterator, timestamp);
   nrf_perf_update_task_statistic(idle_task_iterator, timestamp, idle_runtime);
}

NP_LOCAL_INLINE void nrf_perf_update_isrs_statistic(uint8_t isrIterator, uint32_t timestamp, uint32_t isrRuntime)
{

}

NP_LOCAL_INLINE void nrf_perf_update_uptime_statistic(uint8_t task_iterator, uint32_t task_runtime, np_task_runtime *task_ptr)
{
   hook_uptime_statistic.tasks_runtime[task_iterator] += task_runtime;
   hook_uptime_statistic.max_tasks_runtime[task_iterator] = task_ptr->max_runtime;
   hook_uptime_statistic.tasks_switches[task_iterator] += task_ptr->task_switches;
}

NP_LOCAL void nrf_perf_isr_lock(unsigned int *key)
{
   *key = NP_ISR_LOCK();
}

NP_LOCAL void nrf_perf_isr_unlock(unsigned int key)
{
   NP_ISR_UNLOCK(key);
}

NP_LOCAL void nrf_perf_trace_uptime_statistic(void)
{
   // TODO: same for nrf_perf_trace_cycle_statistic
}

NP_LOCAL void nrf_perf_cycle_backup(void)
{
   unsigned int key;
   nrf_perf_isr_lock(&key);
   memcpy(&window_tasks_statistic, &hook_tasks_statistics, sizeof(hook_tasks_statistics));
   memcpy(&window_uptime_statistic, &hook_uptime_statistic, sizeof(hook_uptime_statistic));
   nrf_perf_isr_unlock(key);
}

NP_LOCAL void nrf_perf_trace_cycle_statistic(uint32_t *time_slice)
{
   NP_TRACE_INFO("# #####################   1 Second System Performance   ###################### #");
   NP_TRACE_INFO("#                                                                              #");
   NP_TRACE_INFO("#   Task Id         Total runtime         Max runtime        Total Switches    #");

   // TODO: add support of formatting names using k_thread_name_get((k_tid_t)thread);
   for (uint8_t task_iterator = 0; task_iterator < NP_TASK_COUNT; task_iterator++)
   {
      NP_TRACE_INFO("%s %6u %22u %19u %17u %10s",
                    "#", task_iterator, window_tasks_statistic[task_iterator].total_runtime,
                    window_tasks_statistic[task_iterator].max_runtime, window_tasks_statistic[task_iterator].task_switches, "#");
   }
   NP_TRACE_INFO("# ############################################################################ #");
   NP_TRACE_INFO("# ### Calculations Timeslice: %u\n", (*time_slice));
}

NP_LOCAL void nrf_perf_reset_runtime_statistic(void)
{
   for (uint8_t task_iterator = 0; task_iterator < NP_TASK_COUNT; task_iterator++)
   {
      nrf_perf_reset_trace_cycle_statistic(task_iterator);
      nrf_perf_reset_hooks_statistic(task_iterator);
   }
}

NP_LOCAL_INLINE void nrf_perf_reset_trace_cycle_statistic(uint8_t task_iterator)
{
   window_tasks_statistic[task_iterator].max_runtime = 0u;
   window_tasks_statistic[task_iterator].max_runtime = 0u;
   window_tasks_statistic[task_iterator].task_switches = 0u;
   window_tasks_statistic[task_iterator].total_runtime = 0u;
}

NP_LOCAL_INLINE void nrf_perf_reset_hooks_statistic(uint8_t task_iterator)
{
   hook_tasks_statistics[task_iterator].max_runtime = 0u;
   hook_tasks_statistics[task_iterator].max_runtime = 0u;
   hook_tasks_statistics[task_iterator].task_switches = 0u;
   hook_tasks_statistics[task_iterator].total_runtime = 0u;
}

NP_LOCAL uint32_t nrf_perf_get_task_runtime(uint8_t task_iterator, uint32_t timestamp)
{
   uint32_t prev_timestamp = hook_tasks_statistics[task_iterator].task_timestamp;
   uint32_t task_runtime = nrf_perf_get_timestamp_delta(prev_timestamp, timestamp);
   return task_runtime;
}
