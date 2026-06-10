/**
 * @file   nrf_perf_startup.c
 * @brief  Implementation of wrappers that encapsulate Zephyr-specific API.
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
*                                                nrf_perf HEADERS                                                   *
*******************************************************************************************************************/
#include "nrf_perf_data_types.h"
#include "nrf_perf_keywords.h"
#include "nrf_perf.h"
#include "wrapper.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(nrf_perf_startup, LOG_LEVEL_DBG);

/*******************************************************************************************************************
*                                            WRAPPER API IMPLEMENTATION                                            *
********************************************************************************************************************/
/* clang-format on */

/**
 * @brief Get unique task_id (zephyr thread ID) of the current Task (Thread).
 */
NP_RTOS_WRAPPER uint32_t nrf_perf_get_task_id(void)
{
	/**
	 * @note Recommendation from Zephyr RTOS: Can't use k_current_get() as thread base might be
	 * incorrect.
	 */
	return (uint32_t)k_sched_current_thread_query();
}

/**
 * @brief Get task iterator (nrf_perf specific) based on task_id (zephyr thread ID).
 */
NP_RTOS_WRAPPER uint8_t nrf_perf_get_task_iterator(uint32_t task_id)
{
	k_tid_t id = (k_tid_t)(task_id);
	return (uint8_t)(uintptr_t)id->custom_data;
}

/**
 * @brief Get task name based on task_id (zephyr thread ID).
 */
NP_RTOS_WRAPPER const char *nrf_perf_get_task_name(uint32_t task_id)
{
	return k_thread_name_get((k_tid_t)task_id);
}

/**
 * @brief Get unique ID of the current ISR.
 */
NP_RTOS_WRAPPER uint32_t nrf_perf_get_isr_id(void)
{
	return k_is_in_isr() ? NP_ISR_ID_SIGN : 0U;
}

NP_RTOS_WRAPPER void nrf_perf_enumerate_callback(np_task_ptr thread, void *user_data)
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
	NP_TRACE_INFO("Iterating %d thread in the system. Thread data: ",
		      (uint8_t)(uintptr_t)mod_thread->custom_data);
	NP_TRACE_INFO("      Name        : %s", mod_thread->name);
	NP_TRACE_INFO("      Priority    : %d", mod_thread->base.prio);
	NP_TRACE_INFO("      Stack size  : %d", mod_thread->stack_info.size);
	NP_TRACE_INFO("      Custom data : %d", (uint8_t)(uintptr_t)mod_thread->custom_data);

	task_iterator++;
}

/**
 * @brief Saves RTU's (Runtime Unit, Tasks/ISRs) configuration.
 */
NP_RTOS_WRAPPER void nrf_perf_init_configuration(void)
{
	/**
	 * @note Assign pointer to the local callback function to save cfg
	 */
	k_thread_foreach(nrf_perf_enumerate_callback, NULL);
}

NP_RTOS_WRAPPER uint32_t nrf_perf_get_uptime_timestamp(void)
{
	uint32_t timestamp;
	timestamp = k_uptime_ticks();
	timestamp = k_ticks_to_us_near32(timestamp);
	return timestamp; /* in uS */
}
