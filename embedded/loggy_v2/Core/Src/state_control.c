/*
 * state_control.c
 *
 * This file will keep track of system state (including the alarms).
 *
 *  Created on: Apr 21, 2025
 *      Author: kevin
 */
#include <optical.h>
#include "state_control.h"
#include "serial.h"
#include "optical.h"
#include "lcd.h"
#include "flash.h"
#include "SEGGER_RTT.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "main.h"
#include "rtc.h"
#define DEBUGGING

static void store_current_state();


/* Struct to hold the system state */
static ControlState_t controlState = {0};

/* Bytes to track when the high/low threshold has been converted */
static uint8_t unitsPrev[NUM_CHANNELS] = {0};

ControlState_t *get_current_state() {
	return &controlState;
}

/**
 * Function to check and update alarms. Must be called before the alarmOn member is set so that the latching mode works.
 */
void static check_alarm(uint8_t i) {

	// Current alarm state
	uint8_t alarmState = (controlState.channelReading[i] < controlState.alarmLowThreshold[i]) ||
							(controlState.channelReading[i] > controlState.alarmHighThreshold[i]);

	switch(controlState.alarmMode[i]) {

		case ACTIVE:
			controlState.alarmOn[i] = alarmState;
			break;

		case LATCHING:
			controlState.alarmOn[i] |= alarmState;
			break;

		case DISABLED:
			controlState.alarmOn[i] = ALARM_OFF;
			break;

		default:
			SEGGER_RTT_printf(0, "FATAL: Unknown alarm mode %d\n", controlState.alarmMode[i]);
	}
}

/**
 * External function to check alarms
 */
void check_alarm_no_input() {
	for (int i = 0; i < NUM_CHANNELS; i++) {
		check_alarm(i);
	}
}

void toggle_recording() {
	controlState.recordingState ^= 1;
}

void toggle_input_range(uint8_t row) {
	controlState.inputRange[row] ^= 1;
	store_current_state();
}

void unlatch_alarm(uint8_t row) {
	// Only do un-latch if in latching mode
	if (controlState.alarmMode[row]) {
		// Only need to un-latch if alarm active
		if (controlState.alarmOn[row]) {
			controlState.alarmOn[row] = 0;
		}
	}
}


// Bidirectional conversion between voltage and temperature for:
//  - Pt1000 RTD (R₀ = 1000 Ω @ 0 °C, α = 0.00385 °C⁻¹)
//  - 10 kΩ@25 °C NTC (B = 3380 K)

#include <math.h>
//#include <float.h>

#ifndef NAN
#define NAN (0.0f/0.0f)
#endif

// Convert measured voltage (V) under constant current excitation (A) to RTD temperature (°C)
static float voltage_to_rtd_temperature(float voltage, float current_amps) {
    if (current_amps <= 0.0f) {
        return NAN;
    }
    float resistance = voltage / current_amps;   // R = V / I
    const float R0    = 1000.0f;                 // Pt1000: 1000 Ω at 0 °C
    const float alpha = 0.00385f;                // Temperature coefficient (3850 ppm/K)

    // Linear approximation: T = (R/R0 – 1) / α
    return (resistance - R0) / (alpha * R0);
}

// Convert temperature (°C) to the voltage you would measure under constant current excitation
static float temperature_to_rtd_voltage(float temp_celsius, float current_amps) {
    if (current_amps <= 0.0f) {
        return NAN;
    }
    const float R0    = 1000.0f;                 // Pt1000: 1000 Ω at 0 °C
    const float alpha = 0.00385f;                // Temperature coefficient

    // R = R0 · (1 + α · T), then V = I · R
    float resistance = R0 * (1.0f + alpha * temp_celsius);
    return resistance * current_amps;
}

// Convert measured voltage (V) under constant current excitation (A) to NTC temperature (°C)
static float voltage_to_ntc_temperature(float voltage, float current_amps) {
    if (current_amps <= 0.0f || voltage <= 0.0f) {
        return NAN;
    }
    float resistance = voltage / current_amps;   // R = V / I
    const float R25   = 10000.0f;                // NTC: 10 kΩ at 25 °C
    const float B     = 3380.0f;                 // B-value (B25/50), in K
    const float T25   = 25.0f + 273.15f;         // 298.15 K

    // B-parameter equation: 1/T = 1/T25 + (1/B)·ln(R/R25)
    float inv_T = (1.0f / T25) + (1.0f / B) * logf(resistance / R25);
    if (inv_T <= 0.0f) {
        return NAN;
    }
    float temp_k = 1.0f / inv_T;
    return temp_k - 273.15f;
}

