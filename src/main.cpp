#include <Arduino.h>

#include <AsyncTCP.h>
#include "ESPAsyncWebServer.h"
#include "AsyncJson.h"
#include "ArduinoJson.h"

#include "SPIFFS.h"
#include "Preferences.h"

#include <nixie.h>
#include <FastLED.h>

#include "time.h"

#include <Wire.h> // must be included here so that Arduino library object file references work
#include <RtcDS3231.h>

#define SSID "ssid"
#define PASSWORD "password"

#define NIXIE_BRIGHTNESS "nixie_brightness"

#define SYNC_TIME "sync_time"
#define TIME_ZONE "time_zone"

#define DISPLAY_MODE "display_mode"

#define TIME_DISP_MODE 0
#define TIME_DATE_DISP_MODE 1
#define TIME_DATE_TEMP_DISP_MODE 2

#define TRANSITION_EFFECT "transition_effect"

#define NO_TRANSITION_EFFECT 0
#define SHIFT_LEFT_TRANSITION_EFFECT 1
#define SHIFT_RIGHT_TRANSITION_EFFECT 2
#define FLIP_ALL_TRANSITION_EFFECT 3
#define FLIP_SEQ_TRANSITION_EFFECT 4

#define DOT_1_PIN 2
#define DOT_2_PIN 23

#define LED_COUNT 6
#define LED_PIN 27
#define LED_BRIGHTNESS "led_brightness"
#define LED_COLOR "led_color"

Preferences myPrefs;

AsyncWebServer server(80);

Nixie nixie;
uint8_t nixieBrightness;

RtcDS3231<TwoWire> Rtc(Wire);
RtcDateTime now;
RtcTemperature temperature;

int isSyncTime;
String timeZone;

uint8_t displayMode;

uint8_t transitionEffect;

CRGB leds[LED_COUNT];
uint8_t ledBrightness;
CRGB color;

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

void setupWebserver()
{
  server.on("/get-settings", HTTP_GET, [](AsyncWebServerRequest *request)
            {
      StaticJsonDocument<256> doc;
      
      doc[SSID] = myPrefs.getString(SSID);
      doc[SYNC_TIME]=isSyncTime;
      doc[TIME_ZONE]=timeZone;
      doc[NIXIE_BRIGHTNESS]=nixieBrightness;
      doc[DISPLAY_MODE]=displayMode;
      doc[TRANSITION_EFFECT]=transitionEffect;
      doc[LED_BRIGHTNESS]=ledBrightness;
      doc[LED_COLOR]=color.r<<16 | color.g<<8 | color.b;

      String jsonStr;
      serializeJson(doc,jsonStr);
      
      request->send(200,"application/json", jsonStr); });

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

    RtcDateTime updatedTime = RtcDateTime(now.Year(), now.Month(), now.Day(),hour, minute, now.Second());

    Rtc.SetDateTime(updatedTime);

    isSyncTime = request->getParam(SYNC_TIME, true)->value().toInt();
    myPrefs.putInt(SYNC_TIME, isSyncTime);

    timeZone = request->getParam(TIME_ZONE, true)->value();
    myPrefs.putString(TIME_ZONE, timeZone);

    configTzTime(timeZone.c_str(), "pool.ntp.org");

    request->redirect("/"); });

  server.on("/save-settings", HTTP_POST, [](AsyncWebServerRequest *request)
            { 
              nixieBrightness = request->getParam(NIXIE_BRIGHTNESS,true)->value().toInt();
              nixie.setBrightness(nixieBrightness);
              myPrefs.putUInt(NIXIE_BRIGHTNESS,nixieBrightness);

              transitionEffect = request->getParam(TRANSITION_EFFECT, true)->value().toInt();
              myPrefs.putUInt(TRANSITION_EFFECT, transitionEffect);

              ledBrightness = request->getParam(LED_BRIGHTNESS,true)->value().toInt();
              FastLED.setBrightness(ledBrightness);
              myPrefs.putUInt(LED_BRIGHTNESS,ledBrightness);

              uint32_t colorUint=request->getParam(LED_COLOR, true)->value().toInt();
              color = CRGB(colorUint);
              myPrefs.putUInt(LED_COLOR, colorUint);

              displayMode = request->getParam(DISPLAY_MODE,true)->value().toInt();
              myPrefs.putUInt(DISPLAY_MODE, displayMode);

              request->redirect("/"); });

  server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");

  server.begin();
}

