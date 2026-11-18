/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Shared helpers for the zperf host unit tests.
 *
 * Each test file includes the zperf sources directly (src/zperf.c and src/zperf_history.c), so
 * it can read and reset their static state without changing the production code. Include this
 * header once per test file, after those sources.
 */

#ifndef ZPERF_TEST_SUPPORT_H
#define ZPERF_TEST_SUPPORT_H

#include <stdio.h>
#include <string.h>

#include "unity.h"
#include "fake_rtos.h"

#define TEST_UNUSED __attribute__((unused))

/* Threads every test starts with, created in this order before zperf enumerates them */
static struct fake_thread *main_thread;
static struct fake_thread *worker_thread;
static struct fake_thread *idle_thread;

/* Put zperf and the fake RTOS back to the state right after power-on */
static TEST_UNUSED void zperf_test_reset(void)
{
	memset(hook_tasks_statistics, 0, sizeof(hook_tasks_statistics));
	memset(window_tasks_statistic, 0, sizeof(window_tasks_statistic));
	memset(&hook_uptime_statistic, 0, sizeof(hook_uptime_statistic));
	memset(&window_uptime_statistic, 0, sizeof(window_uptime_statistic));
	memset(task_registry, 0, sizeof(task_registry));
	task_registry_count = 0U;
	idle_task_iterator = ZP_UNINIT;
	idle_task_id = 0U;
	tasks_enumerated = false;
	window_start_timestamp = 0U;

	memset(&history_handler, 0, sizeof(history_handler));
	memset(history_snapshot, 0, sizeof(history_snapshot));
	history_index = 0U;
	history_size = 0U;
	history_snapshot_size = 0U;

	fake_rtos_reset();
	main_thread = NULL;
	worker_thread = NULL;
	idle_thread = NULL;
}

/* Create main, worker and idle, run zperf init at start_us with main running */
static TEST_UNUSED void zperf_test_start(uint32_t start_us)
{
	main_thread = fake_thread_create("main", false);
	worker_thread = fake_thread_create("worker", false);
	idle_thread = fake_thread_create("idle", true);

	fake_set_current(main_thread);
	fake_set_time(start_us);
	zperf_pre_scheduler_init();
	zperf_post_scheduler_init();
	fake_log_clear();
}

static TEST_UNUSED uint8_t task_of(const struct fake_thread *thread)
{
	return zperf_task_lookup((zp_task_id)thread);
}

/* Statistic of the last finished window for a thread */
static TEST_UNUSED const zp_task_runtime *window_of(const struct fake_thread *thread)
{
	uint8_t task_iterator = task_of(thread);

	TEST_ASSERT_TRUE_MESSAGE(TASK_IS_VALID(task_iterator), "thread is not tracked by zperf");
	return &window_tasks_statistic[task_iterator];
}

/* Close the current window and return its length */
static TEST_UNUSED uint32_t zperf_test_window(void)
{
	uint32_t start = window_start_timestamp;

	fake_log_clear();
	zperf_runtime_overall_statistic();
	return zperf_get_timestamp_delta(start, window_start_timestamp);
}

/* Read the "CPU LOAD x% ### IDLE y% ### KERNEL z%" line of the last window */
static TEST_UNUSED void zperf_test_cpu_line(int *cpu, int *idle, int *kernel)
{
	int line = fake_log_find("CPU LOAD", 0);
	const char *text;

	TEST_ASSERT_TRUE_MESSAGE(line >= 0, "no CPU LOAD line printed");
	text = strstr(fake_log_line(line), "CPU LOAD");
	TEST_ASSERT_EQUAL_INT(3, sscanf(text, "CPU LOAD %d%% ### IDLE %d%% ### KERNEL %d%%", cpu,
					idle, kernel));
}

/* Rows of the last printed history table, as timestamps; returns the row count */
static TEST_UNUSED int zperf_test_history_rows(uint32_t *timestamps, int max_rows)
{
	int header = fake_log_find("Task Name", 0);
	int rows = 0;

	TEST_ASSERT_TRUE_MESSAGE(header >= 0, "no history table printed");
	for (int i = header + 1; i < fake_log_count(); i++) {
		char name[32];
		char state[16];
		unsigned int timestamp;

		if (sscanf(fake_log_line(i), "# %31s %15s %u", name, state, &timestamp) != 3) {
			break; /* footer */
		}
		if (rows < max_rows) {
			timestamps[rows] = timestamp;
		}
		rows++;
	}
	return rows;
}

#endif /* ZPERF_TEST_SUPPORT_H */
