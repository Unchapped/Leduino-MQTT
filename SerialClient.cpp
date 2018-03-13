#include "SerialClient.h"

#define CHAR_NOMS -2
#define CHAR_ERR -1


SerialClientClass::SerialClientClass(HardwareSerial & port) : _status_rate(SERIAL_STATUS_MILLIS), _port(port) {}

static int parse_char(char in) {
    static uint8_t _num_ptr = 0;
    static char    _num_buf[4]; //255 max
    if(in > '0' && in <= '9') { //valid number
    	_num_buf[_num_ptr++] = in;
			if(4 == _num_ptr) {
				_num_buf[_num_ptr++] = 0;
				_num_ptr = 0;
				return CHAR_ERR;
			}
			return CHAR_NOMS;
    } else { //another ascii character encountered, return the value read
    	_num_buf[_num_ptr++] = 0;
			_num_ptr = 0;
			return atoi(_num_buf);
    }
}

void SerialClientClass::init(unsigned int speed ){
	_port.begin(speed);
	_port.println("Leduino Online, H = on, L = off, chan:val");
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

	if (_port.available() > 0) {
    char in = _port.read();
    if (in == 'H') {
    	LedController.power(true);
      _serial_state = SERIAL_STATE_ERR;
    } else if (in == 'L') {
    	LedController.power(false);
      _serial_state = SERIAL_STATE_ERR;
    } else {
    	int res = parse_char(in);
    	switch (res){
    		case CHAR_NOMS:
    			return;
    		break;
    		case CHAR_ERR:
    			_serial_state = SERIAL_STATE_ERR;
    		break;
    		default:
    			switch(_serial_state){
    				case SERIAL_STATE_CHANNEL:
    					_channel = res;
    				break;
    				case SERIAL_STATE_VALUE:
    					LedController.queueChannel(_channel, res, 1);
    				break;
    				default: //SERIAL_STATE_ERROR
    				switch(in){
		          case ',':
		          case ' ':
		          case 0x0A: //new line
		          case 0x11: //tab
		            _serial_state = SERIAL_STATE_CHANNEL;
		          break;
		          default: //just consume input till you reach a space
		          break;
		        }
    			}
    		break;
    	}
    }
  }
}

//Static Instance
SerialClientClass SerialClient(Serial);
