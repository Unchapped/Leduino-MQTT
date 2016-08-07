#ifndef DATATYPES_H
#define DATATYPES_H

#include <Arduino.h>
//number of channels in a keyframe...
#define NUMCHANNELS 16
#define NUMSEQUENCES 16

#define WHITE 0x00
#define RED 0x01
#define GREEN 0x02
#define BLUE 0x03

//interpolation refresh rate is 64Hz
//interpolation counter values are uint32
//with fixed point secs at << 6
#define REFRESH_LSHIFT 6
#define REFRESH_MICROS 15625UL
typedef uint32_t RefreshCtr;

//keyframe resolution is 4 Hz (fastest keyframe is 1/4 sec)
//keyframe delay values are uint16
//longest keyframe = 4.55 hrs
#define KEYFRAME_LSHIFT 2
#define KEYFRAME_MICROS 250000UL
typedef uint16_t KfDelay;

//sequences may only start on every other second
//max sequence delay is 36 hrs
#define SEQUENCE_RSHIFT 1
typedef uint16_t SeqDelay;

#define KF_TO_REFRESH_LSHIFT (REFRESH_LSHIFT - KEYFRAME_LSHIFT)

#define SEQUENCE_TO_REFRESH_LSHIFT (REFRESH_LSHIFT + SEQUENCE_RSHIFT)

//a ChannelConfig buffer should be able to be stored in an 8-bit char
//the color information is the lowest 2 bits
//the channel group id is the upper 6 bits (max 64 groups)

struct ChannelConfig{
	operator const uint8_t() const;
	ChannelConfig &operator=( uint8_t in );

	uint8_t color();
	uint8_t color(uint8_t val);
	uint8_t group();
	uint8_t group(uint8_t val);
	
	private:
	uint8_t packed;
};


//NOTE: Keyframes are 1-indexed, a 0 in next_id is a null_ptr.
typedef uint8_t KfPtr;
typedef uint8_t Channel; //???

struct Keyframe{
	union {
		struct {
			KfPtr next_id;
			KfDelay delay; //how long to fade in this keyframe see KEYFRAME_LSHIFT
			Channel channel[NUMCHANNELS];
		};
		uint8_t eep_data[]; //used to give a pointer to the eeprom storable part of the class
	};
};

struct Sequence{
	KfPtr head; //first keyframe ptr...
	SeqDelay start; //seconds after midnight to start the sequence.
	uint8_t insert(uint8_t idx, Keyframe &frame);
	uint8_t remove();
};

#endif /* DATATYPES_H */