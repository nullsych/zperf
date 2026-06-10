/**
 * @file   PwrMgt.c
 * @brief  Contain Power Management test features.
 * @author nullsych
 * @date   Nov 20, 2024
 *
 */

#include <zephyr/kernel.h>
#include <zephyr/pm/pm.h>
#include <zephyr/pm/state.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(PwrMgt, LOG_LEVEL_DBG);

#include <helpers/nrfx_reset_reason.h>
#include "PwrMgt.h"

PM_LOCAL void PwrMgt_notifyStateExit(enum pm_state);
PM_LOCAL void PwrMgt_notifyStateEntry(enum pm_state);
PM_LOCAL uint32_t PwrMgt_getResetReason(void);

/* Object of power management notifier callbacks */
PM_LOCAL struct pm_notifier PwrMgt_Notifier = {
	.state_entry = PwrMgt_notifyStateEntry,
	.state_exit = PwrMgt_notifyStateExit,
};

/* Track entry/exit to sleep */
PM_LOCAL pm_counters_t PwrMgt_Counters[PM_STATE_COUNT];

PM_LOCAL void PwrMgt_notifyStateEntry(enum pm_state state)
{
	PwrMgt_Counters[(uint8_t)state].entryCounter++;

	LOG_INF("PM state %d entry. State counter: %d", (uint8_t)state,
		PwrMgt_Counters[(uint8_t)state].entryCounter);
}

PM_LOCAL void PwrMgt_notifyStateExit(enum pm_state state)
{
	PwrMgt_Counters[(uint8_t)state].exitCounter++;

	LOG_INF("PM state %d exit. State counter: %d", (uint8_t)state,
		PwrMgt_Counters[(uint8_t)state].exitCounter);
}

/*
const struct pm_state_info *pm_policy_next_state(uint8_t cpu, int32_t ticks)
{
	static const struct pm_state_info state = {
		// .state = PM_STATE_SUSPEND_TO_RAM
	.state = PM_STATE_ACTIVE
	};

    LOG_INF("Current cpu: %d", cpu);

	return &state;
}
*/

PM_LOCAL uint32_t PwrMgt_getResetReason(void)
{
	uint32_t resetReason;

	resetReason = nrfx_reset_reason_get();
	nrfx_reset_reason_clear(resetReason);

	return resetReason;
}

void PwrMgt_traceResetReason(void)
{
	uint32_t resetReason;

	resetReason = PwrMgt_getResetReason();

	if (resetReason & NRF_POWER_RESETREAS_NFC_MASK) {
		LOG_INF("Wake up from 'System OFF' mode when wakeup is triggered from ANADETECT "
			"signal from LPCOMP(NFC field detect)");
	} else if (resetReason & NRF_POWER_RESETREAS_DOG_MASK) {
		LOG_INF("Reset due to wake up from System OFF mode when wakeup is triggered from "
			"DETECT signal from GPIO");
	} else if (resetReason & NRF_POWER_RESETREAS_RESETPIN_MASK) {
		LOG_INF("Reset from pin-reset");
	} else if (resetReason & NRF_POWER_RESETREAS_SREQ_MASK) {
		LOG_INF("Reset from soft reset");
	} else if (resetReason) {
		LOG_INF("Reset by a different source (0x%08X)", resetReason);
	} else {
		LOG_INF("Power-on-reset");
	}
}

void PwrMgt_init(void)
{
	LOG_INF("Registering a power management notifier callbacks");
	pm_notifier_register(&PwrMgt_Notifier);
}
