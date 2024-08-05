/**
 * @file   SysPerf_SetupData.c
 * @brief
 * @author kvant6ubl
 * @date   Jul 20, 2024
 *
 */

/*******************************************************************************************************************
*                                             RTOS-SPECIFIC HEADERS                                                *
*******************************************************************************************************************/
#include <stdlib.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(SysPerf_StartupData, LOG_LEVEL_DBG);

/*******************************************************************************************************************
*                                                SYSPERF HEADERS                                                   *
*******************************************************************************************************************/
#include "SysPerf.h"
#include "SysPerf_DataTypes.h"
#include "SysPerf_SetupData.h"

/*******************************************************************************************************************
*                                              STATISTIC ENTITIES                                                  *
*******************************************************************************************************************/

/**
 * @brief Store configuration/setup information of all Tasks (Threads), used as reference.
 */
SP_LOCAL sp_TaskConfigInfoHandler      Tasks_StartupData[SP_TASK_COUNT] = {NULL};

/*******************************************************************************************************************
*                                        HISTORY LOCAL API IMPLEMENTATION                                          *
*******************************************************************************************************************/

SP_LOCAL sp_TaskConfigInfo* SysPerf_StartupData_FindDataByKey(uint32 key)
{
   uint8 index = 0;
   sp_TaskConfigInfoHandler* dataPtr = &Tasks_StartupData;

   while (dataPtr != NULL)
   {
      if (dataPtr->taskId == key)
      {
         return &dataPtr->data;
      }
      index++;
      dataPtr++;
      if (index == SP_TASK_COUNT)
      {
         break;
      }
   }
   return NULL;
}

/*******************************************************************************************************************
*                                        HISTORY GLOBAL API IMPLEMENTATION                                         *
*******************************************************************************************************************/

SP_HOOK_CALLED void SysPerf_StartupData_Init(uint8 taskIterator, uint32 taskId, const char *taskName, sint8 taskPrio, uint32 taskStackSize)
{
   if (TASK_IS_VALID(taskIterator))
   {
      Tasks_StartupData[taskIterator].data.priority = taskPrio;
      Tasks_StartupData[taskIterator].data.stackSize = taskStackSize;
      Tasks_StartupData[taskIterator].data.taskName = taskName;
      Tasks_StartupData[taskIterator].data.taskIterator = taskIterator;
      Tasks_StartupData[taskIterator].taskId = taskId;
      // SP_TRACE_INFO("SysPerf_StartupData_Init | Tasks_StartupData taskIterator: %u", Tasks_StartupData[taskIterator].data.taskIterator);
   }
}

SP_HOOK_CALLED const char * SysPerf_StartupData_GetTaskName(uint32 taskId)
{
   sp_TaskConfigInfo *data = SysPerf_StartupData_FindDataByKey(taskId);
   return data ? data->taskName : NULL;
}

SP_HOOK_CALLED sint8 SysPerf_StartupData_GetTaskPrio(uint32 taskId)
{
    sp_TaskConfigInfo *data = SysPerf_StartupData_FindDataByKey(taskId);
    return data ? data->priority : -1;
}

SP_HOOK_CALLED uint32 SysPerf_StartupData_GetTaskStackSize(uint32 taskId)
{
   sp_TaskConfigInfo *data = SysPerf_StartupData_FindDataByKey(taskId);
   return data ? data->stackSize : -1;
}

SP_HOOK_CALLED uint8 SysPerf_StartupData_GetTaskIterator(uint32 taskId)
{
   sp_TaskConfigInfo *data = SysPerf_StartupData_FindDataByKey(taskId);
   return data ? data->taskIterator : -1;
}
