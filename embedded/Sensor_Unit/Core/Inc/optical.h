/*
 * optical.h
 *
 *  Created on: Apr 27, 2025
 *      Author: kevin
 */

#ifndef INC_OPTICAL_H_
#define INC_OPTICAL_H_

#include "ring_buffer.h"

#define NUM_CHANNELS 8

extern RingBuffer ringBuffer;
extern volatile uint8_t recvResponseFlag;

// Macros for disabling/enabling UART2
#define UART1_RX_DISABLE()   (CLEAR_BIT(USART1->CR1, USART_CR1_RE))  // Disable RX
#define UART1_RX_ENABLE()    (SET_BIT(USART1->CR1, USART_CR1_RE))    // Enable RX
#define UART1_TX_DISABLE()   (CLEAR_BIT(USART1->CR1, USART_CR1_TE))  // Disable TX
#define UART1_TX_ENABLE()    (SET_BIT(USART1->CR1, USART_CR1_TE))    // Enable TX

typedef struct {
	int16_t alarmLowThreshold[NUM_CHANNELS];
	int16_t alarmHighThreshold[NUM_CHANNELS];
	uint8_t alarmMode[NUM_CHANNELS];  // Alarm mode 3rd bit will be whether alarm is on/off
	uint8_t inputRange[NUM_CHANNELS];
} OpticalIn_t;

typedef struct {
	int16_t channelReading[NUM_CHANNELS];
} OpticalOut_t;

extern uint8_t opticalRecvBuffer[sizeof(OpticalIn_t)];


extern void optical_init();
extern void process_optical_data();


#endif /* INC_OPTICAL_H_ */
