#include "SerialClient.h"

SerialClientClass::SerialClientClass(HardwareSerial & port) : _status_rate(SERIAL_STATUS_MILLIS), _port(port) {}

void SerialClientClass::init(unsigned int speed ){
	_port.begin(speed);
}

void SerialClientClass::poll(){
	//publish the current status
	if(_status_rate.ready()){
		_port.print(LedController.numChannels());
		_port.print("\t");

		for (int i = 0; i < LedController.numChannels(); i++){
			_port.print(LedController.getState(i));
			_port.print("\t");
		}
		_port.println();
	}
	//TODO: define serial contol protocol and parser here!
}

//Static Instance
SerialClientClass SerialClient(Serial);
