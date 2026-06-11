/**
 * @file   zperf.c
 * @brief  Contain main implementation of zperf functionality.
 * @author nullsych
 * @date   May 6, 2024
 *
 */

/* clang-format off */
/*******************************************************************************************************************
 *                                             RTOS-SPECIFIC HEADERS                                               *
 *******************************************************************************************************************/
#include <stdlib.h>
#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(zperf, LOG_LEVEL_DBG);

/*******************************************************************************************************************
 *                                             zperf HEADERS                                                    *
 *******************************************************************************************************************/
#include "zperf_internal.h"
#include "zperf_history.h"
#include "wrapper.h"
#include "zperf.cfg"

/*******************************************************************************************************************
 *                                    MAIN zperf STATISTIC ENTITIES                                             *
 *******************************************************************************************************************/

/**
 * @brief Intermediate storage for uptime statistic, reset within 1 trace cycle (window cycle).
 * @details All collected data later copied to window_uptime_statistic.
 */
ZP_LOCAL zp_uptime_statistic_t       hook_uptime_statistic;

/**
 * @brief Intermediate storage for Tasks (Threads) runtime statistic, reset within 1 trace cycle (window cycle).
 * @details All collected data later copied to window_tasks_statistic.
 */
ZP_LOCAL zp_task_runtime             hook_tasks_statistics[ZP_TASK_COUNT];


/**
 * @brief Idle task runtime metadata.
 */
ZP_LOCAL uint8_t                    idle_task_iterator = ZP_UNINIT;
ZP_LOCAL uint32_t                   idle_task_id       = ZP_UNINIT;

/**
 * @brief Store a copy of uptime statistic when zperf_cycle_backup() routine occurs.
 * @details Used to trace all collected information to avoid race condition in rescheduling points.
 */
ZP_LOCAL zp_uptime_statistic_t       window_uptime_statistic;

/**
 * @brief Store a copy of Tasks (Threads) runtime statistic when zperf_cycle_backup() routine occurs.
 * @details Used to trace all collected information to avoid race condition in rescheduling points.
 */
ZP_LOCAL zp_task_runtime             window_tasks_statistic[ZP_TASK_COUNT];

/* clang-format on */

/*******************************************************************************************************************
 *                                             LOCAL API
 *******************************************************************************************************************/
ZP_LOCAL uint32_t zperf_get_task_runtime(uint8_t task_iterator, uint32_t timestamp);
ZP_LOCAL uint32_t zperf_get_timestamp_delta(uint32_t, uint32_t);
ZP_LOCAL uint32_t zperf_get_system_runtime(void);
ZP_LOCAL uint32_t zperf_get_idle_runtime(void);

ZP_LOCAL void zperf_cycle_backup(void);

ZP_LOCAL void zperf_isr_lock(unsigned int *key);
ZP_LOCAL void zperf_isr_unlock(unsigned int key);
ZP_LOCAL void zperf_reset_runtime_statistic(void);

ZP_LOCAL void zperf_trace_cycle_statistic(uint32_t *);
ZP_LOCAL void zperf_trace_cpu_usage(uint32_t *);
ZP_LOCAL void zperf_trace_uptime_statistic(void);

ZP_LOCAL_INLINE void zperf_update_isrs_statistic(uint8_t isrIterator, uint32_t timestamp,
						 uint32_t isrRuntime);
ZP_LOCAL_INLINE void zperf_update_task_statistic(uint8_t task_iterator, uint32_t timestamp,
						 uint32_t task_runtime);
ZP_LOCAL_INLINE void zperf_update_idle_statistic(uint32_t timestamp);
ZP_LOCAL_INLINE void zperf_update_uptime_statistic(uint8_t task_iterator, uint32_t task_runtime,
						   zp_task_runtime *task);

ZP_LOCAL_INLINE void zperf_reset_trace_cycle_statistic(uint8_t task_iterator);
ZP_LOCAL_INLINE void zperf_reset_hooks_statistic(uint8_t task_iterator);

