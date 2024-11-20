/**
 * @file   SysPerf.c
 * @brief  Contain main implementation of SysPerf functionality.
 * @author kvant6ubl
 * @date   May 6, 2024
 *
 */

/*******************************************************************************************************************
 *                                             RTOS-SPECIFIC HEADERS                                                *
 *******************************************************************************************************************/
#include <stdlib.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(SysPerf, LOG_LEVEL_DBG);

/*******************************************************************************************************************
*                                             SYSPERF HEADERS                                                      *
*******************************************************************************************************************/
#include "SysPerf.h"
#include "SysPerf_History.h"
// #include "SysPerf_Keywords.h"
// #include "SysPerf_DataTypes.h"
// #include "SysPerf_Constants.h"
#include "SysPerf_RTOS_Wrapper.h"
#include "SysPerf_SetupData.h"
#include "SysPerf_Cfg.cfg"

/*******************************************************************************************************************
*                                    MAIN SYSPERF STATISTIC ENTITIES                                               *
*******************************************************************************************************************/

/**
 * @brief Intermediate storage for uptime statistic, reset within 1 trace cycle (SP-cycle).
 * @details All collected data later copied to TraceCycle_UptimeStatistic.
 */
SPK_LOCAL sp_UptimeStatistic_t       Hook_UptimeStatistic;

/**
 * @brief Intermediate storage for Tasks (Threads) runtime statistic, reset within 1 trace cycle (SP-cycle).
 * @details All collected data later copied to TraceCycle_TasksStatistics.
 */
SPK_LOCAL sp_TaskRuntime             Hook_TasksStatistics[SP_TASK_COUNT];


/**
 * @brief Idle task runtime metadata.
 */
SPK_LOCAL uint8_t                    Idle_TaskIterator = SP_UNINIT;
SPK_LOCAL uint32_t                   Idle_TaskId       = SP_UNINIT;

/**
 * @brief Store a copy of uptime statistic when SysPerf_CycleBackup() routine occurs.
 * @details Used to trace all collected information to avoid race condition in rescheduling points.
 */
SPK_LOCAL sp_UptimeStatistic_t       TraceCycle_UptimeStatistic;

/**
 * @brief Store a copy of Tasks (Threads) runtime statistic when SysPerf_CycleBackup() routine occurs.
 * @details Used to trace all collected information to avoid race condition in rescheduling points.
 */
SPK_LOCAL sp_TaskRuntime             TraceCycle_TasksStatistics[SP_TASK_COUNT];

/*******************************************************************************************************************
*                                             SYSPERF LOCAL API                                                    *
*******************************************************************************************************************/
SPK_LOCAL uint32_t SysPerf_GetTaskRuntime(uint8_t taskIterator, uint32_t timeStamp);
SPK_LOCAL uint32_t SysPerf_GetTimestampDelta(uint32_t, uint32_t);
SPK_LOCAL uint32_t SysPerf_GetSystemRuntime(void);
SPK_LOCAL uint32_t SysPerf_GetIdleRuntime(void);

SPK_LOCAL void SysPerf_CycleBackup(void);

SPK_LOCAL void SysPerf_IsrLock(unsigned int *key);
SPK_LOCAL void SysPerf_IsrUnlock(unsigned int key);
SPK_LOCAL void SysPerf_ResetRuntimeStatistic(void);

SPK_LOCAL void SysPerf_TraceCycleStatistic(uint32_t *);
SPK_LOCAL void SysPerf_TraceCpuUsage(uint32_t *);
SPK_LOCAL void SysPerf_TraceUptimeStatistic(void);

SPK_LOCAL_INLINE void SysPerf_UpdateISRsStatistic(uint8_t isrIterator, uint32_t timestamp, uint32_t isrRuntime);
SPK_LOCAL_INLINE void SysPerf_UpdateTaskStatistic(uint8_t taskIterator, uint32_t timestamp, uint32_t taskRuntime);
SPK_LOCAL_INLINE void SysPerf_UpdateIdleStatistic(uint32_t timestamp);
SPK_LOCAL_INLINE void SysPerf_UpdateUptimeStatistic(uint8_t taskIterator, uint32_t taskRuntime, sp_TaskRuntime *task);

SPK_LOCAL_INLINE void SysPerf_ResetTraceCycleStatistic(uint8_t taskIterator);
SPK_LOCAL_INLINE void SysPerf_ResetHooksStatistic(uint8_t taskIterator);

/*******************************************************************************************************************
*                                          USER API IMPLEMENTATION                                                 *
*******************************************************************************************************************/

/**
 * @brief Trace Tasks (Threads) & ISRs runtime statistic per SP-cycle.
 *
 */
