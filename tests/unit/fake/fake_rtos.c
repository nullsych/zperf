/*
 * SPDX-License-Identifier: Apache-2.0
 */

#include "fake_rtos.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "rtos_hooks.h"
#include "wrapper.h"
#include "zperf_internal.h"

static struct fake_thread threads[FAKE_MAX_THREADS];
static int thread_count;
static struct fake_thread *current;
static bool in_isr;
static uint32_t now_us;

static int irq_depth;
static int irq_max_depth;

static char log_lines[FAKE_LOG_MAX_LINES][FAKE_LOG_LINE_LEN];
static int log_count;

void fake_rtos_reset(void)
{
	memset(threads, 0, sizeof(threads));
	thread_count = 0;
	current = NULL;
	in_isr = false;
	now_us = 0U;
	irq_depth = 0;
	irq_max_depth = 0;
	fake_log_clear();
}

struct fake_thread *fake_thread_create(const char *name, bool is_idle)
{
	if (thread_count >= FAKE_MAX_THREADS) {
		return NULL;
	}

	threads[thread_count].name = name;
	threads[thread_count].is_idle = is_idle;
	return &threads[thread_count++];
}

void fake_set_current(struct fake_thread *thread)
{
	current = thread;
}

void fake_set_time(uint32_t time_us)
{
	now_us = time_us;
}

void fake_advance(uint32_t delta_us)
{
	/* Wraps around like the real 32-bit microsecond timestamp */
	now_us += delta_us;
}

uint32_t fake_now(void)
{
	return now_us;
}

void fake_switch_to(struct fake_thread *next)
{
	sys_trace_thread_switched_out_user();
	current = next;
	sys_trace_thread_switched_in_user();
}

void fake_isr(uint32_t duration_us)
{
	in_isr = true;
	sys_trace_isr_enter_user();
	fake_advance(duration_us);
	sys_trace_isr_exit_user();
	in_isr = false;
}

void fake_idle_hook(void)
{
	sys_trace_idle_user();
}

void fake_set_in_isr(bool value)
{
	in_isr = value;
}

/* ---------------------------------------------------------------------------------------------
 * zperf wrapper API
 * ------------------------------------------------------------------------------------------- */

zp_task_id wr_get_task_id(void)
{
	return (zp_task_id)current;
}

void wr_init_configuration(void)
{
	for (int i = 0; i < thread_count; i++) {
		(void)zperf_task_register((zp_task_id)&threads[i]);
	}
}

const char *wr_get_task_name(zp_task_id task_id)
{
	const struct fake_thread *thread = (const struct fake_thread *)task_id;

	return (thread != NULL) ? thread->name : NULL;
}

uint32_t wr_get_uptime_timestamp(void)
{
	return now_us;
}

bool wr_is_idle_task(zp_task_id task_id)
{
	const struct fake_thread *thread = (const struct fake_thread *)task_id;

	/* Same rule as the Zephyr wrapper: the idle thread, and only in thread context */
	return !in_isr && (thread != NULL) && thread->is_idle;
}

/* ---------------------------------------------------------------------------------------------
 * IRQ lock
 * ------------------------------------------------------------------------------------------- */

unsigned int irq_lock(void)
{
	unsigned int key = (unsigned int)irq_depth;

	irq_depth++;
	if (irq_depth > irq_max_depth) {
		irq_max_depth = irq_depth;
	}
	return key;
}

void irq_unlock(unsigned int key)
{
	irq_depth--;
	/* Keys must be released in reverse order */
	if ((unsigned int)irq_depth != key) {
		fprintf(stderr, "fake_rtos: irq_unlock key %u at depth %d\n", key, irq_depth);
		irq_depth = -1000;
	}
}

int fake_irq_depth(void)
{
	return irq_depth;
}

int fake_irq_max_depth(void)
{
	return irq_max_depth;
}

/* ---------------------------------------------------------------------------------------------
 * Log capture
 * ------------------------------------------------------------------------------------------- */

void fake_log(const char *level, const char *fmt, ...)
{
	va_list args;
	char *line;
	size_t len;

	(void)level;
	if (log_count >= FAKE_LOG_MAX_LINES) {
		return;
	}

	line = log_lines[log_count++];
	va_start(args, fmt);
	vsnprintf(line, FAKE_LOG_LINE_LEN, fmt, args);
	va_end(args);

	len = strlen(line);
	while ((len > 0U) && (line[len - 1U] == '\n')) {
		line[--len] = '\0';
	}
}

int fake_log_count(void)
{
	return log_count;
}

const char *fake_log_line(int index)
{
	return ((index >= 0) && (index < log_count)) ? log_lines[index] : NULL;
}

int fake_log_find(const char *needle, int from)
{
	for (int i = (from < 0) ? 0 : from; i < log_count; i++) {
		if (strstr(log_lines[i], needle) != NULL) {
			return i;
		}
	}
	return -1;
}

void fake_log_clear(void)
{
	log_count = 0;
}

void fake_log_dump(void)
{
	for (int i = 0; i < log_count; i++) {
		printf("  | %s\n", log_lines[i]);
	}
}
