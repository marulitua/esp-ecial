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

#include <iostream>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"

using std::cout;
using std::endl;
using std::runtime_error;

static const char TAG[] = "esp-cial main";

/* A simple class which may throw an exception from constructor */
class Throwing
{
public:
    Throwing(int arg)
    : m_arg(arg)
    {
        cout << "In constructor, arg=" << arg << endl;
        if (arg == 0) {
            throw runtime_error("Exception in constructor");
        }
    }

    ~Throwing()
    {
        cout << "In destructor, m_arg=" << m_arg << endl;
    }

protected:
    int m_arg;
};

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

/* Inside .cpp file, app_main function must be declared with C linkage */
extern "C" void app_main(void)
{
    cout << "app_main starting" << endl;

    try {
        /* This will succeed */
        Throwing obj1(42);

        /* This will throw an exception */
        Throwing obj2(0);

        cout << "This will not be printed" << endl;
    } catch (const runtime_error &e) {
        cout << "Exception caught: " << e.what() << endl;
    }

    cout << "app_main done" << endl;

    xTaskCreatePinnedToCore(&monitoring_task, "monitoring_task", 2048, NULL, 1, NULL, 1);
}
