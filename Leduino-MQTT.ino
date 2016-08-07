#include <Arduino.h>
#include <Bridge.h>
#include <YunClient.h>
#include <FancyDelay.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

//local includes, MVC style...
#include "DataTypes.h"
#include "LedController.h"

#include <PubSubClient.h>  

Keyframe kfBuf;
BridgeClient yunClient;

void MQTTcallback(char* topic, byte* payload, unsigned int length);
PubSubClient client("openhab", 1883, MQTTcallback, yunClient);

void MQTTcallback(char* topic, byte* payload, unsigned int length) {
  // handle message arrived
  client.publish("test/leduino_client_out","hello you.");
  kfBuf.channel[0] = 0;
  kfBuf.channel[1] = 255 - kfBuf.channel[1];
  LedController.queueKeyframe(kfBuf);
}

void setup() {
  LedController.init();
  Bridge.begin();
  Console.begin();
  while (!Console) {};
  kfBuf.delay = 2 << KEYFRAME_LSHIFT;
  if (client.connect("leduino")) {
    client.publish("test/leduino_client_out","hello world.");
    client.subscribe("test/leduino_client_in");
    kfBuf.channel[1] = 255;
  } else {
    Console.println("MQTT connection failed!");
    kfBuf.channel[0] = 255;
  }
  LedController.queueKeyframe(kfBuf);
}

void loop() {
  LedController.poll();
  client.loop();
}
