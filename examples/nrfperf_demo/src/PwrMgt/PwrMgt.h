/**
 * @file   PwrMgt.h
 * @brief
 * @author nullsych
 * @date   Nov 20, 2024
 *
 */

#ifndef PWR_MGT_H
#define PWR_MGT_H

#define PM_LOCAL                static
#define PM_LOCAL_INLINE         static inline

typedef struct
{
	uint8_t entryCounter;
	uint8_t exitCounter;
} pm_counters_t;

void PwrMgt_init(void);
void PwrMgt_traceResetReason(void);

#endif /* PWR_MGT_H */