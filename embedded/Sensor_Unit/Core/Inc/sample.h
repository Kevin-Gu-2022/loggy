/*
 * sample.h
 *
 *  Created on: Apr 28, 2025
 *      Author: Lucy
 */

#ifndef INC_SAMPLE_H_
#define INC_SAMPLE_H_

#include "SEGGER_RTT.h"
#include <stdint.h>
#include "stm32l4xx_hal.h"
#include "ADXL343.h"
#include "NAU7802.h"
#include "LM335.h"
#include "MUX.h"
#include "optical.h"
#include "ADS1119.h"
#include "settings.h"

#define X_AXIS 0x32
#define Y_AXIS 0x34
#define Z_AXIS 0x36

#define VREF 2.048
#define BIT_SIZE 32768.0

// CHANNEL 1
#define CH1_H_PORT GPIOA
#define CH1_H_PIN GPIO_PIN_11

#define CH1_L_PORT GPIOA
#define CH1_L_PIN GPIO_PIN_10

// CHANNEL 2
#define CH2_H_PORT GPIOA
#define CH2_H_PIN GPIO_PIN_9

#define CH2_L_PORT GPIOA
#define CH2_L_PIN GPIO_PIN_8

// CHANNEL 3
#define CH3_H_PORT GPIOC
#define CH3_H_PIN GPIO_PIN_9

#define CH3_L_PORT GPIOC
#define CH3_L_PIN GPIO_PIN_8

// CHANNEL 4
#define CH4_H_PORT GPIOC
#define CH4_H_PIN GPIO_PIN_7

#define CH4_L_PORT GPIOC
#define CH4_L_PIN GPIO_PIN_6

// CHANNEL 5
#define CH5_H_PORT GPIOB
#define CH5_H_PIN GPIO_PIN_15

#define CH5_L_PORT GPIOB
#define CH5_L_PIN GPIO_PIN_14

// CHANNEL 6
#define CH6_H_PORT GPIOB
#define CH6_H_PIN GPIO_PIN_13

#define CH6_L_PORT GPIOB
#define CH6_L_PIN GPIO_PIN_12

// CHANNEL 7
#define CH7_H_PORT GPIOA
#define CH7_H_PIN GPIO_PIN_7

#define CH7_L_PORT GPIOC
#define CH7_L_PIN GPIO_PIN_4

// CHANNEL 8
#define CH8_H_PORT GPIOC
#define CH8_H_PIN GPIO_PIN_5

#define CH8_L_PORT GPIOB
#define CH8_L_PIN GPIO_PIN_0

// 8 sensor channels
extern int16_t sensor_data[8];  // Index 0-3: mux voltages, 4-6: accelerometer, 7: temperature

void read_mux();
void read_accelerometer();
void read_temperature();
void sample_data();

#endif /* SAMPLE_H_ */
