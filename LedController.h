#ifndef LedController_H
#define LedController_H

#include <Arduino.h>
#include <Wire.h>
#include <FancyDelay.h>
#include <Adafruit_PWMServoDriver.h>
#include "DataTypes.h"

class LedControllerClass{
    Adafruit_PWMServoDriver _pwm = Adafruit_PWMServoDriver();
    FancyMicrosDelay _rate;

    //State variables
    bool _power = true;
    bool _playing = true;
    Channel _state[NUMCHANNELS];
    KfPtr _next_id; //used to cache the next keyframe to load for the controller to access

    //interpolation variables
    RefreshCtr _interp_ctr, _interp_max; //synchronous cycle marker
    Channel _next[NUMCHANNELS];
    Channel _prev[NUMCHANNELS];

    public:
    LedControllerClass() : _rate(REFRESH_MICROS) {};
    void init();
    void reset();
    void power(bool pwr);
    void poll();
    void play(bool play);
    void queueKeyframe(Keyframe &kf); //enqueue a keyframe or sequence
    KfPtr nextKeyframe(); //what's next in the keyframe queue
    bool done();
};

extern LedControllerClass LedController;

#endif /* LedController_H */
