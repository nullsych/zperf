/**
 * @file   ZPerfTask.c
 * @author nullsych
 * @date   May 7, 2024
 */

#include <zephyr/kernel.h>
#include <zperf.h>

#include "common/TasksApi.h"
#include "common/TasksCfg.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(NrfPerfTask, LOG_LEVEL_DBG);

TASK_LOCAL TASK_ENTRY_POINT void task_zperf_entry(void)
{
	for (;;) {
		zperf_runtime_overall_statistic();
		k_msleep(ZPERF_TASK_PERIOD);
	}
}

/* Define the thread using a macro */
K_THREAD_DEFINE(Task_ZPerf, ZPERF_TASK_STACKSIZE, task_zperf_entry, NULL, NULL, NULL,
		ZPERF_TASK_PRIORITY, 0, 0);
