#ifndef _NIXIE_H_
#define _NIXIE_H_

#include <Arduino.h>

#define DEC_A0_PIN 26
#define DEC_A1_PIN 33
#define DEC_A2_PIN 32
#define DEC_A3_PIN 25

#define L1_PIN 4
#define L2_PIN 16
#define L3_PIN 17
#define L4_PIN 5
#define L5_PIN 18
#define L6_PIN 19

#define REFRESH_PERIOD_IN_US 3000
#define ON_PERIOD_BRIGHTNES_RATIO 29

#define DIGITS_SIZE 6

const uint8_t LPins[] = {L1_PIN, L2_PIN, L3_PIN, L4_PIN, L5_PIN, L6_PIN};

class Nixie
{
private:
    uint8_t digitValues[DIGITS_SIZE];
    
    uint8_t brightness;
    
    uint16_t onPeriod;
    uint16_t offPeriod;

    void onDigit(uint8_t num);

    void offDigit(uint8_t num);

public:
    Nixie();

    void begin();

    void refresh();

    void setBrightness(uint8_t brightness);

    void setDigits(uint8_t dig1, uint8_t dig2, uint8_t dig3, uint8_t dig4, uint8_t dig5, uint8_t dig6);   
};

#endif