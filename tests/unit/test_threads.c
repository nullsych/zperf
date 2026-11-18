/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * zperf's own thread table: startup enumeration, threads created later, a full table.
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

static void test_startup_threads_get_ids_in_enumeration_order(void)
{
	zperf_test_start(1000);

	TEST_ASSERT_EQUAL_UINT8(3, task_registry_count);
	TEST_ASSERT_EQUAL_UINT8(0, task_of(main_thread));
	TEST_ASSERT_EQUAL_UINT8(1, task_of(worker_thread));
	TEST_ASSERT_EQUAL_UINT8(2, task_of(idle_thread));
}

static void test_thread_ids_keep_the_full_pointer(void)
{
	zperf_test_start(1000);

	/* zp_task_id must hold a pointer on 64-bit hosts (and native_sim/native/64) */
	TEST_ASSERT_EQUAL_size_t(sizeof(void *), sizeof(zp_task_id));
	TEST_ASSERT_TRUE(task_registry[0] == (zp_task_id)main_thread);
	TEST_ASSERT_TRUE((const struct fake_thread *)task_registry[0] == main_thread);
}

static void test_thread_created_later_gets_its_own_slot(void)
{
	struct fake_thread *late;

	zperf_test_start(1000);
	late = fake_thread_create("late", false);

	fake_advance(100);
	fake_switch_to(late);
	fake_advance(40);
	fake_switch_to(main_thread);
	zperf_test_window();

	TEST_ASSERT_EQUAL_UINT8(3, task_of(late));
	TEST_ASSERT_EQUAL_UINT32(40, window_of(late)->total_runtime);
	TEST_ASSERT_EQUAL_UINT32(1, window_of(late)->task_switches);
	/* Task 0 only holds main's own time; late threads used to be booked here */
	TEST_ASSERT_EQUAL_UINT32(100, window_of(main_thread)->total_runtime);
}

static void test_thread_beyond_a_full_table_is_not_tracked(void)
{
	struct fake_thread *overflow;
	int cpu;
	int idle;
	int kernel;

	zperf_test_start(1000);

	/* Fill the remaining slots with threads created after startup */
	for (int i = 3; i < ZP_TASK_COUNT; i++) {
		fake_switch_to(fake_thread_create("late", false));
	}
	fake_switch_to(main_thread);
	TEST_ASSERT_EQUAL_UINT8(ZP_TASK_COUNT, task_registry_count);

	overflow = fake_thread_create("overflow", false);
	fake_switch_to(overflow);
	fake_advance(200);
	fake_switch_to(main_thread);

	TEST_ASSERT_EQUAL_UINT32(200, zperf_test_window());
	TEST_ASSERT_EQUAL_UINT8(ZP_UNINIT, task_of(overflow));
	for (uint8_t task_iterator = 0; task_iterator < ZP_TASK_COUNT; task_iterator++) {
		TEST_ASSERT_EQUAL_UINT32(0, window_tasks_statistic[task_iterator].total_runtime);
	}
	/* Time of an untracked thread is reported as KERNEL instead of polluting a task */
	zperf_test_cpu_line(&cpu, &idle, &kernel);
	TEST_ASSERT_EQUAL_INT(100, kernel);
}

int main(void)
{
	UNITY_BEGIN();
	RUN_TEST(test_startup_threads_get_ids_in_enumeration_order);
	RUN_TEST(test_thread_ids_keep_the_full_pointer);
	RUN_TEST(test_thread_created_later_gets_its_own_slot);
	RUN_TEST(test_thread_beyond_a_full_table_is_not_tracked);
	return UNITY_END();
}
