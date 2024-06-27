/**
 * @file   main.c
 * @brief  Contain implementation of OS-specific hook routines.
 * @author kvant6ubl
 * @date   May 7, 2024
 *
 */

#include <zephyr/kernel.h>

void sys_trace_thread_switched_in_user(void) //(struct k_thread *thread)
{
   /* code */
}

void sys_trace_thread_switched_out_user(void) //(struct k_thread *thread)
{
   /* code */
}

void sys_trace_isr_enter_user(void)
{
   /* code */
}

void sys_trace_isr_exit_user(void)
{
   /* code */
}

void sys_trace_idle_user(void)
{
   /* code */
}

// void sys_port_trace_pm_system_suspend_enter(uint32_t ticks)
// {
   // /* code */
// }

// void sys_port_trace_pm_system_suspend_exit(uint32_t ticks, uint32_t ret)
// {
   // /* code */
// }

// void sys_port_trace_pm_device_request_enter(const struct device *dev, uint32_t target_state)
// {
// }

// void sys_port_trace_pm_device_request_exit(const struct device *dev, uint32_t ret)
// {
// }

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
