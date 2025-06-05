/*
 * ADXL343.h
 *
 *  Created on: Apr 01, 2025
 *      Author: Lucy
 */

#ifndef INC_ADXL343_H_
#define INC_ADXL343_H_

#include "SEGGER_RTT.h"
#include <stdint.h>
#include "stm32l4xx_hal.h"
#include "spi.h"

#define CS_PIN GPIO_PIN_6
#define CS_PORT GPIOB

void ADXL343_Init();
int Check_ADXL343();
uint8_t ADXL343_ReadRegister(uint8_t reg);
void ADXL343_Select();
void ADXL343_Deselect();
void ADXL343_WriteRegister(uint8_t reg, uint8_t value);
float ADXL343_ReadAxis(uint8_t reg);
void Read_ADXL343_Values(void);
float ADXL343_ReadXAxis(void);
float ADXL343_ReadYAxis(void);
float ADXL343_ReadZAxis(void);

#endif /* ADXL343_H_ */
