#ifndef LedController_H
#define LedController_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

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

//Need to try the maximum frequency for this, let's start at 1 KHz.
#define LED_PWM_FREQ 1000

//Typdedef for Delays in centiseconds
typedef uint32_t DelayCS;

//Channels are currently 8 bit (24-bit color) values
#define LED_NUMCHANNELS 16
typedef uint8_t Channel;

struct Keyframe{
    DelayCS delay; //how long to fade in this keyframe see KEYFRAME_LSHIFT
    Channel channel[LED_NUMCHANNELS];
};


class LedControllerClass{
    #ifdef TICKER_H
    Ticker _ticker;
    #else
    FancyDelay _rate;
    #endif
    Adafruit_PWMServoDriver _pwm; 

    //State variables
    bool _power;
    Channel _state[LED_NUMCHANNELS];

    //interpolation variables
    DelayCS _interp_ctr[LED_NUMCHANNELS]; //per-channel interpolation counter
    DelayCS _interp_max[LED_NUMCHANNELS]; //per-channel interpolation target
    Channel _next[LED_NUMCHANNELS];
    Channel _prev[LED_NUMCHANNELS];

    public:
    LedControllerClass(uint8_t addr = 0x40);
    void init();
    void reset();
    void power(bool pwr); //set power state
    bool power(); //get power state
    void poll(); //use this in the default arduino loop();
    void tick(); //use this if calling from the ESP8266 Scheduler
    void queueChannel(uint8_t index, Channel value, DelayCS delay); //queue up a new value for a single channel
    void setChannel(uint8_t index, Channel value); //instantly set a value for a new channel
    void queueKeyframe(Keyframe &kf); //queue up a new value for all channels
    Channel getState(uint8_t index);
};

extern LedControllerClass LedController;

#endif /* LedController_H */
