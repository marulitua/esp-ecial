/*
MIT License

Copyright (c) 2019 erwin maruli tua pakpahan

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

@file main.c
@author Erwin Maruli Tua Pakpahan
@brief Entry point for ESP-CIAL application.
@see https://github.com/marulitua/esp-ecial
*/

#include <stdio.h>
#include <string.h>
#include <esp_wifi.h>
#include <esp_netif.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"

#include "nvs.h"
#include "nvs_flash.h"
#include "mdns.h"
#include "lwip/api.h"
#include "lwip/err.h"
#include "lwip/netdb.h"
#include "lwip/ip4_addr.h"

#include "json.h"
#include "json.c"
#include "dns_server.h"
#include "dns_server.c"
#include "nvs_sync.h"
#include "nvs_sync.c"
#include "http_app.h"
#include "http_app.c"

#include "wifi_manager.h"
#include "wifi_manager.c"

#include "led_strip.h"
#include "led_strip.c"

/* @brief tag used for ESP serial console messages */

#define LED_TYPE LED_STRIP_WS2812
#define LED_GPIO 2
#define LED_CHANNEL RMT_CHANNEL_0
#define LED_STRIP_LEN 256

static const rgb_t colors[] = {
    { .raw = { 0x1, 0x1, 0x1 } },
    { .raw = { 0x5, 0x5, 0x5 } },
    { .raw = { 0x10, 0x10, 0x10 } },
    { .raw = { 0x15, 0x15, 0x15 } },
    { .raw = { 0x20, 0x20, 0x20 } },
    { .raw = { 0x25, 0x25, 0x25 } },
    { .raw = { 0x30, 0x30, 0x30 } },
    { .raw = { 0xff, 0xff, 0xff } },
    { .raw = { 0x00, 0x00, 0xff } },
    { .raw = { 0x00, 0xff, 0x00 } },
    { .raw = { 0xff, 0x00, 0x00 } },
};

#define COLORS_TOTAL (sizeof(colors) / sizeof(rgb_t))

/**
 * @brief RTOS task that periodically prints the heap memory available.
 * @note Pure debug information. should not be ever started on production code!
 * This is an example on how you can integrate your code with wifi-manager
 */
void monitoring_task(void *pvParameter)
{
    for(;;) {
        ESP_LOGI(TAG, "free heap: %d", esp_get_free_heap_size());
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}

void cb_connection_ok(void *pvParameter) {
    ESP_LOGI(TAG, "I have a connection!");
}

void test(void *pvParameters)
{
    led_strip_t strip = {
        .type = LED_TYPE,
        .length = LED_STRIP_LEN,
        .gpio = LED_GPIO,
        .channel = LED_CHANNEL,
        .buf = NULL,
    };

    ESP_ERROR_CHECK(led_strip_init(&strip));

    size_t c = 0;
    while (1)
    {
        ESP_ERROR_CHECK(led_strip_fill(&strip, 0, strip.length, colors[c]));
        ESP_ERROR_CHECK(led_strip_flush(&strip));

        vTaskDelay(pdMS_TO_TICKS(1000));

        if (++c >= COLORS_TOTAL)
            c = 0;
    }
}

void app_main(void)
{
    /* start the wifi manager */
    wifi_manager_start();

    /* register a callback as an example to how you can integrate your code with the wifi manager */
    wifi_manager_set_callback(WM_EVENT_STA_GOT_IP, &cb_connection_ok);

    xTaskCreatePinnedToCore(&monitoring_task, "monitoring_task", 2048, NULL, 1, NULL, 1);

    led_strip_install();
    xTaskCreate(test, "test", configMINIMAL_STACK_SIZE * 5, NULL, 5, NULL);
}
