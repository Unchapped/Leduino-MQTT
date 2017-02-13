#include "MQTTClient.h"

//Callback redirect definitions, see below for implementation
static void _MQTTClient_recv_cb (char* topic, byte* payload, unsigned int length);
#ifdef TICKER_H
static void _MQTTClient_status_cb();
#endif


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

//keyframe syntax parser helper function
//expects"[delay(float)]:channel 0 [int], channel 1 [int], ..."
static void _parse_keyframe(char * message, unsigned int length){
      Keyframe _kfBuf; //internal Keyframe buffer
      char * token = message;
      size_t len = _tokenize_in_place(token, ':');
      float f_delay = atof(token);
      _kfBuf.delay = f_delay * LED_REFRESH_HERTZ; //convert to ticks, implicit cast to uint32_t
      token += len + 1; //next chunk will be past the null    
      //parse channels
      int ch_id = 0;
      while (ch_id < LED_NUMCHANNELS){
        len = _tokenize_in_place(token, ',');
        if(len != 0){
          _kfBuf.channel[ch_id++] = atoi(token);
          token += len + 1; //next chunk will be past the null
        }else{
          _kfBuf.channel[ch_id++] = 0;
        }
      }
      LedController.queueKeyframe(_kfBuf);
}

#ifdef TICKER_H
MQTTClientClass::MQTTClientClass() : _client(MQTT_SERVER, MQTT_PORT, _MQTTClient_recv_cb , _netClient) {
#else
MQTTClientClass::MQTTClientClass() : _client(MQTT_SERVER, MQTT_PORT, _MQTTClient_recv_cb , _netClient), _status_rate(MQTT_STATUS_MILLIS) {
#endif
  //initialize internal string buffers
  _topic_buffer = _topic_buffer_secret + sprintf(_topic_buffer_secret, "%s/%02u/", MQTT_TOPIC_NAMESPACE, MQTT_NODE_ID); //write the namespace and node ID to the topic buffer
}

boolean MQTTClientClass::_publish_buffers() {
  _client.publish(this->_topic_buffer_secret, this->_message_buffer);
}



void MQTTClientClass::_callback(char* topic, byte* payload, unsigned int length) {
  //cache the topic name and payload, since they are changed by any publish calls
  char topic_name[MQTT_MAX_TOPIC_LEN]; //HACK: check this!
  strcpy(topic_name, topic + strlen(MQTT_TOPIC_NAMESPACE) + 4); //leduino + "/ID/" 

  char message[length + 1];
  strncpy(message, (char *) payload, length);
  message[length] = 0; //enforce null termination

  switch(topic_name[0]){
    case 'p': //set power off/on
      LedController.power(atoi(message));
      return;
    case 'c': //set channel immediately
      LedController.setChannel(atoi(topic_name + 8), atoi(message)); // 8 = strlen("channel/")
      return;
    case 'q': //queue keyframe expects"[delay(float)]:channel 0 [int], channel 1 [int], ..."
      _parse_keyframe(message, length);
      return;
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
  

  char client_id[] = "leduino_ID";
  sprintf(client_id + sizeof("leduino_") - 1, "%u", MQTT_NODE_ID);
  sprintf(_topic_buffer, "%s", "announce");
  if (_client.connect(client_id, _topic_buffer_secret, 0, true, "down")) {
    _client.publish(_topic_buffer_secret, "up", true); //announce the node is up
    sprintf(_topic_buffer, "%s", "power");
    _client.subscribe(_topic_buffer_secret);
    sprintf(_topic_buffer, "%s", "queue");
    _client.subscribe(_topic_buffer_secret);
    sprintf(_topic_buffer, "%s", "channel/+");
    _client.subscribe(_topic_buffer_secret);
    //digitalWrite(LEDPIN, HIGH); // turn on LED
  } else {
    //fatalError();
  }

  #ifdef TICKER_H
  _status_ticker.attach_ms(MQTT_STATUS_MILLIS, _MQTTClient_status_cb);
  #endif
}

void MQTTClientClass::poll(){
  #ifndef TICKER_H
  //publish the current status
  if(_status_rate.ready()){
    this->report_status();
  }
  #endif
  _client.loop();
}

void MQTTClientClass::report_status(){
  sprintf(_topic_buffer, "%s", "status");
  uint8_t buf_len = LED_NUMCHANNELS * 4;
  char buffer[buf_len];
  char * bPtr = buffer;
  for(int i = 0; i < LED_NUMCHANNELS; i++){
    bPtr += sprintf ( bPtr, "%03u,", LedController.getState(i));
  }
  buffer[buf_len - 1] = 0; //null terminate the last comma
  _client.publish(_topic_buffer_secret, buffer);
}

//Callback redirect functions
MQTTClientClass MQTTClient;
void _MQTTClient_recv_cb (char* topic, byte* payload, unsigned int length) {
  MQTTClient._callback(topic, payload, length);
}

#ifdef TICKER_H
void _MQTTClient_status_cb() {
  MQTTClient.report_status();
}
#endif
