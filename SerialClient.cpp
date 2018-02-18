#include "SerialClient.h"

//Callback redirect definitions, see below for implementation
static void _SerialClient_recv_cb (char* topic, byte* payload, unsigned int length);
#ifdef TICKER_H
static void _SerialClient_status_cb();
#endif

#ifdef TICKER_H
SerialClientClass::SerialClientClass() {}
#else
SerialClientClass::SerialClientClass() : _status_rate(SERIAL_STATUS_MILLIS) {}
#endif

void SerialClientClass::init(const Stream * port){
  _port = port;

  //start the ticker running
  #ifdef TICKER_H
  _status_ticker.attach_ms(SERIAL_STATUS_MILLIS, _serialclient_status_cb);
  #endif
}

void SerialClientClass::poll(){
  #ifndef TICKER_H
  //publish the current status
  if(_status_rate.ready()){
    this->report_status();
  }
  #endif
  //TODO: define serial contol protocol and parser here!

}

void SerialClientClass::report_status(){
  for (int i = 0; i < LedController.numChannels(); i++){
    _port->print(LedController.getState(i));
    _port->print(' ');
  }
  _port->println();
}

//Static Instance
SerialClientClass SerialClient;

#ifdef TICKER_H
void _serialclient_status_cb() {
  SerialClient.report_status();
}
#endif
