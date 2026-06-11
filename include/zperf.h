/**
 * @file   zperf.h
 * @brief  Public API for user code.
 * @author nullsych
 * @date   May 6, 2024
 *
 */

#ifndef ZPERF_H
#define ZPERF_H

#include "zperf_keywords.h"

/* API for user's code */
ZP_USER_CALLED extern void zperf_trace_history(void);
ZP_USER_CALLED extern void zperf_runtime_overall_statistic(void);
ZP_USER_CALLED extern void zperf_pre_scheduler_init(void);
ZP_USER_CALLED extern void zperf_post_scheduler_init(void);

#endif /* ZPERF_H */
