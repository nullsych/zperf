#include <zephyr.h>
#include <debug/thread_analyzer.h>
#include <tracing/tracing.h>

#include "TasksCfg.h"
#include "ApplDevicetree.h"

K_THREAD_STACK_DEFINE(LED_Task_stack_area, LED_TASK_STACKSIZE);
K_THREAD_STACK_DEFINE(BLE_Task_stack_area, BLE_TASK_STACKSIZE);

/* LED thread Structure */
static struct k_thread LED_Task_zthread_data;

/* BLE thread Structure */
// static struct k_thread BLE_Task_zthread_data;

/* Zephyr Thread ID (runtime defined) */
static k_tid_t LED_Task_zid;
// static k_tid_t BLE_Task_zid;

/* User defined Task ID written in custom data */
static uint8_t LED_Task_custom_data = 1; /* taskId */
// static uint8_t BLE_Task_custom_data = 2; /* taskId */

static void Task_LED(void *unused_arg1, void *unused_arg2, void *unused_arg3)
{
   ARG_UNUSED(unused_arg1);
   ARG_UNUSED(unused_arg2);
   ARG_UNUSED(unused_arg3);

   k_thread_custom_data_set((void *)LED_Task_custom_data);

   DeviceTree__toggleLed0(LED_TASK_PERIOD);
}

/*
static void Task_BLE(void *unused_arg1, void *unused_arg2, void *unused_arg3)
{
   ARG_UNUSED(unused_arg1);
   ARG_UNUSED(unused_arg2);
   ARG_UNUSED(unused_arg3);

   // ...
}
*/

void ApplTasks__createApplTasks(void)
{
   LED_Task_zid = k_thread_create(&LED_Task_zthread_data, LED_Task_stack_area,
                                  K_THREAD_STACK_SIZEOF(LED_Task_stack_area),
                                  Task_LED, NULL, NULL, NULL,
                                  LED_TASK_PRIORITY, 0, K_FOREVER);

   k_thread_name_set(LED_Task_zid, "Task_LED");
}

void ApplTasks__startApplTasks(void)
{
   k_thread_start(LED_Task_zid);
}

/**
 * @brief Start a Thread Analyzer as separate thread to get a little runtime statistics.
 * 
 * @warning Must be called after ApplTasks__startApplTasks() only!
*/
void ApplTasks__startThreadAnalyzer(void)
{
   while (1)
   {
      thread_analyzer_print();
      k_msleep(1000);
   }
}
