#ifndef LedController_H
#define LedController_H

#include <Arduino.h>
#include <Wire.h>
#include <FancyDelay.h>
#include <Adafruit_PWMServoDriver.h>

//interpolation refresh rate is 64Hz
//interpolation counter values are uint32
//with fixed point secs at << 6
//max interpolation time is 194 days
#define REFRESH_HERTZ 64
#define REFRESH_LSHIFT 6
#define REFRESH_MICROS 15625UL
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
    FancyMicrosDelay _rate;
    Adafruit_PWMServoDriver _pwm; 

    //State variables
    bool _power;
    //Channel _state[NUMCHANNELS]; see hacky public below.

    //interpolation variables
    RefreshCtr _interp_ctr, _interp_max; //synchronous cycle marker
    Channel _next[NUMCHANNELS];
    Channel _prev[NUMCHANNELS];
    Channel _state[NUMCHANNELS]; //todo: make proper getter for this and make it private.

    public:
    LedControllerClass(uint8_t addr = 0x40) : _rate(REFRESH_MICROS), _pwm(addr), _power(true) {};
    void init();
    void reset();
    void power(bool pwr);
    void poll();
    void queueKeyframe(Keyframe &kf);
    void setChannel(uint8_t index, Channel value);
    Channel getState(uint8_t index);
    bool done();
};

extern LedControllerClass LedController;

#endif /* LedController_H */
