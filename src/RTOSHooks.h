#include <zephyr.h>

void sys_port_trace_pm_system_suspend_enter(uint32_t ticks)
{
}

void sys_port_trace_pm_system_suspend_exit(uint32_t ticks, uint32_t ret)
{
}

void sys_port_trace_pm_device_request_enter(const struct device *dev, uint32_t target_state)
{
}

void sys_port_trace_pm_device_request_exit(const struct device *dev, uint32_t ret)
{
}

void sys_trace_thread_abort_user(struct k_thread *thread)
{
   // printk("# TRACE ############ sys_trace_thread_abort_user: Thread %s\n", thread->name);
}

void sys_trace_thread_suspend_user(struct k_thread *thread)
{
   // printk("# TRACE ############ sys_trace_thread_suspend_user: Thread %s\n", thread->name);
}

void sys_trace_thread_resume_user(struct k_thread *thread)
{
   // printk("# TRACE ############ sys_trace_thread_resume_user: Thread %s\n", thread->name);
}

void sys_trace_thread_name_set_user(struct k_thread *thread)
{
   // printk("# TRACE ############ sys_trace_thread_name_set_user: Thread %s\n", thread->name);
}

void sys_trace_thread_info_user(struct k_thread *thread)
{
   // printk("# TRACE ############ sys_trace_thread_info_user: Thread %s\n", thread->name);
}

void sys_trace_thread_sched_ready_user(struct k_thread *thread)
{
   // printk("# TRACE ############ sys_trace_thread_sched_ready_user: Thread %s\n", thread->name);
}

void sys_trace_thread_pend_user(struct k_thread *thread)
{
   // printk("# TRACE ############ sys_trace_thread_pend_user: Thread %s\n", thread->name);
}

void sys_trace_thread_priority_set_user(struct k_thread *thread, int prio)
{
   // printk("# TRACE ############ sys_trace_thread_priority_set_user: Thread %s\n", thread->name);
}

void sys_trace_isr_enter_user()
{
   // printk("# TRACE ############ sys_trace_isr_enter_user\n");
}

void sys_trace_isr_exit_user(void)
{
   // printk("# TRACE ############ sys_trace_isr_exit_user\n");
}

void sys_trace_idle_user(void)
{
   // printk("# TRACE ############ sys_trace_idle_user\n");
}

void sys_trace_thread_create_user(struct k_thread *thread)
{
   // printk("# TRACE ############ !!! sys_trace_thread_create_user: Thread %s created!\n", thread->name);
}