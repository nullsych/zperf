/**
 * @file   LedTask.c
 * @brief  Contain main implementation for LedTask.
 * @author nullsych
 * @date   May 6, 2024
 *
 */

#include <zephyr/kernel.h>
#include <zephyr/debug/thread_analyzer.h>
#include <zephyr/tracing/tracing.h>

#include "common/TasksApi.h"
#include "common/TasksCfg.h"
#include "common/Leds/Leds.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(LedTask, LOG_LEVEL_DBG);

TASK_LOCAL TASK_ENTRY_POINT void Task_LED_entry(void)
{
	Leds_init();

	for (;;) {
		Leds_toggleLeds(LED_TASK_PERIOD);
	}
}

/* Define the thread using a macro */
K_THREAD_DEFINE(Task_LED, LED_TASK_STACKSIZE, Task_LED_entry, NULL, NULL, NULL, LED_TASK_PRIORITY,
		0, 0);

/* EOF */