// Convert temperature (°C) to the voltage you would measure under constant current excitation
static float temperature_to_ntc_voltage(float temp_celsius, float current_amps) {
    if (current_amps <= 0.0f) {
        return NAN;
    }
    const float R25 = 10000.0f;                  // NTC: 10 kΩ at 25 °C
    const float B   = 3380.0f;                   // B-value (B25/50)
    const float T25 = 25.0f + 273.15f;           // 298.15 K
    float temp_k    = temp_celsius + 273.15f;

    if (temp_k <= 0.0f) {
        return NAN;
    }
    // R = R25 · exp[B · (1/T – 1/T25)], then V = I · R
    float resistance = R25 * expf(B * ((1.0f / temp_k) - (1.0f / T25)));
    return resistance * current_amps;
}


/**
 * Converts readings only to temperature if the unit is set as such. Only run on the first 4 channels
 */
static void convert_readings(uint8_t index) {

	// Ignore if not first 4 channels
	if (index >= 4) {
		return;
	}

	if (controlState.unit[index] == DEG) {

		float currentMagnitude = (controlState.currentSource[index] == CURRENT_10UA) ? 10e-6f : 200e-6f;
		uint8_t sensorType = controlState.sensorType[index];

		// Function pointer to conversion function
		float (*convert_voltage_to_temp)(float, float) = NULL;

		// Define the conversion function
		switch (sensorType) {
			case SENSOR_THERM:
				convert_voltage_to_temp = voltage_to_ntc_temperature;
				break;
			case SENSOR_RTD:
				convert_voltage_to_temp = voltage_to_rtd_temperature;
				break;
			default:
				SEGGER_RTT_printf(0, "ERROR: Unknown sensor type %d on channel %d\n", sensorType, index);
				return;
		}

		// Convert current reading
		controlState.channelReading[index] =
			convert_voltage_to_temp(controlState.channelReading[index], currentMagnitude);
	}
}


static void convert_thresholds(uint8_t index) {

	// Ignore if not first 4 channels
	if (index >= 4) {
		return;
	}

	// Check if units were changed
	uint8_t unitsChanged = 0;  // Local flag to keep track if units changed
	if (controlState.unit[index] != unitsPrev[index]) {
		unitsChanged = 1;
	}

	float currentMagnitude = (controlState.currentSource[index] == CURRENT_10UA) ? 10e-6f : 200e-6f;
	uint8_t sensorType = controlState.sensorType[index];

	if (controlState.unit[index] == DEG) {

		// Function pointer to conversion function
		float (*convert_voltage_to_temp)(float, float) = NULL;

		// Define the conversion function
		switch (sensorType) {
			case SENSOR_THERM:
				convert_voltage_to_temp = voltage_to_ntc_temperature;
				break;
			case SENSOR_RTD:
				convert_voltage_to_temp = voltage_to_rtd_temperature;
				break;
			default:
				SEGGER_RTT_printf(0, "ERROR: Unknown sensor type %d on channel %d\n", sensorType, index);
				return;
		}

		// Convert alarm thresholds if units changed
		if (unitsChanged) {
			controlState.alarmLowThreshold[index] =
				convert_voltage_to_temp(controlState.alarmLowThreshold[index], currentMagnitude);
			controlState.alarmHighThreshold[index] =
				convert_voltage_to_temp(controlState.alarmHighThreshold[index], currentMagnitude);

			if (controlState.alarmLowThreshold[index] > controlState.alarmHighThreshold[index]) {
				SEGGER_RTT_printf(0, "Error in voltage to temperature conversion. Setting to defaults.");
				controlState.alarmLowThreshold[index] = 20.0f;
				controlState.alarmHighThreshold[index] = 21.0f;
			}
			controlState.alarmLowThreshold[index] =
					isnan(controlState.alarmLowThreshold[index]) ? controlState.alarmLowThreshold[index] : 0;
			controlState.alarmHighThreshold[index] =
					isnan(controlState.alarmHighThreshold[index]) ? controlState.alarmHighThreshold[index] : 0;
		}

		// Keep track of previous unit (conversions necessary only once)
		unitsPrev[index] = DEG;


	} else if (controlState.unit[index] == VOLT) {
		// Keep track of previous unit
		unitsPrev[index] = VOLT;

		// Need to convert the high/low thresholds once when moving back from temperature to voltage
		if (unitsChanged) {
			// Function pointer to conversion function
			float (*convert_temp_to_voltage)(float, float) = NULL;

			// Define the conversion function
			switch (sensorType) {
				case SENSOR_THERM:
					convert_temp_to_voltage = temperature_to_ntc_voltage;
					break;
				case SENSOR_RTD:
					convert_temp_to_voltage = temperature_to_rtd_voltage;
					break;
				default:
					SEGGER_RTT_printf(0, "ERROR: Invalid sensor type for channel %d\n", index);
					return;
			}

			// Convert the alarm thresholds
			controlState.alarmLowThreshold[index] = convert_temp_to_voltage(controlState.alarmLowThreshold[index],
					currentMagnitude);
			controlState.alarmHighThreshold[index] = convert_temp_to_voltage(controlState.alarmHighThreshold[index],
					currentMagnitude);

			if (controlState.alarmLowThreshold[index] > controlState.alarmHighThreshold[index]) {
				SEGGER_RTT_printf(0, "Error in temperature to voltage conversion. Setting to defaults.");
				controlState.alarmLowThreshold[index] = 0.0f;
				controlState.alarmHighThreshold[index] = 2.0f;
			}
			controlState.alarmLowThreshold[index] =
					isnan(controlState.alarmLowThreshold[index]) ? controlState.alarmLowThreshold[index] : 0;
			controlState.alarmHighThreshold[index] =
					isnan(controlState.alarmHighThreshold[index]) ? controlState.alarmHighThreshold[index] : 0;

		}

	}
}


