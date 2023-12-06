#include <Arduino.h>

#include <ESPmDNS.h>

#include <AsyncTCP.h>
#include "ESPAsyncWebServer.h"
#include "AsyncJson.h"
#include "ArduinoJson.h"

#include <SPIFFS.h>

#include <FastLED.h>

#include "time.h"

#include <Wire.h> // must be included here so that Arduino library object file references work
#include <RtcDS3231.h>

#include <appPreferences.h>
#include <nixie.h>

#define TIME_DISP_MODE 0
#define TIME_DATE_DISP_MODE 1
#define TIME_DATE_TEMP_DISP_MODE 2

#define NO_TRANSITION_EFFECT 0
#define SHIFT_LEFT_TRANSITION_EFFECT 1
#define SHIFT_RIGHT_TRANSITION_EFFECT 2
#define FLIP_ALL_TRANSITION_EFFECT 3
#define FLIP_SEQ_TRANSITION_EFFECT 4

#define DOT_1_PIN 2
#define DOT_2_PIN 23

#define DOT_OFF_MODE 0
#define DOT_ON_MODE 1
#define DOT_BLINK_MODE 1

#define LED_COUNT 6
#define LED_PIN 27

#define LED_MODE_OFF 0
#define LED_MODE_STATIC 1
#define LED_MODE_RAINBOW 2
#define LED_MODE_RAINBOW_CHASE 3
#define LED_MODE_FADE 4

int wifiNetworksCount;

AsyncWebServer server(80);

RtcDS3231<TwoWire> Rtc(Wire);
RtcDateTime now;
RtcTemperature temperature;

CRGB leds[LED_COUNT];

CRGB color;

void startAP()
{
  WiFi.mode(WIFI_AP);
  // Connect to Wi-Fi network with SSID and password
  Serial.println("Setting AP (Access Point)");
  // NULL sets an open Access Point
  WiFi.softAP(AppPreferences.getHostname().c_str(), NULL);
}

void setupWifi()
{
  String ssid = AppPreferences.getSSID();
  if (!ssid.isEmpty())
  {
    String password = AppPreferences.getPassword();

    WiFi.setHostname(AppPreferences.getHostname().c_str());
    WiFi.mode(WIFI_STA);
    WiFi.begin(AppPreferences.getSSID(), password);

    if (WiFi.waitForConnectResult() != WL_CONNECTED)
    {
      Serial.printf("WiFi Failed!\n");
      startAP();
    }
    else
    {
      Serial.println("Connected to wifi network");
      if (!MDNS.begin(AppPreferences.getHostname()))
      {
        Serial.println("Error starting mDNS");
        return;
      }
    }
  }
  else
  {
    startAP();
  }
}

