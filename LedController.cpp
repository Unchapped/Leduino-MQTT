#include <Arduino.h>
#include "LedController.h"

//use this 8bit->12bit Gamma map has slightly better dynamic range
//source: https://learn.adafruit.com/led-tricks-gamma-correction/the-longer-fix
#ifdef ESP8266
const uint16_t gamma_16[] = { 
#else
const uint16_t PROGMEM gamma_16[] = { 
#endif
     0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   1,   1,   1,   1,   1,
     2,   2,   2,   3,   3,   4,   4,   5,   5,   6,   7,   8,   8,   9,  10,  11,
    12,  13,  15,  16,  17,  18,  20,  21,  23,  25,  26,  28,  30,  32,  34,  36,
    38,  40,  43,  45,  48,  50,  53,  56,  59,  62,  65,  68,  71,  75,  78,  82,
    85,  89,  93,  97, 101, 105, 110, 114, 119, 123, 128, 133, 138, 143, 149, 154,
   159, 165, 171, 177, 183, 189, 195, 202, 208, 215, 222, 229, 236, 243, 250, 258,
   266, 273, 281, 290, 298, 306, 315, 324, 332, 341, 351, 360, 369, 379, 389, 399,
   409, 419, 430, 440, 451, 462, 473, 485, 496, 508, 520, 532, 544, 556, 569, 582,
   594, 608, 621, 634, 648, 662, 676, 690, 704, 719, 734, 749, 764, 779, 795, 811,
   827, 843, 859, 876, 893, 910, 927, 944, 962, 980, 998,1016,1034,1053,1072,1091,
  1110,1130,1150,1170,1190,1210,1231,1252,1273,1294,1316,1338,1360,1382,1404,1427,
  1450,1473,1497,1520,1544,1568,1593,1617,1642,1667,1693,1718,1744,1770,1797,1823,
  1850,1877,1905,1932,1960,1988,2017,2045,2074,2103,2133,2162,2192,2223,2253,2284,
  2315,2346,2378,2410,2442,2474,2507,2540,2573,2606,2640,2674,2708,2743,2778,2813,
  2849,2884,2920,2957,2993,3030,3067,3105,3143,3181,3219,3258,3297,3336,3376,3416,
  3456,3496,3537,3578,3619,3661,3703,3745,3788,3831,3874,3918,3962,4006,4050,4095 };

#ifdef ESP8266
inline uint16_t gamma(uint8_t val) {return gamma_16[val];}
#else
inline uint16_t gamma(uint8_t val) {return pgm_read_word(&gamma_16[val]);}
#endif

#ifdef TICKER_H
static void _LedController_tick_cb();
LedControllerClass::LedControllerClass() : _power(true) {};
#else
LedControllerClass::LedControllerClass() : _rate(LED_REFRESH_MILLIS), _power(true) {};
#endif


//Copied from Adafruit_PWMServoDriver.cpp
#define PCA9685_SUBADR1 0x2
#define PCA9685_SUBADR2 0x3
#define PCA9685_SUBADR3 0x4

#define PCA9685_MODE1 0x0
#define PCA9685_PRESCALE 0xFE

#define LED0_ON_L 0x6
#define LED0_ON_H 0x7
#define LED0_OFF_L 0x8
#define LED0_OFF_H 0x9

#define ALLLED_ON_L 0xFA
#define ALLLED_ON_H 0xFB
#define ALLLED_OFF_L 0xFC
#define ALLLED_OFF_H 0xFD


//quick and dirty inline function
static inline uint8_t _i2caddr(uint8_t index) {
  return LED_PWM_ADDR | (index & 0x0f);
}

//These two helper functions are copied and modified from AdafruitPWMDriver Library
static void _i2c_write8(uint8_t index, uint8_t addr, uint8_t d) {
  WIRE.beginTransmission(_i2caddr(index));
  WIRE.write(addr);
  WIRE.write(d);
  WIRE.endTransmission();
}

static uint8_t _i2c_read8(uint8_t index, uint8_t addr) {
  WIRE.beginTransmission(_i2caddr(index));
  WIRE.write(addr);
  WIRE.endTransmission();

  WIRE.requestFrom(_i2caddr(index), (uint8_t)1);
  return WIRE.read();
}

//Copied and altered from Adafruit_PWMServoDriver::setPWM
static void _i2c_setLED(uint8_t channel, uint8_t value) {
    uint16_t gval = gamma(value);
    WIRE.beginTransmission(_i2caddr(channel >> 4)); //(channel >> 4) is shorthand for (i % 16)
    WIRE.write(LED0_ON_L+4*(channel & 0x0f)); //(channel & 0x0f) is shorthand for (i / 16)
    WIRE.write(0); //On @ 0
    WIRE.write(0); //On @ 0
    WIRE.write(gval);
    WIRE.write(gval>>8);
    WIRE.endTransmission();
}



void LedControllerClass::init() {
    _num_connected_shields = 0;
    _num_connected_channels = 0;

    //initialize all shields
    WIRE.begin();
    for (uint8_t i = 0; i < LED_NUMSHIELDS; i++) {
        if(true){ //TODO: check for live shield
            _num_connected_shields++;
            _num_connected_channels += LED_CHANNELS_PER_SHIELD;


        } else break;
    }
    reset();

    #ifdef TICKER_H
    _ticker.attach_ms(LED_REFRESH_MILLIS, _LedController_tick_cb);
    #endif
}

void LedControllerClass::reset() {
    for (uint8_t i = 0; i < _num_connected_shields; i++) {
        //Copied from adafruit pwm.reset()
        _i2c_write8(i, PCA9685_MODE1, 0x0);

        //set prescale:
        //default prescale is 0x1E (~200 Hz) try this for now)
        //minimum prescale is 0x03 (1526 Hz)
        //Copied from adafriuit setPWMFreq;
        /* uint8_t prescale = 5 //(~1kHz)
        uint8_t oldmode = _i2c_read8(i, PCA9685_MODE1);
        uint8_t newmode = (oldmode&0x7F) | 0x10; // sleep
        _i2c_write8(i, PCA9685_MODE1, newmode); // go to sleep
        _i2c_write8(i, PCA9685_PRESCALE, prescale); // set the prescaler
        _i2c_write8(i, PCA9685_MODE1, oldmode);
        delay(5);
        _i2c_write8(i, PCA9685_MODE1, oldmode | 0xa1);  //  This sets the MODE1 register to turn on auto increment. */
    }
    for(uint8_t i = 0; i < _num_connected_channels; i++){
        _state[i] = 0; //turn all Channels off
        _interp_max[i] = _interp_ctr[i] = 0; //stop interpolation
    }
}

void LedControllerClass::queueChannel(uint8_t index, LedValue value, DelayCS delay) {
    if(index > _num_connected_channels) return; //ignore bad channel indexes.
    if(delay) { //non-instant update, set up interpolation from current point
        _prev[index] = _state[index];
        _next[index] = value;
        _interp_ctr[index] = 0;
        _interp_max[index] = delay;
    } else {
      _interp_ctr[index] = _interp_max[index] = 0; //disable interpolation
      _state[index] = _next[index] = value;
    }   
}

uint16_t LedControllerClass::numChannels(){
  return _num_connected_channels;
}

LedValue LedControllerClass::getState(uint8_t index){
  return _state[index];
}

const LedValue* LedControllerClass::getState(){
  return _state;
}

bool LedControllerClass::done(uint8_t index) {
    return (_interp_ctr[index] >= _interp_max[index]);
}

void LedControllerClass::poll() {
    //Use tick() with the ESP8266 instead of poll()
    #ifndef TICKER_H
    //update interpolation to next keyframe
    if(_rate.ready()){
        this->tick();
    }
    #endif
}

void LedControllerClass::tick() {
    for(int i = 0; i < LED_CHANNELS_PER_SHIELD; i++){
        //Interpolate Channel Values
        if(_interp_ctr[i] < _interp_max[i]) { // !done
            _interp_ctr[i]++;
            _state[i] = ((_prev[i] * (_interp_max[i] - _interp_ctr[i])) + (_next[i] * _interp_ctr[i])) / _interp_max[i]; 
        }else{ //interpolation done
            _state[i] = _next[i]; //clamp state to expected final value
        }

        //refresh output:
        if(_power) { //TODO: Change this to use PCA SLEEP MODE
            _i2c_setLED(i, _state[i]); //(i >> 4) is shorthand for (i % 16)
        } else {
            _i2c_setLED(i, 0); //(i >> 4) is shorthand for (i % 16)
        }
    }
}

void LedControllerClass::power(bool pwr) {
  //TODO: Change this to use PCA SLEEP MODE
    _power = pwr;
}

bool LedControllerClass::power() {
    return _power;
}

//see extern def in header
LedControllerClass LedController;

//Ticker callback redirect
#ifdef TICKER_H
void _LedController_tick_cb(){
  LedController.tick();
}
#endif