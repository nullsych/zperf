/**
 * @file   wrapper.h
 * @brief  Declaration of wrappers that encapsulate RTOS-specific API.
 * @author nullsych
 * @date   Jul 18, 2024
 *
 */

#ifndef ZPERF_WRAPPER_H
#define ZPERF_WRAPPER_H

#include "rtos_wrapper.h"

ZP_RTOS_WRAPPER uint32_t zperf_get_isr_id(void);

#endif /* ZPERF_WRAPPER_H */
