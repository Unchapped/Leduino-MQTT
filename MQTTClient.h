#ifndef MQTTCLIENT_H
#define MQTTCLIENT_H

#include <PubSubClient.h> 
 
#include "LedController.h"
#include "NetworkConfig.h"

#ifdef ESP8266
  #include <ESP8266WiFi.h> 
  #include <Ticker.h>
#else
  #include <FancyDelay.h>
  #include <Bridge.h>
  #include <YunClient.h>
#endif


/* MQTT Topics associated with this node
leduino/ID/power //set 1 or 0 for off/on
leduino/ID/channel/INDEX //Instantly set a Channel value 0-255


TODO: Change float to (ms, uint32 (little endian))
leduino/ID/queue //enqueue a keyframe (human readable) Expects "[delay(float)]:channel 0 [int], channel 1 [int], ..."

TODO:
leduino/ID/binqueue //enqueue a sparce binary keyframe, Expects "[delay(ms, uint32 (little endian))][chid(char)val(char)][chid(char)val(char)]..."

publishes:
leduino/ID/status //refresh at 2 hz, 0-255 uint8[16] array
leduino/ID/announce //online/LWT messages. either "up" or "down"
*/

//HACK, fix this:
//currently maximum 100 nodes
#define MQTT_NODE_ID 0
#define MQTT_MAX_TOPIC_LEN 32
#define MQTT_TOPIC_NAMESPACE "leduino"
#define MQTT_MAX_MSG_LEN MQTT_MAX_PACKET_SIZE - (MQTT_MAX_TOPIC_LEN + 7)
#define MQTT_STATUS_MILLIS 1000



class MQTTClientClass{
    char _topic_buffer_secret[MQTT_MAX_TOPIC_LEN]; //internal topic name buffer
    char * _topic_buffer;
    char _message_buffer[MQTT_MAX_MSG_LEN]; //internal message buffer
    boolean _publish_buffers(); //publishes the internal message buffer to the internal topic name buffer

    #ifdef TICKER_H
    Ticker _status_ticker;
    #else
    FancyDelay _status_rate; //1 Hz report rate
    #endif

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
    void report_status(); //publish our state
    void _callback(char* topic, byte* payload, unsigned int length);
};

extern MQTTClientClass MQTTClient;

#endif /* MQTTCLIENT_H */