USER_CALLED void SysPerf_RuntimeOverallStatistic(void)
{
   uint32_t timeslice;
   uint32_t currTimestamp;
   static uint32_t lastTimestamp = 0U;

   SysPerf_CycleBackup();

   /* Calculate timeslice */
   currTimestamp = SysPerf_GetUptimeTimestamp();
   timeslice = SysPerf_GetTimestampDelta(lastTimestamp, currTimestamp);
   lastTimestamp = currTimestamp;

#if (SP_CFG_TRACE_MAIN_STAT == SP_CFG_ON)
   SysPerf_TraceCycleStatistic(&timeslice);
#endif

#if (SP_CFG_TRACE_UPTIME == SP_CFG_ON)
   SysPerf_TraceUptimeStatistic();
#endif

#if (SP_CFG_TRACE_CPU_LOAD == SP_CFG_ON)
   SysPerf_TraceCpuUsage(&timeslice);
#endif

#if (SP_CFG_TRACE_HISTORY == SP_CFG_ON)
   SysPerf_History_TraceCollected();
#endif

   SysPerf_ResetRuntimeStatistic();
   SysPerf_History_Clear();
}

/*******************************************************************************************************************
*                                       SYSPERF HOOKS IMPLEMENTATION                                               *
*******************************************************************************************************************/

RTOS_CALLED void SysPerf_PreTaskHook(void)
{
   // TODO: add CPU check support by CURRENT_CPU: uint8_t id = CURRENT_CPU;
   uint8_t  taskIterator;
   uint32_t taskId;
   uint32_t timestamp = 0U;

   timestamp = SysPerf_GetUptimeTimestamp();
   taskId = SysPerf_GetTaskId();
   taskIterator = SysPerf_GetTaskIterator(taskId);

   if (TASK_IS_VALID(taskIterator))
   {
      Hook_TasksStatistics[taskIterator].taskTimestamp = timestamp;

      if (IDLE_STATE_RUNNING == SysPerf_History_GetLastState())
      {
         SysPerf_UpdateIdleStatistic(timestamp);
      }

      SysPerf_History_Update(taskId, timestamp, TASK_STATE_ENTER);
   }
   else
   {
      // TODO: add collecting diagnostic data to history
   }
}

RTOS_CALLED void SysPerf_PostTaskHook(void)
{
   uint8_t  taskIterator;
   uint32_t taskId;
   uint32_t taskRuntime;
   uint32_t timestamp = 0U;

   taskId = SysPerf_GetTaskId();
   taskIterator = SysPerf_GetTaskIterator(taskId);

   if (TASK_IS_VALID(taskIterator))
   {
      timestamp = SysPerf_GetUptimeTimestamp();
      taskRuntime = SysPerf_GetTaskRuntime(taskIterator, timestamp);
      SysPerf_UpdateTaskStatistic(taskIterator, timestamp, taskRuntime);

      SysPerf_History_Update(taskId, timestamp, TASK_STATE_EXIT);
   }
   else
   {
      // TODO: add collecting diagnostic data to history
   }
}

RTOS_CALLED void SysPerf_PreISRHook(void)
{
   uint32_t timestamp;

   timestamp = SysPerf_GetUptimeTimestamp();

   if (IDLE_STATE_RUNNING == SysPerf_History_GetLastState())
   {
      SysPerf_UpdateIdleStatistic(timestamp);
   }

   SysPerf_History_Update(0x000, timestamp, ISR_STATE_ENTER);

// TODO: add SysPerf_GetIsrId support
/*
   uint32_t isr_num = SysPerf_GetIsrId();

   if (isr_num != 0)
   {
      SP_TRACE_INFO("Currently running in ISR number: %u\n", isr_num);
   }
   else
   {
      SP_TRACE_INFO("Currently running in thread context.");
   }
*/
}

RTOS_CALLED void SysPerf_PostISRHook(void)
{
   uint32_t timestamp;

   timestamp = SysPerf_GetUptimeTimestamp();

   SysPerf_History_Update(0x000, timestamp, ISR_STATE_EXIT);
}

RTOS_CALLED void SysPerf_IdleTaskRunning(void)
{
   static bool idleNotDetermined = true;

   if (idleNotDetermined)
   {
      Idle_TaskId = SysPerf_GetTaskId();
      Idle_TaskIterator = SysPerf_GetTaskIterator(Idle_TaskId);
      idleNotDetermined = false;
   }

   if (Idle_TaskIterator != SP_UNINIT && Idle_TaskId != SP_UNINIT)
   {
      SysPerf_History_Update(Idle_TaskId, SysPerf_GetUptimeTimestamp(), IDLE_STATE_RUNNING);
   }
}

