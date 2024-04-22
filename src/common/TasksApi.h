#ifndef __COMMON_TASKS_API_H__
#define __COMMON_TASKS_API_H__

#include <zephyr.h>
#include <zephyr/types.h>

void Led_createTask(void);
void Ble_createTask(void);
// void ApplTasks__startThreadAnalyzer(void);

#endif /* __COMMON_TASKS_API_H__ */