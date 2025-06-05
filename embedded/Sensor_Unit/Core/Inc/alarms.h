/*
 * alarms.h
 *
 *  Created on: Apr 28, 2025
 *      Author: Lucy
 */

#include "SEGGER_RTT.h"
#include <stdint.h>
#include "stm32l4xx_hal.h"
#include <stdio.h>
#include "optical.h"

#ifndef INC_ALARMS_H_
#define INC_ALARMS_H_

#define ALARM_MODE_MASK 0x07      // 3 bits for mode (Latched, Active, Disabled)
#define ALARM_STATE_MASK (1 << 1) // 2nd bit for alarm state (On/Off)
#define ALARM_DISABLED_MASK (1)  // 0th bit for Disabled mode

// Enum for alarm state (ON/OFF)
typedef enum {
    ALARM_OFF = 0,
    ALARM_ON = 1,
} AlarmState_t;

// Enum for alarm modes
typedef enum {
    ACTIVE_MODE = 0,    // Active mode
    LATCHED_MODE = 1,   // Latched mode
    DISABLED_MODE = 2,  // Disabled mode
} AlarmMode_t;

AlarmMode_t get_alarm_mode_for_channel(uint8_t *alarmModes, int channel);
void turn_on_led(GPIO_TypeDef* port, uint16_t pin);
void turn_off_led(GPIO_TypeDef* port, uint16_t pin);
#endif /* ALARMS_H_ */
