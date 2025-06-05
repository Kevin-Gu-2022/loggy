/*
 * u8g2_callbacks.h
 *
 *  Created on: Mar 19, 2025
 *      Author: kevin
 */

#ifndef INC_U8G2_U8G2_CALLBACKS_H_
#define INC_U8G2_U8G2_CALLBACKS_H_

/* Callbacks used by u8g2 lib */
uint8_t u8x8_byte_stm32_hal_spi(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_gpio_and_delay_stm32(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);


#endif /* INC_U8G2_U8G2_CALLBACKS_H_ */
