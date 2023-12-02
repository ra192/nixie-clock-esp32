#ifndef APP_PREFERENCES_H
#define APP_PREFERENCES_H

#include <Arduino.h>

#define SSID_PARAM "ssid"
#define PASSWORD "password"
#define HOSTNAME "hostname"

#define NIXIE_BRIGHTNESS "nixie_bright"

#define SYNC_TIME "sync_time"
#define TIME_ZONE "time_zone"

#define DISPLAY_MODE "display_mode"

#define TRANSITION_EFFECT "trans_effect"

#define H24_FORMAT "24h_format"
#define CELSIUS_TEMP "celsius_temp"

#define DOT_MODE "dot_mode"

#define LED_BRIGHTNESS "led_bright"
#define LED_COLOR "led_color"
#define LED_MODE "led_mode"

String ssid;
String hostname;

uint8_t nixieBrightness;

int isSyncTime;
String timeZone;

uint8_t displayMode;

uint8_t transitionEffect;

uint8_t h24Format;
uint8_t celsiusTemp;

uint8_t dotMode;

uint8_t ledBrightness;
uint8_t ledMode;

#endif