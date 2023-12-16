#ifndef _LED_H_
#define _LED_H_

#include <Arduino.h>
#include <FastLED.h>

#define LED_COUNT 6
#define LED_PIN 27

#define LED_MODE_OFF 0
#define LED_MODE_STATIC 1
#define LED_MODE_RAINBOW 2
#define LED_MODE_RAINBOW_CHASE 3
#define LED_MODE_FADE 4

class LedClass
{
private:
    uint8_t brightness;
    CRGB color;
    CRGB leds[LED_COUNT];
    uint8_t mode;

    static void refreshTask(void* params);

public:
    void begin(void);
    void setBrightness(uint8_t val);
    void setColor(uint32_t val);
    void setMode(uint8_t val);
};

extern LedClass Led;

#endif