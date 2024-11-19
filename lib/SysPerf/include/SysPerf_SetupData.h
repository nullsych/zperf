/**
 * @file   SysPerf_SetupData.h
 * @brief
 * @author kvant6ubl
 * @date   Jul 20, 2024
 *
 */

#ifndef SYSPERF_SETUPDATA_H
#define SYSPERF_SETUPDATA_H

#include "SysPerf.h"
#include "SysPerf_DataTypes.h"

/*******************************************************************************************************************
*                                         SYSPERF INIT DATA GLOBAL API                                             *
*******************************************************************************************************************/

extern HOOK_CALLED void          SysPerf_StartupData_Init(uint8_t taskIterator, unsigned int taskId, const char *taskName, int8_t taskPrio, uint32_t taskStackSize);
extern HOOK_CALLED int8_t         SysPerf_StartupData_GetTaskPrio(uint32_t taskId);
extern HOOK_CALLED uint32_t        SysPerf_StartupData_GetTaskStackSize(uint32_t taskId);
extern HOOK_CALLED const char *  SysPerf_StartupData_GetTaskName(uint32_t taskId);
extern HOOK_CALLED uint8_t         SysPerf_StartupData_GetTaskIterator(uint32_t taskId);

#endif /* SYSPERF_SETUPDATA_H */
