#include <dot.h>

void DotClass::begin()
{
    ledcSetup(PWM_DOT_CHANNEL, PWM_DOT_FREQ, PWM_DOT_RES);
    ledcAttachPin(DOT_1_PIN, PWM_DOT_CHANNEL);
    ledcAttachPin(DOT_2_PIN, PWM_DOT_CHANNEL);

    xTaskCreate(refreshTask, "refresh dots", 1024, this, 1, NULL);
}

void DotClass::refreshTask(void *params)
{
    DotClass *dot = (DotClass *)params;
    boolean isOn = 0;
    for (;;)
    {
        switch (dot->mode)
        {
        case DOT_OFF_MODE:
            ledcWrite(PWM_DOT_CHANNEL, 0);
            break;

        case DOT_ON_MODE:
            ledcWrite(PWM_DOT_CHANNEL, dot->brightness);
            break;

        default:
            if (isOn)
            {
                ledcWrite(PWM_DOT_CHANNEL, 0);
                isOn = false;
            }
            else
            {
                ledcWrite(PWM_DOT_CHANNEL, dot->brightness);
                isOn = true;
            }
            break;
        }
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}

void DotClass::setBrightness(uint8_t val)
{
    brightness = val;
}

void DotClass::setMode(uint8_t val)
{
    mode = val;
}

DotClass Dot;