/*******************************************************************************************************************
 *                                       HOOKS IMPLEMENTATION
 *******************************************************************************************************************/

ZP_RTOS_CALLED void pre_task_hook(void)
{
	uint32_t timestamp = wr_get_uptime_timestamp();
	uint32_t task_id = wr_get_task_id();
	uint8_t task_iterator = wr_get_task_iterator(task_id);

	if (TASK_IS_VALID(task_iterator)) {
		hook_tasks_statistics[task_iterator].task_timestamp = timestamp;

		if (IDLE_STATE_RUNNING == history_get_last_state()) {
			zperf_update_idle_statistic(timestamp);
		}

		history_update(task_id, timestamp, TASK_STATE_ENTER);
	} else {
		// TODO: add collecting diagnostic data to history
	}
}

ZP_RTOS_CALLED void post_task_hook(void)
{
	uint8_t task_iterator = 0;
	uint32_t task_id = 0;
	uint32_t task_runtime = 0;
	uint32_t timestamp = 0;

	task_id = wr_get_task_id();
	task_iterator = wr_get_task_iterator(task_id);

	if (TASK_IS_VALID(task_iterator)) {
		timestamp = wr_get_uptime_timestamp();
		task_runtime = zperf_get_task_runtime(task_iterator, timestamp);
		zperf_update_task_statistic(task_iterator, timestamp, task_runtime);

		history_update(task_id, timestamp, TASK_STATE_EXIT);
	} else {
		// TODO: add collecting diagnostic data to history
	}
}

ZP_RTOS_CALLED void pre_isr_hook(void)
{
	uint32_t timestamp = 0;

	timestamp = wr_get_uptime_timestamp();

	if (IDLE_STATE_RUNNING == history_get_last_state()) {
		zperf_update_idle_statistic(timestamp);
	}

	// TODO: add zperf_get_isr_id support
	/*
	   uint32_t isr_num = zperf_get_isr_id();

	   if (isr_num != 0)
	   {
	      ZP_TRACE_INFO("Currently running in ISR number: %u\n", isr_num);
	   }
	   else
	   {
	      ZP_TRACE_INFO("Currently running in thread context.");
	   }
	*/
}

ZP_RTOS_CALLED void post_isr_hook(void)
{
	uint32_t timestamp = wr_get_uptime_timestamp();
}

ZP_RTOS_CALLED void idle_task_running(void)
{
	static bool idle_not_determined = true;

	if (idle_not_determined) {
		idle_task_id = wr_get_task_id();
		idle_task_iterator = wr_get_task_iterator(idle_task_id);
		idle_not_determined = false;
	}

	if (idle_task_iterator != ZP_UNINIT && idle_task_id != ZP_UNINIT) {
		history_update(idle_task_id, wr_get_uptime_timestamp(), IDLE_STATE_RUNNING);
	}
}

/*******************************************************************************************************************
 *                                         LOCAL API IMPLEMENTATION
 *******************************************************************************************************************/

ZP_LOCAL uint32_t zperf_get_timestamp_delta(uint32_t timestamp1, uint32_t timestamp2)
{
	uint32_t delta = 0;

	if (timestamp2 >= timestamp1) {
		delta = timestamp2 - timestamp1;
	} else {
		/* In case of type overflow */
		delta = (ZP_TIMESTAMP_TOP - timestamp1) + timestamp2 + 1;
	}

	return delta;
}

/**
 * @brief Calculate total runtime across all Tasks (Threads) running in the system (with Idle
 * task(s))
 */
ZP_LOCAL uint32_t zperf_get_system_runtime(void)
{
	uint8_t task_iterator = 0;
	uint32_t system_total_runtime = 0U;

	for (task_iterator = 0; task_iterator < ZP_TASK_COUNT; task_iterator++) {
		system_total_runtime += window_tasks_statistic[task_iterator].total_runtime;
	}

	return system_total_runtime;
}

/**
 * @brief Calculate separately Idle task(s) total runtime
 */
