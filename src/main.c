/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>

// /* Display all messages, including debugging ones: */
// #define LOG_LEVEL LOG_LEVEL_DBG
// /* Set the "module" for these log messages: */
// #define LOG_MODULE_NAME main
// #include <logging/log.h>
// /* Initialize module-specific magic state: */
// LOG_MODULE_REGISTER();

/* SysPerf runtime analysis headers */
#include <SysPerf.h>
#include <SysPerf_RTOS_Wrapper.h>

/* Application related headers */
#include "ApplDevicetree.h"
#include "ApplTasks.h"

/* Rest of Zephyr's unsused hooks */
#include "RTOSHooks.h"

void main(void)
{
   DeviceTree__initLeds();   

   ApplTasks__createApplTasks();

   ApplTasks__startApplTasks();

   /* Demo for SysPerf to get runtime statistic per last 1 second */
   while (1)
   {
      SysPerf__EpochOverallStatistic();
	   // LOG_DBG("Verbose debug %d", 3);
	   // LOG_INF("Everything is fine, mask=0x%x", 0xa1);
	   // LOG_WRN("Warning: %s was seen", "something bad");
	   // LOG_ERR("Error %d", 3);
      k_msleep(1000);
   }
   
}

/* EOF */
