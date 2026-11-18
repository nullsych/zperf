/**
 * @file   zperf_startup.c
 * @brief  Implementation of wrappers (wr_*) that encapsulate Zephyr-specific API.
 * @author nullsych
 * @date   Jul 18, 2024
 *
 */

/* clang-format off */
/*******************************************************************************************************************
 *                                             RTOS-SPECIFIC HEADERS                                               *
 *******************************************************************************************************************/
#include <stdint.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/util.h>
#include <zephyr/tracing/tracing.h>
#if defined(CONFIG_ZPERF_TIMESTAMP_TIMING_FUNCTIONS)
#include <zephyr/timing/timing.h>
#endif

/*******************************************************************************************************************
*                                                zperf HEADERS                                                   *
*******************************************************************************************************************/
#include "zperf_data_types.h"
#include "zperf_keywords.h"
#include "zperf_internal.h"
#include "zperf_clock.h"
#include "wrapper.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(zperf_startup, LOG_LEVEL_DBG);

/*******************************************************************************************************************
*                                            WRAPPER API IMPLEMENTATION                                            *
********************************************************************************************************************/
/* clang-format on */

/**
 * @brief Get unique task_id (zephyr thread ID) of the current Task (Thread).
 */
ZP_RTOS_WRAPPER zp_task_id wr_get_task_id(void)
{
	/**
	 * @note Recommendation from Zephyr RTOS: Can't use k_current_get() as thread base might be
	 * incorrect.
	 */
	return (zp_task_id)k_sched_current_thread_query();
}

/**
 * @brief Get task name based on task_id (zephyr thread ID).
 */
ZP_RTOS_WRAPPER const char *wr_get_task_name(zp_task_id task_id)
{
	return k_thread_name_get((k_tid_t)task_id);
}

/**
 * @brief Check that task_id is the RTOS idle thread running in thread context.
 * @note Zephyr calls the idle trace hook from arch_cpu_atomic_idle() too, which drivers use
 * outside the idle thread (nRF LF clock start before the kernel runs, entropy and radio
 * drivers). Only the idle thread may run at K_IDLE_PRIO, so the priority identifies it.
 */
ZP_RTOS_WRAPPER bool wr_is_idle_task(zp_task_id task_id)
{
	k_tid_t thread = (k_tid_t)task_id;

	if (k_is_pre_kernel() || k_is_in_isr() || (thread == NULL)) {
		return false;
	}

	return thread->base.prio == K_IDLE_PRIO;
}

/**
 * @brief Get unique ID of the current ISR.
 */
ZP_RTOS_WRAPPER uint32_t wr_get_isr_id(void)
{
	return k_is_in_isr() ? ZP_ISR_ID_SIGN : 0U;
}

ZP_RTOS_WRAPPER void wr_enumerate_callback(zp_task_ptr thread, void *user_data)
{
	/* zperf keeps its own thread -> iterator table, the thread object is never modified */
	uint8_t task_iterator = zperf_task_register((zp_task_id)thread);

	ARG_UNUSED(user_data);

	if (!TASK_IS_VALID(task_iterator)) {
		ZP_TRACE_WARN("Thread %s is not tracked: no free slot, increase "
			      "CONFIG_ZPERF_USER_TASKS_COUNT or CONFIG_ZPERF_SYSTEM_TASKS_COUNT",
			      thread->name);
		return;
	}

	/* Print config data */
	ZP_TRACE_INFO("Iterating %d thread in the system. Thread data: ", task_iterator);
	ZP_TRACE_INFO("      Name        : %s", thread->name);
	ZP_TRACE_INFO("      Priority    : %d", thread->base.prio);
	ZP_TRACE_INFO("      Stack size  : %zu", thread->stack_info.size);
	ZP_TRACE_INFO("      Task Id     : %d", task_iterator);
}

/**
 * @brief Saves RTU's (Runtime Unit, Tasks/ISRs) configuration.
 */
ZP_RTOS_WRAPPER void wr_init_configuration(void)
{
	/**
	 * @note Assign pointer to the local callback function to save cfg
	 */
	k_thread_foreach(wr_enumerate_callback, NULL);
}

/**
 * @brief Start of the current kernel tick, in microseconds (wraps at 2^32 us).
 */
static uint32_t wr_tick_clock_us(void)
{
	return (uint32_t)k_ticks_to_us_floor64((uint64_t)k_uptime_ticks());
}

#if defined(CONFIG_ZPERF_TIMESTAMP_TIMING_FUNCTIONS)

/* Cycle counter of the timing functions (DWT on Cortex-M) turned into a microsecond uptime */
static zp_clock hires_clock;
static timing_t hires_last_cycles;
static bool hires_ready;

/**
 * @brief Start the cycle counter. Called from thread context before any hook runs: finding the
 * counter rate may busy-wait on some SoCs, which must not happen inside a scheduler hook.
 */
ZP_RTOS_WRAPPER void wr_timestamp_init(void)
{
	uint64_t freq_hz;
	unsigned int key;

	if (hires_ready) {
		return;
	}

	timing_init();
	timing_start();
	freq_hz = timing_freq_get();

	key = irq_lock();
	hires_last_cycles = timing_counter_get();
	zp_clock_init(&hires_clock, freq_hz, k_ticks_to_us_ceil32(1), wr_tick_clock_us());
	hires_ready = true;
	irq_unlock(key);
}

ZP_RTOS_WRAPPER uint32_t wr_get_uptime_timestamp(void)
{
	timing_t now_cycles;
	uint32_t timestamp;
	unsigned int key;

	if (!hires_ready) {
		return wr_tick_clock_us();
	}

	/* Called from threads, the scheduler and ISRs: the clock state is shared */
	key = irq_lock();
	now_cycles = timing_counter_get();
	timestamp = zp_clock_update(&hires_clock,
				    timing_cycles_get(&hires_last_cycles, &now_cycles),
				    wr_tick_clock_us());
	hires_last_cycles = now_cycles;
	irq_unlock(key);

	return timestamp; /* in uS */
}

#else /* CONFIG_ZPERF_TIMESTAMP_UPTIME_TICKS */

ZP_RTOS_WRAPPER void wr_timestamp_init(void)
{
}

ZP_RTOS_WRAPPER uint32_t wr_get_uptime_timestamp(void)
{
	return wr_tick_clock_us(); /* in uS, tick resolution */
}

#endif /* CONFIG_ZPERF_TIMESTAMP_TIMING_FUNCTIONS */
