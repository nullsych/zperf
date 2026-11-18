/**
 * @file   zperf_clock.c
 * @brief  Microsecond uptime from a cycle counter, kept in step with the RTOS tick clock.
 *
 * See zperf_clock.h for the model.
 */

#include "zperf_clock.h"

#define ZP_CLOCK_US_PER_SEC (1000000U)

/* A longer gap between updates can't be converted reliably; the tick clock decides instead */
#define ZP_CLOCK_MAX_DELTA_CYCLES (UINT32_MAX / 2U)

void zp_clock_init(zp_clock *clock, uint64_t freq_hz, uint32_t tick_len_us, uint32_t tick_us)
{
	clock->now_us = tick_us;
	clock->tick_len_us = (tick_len_us != 0U) ? tick_len_us : 1U;
	clock->freq_hz = freq_hz;
	clock->rem = 0U;

	/* Whole MHz rates (e.g. DWT at 64 MHz) convert with 32-bit math, cheap enough for ISRs */
	if ((freq_hz >= ZP_CLOCK_US_PER_SEC) && ((freq_hz % ZP_CLOCK_US_PER_SEC) == 0U) &&
	    ((freq_hz / ZP_CLOCK_US_PER_SEC) <= UINT32_MAX)) {
		clock->cycles_per_us = (uint32_t)(freq_hz / ZP_CLOCK_US_PER_SEC);
	} else {
		clock->cycles_per_us = 0U;
	}
}

/* Convert cycles to whole microseconds, carrying the remainder to the next update */
static uint32_t zp_clock_cycles_to_us(zp_clock *clock, uint64_t delta_cycles)
{
	if (delta_cycles > ZP_CLOCK_MAX_DELTA_CYCLES) {
		clock->rem = 0U;
		return UINT32_MAX;
	}

	if (clock->cycles_per_us != 0U) {
		/* rem < cycles_per_us and delta <= UINT32_MAX / 2, so this fits in 32 bits */
		uint32_t total = (uint32_t)clock->rem + (uint32_t)delta_cycles;

		clock->rem = total % clock->cycles_per_us;
		return total / clock->cycles_per_us;
	}

	if (clock->freq_hz != 0U) {
		uint64_t total = clock->rem + (delta_cycles * ZP_CLOCK_US_PER_SEC);
		uint64_t delta_us = total / clock->freq_hz;

		clock->rem = total % clock->freq_hz;
		return (delta_us > UINT32_MAX) ? UINT32_MAX : (uint32_t)delta_us;
	}

	return 0U;
}

uint32_t zp_clock_update(zp_clock *clock, uint64_t delta_cycles, uint32_t tick_us)
{
	uint32_t delta_us = zp_clock_cycles_to_us(clock, delta_cycles);
	/* Where the clock would be relative to the start of the current tick */
	int64_t lead = (int64_t)(int32_t)(clock->now_us - tick_us) + (int64_t)delta_us;

	if (lead < 0) {
		/* Behind the tick clock: the counter stopped (CPU sleep) or wrapped unseen */
		lead = 0;
		clock->rem = 0U;
	} else if (lead >= (int64_t)clock->tick_len_us) {
		/* A full tick ahead: the counter runs faster than the tick clock */
		lead = (int64_t)clock->tick_len_us - 1;
	}

	clock->now_us = tick_us + (uint32_t)lead;
	return clock->now_us;
}
