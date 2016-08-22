#include <Arduino.h>

#include <FancyDelay.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <PubSubClient.h>  
#include "LedController.h"
#include "NetworkConfig.h"
#include "MQTTClient.h"

#define SDA_PIN 4
#define SCL_PIN 5
#define LED_PIN 2


Keyframe kfBuf;

void setup() {
  #ifdef ESP8266
    Wire.begin(SDA_PIN, SCL_PIN);
  #endif
  LedController.init();
  MQTTClient.init();

  /* DEBUG: turn on a channel for debugging. * /
  kfBuf.delay = REFRESH_HERTZ;
  kfBuf.channel[0] = 255;
  LedController.queueKeyframe(kfBuf);
  */
}

void loop() {
  LedController.poll();
  /* DEBUG: Flash a channel at 1 Hz. * /
  if (LedController.done()){
    kfBuf.channel[0] = 255 - kfBuf.channel[0];
    LedController.queueKeyframe(kfBuf);
  }
  */
  MQTTClient.poll();
}
