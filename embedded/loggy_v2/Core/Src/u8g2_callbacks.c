/*
 * u8g2_callbacks.c
 *
 *  Created on: Mar 19, 2025
 *      Author: kevin
 */

#include "spi.h"
#include "u8g2.h"
#include "lcd.h"

uint8_t u8x8_byte_stm32_hal_spi(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr) {
    switch (msg) {
        case U8X8_MSG_BYTE_INIT:
            // SPI already initialized in CubeMX, nothing to do here
            return 1;

        case U8X8_MSG_BYTE_SEND:
            // Send data over SPI
            if (HAL_SPI_Transmit(&hspi1, (uint8_t *)arg_ptr, arg_int, HAL_MAX_DELAY) != HAL_OK) {
                return 0;  // Transmission failed
            }
            return 1;

        case U8X8_MSG_BYTE_START_TRANSFER:
            // CS LOW - Start SPI communication
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET);
            return 1;

        case U8X8_MSG_BYTE_END_TRANSFER:
            // CS HIGH - End SPI communication
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);
            return 1;

        default:
            return 0;  // Unsupported operation
    }
}

uint8_t u8x8_gpio_and_delay_stm32(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr) {
    switch (msg) {
        case U8X8_MSG_DELAY_MILLI:
            HAL_Delay(arg_int);  // Millisecond delay
            return 1;

        case U8X8_MSG_DELAY_100NANO:  // Add this case for better timing
                    for(uint16_t n = 0; n < arg_int; n++)
                        __NOP();
                    return 1;

        case U8X8_MSG_DELAY_NANO:
            __NOP();  // No operation (minimal delay for nanoseconds)
            return 1;

        case U8X8_MSG_GPIO_AND_DELAY_INIT:
            // GPIOs are initialized in CubeMX, nothing to do here
            return 1;

        case U8X8_MSG_GPIO_CS:
            // Control Chip Select (CS) - Adjust the GPIO pin as per your setup
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, arg_int);
            return 1;

        case U8X8_MSG_GPIO_RESET:
            // Control Reset (RST) - Adjust the GPIO pin as per your setup
            // HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, arg_int);
            return 1;

        case U8X8_MSG_GPIO_DC:
            // Data/Command (DC) Pin - Adjust according to your setup
            // HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, arg_int ? GPIO_PIN_SET : GPIO_PIN_RESET);
            return 1;

        default:
            return 0;  // Unsupported operation
    }
}
