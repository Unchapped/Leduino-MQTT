#ifndef SERIALCLIENT_H
#define SERIALCLIENT_H

#include <Arduino.h>
#include "LedController.h"

//This controller client is basically useless on the ESP8266, so no Ticker version exists.
#include <FancyDelay.h>

//How often to dump channel settings to the serial port
#define SERIAL_STATUS_MILLIS 500

#define SERIAL_STATE_CHANNEL 0
#define SERIAL_STATE_VALUE 1
//#define SERIAL_STATE_DELAY 2 Just use 1 second delay for now
#define SERIAL_STATE_ERR 3

class SerialClientClass{
    HardwareSerial & _port;
    FancyDelay _status_rate; //1 Hz report rate
    uint8_t _serial_state = SERIAL_STATE_CHANNEL;
    uint8_t _channel = 0;

    public:
    SerialClientClass(HardwareSerial & port);
    void init(unsigned int speed = 9600);
    void poll(); //use this in the default arduino loop();
};

extern SerialClientClass SerialClient;

#endif /* SerialCLIENT_H */