/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Per-window runtime accounting: slices, switches, window boundaries and the uptime counters.
 */

#include "zperf.c"
#include "zperf_history.c"

#include "zperf_test_support.h"

void setUp(void)
{
	zperf_test_reset();
}

void tearDown(void)
{
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, fake_irq_depth(), "IRQ lock left unbalanced");
}

static void test_switches_book_runtime_max_and_count(void)
{
	zperf_test_start(1000);

	fake_advance(100);
	fake_switch_to(worker_thread);
	fake_advance(300);
	fake_switch_to(main_thread);
	fake_advance(50);
	fake_switch_to(worker_thread);
	fake_advance(200);
	fake_switch_to(main_thread);
	fake_advance(10);

	TEST_ASSERT_EQUAL_UINT32(660, zperf_test_window());

	/* main: 100 + 50 + the 10 still running at the window end */
	TEST_ASSERT_EQUAL_UINT32(160, window_of(main_thread)->total_runtime);
	TEST_ASSERT_EQUAL_UINT32(100, window_of(main_thread)->max_runtime);
	TEST_ASSERT_EQUAL_UINT32(2, window_of(main_thread)->task_switches);

	TEST_ASSERT_EQUAL_UINT32(500, window_of(worker_thread)->total_runtime);
	TEST_ASSERT_EQUAL_UINT32(300, window_of(worker_thread)->max_runtime);
	TEST_ASSERT_EQUAL_UINT32(2, window_of(worker_thread)->task_switches);
}

static void test_open_slice_is_split_at_window_end_without_a_switch(void)
{
	zperf_test_start(1000);

	fake_advance(500);
	TEST_ASSERT_EQUAL_UINT32(500, zperf_test_window());
	TEST_ASSERT_EQUAL_UINT32(500, window_of(main_thread)->total_runtime);
	TEST_ASSERT_EQUAL_UINT32(0, window_of(main_thread)->task_switches);

	/* Only the remainder after the window end belongs to the next window */
	fake_advance(200);
	fake_switch_to(worker_thread);
	fake_advance(100);
	TEST_ASSERT_EQUAL_UINT32(300, zperf_test_window());
	TEST_ASSERT_EQUAL_UINT32(200, window_of(main_thread)->total_runtime);
	TEST_ASSERT_EQUAL_UINT32(1, window_of(main_thread)->task_switches);
	TEST_ASSERT_EQUAL_UINT32(100, window_of(worker_thread)->total_runtime);
	TEST_ASSERT_EQUAL_UINT32(0, window_of(worker_thread)->task_switches);
}

static void test_tracked_runtime_adds_up_to_the_window(void)
{
	uint32_t slice;
	uint32_t booked = 0U;

	zperf_test_start(1000);
	for (int i = 0; i < 20; i++) {
		fake_advance(37U + (uint32_t)i);
		fake_switch_to((i % 2 == 0) ? worker_thread : main_thread);
	}
	fake_advance(5);

	slice = zperf_test_window();
	for (uint8_t task_iterator = 0; task_iterator < ZP_TASK_COUNT; task_iterator++) {
		booked += window_tasks_statistic[task_iterator].total_runtime;
	}
	TEST_ASSERT_EQUAL_UINT32(slice, booked);
}

static void test_first_window_starts_at_init_not_at_boot(void)
{
	zperf_test_start(5000);

	fake_advance(100);
	TEST_ASSERT_EQUAL_UINT32(100, zperf_test_window());
	TEST_ASSERT_TRUE(fake_log_find("Timeslice: 100", 0) >= 0);
}

