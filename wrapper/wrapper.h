/**
 * @file   wrapper.h
 * @brief  Declaration of wrappers that encapsulate RTOS-specific API.
 * @author nullsych
 * @date   Jul 18, 2024
 *
 */

#ifndef NRF_PERF_WRAPPER_H
#define NRF_PERF_WRAPPER_H

#include "nrf_perf_rtos_wrapper.h"

RTOS_WRAPPER uint32_t nrf_perf_get_isr_id(void);

#endif /* NRF_PERF_WRAPPER_H */
