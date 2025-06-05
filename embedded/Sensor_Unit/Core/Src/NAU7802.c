/*
 * nau7802.c
 *
 *  Created on: Apr 27, 2025
 *      Author: Lucy
 */

#include "NAU7802.h"

extern I2C_HandleTypeDef hi2c2;

void NAU7802_Init(void) {
	uint8_t regVal;

	// ----- Reset registers -----
	// Write 1 to RR bit => reset all registers
	regVal = (1 << 0);
	HAL_I2C_Mem_Write(&hi2c2, NAU7802_ADDR, 0x00, I2C_MEMADD_SIZE_8BIT, &regVal, 1, 100);
	HAL_Delay(1);  // small delay

	// Clear RR bit => exit reset
	regVal = 0x00;
	HAL_I2C_Mem_Write(&hi2c2, NAU7802_ADDR, 0x00, I2C_MEMADD_SIZE_8BIT, &regVal, 1, 100);
	HAL_Delay(1);

	// ----- Power up digital + analog -----
	// We want PUA=1 & PUD=1
	regVal = (1 << 2) | (1 << 1);  // 0b00000110
	HAL_I2C_Mem_Write(&hi2c2, NAU7802_ADDR, 0x00, I2C_MEMADD_SIZE_8BIT, &regVal, 1, 100);

	// Wait until power-up ready (PUR bit) is set
	do
	{
	    //NAU7802_ReadReg(NAU7802_REG_PU_CTRL, &regVal);
	    HAL_I2C_Mem_Read(&hi2c2, NAU7802_ADDR, 0x00, I2C_MEMADD_SIZE_8BIT, &regVal, 1, 100);
	} while((regVal & (1 << 3)) == 0);

	// ----- Configure gain & LDO if desired -----
	//    data = (NAU7802_LDO_3V3 | NAU7802_GAIN_X1);
	regVal = 0x00;
	HAL_I2C_Mem_Write(&hi2c2, NAU7802_ADDR, 0x01, I2C_MEMADD_SIZE_8BIT, &regVal, 1, 100);

	//
	// 4) Bypass the PGA
	//
	//   We only need to set bit 4 (PGA_BYPASS=1) in register 0x1B.
	//   The default contents of 0x1B is 0x00, so just set bit 4 => 0x10
	//
	regVal = 0x10;  // 0b0001_0000
	HAL_I2C_Mem_Write(&hi2c2, NAU7802_ADDR, 0x1B, I2C_MEMADD_SIZE_8BIT, &regVal, 1, 100);
}

int32_t Sample_ADC(void) {

	uint8_t buffer[3];
	uint8_t crBit;
	int32_t rawValue;

	// Wait for 'CR' bit (bit 5) in PU_CTRL => new data ready
	do
	{
	    HAL_I2C_Mem_Read(&hi2c2, NAU7802_ADDR, 0x00, I2C_MEMADD_SIZE_8BIT, &crBit, 1, 100);
	}
	while((crBit & (1 << 5)) == 0);

	// Burst read of 3 data bytes from ADCO_B2..B0
	HAL_I2C_Mem_Read(&hi2c2, NAU7802_ADDR,
					0x12,
	                I2C_MEMADD_SIZE_8BIT,
	                buffer, 3, 100);

	// Combine bytes into 24-bit signed number
	rawValue  = (int32_t)( (uint32_t)buffer[0] << 16 );
	rawValue |= (int32_t)( (uint32_t)buffer[1] <<  8 );
	rawValue |= (int32_t)( (uint32_t)buffer[2] <<  0 );

	// Sign-extend 24-bit
	if(rawValue & 0x00800000)
	{
	    rawValue |= 0xFF000000;
	}
//	SEGGER_RTT_printf(0, "%ld\n\r", rawValue);
	return rawValue;
}

// Function to calculate voltage from the ADC result
float calculateVoltage(int32_t rawValue) {
	return (6e-7f * (float)rawValue) + 1.6999f;
}

void printVoltage(float voltage) {
	char outputBuffer[50];
	sprintf(outputBuffer, "ADC Voltage: %.6f V\r\n", voltage);
	SEGGER_RTT_WriteString(0, outputBuffer);
}

