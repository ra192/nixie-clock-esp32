#include <nixie.h>
#include <Arduino.h>

const uint8_t digitCodes[] = {1, 0, 9, 8, 7, 6, 5, 4, 3, 2, EMPTY_DIGIT};

void Nixie::createShiftArray(uint8_t dig1, uint8_t dig2, uint8_t dig3, uint8_t dig4, uint8_t dig5, uint8_t dig6, uint8_t *resArr)
{
    resArr[0] = digitValues[0];
    resArr[1] = digitValues[1];
    resArr[2] = digitValues[2];
    resArr[3] = digitValues[3];
    resArr[4] = digitValues[4];
    resArr[5] = digitValues[5];
    resArr[6] = EMPTY_DIGIT;
    resArr[7] = digitCodes[dig1];
    resArr[8] = digitCodes[dig2],
    resArr[9] = digitCodes[dig3],
    resArr[10] = digitCodes[dig4],
    resArr[11] = digitCodes[dig5],
    resArr[12] = digitCodes[dig6];
}

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

    xTaskCreate(refreshTask, "refresh nixie", 1024, this, configMAX_PRIORITIES - 1, NULL);
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

void Nixie::setDigVals(uint8_t *digs, uint8_t startInd)
{
    digitValues[0] = digs[startInd];
    digitValues[1] = digs[startInd + 1];
    digitValues[2] = digs[startInd + 2];
    digitValues[3] = digs[startInd + 3];
    digitValues[4] = digs[startInd + 4];
    digitValues[5] = digs[startInd + 5];
}

void Nixie::shiftLeft(uint8_t dig1, uint8_t dig2, uint8_t dig3, uint8_t dig4, uint8_t dig5, uint8_t dig6)
{
    uint8_t shiftDigs[2 * DIGITS_SIZE + 1];
    createShiftArray(dig1, dig2, dig3, dig4, dig5, dig6, shiftDigs);

    for (int i = 1; i <= DIGITS_SIZE + 1; i++)
    {
        setDigVals(shiftDigs, i);
        vTaskDelay(SHIFT_DELAY_MS);
    }
}

void Nixie::shiftRight(uint8_t dig1, uint8_t dig2, uint8_t dig3, uint8_t dig4, uint8_t dig5, uint8_t dig6)
{
    uint8_t shiftDigs[2 * DIGITS_SIZE + 1];
    createShiftArray(dig1, dig2, dig3, dig4, dig5, dig6, shiftDigs);

    for (int i = 6; i >= 0; i--)
    {
        setDigVals(shiftDigs, i);
        vTaskDelay(SHIFT_DELAY_MS);
    }
}

void Nixie::refreshTask(void *params)
{
    Nixie *nixie = (Nixie *)params;
    bool isOn = false;
    uint8_t current = 4;

    TickType_t xLastWakeTime = xTaskGetTickCount();

    for (;;)
    {
        if (isOn)
        {
            nixie->offDigit(current);
            isOn = false;
            current = (current + 1) % DIGITS_SIZE;
            vTaskDelayUntil(&xLastWakeTime, NIXIE_OFF_DELAY);
        }
        else if (nixie->digitValues[current] == EMPTY_DIGIT)
        {
            nixie->offDigit(current);
            current = (current + 1) % DIGITS_SIZE;
            vTaskDelayUntil(&xLastWakeTime, NIXIE_TOTAL_DELAY);
        }
        else
        {
            nixie->onDigit(current);
            isOn = true;
            vTaskDelayUntil(&xLastWakeTime, NIXIE_ON_DELAY);
        }
    }
}