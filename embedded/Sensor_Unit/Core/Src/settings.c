/*
 * settings.c
 *
 *  Created on: Apr 28, 2025
 *      Author: Lucy
 */

#include "settings.h"

OpticalRec_t optical_state;
uint8_t last_stored_range[4];  // Variable to store the last saved range values


// Initialize arrays with the appropriate GPIO ports and pins for each channel
GPIO_TypeDef* AlarmLowPort[NUM_CHANNELS] = {
    GPIOA, GPIOA, GPIOC, GPIOC,  // Channel 1, 2, 3, 4 Low LEDs
    GPIOB, GPIOB, GPIOC, GPIOB   // Channel 5, 6, 7, 8 Low LEDs
};

uint16_t AlarmLowPin[NUM_CHANNELS] = {
    GPIO_PIN_10, GPIO_PIN_8, GPIO_PIN_8, GPIO_PIN_6,  // Channel 1, 2, 3, 4 Low Pins
    GPIO_PIN_14, GPIO_PIN_12, GPIO_PIN_4, GPIO_PIN_0    // Channel 5, 6, 7, 8 Low Pins
};

GPIO_TypeDef* AlarmHighPort[NUM_CHANNELS] = {
    GPIOA, GPIOA, GPIOC, GPIOC,  // Channel 1, 2, 3, 4 High LEDs
    GPIOB, GPIOB, GPIOC, GPIOC   // Channel 5, 6, 7, 8 High LEDs
};

uint16_t AlarmHighPin[NUM_CHANNELS] = {
    GPIO_PIN_11, GPIO_PIN_9, GPIO_PIN_9, GPIO_PIN_7,  // Channel 1, 2, 3, 4 High Pins
    GPIO_PIN_15, GPIO_PIN_13, GPIO_PIN_4, GPIO_PIN_5    // Channel 5, 6, 7, 8 High Pins
};

void update_settings(OpticalIn_t *dataIn) {
	// Copy new state data into the global optical_state
	for (int i = 0; i < NUM_CHANNELS; i++) {
		optical_state.alarmLowThreshold[i] = ((float)dataIn->alarmLowThreshold[i])/1000;
	    optical_state.alarmHighThreshold[i] = ((float)dataIn->alarmHighThreshold[i])/1000;
	    optical_state.alarmMode[i] = dataIn->alarmMode[i] & 0x03;
	    optical_state.inputRange[i] = dataIn->inputRange[i];
	}
	// update range switching
	check_range_switching();

	// After updating the settings, check the alarm conditions and configure the LEDs
	check_alarms_and_update_leds();
}

// Function to check range switching for the first 4 channels
void check_range_switching(void) {
    // Loop through the first 4 channels (0 to 3)
    for (int i = 0; i < 4; i++) {
        // Check if the current value differs from the stored value
        if (optical_state.inputRange[i] != last_stored_range[i]) {
            // If the range has changed, update the corresponding PB9 pin state
            if (optical_state.inputRange[i] == 0) {
                // Set PB9 low if range is 0
                HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_RESET);  // PB9 low
            } else if (optical_state.inputRange[i] == 1) {
                // Set PB9 high if range is 1
                HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_SET);  // PB9 high
            }
            // Update last_stored_range with the new value
            last_stored_range[i] = optical_state.inputRange[i];
        }
    }
}

void check_alarms_and_update_leds() {
    for (int i = 0; i < NUM_CHANNELS; i++) {
        // Get the alarm mode for the current channel
        AlarmMode_t mode = get_alarm_mode_for_channel(optical_state.alarmMode, i);

        // Get the sample value for this channel
        float sampled_value = sensor_data[i];

        // Get the alarm thresholds for the current channel
        float low_threshold = optical_state.alarmLowThreshold[i];
        float high_threshold = optical_state.alarmHighThreshold[i];

        // Check the alarm mode and thresholds, and update the LEDs
        if (mode == ACTIVE_MODE) {
            // If the sampled value exceeds the thresholds, turn on the appropriate LED
            if (sampled_value < low_threshold) {
                turn_on_led(AlarmLowPort[i], AlarmLowPin[i]);
                turn_off_led(AlarmHighPort[i], AlarmHighPin[i]);
            } else if (sampled_value > high_threshold) {
                turn_on_led(AlarmHighPort[i], AlarmHighPin[i]);
                turn_off_led(AlarmLowPort[i], AlarmLowPin[i]);
            } else {
                // If within the thresholds, turn off both LEDs
                turn_off_led(AlarmLowPort[i], AlarmLowPin[i]);
                turn_off_led(AlarmHighPort[i], AlarmHighPin[i]);
            }
        } else if (mode == LATCHED_MODE) {
            // In Latched Mode, the alarm stays on once it's triggered
            if (sampled_value < low_threshold) {
                turn_on_led(AlarmLowPort[i], AlarmLowPin[i]);
            } else if (sampled_value > high_threshold) {
                turn_on_led(AlarmHighPort[i], AlarmHighPin[i]);
            }
        } else if (mode == DISABLED_MODE) {
            // In Disabled Mode, no alarm LED should be on
            turn_off_led(AlarmLowPort[i], AlarmLowPin[i]);
            turn_off_led(AlarmHighPort[i], AlarmHighPin[i]);
        }
    }
}
