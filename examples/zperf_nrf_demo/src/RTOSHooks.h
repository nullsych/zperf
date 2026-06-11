/**
 * @file   main.c
 * @brief  Contain implementation of OS-specific hook routines.
 *         Serves as build mock for West, the real magic is in tool.
 * @author nullsych
 * @date   May 7, 2024
 *
 */

#include <zephyr/kernel.h>

void sys_trace_thread_abort_user(struct k_thread *thread)
{
	/* code */
}

void sys_trace_thread_suspend_user(struct k_thread *thread)
{
	/* code */
}

void sys_trace_thread_resume_user(struct k_thread *thread)
{
	/* code */
}

void sys_trace_thread_name_set_user(struct k_thread *thread)
{
	/* code */
}

void sys_trace_thread_info_user(struct k_thread *thread)
{
	/* code */
}

void sys_trace_thread_sched_ready_user(struct k_thread *thread)
{
	/* code */
}

void sys_trace_thread_pend_user(struct k_thread *thread)
{
	/* code */
}

void sys_trace_thread_priority_set_user(struct k_thread *thread, int prio)
{
	/* code */
}

void sys_trace_thread_create_user(struct k_thread *thread)
{
	/* code */
}
