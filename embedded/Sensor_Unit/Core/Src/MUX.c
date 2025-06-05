/*
 * MUX.C
 *
 *  Created on: Apr 28, 2025
 *      Author: Lucy
 */

#include "MUX.h"

void set_mux_channel(int channel) {
	// A = PC1
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, (channel & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);

	// B = PC2
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, (channel & 0x02) ? GPIO_PIN_SET : GPIO_PIN_RESET);

	// C = PC3
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, (channel & 0x04) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}
