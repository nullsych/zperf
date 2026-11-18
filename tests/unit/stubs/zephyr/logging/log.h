/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Stand-in for <zephyr/logging/log.h>: log calls are captured by the fake RTOS so tests can
 * check what zperf prints.
 */

#ifndef ZPERF_TEST_STUB_ZEPHYR_LOG_H
#define ZPERF_TEST_STUB_ZEPHYR_LOG_H

#define LOG_MODULE_REGISTER(...)

void fake_log(const char *level, const char *fmt, ...) __attribute__((format(printf, 2, 3)));

#define LOG_INF(...) fake_log("inf", __VA_ARGS__)
#define LOG_WRN(...) fake_log("wrn", __VA_ARGS__)
#define LOG_ERR(...) fake_log("err", __VA_ARGS__)

#endif /* ZPERF_TEST_STUB_ZEPHYR_LOG_H */
