/*
 * state_control.h
 *
 *  Created on: Apr 21, 2025
 *      Author: kevin
 */

#ifndef INC_STATE_CONTROL_H_
#define INC_STATE_CONTROL_H_

#include "optical.h"
#include "serial.h"

#define DISPLAY_REQUEST (1 << 0)

#define ALARM_ACTIVE_BIT (1 << 2)

#define NUM_CHANNELS 8

/* Define's for the different units */
#define VOLT 0
#define DEG 1
#define MS2 2

/* Channel Names */
#define CH1 0
#define CH2 1
#define CH3 2
#define CH4 3
#define CH5 4
#define CH6 5
#define CH7 6
#define CH8 7

#define ONE_VOLT 0
#define TEN_VOLT 1

#define ACTIVE 0
#define DISABLED 1
#define LATCHING 2

#define SENSOR_THERM 0
#define SENSOR_RTD 1

#define CURRENT_10UA 0
#define CURRENT_200UA 1

#define ALARM_ON 1
#define ALARM_OFF 0


typedef struct {
	float channelReading[NUM_CHANNELS];
	float alarmLowThreshold[NUM_CHANNELS];
	float alarmHighThreshold[NUM_CHANNELS];
	uint8_t alarmMode[NUM_CHANNELS];
	uint8_t alarmOn[NUM_CHANNELS];
	uint8_t unit[NUM_CHANNELS];
	uint8_t currentSource[NUM_CHANNELS];
	uint8_t sensorType[NUM_CHANNELS];
	uint8_t inputRange[NUM_CHANNELS];
	uint32_t rtcTime;
	uint8_t recordingState;
	uint8_t opticalLinkState;
} ControlState_t;

extern void init_control_state();

extern ControlState_t *get_current_state();

extern void update_state_from_serial(SerialDataInput_t *input);
extern void update_state_from_optical(int16_t *readings);
extern void check_alarm_no_input();

/* LCD Update Functions */
extern void toggle_recording();
extern void toggle_input_range(uint8_t row);
extern void unlatch_alarm(uint8_t row);

extern void set_optical_link_state(uint8_t state);




#endif /* INC_STATE_CONTROL_H_ */
