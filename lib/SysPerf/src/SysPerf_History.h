/**
 * @file   SysPerf_History.h
 * @brief
 * @author kvant6ubl
 * @date   May 6, 2024
 *
 */

#ifndef SYSPERF_HISTORY_H
#define SYSPERF_HISTORY_H

#include "SysPerf_DataTypes.h"

/*******************************************************************************************************************
*                                            HISTORY GLOBAL API                                                    *
********************************************************************************************************************/

SP_HOOK_CALLED void     SysPerf_History_Init(void);
SP_HOOK_CALLED void     SysPerf_History_Update(uint32, uint64, uint8);
SP_HOOK_CALLED void     SysPerf_History_Dump(void);

SP_HOOK_CALLED uint8    SysPerf_History_GetLastStateMask(void);
SP_HOOK_CALLED uint8    SysPerf_History_GetLastTask(void);
SP_HOOK_CALLED uint32   SysPerf_History_GetLastTimestamp(void);

SP_USER_CALLED void     SysPerf_History_TraceCollected(void);

#endif /* SYSPERF_HISTORY_H */
