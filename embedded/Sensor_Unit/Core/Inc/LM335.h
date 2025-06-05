/*
 * LM335.h
 *
 *  Created on: Apr 01, 2025
 *      Author: Lucy
 */

#ifndef INC_LM335_H_
#define INC_LM335_H_

#include "SEGGER_RTT.h"
#include <stdint.h>
#include "stm32l4xx_hal.h"
#include "stdio.h"

#define VREF 3.28

float Read_LM335_ADC();
float Convert_ADC_To_Voltage(uint32_t adc_value);
float Convert_Voltage_To_Temperature(float vout);
void printTemperature(float temperature);
#endif /* LM335_H_ */
