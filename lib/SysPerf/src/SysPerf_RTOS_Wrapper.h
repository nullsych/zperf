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
#include "SysPerf_RTOS_Map.h"

/*******************************************************************************************************************
*                                        HISTORY GLOBAL API DECLARATION                                            *
*******************************************************************************************************************/

extern SP_HOOK_CALLED uint32           SysPerf_GetTaskId(void);
extern SP_HOOK_CALLED void             SysPerf_SaveTasksCfg(void);
extern SP_HOOK_CALLED uint8            SysPerf_GetTaskIterator(uint32);
extern SP_HOOK_CALLED uint64           SysPerf_GetUptimeTimestamp(void);

#endif /* SYSPERF_RTOS_WRAPPER_H */
