#ifndef MQTTCLIENT_H
#define MQTTCLIENT_H

#include <PubSubClient.h> 
#include <FancyDelay.h> 
#include "LedController.h"
#include "NetworkConfig.h"

#ifdef ESP8266
  #include <ESP8266WiFi.h> 
#else
  #include <Bridge.h>
  #include <YunClient.h>
#endif


/* MQTT Topics associated with this node
home/leduino/ID/power //set 1 or 0 for off/on
home/leduino/ID/queue //enqueue a keyframe Expects "[delay(float)]:channel 0 [int], channel 1 [int], ..."
home/leduino/ID/channel/INDEX //Instantly set a Channel value 0-255

publishes:
home/leduino/ID/num_channels //Latched, NUMCHANNELS
home/leduino/ID/status //refresh at 2 hz, 0-255 uint8[16] array
home/leduino/node_announce //online/LWT messages.
*/

//HACK, fix this:
#define MQTT_CLIENT_ID "leduino_0"
#define MQTT_TOPIC_PREFIX "home/leduino/0/"
#define MQTT_TOPIC_ANNOUNCE "home/leduino/node_announce"
#define MQTT_ANNOUNCE_MSG "0 up"
#define MQTT_LWT_MSG "0 down"



class MQTTClientClass{
    Keyframe _kfBuf; //internal Keyframe buffer
    FancyDelay _status_rate; //1 Hz report rate

    #ifdef ESP8266
    WiFiClient _netClient;
    #else
    YunClient _netClient;
    #endif
    PubSubClient _client;

    public:
    MQTTClientClass();
    void init();
    void poll(); //use this in the default arduino loop();
    void tick(); //use this if calling from the ESP8266 Scheduler
    void _callback(char* topic, byte* payload, unsigned int length);
};

extern MQTTClientClass MQTTClient;

#endif /* MQTTCLIENT_H */