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


//interpolation refresh rate is 64Hz
//interpolation counter values are uint32
//with fixed point secs at << 6
//max interpolation time is 194 days
#define REFRESH_HERTZ 100
#define REFRESH_MILLIS 10
typedef uint32_t RefreshCtr;

//keyframe resolution is the same as interpolation rate
typedef uint32_t KfDelay;

//Channels are currently 8 bit (24-bit color) values
#define NUMCHANNELS 16
typedef uint8_t Channel;

struct Keyframe{
    KfDelay delay; //how long to fade in this keyframe see KEYFRAME_LSHIFT
    Channel channel[NUMCHANNELS];
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
    Channel _state[NUMCHANNELS];

    //interpolation variables
    RefreshCtr _interp_ctr, _interp_max; //synchronous cycle marker
    Channel _next[NUMCHANNELS];
    Channel _prev[NUMCHANNELS];

    public:
    LedControllerClass(uint8_t addr = 0x40);
    void init();
    void reset();
    void power(bool pwr);
    bool power(); //read power state
    void poll(); //use this in the default arduino loop();
    void tick(); //use this if calling from the ESP8266 Scheduler
    void queueKeyframe(Keyframe &kf);
    void setChannel(uint8_t index, Channel value);
    Channel getState(uint8_t index);
    bool done();
};

extern LedControllerClass LedController;

#endif /* LedController_H */