/*******************************************************************************************************************
*                                       USER API IMPLEMENTATION                                                    *
*******************************************************************************************************************/

USER_CALLED void SysPerf_PreSchedulerInit(void)
{
   SysPerf_History_Init();
   SysPerf_ResetRuntimeStatistic();
}

USER_CALLED void SysPerf_PostSchedulerInit(void)
{
   SysPerf_InitRTUConfiguration();
}

USER_CALLED void SysPerf_TraceHistory(void)
{
   SysPerf_History_TraceCollected();
}

/*******************************************************************************************************************
*                                         LOCAL API IMPLEMENTATION                                                 *
*******************************************************************************************************************/

SPK_LOCAL uint32_t SysPerf_GetTimestampDelta(uint32_t timestamp1, uint32_t timestamp2)
{
   uint32_t delta;
   if (timestamp2 >= timestamp1)
   {
      delta = timestamp2 - timestamp1;
   }
   else
   {
      /* In case of type overflow */
      delta = (SP_TIMESTAMP_TOP - timestamp1) + timestamp2 + 1;
   }

   return delta;
}

/**
 * @brief Calculate total runtime across all Tasks (Threads) running in the system (with Idle task(s))
 */
SPK_LOCAL uint32_t SysPerf_GetSystemRuntime(void)
{
   uint8_t taskIterator;
   uint32_t systemTotalRuntime = 0U;

   for (taskIterator = 0; taskIterator < SP_TASK_COUNT; taskIterator++)
   {
      systemTotalRuntime += TraceCycle_TasksStatistics[taskIterator].totalRuntime;
   }

   return systemTotalRuntime;
}

/**
 * @brief Calculate separately Idle task(s) total runtime
 */
SPK_LOCAL uint32_t SysPerf_GetIdleRuntime(void)
{
   uint32_t idleTotalRuntime = 0U;

   if (SP_UNINIT != Idle_TaskIterator)
   {
      idleTotalRuntime = TraceCycle_TasksStatistics[Idle_TaskIterator].totalRuntime;
   }

   return idleTotalRuntime;
}

SPK_LOCAL void SysPerf_TraceCpuUsage(uint32_t *timeslice)
{
   /* System Tasks (Threads) total runtime per timeslice */
   uint32_t systemTotalRuntime;
   /* Percentage with application total runtime per timeslice */
   uint8_t cpuLoadPercent;
   /* Percentage with idle total runtime per timeslice */
   uint8_t idleTimePercent;
   /* Percentage for os resource runtime (scheduling etc) */
   uint8_t osTimePercent;
   /* Idle task(s) total runtime per timeslice */
   uint32_t idleTotalRuntime;
   /* Os total runtime per timeslice */
   uint32_t osTotalRuntime;

   /* Calculate separately Idle task(s) total runtime */
   idleTotalRuntime = SysPerf_GetIdleRuntime();

   /* Calculate total runtime across all Tasks (Threads) running in the system, except Idle task */
   systemTotalRuntime = SysPerf_GetSystemRuntime() - idleTotalRuntime;

   /* Calculate RTOS's resources total runtime */
   osTotalRuntime = (*timeslice) - (systemTotalRuntime + idleTotalRuntime);

   /* Calculate percentages */
   idleTimePercent = (uint8_t)GET_CPU_PERCENT((*timeslice), idleTotalRuntime);
   cpuLoadPercent = (uint8_t)GET_CPU_PERCENT((*timeslice), systemTotalRuntime);
   osTimePercent = (uint8_t)GET_CPU_PERCENT((*timeslice), osTotalRuntime);

   SP_TRACE_INFO("# ################# CPU LOAD %d%% ### IDLE %d%% ### KERNEL %d%% ################## #\n", cpuLoadPercent, idleTimePercent, osTimePercent);
}

SPK_LOCAL_INLINE void SysPerf_UpdateTaskStatistic(uint8_t taskIterator, uint32_t timestamp, uint32_t taskRuntime)
{
   sp_TaskRuntime *taskPtr = &Hook_TasksStatistics[taskIterator];

   if (taskRuntime > taskPtr->maxRuntime)
   {
      taskPtr->maxRuntime = taskRuntime;
   }

   taskPtr->taskTimestamp = timestamp;
   taskPtr->totalRuntime += taskRuntime;
   taskPtr->taskSwitches++;

   SysPerf_UpdateUptimeStatistic(taskIterator, taskRuntime, taskPtr);
}

