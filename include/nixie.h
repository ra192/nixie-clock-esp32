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

class Nixie
{
private:
    uint8_t digit_values[DIGITS_SIZE];
    uint8_t current = 0;
    uint8_t is_on = 0;

    void on_digit(uint8_t num);

    void off_digits(void);

public:
    Nixie();

    void begin();

    void set_digits(uint8_t dig1, uint8_t dig2, uint8_t dig3, uint8_t dig4, uint8_t dig5, uint8_t dig6);

    void refresh(void);
};

#endif