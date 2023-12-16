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

#define NIGHT_BRIGHTNESS_PERCENT "night_bright"
#define NIGHT_BRIGHTNESS_PERCENT_DEFAULT 50
#define NIGHT_FROM "night_from"
#define NIGHT_FROM_DEFAULT (23*60)
#define NIGHT_TO "night_to"
#define NIGHT_TO_DEFAULT (7*60)

#define SYNC_TIME "sync_time"
#define TIME_ZONE "time_zone"

#define DISPLAY_MODE_FREQ "disp_mode_freq"

#define EVERY_30_SEC_DISP_MODE 0
#define EVERY_MIN_DISP_MODE 1  
#define EVERY_2_MIN_DISP_MODE 2
#define EVERY_5_MIN_DISP_MODE 3

#define DISPLAY_MODE "display_mode"

#define TIME_DISP_MODE 0
#define TIME_DATE_DISP_MODE 1
#define TIME_TEMP_DISP_MODE 2
#define TIME_DATE_TEMP_DISP_MODE 3

#define DIGIT_EFFECT "digit_effect"

#define NO_DIGIT_EFFECT 0
#define FLIP_DIGIT_EFFECT 1
#define FLIP_SEQ_DIGIT_EFFECT 2
#define FADE_DIGIT_EFFECT 3

#define TRANSITION_EFFECT "trans_effect"

#define NO_TRANSITION_EFFECT 0
#define SHIFT_LEFT_TRANSITION_EFFECT 1
#define SHIFT_RIGHT_TRANSITION_EFFECT 2
#define FLIP_TRANSITION_EFFECT 3
#define FLIP_SEQ_TRANSITION_EFFECT 4
#define FADE_TRANSITION_EFFECT 5

#define H24_FORMAT "24h_format"
#define CELSIUS_TEMP "celsius_temp"

#define DOT_BRIGHTNESS "dot_brightness"
#define DOT_BRIGHTNESS_DEFAULT 128
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
    uint8_t nightBrightnessPercent;
    
    uint16_t nightFromInMinutes;
    uint16_t nightToInMinutes;

    uint8_t syncTime;
    String timeZone;

    uint8_t displayModeFreq;

    uint8_t displayMode;

    uint8_t digitEffect;

    uint8_t transitionEffect;

    uint8_t h24Format;
    uint8_t celsiusTemp;

    uint8_t dotBrightness;
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

    uint8_t getNightBrightnessPercent(void);

    void setNightBrightnessPercent(uint8_t val);

    uint16_t getNightFromInMinutes(void);

    void setNightFromInMinutes(uint16_t val);

    uint16_t getNightToInMinutes(void);

    void setNightToInMinutes(uint16_t val);

    uint8_t getSyncTime(void);

    void setSyncTime(uint8_t val);

    String getTimeZone(void);

    void setTimeZone(String val);

    uint8_t getDisplayModeFreq(void);

    void setDisplayModeFreq(uint8_t val);

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

    uint8_t getDotBrightness(void);

    void setDotBrightness(uint8_t val);

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