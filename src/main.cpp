#include <Arduino.h>
#include <IotWebConf.h>
#include <FastLED.h>
#include <sprite.h>
#include <matrix.h>

#define LED_PIN  14
#define BRIGHTNESS 3
#define CHIPSET WS2812B
#define COLOR_ORDER GRB

const char thingName[] = "esp-cial";
const char wifiInitialApPassword[] = "nyamukhausdara";

// Params for width and height
const uint8_t kMatrixWidth = 16;
const uint8_t kMatrixHeight = 16;

const bool    kMatrixSerpentineLayout = true;
#define NUM_LEDS (kMatrixWidth * kMatrixHeight)
CRGB leds_plus_safety_pixel[ NUM_LEDS + 1];
CRGB leds[ NUM_LEDS ];
#define LAST_VISIBLE_LED 255

const int led = 2;
unsigned long previousMillis = 0;
const long interval = 1000;
int ledState = LOW;

DNSServer dnsServer;
WebServer server(80);

IotWebConf iotWebConf(thingName, &dnsServer, &server, wifiInitialApPassword);

void handleRoot()
{
  if (iotWebConf.handleCaptivePortal())
  {
    return;
  }
  String s = "<!DOCTYPE html><html lang=\"en\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/>";
  s += "<title>IotWebConf 01 Minimal</title></head><body>Hello world!";
  s += "Go to <a href='config'>configure page</a> to change setting.";
  s += "</body></html>\n";

  server.send(200, "text/html", s);
}

uint8_t XY (uint8_t x, uint8_t y) {
  // any out of bounds address maps to the first hidden pixel
  if ( (x >= kMatrixWidth) || (y >= kMatrixHeight) ) {
    return (LAST_VISIBLE_LED + 1);
  }

  uint8_t i = (y * kMatrixWidth) + x;
  uint8_t j = XYTable[i];
  return j;
}

void setup()
{
  pinMode(led, OUTPUT);
  Serial.begin(115200);
  Serial.println();
  Serial.println("Starting up...");

  iotWebConf.init();

  server.on("/", handleRoot);
  server.on("/config", []{ iotWebConf.handleConfig(); });
  server.onNotFound([](){ iotWebConf.handleNotFound(); });

  /*handling uploading firmware file */
  server.on("/update", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      Serial.printf("Update: %s\n", upload.filename.c_str());
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      /* flashing firmware to ESP*/
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { //true to set the size to the current progress
        Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
    }
  });

  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalSMD5050);
  FastLED.setBrightness( BRIGHTNESS );
  for (byte y = 0; y < kMatrixHeight; y++) {
    for (byte x = 0; x < kMatrixWidth; x++) {
      if (DigDug01[ kMatrixWidth * y + x ] != 0x000000) {
        leds[ XY(x, y) ]  = pgm_read_dword(&(DigDug01[ kMatrixWidth * y + x ]));
      }
      else {
        //leds[ XY(x, y) ]  = CRGB::Yellow;
      }
    }
  }

  FastLED.show();
  Serial.println("Ready");
}

void loop()
{
  iotWebConf.doLoop();

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    ledState = not(ledState);

    digitalWrite(led, ledState);
  }
}
