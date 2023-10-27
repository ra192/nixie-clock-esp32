#include <nixie.h>
#include <Arduino.h>

const uint8_t digitCodes[] = {1, 0, 9, 8, 7, 6, 5, 4, 3, 2};

void Nixie::onDigit(uint8_t num)
{
    digitalWrite(DEC_A0_PIN, digitValues[num] & 0x01);
    digitalWrite(DEC_A1_PIN, digitValues[num] >> 1 & 0x01);
    digitalWrite(DEC_A2_PIN, digitValues[num] >> 2 & 0x01);
    digitalWrite(DEC_A3_PIN, digitValues[num] >> 3 & 0x01);

    digitalWrite(LPins[num], HIGH);
}

void Nixie::offDigit(uint8_t num)
{
    digitalWrite(LPins[num], LOW);
}

Nixie::Nixie()
{
    setBrightness(100);
}

void Nixie::begin()
{
    pinMode(DEC_A0_PIN, OUTPUT);
    pinMode(DEC_A1_PIN, OUTPUT);
    pinMode(DEC_A2_PIN, OUTPUT);
    pinMode(DEC_A3_PIN, OUTPUT);

    pinMode(L1_PIN, OUTPUT);
    pinMode(L2_PIN, OUTPUT);
    pinMode(L3_PIN, OUTPUT);
    pinMode(L4_PIN, OUTPUT);
    pinMode(L5_PIN, OUTPUT);
    pinMode(L6_PIN, OUTPUT);
}

void Nixie::setBrightness(uint8_t brightness)
{
    this->brightness = brightness;
    this->onPeriod = ON_PERIOD_BRIGHTNES_RATIO * brightness;
    this->offPeriod = REFRESH_PERIOD_IN_US - onPeriod;
}

void Nixie::setDigits(uint8_t dig1, uint8_t dig2, uint8_t dig3, uint8_t dig4, uint8_t dig5, uint8_t dig6)
{
    digitValues[0] = digitCodes[dig1];
    digitValues[1] = digitCodes[dig2];
    digitValues[2] = digitCodes[dig3];
    digitValues[3] = digitCodes[dig4];
    digitValues[4] = digitCodes[dig5];
    digitValues[5] = digitCodes[dig6];
}

bool isOn = false;
uint8_t current = 0;
int64_t switchTime_us;

void Nixie::refresh()
{
    int64_t current_time = esp_timer_get_time();
    if (isOn && current_time > switchTime_us + onPeriod)
    {
        offDigit(current);
        isOn = false;
        current = (current + 1) % DIGITS_SIZE;

        switchTime_us = current_time;
    }
    else if (!isOn && current_time > switchTime_us + offPeriod)
    {
        onDigit(current);
        isOn = true;
        switchTime_us = current_time;
    }
}