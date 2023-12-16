#include <led.h>

void LedClass::begin(void)
{
    FastLED.addLeds<NEOPIXEL, LED_PIN>(leds, LED_COUNT);
    xTaskCreate(refreshTask, "update leds", 1024, this, 4, NULL);
}

void LedClass::refreshTask(void *params)
{
    LedClass *led = (LedClass *)params;

    uint8_t hue;
    uint8_t fadeBrightness;
    int fadeAdd;
    for (;;)
    {
        switch (led->mode)
        {
        case LED_MODE_STATIC:
            FastLED.showColor(led->color);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            break;

        case LED_MODE_RAINBOW:
            hue = (hue + 1) % 256;
            FastLED.showColor(CHSV(hue, 255, 255));
            vTaskDelay(20 / portTICK_PERIOD_MS);
            break;

        case LED_MODE_RAINBOW_CHASE:
            for (int i = 0; i < LED_COUNT; i++)
            {
                led->leds[i] = CHSV(hue + i * 12, 255, 255);
            }
            FastLED.show();
            hue = (hue + 1) % 256;
            vTaskDelay(100 / portTICK_PERIOD_MS);
            break;

        case LED_MODE_FADE:
            if (fadeBrightness == 0)
            {
                fadeAdd = 1;
            }
            else if (fadeBrightness >= led->brightness)
            {
                fadeAdd = -1;
            }
            fadeBrightness += fadeAdd;
            FastLED.setBrightness(fadeBrightness);
            FastLED.showColor(led->color);
            vTaskDelay(30 / portTICK_PERIOD_MS);
            break;

        default:
            FastLED.showColor(0);
            vTaskDelay(2000 / portTICK_PERIOD_MS);
            break;
        }
    }
}

void LedClass::setBrightness(uint8_t val)
{
    brightness = val;
    FastLED.setBrightness(val);
}

void LedClass::setColor(uint32_t val)
{
    color = CRGB(val);
}

void LedClass::setMode(uint8_t val)
{
    mode = val;
}

LedClass Led;