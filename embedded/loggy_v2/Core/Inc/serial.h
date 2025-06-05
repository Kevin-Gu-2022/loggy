/*
 * serial.h
 *
 *  Created on: Mar 23, 2025
 *      Author: kevin
 */

#ifndef INC_SERIAL_H_
#define INC_SERIAL_H_

#include "ring_buffer.h"

#define NUM_CHANNELS 8

/* All relevant data for one channel (units signal the temperature configuration) */
typedef struct __attribute__((packed)) {
	float alarmLowThreshold[NUM_CHANNELS];
	float alarmHighThreshold[NUM_CHANNELS];
	uint8_t alarmStatus[NUM_CHANNELS];  // The third bit will represent if alarm on or not
	uint8_t unit[NUM_CHANNELS];
	uint8_t inputRange[NUM_CHANNELS];
	uint32_t rtcTime;
	uint8_t recordingState;
} SerialDataInput_t;

typedef struct __attribute__((packed)) {
	float channelReading[NUM_CHANNELS];  // Must memcpy into these arrays
	float alarmLowThreshold[NUM_CHANNELS];
	float alarmHighThreshold[NUM_CHANNELS];
	uint8_t alarmStatus[NUM_CHANNELS];  // The third bit will represent if alarm on or not
	uint8_t unit[NUM_CHANNELS];
	uint8_t inputRange[NUM_CHANNELS];
	uint32_t rtcTime;
	uint8_t recordingState;
	uint8_t opticalLinkState;
} SerialDataOutput_t;

extern RingBuffer ringBuff;
//extern volatile uint32_t prevRecvTick;
extern volatile uint8_t uartRecvFlag;


extern void transmit_state();
extern void serialIn_init();

extern void check_serial();

#endif /* INC_SERIAL_H_ */
