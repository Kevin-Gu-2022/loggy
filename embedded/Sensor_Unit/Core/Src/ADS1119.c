
#include "ADS1119.h"
#include "stm32l4xx_hal.h"  // Adjust if using another STM32 series

#define ADS1119_ADDR         (0x40 << 1)   // A1 = A0 = GND, 7-bit address shifted for HAL
#define ADS1119_RESET        0x06
#define ADS1119_START_SYNC   0x08
#define ADS1119_PWRDWN       0x02
#define ADS1119_RDATA        0x10
#define ADS1119_RREG         0x20
#define ADS1119_WREG         0x40

extern I2C_HandleTypeDef hi2c2;  // Ensure I2C2 is properly set up

int16_t ADS1119_ReadVoltage() {
	uint8_t config =
			(ADS1119_MUX_SINGLE_0 	<< 5) |
			(ADS1119_GAIN_1X 		<< 4) |
			(ADS1119DR330SPS 		<< 2) |
			(ADS1119_MODE_SINGLE    << 1) |
			ADS1119_VREF_INTERNAL;
	// Optional: Check device is ready
	    if (HAL_I2C_IsDeviceReady(&hi2c2, ADS1119_ADDR, 1, 100) != HAL_OK) {
	        SEGGER_RTT_WriteString(0, "ADS1119 Init: Device not ready!\r\n");
	        return 1;
	    }
    // Send RESET command
    uint8_t reset_cmd = ADS1119_RESET;
    if (HAL_I2C_Master_Transmit(&hi2c2, ADS1119_ADDR, &reset_cmd, 1, 100) != HAL_OK) {
        SEGGER_RTT_WriteString(0, "ADS1119 Init: Reset failed.\r\n");
        return 2;
    }

	uint8_t cmd[2] = {0x40, config};
	 if (HAL_I2C_Master_Transmit(&hi2c2, ADS1119_I2C_ADDR << 1, cmd, 2, HAL_MAX_DELAY) != HAL_OK) {
	        SEGGER_RTT_WriteString(0, "I2C Transmit failed (config)\n");
	        return -1;
	    } else {
	        //SEGGER_RTT_printf(0, "I2C Transmit OK: cmd = 0x%02X 0x%02X\n", cmd[0], cmd[1]);
	    }

	    HAL_Delay(1);

	    uint8_t start = 0x08;
	    if (HAL_I2C_Master_Transmit(&hi2c2, ADS1119_I2C_ADDR << 1, &start, 1, HAL_MAX_DELAY) != HAL_OK) {
	        SEGGER_RTT_WriteString(0, "I2C Transmit failed (start)\n");
	        return -2;
	    } else {
	        //SEGGER_RTT_WriteString(0, "I2C Transmit OK: start\n");
	    }

	    HAL_Delay(4);

	    uint8_t read_cmd = 0x10;
	    if (HAL_I2C_Master_Transmit(&hi2c2, ADS1119_I2C_ADDR << 1, &read_cmd, 1, HAL_MAX_DELAY) != HAL_OK) {
	        SEGGER_RTT_WriteString(0, "I2C Transmit failed (read cmd)\n");
	        return -3;
	    } else {
	        //SEGGER_RTT_WriteString(0, "I2C Transmit OK: read cmd\n");
	    }

	    uint8_t data[2] = {0, 0};
	    if (HAL_I2C_Master_Receive(&hi2c2, ADS1119_I2C_ADDR << 1, data, 2, HAL_MAX_DELAY) != HAL_OK) {
	        SEGGER_RTT_WriteString(0, "I2C Receive failed\n");
	        return -4;
	    } else {
	        //SEGGER_RTT_printf(0, "I2C Receive OK: 0x%02X 0x%02X\n", data[0], data[1]);
	    }

	    int16_t raw = (int16_t)((data[0] << 8) | data[1]);
	    //SEGGER_RTT_printf(0, "Raw value: %d (0x%04X)\n", raw, raw);
	return raw;
}
