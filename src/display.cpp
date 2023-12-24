#include <display.h>

#include <appPreferences.h>
#include <nixie.h>
#include <clock.h>
#include <dot.h>
#include <led.h>

void DisplayClass::changeDigits(uint8_t dig1, uint8_t dig2, uint8_t dig3, uint8_t dig4, uint8_t dig5, uint8_t dig6)
{
    switch (AppPreferences.getDigitEffect())
    {
    case FLIP_DIGIT_EFFECT:
        Nixie.flip(dig1, dig2, dig3, dig4, dig5, dig6, false);
        break;
    case FLIP_SEQ_DIGIT_EFFECT:
        Nixie.flipSeq(dig1, dig2, dig3, dig4, dig5, dig6, false);
        break;
    case FADE_DIGIT_EFFECT:
        Nixie.fade(dig1, dig2, dig3, dig4, dig5, dig6, false);
        break;
    default:
        Nixie.setDigits(dig1, dig2, dig3, dig4, dig5, dig6);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

bool DisplayClass::checkTransition()
{
    switch (AppPreferences.getDisplayModeFreq())
    {
    case EVERY_30_SEC_DISP_MODE:
        return Clock.getSecond() % 30 == 0;
        break;

    case EVERY_MIN_DISP_MODE:
        return Clock.getSecond() == 30;
        break;

    case EVERY_2_MIN_DISP_MODE:
        return Clock.getMinute() % 2 == 0 && Clock.getSecond() == 30;
        break;

    default:
        return Clock.getMinute() % 5 == 0 && Clock.getSecond() == 30;
        break;
    }
}

void DisplayClass::doTransition(uint8_t dig1, uint8_t dig2, uint8_t dig3, uint8_t dig4, uint8_t dig5, uint8_t dig6)
{
    switch (AppPreferences.getTransitionEffect())
    {
    case SHIFT_LEFT_TRANSITION_EFFECT:
        Nixie.shiftLeft(dig1, dig2, dig3, dig4, dig5, dig6);
        break;
    case SHIFT_RIGHT_TRANSITION_EFFECT:
        Nixie.shiftRight(dig1, dig2, dig3, dig4, dig5, dig6);
        break;
    case FLIP_TRANSITION_EFFECT:
        Nixie.flip(dig1, dig2, dig3, dig4, dig5, dig6);
        break;
    case FLIP_SEQ_TRANSITION_EFFECT:
        Nixie.flipSeq(dig1, dig2, dig3, dig4, dig5, dig6);
        break;
    case FADE_TRANSITION_EFFECT:
        Nixie.fade(dig1, dig2, dig3, dig4, dig5, dig6);
        break;
    default:
        Nixie.setDigits(dig1, dig2, dig3, dig4, dig5, dig6);
        break;
    }
}

void DisplayClass::setBrightness(void)
{
    uint8_t nixieBrightness;
    uint8_t dotBrightness;
    uint8_t ledBrightness;

    if (Clock.isNightTime(AppPreferences.getNightFromInMinutes(), AppPreferences.getNightToInMinutes()))
    {
        nixieBrightness = AppPreferences.getNixieBrightness() * AppPreferences.getNightBrightnessPercent() / 100;
        dotBrightness = AppPreferences.getDotBrightness() * AppPreferences.getNightBrightnessPercent() / 100;
        ledBrightness = AppPreferences.getLedBrightness() * AppPreferences.getNightBrightnessPercent() / 100;
    }
    else
    {
        nixieBrightness = AppPreferences.getNixieBrightness();
        dotBrightness = AppPreferences.getDotBrightness();
        ledBrightness = AppPreferences.getLedBrightness();
    }
    Nixie.setBrightness(nixieBrightness);
    Dot.setBrightness(dotBrightness);
    Led.setBrightness(ledBrightness);
}

void DisplayClass::refreshTask(void *params)
{
    DisplayClass *display = (DisplayClass *)display;

    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint16_t tempCenti;

    for (;;)
    {
        display->setBrightness();
        if (display->checkTransition())
        {
            switch (AppPreferences.getDisplayMode())
            {
            case TIME_DATE_DISP_MODE:
                display->doTransition(Clock.getDay() / 10, Clock.getDay() % 10, Clock.getMonth() / 10, Clock.getMonth() % 10, Clock.getYear() % 100 / 10, Clock.getYear() % 10);
                vTaskDelay(DISPLAY_DATE_TEMP_DELAY_MS / portTICK_PERIOD_MS);

                hour = Clock.getHour();
                minute = Clock.getMinute();
                second = Clock.getSecond();

                display->doTransition(hour / 10, hour % 10, minute / 10, minute % 10, second / 10, second % 10);

                break;

            case TIME_TEMP_DISP_MODE:
                tempCenti = Clock.getTempCenti();
                display->doTransition(EMPTY_DIGIT, EMPTY_DIGIT, tempCenti / 1000, tempCenti % 1000 / 100, tempCenti % 100 / 10, EMPTY_DIGIT);
                vTaskDelay(DISPLAY_DATE_TEMP_DELAY_MS / portTICK_PERIOD_MS);

                hour = Clock.getHour();
                minute = Clock.getMinute();
                second = Clock.getSecond();
                display->doTransition(hour / 10, hour % 10, minute / 10, minute % 10, second / 10, second % 10);

                break;

            case TIME_DATE_TEMP_DISP_MODE:
                display->doTransition(Clock.getDay() / 10, Clock.getDay() % 10, Clock.getMonth() / 10, Clock.getMonth() % 10, Clock.getYear() % 100 / 10, Clock.getYear() % 10);
                vTaskDelay(DISPLAY_DATE_TEMP_DELAY_MS / portTICK_PERIOD_MS);

                tempCenti = Clock.getTempCenti();
                display->doTransition(EMPTY_DIGIT, EMPTY_DIGIT, tempCenti / 1000, tempCenti % 1000 / 100, tempCenti % 100 / 10, EMPTY_DIGIT);
                vTaskDelay(DISPLAY_DATE_TEMP_DELAY_MS / portTICK_PERIOD_MS);

                hour = Clock.getHour();
                minute = Clock.getMinute();
                second = Clock.getSecond();
                display->doTransition(hour / 10, hour % 10, minute / 10, minute % 10, second / 10, second % 10);

                break;

            default:
                hour = Clock.getHour();
                minute = Clock.getMinute();
                second = Clock.getSecond();

                display->doTransition(hour / 10, hour % 10, minute / 10, minute % 10, second / 10, second % 10);
                break;
            }
        }
        else
        {
            hour = Clock.getHour();
            minute = Clock.getMinute();
            second = Clock.getSecond();

            display->changeDigits(hour / 10, hour % 10, minute / 10, minute % 10, second / 10, second % 10);
        }
    }
}

void DisplayClass::begin(void)
{
    xTaskCreate(refreshTask, "refresh display", 1024, this, DISPLAY_REFRESH_TASK_PRIORITY, NULL);
}

DisplayClass Display;