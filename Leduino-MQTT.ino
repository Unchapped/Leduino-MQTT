#include <Arduino.h>
#include <Bridge.h>
#include <YunClient.h>
#include <FancyDelay.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <PubSubClient.h>  
#include "LedController.h"

Keyframe kfBuf;
YunClient yunClient;

/* MQTT Topics associated with this node
home/leduino/ID/power //set 1 or 0 for off/on
home/leduino/ID/queue //enqueue a keyframe Expects "[delay(float)]:channel 0 [int], channel 1 [int], ..."

publishes:
home/leduino/ID/num_channels //Latched, NUMCHANNELS
home/leduino/ID/status //refresh at 2 hz, uint8[16] array
home/leduino/node_announce //announces it's presence to the world
*/

//HACK: figure out how to parse these better.
#define NODE_ID 0
#define TOPIC_PREFIX "home/leduino/0/"
#define MQTT_SERVER "openhab.home.nathanielchapman.com"
#define MQTT_PORT 1883
#define ANNOUNCE_MSG "Node 0 online, 16 channels"

void fatalError(){
  pinMode(13, OUTPUT);
  while(true){
    digitalWrite(13, HIGH);
    delay(100);
    digitalWrite(13, LOW);
    delay(100);
  }
}

void MQTTcallback(char* topic, byte* payload, unsigned int length);
PubSubClient client(MQTT_SERVER, MQTT_PORT, MQTTcallback, yunClient);
FancyDelay status_report_rate(200); //5 Hz report rate

size_t tokenize_in_place(char * buffer, char limiter){
  size_t offset = 0;
  while(buffer[offset] != limiter) {
    if(buffer[offset] == 0) {
        return 0; //end of string, nothing consumed
    }
    offset++;
  }
  buffer[offset] = 0; //null terminate the chunk;
  return offset;
}

void MQTTcallback(char* topic, byte* payload, unsigned int length) {
  //cache the topic name and payload, since they are changed by any publish calls
  char topic_name[16]; //HACK: fixed-length buffer!
  strcpy(topic_name, topic + strlen(TOPIC_PREFIX)); 

  char message[length + 1];
  strncpy(message, payload, length);
  message[length] = 0; //enforce null termination

  if(topic_name[0] == 'p') { //power
    LedController.power(atoi(message)); //set power off/on
    return;
  } else { //queue keyframe expects"[delay(float)]:channel 0 [int], channel 1 [int], ..."
    char * token = message;
    size_t len = tokenize_in_place(token, ':');
    float f_delay = atof(token);
    kfBuf.delay = f_delay * REFRESH_HERTZ; //convert to ticks, implicit cast to uint32_t
    token += len + 1; //next chunk will be past the null    
    //parse channels
    int ch_id = 0;
    while (ch_id < NUMCHANNELS){
      len = tokenize_in_place(token, ',');
      kfBuf.channel[ch_id++] = atoi(token);
      if(len != 0){
        token += len + 1; //next chunk will be past the null
      }else{
        break;
      }
    }
    LedController.queueKeyframe(kfBuf);
  }
}

void setup() {
  LedController.init();
  Bridge.begin();
  kfBuf.delay = REFRESH_HERTZ; //1 sec by default
  if (client.connect("leduino")) {
    client.publish("home/leduino/node_announce",ANNOUNCE_MSG);
    client.publish("home/leduino/0/num_channels", "16", true);
    client.subscribe("home/leduino/0/power");
    client.subscribe("home/leduino/0/queue");
  } else {
    fatalError();
  }

  //turn on a channel for debugging.
  kfBuf.channel[0] = 200 - kfBuf.channel[0];
  LedController.queueKeyframe(kfBuf);
}

void loop() {
  LedController.poll();

  //publish the current status
  if(status_report_rate.ready()){
    char buffer[NUMCHANNELS * 4 + 1];
    char *bPtr = buffer;
    for(int i = 0; i < NUMCHANNELS; i++){
      bPtr += sprintf ( bPtr, "%03u,", LedController._state[i]); //back off the null terminator
    }
    client.publish("home/leduino/0/status", buffer);
  }

  client.loop();
}