SPK_LOCAL_INLINE void SysPerf_UpdateIdleStatistic(uint32_t timestamp)
{
   uint32_t idleRuntime;
   idleRuntime = SysPerf_GetTaskRuntime(Idle_TaskIterator, timestamp);
   SysPerf_UpdateTaskStatistic(Idle_TaskIterator, timestamp, idleRuntime);
}

SPK_LOCAL_INLINE void SysPerf_UpdateISRsStatistic(uint8_t isrIterator, uint32_t timestamp, uint32_t isrRuntime)
{

}

SPK_LOCAL_INLINE void SysPerf_UpdateUptimeStatistic(uint8_t taskIterator, uint32_t taskRuntime, sp_TaskRuntime *taskPtr)
{
   Hook_UptimeStatistic.TasksRuntime[taskIterator] += taskRuntime;
   Hook_UptimeStatistic.MaxTasksRuntime[taskIterator] = taskPtr->maxRuntime;
   Hook_UptimeStatistic.TasksSwitches[taskIterator] += taskPtr->taskSwitches;
}

SPK_LOCAL void SysPerf_IsrLock(unsigned int *key)
{
   *key = SP_ISR_LOCK();
}

SPK_LOCAL void SysPerf_IsrUnlock(unsigned int key)
{
   SP_ISR_UNLOCK(key);
}

SPK_LOCAL void SysPerf_TraceUptimeStatistic(void)
{
   // TODO: same for SysPerf_TraceCycleStatistic
}

SPK_LOCAL void SysPerf_CycleBackup(void)
{
   unsigned int key;
   SysPerf_IsrLock(&key);
   memcpy(&TraceCycle_TasksStatistics, &Hook_TasksStatistics, sizeof(Hook_TasksStatistics));
   memcpy(&TraceCycle_UptimeStatistic, &Hook_UptimeStatistic, sizeof(Hook_UptimeStatistic));
   // add delta !!!
   SysPerf_IsrUnlock(key);
}

SPK_LOCAL void SysPerf_TraceCycleStatistic(uint32_t *timeslice)
{
   SP_TRACE_INFO("# #####################   1 Second System Performance   ###################### #");
   SP_TRACE_INFO("#                                                                              #");
   SP_TRACE_INFO("#   Task Id         Total runtime         Max runtime        Total Switches    #");

   // TODO: add support of formatting names using k_thread_name_get((k_tid_t)thread);
   for (uint8_t taskIterator = 0; taskIterator < SP_TASK_COUNT; taskIterator++)
   {
      SP_TRACE_INFO("%s %6u %22u %19u %17u %10s",
                    "#", taskIterator, TraceCycle_TasksStatistics[taskIterator].totalRuntime,
                    TraceCycle_TasksStatistics[taskIterator].maxRuntime, TraceCycle_TasksStatistics[taskIterator].taskSwitches, "#");
   }
   SP_TRACE_INFO("# ############################################################################ #");
   SP_TRACE_INFO("# ### Calculations Timeslice: %u\n", (*timeslice));
}

SPK_LOCAL void SysPerf_ResetRuntimeStatistic(void)
{
   for (uint8_t taskIterator = 0; taskIterator < SP_TASK_COUNT; taskIterator++)
   {
      SysPerf_ResetTraceCycleStatistic(taskIterator);
      SysPerf_ResetHooksStatistic(taskIterator);
   }
}

SPK_LOCAL_INLINE void SysPerf_ResetTraceCycleStatistic(uint8_t taskIterator)
{
   TraceCycle_TasksStatistics[taskIterator].maxRuntime = 0u;
   TraceCycle_TasksStatistics[taskIterator].maxRuntime = 0u;
   TraceCycle_TasksStatistics[taskIterator].taskSwitches = 0u;
   TraceCycle_TasksStatistics[taskIterator].totalRuntime = 0u;
}

SPK_LOCAL_INLINE void SysPerf_ResetHooksStatistic(uint8_t taskIterator)
{
   Hook_TasksStatistics[taskIterator].maxRuntime = 0u;
   Hook_TasksStatistics[taskIterator].maxRuntime = 0u;
   Hook_TasksStatistics[taskIterator].taskSwitches = 0u;
   Hook_TasksStatistics[taskIterator].totalRuntime = 0u;
}

SPK_LOCAL uint32_t SysPerf_GetTaskRuntime(uint8_t taskIterator, uint32_t timeStamp)
{
   uint32_t taskRuntime;
   uint32_t prevTaskTimeStamp;
   prevTaskTimeStamp = Hook_TasksStatistics[taskIterator].taskTimestamp;
   taskRuntime = SysPerf_GetTimestampDelta(prevTaskTimeStamp, timeStamp);
   return taskRuntime;
}
