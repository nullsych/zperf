/**
 * @file   zperf_clock.h
 * @brief  Microsecond uptime from a cycle counter, kept in step with the RTOS tick clock.
 *
 * A free-running cycle counter (e.g. DWT CYCCNT on Cortex-M) gives sub-tick resolution, but it
 * may stop while the CPU sleeps and it wraps quickly (about every 67 s at 64 MHz). This clock
 * advances by the cycles counted since its previous update, and is then held within the
 * current tick of the RTOS tick clock:
 *
 *   - it never falls behind the tick clock: after sleep, or a counter wrap that was missed, it
 *     catches up to the start of the current tick;
 *   - it never runs a full tick ahead of it, which bounds drift between the two oscillators.
 *
 * So time is measured at counter resolution while the CPU runs, and at tick resolution across
 * sleep. The returned value is monotonic (modulo 2^32 us) as long as the tick clock is.
 *
 * The code is RTOS-agnostic: the RTOS wrapper reads the counter and the tick clock.
 */

#ifndef ZPERF_CLOCK_H
#define ZPERF_CLOCK_H

#include <stdint.h>

typedef struct {
	uint32_t now_us;        /* Last returned uptime */
	uint32_t tick_len_us;   /* Length of one tick, rounded up */
	uint32_t cycles_per_us; /* Counter rate if it is a whole number of MHz, otherwise 0 */
	uint64_t freq_hz;       /* Counter rate, used when cycles_per_us is 0 */
	uint64_t rem;           /* Counted but not yet converted: cycles, or cycles * 1e6 */
} zp_clock;

/**
 * @param freq_hz     Counter rate in Hz
 * @param tick_len_us Length of one RTOS tick in microseconds, rounded up
 * @param tick_us     Current tick clock time in microseconds, the clock starts here
 */
void zp_clock_init(zp_clock *clock, uint64_t freq_hz, uint32_t tick_len_us, uint32_t tick_us);

/**
 * @brief Advance the clock and return the current uptime in microseconds.
 *
 * @param delta_cycles Counter cycles since the previous update
 * @param tick_us      Current tick clock time in microseconds (start of the current tick)
 */
uint32_t zp_clock_update(zp_clock *clock, uint64_t delta_cycles, uint32_t tick_us);

#endif /* ZPERF_CLOCK_H */
