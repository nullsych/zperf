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
#include <SysPerf_RTOS_Wrapper.h>

/* SysPerf's global API */
#include <SysPerf.h>

/* Rest of Zephyr's unsused hooks */
#include "RTOSHooks.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

// #define HASH_SIZE 32u /* 32 bytes for 256 hash */

// typedef struct
// {
//    volatile const uint8_t sha256[HASH_SIZE];
// } Hash_t;

// const Hash_t firmware_hash =
// {
//    {
//       0x12 , 0x34 , 0x56 , 0x78 , 0x90 , 0x11 , 0x12 , 0x12 ,
//       0x12 , 0x12 , 0x12 , 0x12 , 0x12 , 0x12 , 0x12 , 0x12 ,
//       0x12 , 0x12 , 0x12 , 0x12 , 0x12 , 0x12 , 0x12 , 0x12 ,
//       0x12 , 0x12 , 0x12 , 0x12 , 0x12 , 0x12 , 0x12 , 0x12
//    }
// };

int main(void)
{
	// LOG_HEXDUMP_INF("Firmware hash version: %X", firmware_hash);
	LOG_INF("Perform pre-scheduler initializations for SysPerf");
	SysPerf_PreSchedulerInit();

	LOG_INF("Application tasks creating...");
	Led_createTask();
	Ble_createTask();
	SysPerf_createTask();
}

/* EOF */