static void test_hooks_before_init_are_ignored(void)
{
	main_thread = fake_thread_create("main", false);
	worker_thread = fake_thread_create("worker", false);
	idle_thread = fake_thread_create("idle", true);
	fake_set_current(main_thread);
	fake_set_time(1000);
	zperf_pre_scheduler_init();

	/* Scheduling before zperf_post_scheduler_init() must not register or book anything */
	fake_advance(100);
	fake_switch_to(worker_thread);
	fake_idle_hook();
	fake_advance(100);
	fake_switch_to(main_thread);
	TEST_ASSERT_EQUAL_UINT8(0, task_registry_count);
	TEST_ASSERT_EQUAL_UINT8(HISTORY_STATE_NONE, history_get_last_state());

	zperf_post_scheduler_init();
	/* Enumeration order, not the order threads happened to run in */
	TEST_ASSERT_EQUAL_UINT8(0, task_of(main_thread));
	TEST_ASSERT_EQUAL_UINT8(1, task_of(worker_thread));

	fake_advance(100);
	TEST_ASSERT_EQUAL_UINT32(100, zperf_test_window());
	TEST_ASSERT_EQUAL_UINT32(100, window_of(main_thread)->total_runtime);
	TEST_ASSERT_EQUAL_UINT32(0, window_of(worker_thread)->total_runtime);
}

static void test_uptime_switches_increment_by_one(void)
{
	zperf_test_start(1000);

	for (int i = 0; i < 3; i++) {
		fake_advance(10);
		fake_switch_to(worker_thread);
		fake_advance(10);
		fake_switch_to(main_thread);
	}
	zperf_test_window();

	/* Was 1 + 2 + 3 = 6 when the accumulated counter was added on every switch */
	TEST_ASSERT_EQUAL_UINT32(3, window_uptime_statistic.tasks_switches[task_of(main_thread)]);
	TEST_ASSERT_EQUAL_UINT32(3,
				 window_uptime_statistic.tasks_switches[task_of(worker_thread)]);
}

static void test_uptime_max_survives_the_window_reset(void)
{
	zperf_test_start(1000);

	fake_advance(300);
	fake_switch_to(worker_thread);
	zperf_test_window();

	fake_switch_to(main_thread);
	fake_advance(100);
	fake_switch_to(worker_thread);
	zperf_test_window();

	TEST_ASSERT_EQUAL_UINT32(100, window_of(main_thread)->max_runtime);
	TEST_ASSERT_EQUAL_UINT32(300,
				 window_uptime_statistic.max_tasks_runtime[task_of(main_thread)]);
}

static void test_timestamp_wraparound(void)
{
	TEST_ASSERT_EQUAL_UINT32(16, zperf_get_timestamp_delta(UINT32_MAX - 10U, 5U));
	TEST_ASSERT_EQUAL_UINT32(0, zperf_get_timestamp_delta(42U, 42U));

	/* A window and a slice that both cross the 32-bit microsecond wrap */
	zperf_test_start(UINT32_MAX - 49U);
	fake_advance(100);
	fake_switch_to(worker_thread);
	fake_advance(20);

	TEST_ASSERT_EQUAL_UINT32(120, zperf_test_window());
	TEST_ASSERT_EQUAL_UINT32(100, window_of(main_thread)->total_runtime);
	TEST_ASSERT_EQUAL_UINT32(20, window_of(worker_thread)->total_runtime);
}

static void test_window_snapshot_runs_under_irq_lock(void)
{
	zperf_test_start(1000);
	fake_advance(10);
	zperf_test_window();

	TEST_ASSERT_GREATER_OR_EQUAL_INT(1, fake_irq_max_depth());
}

int main(void)
{
	UNITY_BEGIN();
	RUN_TEST(test_switches_book_runtime_max_and_count);
	RUN_TEST(test_open_slice_is_split_at_window_end_without_a_switch);
	RUN_TEST(test_tracked_runtime_adds_up_to_the_window);
	RUN_TEST(test_first_window_starts_at_init_not_at_boot);
	RUN_TEST(test_hooks_before_init_are_ignored);
	RUN_TEST(test_uptime_switches_increment_by_one);
	RUN_TEST(test_uptime_max_survives_the_window_reset);
	RUN_TEST(test_timestamp_wraparound);
	RUN_TEST(test_window_snapshot_runs_under_irq_lock);
	return UNITY_END();
}
