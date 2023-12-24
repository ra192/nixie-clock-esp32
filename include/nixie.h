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

#define DIGITS_SIZE 6

#define PWM_L1_CHANNEL 0
#define PWM_L2_CHANNEL 1
#define PWM_L3_CHANNEL 2
#define PWM_L4_CHANNEL 3
#define PWM_L5_CHANNEL 4
#define PWM_L6_CHANNEL 5

#define PWM_FREQ 10000
#define PWM_RES 8

#define NIXIE_ON_DELAY_MS 2
#define NIXIE_OFF_DELAY_MS 1
#define NIXIE_TOTAL_DELAY_MS (NIXIE_ON_DELAY_MS + NIXIE_OFF_DELAY_MS)

#define EMPTY_DIGIT 10

#define FLIP_ALL_DELAY_MS 50
#define FLIP_SEQ_DELAY_MS 10
#define SHIFT_DELAY_MS 100
#define FADE_DELAY_MS 100

#define NIXIE_REFRESH_TASK_PRIORITY (configMAX_PRIORITIES - 1)

const uint8_t LPins[] = {L1_PIN, L2_PIN, L3_PIN, L4_PIN, L5_PIN, L6_PIN};
const uint8_t PwmLChannels[] = {PWM_L1_CHANNEL, PWM_L2_CHANNEL, PWM_L3_CHANNEL, PWM_L4_CHANNEL, PWM_L5_CHANNEL, PWM_L6_CHANNEL};

class NixieClass
{
private:
    uint8_t digitValues[DIGITS_SIZE];

    uint8_t brightnessValues[DIGITS_SIZE];

    void copyShiftArray(uint8_t dig1, uint8_t dig2, uint8_t dig3, uint8_t dig4, uint8_t dig5, uint8_t dig6, uint8_t *resArr);

    void onDigit(uint8_t num);

    void offDigit(uint8_t num);

    void setDigVals(uint8_t *digs, uint8_t startInd);

    static void refreshTask(void *params);

public:
    NixieClass();

    void begin();

    void fade(uint8_t dig1, uint8_t dig2, uint8_t dig3, uint8_t dig4, uint8_t dig5, uint8_t dig6, bool allDigits = true);

    void flip(uint8_t dig1, uint8_t dig2, uint8_t dig3, uint8_t dig4, uint8_t dig5, uint8_t dig6, bool allDigits = true);

    void flipSeq(uint8_t dig1, uint8_t dig2, uint8_t dig3, uint8_t dig4, uint8_t dig5, uint8_t dig6, bool allDigits = true);

    void setBrightness(uint8_t brightness);

    void setDigits(uint8_t dig1, uint8_t dig2, uint8_t dig3, uint8_t dig4, uint8_t dig5, uint8_t dig6);

    void shiftLeft(uint8_t dig1, uint8_t dig2, uint8_t dig3, uint8_t dig4, uint8_t dig5, uint8_t dig6);

    void shiftRight(uint8_t dig1, uint8_t dig2, uint8_t dig3, uint8_t dig4, uint8_t dig5, uint8_t dig6);
};

extern NixieClass Nixie;

#endif