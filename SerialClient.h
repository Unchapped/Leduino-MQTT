#ifndef SERIALCLIENT_H
#define SERIALCLIENT_H

#include "LedController.h"

#ifdef ESP8266
  #include <Ticker.h>
#else
  #include <FancyDelay.h>
#endif

//How often to dump channel settings to the serial port
#define SERIAL_STATUS_MILLIS 1000

class SerialClientClass{
    const Stream * _port;

    #ifdef TICKER_H
    Ticker _status_ticker;
    #else
    FancyDelay _status_rate; //1 Hz report rate
    #endif

    public:
    SerialClientClass();
    void init(const Stream * port);
    void poll(); //use this in the default arduino loop();
    void report_status(); //publish our state
};

extern SerialClientClass SerialClient;

#endif /* SerialCLIENT_H */