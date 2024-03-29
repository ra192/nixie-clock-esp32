#include <clock.h>

#include "time.h"
#include <WiFi.h>

void ClockClass::updateTimeTask(void *params)
{
    ClockClass *clock = (ClockClass *)params;

    TickType_t xLastWakeTime = xTaskGetTickCount();

    for (;;)
    {
        clock->now = clock->Rtc.GetDateTime();
        if (clock->readTemp)
            clock->temperature = clock->Rtc.GetTemperature();
        vTaskDelayUntil(&xLastWakeTime, UPDATE_TIME_DELAY_MS / portTICK_PERIOD_MS);
    }
}

void ClockClass::syncTimeTask(void *params)
{
    ClockClass *clock = (ClockClass *)params;

    TickType_t xLastWakeTime = xTaskGetTickCount();

    for (;;)
    {
        if (WiFi.isConnected() && clock->syncTime)
        {
            struct tm timeinfo;

            if (getLocalTime(&timeinfo))
            {
                RtcDateTime updatedTime = RtcDateTime(timeinfo.tm_year % 100, timeinfo.tm_mon + 1, timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
                clock->Rtc.SetDateTime(updatedTime);
            }
        }

        vTaskDelayUntil(&xLastWakeTime, SYNC_TIME_DELAY_MS / portTICK_PERIOD_MS);
    }
}

void ClockClass::begin()
{
    Rtc.Begin();
    xTaskCreate(updateTimeTask, "update time", 2048, this, UPDATE_TIME_TASK_PRIORITY, NULL);

    configTzTime(timeZone.c_str(), "pool.ntp.org");
    xTaskCreate(syncTimeTask, "sync time", 2048, this, SYNC_TIME_TASK_PRIORITY, NULL);
}

uint8_t ClockClass::getYear(void)
{
    return now.Year();
}

uint8_t ClockClass::getMonth(void)
{
    return now.Month();
}

uint8_t ClockClass::getDay(void)
{
    return now.Day();
}

uint8_t ClockClass::getHour(void)
{
    if (!h24Format)
    {
        if (now.Hour() == 0)
        {
            return 12;
        }
        if (now.Hour() > 12)
        {
            return now.Hour() - 12;
        }
    }
    return now.Hour();
}

uint8_t ClockClass::getMinute(void)
{
    return now.Minute();
}

uint8_t ClockClass::getSecond(void)
{
    return now.Second();
}

uint16_t ClockClass::getTempCenti()
{
    if (celsiusTemp)
    {
        return temperature.AsCentiDegC() - tempCorrection * 100;
    }
    else
    {
        return (temperature.AsFloatDegF() - tempCorrection) * 100;
    }
}

bool ClockClass::isNightTime(uint16_t nightFromInMinutes, uint16_t nightToInMinutes)
{
    uint16_t nowInMins = now.Hour() * 60 + now.Minute();

    return (nightFromInMinutes <= nightToInMinutes && nowInMins >= nightFromInMinutes && nowInMins < nightToInMinutes) ||
           (nightFromInMinutes > nightToInMinutes && (nowInMins < nightToInMinutes || nowInMins >= nightFromInMinutes));
}

void ClockClass::setCelsiusTemp(bool val)
{
    celsiusTemp = val;
}

void ClockClass::setDateTime(uint8_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second)
{
    RtcDateTime updatedTime = RtcDateTime(year, month, day, hour, minute, second);
    Rtc.SetDateTime(updatedTime);
}

void ClockClass::setH24Format(bool val)
{
    h24Format = val;
}

void ClockClass::setReadTemp(bool val)
{
    readTemp = val;
}

void ClockClass::setSyncTime(bool val)
{
    syncTime = val;
}

void ClockClass::setTempCorrection(uint8_t val)
{
    tempCorrection = val;
}

void ClockClass::setTimeZone(String val)
{
    timeZone = val;
    configTzTime(timeZone.c_str(), "pool.ntp.org");
}

ClockClass Clock(Wire);