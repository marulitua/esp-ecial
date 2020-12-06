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

#include "FastLED.h"
#include "FX.h"

using std::cout;
using std::endl;
using std::runtime_error;

static const char TAG[] = "esp-cial main";

CRGBPalette16 currentPalette;
TBlendType    currentBlending;

extern CRGBPalette16 myRedWhiteBluePalette;
extern const TProgmemPalette16 IRAM_ATTR myRedWhiteBluePalette_p;

#include "palettes.h"

//#define NUM_LEDS 512
#define NUM_LEDS 256
#define DATA_PIN_1 2
#define BRIGHTNESS  10
#define LED_TYPE    WS2812B
#define COLOR_ORDER RGB

CRGB leds1[NUM_LEDS];
CRGB leds2[NUM_LEDS];

#define N_COLORS 17
static const CRGB colors[N_COLORS] = { 
  CRGB::Red,
  CRGB::Green,
  CRGB::Blue,
  CRGB::White,
  CRGB::AliceBlue,
  CRGB::ForestGreen,
  CRGB::Lavender,
  CRGB::MistyRose,
  CRGB::DarkOrchid,
  CRGB::DarkOrange,
  CRGB::Black,
  CRGB::Teal,
  CRGB::Violet,
  CRGB::Lime,
  CRGB::Chartreuse,
  CRGB::BlueViolet,
  CRGB::Aqua
};

static const char *colors_names[N_COLORS] {
  "Red",
  "Green",
  "Blue",
  "White",
  "aliceblue",
  "ForestGreen",
  "Lavender",
  "MistyRose",
  "DarkOrchid",
  "DarkOrange",
  "Black",
  "Teal",
  "Violet",
  "Lime",
  "Chartreuse",
  "BlueViolet",
  "Aqua"
};

/* test using the FX unit
**
*/

static void blinkWithFx_allpatterns(void *pvParameters) {

	uint16_t mode = FX_MODE_STATIC;

	WS2812FX ws2812fx;

	ws2812fx.init(NUM_LEDS, leds1, false); // type was configured before
	ws2812fx.setBrightness(255);
	ws2812fx.setMode(0 /*segid*/, mode);


	// microseconds
	uint64_t mode_change_time = esp_timer_get_time();

	while (true) {

		if ((mode_change_time + 10000000L) < esp_timer_get_time() ) {
			mode += 1;
			mode %= MODE_COUNT;
			mode_change_time = esp_timer_get_time();
			ws2812fx.setMode(0 /*segid*/, mode);
			printf(" changed mode to %d\n", mode);
		}

		ws2812fx.service();
		vTaskDelay(10 / portTICK_PERIOD_MS); /*10ms*/
	}
};

void blinkLeds_simple(void *pvParameters){

 	while(1){

		for (int j=0;j<N_COLORS;j++) {
			printf("blink leds\n");

			for (int i=0;i<NUM_LEDS;i++) {
			  leds1[i] = colors[j];
        leds2[i] = colors[j];
			}
			FastLED.show();
			delay(1000);
		};
	}
};

void ChangePalettePeriodically(){

  uint8_t secondHand = (millis() / 1000) % 60;
  static uint8_t lastSecond = 99;

  if( lastSecond != secondHand) {
    lastSecond = secondHand;
    if( secondHand ==  0)  { currentPalette = RainbowColors_p;         currentBlending = LINEARBLEND; }
    if( secondHand == 10)  { currentPalette = RainbowStripeColors_p;   currentBlending = NOBLEND;  }
    if( secondHand == 15)  { currentPalette = RainbowStripeColors_p;   currentBlending = LINEARBLEND; }
    if( secondHand == 20)  { SetupPurpleAndGreenPalette();             currentBlending = LINEARBLEND; }
    if( secondHand == 25)  { SetupTotallyRandomPalette();              currentBlending = LINEARBLEND; }
    if( secondHand == 30)  { SetupBlackAndWhiteStripedPalette();       currentBlending = NOBLEND; }
    if( secondHand == 35)  { SetupBlackAndWhiteStripedPalette();       currentBlending = LINEARBLEND; }
    if( secondHand == 40)  { currentPalette = CloudColors_p;           currentBlending = LINEARBLEND; }
    if( secondHand == 45)  { currentPalette = PartyColors_p;           currentBlending = LINEARBLEND; }
    if( secondHand == 50)  { currentPalette = myRedWhiteBluePalette_p; currentBlending = NOBLEND;  }
    if( secondHand == 55)  { currentPalette = myRedWhiteBluePalette_p; currentBlending = LINEARBLEND; }
  }

}

void blinkLeds_interesting(void *pvParameters){
  while(1){
  	printf("blink leds\n");
    ChangePalettePeriodically();
    
    static uint8_t startIndex = 0;
    startIndex = startIndex + 1; /* motion speed */
    
    for( int i = 0; i < NUM_LEDS; i++) {
        leds1[i] = ColorFromPalette( currentPalette, startIndex, 64, currentBlending);
        leds2[i] = ColorFromPalette( currentPalette, startIndex, 64, currentBlending);
        startIndex += 3;
    }
    printf("show leds\n");
    FastLED.show();
    delay(400);
  };

};

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
    //
    // the WS2811 family uses the RMT driver
    FastLED.addLeds<LED_TYPE, DATA_PIN_1>(leds1, NUM_LEDS);

    // this is a good test because it uses the GPIO ports, these are 4 wire not 3 wire
    //FastLED.addLeds<APA102, 13, 15>(leds, NUM_LEDS);

    printf(" set max power\n");
    // I have a 2A power supply, although it's 12v
    FastLED.setMaxPowerInVoltsAndMilliamps(5,1000);
    // change the task below to one of the functions above to try different patterns
    printf("create task for led blinking\n");

    xTaskCreatePinnedToCore(&monitoring_task, "monitoring_task", 2048, NULL, 1, NULL, 1);

    //xTaskCreatePinnedToCore(&blinkWithFx_allpatterns , "blinkLeds", 4000, NULL, 5, NULL, 0);
    xTaskCreatePinnedToCore(&blinkLeds_interesting, "blinkLeds", 4000, NULL, 5, NULL, 0);
}
