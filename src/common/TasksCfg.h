#ifndef __COMMON_TASKS_CFG_H__
#define __COMMON_TASKS_CFG_H__

#define LED_TASK_PERIOD            (450u)  /* ms */
#define BLE_TASK_PERIOD            (1010u) /* ms */
#define SYSPERF_TASK_PERIOD        (1000u) /* ms */

/* Size of stack area used by each thread */
#define LED_TASK_STACKSIZE         (1024u)
#define BLE_TASK_STACKSIZE         (2048u)
#define SYSPERF_TASK_STACKSIZE     (1024u)

/* Scheduling priority used by each thread */
#define LED_TASK_PRIORITY          (7u)
#define BLE_TASK_PRIORITY          (3u)
#define SYSPERF_TASK_PRIORITY      (5u)

/* Custom task id used by each thread */
#define LED_TASKID                 (1u)
#define BLE_TASKID                 (2u)
#define SYSPERF_TASKID             (3u)

#endif /* __COMMON_TASKS_CFG_H__ */