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
typedef struct SPK_PACKED
{
    sp_Hook entities[SP_HISTORY_SIZE];
    uint32_t  crc32;
} sp_HookHistory;

/*******************************************************************************************************************
*                                            HISTORY GLOBAL API                                                    *
********************************************************************************************************************/

HOOK_CALLED SPK_GLOBAL void         SysPerf_History_Init(void);
HOOK_CALLED SPK_GLOBAL void         SysPerf_History_Update(uint32_t, uint32_t, uint8_t);
HOOK_CALLED SPK_GLOBAL void         SysPerf_History_Clear(void);
HOOK_CALLED SPK_GLOBAL void         SysPerf_History_DumpByteStream(void);
USER_CALLED SPK_GLOBAL void         SysPerf_History_TraceCollected(void);

HOOK_CALLED SPK_GLOBAL uint8_t      SysPerf_History_GetLastState(void);
HOOK_CALLED SPK_GLOBAL uint8_t      SysPerf_History_GetLastTask(void);
HOOK_CALLED SPK_GLOBAL uint32_t     SysPerf_History_GetLastTimestamp(void);

#endif /* SYSPERF_HISTORY_H */
