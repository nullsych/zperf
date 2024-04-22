/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/uuid.h>

/* Application related headers */
#include "common/Leds/Leds.h"
#include "common/TasksCfg.h"
#include "common/TasksApi.h"

/* SysPerf's hooks, used under Zephyr's hooks */
#include <SysPerf_RTOS_Wrapper.h>

/* Rest of Zephyr's unsused hooks */
#include "RTOSHooks.h"

#include <logging/log.h>
LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

void main(void)
{
   Led_init0();

   Led_createTask();

   Ble_createTask();

   SysPerf_createTask();
}

/* EOF */
