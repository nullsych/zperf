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
ZP_LOCAL zp_task_id                 idle_task_id       = 0U; /* 0: no idle thread seen yet */

/**
 * @brief Thread ID for each zperf task iterator, the index in this table is the iterator.
 * @details zperf keeps its own table instead of storing the iterator in the thread object,
 * so it doesn't take over fields the application may use (e.g. Zephyr's custom_data).
 * Slots are never released: a thread object reused for a new thread keeps its slot.
 */
ZP_LOCAL zp_task_id                 task_registry[ZP_TASK_COUNT];
ZP_LOCAL uint8_t                    task_registry_count = 0U;

/**
 * @brief Set once the threads that exist at startup are registered. Hooks are ignored until
 * then, so the startup log lists threads in a stable order.
 */
ZP_LOCAL volatile bool              tasks_enumerated   = false;

/**
 * @brief Start of the current statistic window.
 */
ZP_LOCAL uint32_t                   window_start_timestamp = 0U;

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

ZP_LOCAL uint32_t zperf_cycle_backup(void);
ZP_LOCAL void zperf_close_running_slice(uint32_t timestamp);

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
ZP_LOCAL_INLINE void zperf_book_runtime(uint8_t task_iterator, uint32_t timestamp,
					uint32_t task_runtime);

ZP_LOCAL_INLINE void zperf_reset_trace_cycle_statistic(uint8_t task_iterator);
ZP_LOCAL_INLINE void zperf_reset_hooks_statistic(uint8_t task_iterator);

/*******************************************************************************************************************
 *                                       HOOKS IMPLEMENTATION
 *******************************************************************************************************************/

/**
 * @brief Find the zperf iterator of a thread, ZP_UNINIT if it is not registered.
 * @details Linear search: the table holds ZP_TASK_COUNT entries, typically around ten.
 */
ZP_LOCAL uint8_t zperf_task_lookup(zp_task_id task_id)
{
	for (uint8_t task_iterator = 0; task_iterator < task_registry_count; task_iterator++) {
		if (task_registry[task_iterator] == task_id) {
			return task_iterator;
		}
	}

	return ZP_UNINIT;
}

/**
 * @brief Get the iterator of a thread, registering it in a free slot if needed.
 * @return ZP_UNINIT if the table is full. Must be called with IRQs locked.
 */
ZP_RTOS_CALLED uint8_t zperf_task_register(zp_task_id task_id)
{
	uint8_t task_iterator = zperf_task_lookup(task_id);

	if ((task_iterator == ZP_UNINIT) && (task_id != 0U) &&
	    (task_registry_count < ZP_TASK_COUNT)) {
		task_iterator = task_registry_count;
		task_registry[task_registry_count++] = task_id;
	}

	return task_iterator;
}

ZP_RTOS_CALLED void pre_task_hook(void)
{
	if (!tasks_enumerated) {
		return;
	}

	uint32_t timestamp = wr_get_uptime_timestamp();
	zp_task_id task_id = wr_get_task_id();
	/* Threads created after startup get a slot on their first run, while slots are left */
	uint8_t task_iterator = zperf_task_register(task_id);

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
	zp_task_id task_id = 0;
	uint32_t task_runtime = 0;
	uint32_t timestamp = 0;

	if (!tasks_enumerated) {
		return;
	}

	task_id = wr_get_task_id();
	task_iterator = zperf_task_lookup(task_id);

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
	/* ISR runtime accounting is not implemented yet */
}

ZP_RTOS_CALLED void idle_task_running(void)
{
	zp_task_id task_id;
	uint8_t task_iterator;

	if (!tasks_enumerated) {
		return;
	}

	task_id = wr_get_task_id();

	/* The hook also fires from drivers outside the idle thread, ignore those calls */
	if (!wr_is_idle_task(task_id)) {
		return;
	}

	task_iterator = zperf_task_lookup(task_id);
	if (!TASK_IS_VALID(task_iterator)) {
		return;
	}

	idle_task_id = task_id;
	idle_task_iterator = task_iterator;
	history_update(task_id, wr_get_uptime_timestamp(), IDLE_STATE_RUNNING);
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

	/* Calculate RTOS's resources total runtime. Tick rounding can make the measured runtime
	 * slightly exceed the window, so clamp instead of letting the subtraction wrap around. */
	if ((system_total_runtime + idle_total_runtime) < (*time_slice)) {
		os_total_runtime = (*time_slice) - (system_total_runtime + idle_total_runtime);
	} else {
		os_total_runtime = 0U;
	}

	/* Calculate percentages */
	idle_time_per = (uint8_t)MIN(GET_CPU_PERCENT((*time_slice), idle_total_runtime), 100U);
	cpu_load_per = (uint8_t)MIN(GET_CPU_PERCENT((*time_slice), system_total_runtime), 100U);
	os_per = (uint8_t)MIN(GET_CPU_PERCENT((*time_slice), os_total_runtime), 100U);

	ZP_TRACE_INFO("# ################# CPU LOAD %d%% ### IDLE %d%% ### KERNEL %d%% "
		      "################## #\n",
		      cpu_load_per, idle_time_per, os_per);
}

/**
 * @brief Book a piece of a thread's runtime to the window and uptime statistic.
 * @details Doesn't count a context switch: callers that end a slice without a switch
 * (idle split at ISR entry, open slice closed at the window end) use it directly.
 */
