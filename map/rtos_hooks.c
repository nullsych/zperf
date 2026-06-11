/**
 * @file   rtos_hooks.c
 * @brief  Implement Zephyr trace hook functions with invoked zperf API.
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
 *                                                zperf HEADERS                                                  *
 ******************************************************************************************************************/
#include "rtos_hooks.h"
#include "zperf_internal.h"

/*******************************************************************************************************************
 *                                    PLACE IMPLEMENTATION OF RTOS-SPECIFIC HOOKS                                  *
 ******************************************************************************************************************/
/* clang-format on */

void sys_trace_thread_switched_in_user(void)
{
	pre_task_hook();
}

void sys_trace_thread_switched_out_user(void)
{
	post_task_hook();
}

void sys_trace_isr_enter_user(void)
{
	pre_isr_hook();
}

void sys_trace_isr_exit_user(void)
{
	post_isr_hook();
}

void sys_trace_idle_user(void)
{
	idle_task_running();
}
