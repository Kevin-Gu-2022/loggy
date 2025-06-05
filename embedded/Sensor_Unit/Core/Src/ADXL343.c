/*
 * ADXL343.c
 *
 *  Created on: Apr 01, 2025
 *      Author: Lucy
 */

#include "ADXL343.h"

extern SPI_HandleTypeDef hspi1;

void ADXL343_Init() {
	if (Check_ADXL343() == 1) {
		ADXL343_WriteRegister(0x31, 0x08);	// Set range to ±2g, and sign extension enabled
		ADXL343_WriteRegister(0x2D, 0x08);  // Enable measurement mode
	}
}

// Function to check if the ADXL343 is connected by reading the Device ID (Register 0x00)
int Check_ADXL343() {
    uint8_t deviceID = ADXL343_ReadRegister(0x00);
    if (deviceID == 0xE5) {
    	SEGGER_RTT_printf(0,"detected ADXL343. Device ID: 0x%02X\n\r", deviceID);
        return 1;
    } else {
    	SEGGER_RTT_printf(0,"Failed to detect ADXL343. Device ID: 0x%02X\n\r", deviceID);
    	return 0;
    }
}

// Function to read a register from the ADXL343
uint8_t ADXL343_ReadRegister(uint8_t reg) {

    uint8_t txData = reg | 0x80;  // Set Read Bit
    uint8_t rxData = 0;

    ADXL343_Select();  // CS Low
    HAL_SPI_Transmit(&hspi1, &txData, 1, HAL_MAX_DELAY);
    HAL_SPI_Receive(&hspi1, &rxData, 1, HAL_MAX_DELAY);
    ADXL343_Deselect();  // CS High

    return rxData;
}

void ADXL343_Select() {
    HAL_GPIO_WritePin(CS_PORT, CS_PIN, GPIO_PIN_RESET);  // Pull CS Low (Active)
    HAL_Delay(1);
}

void ADXL343_Deselect() {
    HAL_GPIO_WritePin(CS_PORT, CS_PIN, GPIO_PIN_SET);    // Pull CS High (Inactive)
    HAL_Delay(1);
}

void ADXL343_WriteRegister(uint8_t reg, uint8_t value) {
    uint8_t txData[2] = {reg, value};

    ADXL343_Select();  // CS Low
    HAL_SPI_Transmit(&hspi1, txData, 2, HAL_MAX_DELAY);
    ADXL343_Deselect();  // CS High
}

float ADXL343_ReadAxis(uint8_t reg) {
	uint8_t lsb = ADXL343_ReadRegister(reg);
	uint8_t msb = ADXL343_ReadRegister(reg + 1);
	int16_t raw = (int16_t)((msb << 8) | lsb);  // Interpret as signed

	float g_val = (float)raw * 0.0039f * 9.80665f;  // Convert to m/s²

	char buffer[64];
	sprintf(buffer, "Reg 0x%02X: Raw = %d, Accel = %.2f m/s^2\n", reg, raw, g_val);
	SEGGER_RTT_WriteString(0, buffer);

	return g_val;
//    uint8_t lsb = ADXL343_ReadRegister(reg);
//    uint8_t msb = ADXL343_ReadRegister(reg + 1);
//    int16_t raw = (int16_t)((msb << 8) | lsb);  // Interpret as signed
//
//    float g_val = (float)raw * 0.0039f * 9.80665f;  // Convert to m/s²
//    int16_t accel_ms2 = (int16_t)(g_val);  // Round/truncate to integer m/s²
//
//        char buffer[64];
//        sprintf(buffer, "Reg 0x%02X: Raw = %d, Accel = %ld m/s^2\n", reg, raw, accel_ms2);
//        SEGGER_RTT_WriteString(0, buffer);
//
//    return accel_ms2;
}

void Read_ADXL343_Values(void) {
	float x = ADXL343_ReadAxis(0x32);
	float y = ADXL343_ReadAxis(0x34);
	float z = ADXL343_ReadAxis(0x36);
	float x_ms2 = x * 9.80665f;
	float y_ms2 = y * 9.80665f;
	float z_ms2 = z * 9.80665f;

	    char buffer[128];
	    sprintf(buffer, "ADXL343 Accel Data: X = %.3f m/s², Y = %.3f m/s², Z = %.3f m/s²\n", x_ms2, y_ms2, z_ms2);
	    SEGGER_RTT_WriteString(0, buffer);
}

