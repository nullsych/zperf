/**
 * @file   main.c
 * @brief  Contain implementation of main function.
 * @author nullsych
 * @date   May 7, 2024
 *
 */

#include <stdio.h>
#include <zephyr/kernel.h>

/* Application headers */
#include "common/Leds/Leds.h"
#include "common/TasksCfg.h"
#include "common/TasksApi.h"

/* Tool global API */
#include <zperf.h>

/* Rest of unmapped Zephyr hooks */
#include "RTOSHooks.h"

/* Power Management */
#include "PwrMgt/PwrMgt.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

int main(void)
{
	zperf_pre_scheduler_init();

	PwrMgt_init();
	PwrMgt_traceResetReason();

	zperf_post_scheduler_init();
}

/* EOF */