/**
 * Function to call from serial file to update the state
 */
void update_state_from_serial(SerialDataInput_t *input) {

	if (input->rtcTime > 0) {
		// Set the RTC hardware time and update internal control state variable
		set_RTC_from_unix_time(input->rtcTime);
		controlState.rtcTime = input->rtcTime;
	}
	// Bit 0 is recording state
	controlState.recordingState = (input->recordingState) & 0x01;

	// Bit 1 is the serial connection state
	softwareConnectedFlag = !!((input->recordingState) & 0x02);

	for (int i = 0; i < NUM_CHANNELS; i++) {

		controlState.alarmLowThreshold[i] = input->alarmLowThreshold[i];
		controlState.alarmHighThreshold[i] = input->alarmHighThreshold[i];

		uint8_t alarm = input->alarmStatus[i];
		controlState.alarmMode[i] = alarm & 0x03;
		check_alarm(i);
		controlState.alarmOn[i] = (alarm >> 2) & 0x01;

		uint8_t unit = input->unit[i];

		// Only update the lower 4
		if (i < 4) {
			controlState.unit[i] = unit & 0x03;
			controlState.currentSource[i] = (unit >> 2) & 0x01;
			controlState.sensorType[i] = (unit >> 3) & 0x01;
		}

		controlState.inputRange[i] = input->inputRange[i];

		// Update the alarm thresholds
//		convert_thresholds(i);
	}

	store_current_state();

}

/**
 * Function to call from optical file to update the state.
 */
void update_state_from_optical(int16_t *readings) {

	for (int i = 0; i < NUM_CHANNELS; i++) {
		controlState.channelReading[i] = ((float)readings[i]) / 1000;
		// Convert readings if necessary
		convert_readings(i);
		// Check if any alarms activated
		check_alarm(i);
	}
}

/**
 * Update optical link state
 */
void set_optical_link_state(uint8_t state) {
	controlState.opticalLinkState = state;
}


static void store_current_state() {
    FlashState_t flash = {0};

    // Copy the values from control state into flash state struct
    memcpy(flash.alarmLowThreshold,  controlState.alarmLowThreshold,  sizeof(flash.alarmLowThreshold));
    memcpy(flash.alarmHighThreshold, controlState.alarmHighThreshold, sizeof(flash.alarmHighThreshold));
    memcpy(flash.alarmMode,          controlState.alarmMode,          sizeof(flash.alarmMode));

    // Only store first 4 channels
    memcpy(flash.unit,               controlState.unit,               sizeof(flash.unit));
    memcpy(flash.currentSource,      controlState.currentSource,      sizeof(flash.currentSource));
    memcpy(flash.sensorType,         controlState.sensorType,         sizeof(flash.sensorType));
    memcpy(flash.inputRange,         controlState.inputRange,         sizeof(flash.inputRange));

    // Store into flash memory
    HAL_StatusTypeDef ret = EEPROM_WriteStruct(&flash);
    SEGGER_RTT_printf(0, "Write status: %d\n", ret);
}

/**
 * Only run this the first time the chip is flashed to load the flash memory with a base struct.
 */
