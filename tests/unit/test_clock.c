/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * zp_clock: cycle counter to microsecond uptime, kept in step with the tick clock.
 */

#include <stdbool.h>
#include <stdint.h>

#include "unity.h"
#include "zperf_clock.h"

/* nRF52: DWT at 64 MHz, 32768 Hz kernel ticks of 30.52 us, rounded up */
#define DWT_HZ      64000000ULL
#define TICK_HZ     32768U
#define TICK_LEN_US 31U

static zp_clock clock;

void setUp(void)
{
}

void tearDown(void)
{
}

static void test_whole_mhz_counter_carries_the_remainder(void)
{
	zp_clock_init(&clock, DWT_HZ, TICK_LEN_US, 1000);
	TEST_ASSERT_NOT_EQUAL_UINT32(0, clock.cycles_per_us);

	TEST_ASSERT_EQUAL_UINT32(1010, zp_clock_update(&clock, 64 * 10, 1000));
	/* Half a microsecond twice adds up to one */
	TEST_ASSERT_EQUAL_UINT32(1010, zp_clock_update(&clock, 32, 1000));
	TEST_ASSERT_EQUAL_UINT32(1011, zp_clock_update(&clock, 32, 1000));
}

static void test_other_counter_rates_use_exact_64_bit_math(void)
{
	/* 1.5 MHz is not a whole number of MHz: 3 cycles are exactly 2 us */
	zp_clock_init(&clock, 1500000ULL, 1000000U, 0);
	TEST_ASSERT_EQUAL_UINT32(0, clock.cycles_per_us);

	TEST_ASSERT_EQUAL_UINT32(2, zp_clock_update(&clock, 3, 0));
	TEST_ASSERT_EQUAL_UINT32(2, zp_clock_update(&clock, 1, 0));
	TEST_ASSERT_EQUAL_UINT32(3, zp_clock_update(&clock, 1, 0));

	/* A 32768 Hz counter, as the generic timing functions fall back to on nRF */
	zp_clock_init(&clock, 32768ULL, 1000000U, 0);
	TEST_ASSERT_EQUAL_UINT32(500000, zp_clock_update(&clock, 16384, 0));
}

static void test_catches_up_after_the_counter_stopped_in_sleep(void)
{
	zp_clock_init(&clock, DWT_HZ, TICK_LEN_US, 1000);
	TEST_ASSERT_EQUAL_UINT32(1010, zp_clock_update(&clock, 64 * 10, 1000));

	/* CPU slept for many ticks: the counter barely moved, the tick clock did */
	TEST_ASSERT_EQUAL_UINT32(5000, zp_clock_update(&clock, 64, 5000));
	/* And continues at counter resolution from there */
	TEST_ASSERT_EQUAL_UINT32(5007, zp_clock_update(&clock, 64 * 7, 5000));
}

static void test_never_runs_a_full_tick_ahead(void)
{
	zp_clock_init(&clock, DWT_HZ, TICK_LEN_US, 1000);

	TEST_ASSERT_EQUAL_UINT32(1000 + TICK_LEN_US - 1, zp_clock_update(&clock, 64 * 100, 1000));
	/* An unconvertible gap (a missed wrap of a 64-bit counter) stays within the tick too */
	TEST_ASSERT_EQUAL_UINT32(2000 + TICK_LEN_US - 1, zp_clock_update(&clock, 1ULL << 40, 2000));
}

static void test_microseconds_wrap_at_32_bits(void)
{
	zp_clock_init(&clock, DWT_HZ, TICK_LEN_US, UINT32_MAX - 5U);

	TEST_ASSERT_EQUAL_UINT32(4, zp_clock_update(&clock, 64 * 10, UINT32_MAX - 5U));
	/* The tick clock wrapped as well */
	TEST_ASSERT_EQUAL_UINT32(20, zp_clock_update(&clock, 0, 20));
}

static void test_zero_rate_falls_back_to_the_tick_clock(void)
{
	zp_clock_init(&clock, 0, TICK_LEN_US, 1000);

	TEST_ASSERT_EQUAL_UINT32(1000, zp_clock_update(&clock, 12345, 1000));
	TEST_ASSERT_EQUAL_UINT32(2000, zp_clock_update(&clock, 12345, 2000));
}

/* Small deterministic PRNG so failures reproduce */
static uint32_t prng_state = 12345U;

static uint32_t prng(uint32_t bound)
{
	prng_state = prng_state * 1664525U + 1013904223U;
	return (prng_state >> 8) % bound;
}

static void test_simulated_sleep_and_drift_stay_monotonic_and_within_a_tick(void)
{
	/* The counter oscillator runs 2% fast relative to the tick oscillator */
	const uint64_t counter_hz = DWT_HZ * 102U / 100U;
	uint64_t real_ns = 1000000000ULL; /* start at 1 s uptime */
	uint64_t counter = 0U;
	uint32_t last = 0U;

	zp_clock_init(&clock, DWT_HZ, TICK_LEN_US,
		      (uint32_t)((real_ns * TICK_HZ / 1000000000ULL) * 1000000ULL / TICK_HZ));
	last = clock.now_us;

	for (int step = 0; step < 20000; step++) {
		uint64_t gap_ns = 200U + prng(50000U);
		bool asleep = (prng(4U) == 0U);
		uint64_t ticks;
		uint32_t tick_us;
		uint32_t now;
		uint64_t real_us;

		real_ns += gap_ns;
		if (!asleep) {
			counter += gap_ns * counter_hz / 1000000000ULL;
		}

		ticks = real_ns * TICK_HZ / 1000000000ULL;
		tick_us = (uint32_t)(ticks * 1000000ULL / TICK_HZ);
		now = zp_clock_update(&clock, counter, tick_us);
		counter = 0U;
		real_us = real_ns / 1000U;

		TEST_ASSERT_TRUE_MESSAGE((int32_t)(now - last) >= 0, "clock went backwards");
		TEST_ASSERT_TRUE_MESSAGE((int32_t)(now - tick_us) >= 0, "behind the tick clock");
		TEST_ASSERT_TRUE_MESSAGE((now - tick_us) < TICK_LEN_US, "a full tick ahead");
		/* Whatever the counter does, the error against real time is at most one tick */
		TEST_ASSERT_UINT64_WITHIN(TICK_LEN_US, real_us, now);
		last = now;
	}
}

int main(void)
{
	UNITY_BEGIN();
	RUN_TEST(test_whole_mhz_counter_carries_the_remainder);
	RUN_TEST(test_other_counter_rates_use_exact_64_bit_math);
	RUN_TEST(test_catches_up_after_the_counter_stopped_in_sleep);
	RUN_TEST(test_never_runs_a_full_tick_ahead);
	RUN_TEST(test_microseconds_wrap_at_32_bits);
	RUN_TEST(test_zero_rate_falls_back_to_the_tick_clock);
	RUN_TEST(test_simulated_sleep_and_drift_stay_monotonic_and_within_a_tick);
	return UNITY_END();
}
