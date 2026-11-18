/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Minimal stand-in for <zephyr/kernel.h> for host unit tests.
 * Provides only what the zperf core (src/, map/rtos_hooks.c) uses.
 */

#ifndef ZPERF_TEST_STUB_ZEPHYR_KERNEL_H
#define ZPERF_TEST_STUB_ZEPHYR_KERNEL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define __packed     __attribute__((__packed__))
#define __aligned(x) __attribute__((__aligned__(x)))

#define ARG_UNUSED(x) (void)(x)

#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

/* Opaque: the fake RTOS uses thread pointers as IDs and never dereferences them in the core */
struct k_thread;

/* Implemented by the fake RTOS, which tracks the lock depth */
unsigned int irq_lock(void);
void irq_unlock(unsigned int key);

#endif /* ZPERF_TEST_STUB_ZEPHYR_KERNEL_H */
