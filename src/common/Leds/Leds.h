/**
 * @file   Leds.h
 * @brief
 * @author kvant6ubl
 * @date   May 24, 2024
 *
 */

#ifndef SRC_COMMON_LEDS_LEDS_H
#define SRC_COMMON_LEDS_LEDS_H

#include <zephyr.h>
#include "common/Leds/LedPins.h"

#define LEDS_LOCAL static

typedef struct
{
    bool state;
    uint8_t pin;
    const char *label;
    const struct device *tree;
    uint32_t flags;
} Leds_config;

typedef enum
{
    LED_IDX_0 = 0,
    LED_IDX_1 = 1,
    LED_IDX_2 = 2,
    LED_IDX_3 = 3,
    LED_IDX_NUMBER = (LED_IDX_3 + 1)
} Leds_idx;

void Leds_toggleLeds(uint32_t delay);
void Leds_init(void);

#endif /* SRC_COMMON_LEDS_LEDS_H */