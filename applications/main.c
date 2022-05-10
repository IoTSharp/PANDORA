/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-06     SummerGift   first version
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include <iotsharp_client.h>

#include "wifi_config.h"
#include <wlan_mgnt.h>

#include <aht10.h>

/* defined the LED0 pin: PE7 */
#define LED0_PIN    GET_PIN(E, 7)

int main(void)
{

    rt_pin_mode(LED0_PIN, PIN_MODE_OUTPUT);

    aht10_device_t dev = NULL;
    const char *i2c_bus_name = "i2c4";
    rt_wlan_init();

    while (1)
    {
        rt_thread_mdelay(1000);
        if (!rt_wlan_is_connected())
        {
           if (rt_wlan_scan() == RT_EOK)
            {
              if (rt_wlan_connect("CMCC-501", "yykissme") == RT_EOK)
                {
                    /* register the wlan ready callback function */
                  rt_wlan_register_event_handler(RT_WLAN_EVT_READY, (void (*)(int, struct rt_wlan_buff *, void *))iotsharp_start, RT_NULL);

                    /* enable wlan auto connect */
                   rt_wlan_config_autoreconnect(RT_TRUE);
                }
            }
        }
        if (rt_wlan_is_connected())
        {
            if (dev == RT_NULL)
            {
                dev = aht10_init(i2c_bus_name);
                if (dev == RT_NULL)
                {
                    // rt_kprintf(" The sensor initializes failure");
                    iotsharp_upload_telemetry_to_device("\"aht10_status\":false");
                }
                else
                {
                    // rt_kprintf(" The sensor initializes success");
                    iotsharp_upload_telemetry_to_device("\"aht10_status\":true");
                }
            }
            else
            {
                float humidity, temperature;
                char application_message[256] = { 0 };
                humidity = aht10_read_humidity(dev);
                temperature = aht10_read_temperature(dev);
                snprintf(application_message, sizeof(application_message), "{\"humidity\":%5.2f,\"temperature\":%5.2f}",
                        humidity, temperature);
                iotsharp_upload_telemetry_to_device(application_message);
                //   lcd_show_string(1, 72, 24, "%s", application_message);
                char loc_message[256] = { 0 }; //latitude  104.053681  longitude 30.663863
                snprintf(loc_message, sizeof(loc_message), "{\"latitude\": 104.053681 ,\"longitude\":30.663863}");
                iotsharp_upload_telemetry_to_device(loc_message);

                rt_pin_write(LED0_PIN, PIN_HIGH);
                rt_thread_mdelay(500);
                rt_pin_write(LED0_PIN, PIN_LOW);
                rt_thread_mdelay(500);
            }
        }

    }
    iotsharp_stop();
}
RT_WEAK rt_tick_t rt_tick_get_millisecond(void)
{
#if 1000 % RT_TICK_PER_SECOND == 0u
    return rt_tick_get() * (1000u / RT_TICK_PER_SECOND);
#else
    #warning "rt-thread cannot provide a correct 1ms-based tick any longer,\
    please redefine this function in another file by using a high-precision hard-timer."
    return 0;
#endif
}
