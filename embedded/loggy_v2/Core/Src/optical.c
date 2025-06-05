/*
 * optical.c
 *
 *  Created on: Mar 22, 2025
 *      Author: kevin
 */
#include "usart.h"
#include "optical.h"
#include "serial.h"
#include "ring_buffer.h"
#include "state_control.h"

#include "main.h"

#include <stdlib.h>  // For random num
#include <stdio.h>

#define DEBUGGING
#ifdef DEBUGGING
#include "SEGGER_RTT.h"
#endif /* DEBUGGING */

uint16_t sensorReading[NUM_CHANNELS];

// Initialise so that chip is in receiving mode
uint8_t opticalRecvMode = 1;

uint8_t opticRecvBuffer[sizeof(OpticalIn_t)];

RingBuffer opticRingBuffer;


static uint32_t tempTick = 0;



/* Global variable to keep track of when last tick of optical link was */
volatile uint32_t prevOpticRecvTick = 0;

volatile uint8_t opticDataFlag = 0;

/**
 * Convert ControlState_t to OpticalOut_t
 */
static OpticalOut_t control_to_optical_out(ControlState_t *currentState) {
	 OpticalOut_t output = {0};

	for (int i = 0; i < NUM_CHANNELS; i++) {
		output.alarmLowThreshold[i] = (currentState->alarmLowThreshold[i]) * 1000;
		output.alarmHighThreshold[i] = (currentState->alarmHighThreshold[i]) * 1000;
		output.alarmMode[i] = (currentState->alarmMode[i] & 0x03) | ((currentState->alarmOn[i] & 0x01) << 2);
		output.inputRange[i] = currentState->inputRange[i];
	}

	return output;

}

/**
 * Function to send optic data
 */
void send_optic_data() {

	set_optical_link_state(1);  // Optical state is working, so set it as active

	// Transmit some data through optic
//	char zeros[5] = {'A', 'A', 'A', 'A', 'B'}; // array of 10 zeros


	// Get stuff to send out
	OpticalOut_t output = control_to_optical_out(get_current_state());

	UART2_RX_DISABLE();
	HAL_UART_Transmit(&huart2, (uint8_t*)&output, sizeof(OpticalOut_t), HAL_MAX_DELAY);
	UART2_RX_ENABLE();

	opticDataFlag = 0;

//	update_state_from_optical(sensorReading);
}

/**
 * Function to receive optic data. Reads from ring buffer and updates the state.
 */
void process_recv_optic_data() {
	OpticalIn_t dataIn = {0};
	RingBuffer_Read(&opticRingBuffer, (uint8_t *)&dataIn, sizeof(OpticalIn_t));
	update_state_from_optical(dataIn.channelReading);
}

/**
 * Temporary function to generate random values and update the state
 */
static void generate_random_vals() {
	// Fill sensor reading array
	for (int i = 0; i < NUM_CHANNELS; i++) {
		// Generate a random 4 digit integer
		uint16_t randomValue = (rand() % 10000);
		sensorReading[i] = randomValue;

//			char readingBuff[10] = {0};
//			snprintf(readingBuff, sizeof(readingBuff), "%dV", sensorReading[i]);
//			SEGGER_RTT_printf(0, "%s ", readingBuff);
	}

//	update_state_from_optical(sensorReading);

}

/**
 * Initialise ring buffer and optical receival
 */
void optical_init() {
	RingBuffer_Init(&opticRingBuffer);
	HAL_UART_Receive_IT(&huart2, opticRecvBuffer, sizeof(opticRecvBuffer));
}

void process_optical_input() {


	if (HAL_GetTick() - tempTick > 500) {

//		generate_random_vals(); // TEMP
//		send_optic_data();

		tempTick = HAL_GetTick();

	}



	if (opticDataFlag) {

		// PLAN:
		// Move the send_optic_data() to ISR. Then process_recv here. Do not enable receival again until data is processed.
		process_recv_optic_data();
		send_optic_data();

		// Restart receival
		HAL_UART_Receive_IT(&huart2, opticRecvBuffer, sizeof(opticRecvBuffer));
	}

	// Timeout of 1 second for optical link
	if (HAL_GetTick() - prevOpticRecvTick > 1000) {
		set_optical_link_state(0);
	}


}