void updateTimeTask(void *params)
{
  for (;;)
  {
    now = Rtc.GetDateTime();
    if (displayMode == TIME_DATE_TEMP_DISP_MODE)
      temperature = Rtc.GetTemperature();
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void doTransition(uint8_t dig1, uint8_t dig2, uint8_t dig3, uint8_t dig4, uint8_t dig5, uint8_t dig6)
{
  switch (transitionEffect)
  {
  case SHIFT_LEFT_TRANSITION_EFFECT:
    nixie.shiftLeft(dig1, dig2, dig3, dig4, dig5, dig6);
    break;
  case SHIFT_RIGHT_TRANSITION_EFFECT:
    nixie.shiftRight(dig1, dig2, dig3, dig4, dig5, dig6);
    break;
  case FLIP_ALL_TRANSITION_EFFECT:
    nixie.flip_all(dig1, dig2, dig3, dig4, dig5, dig6);
    break;
  case FLIP_SEQ_TRANSITION_EFFECT:
    nixie.flip_seq(dig1, dig2, dig3, dig4, dig5, dig6);
    break;
  default:
    nixie.setDigits(dig1, dig2, dig3, dig4, dig5, dig6);
    break;
  }
}

void updateNixieTask(void *params)
{
  for (;;)
  {
    if (now.Second() % 30 == 0)
    {
      switch (displayMode)
      {
      case TIME_DATE_DISP_MODE:
        doTransition(now.Day() / 10, now.Day() % 10, now.Month() / 10, now.Month() % 10, now.Year() % 100 / 10, now.Year() % 10);
        vTaskDelay(2000);

        doTransition(now.Hour() / 10, now.Hour() % 10, now.Minute() / 10, now.Minute() % 10, now.Second() / 10, now.Second() % 10);

        break;

      case TIME_DATE_TEMP_DISP_MODE:
        doTransition(now.Day() / 10, now.Day() % 10, now.Month() / 10, now.Month() % 10, now.Year() % 100 / 10, now.Year() % 10);
        vTaskDelay(2000);

        doTransition(EMPTY_DIGIT, temperature.AsCentiDegC() / 1000, temperature.AsCentiDegC() % 1000 / 100, temperature.AsCentiDegC() % 100 / 10, temperature.AsCentiDegC() % 10, EMPTY_DIGIT);
        vTaskDelay(2000);

        doTransition(now.Hour() / 10, now.Hour() % 10, now.Minute() / 10, now.Minute() % 10, now.Second() / 10, now.Second() % 10);

        break;

      default:
        break;
      }
    }
    else
    {
      nixie.setDigits(now.Hour() / 10, now.Hour() % 10, now.Minute() / 10, now.Minute() % 10, now.Second() / 10, now.Second() % 10);
      vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
  }
}

void toggleDotsTask(void *params)
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

void updateLedsTask(void *args)
{
  for (;;)
  {
    FastLED.showColor(color);
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

void syncTimeTask(void *params)
{
  for (;;)
  {
    if (isSyncTime)
    {
      struct tm timeinfo;

      if (getLocalTime(&timeinfo))
      {
        RtcDateTime updatedTime = RtcDateTime(timeinfo.tm_year % 100, timeinfo.tm_mon + 1, timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
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

  Rtc.Begin();

  now = Rtc.GetDateTime();

  xTaskCreate(updateTimeTask, "update time", 2048, NULL, 1, NULL);

  nixieBrightness = myPrefs.getUInt(NIXIE_BRIGHTNESS, 255);
  nixie.setBrightness(nixieBrightness);
  nixie.begin();

  displayMode = myPrefs.getUInt(DISPLAY_MODE, TIME_DISP_MODE);

  transitionEffect = myPrefs.getUInt(TRANSITION_EFFECT, SHIFT_LEFT_TRANSITION_EFFECT);

  xTaskCreate(updateNixieTask, "update nixie", 1024, NULL, 1, NULL);

  FastLED.addLeds<NEOPIXEL, LED_PIN>(leds, LED_COUNT);
  ledBrightness = myPrefs.getUInt(LED_BRIGHTNESS, 128);
  FastLED.setBrightness(ledBrightness);
  if (myPrefs.isKey(LED_COLOR))
  {
    color = CRGB(myPrefs.getUInt(LED_COLOR));
  }
  else
  {
    color = CRGB::Red;
  }

  xTaskCreate(updateLedsTask, "update leds", 1024, NULL, 4, NULL);

  pinMode(DOT_1_PIN, OUTPUT);
  pinMode(DOT_2_PIN, OUTPUT);

  xTaskCreate(toggleDotsTask, "toggle dots", 1024, NULL, 1, NULL);

  setupWifi();
  setupWebserver();

  if (WiFi.isConnected())
  {
    isSyncTime = myPrefs.getInt(SYNC_TIME, 1);
    timeZone = myPrefs.getString(TIME_ZONE, "Etc/GMT");

    configTzTime(timeZone.c_str(), "pool.ntp.org");

    xTaskCreate(syncTimeTask, "sync time", 2048, NULL, 1, NULL);
  }
}

void loop()
{
}
