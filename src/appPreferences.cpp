#include <appPreferences.h>
#include <SPIFFS.h>

void AppPreferencesClass::begin(void)
{
    preferences.begin(PREFERENCES_NAME, false);

    // Initialize SPIFFS
    if (!SPIFFS.begin(true))
    {
        Serial.println("An Error has occurred while mounting SPIFFS");
        return;
    }

    ssid = preferences.getString(SSID_PARAM);
    password = preferences.getString(PASSWORD);
    hostname = preferences.getString(HOSTNAME, HOSTNAME_DEFAULT);
    nixieBrightness = preferences.getUInt(NIXIE_BRIGHTNESS, NIXIE_BRIGHTNESS_DEFAULT);
    syncTime = preferences.getUInt(SYNC_TIME);
    timeZone = preferences.getString(TIME_ZONE);
    displayMode = preferences.getUInt(DISPLAY_MODE);
    digitEffect = preferences.getUInt(DIGIT_EFFECT);
    transitionEffect = preferences.getUInt(TRANSITION_EFFECT, TRANSITION_EFFECT_DEFAULT);
    h24Format = preferences.getUInt(H24_FORMAT, 1);
    celsiusTemp = preferences.getUInt(CELSIUS_TEMP, 1);
    dotMode = preferences.getUInt(DOT_MODE, DOT_MODE_DEFAULT);
    ledBrightness = preferences.getUInt(LED_BRIGHTNESS, LED_BRIGHTNESS_DEFAULT);
    ledColor = preferences.getUInt(LED_COLOR, LED_COLOR_DEFAULT);
    ledMode = preferences.getUInt(LED_MODE, LED_MODE_DEFAULT);
}

String AppPreferencesClass::getSSID(void)
{
    return ssid;
}

void AppPreferencesClass::setSSID(String val)
{
    ssid = val;
    preferences.putString(SSID_PARAM, val);
}

String AppPreferencesClass::getPassword(void)
{
    return password;
}

void AppPreferencesClass::setPassword(String val)
{
    password = val;
    preferences.putString(PASSWORD, val);
}

String AppPreferencesClass::getHostname(void)
{
    return hostname;
}

void AppPreferencesClass::setHostname(String val)
{
    hostname = val;
    preferences.putString(HOSTNAME, val);
}

uint8_t AppPreferencesClass::getNixieBrightness(void)
{
    return nixieBrightness;
}

void AppPreferencesClass::setNixieBrightness(uint8_t val)
{
    nixieBrightness = val;
    preferences.putUInt(NIXIE_BRIGHTNESS, val);
}

uint8_t AppPreferencesClass::getSyncTime(void)
{
    return syncTime;
}

void AppPreferencesClass::setSyncTime(uint8_t val)
{
    syncTime = val;
    preferences.putUInt(SYNC_TIME, val);
}

String AppPreferencesClass::getTimeZone(void)
{
    return timeZone;
}

void AppPreferencesClass::setTimeZone(String val)
{
    timeZone = val;
    preferences.putString(TIME_ZONE, val);
}

uint8_t AppPreferencesClass::getDisplayMode(void)
{
    return displayMode;
}

void AppPreferencesClass::setDisplayMode(uint8_t val)
{
    displayMode = val;
    preferences.putUInt(DISPLAY_MODE, val);
}

uint8_t AppPreferencesClass::getDigitEffect(void)
{
    return digitEffect;
}

void AppPreferencesClass::setDigitEffect(uint8_t val)
{
    digitEffect = val;
    preferences.putUInt(DIGIT_EFFECT, val);
}

uint8_t AppPreferencesClass::getTransitionEffect(void)
{
    return transitionEffect;
}

void AppPreferencesClass::setTransitionEffect(uint8_t val)
{
    transitionEffect = val;
    preferences.putUInt(TRANSITION_EFFECT, val);
}

uint8_t AppPreferencesClass::getH24Format(void)
{
    return h24Format;
}

void AppPreferencesClass::setH24Format(uint8_t val)
{
    h24Format = val;
    preferences.putUInt(H24_FORMAT, val);
}

uint8_t AppPreferencesClass::getCelsiusTemp(void)
{
    return celsiusTemp;
}

void AppPreferencesClass::setCelsiusTemp(uint8_t val)
{
    celsiusTemp = val;
    preferences.putUInt(CELSIUS_TEMP, val);
}

uint8_t AppPreferencesClass::getDotMode(void)
{
    return dotMode;
}

void AppPreferencesClass::setDotMode(uint8_t val)
{
    dotMode = val;
    preferences.putUInt(DOT_MODE, val);
}

uint8_t AppPreferencesClass::getLedBrightness(void)
{
    return ledBrightness;
}

void AppPreferencesClass::setLedBrightness(uint8_t val)
{
    ledBrightness = val;
    preferences.putUInt(LED_BRIGHTNESS, val);
}

uint32_t AppPreferencesClass::getLedColor(void)
{
    return ledColor;
}

void AppPreferencesClass::setLedColor(uint32_t val)
{
    ledColor = val;
    preferences.putUInt(LED_COLOR, val);
}

uint8_t AppPreferencesClass::getLedMode(void)
{
    return ledMode;
}

void AppPreferencesClass::setLedMode(uint8_t val)
{
    ledMode = val;
    preferences.putUInt(LED_MODE, val);
}

AppPreferencesClass AppPreferences;