void setupWebserver()
{
  server.on(
      "/get-network-settings", HTTP_GET, [](AsyncWebServerRequest *request)
      {     
      StaticJsonDocument<1024> doc;

      doc[SSID_PARAM] = AppPreferences.getSSID();
      doc[HOSTNAME] = AppPreferences.getHostname();
      
      JsonArray ssidList = doc.createNestedArray("ssid_list");
      for(int i=0;i<wifiNetworksCount;i++)
      {
       ssidList.add(WiFi.SSID(i));
      }

      String jsonStr;
      serializeJson(doc,jsonStr);
      
      request->send(200,"application/json", jsonStr); });

  server.on("/get-time", HTTP_GET, [](AsyncWebServerRequest *request)
            {
      StaticJsonDocument<256> doc;
      
      doc[SYNC_TIME]=AppPreferences.getSyncTime();
      doc[TIME_ZONE]=AppPreferences.getTimeZone();
      doc[H24_FORMAT]=AppPreferences.getH24Format();

      String jsonStr;
      serializeJson(doc,jsonStr);
      
      request->send(200,"application/json", jsonStr); });

  server.on("/get-settings", HTTP_GET, [](AsyncWebServerRequest *request)
            {
      StaticJsonDocument<256> doc;
      
      doc[NIXIE_BRIGHTNESS]=AppPreferences.getNixieBrightness();
      doc[DISPLAY_MODE]=AppPreferences.getDisplayMode();
      doc[TRANSITION_EFFECT]=AppPreferences.getTransitionEffect();
      doc[CELSIUS_TEMP]=AppPreferences.getCelsiusTemp();
      doc[DOT_MODE]=AppPreferences.getDotMode();

      String jsonStr;
      serializeJson(doc,jsonStr);
      
      request->send(200,"application/json", jsonStr); });

  server.on("/get-led-settings", HTTP_GET, [](AsyncWebServerRequest *request)
            {
      StaticJsonDocument<256> doc;
      
      doc[LED_BRIGHTNESS]=AppPreferences.getLedBrightness();
      
      char colorCStr[7]; 
      sprintf(colorCStr, "#%06x",AppPreferences.getLedColor());
      doc[LED_COLOR]=String(colorCStr);
      
      doc[LED_MODE]=AppPreferences.getLedMode();

      String jsonStr;
      serializeJson(doc,jsonStr);
      
      request->send(200,"application/json", jsonStr); });

  server.on("/save-wifi-creds", HTTP_POST, [](AsyncWebServerRequest *request)
            {
      String ssid = request->getParam(SSID_PARAM, true)->value();
      AppPreferences.setSSID(ssid);
      
      String password = request->getParam(PASSWORD, true)->value();
      if(!password.isEmpty())
      {
        AppPreferences.setPassword(password);
      }

      String hostname = request->getParam(HOSTNAME,true)->value();
      AppPreferences.setHostname(hostname);

      request->send(200, "text/plain", "Done. ESP will restart");
      delay(3000);

      ESP.restart(); });

  server.on("/save-time", HTTP_POST, [](AsyncWebServerRequest *request)
            {
    String time = request->getParam("time", true)->value();

    uint8_t hour = time.substring(0,2).toInt();
    uint8_t minute = time.substring(3).toInt();

    String date = request->getParam("date",true)->value();

    uint8_t year = date.substring(0,4).toInt() - 2000;
    uint8_t month = date.substring(5,7).toInt();
    uint8_t day = date.substring(8).toInt();

    RtcDateTime updatedTime = RtcDateTime(year, month, day, hour, minute, now.Second());

    Rtc.SetDateTime(updatedTime);

    uint8_t syncTime = request->getParam(SYNC_TIME, true)->value().toInt();
    AppPreferences.setSyncTime(syncTime);

    String timeZone = request->getParam(TIME_ZONE, true)->value();
    AppPreferences.setTimeZone(timeZone);

    configTzTime(timeZone.c_str(), "pool.ntp.org");

    uint8_t h24Format = request->getParam(H24_FORMAT,true)->value().toInt();
    AppPreferences.setH24Format(h24Format);

    request->redirect("/"); });

  server.on("/save-settings", HTTP_POST, [](AsyncWebServerRequest *request)
            { 
              uint8_t nixieBrightness = request->getParam(NIXIE_BRIGHTNESS,true)->value().toInt();
              Nixie.setBrightness(nixieBrightness);
              AppPreferences.setNixieBrightness(nixieBrightness);

              uint8_t transitionEffect = request->getParam(TRANSITION_EFFECT, true)->value().toInt();
              AppPreferences.setTransitionEffect(transitionEffect);

              uint8_t displayMode = request->getParam(DISPLAY_MODE,true)->value().toInt();
              AppPreferences.setDisplayMode(displayMode);

              uint8_t celsiusTemp = request->getParam(CELSIUS_TEMP,true)->value().toInt();
              AppPreferences.setCelsiusTemp(celsiusTemp);

              uint8_t dotMode = request->getParam(DOT_MODE, true)->value().toInt();
              AppPreferences.setDotMode(dotMode);

              request->redirect("/"); });

  server.on("/save-led-settings", HTTP_POST, [](AsyncWebServerRequest *request)
            { 
              uint8_t ledBrightness = request->getParam(LED_BRIGHTNESS,true)->value().toInt();
              FastLED.setBrightness(ledBrightness);
              AppPreferences.setLedBrightness(ledBrightness);

              String colorStr=request->getParam(LED_COLOR, true)->value();
              colorStr.replace("#","");
              int colorInt = strtol(colorStr.c_str(),0,16);
              
              color = CRGB(colorInt);
              AppPreferences.setLedColor(colorInt);

              uint8_t ledMode = request->getParam(LED_MODE, true)->value().toInt();
              AppPreferences.setLedMode(ledMode);

              request->redirect("/"); });

  server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");

  server.begin();
}

