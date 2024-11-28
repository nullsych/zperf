/**
 * @file   SysPerf_RTOS_Wrapper.h
 * @brief  Declaration of external wrappers API.
 * @author kvant6ubl
 * @date   Jul 18, 2024
 *
 */

#ifndef SYSPERF_RTOS_WRAPPER_H
#define SYSPERF_RTOS_WRAPPER_H

/*******************************************************************************************************************
 *                                                SYSPERF HEADERS                                                  *
 *******************************************************************************************************************/
#include "SysPerf_Keywords.h"

/*******************************************************************************************************************
*                                        HISTORY GLOBAL API DECLARATION                                            *
*******************************************************************************************************************/

extern RTOS_WRAPPER  uint32_t           SysPerf_GetTaskId(void);
extern RTOS_WRAPPER  void               SysPerf_InitRTUConfiguration(void);
extern RTOS_WRAPPER  uint8_t            SysPerf_GetTaskIterator(uint32_t);
extern RTOS_WRAPPER  uint32_t           SysPerf_GetUptimeTimestamp(void);

#endif /* SYSPERF_RTOS_WRAPPER_H */
