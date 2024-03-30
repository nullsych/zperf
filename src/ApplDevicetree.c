#include <zephyr.h>
#include <device.h>
#include <devicetree.h>
#include <drivers/gpio.h>

#include "ApplDevicetree.h"

/* The devicetree node identifier for the "led0" alias. */
#define LED0_NODE DT_ALIAS(led0)
#define LED0 DT_GPIO_LABEL(LED0_NODE, gpios)
#define PIN0 DT_GPIO_PIN(LED0_NODE, gpios)
#define FLAGS0 DT_GPIO_FLAGS(LED0_NODE, gpios)

/* Runtime device structure (in ROM) per driver instance for LED0 */
static const struct device *dev_led0;

/* Get runtime device structure, generic for all LEDs */
static const struct device *DeviceTree__getLedDevTree(const char *LED, uint8_t PIN, uint32_t FLAGS)
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

void DeviceTree__toggleLed0(uint32_t delay)
{
   bool led_is_on;
   led_is_on = true;

   while (1)
   {
      gpio_pin_set(dev_led0, PIN0, (int)led_is_on);
      led_is_on = !led_is_on;
      k_msleep(delay);
   }
}

void DeviceTree__initLeds(void)
{
   dev_led0 = DeviceTree__getLedDevTree(LED0, PIN0, FLAGS0);
   if (dev_led0 == NULL)
   {
      printk("Device LED0 initialization failed");
      return;
   }
}