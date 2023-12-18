#ifndef _APP_WEBSERWER_H_
#define _APP_WEBSERWER_H_

#include <Arduino.h>

#include "ESPAsyncWebServer.h"

class AppWebserverClass
{
private:
    AsyncWebServer server;

public:
    AppWebserverClass(uint8_t port) : server(port)
    {
    }

    void start(void);
};

extern AppWebserverClass AppWebserver;

#endif