/**
 * @file   rtos_wrapper.h
 * @brief  Declaration of external wrappers API.
 * @author nullsych
 * @date   Jul 18, 2024
 *
 */

#ifndef ZEPHYR_WRAPPER_H
#define ZEPHYR_WRAPPER_H

/*******************************************************************************************************************
 *                                                STD HEADERS
 *******************************************************************************************************************/
#include <stdint.h>

/*******************************************************************************************************************
 *                                                  HEADERS
 *******************************************************************************************************************/
#include "zperf_keywords.h"

/*******************************************************************************************************************
 *                                        GLOBAL API DECLARATION
 *******************************************************************************************************************/

extern ZP_RTOS_WRAPPER uint32_t wr_get_task_id(void);
extern ZP_RTOS_WRAPPER void wr_init_configuration(void);
extern ZP_RTOS_WRAPPER uint8_t wr_get_task_iterator(uint32_t);
extern ZP_RTOS_WRAPPER const char *wr_get_task_name(uint32_t);
extern ZP_RTOS_WRAPPER uint32_t wr_get_uptime_timestamp(void);

#endif /* ZEPHYR_WRAPPER_H */
