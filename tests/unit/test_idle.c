/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Idle thread detection and idle accounting around interrupts.
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

static void test_interrupts_during_idle_are_not_switches(void)
{
	zperf_test_start(1000);
	fake_switch_to(idle_thread);
	fake_idle_hook();

	/* Three interrupts hit idle; idle is split at each ISR entry but never switched out */
	for (int i = 0; i < 3; i++) {
		fake_advance(90);
		fake_isr(10);
	}
	fake_advance(170);
	fake_switch_to(main_thread);

	/* Pieces booked to idle: 90, 100, 100 at the ISR entries, then 180 on switch out */
	TEST_ASSERT_EQUAL_UINT32(470, zperf_test_window());
	TEST_ASSERT_EQUAL_UINT32(470, window_of(idle_thread)->total_runtime);
	/* Was 4 when every interrupt that hit idle counted as a switch */
	TEST_ASSERT_EQUAL_UINT32(1, window_of(idle_thread)->task_switches);
	/* Longest stretch between ISR entries: the last ISR (10) plus 170 */
	TEST_ASSERT_EQUAL_UINT32(180, window_of(idle_thread)->max_runtime);
}

static void test_idle_hook_from_another_thread_is_ignored(void)
{
	/* Zephyr also calls the idle hook from drivers via k_cpu_atomic_idle() */
	zperf_test_start(1000);
	fake_switch_to(worker_thread);
	fake_idle_hook();

	TEST_ASSERT_EQUAL_UINT8(TASK_STATE_ENTER, history_get_last_state());
	TEST_ASSERT_EQUAL_UINT8(ZP_UNINIT, idle_task_iterator);
}

static void test_idle_hook_in_isr_context_is_ignored(void)
{
	zperf_test_start(1000);
	fake_switch_to(idle_thread);

	fake_set_in_isr(true);
	fake_idle_hook();
	fake_set_in_isr(false);

	TEST_ASSERT_EQUAL_UINT8(TASK_STATE_ENTER, history_get_last_state());
	TEST_ASSERT_EQUAL_UINT8(ZP_UNINIT, idle_task_iterator);
}

static void test_idle_is_found_by_thread_not_by_first_caller(void)
{
	zperf_test_start(1000);

	/* A non-idle caller first must not be cached as the idle thread */
	fake_idle_hook();
	fake_switch_to(idle_thread);
	fake_idle_hook();

	TEST_ASSERT_EQUAL_UINT8(task_of(idle_thread), idle_task_iterator);
	TEST_ASSERT_EQUAL_UINT8(IDLE_STATE_RUNNING, history_get_last_state());
	TEST_ASSERT_TRUE(history_get_last_task() == (zp_task_id)idle_thread);
}

static void test_idle_history_rows_carry_the_thread_name(void)
{
	int line;

	zperf_test_start(1000);
	fake_switch_to(idle_thread);
	fake_idle_hook();
	fake_advance(100);
	zperf_test_window();

	line = fake_log_find("IDLE ", fake_log_find("Task Name", 0));
	TEST_ASSERT_TRUE_MESSAGE(line >= 0, "no IDLE row in the history");
	TEST_ASSERT_EQUAL_STRING_LEN("# idle ", fake_log_line(line), 7);
}

int main(void)
{
	UNITY_BEGIN();
	RUN_TEST(test_interrupts_during_idle_are_not_switches);
	RUN_TEST(test_idle_hook_from_another_thread_is_ignored);
	RUN_TEST(test_idle_hook_in_isr_context_is_ignored);
	RUN_TEST(test_idle_is_found_by_thread_not_by_first_caller);
	RUN_TEST(test_idle_history_rows_carry_the_thread_name);
	return UNITY_END();
}
