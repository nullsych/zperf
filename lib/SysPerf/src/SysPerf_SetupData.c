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
#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(SysPerf_StartupData, LOG_LEVEL_DBG);

/*******************************************************************************************************************
*                                                SYSPERF HEADERS                                                   *
*******************************************************************************************************************/
#include "SysPerf.h"
#include "SysPerf_Keywords.h"
#include "SysPerf_Constants.h"
#include "SysPerf_SetupData.h"

/*******************************************************************************************************************
*                                              STATISTIC ENTITIES                                                  *
*******************************************************************************************************************/

/**
 * @brief Store configuration/setup information of all Tasks (Threads), used as reference.
 */
SPK_LOCAL sp_TaskConfigInfoHandler      Tasks_StartupData[SP_TASK_COUNT];

/*******************************************************************************************************************
*                                        HISTORY LOCAL API IMPLEMENTATION                                          *
*******************************************************************************************************************/

SPK_LOCAL sp_TaskConfigInfo* SysPerf_StartupData_FindDataByKey(uint32_t key)
{
   uint8_t index = 0;
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

HOOK_CALLED void SysPerf_StartupData_Init(uint8_t taskIterator, uint32_t taskId, const char *taskName, int8_t taskPrio, uint32_t taskStackSize)
{
   if (TASK_IS_VALID(taskIterator))
   {
      Tasks_StartupData[taskIterator].data.priority = taskPrio;
      Tasks_StartupData[taskIterator].data.stackSize = taskStackSize;
      Tasks_StartupData[taskIterator].data.taskIterator = taskIterator;
      Tasks_StartupData[taskIterator].taskId = taskId;
      memcpy(Tasks_StartupData[taskIterator].data.taskName, taskName, SP_TASK_NAME_LEN);
      SP_TRACE_INFO("SysPerf_StartupData_Init | Tasks_StartupData taskIterator: %u", Tasks_StartupData[taskIterator].data.taskIterator);
   }
}

HOOK_CALLED const char * SysPerf_StartupData_GetTaskName(uint32_t taskId)
{
   sp_TaskConfigInfo *data = SysPerf_StartupData_FindDataByKey(taskId);
   return data ? data->taskName : NULL;
}

HOOK_CALLED int8_t SysPerf_StartupData_GetTaskPrio(uint32_t taskId)
{
   sp_TaskConfigInfo *data = SysPerf_StartupData_FindDataByKey(taskId);
   return data ? data->priority : -1;
}

HOOK_CALLED uint32_t SysPerf_StartupData_GetTaskStackSize(uint32_t taskId)
{
   sp_TaskConfigInfo *data = SysPerf_StartupData_FindDataByKey(taskId);
   return data ? data->stackSize : -1;
}

HOOK_CALLED uint8_t SysPerf_StartupData_GetTaskIterator(uint32_t taskId)
{
   sp_TaskConfigInfo *data = SysPerf_StartupData_FindDataByKey(taskId);
   return data ? data->taskIterator : -1;
}
