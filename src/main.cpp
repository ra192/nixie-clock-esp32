#include <Arduino.h>

#include<WiFi.h>
#include <ESPmDNS.h>

#include <appPreferences.h>
#include <appWebserver.h>
#include <clock.h>
#include <nixie.h>
#include <dot.h>
#include <led.h>
#include <display.h>

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

void setup()
{
  Serial.begin(115200);

  AppPreferences.begin();

  Clock.setCelsiusTemp(AppPreferences.getCelsiusTemp());
  Clock.setH24Format(AppPreferences.getH24Format());

  bool readTemp = AppPreferences.getDisplayMode() == TIME_DATE_TEMP_DISP_MODE || AppPreferences.getDisplayMode() == TIME_TEMP_DISP_MODE;
  Clock.setReadTemp(readTemp);

  Clock.setSyncTime(AppPreferences.getSyncTime());
  Clock.setTempCorrection(AppPreferences.getTempCorrection());
  Clock.setTimeZone(AppPreferences.getTimeZone());

  Clock.begin();

  Nixie.begin();
  Display.begin();

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
