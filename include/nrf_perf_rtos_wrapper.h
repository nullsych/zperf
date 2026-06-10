/**
 * @file   Wrapper.h
 * @brief  Declaration of external wrappers API.
 * @author nullsych
 * @date   Jul 18, 2024
 *
 */

#ifndef NRF_PERF_RTOS_WRAPPER_H
#define NRF_PERF_RTOS_WRAPPER_H

/*******************************************************************************************************************
 *                                                STD HEADERS                                                      *
 *******************************************************************************************************************/
#include <stdint.h>

/*******************************************************************************************************************
 *                                                nrf_perf HEADERS                                                  *
 *******************************************************************************************************************/
#include "nrf_perf_keywords.h"

/*******************************************************************************************************************
*                                        GLOBAL API DECLARATION                                                    *
*******************************************************************************************************************/

extern RTOS_WRAPPER  uint32_t           nrf_perf_get_task_id(void);
extern RTOS_WRAPPER  void               nrf_perf_init_configuration(void);
extern RTOS_WRAPPER  uint8_t            nrf_perf_get_task_iterator(uint32_t);
extern RTOS_WRAPPER  const char*        nrf_perf_get_task_name(uint32_t);
extern RTOS_WRAPPER  uint32_t           nrf_perf_get_uptime_timestamp(void);

#endif /* NRF_PERF_RTOS_WRAPPER_H */
