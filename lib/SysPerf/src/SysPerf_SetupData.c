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
LOG_MODULE_REGISTER(SysPerfSetupData, LOG_LEVEL_DBG);

/*******************************************************************************************************************
 *                                                SYSPERF HEADERS                                                  *
 *******************************************************************************************************************/
#include "SysPerf.h"
#include "SysPerf_DataTypes.h"
#include "SysPerf_SetupData.h"

/*******************************************************************************************************************
 *                                              STATISTIC ENTITIES                                                 *
 *******************************************************************************************************************/

typedef struct
{
   uint32 taskId;
   sp_TaskConfigInfo data;
} sp_TaskConfigInfoHandler;


/**
 * @brief Store configuration/setup information of all tasks, used as reference.
 */
SP_LOCAL sp_TaskConfigInfoHandler  Tasks_SetupData[SP_TASK_COUNT] = {NULL};

/*******************************************************************************************************************
*                                        HISTORY GLOBAL API IMPLEMENTATION                                         *
*******************************************************************************************************************/

SP_LOCAL sp_TaskConfigInfo* SysPerfSetupData_FindDataByKey(uint32 key)
{
   uint8 index = 0;
   sp_TaskConfigInfoHandler* dataPtr = &Tasks_SetupData;

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

SP_HOOK_CALLED void SysPerfSetupData_Init(uint8 taskIterator, uint32 taskId, const char *taskName, sint8 taskPrio, uint32 taskStackSize)
{
   if (TASK_IS_VALID(taskIterator))
   {
      Tasks_SetupData[taskIterator].data.priority = taskPrio;
      Tasks_SetupData[taskIterator].data.stackSize = taskStackSize;
      Tasks_SetupData[taskIterator].data.taskName = taskName;
      Tasks_SetupData[taskIterator].data.taskIterator = taskIterator;
      Tasks_SetupData[taskIterator].taskId = taskId;
      SP_TRACE_INFO("SysPerfSetupData_Init | Tasks_SetupData taskIterator: %u", Tasks_SetupData[taskIterator].data.taskIterator);
   }
}

SP_HOOK_CALLED const char * SysPerfSetupData_GetTaskName(uint32 taskId)
{
   sp_TaskConfigInfo *data = SysPerfSetupData_FindDataByKey(taskId);
   return data ? data->taskName : NULL;
}

SP_HOOK_CALLED sint8 SysPerfSetupData_GetTaskPrio(uint32 taskId)
{
    sp_TaskConfigInfo *data = SysPerfSetupData_FindDataByKey(taskId);
    return data ? data->priority : -1;
}

SP_HOOK_CALLED uint32 SysPerfSetupData_GetTaskStackSize(uint32 taskId)
{
   sp_TaskConfigInfo *data = SysPerfSetupData_FindDataByKey(taskId);
   return data ? data->stackSize : -1;
}

SP_HOOK_CALLED uint8 SysPerfSetupData_GetTaskIterator(uint32 taskId)
{
   sp_TaskConfigInfo *data = SysPerfSetupData_FindDataByKey(taskId);
   return data ? data->taskIterator : -1;
}
