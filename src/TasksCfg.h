#include <zephyr.h>

#define LED_TASK_PERIOD 500u /* ms */
#define BLE_TASK_PERIOD 300u /* ms */

/* Size of stack area used by each thread */
#define LED_TASK_STACKSIZE 1024
#define BLE_TASK_STACKSIZE 2048


/* Scheduling priority used by each thread */
#define LED_TASK_PRIORITY 7
#define BLE_TASK_PRIORITY 5

void ApplTasks__createApplTasks(void);
void ApplTasks__startApplTasks(void);
void ApplTasks__startThreadAnalyzer(void);