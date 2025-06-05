/*
 * NAU7802.h
 *
 *  Created on: Apr 27, 2025
 *      Author: Lucy
 */

#ifndef INC_NAU7802_H_
#define INC_NAU7802_H_

#include <stdint.h>
#include "stm32l4xx_hal.h"
#include "SEGGER_RTT.h"
#include "stdio.h"

// NAU7802 Registers
#define NAU7802_ADDR (0x2A << 1)
#define REG_STATUS     0x00        // Status Register address
#define REG0x12        0x12        // Start of ADC result data
#define ADC_VREF        2

void NAU7802_Init(void);
int32_t Sample_ADC(void);
float calculateVoltage(int32_t rawValue);
void printVoltage(float voltage);

#endif /* NAU7802_H_ */
