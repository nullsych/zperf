/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Task history: circular buffer, getters, snapshot and the printed table.
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

static void test_getters_on_an_empty_history(void)
{
	history_init();

	TEST_ASSERT_EQUAL_UINT8(HISTORY_STATE_NONE, history_get_last_state());
	TEST_ASSERT_TRUE(history_get_last_task() == 0U);
	TEST_ASSERT_EQUAL_UINT32(0, history_get_last_timestamp());
}

static void test_getters_return_the_last_written_event(void)
{
	zperf_test_start(1000);
	fake_advance(100);
	fake_switch_to(worker_thread);

	/* Switch out of main, then in to worker: the last event is worker IN */
	TEST_ASSERT_EQUAL_UINT8(TASK_STATE_ENTER, history_get_last_state());
	TEST_ASSERT_TRUE(history_get_last_task() == (zp_task_id)worker_thread);
	TEST_ASSERT_EQUAL_UINT32(1100, history_get_last_timestamp());
}

static void test_wrapped_history_is_printed_oldest_first(void)
{
	uint32_t timestamps[ZP_HISTORY_SIZE + 4];
	int rows;

	zperf_test_start(1000);
	/* 6 switches = 12 events, more than the buffer holds */
	for (int i = 0; i < 6; i++) {
		fake_advance(10);
		fake_switch_to((i % 2 == 0) ? worker_thread : main_thread);
	}
	zperf_test_window();

	rows = zperf_test_history_rows(timestamps, (int)(sizeof(timestamps) / sizeof(timestamps[0])));
	TEST_ASSERT_EQUAL_INT(ZP_HISTORY_SIZE, rows);
	for (int i = 1; i < rows; i++) {
		TEST_ASSERT_TRUE_MESSAGE(timestamps[i - 1] <= timestamps[i], "history out of order");
	}
	/* The oldest events that still fit, up to the last switch */
	TEST_ASSERT_EQUAL_UINT32(1060 - 10 * (ZP_HISTORY_SIZE / 2 - 1), timestamps[0]);
	TEST_ASSERT_EQUAL_UINT32(1060, timestamps[rows - 1]);
}

static void test_window_starts_a_new_history(void)
{
	uint32_t timestamps[ZP_HISTORY_SIZE];

	zperf_test_start(1000);
	fake_advance(10);
	fake_switch_to(worker_thread);
	zperf_test_window();
	TEST_ASSERT_EQUAL_UINT8(HISTORY_STATE_NONE, history_get_last_state());

	fake_advance(10);
	fake_switch_to(main_thread);
	zperf_test_window();
	TEST_ASSERT_EQUAL_INT(2, zperf_test_history_rows(timestamps, ZP_HISTORY_SIZE));
	TEST_ASSERT_EQUAL_UINT32(1020, timestamps[0]);
}

static void test_on_demand_trace_keeps_the_history(void)
{
	uint32_t timestamps[ZP_HISTORY_SIZE];

	zperf_test_start(1000);
	fake_advance(10);
	fake_switch_to(worker_thread);

	for (int i = 0; i < 2; i++) {
		fake_log_clear();
		zperf_trace_history();
		TEST_ASSERT_EQUAL_INT(2, zperf_test_history_rows(timestamps, ZP_HISTORY_SIZE));
	}
	TEST_ASSERT_EQUAL_UINT8(TASK_STATE_ENTER, history_get_last_state());
}

static void test_table_is_aligned_and_long_names_are_truncated(void)
{
	struct fake_thread *long_named;
	int header;
	int footer;
	size_t width;

	main_thread = fake_thread_create("main", false);
	long_named = fake_thread_create("a_very_long_thread_name_over_20", false);
	fake_set_current(main_thread);
	fake_set_time(1000);
	zperf_pre_scheduler_init();
	zperf_post_scheduler_init();

	fake_advance(10);
	fake_switch_to(long_named);
	zperf_test_window();

	header = fake_log_find("Collected History", 0);
	footer = fake_log_find("# ######", header + 1);
	TEST_ASSERT_TRUE(header >= 0);
	TEST_ASSERT_TRUE(footer > header);

	/* Title, blank line, column header, rows and footer all have the same width */
	width = strlen(fake_log_line(header));
	for (int i = header; i <= footer; i++) {
		TEST_ASSERT_EQUAL_size_t_MESSAGE(width, strlen(fake_log_line(i)), fake_log_line(i));
	}

	TEST_ASSERT_TRUE(fake_log_find("a_very_long_thread_n ", header) >= 0);
	TEST_ASSERT_EQUAL_INT(-1, fake_log_find("a_very_long_thread_name_over_20", header));
}

int main(void)
{
	UNITY_BEGIN();
	RUN_TEST(test_getters_on_an_empty_history);
	RUN_TEST(test_getters_return_the_last_written_event);
	RUN_TEST(test_wrapped_history_is_printed_oldest_first);
	RUN_TEST(test_window_starts_a_new_history);
	RUN_TEST(test_on_demand_trace_keeps_the_history);
	RUN_TEST(test_table_is_aligned_and_long_names_are_truncated);
	return UNITY_END();
}
