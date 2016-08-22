#include "MQTTClient.h"


static void _MQTTcallback (char* topic, byte* payload, unsigned int length);

//Buffer tokenizer helper finction
static size_t _tokenize_in_place(char * buffer, char limiter){
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

/*
void fatalError(){
  pinMode(LEDPIN, OUTPUT);
  while(true){
    digitalWrite(LEDPIN, HIGH);
    delay(100);
    digitalWrite(LEDPIN, LOW);
    delay(100);
  }
}

uint8_t blink = HIGH;
*/

MQTTClientClass::MQTTClientClass() : _client(MQTT_SERVER, MQTT_PORT, _MQTTcallback , _netClient), _status_rate(500) {}

void MQTTClientClass::_callback(char* topic, byte* payload, unsigned int length) {
  //cache the topic name and payload, since they are changed by any publish calls
  char topic_name[16]; //HACK: fixed-length buffer!
  strcpy(topic_name, topic + strlen(MQTT_TOPIC_PREFIX)); 

  char message[length + 1];
  strncpy(message, (char *) payload, length);
  message[length] = 0; //enforce null termination

  if(topic_name[0] == 'p') { //power
    LedController.power(atoi(message)); //set power off/on
    return;
  } else { //queue keyframe expects"[delay(float)]:channel 0 [int], channel 1 [int], ..."
    char * token = message;
    size_t len = _tokenize_in_place(token, ':');
    float f_delay = atof(token);
    _kfBuf.delay = f_delay * REFRESH_HERTZ; //convert to ticks, implicit cast to uint32_t
    token += len + 1; //next chunk will be past the null    
    //parse channels
    int ch_id = 0;
    while (ch_id < NUMCHANNELS){
      len = _tokenize_in_place(token, ',');
      _kfBuf.channel[ch_id++] = atoi(token);
      if(len != 0){
        token += len + 1; //next chunk will be past the null
      }else{
        break;
      }
    }
    LedController.queueKeyframe(_kfBuf);
  }
}

void MQTTClientClass::init(){
  #ifdef ESP8266
    WiFi.begin(WIFI_SSID, WIFI_PASSWD);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      //digitalWrite(LEDPIN, blink = HIGH - blink); //TODO: Abstract Status LED functions, slow blink while connecting
    }
  #else //TODO: check for yun explicitly.
    Bridge.begin();
  #endif
  
  if (_client.connect(MQTT_CLIENT_ID, MQTT_TOPIC_ANNOUNCE, 1, false, MQTT_LWT_MSG)) {
    _client.publish(MQTT_TOPIC_ANNOUNCE, MQTT_ANNOUNCE_MSG);
    _client.publish("home/leduino/0/num_channels", "16", true); //HACK, use #defines
    _client.subscribe("home/leduino/0/power");
    _client.subscribe("home/leduino/0/queue");
    //digitalWrite(LEDPIN, HIGH); // turn on LED
  } else {
    //fatalError();
  }
}

void MQTTClientClass::poll(){
  //publish the current status
  if(_status_rate.ready()){
    char buffer[NUMCHANNELS * 4 + 1];
    char *bPtr = buffer;
    for(int i = 0; i < NUMCHANNELS; i++){
      bPtr += sprintf ( bPtr, "%03u,", LedController.getState(i)); //back off the null terminator
    }
    bPtr--;
    *bPtr = '\0'; //null terminate the last comma
    _client.publish("home/leduino/0/status", buffer);
  }
  _client.loop();
}

MQTTClientClass MQTTClient;
void _MQTTcallback (char* topic, byte* payload, unsigned int length) {
  MQTTClient._callback(topic, payload, length);
}