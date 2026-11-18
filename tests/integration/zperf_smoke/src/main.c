/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * zperf on a real Zephyr kernel. A worker keeps the CPU partly busy and zperf prints a window
 * every second; twister checks the console output (see testcase.yaml): the startup thread
 * list, the CPU load line and history rows for the idle and worker threads.
 */

#include <zephyr/kernel.h>
#include <zperf.h>

#define BUSY_WORKER_BUSY_US    2000
#define BUSY_WORKER_SLEEP_MS   8
#define ZPERF_REPORT_PERIOD_MS 1000

static void busy_worker_entry(void *p1, void *p2, void *p3)
{
	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	for (;;) {
		k_busy_wait(BUSY_WORKER_BUSY_US);
		k_msleep(BUSY_WORKER_SLEEP_MS);
	}
}

static void zperf_report_entry(void *p1, void *p2, void *p3)
{
	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	for (;;) {
		k_msleep(ZPERF_REPORT_PERIOD_MS);
		zperf_runtime_overall_statistic();
	}
}

/* Both start after main() returns or blocks: main runs at a higher priority */
K_THREAD_DEFINE(busy_worker, 1024, busy_worker_entry, NULL, NULL, NULL, 7, 0, 0);
K_THREAD_DEFINE(zperf_report, 2048, zperf_report_entry, NULL, NULL, NULL, 5, 0, 0);

int main(void)
{
	zperf_pre_scheduler_init();
	zperf_post_scheduler_init();
	return 0;
}