static void first_time_flash_setup() {

	// Temporarily fill control state struct with some initial values.
	float lowVal[8] = {-0.2, 0, 0, 0, 0, 0, 0, 0};
	memcpy(controlState.alarmLowThreshold, lowVal, sizeof(lowVal));
	float highVal[8] = {0.2, 0.3, 0.4, 0.6, 0.4, 0.6, 0.01, 0.07};
	memcpy(controlState.alarmHighThreshold, highVal, sizeof(lowVal));
	controlState.inputRange[2] = TEN_VOLT;
	controlState.alarmMode[0] = 0x00;
	controlState.alarmMode[1] = 0x02;
	controlState.recordingState = 0;
	controlState.opticalLinkState = 0;
	controlState.rtcTime = get_unix_time_RTC();

	uint8_t initialUnits[NUM_CHANNELS] = {VOLT, VOLT, VOLT, VOLT, MS2, MS2, MS2, DEG};
	for (int i = 0; i < NUM_CHANNELS; i++) {
		controlState.unit[i] = initialUnits[i];
	}

	// First time flash, there will be errors if there is no base state.
	clear_flash_page();
	store_current_state();

}

void print_control_state(const ControlState_t *state) {
    char buf[64];
    SEGGER_RTT_printf(0, "RTC:%lu Rec:%u Link:%u\n", state->rtcTime, state->recordingState, state->opticalLinkState);

    for (int i = 0; i < NUM_CHANNELS; i++) {
        SEGGER_RTT_printf(0, "CH%d ", i);

        snprintf(buf, sizeof(buf), "R:%.2f ", state->channelReading[i]);
		SEGGER_RTT_WriteString(0, buf);

		snprintf(buf, sizeof(buf), "L:%.2f ", state->alarmLowThreshold[i]);
		SEGGER_RTT_WriteString(0, buf);

		snprintf(buf, sizeof(buf), "H:%.2f ", state->alarmHighThreshold[i]);
		SEGGER_RTT_WriteString(0, buf);

        SEGGER_RTT_printf(0, "M:%u O:%u U:%u S:%u T:%u IR:%u\n",
                          state->alarmMode[i],
                          state->alarmOn[i],
                          state->unit[i],
                          state->currentSource[i],
                          state->sensorType[i],
                          state->inputRange[i]);
    }
}

/**
 * Validate the control state
 */
void validate_control_state() {
	uint8_t error = 0;
	for (int i = 0; i < NUM_CHANNELS; i++) {
		if (controlState.alarmLowThreshold[i] > controlState.alarmHighThreshold[i]) {
			error = 1;
		}
		switch (controlState.unit[i]) {

			case VOLT:
				if ((controlState.alarmLowThreshold[i] < -10) || (controlState.alarmHighThreshold[i] > 10)) {
					error = 1;
				}
				break;

			case DEG:
				if ((controlState.alarmLowThreshold[i] < -10) || (controlState.alarmHighThreshold[i] > 45)) {
					error = 1;
				}
				break;

			case MS2:
				if ((controlState.alarmLowThreshold[i] < -20) || (controlState.alarmHighThreshold[i] > 20)) {
					error = 1;
				}
				break;

		}
	}

	for (int j = 0; j < 4; j++) {
		if (controlState.unit[0] == MS2) {
			error = 1;
		}
	}


	if (error) {
		first_time_flash_setup();
	}

}



/**
 * Initialise the control state from EEPROM
 */
void init_control_state() {

//	 first_time_flash_setup();

	 // Read the state from flash
	 FlashState_t flashState = {0};
	 if (EEPROM_ReadLastStruct(&flashState) == HAL_ERROR) {
		 SEGGER_RTT_printf(0, "ERROR: Read from flash failed!");
		 first_time_flash_setup();
//		 Error_Handler();
	 } else {
		memcpy(controlState.alarmLowThreshold,  flashState.alarmLowThreshold,  sizeof(flashState.alarmLowThreshold));
		memcpy(controlState.alarmHighThreshold, flashState.alarmHighThreshold, sizeof(flashState.alarmHighThreshold));
		memcpy(controlState.alarmMode,          flashState.alarmMode,          sizeof(flashState.alarmMode));
		memcpy(controlState.unit,               flashState.unit,               sizeof(flashState.unit));
		memcpy(controlState.currentSource,      flashState.currentSource,      sizeof(flashState.currentSource));
		memcpy(controlState.sensorType,         flashState.sensorType,         sizeof(flashState.sensorType));
		memcpy(controlState.inputRange,         flashState.inputRange,         sizeof(flashState.inputRange));
		controlState.rtcTime = get_unix_time_RTC();

		controlState.recordingState = 0;  // Recording state defaults to off on power on
		controlState.opticalLinkState = 0;  // Optical link 0 on startup

	 }

	 validate_control_state();

	print_control_state(&controlState);

}
