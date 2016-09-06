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

#ifdef TICKER_H
MQTTClientClass::MQTTClientClass() : _client(MQTT_SERVER, MQTT_PORT, _MQTTClient_recv_cb , _netClient) {
#else
MQTTClientClass::MQTTClientClass() : _client(MQTT_SERVER, MQTT_PORT, _MQTTClient_recv_cb , _netClient), _status_rate(MQTT_STATUS_MILLIS) {
#endif
  //initialize internal string buffers
  _topic_buffer = _topic_buffer_secret + sprintf(_topic_buffer_secret, "%s/%02u/", MQTT_TOPIC_NAMESPACE, MQTT_NODE_ID); //write the namespace and node ID to the topic buffer
}

boolean MQTTClientClass::_publish_buffers() {
  _client.publish(this->_topic_buffer, this->_message_buffer);
}

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
  /* TODO: fix this!
  sprintf ( topic_id_buf, "%u", i);
  char message_buf[] = "000";
  sprintf ( message_buf, "%u", LedController.power());
  _client.publish(topic_buf, message_buf); //publish power state

  for(int i = 0; i < NUMCHANNELS; i++){
    sprintf ( topic_id_buf, "%u", i);
    sprintf ( message_buf, "%03u", LedController.getState(i));
    _client.publish(topic_buf, message_buf);
  } */
  /*
  char buffer[NUMCHANNELS * 4 + 1];
  char *bPtr = buffer;
  for(int i = 0; i < NUMCHANNELS; i++){
    bPtr += sprintf ( bPtr, "%03u,", LedController.getState(i)); //back off the null terminator
  }
  bPtr--;
  *bPtr = '\0'; //null terminate the last comma
  _client.publish("home/leduino/0/status", buffer);
  */
  //_client.loop();
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
