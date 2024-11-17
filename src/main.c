/**
 * @file   main.c
 * @brief  Contain implementation of main function.
 * @author kvant6ubl
 * @date   May 7, 2024
 *
 */

#include <stdio.h>
#include <zephyr/kernel.h>

/* Application headers */
#include "common/Leds/Leds.h"
#include "common/TasksCfg.h"
#include "common/TasksApi.h"

/* SysPerf's hooks, used under Zephyr's hooks */
#include <SysPerf_RTOS_SystemHooks.h>

/* SysPerf's global API */
#include <SysPerf.h>

/* Rest of Zephyr hooks */
#include "RTOSHooks.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

int main(void)
{
	LOG_INF("Perform pre-scheduler initializations for SysPerf");
	SysPerf_PreSchedulerInit();

	LOG_INF("Application tasks creating...");
	Led_createTask();
	Ble_createTask();
	SysPerf_createTask();

	LOG_INF("Perform post-scheduler initializations for SysPerf");
	SysPerf_PostSchedulerInit();
}

/* EOF */
