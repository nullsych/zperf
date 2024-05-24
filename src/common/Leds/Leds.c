#include <zephyr.h>
#include <device.h>
#include <devicetree.h>
#include <drivers/gpio.h>

#include <logging/log.h>
LOG_MODULE_REGISTER(Leds, LOG_LEVEL_DBG);

#include "common/Leds/Leds.h"
#include "common/Leds/LedPins.h"

LEDS_LOCAL void Leds_setLedConfig(Leds_config *OnboardLeds, const char *, uint8_t, uint32_t);
LEDS_LOCAL void Leds_getDevTree(Leds_config *);
LEDS_LOCAL void Leds_initStates(void);

LEDS_LOCAL Leds_config OnboardLeds[LEDS_NUMBER];

LEDS_LOCAL void Leds_setLedConfig(Leds_config *OnboardLeds, const char *ledLabel, uint8_t pin, uint32_t flags)
{
   OnboardLeds->label = ledLabel;
   OnboardLeds->flags = flags;
   OnboardLeds->pin = pin;

   Leds_getDevTree(OnboardLeds);

   if (NULL == OnboardLeds->tree)
   {
      LOG_ERR("Device tree initialization failed");
   }
   else
   {
      LOG_INF("Led initialization done successfully!");
   }
}

/* Get runtime device structure for specific LED */
LEDS_LOCAL void Leds_getDevTree(Leds_config *OnboardLeds)
{
   const struct device *dev;
   int pinConfigured;

   dev = device_get_binding(OnboardLeds->label);
   if (NULL == dev)
   {
      LOG_ERR("Device structure for a driver by name failed!");
      dev = NULL;
   }
   else
   {
      pinConfigured = gpio_pin_configure(dev, OnboardLeds->pin, GPIO_OUTPUT_ACTIVE | OnboardLeds->flags);
      if (0 != pinConfigured)
      {
         LOG_ERR("Pin is not configured! ret: %d", pinConfigured);
         dev = NULL;
      }
   }

   OnboardLeds->tree = dev;
}

LEDS_LOCAL void Leds_initStates(void)
{
   Leds_idx ledIndex;

   for (ledIndex = (Leds_idx)0; ledIndex < LED_IDX_NUMBER; ledIndex++)
   {
      OnboardLeds[ledIndex].state = LED_OFF;
   }
}

void Leds_toggleLeds(uint32_t delay)
{
   Leds_idx ledIndex;
   Leds_initStates();

   for (;;)
   {
      for (ledIndex = (Leds_idx)0; ledIndex < LED_IDX_NUMBER; ledIndex++)
      {
         gpio_pin_set(OnboardLeds[ledIndex].tree, OnboardLeds[ledIndex].pin, (int)OnboardLeds[ledIndex].state);
         OnboardLeds[ledIndex].state = !(OnboardLeds[ledIndex].state);
         k_msleep(delay);
      }
   }
}

void Leds_init(void)
{
   Leds_setLedConfig(&OnboardLeds[LED_IDX_0], LED_PIN_0_LABEL, LED_PIN0, LED_PIN_0_FLAGS);
   Leds_setLedConfig(&OnboardLeds[LED_IDX_1], LED_PIN_1_LABEL, LED_PIN1, LED_PIN_1_FLAGS);
   Leds_setLedConfig(&OnboardLeds[LED_IDX_2], LED_PIN_2_LABEL, LED_PIN2, LED_PIN_2_FLAGS);
   Leds_setLedConfig(&OnboardLeds[LED_IDX_3], LED_PIN_3_LABEL, LED_PIN3, LED_PIN_3_FLAGS);
}

/* EOF */