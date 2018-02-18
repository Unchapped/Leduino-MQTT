#include <Arduino.h>

#include <FancyDelay.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <PubSubClient.h>  

#ifdef ESP8266
#include <Ticker.h>
#define SDA_PIN 4
#define SCL_PIN 5
#define LED_PIN 2
#else
#define LED_PIN 13
#endif

#include "LedController.h"
//#include "NetworkConfig.h"
//#include "MQTTClient.h"
#include "SerialClient.h"

void setup() {
  #ifdef ESP8266
    Wire.begin(SDA_PIN, SCL_PIN);
  #endif
  LedController.init();

  //TODO: set the id frem ESP.getchipID?
  //MQTTClient.init(0);

  Serial.begin(9600);
  SerialClient.init(Serial);
}

void loop() {
  LedController.poll();
  //MQTTClient.poll();
  SerialClient.poll();
}
