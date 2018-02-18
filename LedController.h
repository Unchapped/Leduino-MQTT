//TODO: figure out how to reference Adafruit legal for copied code.

#ifndef LedController_H
#define LedController_H

#include <Arduino.h>

//Copied from Adafruit_PWMServoDriver.h
#include <Wire.h>
#if defined(__AVR__)
 #define WIRE Wire
#elif defined(CORE_TEENSY) // Teensy boards
 #define WIRE Wire
#else // Arduino Due
 #define WIRE Wire1
#endif

//Use the ESP8266 scheduler if available.
#ifdef ESP8266
#include <Ticker.h>
#else
#include <FancyDelay.h>
#endif


//interpolation refresh rate is 100Hz
//Counter values are 1 centisecond
//interpolation counter values are uint32
//max delay is like 497 days, but 16 bit values are too small, and 24 bit is just weird.
#define LED_REFRESH_HERTZ 100
#define LED_REFRESH_MILLIS 10

//Typdedef for Delays in centiseconds
typedef uint32_t DelayCS;

//Channels are currently 8 bit (24-bit color) values
typedef uint8_t LedValue;

//Static Channel array sizes
//The range configurable on my shields it 0x40 - 0x4f
//This library assumes you always have consecutive boards on one bus, starting at 0x40 and counting up.
#define LED_NUMSHIELDS 16
#define LED_CHANNELS_PER_SHIELD 16
#define LED_MAXCHANNELS 256

//PWM Base address from Adafruit
#define LED_PWM_ADDR 0x40

class LedControllerClass{
    #ifdef TICKER_H
    Ticker _ticker;
    #else
    FancyDelay _rate;
    #endif
    
    //State variables
    bool _power;
    LedValue _state[LED_MAXCHANNELS];

    //interpolation variables
    DelayCS _interp_ctr[LED_MAXCHANNELS]; //per-channel interpolation counter
    DelayCS _interp_max[LED_MAXCHANNELS]; //per-Channel interpolation target
    LedValue _next[LED_MAXCHANNELS];
    LedValue _prev[LED_MAXCHANNELS];

    //How many actual channels are connected?
    uint8_t _num_connected_shields;
    uint16_t _num_connected_channels;

    public:
    LedControllerClass();
    void init();
    uint16_t numChannels(); //return the number of connected channels (Max = 256).
    void reset();
    void power(bool pwr); //set power state
    bool power(); //get power state
    void poll(); //use this in the default arduino loop();
    void tick(); //use this if calling from the ESP8266 Scheduler
    void queueChannel(uint8_t index, LedValue value, DelayCS delay); //queue up a new value for a single Channel
    LedValue getState(uint8_t index);
    const LedValue* getState();
    bool done(uint8_t index); //returns true if a given Channel is done interpolating
};

extern LedControllerClass LedController;

#endif /* LedController_H */
