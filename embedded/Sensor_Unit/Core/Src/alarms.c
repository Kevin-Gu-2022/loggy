/*
 * alarms.c
 *
 *  Created on: Apr 28, 2025
 *      Author: Lucy
 */

#include "alarms.h"

// Function to get the alarm mode for a specific channel
AlarmMode_t get_alarm_mode_for_channel(uint8_t *alarmModes, int channel) {
    if (channel < 0 || channel >= NUM_CHANNELS) {
        printf("Invalid channel number.\n");
        return DISABLED_MODE;  // Return DISABLED_MODE if invalid channel
    }

    uint8_t alarmMode = alarmModes[channel];  // Get the alarm mode for the channel
    return (AlarmMode_t)(alarmMode & ALARM_MODE_MASK);  // Extract the mode using the mask
}

// Function to turn on the Alarm LED
void turn_on_led(GPIO_TypeDef* port, uint16_t pin) {
    HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET);   // Set LED pin HIGH
}

// Function to turn off the Alarm LED
void turn_off_led(GPIO_TypeDef* port, uint16_t pin) {
    HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET); // Set LED pin LOW
}
