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

/*******************************************************************************************************************
*                                                zperf HEADERS                                                   *
*******************************************************************************************************************/
#include "zperf_data_types.h"
#include "zperf_keywords.h"
#include "zperf_internal.h"
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
ZP_RTOS_WRAPPER uint32_t wr_get_task_id(void)
{
	/**
	 * @note Recommendation from Zephyr RTOS: Can't use k_current_get() as thread base might be
	 * incorrect.
	 */
	return (uint32_t)k_sched_current_thread_query();
}

/**
 * @brief Get task iterator (zperf specific) based on task_id (zephyr thread ID).
 */
ZP_RTOS_WRAPPER uint8_t wr_get_task_iterator(uint32_t task_id)
{
	k_tid_t id = (k_tid_t)(task_id);
	return (uint8_t)(uintptr_t)id->custom_data;
}

/**
 * @brief Get task name based on task_id (zephyr thread ID).
 */
ZP_RTOS_WRAPPER const char *wr_get_task_name(uint32_t task_id)
{
	return k_thread_name_get((k_tid_t)task_id);
}

/**
 * @brief Check that task_id is the RTOS idle thread running in thread context.
 * @note Zephyr calls the idle trace hook from arch_cpu_atomic_idle() too, which drivers use
 * outside the idle thread (nRF LF clock start before the kernel runs, entropy and radio
 * drivers). Only the idle thread may run at K_IDLE_PRIO, so the priority identifies it.
 */
ZP_RTOS_WRAPPER bool wr_is_idle_task(uint32_t task_id)
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
	static uint8_t task_iterator = 0;

	ARG_UNUSED(user_data);

	/* Here k_thread_custom_data_set() is not used cause id doesn't allowed to write it during
	 * k_thread_foreach */
	// k_thread_custom_data_set((void *)(uintptr_t)task_iterator);

	/* Instead of this we use also not the best option by casting away a const qualification on
	 * thread object of pointer type. */
	k_tid_t mod_thread = (k_tid_t)(thread);

	/* And assign a task_iterator to it*/
	mod_thread->custom_data = (void *)(uintptr_t)task_iterator;

	/* Print config data */
	ZP_TRACE_INFO("Iterating %d thread in the system. Thread data: ",
		      (uint8_t)(uintptr_t)mod_thread->custom_data);
	ZP_TRACE_INFO("      Name        : %s", mod_thread->name);
	ZP_TRACE_INFO("      Priority    : %d", mod_thread->base.prio);
	ZP_TRACE_INFO("      Stack size  : %d", mod_thread->stack_info.size);
	ZP_TRACE_INFO("      Custom data : %d", (uint8_t)(uintptr_t)mod_thread->custom_data);

	task_iterator++;
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

ZP_RTOS_WRAPPER uint32_t wr_get_uptime_timestamp(void)
{
	uint32_t timestamp;
	timestamp = k_uptime_ticks();
	timestamp = k_ticks_to_us_near32(timestamp);
	return timestamp; /* in uS */
}
