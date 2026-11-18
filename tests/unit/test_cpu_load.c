/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * The CPU LOAD / IDLE / KERNEL summary line and the percentage math behind it.
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

static void test_load_and_idle_percentages(void)
{
	int cpu;
	int idle;
	int kernel;

	zperf_test_start(1000);
	fake_advance(250);
	fake_switch_to(idle_thread);
	fake_idle_hook();
	fake_advance(750);
	fake_switch_to(main_thread);

	TEST_ASSERT_EQUAL_UINT32(1000, zperf_test_window());
	zperf_test_cpu_line(&cpu, &idle, &kernel);
	TEST_ASSERT_EQUAL_INT(25, cpu);
	TEST_ASSERT_EQUAL_INT(75, idle);
	TEST_ASSERT_EQUAL_INT(0, kernel);
}

static void test_kernel_is_clamped_when_booked_runtime_exceeds_the_window(void)
{
	uint32_t time_slice = 1000U;
	int cpu;
	int idle;
	int kernel;

	/* Tick rounding can book slightly more than the window. KERNEL used to wrap around */
	window_tasks_statistic[0].total_runtime = 700U;
	window_tasks_statistic[1].total_runtime = 400U;
	idle_task_iterator = 2U;

	zperf_trace_cpu_usage(&time_slice);
	zperf_test_cpu_line(&cpu, &idle, &kernel);
	TEST_ASSERT_EQUAL_INT(100, cpu);
	TEST_ASSERT_EQUAL_INT(0, idle);
	TEST_ASSERT_EQUAL_INT(0, kernel);
}

static void test_untracked_time_shows_up_as_kernel(void)
{
	uint32_t time_slice = 1000U;
	int cpu;
	int idle;
	int kernel;

	window_tasks_statistic[0].total_runtime = 300U;
	window_tasks_statistic[2].total_runtime = 500U;
	idle_task_iterator = 2U;

	zperf_trace_cpu_usage(&time_slice);
	zperf_test_cpu_line(&cpu, &idle, &kernel);
	TEST_ASSERT_EQUAL_INT(30, cpu);
	TEST_ASSERT_EQUAL_INT(50, idle);
	TEST_ASSERT_EQUAL_INT(20, kernel);
}

static void test_cpu_percent_math(void)
{
	/* Empty window */
	TEST_ASSERT_EQUAL_UINT32(0, GET_CPU_PERCENT(0U, 123U));
	TEST_ASSERT_EQUAL_UINT32(50, GET_CPU_PERCENT(1000U, 500U));
	/* total * 100 doesn't fit in 32 bits: needs the 64-bit math */
	TEST_ASSERT_EQUAL_UINT32(50, GET_CPU_PERCENT(100000000U, 50000000U));
	TEST_ASSERT_EQUAL_UINT32(99, GET_CPU_PERCENT(UINT32_MAX, UINT32_MAX - (UINT32_MAX / 100U)));
	TEST_ASSERT_EQUAL_UINT32(100, GET_CPU_PERCENT(UINT32_MAX, UINT32_MAX));
}

int main(void)
{
	UNITY_BEGIN();
	RUN_TEST(test_load_and_idle_percentages);
	RUN_TEST(test_kernel_is_clamped_when_booked_runtime_exceeds_the_window);
	RUN_TEST(test_untracked_time_shows_up_as_kernel);
	RUN_TEST(test_cpu_percent_math);
	return UNITY_END();
}
