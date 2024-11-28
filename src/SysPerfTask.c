/**
 * @file   SysPerfTask.c
 * @brief  Contain main implementation for SysPerfTask.
 * @author kvant6ubl
 * @date   May 7, 2024
 * The code inside SysPerfTask consist of the main measuring routine from SysPerf lib.
 *
 */

#include <zephyr/kernel.h>
#include <SysPerf.h>

#include "common/TasksApi.h"
#include "common/TasksCfg.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(SysPerfTask, LOG_LEVEL_DBG);


TASK_LOCAL TASK_ENTRY_POINT void Task_SysPerf_entry(void)
{
    for (;;)
    {
        SysPerf_RuntimeOverallStatistic();
        k_msleep(SYSPERF_TASK_PERIOD);
    }
}

/* Define the thread using a macro */
K_THREAD_DEFINE(Task_SysPerf, SYSPERF_TASK_STACKSIZE, Task_SysPerf_entry, NULL, NULL, NULL,
                SYSPERF_TASK_PRIORITY, 0, 0);
