/*
 * optical.c
 *
 * An FSM that handles the optical communication with the control unit.
 *
 *  Created on: Apr 27, 2025
 *      Author: kevin
 */
#include "ring_buffer.h"
#include "optical.h"
#include "usart.h"
#include "settings.h"
#include "sample.h"

#include <stdio.h>
#include <string.h>

#include "SEGGER_RTT.h"

#define SAMPLING_DELAY 500
#define RETRY_TIMEOUT 2000

/* States */
#define IDLE 0
#define SENDING 1
#define WAIT_RESPONSE 2
#define RECV_PROCESS 3
#define ERROR_STATE 4

char *stateNames[5] = {"IDLE", "SENDING", "WAIT", "RECV", "ERROR"};

static uint8_t currentState = IDLE;

RingBuffer ringBuffer;
uint8_t opticalRecvBuffer[sizeof(OpticalIn_t)];

volatile uint8_t recvResponseFlag = 0;

// Variable used to keep track of time between samples
static uint32_t responseTickCount = 0;

// Global array of sensor values
uint16_t sensorReading[NUM_CHANNELS];


void optical_init() {
	RingBuffer_Init(&ringBuffer);

	// Start control unit in IDLE state
	currentState = IDLE;
}

static void print_input(OpticalIn_t *opticalIn) {
	char buffer[200];  // For sprintf

	for (int i = 0; i < NUM_CHANNELS; i++) {
		// Format the alarmLowThreshold and alarmHighThreshold as floats
		float lowThreshold = opticalIn->alarmLowThreshold[i] / 1000.0f;
		float highThreshold = opticalIn->alarmHighThreshold[i] / 1000.0f;

		// Use sprintf to store the formatted string in buffer
		sprintf(buffer, "Channel %d: Low Threshold = %d, High Threshold = %d, Alarm Mode = %d, Input Range = %d\n",
				i, opticalIn->alarmLowThreshold[i], opticalIn->alarmHighThreshold[i],
				opticalIn->alarmMode[i] & 0x03, opticalIn->inputRange[i]);

		// Print out using RTT printf
		SEGGER_RTT_printf(0, "%s", buffer);
	}
}

void process_input() {
	OpticalIn_t dataIn = {0};
	RingBuffer_Read(&ringBuffer, (uint8_t*)&dataIn, sizeof(OpticalIn_t));
//	print_input(&dataIn);

	update_settings(&dataIn);

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
	}
}

void send_optic_data() {

	// Transmit some data through optic
//	char zeros[5] = {'A', 'A', 'A', 'A', 'B'}; // array of 10 zeros

	// Multiply each value of sensor_data by 1000
//	for (int i = 0; i < sizeof(sensor_data) / sizeof(sensor_data[0]); i++) {
//	    sensor_data[i] *= 1000;
//	}

	UART1_RX_DISABLE();
	HAL_UART_Transmit(&huart2, (uint8_t*)sensor_data, sizeof(OpticalOut_t), HAL_MAX_DELAY);
	UART1_RX_ENABLE();

}

void process_optical_data() {

	uint8_t nextState = ERROR_STATE;

//	SEGGER_RTT_printf(0, "Current State: %s\n", stateNames[currentState]);

	switch (currentState) {


		case IDLE:
			if (HAL_GetTick() - responseTickCount > SAMPLING_DELAY) {
				nextState = SENDING;
			} else {
				nextState = IDLE;
			}
			break;

		case SENDING:
//			generate_random_vals();
			sample_data();  // Will be sampling from sensors when function written
			send_optic_data();

			// Begin receiving ready for next state
			HAL_UART_Receive_IT(&huart2, opticalRecvBuffer, sizeof(OpticalIn_t));

			// Get current tick to start timer until next time we need to send
			responseTickCount = HAL_GetTick();

			nextState = WAIT_RESPONSE;
			break;

		case WAIT_RESPONSE:
			if (recvResponseFlag) {
				nextState = RECV_PROCESS;
			} else {
				// Data may be corrupted during transmission or the TOSLINK was disconnected/reconnected
				if (HAL_GetTick() - responseTickCount > RETRY_TIMEOUT) {
//					SEGGER_RTT_printf(0, "Warning: 500 ms exceeded\n");
//					SEGGER_RTT_printf(0, "Warning: 500 ms exceeded, ready for next sample, but still no response from control unit.\n");

					// Re-send
					nextState = SENDING;

					// May need to clear input buffer...
					break;
				}
				nextState = WAIT_RESPONSE;
			}
			break;

		case RECV_PROCESS:
			process_input();
			recvResponseFlag = 0;
			// If already exceeded 500 ms timer, then go straight to sending again
			if (HAL_GetTick() - responseTickCount > SAMPLING_DELAY) {
				nextState = SENDING;
			} else {
				nextState = IDLE;
			}
			break;

		default:
			SEGGER_RTT_printf(0, "Error state!\n");
			Error_Handler();
	}

	// Update state
	currentState = nextState;
}

