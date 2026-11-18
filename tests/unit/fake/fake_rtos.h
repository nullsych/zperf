/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Fake RTOS for host unit tests. It implements the zperf wrapper API (wr_*) and the IRQ lock,
 * owns a simulated clock and drives the zperf hooks through the same sys_trace_*_user()
 * functions Zephyr calls, so tests describe a schedule and check what zperf booked.
 */

#ifndef ZPERF_TEST_FAKE_RTOS_H
#define ZPERF_TEST_FAKE_RTOS_H

#include <stdbool.h>
#include <stdint.h>

#define FAKE_MAX_THREADS   16
#define FAKE_LOG_MAX_LINES 512
#define FAKE_LOG_LINE_LEN  192

struct fake_thread {
	const char *name;
	bool is_idle;
};

/* Forget all threads, logs and the IRQ lock state, and set the clock to 0 */
void fake_rtos_reset(void);

/* Create a thread. Threads that exist when zperf_post_scheduler_init() runs are enumerated */
struct fake_thread *fake_thread_create(const char *name, bool is_idle);

/* The thread the fake scheduler runs now, without calling any hook */
void fake_set_current(struct fake_thread *thread);

/* Simulated clock in microseconds, as returned by wr_get_uptime_timestamp() */
void fake_set_time(uint32_t now_us);
void fake_advance(uint32_t delta_us);
uint32_t fake_now(void);

/* Context switch: switched-out hook for the current thread, then switched-in for the next */
void fake_switch_to(struct fake_thread *next);

/* An interrupt of duration_us on top of the current thread: ISR enter and exit hooks */
void fake_isr(uint32_t duration_us);

/* The idle trace hook, as Zephyr calls it from the idle loop and from k_cpu_atomic_idle() */
void fake_idle_hook(void);

/* Mark code as running in ISR context, without calling the ISR hooks */
void fake_set_in_isr(bool value);

/* IRQ lock bookkeeping */
int fake_irq_depth(void);
int fake_irq_max_depth(void);

/* Captured log output, one entry per log call, trailing newlines removed */
int fake_log_count(void);
const char *fake_log_line(int index);
int fake_log_find(const char *needle, int from);
void fake_log_clear(void);
void fake_log_dump(void);

#endif /* ZPERF_TEST_FAKE_RTOS_H */
