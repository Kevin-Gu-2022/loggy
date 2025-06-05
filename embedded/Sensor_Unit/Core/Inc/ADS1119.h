/*
 * ADS1119.h
 *
 *  Created on: May 24, 2025
 *      Author: Lucy
 */

#ifndef INC_ADS1119_H_
#define INC_ADS1119_H_

#include "SEGGER_RTT.h"
#include <stdint.h>
#include "stm32l4xx_hal.h"
#include "i2c.h"
#include "stdio.h"


#define ADS1119_I2C_ADDR 0b1000000

#define ADS1119_MUX_SINGLE_0 0b011
#define ADS1119_MUX_DIFFERENTIAL_0 0b000
#define ADS1119_GAIN_1X 0
#define ADS1119DR330SPS 0b10
#define ADS1119_MODE_SINGLE 0
#define ADS1119_VREF_INTERNAL 0

uint8_t ADS1119_Init(void);
int16_t ADS1119_ReadVoltage(void);

#endif /* ADS1119_H_ */