ZP_LOCAL_INLINE void zperf_book_runtime(uint8_t task_iterator, uint32_t timestamp,
					uint32_t task_runtime)
{
	zp_task_runtime *task_ptr = &hook_tasks_statistics[task_iterator];

	if (task_runtime > task_ptr->max_runtime) {
		task_ptr->max_runtime = task_runtime;
	}

	task_ptr->task_timestamp = timestamp;
	task_ptr->total_runtime += task_runtime;

	hook_uptime_statistic.tasks_runtime[task_iterator] += task_runtime;

	/* Uptime max is never reset, unlike the per-window max in task_ptr */
	if (task_runtime > hook_uptime_statistic.max_tasks_runtime[task_iterator]) {
		hook_uptime_statistic.max_tasks_runtime[task_iterator] = task_runtime;
	}
}

/**
 * @brief Book a finished slice on a context switch out.
 */
ZP_LOCAL_INLINE void zperf_update_task_statistic(uint8_t task_iterator, uint32_t timestamp,
						 uint32_t task_runtime)
{
	zperf_book_runtime(task_iterator, timestamp, task_runtime);

	hook_tasks_statistics[task_iterator].task_switches++;
	hook_uptime_statistic.tasks_switches[task_iterator]++;
}

/**
 * @brief Book idle runtime up to timestamp while idle keeps running.
 * @details Called on ISR entry when idle was running. Idle isn't switched out there, so the
 * switch counter is left as is: it counts real context switches only, like for other threads.
 */
ZP_LOCAL_INLINE void zperf_update_idle_statistic(uint32_t timestamp)
{
	uint32_t idle_runtime = zperf_get_task_runtime(idle_task_iterator, timestamp);
	zperf_book_runtime(idle_task_iterator, timestamp, idle_runtime);
}

ZP_LOCAL_INLINE void zperf_update_isrs_statistic(uint8_t isrIterator, uint32_t timestamp,
						 uint32_t isrRuntime)
{
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

/**
 * @brief Book the running thread's open slice up to the window end.
 * @details Without this, a slice crossing the window boundary is booked whole to the window
 * where the thread is switched out, so that window can exceed 100%. The remainder of the slice
 * is counted from the window end on switch out. It is not a context switch, so the switch
 * counter is left as is. Must be called with IRQs locked.
 */
ZP_LOCAL void zperf_close_running_slice(uint32_t timestamp)
{
	uint8_t task_iterator = zperf_task_lookup(wr_get_task_id());

	if (!tasks_enumerated || !TASK_IS_VALID(task_iterator)) {
		return;
	}

	zperf_book_runtime(task_iterator, timestamp,
			   zperf_get_task_runtime(task_iterator, timestamp));
}

/**
 * @brief Snapshot the window statistic and start a new window in one atomic step.
 * @details Copy and reset must happen under the same lock, otherwise events arriving
 * between them are lost. The window end timestamp is taken there as well.
 */
ZP_LOCAL uint32_t zperf_cycle_backup(void)
{
	unsigned int key;
	uint32_t timestamp;

	zperf_isr_lock(&key);
	timestamp = wr_get_uptime_timestamp();
	zperf_close_running_slice(timestamp);
	memcpy(&window_tasks_statistic, &hook_tasks_statistics, sizeof(hook_tasks_statistics));
	memcpy(&window_uptime_statistic, &hook_uptime_statistic, sizeof(hook_uptime_statistic));
	for (uint8_t task_iterator = 0; task_iterator < ZP_TASK_COUNT; task_iterator++) {
		zperf_reset_hooks_statistic(task_iterator);
	}
	/* The history window ends at the same point as the statistic window */
	history_snapshot_take(true);
	zperf_isr_unlock(key);

	return timestamp;
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
	window_tasks_statistic[task_iterator].task_switches = 0u;
	window_tasks_statistic[task_iterator].total_runtime = 0u;
}

ZP_LOCAL_INLINE void zperf_reset_hooks_statistic(uint8_t task_iterator)
{
	/* task_timestamp is kept: it is the start of the running thread's open slice */
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

	curr_timestamp = zperf_cycle_backup();

	/* Calculate time_slice */
	time_slice = zperf_get_timestamp_delta(window_start_timestamp, curr_timestamp);
	window_start_timestamp = curr_timestamp;

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
	history_trace_snapshot();
#endif

	/* Hook statistic and history were already reset atomically in zperf_cycle_backup() */
}

ZP_USER_CALLED void zperf_pre_scheduler_init(void)
{
	history_init();
	zperf_reset_runtime_statistic();
}

ZP_USER_CALLED void zperf_post_scheduler_init(void)
{
	unsigned int key;
	uint32_t timestamp;

	wr_init_configuration();

	/* Start accounting from now: hooks were ignored so far, so no task has a valid start time */
	zperf_isr_lock(&key);
	timestamp = wr_get_uptime_timestamp();
	for (uint8_t task_iterator = 0; task_iterator < ZP_TASK_COUNT; task_iterator++) {
		hook_tasks_statistics[task_iterator].task_timestamp = timestamp;
	}
	window_start_timestamp = timestamp;
	tasks_enumerated = true;
	zperf_isr_unlock(key);
}

ZP_USER_CALLED void zperf_trace_history(void)
{
	history_trace_collected();
}
