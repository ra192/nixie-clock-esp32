#include <nixie.h>
#include <Arduino.h>

const uint8_t digitCodes[] = {1, 0, 9, 8, 7, 6, 5, 4, 3, 2};

void Nixie::onDigit(uint8_t num)
{
    digitalWrite(DEC_A0_PIN, digitValues[num] & 0x01);
    digitalWrite(DEC_A1_PIN, digitValues[num] >> 1 & 0x01);
    digitalWrite(DEC_A2_PIN, digitValues[num] >> 2 & 0x01);
    digitalWrite(DEC_A3_PIN, digitValues[num] >> 3 & 0x01);

    ledcWrite(PwmLChannels[num], brightness);
}

void Nixie::offDigit(uint8_t num)
{
    ledcWrite(PwmLChannels[num], 0);
}

Nixie::Nixie()
{
    setBrightness(255);
}

void Nixie::begin()
{
    pinMode(DEC_A0_PIN, OUTPUT);
    pinMode(DEC_A1_PIN, OUTPUT);
    pinMode(DEC_A2_PIN, OUTPUT);
    pinMode(DEC_A3_PIN, OUTPUT);

    ledcSetup(PWM_L1_CHANNEL, PWM_FREQ, PWM_RES);
    ledcAttachPin(L1_PIN, PWM_L1_CHANNEL);

    ledcSetup(PWM_L2_CHANNEL, PWM_FREQ, PWM_RES);
    ledcAttachPin(L2_PIN, PWM_L2_CHANNEL);

    ledcSetup(PWM_L3_CHANNEL, PWM_FREQ, PWM_RES);
    ledcAttachPin(L3_PIN, PWM_L3_CHANNEL);

    ledcSetup(PWM_L4_CHANNEL, PWM_FREQ, PWM_RES);
    ledcAttachPin(L4_PIN, PWM_L4_CHANNEL);

    ledcSetup(PWM_L5_CHANNEL, PWM_FREQ, PWM_RES);
    ledcAttachPin(L5_PIN, PWM_L5_CHANNEL);

    ledcSetup(PWM_L6_CHANNEL, PWM_FREQ, PWM_RES);
    ledcAttachPin(L6_PIN, PWM_L6_CHANNEL);
}

void Nixie::setBrightness(uint8_t brightness)
{
    this->brightness = brightness;
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
    if (isOn && current_time > switchTime_us + REFRESH_ON_PERIOD_IN_US)
    {
        offDigit(current);
        isOn = false;
        current = (current + 1) % DIGITS_SIZE;

        switchTime_us = current_time;
    }
    else if (!isOn && current_time > switchTime_us + REFRESH_OFF_PERIOD_IN_US)
    {
        onDigit(current);
        isOn = true;
        switchTime_us = current_time;
    }
}