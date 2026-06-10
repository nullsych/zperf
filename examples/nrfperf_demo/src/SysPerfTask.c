/**
 * @file   NrfPerfTask.c
 * @brief  Contain main implementation for NrfPerfTask.
 * @author nullsych
 * @date   May 7, 2024
 * The code inside NrfPerfTask consist of the main measuring routine from nrf_perf lib.
 *
 */

#include <zephyr/kernel.h>
#include <nrf_perf.h>

#include "common/TasksApi.h"
#include "common/TasksCfg.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(NrfPerfTask, LOG_LEVEL_DBG);

TASK_LOCAL TASK_ENTRY_POINT void task_nrf_perf_entry(void)
{
	for (;;) {
		nrf_perf_runtime_overall_statistic();
		k_msleep(NRF_PERF_TASK_PERIOD);
	}
}

/* Define the thread using a macro */
K_THREAD_DEFINE(Task_NrfPerf, NRF_PERF_TASK_STACKSIZE, task_nrf_perf_entry, NULL, NULL, NULL,
		NRF_PERF_TASK_PRIORITY, 0, 0);
