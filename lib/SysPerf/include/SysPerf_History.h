/**
 * @file   SysPerf_History.h
 * @brief
 * @author kvant6ubl
 * @date   May 6, 2024
 *
 */

#ifndef SYSPERF_HISTORY_H
#define SYSPERF_HISTORY_H

#include "SysPerf_Keywords.h"
#include "SysPerf_DataTypes.h"

/*******************************************************************************************************************
*                                              HISTORY CONSTANTS                                                   *
*******************************************************************************************************************/
#define SP_HISTORY_SIZE                     (8U)     /* Total number of hook-history entities, static allocated   */
#define SP_CPU_USAGE_THR                    (50U)    /* Threshold for CPU utilization across all cores           */


/* History statistic */
typedef struct SP_PACKED
{
    sp_Hook entities[SP_HISTORY_SIZE];
    uint32  crc32;
} sp_HookHistory;

/*******************************************************************************************************************
*                                            HISTORY GLOBAL API                                                    *
********************************************************************************************************************/

HOOK_CALLED SP_EXTERN void     SysPerf_History_Init(void);
HOOK_CALLED SP_EXTERN void     SysPerf_History_Update(uint32, uint64, uint8);
HOOK_CALLED SP_EXTERN void     SysPerf_History_Dump(void);

HOOK_CALLED SP_EXTERN uint8    SysPerf_History_GetLastStateMask(void);
HOOK_CALLED SP_EXTERN uint8    SysPerf_History_GetLastTask(void);
HOOK_CALLED SP_EXTERN uint32   SysPerf_History_GetLastTimestamp(void);

USER_CALLED void     SysPerf_History_TraceCollected(void);

#endif /* SYSPERF_HISTORY_H */
