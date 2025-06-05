/*
 * optical.h
 *
 *  Created on: Mar 22, 2025
 *      Author: kevin
 */

#ifndef INC_READINGS_H_
#define INC_READINGS_H_

#include "stm32l4xx_hal.h"
#include "state_control.h"
#include "ring_buffer.h"

#define NUM_CHANNELS 8

// Macros for disabling/enabling UART2
#define UART2_RX_DISABLE()   (CLEAR_BIT(USART2->CR1, USART_CR1_RE))
#define UART2_RX_ENABLE()    (SET_BIT(USART2->CR1, USART_CR1_RE))
#define UART2_TX_DISABLE()   (CLEAR_BIT(USART2->CR1, USART_CR1_TE))
#define UART2_TX_ENABLE()    (SET_BIT(USART2->CR1, USART_CR1_TE))

typedef struct {
	int16_t alarmLowThreshold[NUM_CHANNELS];
	int16_t alarmHighThreshold[NUM_CHANNELS];
	uint8_t alarmMode[NUM_CHANNELS];  // Alarm mode 3rd bit will be whether alarm is on/off
	uint8_t inputRange[NUM_CHANNELS];
} OpticalOut_t;

typedef struct {
	int16_t channelReading[NUM_CHANNELS];
} OpticalIn_t;

extern uint8_t opticRecvBuffer[sizeof(OpticalIn_t)];
extern volatile uint32_t prevOpticRecvTick;
extern volatile uint8_t opticDataFlag;

extern RingBuffer opticRingBuffer;

extern void optical_init();
extern void process_optical_input();


#endif /* INC_READINGS_H_ */