ZP_LOCAL uint32_t zperf_get_idle_runtime(void)
{
	uint32_t idle_total_runtime = 0U;

	if (ZP_UNINIT != idle_task_iterator) {
		idle_total_runtime = window_tasks_statistic[idle_task_iterator].total_runtime;
	}

	return idle_total_runtime;
}

ZP_LOCAL void zperf_trace_cpu_usage(uint32_t *time_slice)
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
	idle_total_runtime = zperf_get_idle_runtime();

	/* Calculate total runtime across all Tasks (Threads) running in the system, except Idle
	 * task */
	system_total_runtime = zperf_get_system_runtime() - idle_total_runtime;

	/* Calculate RTOS's resources total runtime */
	os_total_runtime = (*time_slice) - (system_total_runtime + idle_total_runtime);

	/* Calculate percentages */
	idle_time_per = (uint8_t)GET_CPU_PERCENT((*time_slice), idle_total_runtime);
	cpu_load_per = (uint8_t)GET_CPU_PERCENT((*time_slice), system_total_runtime);
	os_per = (uint8_t)GET_CPU_PERCENT((*time_slice), os_total_runtime);

	ZP_TRACE_INFO("# ################# CPU LOAD %d%% ### IDLE %d%% ### KERNEL %d%% "
		      "################## #\n",
		      cpu_load_per, idle_time_per, os_per);
}

ZP_LOCAL_INLINE void zperf_update_task_statistic(uint8_t task_iterator, uint32_t timestamp,
						 uint32_t task_runtime)
{
	zp_task_runtime *task_ptr = &hook_tasks_statistics[task_iterator];

	if (task_runtime > task_ptr->max_runtime) {
		task_ptr->max_runtime = task_runtime;
	}

	task_ptr->task_timestamp = timestamp;
	task_ptr->total_runtime += task_runtime;
	task_ptr->task_switches++;

	zperf_update_uptime_statistic(task_iterator, task_runtime, task_ptr);
}

ZP_LOCAL_INLINE void zperf_update_idle_statistic(uint32_t timestamp)
{
	uint32_t idle_runtime = zperf_get_task_runtime(idle_task_iterator, timestamp);
	zperf_update_task_statistic(idle_task_iterator, timestamp, idle_runtime);
}

ZP_LOCAL_INLINE void zperf_update_isrs_statistic(uint8_t isrIterator, uint32_t timestamp,
						 uint32_t isrRuntime)
{
}

ZP_LOCAL_INLINE void zperf_update_uptime_statistic(uint8_t task_iterator, uint32_t task_runtime,
						   zp_task_runtime *task_ptr)
{
	hook_uptime_statistic.tasks_runtime[task_iterator] += task_runtime;
	hook_uptime_statistic.max_tasks_runtime[task_iterator] = task_ptr->max_runtime;
	hook_uptime_statistic.tasks_switches[task_iterator] += task_ptr->task_switches;
}

ZP_LOCAL void zperf_isr_lock(unsigned int *key)
{
	*key = ZP_ISR_LOCK();
}

ZP_LOCAL void zperf_isr_unlock(unsigned int key)
{
	ZP_ISR_UNLOCK(key);
}

ZP_LOCAL void zperf_trace_uptime_statistic(void)
{
	// TODO: same for zperf_trace_cycle_statistic
}

ZP_LOCAL void zperf_cycle_backup(void)
{
	unsigned int key;
	zperf_isr_lock(&key);
	memcpy(&window_tasks_statistic, &hook_tasks_statistics, sizeof(hook_tasks_statistics));
	memcpy(&window_uptime_statistic, &hook_uptime_statistic, sizeof(hook_uptime_statistic));
	zperf_isr_unlock(key);
}

