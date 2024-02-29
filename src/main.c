/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <device.h>
#include <devicetree.h>
#include <drivers/gpio.h>
// #include <logging/log.h>

/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS 500u

/* The devicetree node identifier for the "led0" alias. */
#define LED0_NODE DT_ALIAS(led0)
#define LED1_NODE DT_ALIAS(led1)
#define LED2_NODE DT_ALIAS(led2)

#define LED0 DT_GPIO_LABEL(LED0_NODE, gpios)
#define PIN0 DT_GPIO_PIN(LED0_NODE, gpios)
#define FLAGS0 DT_GPIO_FLAGS(LED0_NODE, gpios)

#define LED1 DT_GPIO_LABEL(LED1_NODE, gpios)
#define PIN1 DT_GPIO_PIN(LED1_NODE, gpios)
#define FLAGS1 DT_GPIO_FLAGS(LED1_NODE, gpios)

#define LED2 DT_GPIO_LABEL(LED2_NODE, gpios)
#define PIN2 DT_GPIO_PIN(LED2_NODE, gpios)
#define FLAGS2 DT_GPIO_FLAGS(LED2_NODE, gpios)

/* Make common */
#define DEV_OK 0u
#define DEV_NOK 1u

/* Size of stack area used by each thread */
#define STACKSIZE 1024

/* Scheduling priority used by each thread */
#define PRIORITY 7

/* Runtime device structure (in ROM) per driver instance for LED0 */
const struct device *dev_led0;
/* Runtime device structure (in ROM) per driver instance for LED1 */
const struct device *dev_led1;
/* Runtime device structure (in ROM) per driver instance for LED2 */
const struct device *dev_led2;

/* Thread Structure */
static struct k_thread Task_Blink_LED0__data;

K_THREAD_STACK_DEFINE(Task_Blink_LED0___stack_area, STACKSIZE);

/* Get runtime device structure */
const struct device *Device_getLedDevice(const char *LED, uint8_t PIN, uint32_t FLAGS)
{
   const struct device *dev;
   int retVal;

   dev = device_get_binding(LED);
   if (dev == NULL)
   {
      printk("Device structure for a driver by name failed! dev == NULL");
      return NULL;
   }

   retVal = gpio_pin_configure(dev, PIN, GPIO_OUTPUT_ACTIVE | FLAGS);
   if (retVal < 0)
   {
      printk("Pin is not configured! ret == %d", retVal);
      return NULL;
   }

   return dev;
}

void Blink_LED(const struct device *dev, uint8_t PIN)
{
   bool led_is_on = true;
   while (1)
   {
      printk("Task_Blink_LED0 : GPIO state: %d", led_is_on);
      gpio_pin_set(dev, PIN, (int)led_is_on);
      led_is_on = !led_is_on;
      k_msleep(SLEEP_TIME_MS);
   }
}

void TASK_Blink_LED0(void *unused_arg1, void *unused_arg2, void *unused_arg3)
{
	ARG_UNUSED(unused_arg1);
	ARG_UNUSED(unused_arg2);
	ARG_UNUSED(unused_arg3);

   Blink_LED(dev_led0, PIN0);
}

void main(void)
{
   dev_led0 = Device_getLedDevice(LED0, PIN0, FLAGS0);
   if (dev_led0 == NULL)
   {
      printk("Device 1 Initialization failed");
      return;
   }

   k_thread_create(&Task_Blink_LED0__data, Task_Blink_LED0___stack_area,
			K_THREAD_STACK_SIZEOF(Task_Blink_LED0___stack_area),
			TASK_Blink_LED0, NULL, NULL, NULL,
			PRIORITY, 0, K_FOREVER);

	k_thread_name_set(&Task_Blink_LED0__data, "Task_Blink_LED0");

   k_thread_start(&Task_Blink_LED0__data);
}

/* EOF */
