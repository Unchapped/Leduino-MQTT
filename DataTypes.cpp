#include "DataTypes.h"

/* ChannelConfig Methods */	
ChannelConfig::operator const uint8_t() const { return packed; }
ChannelConfig &ChannelConfig::operator=( uint8_t in ) { packed = in; return *this;  }

uint8_t ChannelConfig::color(){ //Get
	return packed & 0x03;
}
uint8_t ChannelConfig::color(uint8_t val){ //set
	packed &= ~0x03;
	return packed |= val & 0x03;
}

uint8_t ChannelConfig::group(){ //Get
	return packed >> 2;
}
uint8_t ChannelConfig::group(uint8_t val){ //set
	packed &= 0x03;
	return packed |= val << 2;
}