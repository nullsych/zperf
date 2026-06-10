/**
 * @file   RTOS_Hooks.h
 * @brief  Declare RTOS-specific hook functions with invoked nrf_perf's API.
 * @author nullsych
 * @date   May 6, 2024
 *
 * @details Use next template to define RTOS-specific hook function's body:
 *
 * Os_Hook_Function (...)
 * {
 *    nrf_perf_<Hook>(...);
 * }
 *
 */
/* clang-format off */
#ifndef NRF_PERF_RTOS_SYSTEM_HOOKS_H
#define NRF_PERF_RTOS_SYSTEM_HOOKS_H

/*******************************************************************************************************************
 *                                             RTOS-SPECIFIC HEADERS                                               *
 ******************************************************************************************************************/
#include <zephyr/kernel.h>
#include <zephyr/tracing/tracing.h>

/*******************************************************************************************************************
 *                                      RTOS-SPECIFIC HOOK DECLARATIONS                                            *
 ******************************************************************************************************************/
void sys_trace_thread_switched_in_user(void);
void sys_trace_thread_switched_out_user(void);
void sys_trace_isr_enter_user(void);
void sys_trace_isr_exit_user(void);
void sys_trace_idle_user(void);

#endif /* NRF_PERF_RTOS_SYSTEM_HOOKS_H */
/* clang-format on */
