#include <Arduino.h>

#include <SPIFFS.h>
#include "time.h"

#include <ESPmDNS.h>
#include <AsyncTCP.h>
#include "ESPAsyncWebServer.h"
#include "AsyncJson.h"
#include "ArduinoJson.h"

#include <Wire.h> // must be included here so that Arduino library object file references work
#include <RtcDS3231.h>

#include <appPreferences.h>
#include <nixie.h>
#include <led.h>

int wifiNetworksCount;

AsyncWebServer server(80);

RtcDS3231<TwoWire> Rtc(Wire);
RtcDateTime now;
RtcTemperature temperature;

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
    }
  }
  else
  {
    startAP();
  }

  if (!MDNS.begin(AppPreferences.getHostname()))
  {
    Serial.println("Error starting mDNS");
    return;
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
      doc[NIGHT_BRIGHTNESS_PERCENT] = AppPreferences.getNightBrightnessPercent();
      
      uint16_t nightFromInMins = AppPreferences.getNightFromInMinutes();
      char nightFrom[5];
      sprintf(nightFrom,"%02u:%02u", nightFromInMins/60, nightFromInMins % 60);
      doc[NIGHT_FROM] = nightFrom;
      
      uint16_t nightToInMins = AppPreferences.getNightToInMinutes();
      char nightTo[5];
      sprintf(nightTo,"%02u:%02u", nightToInMins/60, nightToInMins % 60);
      doc[NIGHT_TO] = nightTo; 

      doc[DISPLAY_MODE_FREQ]=AppPreferences.getDisplayModeFreq();
      doc[DISPLAY_MODE]=AppPreferences.getDisplayMode();
      doc[DIGIT_EFFECT]=AppPreferences.getDigitEffect();
      doc[TRANSITION_EFFECT]=AppPreferences.getTransitionEffect();
      doc[CELSIUS_TEMP]=AppPreferences.getCelsiusTemp();
      doc[DOT_BRIGHTNESS]=AppPreferences.getDotBrightness();
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
    uint8_t minute = time.substring(3,5).toInt();
    uint8_t second = time.substring(6).toInt();

    String date = request->getParam("date",true)->value();

    uint8_t year = date.substring(0,4).toInt() - 2000;
    uint8_t month = date.substring(5,7).toInt();
    uint8_t day = date.substring(8).toInt();

    RtcDateTime updatedTime = RtcDateTime(year, month, day, hour, minute, second);

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

              uint8_t nightBrightness = request->getParam(NIGHT_BRIGHTNESS_PERCENT, true)->value().toInt();
              AppPreferences.setNightBrightnessPercent(nightBrightness);

              String nightFrom = request->getParam(NIGHT_FROM, true)->value();
              uint16_t nightFromInMinutes = nightFrom.substring(0,2).toInt()*60 + nightFrom.substring(3).toInt();
              AppPreferences.setNightFromInMinutes(nightFromInMinutes);

              String nightTo = request->getParam(NIGHT_TO, true)->value();
              uint16_t nightToInMinutes = nightTo.substring(0,2).toInt()*60 + nightTo.substring(3).toInt();
              AppPreferences.setNightToInMinutes(nightToInMinutes);

              uint8_t displayModeFreq = request->getParam(DISPLAY_MODE_FREQ,true)->value().toInt();
              AppPreferences.setDisplayModeFreq(displayModeFreq);

              uint8_t displayMode = request->getParam(DISPLAY_MODE,true)->value().toInt();
              AppPreferences.setDisplayMode(displayMode);

              uint8_t digitEffect = request->getParam(DIGIT_EFFECT, true)->value().toInt();
              AppPreferences.setDigitEffect(digitEffect);

              uint8_t transitionEffect = request->getParam(TRANSITION_EFFECT, true)->value().toInt();
              AppPreferences.setTransitionEffect(transitionEffect);

              uint8_t celsiusTemp = request->getParam(CELSIUS_TEMP,true)->value().toInt();
              AppPreferences.setCelsiusTemp(celsiusTemp);

              uint8_t dotBrightness = request->getParam(DOT_BRIGHTNESS, true)->value().toInt();
              AppPreferences.setDotBrightness(dotBrightness);

              uint8_t dotMode = request->getParam(DOT_MODE, true)->value().toInt();
              AppPreferences.setDotMode(dotMode);

              request->redirect("/"); });

  server.on("/save-led-settings", HTTP_POST, [](AsyncWebServerRequest *request)
            { 
              uint8_t ledBrightness = request->getParam(LED_BRIGHTNESS, true)->value().toInt();
              Led.setBrightness(ledBrightness);
              AppPreferences.setLedBrightness(ledBrightness);

              if(request->hasParam(LED_COLOR, true))
              {
                String colorStr=request->getParam(LED_COLOR, true)->value();
                colorStr.replace("#","");
                int colorInt = strtol(colorStr.c_str(),0,16);
              
                Led.setColor(colorInt);
                AppPreferences.setLedColor(colorInt);
              }

              uint8_t ledMode = request->getParam(LED_MODE, true)->value().toInt();
              Led.setMode(ledMode);
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
    if (AppPreferences.getDisplayMode() == TIME_DATE_TEMP_DISP_MODE || AppPreferences.getDisplayMode() == TIME_TEMP_DISP_MODE)
      temperature = Rtc.GetTemperature();
    vTaskDelay(1000 / portTICK_PERIOD_MS);
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

uint16_t getTempCenti()
{
  if (AppPreferences.getCelsiusTemp())
  {
    return temperature.AsCentiDegC();
  }
  else
  {
    return temperature.AsFloatDegF() * 100;
  }
}

void changeDigits(uint8_t dig1, uint8_t dig2, uint8_t dig3, uint8_t dig4, uint8_t dig5, uint8_t dig6)
{
  switch (AppPreferences.getDigitEffect())
  {
  case FLIP_DIGIT_EFFECT:
    Nixie.flip(dig1, dig2, dig3, dig4, dig5, dig6, false);
    break;
  case FLIP_SEQ_DIGIT_EFFECT:
    Nixie.flipSeq(dig1, dig2, dig3, dig4, dig5, dig6, false);
    break;
  case FADE_DIGIT_EFFECT:
    Nixie.fade(dig1, dig2, dig3, dig4, dig5, dig6, false);
    break;
  default:
    Nixie.setDigits(dig1, dig2, dig3, dig4, dig5, dig6);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

bool checkTransition()
{
  switch (AppPreferences.getDisplayModeFreq())
  {
  case EVERY_30_SEC_DISP_MODE:
    return now.Second() % 30 == 0;
    break;

  case EVERY_MIN_DISP_MODE:
    return now.Second() == 30;
    break;

  case EVERY_2_MIN_DISP_MODE:
    return now.Minute() % 2 == 0 && now.Second() == 30;
    break;

  default:
    return now.Minute() % 5 == 0 && now.Second() == 30;
    break;
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
  case FLIP_TRANSITION_EFFECT:
    Nixie.flip(dig1, dig2, dig3, dig4, dig5, dig6);
    break;
  case FLIP_SEQ_TRANSITION_EFFECT:
    Nixie.flipSeq(dig1, dig2, dig3, dig4, dig5, dig6);
    break;
  case FADE_TRANSITION_EFFECT:
    Nixie.fade(dig1, dig2, dig3, dig4, dig5, dig6);
    break;
  default:
    Nixie.setDigits(dig1, dig2, dig3, dig4, dig5, dig6);
    break;
  }
}

bool isNightTime()
{
  uint16_t nightFromInMinutes = AppPreferences.getNightFromInMinutes();
  uint16_t nightToInMinutes = AppPreferences.getNightToInMinutes();
  uint16_t nowInMins = now.Hour() * 60 + now.Minute();

  return (nightFromInMinutes <= nightToInMinutes && nowInMins >= nightFromInMinutes && nowInMins < nightToInMinutes) ||
         (nightFromInMinutes > nightToInMinutes && (nowInMins < nightToInMinutes || nowInMins >= nightFromInMinutes));
}

void setBrightness(void)
{
  uint8_t nixieBrightness;
  uint8_t ledBrightness;

  if(isNightTime())
    {
      nixieBrightness = AppPreferences.getNixieBrightness() * AppPreferences.getNightBrightnessPercent() / 100;
      ledBrightness = AppPreferences.getLedBrightness() * AppPreferences.getNightBrightnessPercent() / 100;
    }
    else
    {
      nixieBrightness = AppPreferences.getNixieBrightness();
      ledBrightness = AppPreferences.getLedBrightness();
    }
    Nixie.setBrightness(nixieBrightness);
    Led.setBrightness(ledBrightness);
}

void updateNixieTask(void *params)
{
  uint8_t hour;
  uint16_t tempCenti;
  for (;;)
  {
    setBrightness();
    if (checkTransition())
    {
      switch (AppPreferences.getDisplayMode())
      {
      case TIME_DATE_DISP_MODE:
        doTransition(now.Day() / 10, now.Day() % 10, now.Month() / 10, now.Month() % 10, now.Year() % 100 / 10, now.Year() % 10);
        vTaskDelay(2000);

        hour = getHour();
        doTransition(hour / 10, hour % 10, now.Minute() / 10, now.Minute() % 10, now.Second() / 10, now.Second() % 10);

        break;

      case TIME_TEMP_DISP_MODE:
        tempCenti = getTempCenti();
        doTransition(EMPTY_DIGIT, EMPTY_DIGIT, tempCenti / 1000, tempCenti % 1000 / 100, tempCenti % 100 / 10, EMPTY_DIGIT);
        vTaskDelay(2000);

        hour = getHour();
        doTransition(hour / 10, hour % 10, now.Minute() / 10, now.Minute() % 10, now.Second() / 10, now.Second() % 10);

        break;

      case TIME_DATE_TEMP_DISP_MODE:
        doTransition(now.Day() / 10, now.Day() % 10, now.Month() / 10, now.Month() % 10, now.Year() % 100 / 10, now.Year() % 10);
        vTaskDelay(2000);

        tempCenti = getTempCenti();
        doTransition(EMPTY_DIGIT, EMPTY_DIGIT, tempCenti / 1000, tempCenti % 1000 / 100, tempCenti % 100 / 10, EMPTY_DIGIT);
        vTaskDelay(2000);

        hour = getHour();
        doTransition(hour / 10, hour % 10, now.Minute() / 10, now.Minute() % 10, now.Second() / 10, now.Second() % 10);

        break;

      default:
        doTransition(hour / 10, hour % 10, now.Minute() / 10, now.Minute() % 10, now.Second() / 10, now.Second() % 10);
        break;
      }
    }
    else
    {
      hour = getHour();
      changeDigits(hour / 10, hour % 10, now.Minute() / 10, now.Minute() % 10, now.Second() / 10, now.Second() % 10);
    }
  }
}

void toggleDotsTask(void *params)
{
  boolean isOn = 0;
  uint8_t brightness;
  for (;;)
  {
    brightness = isNightTime() ? AppPreferences.getDotBrightness() * AppPreferences.getNightBrightnessPercent() / 100 : AppPreferences.getDotBrightness();
    switch (AppPreferences.getDotMode())
    {
    case DOT_OFF_MODE:
      ledcWrite(PWM_DOT_CHANNEL, 0);
      break;

    case DOT_ON_MODE:
      ledcWrite(PWM_DOT_CHANNEL, brightness);
      break;

    default:
      if (isOn)
      {
        ledcWrite(PWM_DOT_CHANNEL, 0);
        isOn = false;
      }
      else
      {
        ledcWrite(PWM_DOT_CHANNEL, brightness);
        isOn = true;
      }
      break;
    }
    vTaskDelay(500 / portTICK_PERIOD_MS);
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

  Nixie.begin();
  xTaskCreate(updateNixieTask, "update nixie", 1024, NULL, 1, NULL);

  ledcSetup(PWM_DOT_CHANNEL, PWM_DOT_FREQ, PWM_DOT_RES);
  ledcAttachPin(DOT_1_PIN, PWM_DOT_CHANNEL);
  ledcAttachPin(DOT_2_PIN, PWM_DOT_CHANNEL);

  xTaskCreate(toggleDotsTask, "toggle dots", 1024, NULL, 1, NULL);

  Led.setBrightness(AppPreferences.getLedBrightness());
  Led.setColor(AppPreferences.getLedColor());
  Led.setMode(AppPreferences.getLedMode());
  Led.begin();

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
