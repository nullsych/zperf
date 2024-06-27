/**
 * @file   LedPins.h
 * @brief
 * @author kvant6ubl
 * @date   May 24, 2024
 *
 */

#ifndef SRC_COMMON_LEDS_LED_PINS_H
#define SRC_COMMON_LEDS_LED_PINS_H

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>

/* Number of connected leds */
#define LEDS_NUMBER             (1U)

#define LED_ON                  (true)
#define LED_OFF                 (false)

/* The devicetree node identifier for the "led0" */
#define LED_PIN_0_NODE          DT_ALIAS(led0)


/* The devicetree node identifier for the "led1" */
// #define LED_PIN_1_NODE          DT_ALIAS(led1)
// #define LED_PIN_1_LABEL         DT_GPIO_LABEL(LED_PIN_1_NODE, gpios)
// #define LED_PIN1                DT_GPIO_PIN(LED_PIN_1_NODE, gpios)
// #define LED_PIN_1_FLAGS         DT_GPIO_FLAGS(LED_PIN_1_NODE, gpios)

/* The devicetree node identifier for the "led2" */
// #define LED_PIN_2_NODE          DT_ALIAS(led2)
// #define LED_PIN_2_LABEL         DT_GPIO_LABEL(LED_PIN_2_NODE, gpios)
// #define LED_PIN2                DT_GPIO_PIN(LED_PIN_2_NODE, gpios)
// #define LED_PIN_2_FLAGS         DT_GPIO_FLAGS(LED_PIN_2_NODE, gpios)

/* The devicetree node identifier for the "led3" */
// #define LED_PIN_3_NODE          DT_ALIAS(led3)
// #define LED_PIN_3_LABEL         DT_GPIO_LABEL(LED_PIN_3_NODE, gpios)
// #define LED_PIN3                DT_GPIO_PIN(LED_PIN_3_NODE, gpios)
// #define LED_PIN_3_FLAGS         DT_GPIO_FLAGS(LED_PIN_3_NODE, gpios)

#endif /* SRC_COMMON_LEDS_LED_PINS_H */