void updateTimeTask(void *params)
{
  for (;;)
  {
    now = Rtc.GetDateTime();
    if (AppPreferences.getDisplayMode() == TIME_DATE_TEMP_DISP_MODE)
      temperature = Rtc.GetTemperature();
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void doTransition(uint8_t dig1, uint8_t dig2, uint8_t dig3, uint8_t dig4, uint8_t dig5, uint8_t dig6)
{
  switch (AppPreferences.getTransitionEffect())
  {
  case SHIFT_LEFT_TRANSITION_EFFECT:
    Nixie.shiftLeft(dig1, dig2, dig3, dig4, dig5, dig6);
    break;
  case SHIFT_RIGHT_TRANSITION_EFFECT:
    Nixie.shiftRight(dig1, dig2, dig3, dig4, dig5, dig6);
    break;
  case FLIP_ALL_TRANSITION_EFFECT:
    Nixie.flip_all(dig1, dig2, dig3, dig4, dig5, dig6);
    break;
  case FLIP_SEQ_TRANSITION_EFFECT:
    Nixie.flip_seq(dig1, dig2, dig3, dig4, dig5, dig6);
    break;
  default:
    Nixie.setDigits(dig1, dig2, dig3, dig4, dig5, dig6);
    break;
  }
}

uint8_t getHour()
{
  if (now.Hour() == 0 && !AppPreferences.getH24Format())
  {
    return 12;
  }
  else if (now.Hour() > 12 && !AppPreferences.getH24Format())
  {
    return now.Hour() - 12;
  }
  else
  {
    return now.Hour();
  }
}

void updateNixieTask(void *params)
{
  uint8_t hour;
  for (;;)
  {
    if (now.Second() % 30 == 0)
    {
      switch (AppPreferences.getDisplayMode())
      {
      case TIME_DATE_DISP_MODE:
        doTransition(now.Day() / 10, now.Day() % 10, now.Month() / 10, now.Month() % 10, now.Year() % 100 / 10, now.Year() % 10);
        vTaskDelay(2000);

        hour = getHour();
        doTransition(hour / 10, hour % 10, now.Minute() / 10, now.Minute() % 10, now.Second() / 10, now.Second() % 10);

        break;

      case TIME_DATE_TEMP_DISP_MODE:
        doTransition(now.Day() / 10, now.Day() % 10, now.Month() / 10, now.Month() % 10, now.Year() % 100 / 10, now.Year() % 10);
        vTaskDelay(2000);

        if (AppPreferences.getCelsiusTemp())
        {
          doTransition(EMPTY_DIGIT, EMPTY_DIGIT, temperature.AsCentiDegC() / 1000, temperature.AsCentiDegC() % 1000 / 100, temperature.AsCentiDegC() % 100 / 10, EMPTY_DIGIT);
        }
        else
        {
          float temperatureInF = temperature.AsFloatDegF();
          int temperatureInF_Int = (int)temperatureInF;
          int temperatureInF_Centi = (temperatureInF - temperatureInF_Int) * 100;

          doTransition(EMPTY_DIGIT, EMPTY_DIGIT, temperatureInF_Int / 10, temperatureInF_Int % 10, temperatureInF_Centi / 10, EMPTY_DIGIT);
        }
        vTaskDelay(2000);

        hour = getHour();
        doTransition(hour / 10, hour % 10, now.Minute() / 10, now.Minute() % 10, now.Second() / 10, now.Second() % 10);

        break;

      default:
        break;
      }
    }
    else
    {
      hour = getHour();
      Nixie.setDigits(hour / 10, hour % 10, now.Minute() / 10, now.Minute() % 10, now.Second() / 10, now.Second() % 10);
      vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
  }
}

void toggleDotsTask(void *params)
{
  boolean isOn = 0;
  for (;;)
  {
    switch (AppPreferences.getDotMode())
    {
    case DOT_OFF_MODE:
      digitalWrite(DOT_1_PIN, LOW);
      digitalWrite(DOT_2_PIN, LOW);
      break;

    case DOT_ON_MODE:
      digitalWrite(DOT_1_PIN, HIGH);
      digitalWrite(DOT_2_PIN, HIGH);
      break;

    default:
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
      break;
    }
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

void updateLedsTask(void *args)
{
  uint8_t hue;
  uint8_t fadeBrightness;
  int fadeAdd;
  for (;;)
  {
    switch (AppPreferences.getLedMode())
    {
    case LED_MODE_STATIC:
      FastLED.setBrightness(AppPreferences.getLedBrightness());
      FastLED.showColor(color);
      vTaskDelay(1000 / portTICK_PERIOD_MS);
      break;

    case LED_MODE_RAINBOW:
      hue = (hue + 1) % 256;
      FastLED.setBrightness(AppPreferences.getLedBrightness());
      FastLED.showColor(CHSV(hue, 255, 255));
      vTaskDelay(20 / portTICK_PERIOD_MS);
      break;

    case LED_MODE_RAINBOW_CHASE:
      FastLED.setBrightness(AppPreferences.getLedBrightness());
      for (int i = 0; i < LED_COUNT; i++)
      {
        leds[i] = CHSV(hue + i * 12, 255, 255);
      }
      FastLED.show();
      hue = (hue + 1) % 256;
      vTaskDelay(100 / portTICK_PERIOD_MS);
      break;

    case LED_MODE_FADE:
      if (fadeBrightness == 0)
      {
        fadeAdd = 1;
      }
      else if (fadeBrightness == AppPreferences.getLedBrightness())
      {
        fadeAdd = -1;
      }
      fadeBrightness += fadeAdd;
      FastLED.setBrightness(fadeBrightness);
      FastLED.showColor(color);
      vTaskDelay(30 / portTICK_PERIOD_MS);
      break;

    default:
      FastLED.showColor(0);
      vTaskDelay(1000 / portTICK_PERIOD_MS);
      break;
    }
  }
}

void syncTimeTask(void *params)
{
  for (;;)
  {
    if (AppPreferences.getSyncTime())
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

  AppPreferences.begin();

  Rtc.Begin();

  now = Rtc.GetDateTime();

  xTaskCreate(updateTimeTask, "update time", 2048, NULL, 1, NULL);

  Nixie.setBrightness(AppPreferences.getNixieBrightness());
  Nixie.begin();

  xTaskCreate(updateNixieTask, "update nixie", 1024, NULL, 1, NULL);

  FastLED.addLeds<NEOPIXEL, LED_PIN>(leds, LED_COUNT);

  color = CRGB(AppPreferences.getLedColor());

  xTaskCreate(updateLedsTask, "update leds", 1024, NULL, 4, NULL);

  pinMode(DOT_1_PIN, OUTPUT);
  pinMode(DOT_2_PIN, OUTPUT);

  xTaskCreate(toggleDotsTask, "toggle dots", 1024, NULL, 1, NULL);

  setupWifi();
  setupWebserver();

  if (WiFi.isConnected())
  {
    configTzTime(AppPreferences.getTimeZone().c_str(), "pool.ntp.org");

    xTaskCreate(syncTimeTask, "sync time", 2048, NULL, 1, NULL);
  }

  wifiNetworksCount = WiFi.scanNetworks();
}

void loop()
{
}
