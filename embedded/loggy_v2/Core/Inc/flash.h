/*
 * flash.h
 *
 *  Created on: May 6, 2025
 *      Author: kevin
 */

#ifndef INC_FLASH_H_
#define INC_FLASH_H_

#include "state_control.h"

/* Struct to store in flash. Size = 112 B */
typedef struct __attribute__((packed, aligned(8))) {
	float alarmLowThreshold[NUM_CHANNELS];
	float alarmHighThreshold[NUM_CHANNELS];
	uint8_t alarmMode[NUM_CHANNELS];
	uint8_t unit[NUM_CHANNELS];
	uint8_t currentSource[NUM_CHANNELS];
	uint8_t sensorType[NUM_CHANNELS];
	uint8_t inputRange[NUM_CHANNELS];
} FlashState_t;


extern HAL_StatusTypeDef EEPROM_Init(void);
extern HAL_StatusTypeDef EEPROM_WriteStruct(FlashState_t *data);
extern HAL_StatusTypeDef EEPROM_ReadLastStruct(FlashState_t *data);

extern HAL_StatusTypeDef clear_flash_page();


#endif /* INC_FLASH_H_ */
