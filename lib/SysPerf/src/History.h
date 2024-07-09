/**
 * @file   History.h
 * @brief
 * @author kvant6ubl
 * @date   May 6, 2024
 *
 */

#ifndef HISTORY_H
#define HISTORY_H

#include "SysPerf.h"

/*******************************************************************************************************************
*                                            HISTORY GLOBAL API                                                    *
*******************************************************************************************************************/

SP_HOOK_CALLED uint8 History_GetLastEntityState(uint8 taskId);

SP_HOOK_CALLED void History_Init(void);

SP_HOOK_CALLED void History_Update(uint8 taskId, uint32 timestamp, uint8 state, uint8 lastOsEvent);

SP_HOOK_CALLED uint8 History_GetLastState(void);
SP_HOOK_CALLED uint8 History_GetLastTask(void);
SP_HOOK_CALLED uint32 History_GetLastTimestamp(void);

SP_USER_CALLED void History_TraceCollected(void);

#endif /* HISTORY_H */
