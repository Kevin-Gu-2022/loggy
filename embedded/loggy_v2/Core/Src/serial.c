/*
 * serial.c
 *
 *  Created on: Mar 23, 2025
 *      Author: kevin
 */
#include "usart.h"
#include "lcd.h"
#include "serial.h"
#include "optical.h"
#include <stdio.h>
#include <string.h>
#include "ring_buffer.h"
#include "state_control.h"
#include "rtc.h"

#include "SEGGER_RTT.h"

/* How often UART will transmit data to the PC */
#define UART_TRANSMIT_DELAY 500
// Variable to keep track of when something was last transmitted.
static uint32_t uartTransmitTick = 0;

// Variable to keep track of how many batches of data have arrived
volatile uint8_t uartRecvFlag = 0;
// Last tick when data was received
volatile uint32_t prevRecvTick = 0;


/* Ring buffer for UART receive */
RingBuffer ringBuff;



static void print_serialIn_data(SerialDataInput_t *serialData) {
	SEGGER_RTT_printf(0, "\n-- SerialDataInput --\n");

    char floatBuf[128];
    for (int i = 0; i < NUM_CHANNELS; i++) {
        snprintf(floatBuf, sizeof(floatBuf), "L:%.2f H:%.2f",
                 serialData->alarmLowThreshold[i],
                 serialData->alarmHighThreshold[i]);

        SEGGER_RTT_printf(0,
            "CH%d %s S:0x%02X(%s) U:0x%02X R:%u\n",
            i,
            floatBuf,
            serialData->alarmStatus[i],
            (serialData->alarmStatus[i] & 0b00000100) ? "On" : "Off",
            serialData->unit[i],
            serialData->inputRange[i]
        );
    }

	SEGGER_RTT_printf(0, "====================\n\n");
}

static void printSerialDataOutputRTT(const SerialDataOutput_t* data) {
    SEGGER_RTT_printf(0, "rtcTime: %u\n", data->rtcTime);
    SEGGER_RTT_printf(0, "recordingState: %u\n", data->recordingState);
    SEGGER_RTT_printf(0, "opticalLinkState: %u\n", data->opticalLinkState);

    for (int i = 0; i < NUM_CHANNELS; i++) {
        SEGGER_RTT_printf(0, "CH%d:\n", i);
        SEGGER_RTT_printf(0, "  Reading: %d\n", data->channelReading[i]);
        SEGGER_RTT_printf(0, "  LowThreshold: %d\n", data->alarmLowThreshold[i]);
        SEGGER_RTT_printf(0, "  HighThreshold: %d\n", data->alarmHighThreshold[i]);
        SEGGER_RTT_printf(0, "  AlarmStatus: 0x%02X (Alarm %s)\n", data->alarmStatus[i],
                          (data->alarmStatus[i] & (1 << 2)) ? "ON" : "OFF");
        SEGGER_RTT_printf(0, "  Unit: %u\n", data->unit[i]);
        SEGGER_RTT_printf(0, "  InputRange: %u\n", data->inputRange[i]);
    }
}


/**
 * Send the current state to PC. Converts the ControlState_t struct into the SerialDataOutput_t struct.
 */
void transmit_state(ControlState_t *control) {

	SerialDataOutput_t output = {0};

    for (int i = 0; i < NUM_CHANNELS; i++) {

    	output.channelReading[i] = control->channelReading[i];

        // Thresholds
        output.alarmLowThreshold[i] = control->alarmLowThreshold[i];
        output.alarmHighThreshold[i] = control->alarmHighThreshold[i];

        // Pack alarmStatus: bits 0–1 = alarmMode, bit 2 = alarmOn
        output.alarmStatus[i] =
            (control->alarmMode[i] & 0x03) |
            ((control->alarmOn[i] & 0x01) << 2);

        // Pack unit byte: bits 0–1 = unit, bit 2 = currentSource, bit 3 = sensorType
        output.unit[i] =
            (control->unit[i] & 0x03) |
            ((control->currentSource[i] & 0x01) << 2) |
            ((control->sensorType[i] & 0x01) << 3);

        // Direct copy
        output.inputRange[i] = control->inputRange[i];
    }

    // Might lag system out...
    output.rtcTime = get_unix_time_RTC();
    output.recordingState = control->recordingState;
    output.opticalLinkState = control->opticalLinkState;

//    printSerialDataOutputRTT(&output);

    // Send out the final converted struct
	HAL_UART_Transmit(&huart1, (uint8_t*)&output, sizeof(SerialDataOutput_t), 100);
}



void check_serial() {

	if (uartRecvFlag) {

		while (uartRecvFlag > 0) {

			SerialDataInput_t dataIn = {0};

			RingBuffer_Read(&ringBuff, (uint8_t *)&dataIn, sizeof(SerialDataInput_t));
			update_state_from_serial(&dataIn);

			print_serialIn_data(&dataIn);

			uartRecvFlag--;
		}
	} else {
		check_alarm_no_input();
	}

	// Transmit something to PC every 500 ms
	if (HAL_GetTick() - uartTransmitTick > UART_TRANSMIT_DELAY) {

		transmit_state(get_current_state());

		uartTransmitTick = HAL_GetTick();
	}

}

