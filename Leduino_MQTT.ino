#include <Arduino.h>
#include <FancyDelay.h>
#include <Wire.h>
//#include <Adafruit_PWMServoDriver.h>
 

#ifdef ESP8266
	#include <Ticker.h>
	#define SDA_PIN 4
	#define SCL_PIN 5
	#define LED_PIN 2
#else
	#define LED_PIN 13
#endif

//#define LED_MQTT_CLIENT_ENABLE 1
#define LED_SERIAL_CLIENT_ENABLE 1
#define LED_DEBUG_ENABLE 1


#include "LedController.h"

#ifdef LED_MQTT_CLIENT_ENABLE
	#include <PubSubClient.h>
	#include "NetworkConfig.h"
	#include "MQTTClient.h"
#endif

#ifdef LED_SERIAL_CLIENT_ENABLE
	#include "SerialClient.h"
#endif

void setup() {
	#ifdef ESP8266
		Wire.begin(SDA_PIN, SCL_PIN);
	#endif

	LedController.init();

	#ifdef LED_MQTT_CLIENT_ENABLE
		MQTTClient.init(0);
	#endif

	#ifdef LED_SERIAL_CLIENT_ENABLE
		SerialClient.init(9600);
	#endif
}

void loop() {
	LedController.poll();
	#ifdef LED_MQTT_CLIENT_ENABLE
		MQTTClient.poll();
	#endif

	#ifdef LED_SERIAL_CLIENT_ENABLE
		SerialClient.poll();
	#endif

	#ifdef LED_DEBUG_ENABLE
	if(LedController.done(0)){
		LedController.queueChannel(0, 255 -  LedController.getState(0), 200);
	}
	#endif
}
