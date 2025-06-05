/*
 * flash.c
 *
 * Always writes to the same flash location (EEPROM_START_ADDR),
 * overwriting previous data each time.
 *
 *  Created on: May 6, 2025
 *      Author: kevin
 */

#include "stm32l4xx_hal.h"
#include "state_control.h"
#include "SEGGER_RTT.h"
#include "flash.h"
#include <string.h>

#define EEPROM_PAGE_SIZE       2048U
#define FLASH_PAGE_NB          128
#define EEPROM_START_PAGE      (FLASH_PAGE_NB - 2)  // Page 126
#define EEPROM_START_ADDR      (FLASH_BASE + EEPROM_PAGE_SIZE * EEPROM_START_PAGE)
#define STRUCT_SIZE            sizeof(FlashState_t)

/**
 * Print the flash state
 */
void print_flash_state(const FlashState_t *state) {
	char buf[128];

	SEGGER_RTT_printf(0, "alarmLowThreshold: ");
	for (int i = 0; i < NUM_CHANNELS; i++) {
		sprintf(buf, "%.2f ", state->alarmLowThreshold[i]);
		SEGGER_RTT_WriteString(0, buf);
	}
	SEGGER_RTT_WriteString(0, "\n");

	SEGGER_RTT_printf(0, "alarmHighThreshold: ");
	for (int i = 0; i < NUM_CHANNELS; i++) {
		sprintf(buf, "%.2f ", state->alarmHighThreshold[i]);
		SEGGER_RTT_WriteString(0, buf);
	}
	SEGGER_RTT_WriteString(0, "\n");

	SEGGER_RTT_printf(0, "alarmMode: ");
	for (int i = 0; i < NUM_CHANNELS; i++) {
		sprintf(buf, "%u ", state->alarmMode[i]);
		SEGGER_RTT_WriteString(0, buf);
	}
	SEGGER_RTT_WriteString(0, "\n");

	SEGGER_RTT_printf(0, "unit: ");
	for (int i = 0; i < NUM_CHANNELS; i++) {
		sprintf(buf, "%u ", state->unit[i]);
		SEGGER_RTT_WriteString(0, buf);
	}
	SEGGER_RTT_WriteString(0, "\n");

	SEGGER_RTT_printf(0, "currentSource: ");
	for (int i = 0; i < NUM_CHANNELS; i++) {
		sprintf(buf, "%u ", state->currentSource[i]);
		SEGGER_RTT_WriteString(0, buf);
	}
	SEGGER_RTT_WriteString(0, "\n");

	SEGGER_RTT_printf(0, "sensorType: ");
	for (int i = 0; i < NUM_CHANNELS; i++) {
		sprintf(buf, "%u ", state->sensorType[i]);
		SEGGER_RTT_WriteString(0, buf);
	}
	SEGGER_RTT_WriteString(0, "\n");

	SEGGER_RTT_printf(0, "inputRange: ");
	for (int i = 0; i < NUM_CHANNELS; i++) {
		sprintf(buf, "%u ", state->inputRange[i]);
		SEGGER_RTT_WriteString(0, buf);
	}
	SEGGER_RTT_WriteString(0, "\n");
}



/**
 * Always points to fixed EEPROM address.
 */
HAL_StatusTypeDef EEPROM_Init(void) {
    return HAL_OK;
}

HAL_StatusTypeDef clear_flash_page() {
    HAL_FLASH_Unlock();

    FLASH_EraseInitTypeDef eraseInit;
    uint32_t pageError = 0;
    eraseInit.TypeErase = FLASH_TYPEERASE_PAGES;
    eraseInit.Page = EEPROM_START_PAGE;
    eraseInit.NbPages = 1;

    if (HAL_FLASHEx_Erase(&eraseInit, &pageError) != HAL_OK) {
        HAL_FLASH_Lock();
        SEGGER_RTT_printf(0, "Flash page clear error\n");
        return HAL_ERROR;
    }

    HAL_FLASH_Lock();
    SEGGER_RTT_printf(0, "Flash page clear success\n");
    return HAL_OK;
}

/**
 * Always writes to EEPROM_START_ADDR, overwriting existing data.
 * Erases the page before each write.
 */
HAL_StatusTypeDef EEPROM_WriteStruct(FlashState_t *data) {
	print_flash_state(data);
    if (clear_flash_page() != HAL_OK) {
        return HAL_ERROR;
    }

    HAL_FLASH_Unlock();

    for (uint32_t offset = 0; offset < STRUCT_SIZE; offset += 8) {
        uint64_t data64;
        memcpy(&data64, ((uint8_t *)data) + offset, 8);
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, EEPROM_START_ADDR + offset, data64) != HAL_OK) {
            HAL_FLASH_Lock();
            return HAL_ERROR;
        }
    }

    HAL_FLASH_Lock();

    // Verify write
    FlashState_t readback;
    memcpy(&readback, (void *)EEPROM_START_ADDR, STRUCT_SIZE);

    if (memcmp(&readback, data, STRUCT_SIZE) != 0) {
        SEGGER_RTT_printf(0, "Write failure\n");
        return HAL_ERROR;
    }


    SEGGER_RTT_printf(0, "Write success\n");
    return HAL_OK;
}

/**
 * Reads from the fixed EEPROM address.
 */
HAL_StatusTypeDef EEPROM_ReadLastStruct(FlashState_t *data) {
    uint64_t *p = (uint64_t *)EEPROM_START_ADDR;
    if (*p == 0xFFFFFFFFFFFFFFFF) {
        return HAL_ERROR;
    }
    SEGGER_RTT_printf(0, "Got here");
    memcpy(data, (void *)EEPROM_START_ADDR, STRUCT_SIZE);
    return HAL_OK;
}
