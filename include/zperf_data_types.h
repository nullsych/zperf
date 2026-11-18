/**
 * @file   zperf_data_types.h
 * @brief
 * @author nullsych
 * @date   Jule 9, 2024
 *
 */

#ifndef ZPERF_DATA_TYPES_H
#define ZPERF_DATA_TYPES_H

#include "rtos_map.h"
#include "zperf_constants.h"
#include "zperf_keywords.h"

/* clang-format off */

typedef MAP_RTOS_PTR2TASK  zp_task_ptr;
typedef MAP_RTOS_PRIO      zp_task_prio;
typedef MAP_RTOS_STACKSIZE zp_task_stack_size;
typedef MAP_RTOS_UNIQ_ID   zp_task_id;
typedef MAP_RTOS_UNIQ_ID   zp_isr_id;

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
    IDLE_STATE_RUNNING      = 0x04,

    /* Returned by history getters when no event has been recorded yet */
    HISTORY_STATE_NONE      = 0xFF

} zp_runtime_state_mask;
/* clang-format on */

/* Hook statistic */
typedef struct ZP_PACKED {
	uint32_t timestamp;
	uint32_t task_id;
	uint8_t state;
} zp_hook;

/* Task (Thread) runtime information */
typedef struct ZP_PACKED {
	uint32_t task_timestamp;
	uint32_t total_runtime;
	uint32_t max_runtime;
	uint32_t task_switches;
} zp_task_runtime;

/* Runtime information of a ISR */
typedef struct ZP_PACKED {
	uint32_t isr_timestamp;
	uint32_t total_runtime;
	uint32_t max_runtime;
	uint32_t isr_switches;
} zp_isr_runtime;

/* Total system uptime statistic */
typedef struct ZP_PACKED {
	uint32_t tasks_runtime[ZP_TASK_COUNT];
	uint32_t tasks_switches[ZP_TASK_COUNT];
	uint32_t max_tasks_runtime[ZP_TASK_COUNT];
	uint32_t isr_runtime[ZP_TASK_COUNT];
	uint32_t idle_runtime;
	uint32_t timestamp;
} zp_uptime_statistic_t;

#endif /* ZPERF_DATA_TYPES_H */
