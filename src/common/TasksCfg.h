/**
 * @file   TasksCfg.h
 * @brief  Contain runtime configuration such as stack size, priority number, unique identifiers for user-defined Tasks (Threads).
 * @author kvant6ubl
 * @date   May 6, 2024
 *
 */

#ifndef SRC_COMMON_TASKS_CFG_H
#define SRC_COMMON_TASKS_CFG_H

#define LED_TASK_PERIOD            (255U)  /* ms */
#define BLE_TASK_PERIOD            (1010U) /* ms */
#define SYSPERF_TASK_PERIOD        (1000U) /* ms */

/* Size of stack area used by each thread */
#define LED_TASK_STACKSIZE         (1024U)
#define BLE_TASK_STACKSIZE         (2048U)
#define SYSPERF_TASK_STACKSIZE     (1024U)

/**
 * Scheduling priority used by each thread (descending order defined).
 * @details Numerically lower priorities takes precedence over numerically higher values.
 *          e.g. X-thread with 2 value has higher priority than Y-thread with 5 value.
*/
#define BLE_TASK_PRIORITY          (3U)
#define SYSPERF_TASK_PRIORITY      (5U)
#define LED_TASK_PRIORITY          (7U)

/* Custom task id used by each thread */
#define LED_TASKID                 (0x00)
#define BLE_TASKID                 (0x01)
#define SYSPERF_TASKID             (0x02)

#endif /* SRC_COMMON_TASKS_CFG_H */