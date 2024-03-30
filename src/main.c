/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>

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
      SysPerf__OneSecondStatistics();
      k_msleep(1000);
   }
   
}

/* EOF */
