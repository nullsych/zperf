/**
 * @file   Leds.h
 * @brief
 * @author nullsych
 * @date   May 24, 2024
 *
 */

#ifndef SRC_COMMON_LEDS_LEDS_H
#define SRC_COMMON_LEDS_LEDS_H

#include <zephyr/kernel.h>
#include "common/Leds/LedPins.h"

#define LEDS_LOCAL      static
#define LED_NUMBER      (4u)

/* Maped Container for GPIO pin information specified in devicetree */
typedef struct gpio_dt_spec Led_Cfg;

typedef enum
{
    LED_IDX_0 = 0,
    LED_IDX_1 = 1,
    LED_IDX_2 = 2,
    LED_IDX_3 = 3,
    LED_IDX_NUMBER = LED_NUMBER
} Led_idx;

void Leds_toggleLeds(uint32_t delay);
void Leds_toggleLeds(uint32_t);
void Leds_init(void);

#endif /* SRC_COMMON_LEDS_LEDS_H */