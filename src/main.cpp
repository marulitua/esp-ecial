#include <Arduino.h>
#include <IotWebConf.h>
#include <NeoPixelBus.h>
#include <NeoPixelAnimator.h>
#include "DisplayPixelsText.h"
#include "font8x8.h"

#define LED_PIN  14

const char thingName[] = "esp-cial";
const char wifiInitialApPassword[] = "nyamukhausdara";

const int led = 2;
unsigned long previousMillis = 0;
const long interval = 1000;
int ledState = LOW;

DNSServer dnsServer;
WebServer server(80);

IotWebConf iotWebConf(thingName, &dnsServer, &server, wifiInitialApPassword);
NeoPixelBus<MyPixelColorFeature, Neo800KbpsMethod> *strip = new NeoPixelBus<MyPixelColorFeature, Neo800KbpsMethod> (PixelCount, LED_PIN);
DisplayPixelsText *pixelText = new DisplayPixelsText();

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

  strip->Begin();
  Serial.println("Ready");
  pixelText->SetColor(RgbColor(4, 123, 56));
  //pixelText->SetText("Dad I’m hungry’ … ‘Hi hungry I’m dad");
  pixelText->SetText("The project I am working on is on behalf of a company that releases their code base as open source IF it comes to fruition then I");
  //pixelText->SetText("Angpao Nalai, Om Totong!");
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

  pixelText->UpdateAnimation();
  strip->Show();
}
