/**
 * @file   SysPerf_SetupData.h
 * @brief
 * @author kvant6ubl
 * @date   Jul 20, 2024
 *
 */

#ifndef SYSPERF_INITDATA_H
#define SYSPERF_INITDATA_H

#include "SysPerf.h"
#include "SysPerf_DataTypes.h"

/*******************************************************************************************************************
*                                         SYSPERF INIT DATA GLOBAL API                                             *
*******************************************************************************************************************/

extern HOOK_CALLED void          SysPerf_StartupData_Init(uint8 taskIterator, unsigned int taskId, const char *taskName, sint8 taskPrio, uint32 taskStackSize);
extern HOOK_CALLED sint8         SysPerf_StartupData_GetTaskPrio(uint32 taskId);
extern HOOK_CALLED uint32        SysPerf_StartupData_GetTaskStackSize(uint32 taskId);
extern HOOK_CALLED const char *  SysPerf_StartupData_GetTaskName(uint32 taskId);
extern HOOK_CALLED uint8         SysPerf_StartupData_GetTaskIterator(uint32 taskId);

#endif /*  */
