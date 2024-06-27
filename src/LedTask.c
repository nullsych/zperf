/**
 * @file   LedTask.c
 * @brief  Contain main implementation for LedTask.
 * @author kvant6ubl
 * @date   May 6, 2024
 *
 */

#include <zephyr/kernel.h>
#include <zephyr/debug/thread_analyzer.h>
#include <zephyr/tracing/tracing.h>

#include "common/TasksApi.h"
#include "common/TasksCfg.h"
#include "common/Leds/Leds.h"

K_THREAD_STACK_DEFINE(LED_Task_stack_area, LED_TASK_STACKSIZE);

TASK_LOCAL struct k_thread Led_zthread_data;
TASK_LOCAL k_tid_t Led_zid;
TASK_LOCAL uint8_t Led_taskId = LED_TASKID;

TASK_LOCAL void Task_LED(void *unused_arg1, void *unused_arg2, void *unused_arg3)
{
   ARG_UNUSED(unused_arg1);
   ARG_UNUSED(unused_arg2);
   ARG_UNUSED(unused_arg3);

   k_thread_custom_data_set((void *)Led_taskId);

   Leds_init();

   for (;;)
   {
      Leds_toggleLeds(LED_TASK_PERIOD);
   }
}

void Led_createTask(void)
{
   Led_zid = k_thread_create(&Led_zthread_data, LED_Task_stack_area,
                             K_THREAD_STACK_SIZEOF(LED_Task_stack_area),
                             Task_LED, NULL, NULL, NULL,
                             LED_TASK_PRIORITY, 0, K_FOREVER);

   k_thread_name_set(Led_zid, "Task_LED");

   k_thread_start(Led_zid);
}

/**
 * @brief Start a Thread Analyzer as separate thread to get a little runtime statistics.
 */
/*
void ThreadAnalyzerTask(void)
{
   while (1)
   {
      thread_analyzer_print();
      k_msleep(1000);
   }
}
*/

/* EOF */