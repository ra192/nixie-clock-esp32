#include <Arduino.h>

#include<WiFi.h>
#include <ESPmDNS.h>

#include <appPreferences.h>
#include <appWebserver.h>
#include <clock.h>
#include <nixie.h>
#include <dot.h>
#include <led.h>

int wifiNetworksCount;

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
    return Clock.getSecond() % 30 == 0;
    break;

  case EVERY_MIN_DISP_MODE:
    return Clock.getSecond() == 30;
    break;

  case EVERY_2_MIN_DISP_MODE:
    return Clock.getMinute() % 2 == 0 && Clock.getSecond() == 30;
    break;

  default:
    return Clock.getMinute() % 5 == 0 && Clock.getSecond() == 30;
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

void setBrightness(void)
{
  uint8_t nixieBrightness;
  uint8_t dotBrightness;
  uint8_t ledBrightness;

  if (Clock.isNightTime(AppPreferences.getNightFromInMinutes(), AppPreferences.getNightToInMinutes()))
  {
    nixieBrightness = AppPreferences.getNixieBrightness() * AppPreferences.getNightBrightnessPercent() / 100;
    dotBrightness = AppPreferences.getDotBrightness() * AppPreferences.getNightBrightnessPercent() / 100;
    ledBrightness = AppPreferences.getLedBrightness() * AppPreferences.getNightBrightnessPercent() / 100;
  }
  else
  {
    nixieBrightness = AppPreferences.getNixieBrightness();
    dotBrightness = AppPreferences.getDotBrightness();
    ledBrightness = AppPreferences.getLedBrightness();
  }
  Nixie.setBrightness(nixieBrightness);
  Dot.setBrightness(dotBrightness);
  Led.setBrightness(ledBrightness);
}

void updateNixieTask(void *params)
{
  uint8_t hour;
  uint8_t minute;
  uint8_t second;
  uint16_t tempCenti;
  for (;;)
  {
    setBrightness();
    if (checkTransition())
    {
      switch (AppPreferences.getDisplayMode())
      {
      case TIME_DATE_DISP_MODE:
        doTransition(Clock.getDay() / 10, Clock.getDay() % 10, Clock.getMonth() / 10, Clock.getMonth() % 10, Clock.getYear() % 100 / 10, Clock.getYear() % 10);
        vTaskDelay(2000);

        hour = Clock.getHour();
        minute = Clock.getMinute();
        second = Clock.getSecond();

        doTransition(hour / 10, hour % 10, minute / 10, minute % 10, second / 10, second % 10);

        break;

      case TIME_TEMP_DISP_MODE:
        tempCenti = Clock.getTempCenti();
        doTransition(EMPTY_DIGIT, EMPTY_DIGIT, tempCenti / 1000, tempCenti % 1000 / 100, tempCenti % 100 / 10, EMPTY_DIGIT);
        vTaskDelay(2000);

        hour = Clock.getHour();
        minute = Clock.getMinute();
        second = Clock.getSecond();
        doTransition(hour / 10, hour % 10, minute / 10, minute % 10, second / 10, second % 10);

        break;

      case TIME_DATE_TEMP_DISP_MODE:
        doTransition(Clock.getDay() / 10, Clock.getDay() % 10, Clock.getMonth() / 10, Clock.getMonth() % 10, Clock.getYear() % 100 / 10, Clock.getYear() % 10);
        vTaskDelay(2000);

        tempCenti = Clock.getTempCenti();
        doTransition(EMPTY_DIGIT, EMPTY_DIGIT, tempCenti / 1000, tempCenti % 1000 / 100, tempCenti % 100 / 10, EMPTY_DIGIT);
        vTaskDelay(2000);

        hour = Clock.getHour();
        minute = Clock.getMinute();
        second = Clock.getSecond();
        doTransition(hour / 10, hour % 10, minute / 10, minute % 10, second / 10, second % 10);

        break;

      default:
        hour = Clock.getHour();
        minute = Clock.getMinute();
        second = Clock.getSecond();

        doTransition(hour / 10, hour % 10, minute / 10, minute % 10, second / 10, second % 10);
        break;
      }
    }
    else
    {
      hour = Clock.getHour();
      minute = Clock.getMinute();
      second = Clock.getSecond();
      
      changeDigits(hour / 10, hour % 10, minute / 10, minute % 10, second / 10, second % 10);
    }
  }
}

void setup()
{
  Serial.begin(115200);

  AppPreferences.begin();

  Clock.setCelsiusTemp(AppPreferences.getCelsiusTemp());
  Clock.setH24Format(AppPreferences.getH24Format());

  bool readTemp = AppPreferences.getDisplayMode() == TIME_DATE_TEMP_DISP_MODE || AppPreferences.getDisplayMode() == TIME_TEMP_DISP_MODE;
  Clock.setReadTemp(readTemp);

  Clock.setSyncTime(AppPreferences.getSyncTime());
  Clock.setTimeZone(AppPreferences.getTimeZone());

  Clock.begin();

  Nixie.begin();
  xTaskCreate(updateNixieTask, "update nixie", 1024, NULL, 2, NULL);

  Dot.setMode(AppPreferences.getDotMode());
  Dot.begin();

  Led.setColor(AppPreferences.getLedColor());
  Led.setMode(AppPreferences.getLedMode());
  Led.begin();

  setupWifi();
  AppWebserver.start();

  AppPreferences.setWiFiNetworksCount(WiFi.scanNetworks());
}

void loop()
{
}
