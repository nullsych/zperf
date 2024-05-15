/**
 * @file   SysPerfTask.c
 * @brief  Contain main implementation for SysPerfTask.
 * @author kvant6ubl
 * @date   May 7, 2024
 * The code inside SysPerfTask consist of the main measuring routine from SysPerf lib.
 *
 */

#include <zephyr.h>
#include <SysPerf.h>

#include "common/TasksApi.h"
#include "common/TasksCfg.h"

K_THREAD_STACK_DEFINE(SysPerf_Task_stack_area, SYSPERF_TASK_STACKSIZE);

TASK_LOCAL struct k_thread SysPerf_zthread_data;
TASK_LOCAL k_tid_t SysPerf_zid;
TASK_LOCAL uint8_t SysPerf_taskId = SYSPERF_TASKID;

TASK_LOCAL void Task_SysPerf(void *unused_arg1, void *unused_arg2, void *unused_arg3)
{
    ARG_UNUSED(unused_arg1);
    ARG_UNUSED(unused_arg2);
    ARG_UNUSED(unused_arg3);

    k_thread_custom_data_set((void *)SysPerf_taskId);

    for (;;)
    {
        SysPerf_RuntimeOverallStatistic();
        k_msleep(SYSPERF_TASK_PERIOD);
    }
}

void SysPerf_createTask(void)
{
    SysPerf_zid = k_thread_create(&SysPerf_zthread_data, SysPerf_Task_stack_area,
                                  K_THREAD_STACK_SIZEOF(SysPerf_Task_stack_area),
                                  Task_SysPerf, NULL, NULL, NULL,
                                  SYSPERF_TASK_PRIORITY, 0, K_FOREVER);

    k_thread_name_set(SysPerf_zid, "Task_LED");

    k_thread_start(SysPerf_zid);
}
