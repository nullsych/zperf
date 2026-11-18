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
#include <stdbool.h>
#include <stdint.h>

/*******************************************************************************************************************
 *                                                  HEADERS
 *******************************************************************************************************************/
#include "zperf_keywords.h"
#include "zperf_data_types.h"

/*******************************************************************************************************************
 *                                        GLOBAL API DECLARATION
 *******************************************************************************************************************/

extern ZP_RTOS_WRAPPER zp_task_id wr_get_task_id(void);
extern ZP_RTOS_WRAPPER void wr_init_configuration(void);
extern ZP_RTOS_WRAPPER const char *wr_get_task_name(zp_task_id);
extern ZP_RTOS_WRAPPER void wr_timestamp_init(void);
extern ZP_RTOS_WRAPPER uint32_t wr_get_uptime_timestamp(void);
extern ZP_RTOS_WRAPPER bool wr_is_idle_task(zp_task_id);

#endif /* ZEPHYR_WRAPPER_H */