ZP_LOCAL void zperf_trace_cycle_statistic(uint32_t *time_slice)
{
	ZP_TRACE_INFO(
		"# #####################   1 Second System Performance   ###################### #");
	ZP_TRACE_INFO(
		"#                                                                              #");
	ZP_TRACE_INFO(
		"#   Task Id         Total runtime         Max runtime        Total Switches    #");

	// TODO: add support of formatting names using k_thread_name_get((k_tid_t)thread);
	for (uint8_t task_iterator = 0; task_iterator < ZP_TASK_COUNT; task_iterator++) {
		ZP_TRACE_INFO("%s %6u %22u %19u %17u %10s", "#", task_iterator,
			      window_tasks_statistic[task_iterator].total_runtime,
			      window_tasks_statistic[task_iterator].max_runtime,
			      window_tasks_statistic[task_iterator].task_switches, "#");
	}
	ZP_TRACE_INFO(
		"# ############################################################################ #");
	ZP_TRACE_INFO("# ### Calculations Timeslice: %u\n", (*time_slice));
}

ZP_LOCAL void zperf_reset_runtime_statistic(void)
{
	for (uint8_t task_iterator = 0; task_iterator < ZP_TASK_COUNT; task_iterator++) {
		zperf_reset_trace_cycle_statistic(task_iterator);
		zperf_reset_hooks_statistic(task_iterator);
	}
}

ZP_LOCAL_INLINE void zperf_reset_trace_cycle_statistic(uint8_t task_iterator)
{
	window_tasks_statistic[task_iterator].max_runtime = 0u;
	window_tasks_statistic[task_iterator].max_runtime = 0u;
	window_tasks_statistic[task_iterator].task_switches = 0u;
	window_tasks_statistic[task_iterator].total_runtime = 0u;
}

ZP_LOCAL_INLINE void zperf_reset_hooks_statistic(uint8_t task_iterator)
{
	hook_tasks_statistics[task_iterator].max_runtime = 0u;
	hook_tasks_statistics[task_iterator].max_runtime = 0u;
	hook_tasks_statistics[task_iterator].task_switches = 0u;
	hook_tasks_statistics[task_iterator].total_runtime = 0u;
}

ZP_LOCAL uint32_t zperf_get_task_runtime(uint8_t task_iterator, uint32_t timestamp)
{
	uint32_t prev_timestamp = hook_tasks_statistics[task_iterator].task_timestamp;
	uint32_t task_runtime = zperf_get_timestamp_delta(prev_timestamp, timestamp);
	return task_runtime;
}

/*******************************************************************************************************************
 *                                          USER API IMPLEMENTATION
 *******************************************************************************************************************/

/**
 * @brief Trace Tasks (Threads) & ISRs runtime statistic per window cycle.
 *
 */
ZP_USER_CALLED void zperf_runtime_overall_statistic(void)
{
	uint32_t time_slice = 0;
	uint32_t curr_timestamp = 0;
	static uint32_t last_timestamp = 0U;

	zperf_cycle_backup();

	/* Calculate time_slice */
	curr_timestamp = wr_get_uptime_timestamp();
	time_slice = zperf_get_timestamp_delta(last_timestamp, curr_timestamp);
	last_timestamp = curr_timestamp;

#if (ZP_CFG_TRACE_MAIN_STAT == ZP_CFG_ON)
	zperf_trace_cycle_statistic(&time_slice);
#endif

#if (ZP_CFG_TRACE_UPTIME == ZP_CFG_ON)
	zperf_trace_uptime_statistic();
#endif

#if (ZP_CFG_TRACE_CPU_LOAD == ZP_CFG_ON)
	zperf_trace_cpu_usage(&time_slice);
#endif

#if (ZP_CFG_TRACE_HISTORY == ZP_CFG_ON)
	history_trace_collected();
#endif

	zperf_reset_runtime_statistic();
	history_clear();
}

ZP_USER_CALLED void zperf_pre_scheduler_init(void)
{
	history_init();
	zperf_reset_runtime_statistic();
}

ZP_USER_CALLED void zperf_post_scheduler_init(void)
{
	wr_init_configuration();
}

ZP_USER_CALLED void zperf_trace_history(void)
{
	history_trace_collected();
}
