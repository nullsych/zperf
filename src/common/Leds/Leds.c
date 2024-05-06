#include <zephyr.h>
#include <device.h>
#include <devicetree.h>
#include <drivers/gpio.h>

#include <logging/log.h>
LOG_MODULE_REGISTER(Leds, LOG_LEVEL_DBG);

#include "common/Leds/Leds.h"

/* The devicetree node identifier for the "led0" alias. */
#define LED0_NODE DT_ALIAS(led0)
#define LED0 DT_GPIO_LABEL(LED0_NODE, gpios)
#define PIN0 DT_GPIO_PIN(LED0_NODE, gpios)
#define FLAGS0 DT_GPIO_FLAGS(LED0_NODE, gpios)

/* Runtime device structure (in ROM) per driver instance for LED0 */
LEDS_LOCAL const struct device *Leds_dev0;

/* Get runtime device structure, generic for all LEDs */
LEDS_LOCAL const struct device *Led_getDevTree0(const char *LED, uint8_t PIN, uint32_t FLAGS)
{
   const struct device *dev;
   int retVal;

   dev = device_get_binding(LED);
   if (dev == NULL)
   {
      printk("Device structure for a driver by name failed! dev == NULL");
      return dev;
   }

   retVal = gpio_pin_configure(dev, PIN, GPIO_OUTPUT_ACTIVE | FLAGS);
   if (retVal < 0)
   {
      printk("Pin is not configured! ret == %d", retVal);
      return NULL;
   }

   return dev;
}

void Led_toggle0(uint32_t delay)
{
   bool led_is_on;
   led_is_on = true;

   for (;;)
   {
      gpio_pin_set(Leds_dev0, PIN0, (int)led_is_on);
      led_is_on = !led_is_on;

      k_msleep(delay);
   }
}

void Led_init0(void)
{
   Leds_dev0 = Led_getDevTree0(LED0, PIN0, FLAGS0);
   if (Leds_dev0 == NULL)
   {
      printk("Device LED0 initialization failed");
      return;
   }
}

/* EOF */