/**
 * @file   nrf_perf_data_types.h
 * @brief
 * @author nullsych
 * @date   Jule 9, 2024
 *
 */

#ifndef NRF_PERF_DATA_TYPES_H
#define NRF_PERF_DATA_TYPES_H

#include "rtos_map.h"
#include "nrf_perf_constants.h"
#include "nrf_perf_keywords.h"

/* clang-format off */

typedef MAP_RTOS_PTR2TASK  np_task_ptr;
typedef MAP_RTOS_PRIO      np_task_prio;
typedef MAP_RTOS_STACKSIZE np_task_stack_size;
typedef MAP_RTOS_UNIQ_ID   np_task_id;
typedef MAP_RTOS_UNIQ_ID   np_isr_id;

/* Runtime Unit's (Task's/ISR's) state mask */
typedef enum
{
    /* Written when new Task (Thread) take precedence and start works */
    TASK_STATE_ENTER        = 0x00,

    /* Written when current Task (Thread) finished it work, and new one Task takes precedence */
    TASK_STATE_EXIT         = 0x01,

    /* Written when new ISR take precedence and start works */
    ISR_STATE_ENTER         = 0x02,

    /* Written when current ISR finished it work, and new one Task/ISR takes precedence */
    ISR_STATE_EXIT          = 0x03,

    /* Written when Idle Tasks (Thread) is running */
    IDLE_STATE_RUNNING      = 0x04

} np_runtime_state_mask;
/* clang-format on */

/* Hook statistic */
typedef struct NP_PACKED {
	uint32_t timestamp;
	uint32_t identifier;
	uint8_t state;
} np_hook;

/* Task (Thread) runtime information */
typedef struct NP_PACKED {
	uint32_t task_timestamp;
	uint32_t total_runtime;
	uint32_t max_runtime;
	uint32_t task_switches;
} np_task_runtime;

/* Runtime information of a ISR */
typedef struct NP_PACKED {
	uint32_t isr_timestamp;
	uint32_t total_runtime;
	uint32_t max_runtime;
	uint32_t isr_switches;
} np_isr_runtime;

/* Total system uptime statistic */
typedef struct NP_PACKED {
	uint32_t tasks_runtime[NP_TASK_COUNT];
	uint32_t tasks_switches[NP_TASK_COUNT];
	uint32_t max_tasks_runtime[NP_TASK_COUNT];
	uint32_t isr_runtime[NP_TASK_COUNT];
	uint32_t idle_runtime;
	uint32_t timestamp;
} np_uptime_statistic_t;

#endif /* NRF_PERF_DATA_TYPES_H */
