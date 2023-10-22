#include <nixie.h>
#include <Arduino.h>

uint8_t digit_codes[] = {1,0,9,8,7,6,5,4,3,2};

void Nixie::on_digit(uint8_t num)
{
    digitalWrite(DEC_A0_PIN, digit_values[num] & 0x01);
    digitalWrite(DEC_A1_PIN, digit_values[num] >> 1 & 0x01);
    digitalWrite(DEC_A2_PIN, digit_values[num] >> 2 & 0x01);
    digitalWrite(DEC_A3_PIN, digit_values[num] >> 3 & 0x01);

    uint8_t l_pin = 1 << num;
    digitalWrite(L1_PIN, l_pin & 0x01);
    digitalWrite(L2_PIN, l_pin >> 1 & 0x01);
    digitalWrite(L3_PIN, l_pin >> 2 & 0x01);
    digitalWrite(L4_PIN, l_pin >> 3 & 0x01);
    digitalWrite(L5_PIN, l_pin >> 4 & 0x01);
    digitalWrite(L6_PIN, l_pin >> 5 & 0x01);
}

void Nixie::off_digits()
{
    digitalWrite(L1_PIN, 0);
    digitalWrite(L2_PIN, 0);
    digitalWrite(L3_PIN, 0);
    digitalWrite(L4_PIN, 0);
    digitalWrite(L5_PIN, 0);
    digitalWrite(L6_PIN, 0);
}

Nixie::Nixie()
{
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

void Nixie::set_digits(uint8_t dig1, uint8_t dig2, uint8_t dig3, uint8_t dig4, uint8_t dig5, uint8_t dig6)
{
    digit_values[0] = digit_codes[dig1];
    digit_values[1] = digit_codes[dig2];
    digit_values[2] = digit_codes[dig3];
    digit_values[3] = digit_codes[dig4];
    digit_values[4] = digit_codes[dig5];
    digit_values[5] = digit_codes[dig6];
}

void Nixie::refresh(void)
{
    if (is_on)
    {
        off_digits();
        is_on = 0;
    }
    else
    {
        on_digit(current);
        current = (current + 1) % DIGITS_SIZE;
    }
}