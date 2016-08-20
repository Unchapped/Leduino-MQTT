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


void LedControllerClass::init() {
    #ifdef TWBR
    TWBR = 12; // set I2C to 400KHz
    #endif
    _pwm.begin();
    reset();
}

void LedControllerClass::queueKeyframe(Keyframe &kf) {
    for(int i = 0; i < NUMCHANNELS; i++){
        if(kf.delay) { //non-instant update, set up interpolation from current point
            _prev[i] = _state[i];
            _next[i] = kf.channel[i];
            _interp_ctr = 0;
            _interp_max = kf.delay;
        } else {
          _interp_ctr = _interp_max = 0; //disable interpolation
          _state[i] = _next[i] = kf.channel[i];
        }
    }   
}

//Sets one channel immediately, while allowing all others to continue interpolating
void LedControllerClass::setChannel(uint8_t index, Channel value){
  _prev[index] = _state[index] = _next[index] = value;
}

const Channel* LedControllerClass::getState(){
  return (const Channel*) _state;
}

void LedControllerClass::poll() {
    //update interpolation to next keyframe
    if(_rate.ready()){
        _interp_ctr++;
        uint8_t _interp_pct = map(_interp_ctr, 0, _interp_max, 0, 0xff); 
        for(int i = 0; i < NUMCHANNELS; i++){
            if(done()) { //interpolation done
                _interp_ctr = _interp_max = 0; //disable interpolation
                _state[i] = _next[i]; //clamp state to expected final value
            }else{
                //Interpolation is (A*(255-x)+B*x)/255. It requires only 8x8 multiplication, and a final division by 255,
                //which can be approximated by simply taking the high byte of the sum.
                _state[i] = ((_prev[i] * (uint16_t) (0xff-_interp_pct)) + (_next[i] * (uint16_t) _interp_pct)) >> 8;
            }

            //refresh output:
            if(_power) {
                uint16_t gval = gamma(_state[i]);
                _pwm.setPWM(i, 0, gval);
            } else {
                _pwm.setPWM(i, 0, 0);
            }
        }
    }
}

bool LedControllerClass::done() {
    return (_interp_ctr >= _interp_max);
}

void LedControllerClass::power(bool pwr) {
    _power = pwr;
}

void LedControllerClass::reset() {
    _pwm.reset();
    _pwm.setPWMFreq(120);
    for(uint8_t i = 0; i < NUMCHANNELS; i++){
        _state[i] = 0;
    }
    _interp_max = _interp_ctr = 0; //stop interpolation
}

//see extern def in header
LedControllerClass LedController;
