#ifndef _DOT_H_
#define _DOT_H_

#include <Arduino.h>

#define DOT_1_PIN 2
#define DOT_2_PIN 23

#define PWM_DOT_CHANNEL 7
#define PWM_DOT_FREQ 10000
#define PWM_DOT_RES 8

#define DOT_OFF_MODE 0
#define DOT_ON_MODE 1
#define DOT_BLINK_MODE 2

class DotClass
{
private:
    uint8_t brightness;
    u_int8_t mode;
    static void refreshTask(void *params);

public:
    void begin();
    void setBrightness(uint8_t val);
    void setMode(uint8_t mode);
};

extern DotClass Dot;

#endif