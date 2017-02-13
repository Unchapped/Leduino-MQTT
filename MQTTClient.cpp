#include "MQTTClient.h"

//Callback redirect definitions, see below for implementation
static void _MQTTClient_recv_cb (char* topic, byte* payload, unsigned int length);
#ifdef TICKER_H
static void _MQTTClient_status_cb();
#endif

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
MQTTClientClass::MQTTClientClass() : _client(MQTT_SERVER, MQTT_PORT, _MQTTClient_recv_cb , _netClient) {}
#else
MQTTClientClass::MQTTClientClass() : _client(MQTT_SERVER, MQTT_PORT, _MQTTClient_recv_cb , _netClient), _status_rate(MQTT_STATUS_MILLIS) {}
#endif

void MQTTClientClass::init(uint8_t id){
  _node_id = id;
  #ifdef ESP8266
    WiFi.begin(WIFI_SSID, WIFI_PASSWD);
    while (WiFi.status() != WL_CONNECTED) {
      delay(250);
      //digitalWrite(LEDPIN, blink = HIGH - blink); //TODO: Abstract Status LED functions, slow blink while connecting
    }
  #else //TODO: check for yun explicitly.
    Bridge.begin();
  #endif

  //initialize internal string buffers
  //write the namespace and node ID to the topic buffer
  _topic_buffer = _topic_buffer_secret + sprintf(_topic_buffer_secret, "%s/%02u/", MQTT_TOPIC_NAMESPACE, _node_id);

  //Connect to the MQTT server, prepping client id and LWT messages
  char client_id[] = "leduino_ID";
  sprintf(client_id + sizeof("leduino_") - 1, "%02u", _node_id);
  sprintf(_topic_buffer, "%s", "announce");
  if (_client.connect(client_id, _topic_buffer_secret, 0, true, "down")) {
    //announce the node is up
    _client.publish(_topic_buffer_secret, "up", true);

    //subscribe to the powere and queue topics
    sprintf(_topic_buffer, "%s", "power");
    _client.subscribe(_topic_buffer_secret);
    sprintf(_topic_buffer, "%s", "queue");
    _client.subscribe(_topic_buffer_secret);
    //digitalWrite(LEDPIN, HIGH); // turn on LED
  } else {
    //fatalError();
  }

  //start the ticker running
  #ifdef TICKER_H
  _status_ticker.attach_ms(MQTT_STATUS_MILLIS, _MQTTClient_status_cb);
  #endif
}

boolean MQTTClientClass::_publish() {
  _client.publish(this->_topic_buffer_secret, this->_message_buffer);
}

void MQTTClientClass::_callback(char* topic, byte* payload, unsigned int length) {
  //strip the first character off the topic name, since I don't currently need the rest
  char topic_opt = topic[11]; // 11 = omit 'leduino/ID/'

  if(topic_opt = 'q'){
    //expects [delay(cs, uint32 (little endian))][chid(char)val(char)][chid(char)val(char)]...
    DelayCS delay = *((DelayCS*) payload);
    unsigned int offset = sizeof(DelayCS);
    while(offset < length){
      uint8_t channel = *((uint8_t*) payload + offset++);
      LedValue value = *((LedValue*) payload + offset++);
      LedController.queueChannel(channel, value, delay); //queue up a new value for a single Channel
    }
  }
  if(topic_opt = 'p'){ //set power off/on
    LedController.power(payload[0] == '1'); //faster than atoi()
  }
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
  //publish in binary and let the wee pythonic bastards figure it oot!!!
  _client.publish(_topic_buffer_secret, LedController.getState(), LED_NUMCHANNELS, false);
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
