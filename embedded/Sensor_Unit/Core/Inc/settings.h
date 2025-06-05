/*
 * settings.h
 *
 *  Created on: Apr 28, 2025
 *      Author: Lucy
 */

#ifndef INC_SETTINGS_H_
#define INC_SETTINGS_H_

#include <stdint.h>
#include "stm32l4xx_hal.h"
#include "alarms.h"
#include "optical.h"
#include "sample.h"
#include "gpio.h"
#include "stm32l4xx_hal_flash.h"

#define NUM_CHANNELS 8  // Define the number of channels, adjust as needed
#define NUM_RANGES 4

extern GPIO_TypeDef* AlarmLowPort[NUM_CHANNELS];
extern uint16_t AlarmLowPin[NUM_CHANNELS];

extern GPIO_TypeDef* AlarmHighPort[NUM_CHANNELS];
extern uint16_t AlarmHighPin[NUM_CHANNELS];

typedef struct {
	float alarmLowThreshold[NUM_CHANNELS];
	float alarmHighThreshold[NUM_CHANNELS];
	uint8_t alarmMode[NUM_CHANNELS];  // Alarm mode 3rd bit will be whether alarm is on/off
	uint8_t inputRange[NUM_CHANNELS];
} OpticalRec_t;

// Declare a global variable to store the current optical state
extern OpticalRec_t optical_state;

void update_settings(OpticalIn_t *dataIn);
void check_alarms_and_update_leds();
void check_range_switching(void);

#endif /* SETTINGS_H_ */
