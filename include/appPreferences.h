#ifndef APP_PREFERENCES_H
#define APP_PREFERENCES_H

#include <Arduino.h>
#include "Preferences.h"

#define PREFERENCES_NAME "myPrefs"

#define SSID_PARAM "ssid"
#define PASSWORD "password"
#define HOSTNAME "hostname"
#define HOSTNAME_DEFAULT "nixie"

#define NIXIE_BRIGHTNESS "nixie_bright"
#define NIXIE_BRIGHTNESS_DEFAULT 255

#define SYNC_TIME "sync_time"
#define TIME_ZONE "time_zone"

#define DISPLAY_MODE "display_mode"

#define DIGIT_EFFECT "digit_effect"

#define TRANSITION_EFFECT "trans_effect"
#define TRANSITION_EFFECT_DEFAULT 4

#define H24_FORMAT "24h_format"
#define CELSIUS_TEMP "celsius_temp"

#define DOT_MODE "dot_mode"
#define DOT_MODE_DEFAULT 2

#define LED_BRIGHTNESS "led_bright"
#define LED_BRIGHTNESS_DEFAULT 64
#define LED_COLOR "led_color"
#define LED_COLOR_DEFAULT 0xff0000
#define LED_MODE "led_mode"
#define LED_MODE_DEFAULT 1

class AppPreferencesClass
{
private:
    Preferences preferences;

    String ssid;
    String password;
    String hostname;

    uint8_t nixieBrightness;

    uint8_t syncTime;
    String timeZone;

    uint8_t displayMode;

    uint8_t digitEffect;

    uint8_t transitionEffect;

    uint8_t h24Format;
    uint8_t celsiusTemp;

    uint8_t dotMode;

    uint8_t ledBrightness;
    uint32_t ledColor;
    uint8_t ledMode;

public:
    void begin(void);

    String getSSID(void);

    void setSSID(String val);

    String getPassword(void);

    void setPassword(String val);

    String getHostname(void);

    void setHostname(String val);

    uint8_t getNixieBrightness(void);

    void setNixieBrightness(uint8_t val);

    uint8_t getSyncTime(void);

    void setSyncTime(uint8_t val);

    String getTimeZone(void);

    void setTimeZone(String val);

    uint8_t getDisplayMode(void);

    void setDisplayMode(uint8_t val);

    uint8_t getDigitEffect(void);

    void setDigitEffect(uint8_t val);

    uint8_t getTransitionEffect(void);

    void setTransitionEffect(uint8_t val);

    uint8_t getH24Format(void);

    void setH24Format(uint8_t val);

    uint8_t getCelsiusTemp(void);

    void setCelsiusTemp(uint8_t val);

    uint8_t getDotMode(void);

    void setDotMode(uint8_t val);

    uint8_t getLedBrightness(void);

    void setLedBrightness(uint8_t val);

    uint32_t getLedColor(void);

    void setLedColor(uint32_t val);

    uint8_t getLedMode(void);

    void setLedMode(uint8_t val);
};

extern AppPreferencesClass AppPreferences;

#endif