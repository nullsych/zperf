/**
 * @file   SysPerf_History.h
 * @brief
 * @author kvant6ubl
 * @date   May 6, 2024
 *
 */

#ifndef SYSPERF_HISTORY_H
#define SYSPERF_HISTORY_H

#include "SysPerf.h"

/*******************************************************************************************************************
*                                            HISTORY GLOBAL API                                                    *
*******************************************************************************************************************/

SP_HOOK_CALLED void     SysPerfHistory_Init(void);
SP_HOOK_CALLED void     SysPerfHistory_Update(uint8 taskIterator, uint64 timestamp, uint8 state, uint8 lastOsEvent);
SP_HOOK_CALLED uint8    SysPerfHistory_GetLastEntityState(uint8 taskIterator);
SP_HOOK_CALLED uint8    SysPerfHistory_GetLastState(void);
SP_HOOK_CALLED uint8    SysPerfHistory_GetLastTask(void);
SP_HOOK_CALLED uint32   SysPerfHistory_GetLastTimestamp(void);
SP_USER_CALLED void     SysPerfHistory_TraceCollected(void);

#endif /* SYSPERF_HISTORY_H */
