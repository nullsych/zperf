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

/*******************************************************************************************************************
*                                         SYSPERF INIT DATA GLOBAL API                                             *
*******************************************************************************************************************/

extern SP_HOOK_CALLED void          SysPerf_StartupData_Init(uint8 taskIterator, unsigned int taskId, const char *taskName, sint8 taskPrio, uint32 taskStackSize);
extern SP_HOOK_CALLED sint8         SysPerf_StartupData_GetTaskPrio(uint32 taskId);
extern SP_HOOK_CALLED uint32        SysPerf_StartupData_GetTaskStackSize(uint32 taskId);
extern SP_HOOK_CALLED const char *  SysPerf_StartupData_GetTaskName(uint32 taskId);
extern SP_HOOK_CALLED uint8         SysPerf_StartupData_GetTaskIterator(uint32 taskId);

#endif /*  */
