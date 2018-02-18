#ifndef SERIALCLIENT_H
#define SERIALCLIENT_H

#include <Arduino.h>
#include "LedController.h"

//This controller client is basically useless on the ESP8266, so no Ticker version exists.
#include <FancyDelay.h>

//How often to dump channel settings to the serial port
#define SERIAL_STATUS_MILLIS 500

class SerialClientClass{
    HardwareSerial & _port;
    FancyDelay _status_rate; //1 Hz report rate

    public:
    SerialClientClass(HardwareSerial & port);
    void init(unsigned int speed = 9600);
    void poll(); //use this in the default arduino loop();
};

extern SerialClientClass SerialClient;

#endif /* SerialCLIENT_H */