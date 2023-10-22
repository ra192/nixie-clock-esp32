#include <Arduino.h>
#include <AsyncTCP.h>
#include "ESPAsyncWebServer.h"

#include "SPIFFS.h"
#include "Preferences.h"

#include <nixie.h>
#include <NeoPixelBus.h>

#include <Wire.h> // must be included here so that Arduino library object file references work
#include <RtcDS3231.h>

#define SSID "ssid"
#define PASSWORD "password"

#define pixelCount 6
#define pixelPin 27

#define colorSaturation 128

Preferences myPrefs;

AsyncWebServer server(80);

Nixie nixie;

RtcDS3231<TwoWire> Rtc(Wire);

NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip(pixelCount, pixelPin);

RgbColor red(colorSaturation, 0, 0);
RgbColor green(0, colorSaturation, 0);
RgbColor blue(0, 0, colorSaturation);
RgbColor white(colorSaturation);
RgbColor black(0);

void startWifiManager()
{
  WiFi.mode(WIFI_AP);
  // Connect to Wi-Fi network with SSID and password
  Serial.println("Setting AP (Access Point)");
  // NULL sets an open Access Point
  WiFi.softAP("nixie", NULL);
}

void setup_wifi()
{
  if (myPrefs.isKey(SSID) && myPrefs.isKey(PASSWORD))
  {
    String ssid = myPrefs.getString(SSID);
    String password = myPrefs.getString(PASSWORD);

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    if (WiFi.waitForConnectResult() != WL_CONNECTED)
    {
      Serial.printf("WiFi Failed!\n");
      startWifiManager();
    }
  }
  else
  {
    startWifiManager();
  }

  Serial.println("Connected to wifi network");
}

String processor(const String &var)
{
  if (var == "TIME_TEMPLATE")
  {
    RtcDateTime now = Rtc.GetDateTime();

    String time(now.Hour());
    time.concat(":");
    time.concat(now.Minute());

    return time;
  }

  return String();
}

void setup_webserver()
{
  server.rewrite("/", "wifi-manager.html").setFilter(ON_AP_FILTER);
  server.on("/save-wifi-creds", HTTP_POST, [](AsyncWebServerRequest *request)
            {
      String ssid = request->getParam("ssid", true)->value();
      String password = request->getParam("pass", true)->value();

      myPrefs.putString(SSID, ssid);
      myPrefs.putString(PASSWORD, password);

      request->send(200, "text/plain", "Done. ESP will restart");
      delay(3000);

      ESP.restart(); });

  server.on("/save-time", HTTP_POST, [](AsyncWebServerRequest *request)
            {
    String time = request->getParam("time")->value();
    int delimInd = time.indexOf(":");

    uint8_t hour = atoi(time.substring(0,delimInd).c_str());
    uint8_t minute = atoi(time.substring(delimInd+1).c_str());

    RtcDateTime now = Rtc.GetDateTime();
    RtcDateTime updatedTime = RtcDateTime(now.Year(), now.Month(), now.Day(),hour, minute, now.Second());

    Rtc.SetDateTime(updatedTime);

    request->send(200, "text/plain", "Time saved"); });

  server.serveStatic("/", SPIFFS, "/").setTemplateProcessor(processor);

  server.begin();
}

void updateTime(void *params)
{
  for (;;)
  {
    RtcDateTime now = Rtc.GetDateTime();
    nixie.set_digits(now.Hour() / 10, now.Hour() % 10, now.Minute() / 10, now.Minute() % 10, now.Second() / 10, now.Second() % 10);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void setup()
{
  Serial.begin(115200);

  myPrefs.begin("myPrefs", false);

  // Initialize SPIFFS
  if (!SPIFFS.begin(true))
  {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  setup_wifi();
  setup_webserver();

  nixie.begin();
  Rtc.Begin();

  strip.Begin();
  strip.ClearTo(red);
  strip.Show();

  xTaskCreate(
      updateTime,    // Function that should be called
      "update time", // Name of the task (for debugging)
      2048,          // Stack size (bytes)
      NULL,          // Parameter to pass
      1,             // Task priority
      NULL           // Task handle
  );
}

void loop()
{
  nixie.refresh();
  vTaskDelay(2 / portTICK_PERIOD_MS);
}
