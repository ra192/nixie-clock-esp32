#include <Arduino.h>
#include <AsyncTCP.h>
#include "ESPAsyncWebServer.h"

#include "SPIFFS.h"
#include "Preferences.h"

#include <nixie.h>
#include <NeoPixelBus.h>

#include <Wire.h> // must be included here so that Arduino library object file references work
#include <RtcDS3231.h>

#include <WiFiUdp.h>
#include <NTPClient.h>

#define SSID "ssid"
#define PASSWORD "password"

#define BRIGHTNESS "brightness"

#define SYNC_TIME "sync_time"
#define TIME_ZONE "time_zone"

#define DOT_1_PIN 2
#define DOT_2_PIN 23

#define pixelCount 6
#define pixelPin 27

#define colorSaturation 128

Preferences myPrefs;

AsyncWebServer server(80);

Nixie nixie;
uint8_t brightness;

RtcDS3231<TwoWire> Rtc(Wire);

NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip(pixelCount, pixelPin);

RgbColor red(colorSaturation, 0, 0);
RgbColor green(0, colorSaturation, 0);
RgbColor blue(0, 0, colorSaturation);
RgbColor white(colorSaturation);
RgbColor black(0);

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
uint8_t isSyncTime;
int timeZone;

void startAP()
{
  WiFi.mode(WIFI_AP);
  // Connect to Wi-Fi network with SSID and password
  Serial.println("Setting AP (Access Point)");
  // NULL sets an open Access Point
  WiFi.softAP("nixie", NULL);
}

void setupWifi()
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
      startAP();
    }
    else
    {
      Serial.println("Connected to wifi network");
    }
  }
  else
  {
    startAP();
  }
}

String processor(const String &var)
{
  if (var == "TIME_TEMPLATE")
  {
    RtcDateTime now = Rtc.GetDateTime();

    char timeBuf[5];
    sprintf(timeBuf, "%02d:%02d", now.Hour(), now.Minute());

    return String(timeBuf);
  }

  if (var == "SYNC_TIME_TEMPLATE")
  {
    return String(isSyncTime);
  }

  if (var == "TIME_ZONE_TEMPLATE")
  {
    return String(timeZone);
  }

  if (var == "BRIGHTNESS_TEMPLATE")
  {
    return String(brightness);
  }

  return String();
}

void setupWebserver()
{
  server.on("/save-wifi-creds", HTTP_POST, [](AsyncWebServerRequest *request)
            {
      Serial.println("save wifi creds called");
      String ssid = request->getParam(SSID, true)->value();
      Serial.printf("Ssid : %s", ssid);
      String password = request->getParam(PASSWORD, true)->value();
      Serial.printf("password : %s", ssid);

      myPrefs.putString(SSID, ssid);
      myPrefs.putString(PASSWORD, password);

      request->send(200, "text/plain", "Done. ESP will restart");
      delay(3000);

      ESP.restart(); });

  server.on("/save-time", HTTP_POST, [](AsyncWebServerRequest *request)
            {
    String time = request->getParam("time", true)->value();

    int delimInd = time.indexOf(":");

    uint8_t hour = time.substring(0,delimInd).toInt();
    uint8_t minute = time.substring(delimInd+1).toInt();

    RtcDateTime now = Rtc.GetDateTime();
    RtcDateTime updatedTime = RtcDateTime(now.Year(), now.Month(), now.Day(),hour, minute, now.Second());

    Rtc.SetDateTime(updatedTime);

    isSyncTime = request->getParam(SYNC_TIME, true)->value().toInt();
    myPrefs.putInt(SYNC_TIME, isSyncTime);

    timeZone = request->getParam(TIME_ZONE, true)->value().toInt();
    myPrefs.putInt(TIME_ZONE, timeZone);
    timeClient.setTimeOffset(timeZone);

    request->send(200, "text/plain", "Time saved"); });

  server.on("/save-settings", HTTP_POST, [](AsyncWebServerRequest *request)
            { 
              brightness = request->getParam("brightness",true)->value().toInt();
              nixie.setBrightness(brightness);
              myPrefs.putUInt(BRIGHTNESS,brightness);
              request->send(200, "text/plain", "Settings saved"); });

  server.serveStatic("/", SPIFFS, "/").setTemplateProcessor(processor).setDefaultFile("index.html");

  server.begin();
}

void updateTime(void *params)
{
  for (;;)
  {
    RtcDateTime now = Rtc.GetDateTime();
    nixie.setDigits(now.Hour() / 10, now.Hour() % 10, now.Minute() / 10, now.Minute() % 10, now.Second() / 10, now.Second() % 10);

    // Serial.printf("Sync time: %s\r\n", isSyncTime);
    // Serial.printf("Timezone: %s\r\n", timeZone);

    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void toggleDots(void *params)
{
  boolean isOn = 0;
  for (;;)
  {
    if (isOn)
    {
      digitalWrite(DOT_1_PIN, LOW);
      digitalWrite(DOT_2_PIN, LOW);
      isOn = false;
    }
    else
    {
      digitalWrite(DOT_1_PIN, HIGH);
      digitalWrite(DOT_2_PIN, HIGH);
      isOn = true;
    }
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

void syncTime(void *params)
{
  for (;;)
  {
    if (isSyncTime)
    {
      timeClient.update();

      if (timeClient.isTimeSet())
      {
        RtcDateTime now = Rtc.GetDateTime();
        RtcDateTime updatedTime = RtcDateTime(now.Year(), now.Month(), timeClient.getDay(),
                                              timeClient.getHours(), timeClient.getMinutes(), timeClient.getSeconds());
        Rtc.SetDateTime(updatedTime);
      }
    }

    vTaskDelay(60000 / portTICK_PERIOD_MS);
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

  brightness = myPrefs.getUInt(BRIGHTNESS, 100);
  nixie.setBrightness(brightness);
  nixie.begin();

  Rtc.Begin();

  pinMode(DOT_1_PIN, OUTPUT);
  pinMode(DOT_2_PIN, OUTPUT);

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

  xTaskCreate(
      toggleDots,    // Function that should be called
      "toggle dots", // Name of the task (for debugging)
      1024,          // Stack size (bytes)
      NULL,          // Parameter to pass
      1,             // Task priority
      NULL           // Task handle
  );

  setupWifi();
  setupWebserver();

  if (WiFi.isConnected())
  {
    isSyncTime = myPrefs.getInt(SYNC_TIME, 1);
    timeZone = myPrefs.getInt(TIME_ZONE);
    timeClient.setTimeOffset(timeZone);
    timeClient.begin();

    xTaskCreate(
        syncTime,    // Function that should be called
        "sync time", // Name of the task (for debugging)
        2048,        // Stack size (bytes)
        NULL,        // Parameter to pass
        1,           // Task priority
        NULL         // Task handle
    );
  }
}

void loop()
{
  nixie.refresh();
}
