/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/uuid.h>

/* SysPerf runtime analysis headers */
#include <SysPerf.h>
#include <SysPerf_RTOS_Wrapper.h>

/* Application related headers */
#include "ApplDevicetree.h"
#include "TasksCfg.h"
#include "TasksApi.h"

/* Rest of Zephyr's unsused hooks */
#include "RTOSHooks.h"

#include <zephyr.h>
// #define LOG_LEVEL LOG_LEVEL_DBG
// #define LOG_MODULE_NAME Nrf52
#include <logging/log.h>
LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

void main(void)
{

   BleTask_main();

   // DeviceTree__initLeds();

   // ApplTasks__createApplTasks();

   // ApplTasks__startApplTasks();

   /* Demo for SysPerf to get runtime statistic per last 1 second */
   // while (1)
   // {
   // SysPerf__EpochOverallStatistic();
   // LOG_DBG("Verbose debug %d", 3);
   // LOG_INF("Everything is fine, mask=0x%x", 0xa1);
   // LOG_WRN("Warning: %s was seen", "something bad");
   // LOG_ERR("Error %d", 3);
   // k_msleep(1000);
   // }
}

/* EOF */
