/**
 * @file   rtos_hooks.c
 * @brief  Implement Zephyr trace hook functions with invoked nrf_perf API.
 * @author nullsych
 * @date   May 6, 2024
 *
 */
/* clang-format off */
/*******************************************************************************************************************
 *                                             RTOS-SPECIFIC HEADERS                                               *
 ******************************************************************************************************************/
#include <zephyr/kernel.h>
#include <zephyr/tracing/tracing.h>

/*******************************************************************************************************************
 *                                                nrf_perf HEADERS                                                  *
 ******************************************************************************************************************/
#include "rtos_hooks.h"
#include "nrf_perf.h"

/*******************************************************************************************************************
 *                                    PLACE IMPLEMENTATION OF RTOS-SPECIFIC HOOKS                                  *
 ******************************************************************************************************************/
/* clang-format on */

void sys_trace_thread_switched_in_user(void)
{
	nrf_perf_pre_task_hook();
}

void sys_trace_thread_switched_out_user(void)
{
	nrf_perf_post_task_hook();
}

void sys_trace_isr_enter_user(void)
{
	nrf_perf_pre_isr_hook();
}

void sys_trace_isr_exit_user(void)
{
	nrf_perf_post_isr_hook();
}

void sys_trace_idle_user(void)
{
	nrf_perf_idle_task_running();
}
