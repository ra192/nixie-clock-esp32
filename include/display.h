#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include <Arduino.h>

#define DISPLAY_DATE_TEMP_DELAY_MS 2000
#define DISPLAY_REFRESH_TASK_PRIORITY 2

class DisplayClass
{
private:
    void changeDigits(uint8_t dig1, uint8_t dig2, uint8_t dig3, uint8_t dig4, uint8_t dig5, uint8_t dig6);

    bool checkTransition();

    void doTransition(uint8_t dig1, uint8_t dig2, uint8_t dig3, uint8_t dig4, uint8_t dig5, uint8_t dig6);

    void setBrightness(void);

    static void refreshTask(void *params);

public:
    void begin(void);
};

extern DisplayClass Display;

#endif