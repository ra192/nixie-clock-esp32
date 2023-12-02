#include <Arduino.h>

#include <AsyncTCP.h>
#include "ESPAsyncWebServer.h"
#include "AsyncJson.h"
#include "ArduinoJson.h"

#include "SPIFFS.h"
#include "Preferences.h"

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
#define LED_MODE_FADE 3

Preferences myPrefs;

AsyncWebServer server(80);

Nixie nixie;

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
  WiFi.softAP(hostname.c_str(), NULL);
}

void setupWifi()
{
  if (!ssid.isEmpty() && myPrefs.isKey(PASSWORD))
  {
    String password = myPrefs.getString(PASSWORD);

    WiFi.setHostname(hostname.c_str());
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
      
      doc[NIXIE_BRIGHTNESS]=nixieBrightness;
      doc[DISPLAY_MODE]=displayMode;
      doc[TRANSITION_EFFECT]=transitionEffect;
      doc[CELSIUS_TEMP]=celsiusTemp;
      doc[DOT_MODE]=dotMode;
      doc[LED_BRIGHTNESS]=ledBrightness;
      
      uint colorUint=color.r<<16 | color.g<<8 | color.b; 
      char colorCStr[7]; 
      sprintf(colorCStr, "#%06x",colorUint);
      
      doc[LED_COLOR]=String(colorCStr);
      
      doc[LED_MODE]=ledMode;

      String jsonStr;
      serializeJson(doc,jsonStr);
      
      request->send(200,"application/json", jsonStr); });

  server.on("/get-time", HTTP_GET, [](AsyncWebServerRequest *request)
            {
      StaticJsonDocument<256> doc;
      
      doc[SYNC_TIME]=isSyncTime;
      doc[TIME_ZONE]=timeZone;
      doc[H24_FORMAT]=h24Format;

      String jsonStr;
      serializeJson(doc,jsonStr);
      
      request->send(200,"application/json", jsonStr); });

  server.on(
      "/get-network-settings", HTTP_GET, [](AsyncWebServerRequest *request)
      {
      int n = WiFi.scanNetworks();     

      StaticJsonDocument<1024> doc;

      doc[SSID_PARAM] = ssid;

      JsonArray ssidList = doc.createNestedArray("ssid_list");
      
     for(int i=0;i<n;i++)
      {
       ssidList.add(WiFi.SSID(i));
      }

      doc[HOSTNAME] = hostname;

      String jsonStr;
      serializeJson(doc,jsonStr);
      
      request->send(200,"application/json", jsonStr); });

  server.on("/save-wifi-creds", HTTP_POST, [](AsyncWebServerRequest *request)
            {
      ssid = request->getParam(SSID_PARAM, true)->value();
      myPrefs.putString(SSID_PARAM, ssid);
      
      String password = request->getParam(PASSWORD, true)->value();
      if(!password.isEmpty())
      {
        myPrefs.putString(PASSWORD, password);
      }

      hostname = request->getParam(HOSTNAME,true)->value();

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

    isSyncTime = request->getParam(SYNC_TIME, true)->value().toInt();
    myPrefs.putInt(SYNC_TIME, isSyncTime);

    timeZone = request->getParam(TIME_ZONE, true)->value();
    myPrefs.putString(TIME_ZONE, timeZone);

    configTzTime(timeZone.c_str(), "pool.ntp.org");

    h24Format = request->getParam(H24_FORMAT,true)->value().toInt();
    myPrefs.putUInt(H24_FORMAT,h24Format);

    request->redirect("/"); });

  server.on("/save-settings", HTTP_POST, [](AsyncWebServerRequest *request)
            { 
              nixieBrightness = request->getParam(NIXIE_BRIGHTNESS,true)->value().toInt();
              nixie.setBrightness(nixieBrightness);
              myPrefs.putUInt(NIXIE_BRIGHTNESS,nixieBrightness);

              transitionEffect = request->getParam(TRANSITION_EFFECT, true)->value().toInt();
              myPrefs.putUInt(TRANSITION_EFFECT, transitionEffect);

              displayMode = request->getParam(DISPLAY_MODE,true)->value().toInt();
              myPrefs.putUInt(DISPLAY_MODE, displayMode);

              ledBrightness = request->getParam(LED_BRIGHTNESS,true)->value().toInt();
              FastLED.setBrightness(ledBrightness);
              myPrefs.putUInt(LED_BRIGHTNESS,ledBrightness);

              celsiusTemp = request->getParam(CELSIUS_TEMP,true)->value().toInt();
              myPrefs.putUInt(CELSIUS_TEMP,celsiusTemp);

              dotMode = request->getParam(DOT_MODE, true)->value().toInt();
              myPrefs.putUInt(DOT_MODE,dotMode);

              String colorStr=request->getParam(LED_COLOR, true)->value();
              colorStr.replace("#","");
              int colorInt = strtol(colorStr.c_str(),0,16);
              
              color = CRGB(colorInt);
              myPrefs.putUInt(LED_COLOR, colorInt);

              ledMode = request->getParam(LED_MODE, true)->value().toInt();
              myPrefs.putUInt(LED_MODE,ledMode);

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

uint8_t getHour()
{
  if (now.Hour() == 0 && !h24Format)
  {
    return 12;
  }
  else if (now.Hour() > 12 && !h24Format)
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
      switch (displayMode)
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

        if (celsiusTemp)
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
      nixie.setDigits(hour / 10, hour % 10, now.Minute() / 10, now.Minute() % 10, now.Second() / 10, now.Second() % 10);
      vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
  }
}

void toggleDotsTask(void *params)
{
  boolean isOn = 0;
  for (;;)
  {
    switch (dotMode)
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
  uint8_t fadeScale;
  int fadeAdd;
  for (;;)
  {
    switch (ledMode)
    {
    case LED_MODE_STATIC:
      FastLED.setBrightness(ledBrightness);
      FastLED.showColor(color);
      vTaskDelay(1000 / portTICK_PERIOD_MS);
      break;

    case LED_MODE_RAINBOW:
      hue = (hue + 1) % 256;
      FastLED.setBrightness(ledBrightness);
      FastLED.showColor(CHSV(hue, 255, 255));
      vTaskDelay(100 / portTICK_PERIOD_MS);
      break;

    case LED_MODE_FADE:
      if (fadeScale == 0)
      {
        fadeAdd = 1;
      }
      else if (fadeScale == ledBrightness)
      {
        fadeAdd = -1;
      }

      fadeScale += fadeAdd;
      FastLED.setBrightness(fadeScale);
      FastLED.showColor(color);
      vTaskDelay(100 / portTICK_PERIOD_MS);
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
  h24Format = myPrefs.getUInt(H24_FORMAT, 1);
  celsiusTemp = myPrefs.getUInt(CELSIUS_TEMP, 1);

  xTaskCreate(updateNixieTask, "update nixie", 1024, NULL, 1, NULL);

  FastLED.addLeds<NEOPIXEL, LED_PIN>(leds, LED_COUNT);
  ledBrightness = myPrefs.getUInt(LED_BRIGHTNESS, 128);

  if (myPrefs.isKey(LED_COLOR))
  {
    color = CRGB(myPrefs.getUInt(LED_COLOR));
  }
  else
  {
    color = CRGB::Red;
  }

  ledMode = myPrefs.getUInt(LED_MODE, LED_MODE_STATIC);

  xTaskCreate(updateLedsTask, "update leds", 1024, NULL, 4, NULL);

  pinMode(DOT_1_PIN, OUTPUT);
  pinMode(DOT_2_PIN, OUTPUT);

  dotMode = myPrefs.getUInt(DOT_MODE, 2);

  xTaskCreate(toggleDotsTask, "toggle dots", 1024, NULL, 1, NULL);

  ssid = myPrefs.getString(SSID_PARAM);
  hostname = myPrefs.getString(HOSTNAME, "nixie");

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
