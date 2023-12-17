#ifndef _CLOCK_H_
#define _CLOCK_H_

#include <Arduino.h>

#include <Wire.h>
#include <RtcDS3231.h>

class ClockClass
{
private:
    RtcDS3231<TwoWire> Rtc;
    RtcDateTime now;
    RtcTemperature temperature;
    
    bool celsiusTemp;
    bool h24Format;
    bool readTemp;
    bool syncTime;
    
    String timeZone;

    static void updateTimeTask(void *params);
    
    static void syncTimeTask(void *params);

public:
    ClockClass(TwoWire &wire) : Rtc(wire)
    {
    }
    
    void begin(void);

    uint8_t getYear(void);

    uint8_t getMonth(void);

    uint8_t getDay(void);

    uint8_t getHour(void);

    uint8_t getMinute(void);

    uint8_t getSecond(void);

    uint16_t getTempCenti();

    bool isNightTime(uint16_t nightFromInMinutes, uint16_t nightToInMinutes);

    void setCelsiusTemp(bool val);

    void setDateTime(uint8_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second);

    void setH24Format(bool val);

    void setReadTemp(bool val);

    void setSyncTime(bool val);

    void setTimeZone(String val);
};

extern ClockClass Clock;

#endif