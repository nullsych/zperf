#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(Leds, LOG_LEVEL_DBG);

#include "common/Leds/Leds.h"
#include "common/Leds/LedPins.h"

static const Led_Cfg led = GPIO_DT_SPEC_GET(LED_PIN_0_NODE, gpios);

void Leds_toggleLeds(uint32_t delay)
{
   int ret;

   while (1)
   {
      ret = gpio_pin_toggle_dt(&led);
	   if (ret < 0)
      {
         LOG_ERR("Error while toggle pin level!");
	   }
      k_msleep(delay);
      // LOG_INF("Toggling on-board LEDs");
   }
}

void Leds_init(void)
{
	if (!gpio_is_ready_dt(&led))
   {
      LOG_ERR("GPIO is not ready for configuration!");
	}
   else
   {
      int ret;
      ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
	   if (ret < 0)
      {
         LOG_ERR("GPIO is not configured!");
	   }
   }
}

/* EOF */