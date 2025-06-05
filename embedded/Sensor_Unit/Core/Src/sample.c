/*
 * sample.c
 *
 *  Created on: Apr 28, 2025
 *      Author: Lucy
 */

#include "sample.h"

int16_t sensor_data[8] = {0};

void read_mux() {
	// Read multiplexer values
	for (int mux_channel = 0; mux_channel < 4; mux_channel++) {
		set_mux_channel(mux_channel);  // Set your GPIOs
		HAL_Delay(5);                   // Small settling delay
		sensor_data[mux_channel] = (ADS1119_ReadVoltage() / BIT_SIZE) * VREF * 1000;  // Read voltage
	}
}

void read_accelerometer() {
	sensor_data[4]= (int16_t)(ADXL343_ReadAxis(X_AXIS) * 1000.0f);
	sensor_data[5]= (int16_t)(ADXL343_ReadAxis(Y_AXIS) * 1000.0f);
	sensor_data[6]= (int16_t)(ADXL343_ReadAxis(Z_AXIS) * 1000.0f);
}

void read_temperature() {
    // Will overflow at around 32 degrees. Maybe subtract 20000 after and add 20000 on Control Unit side.
	sensor_data[7] = (int16_t)(Read_LM335_ADC() * 1000);
}

void sample_data() {
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);   // ON
	read_mux();
	read_accelerometer();
	read_temperature();
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET); // OFF
}
