/*
 * MUX.h
 *
 *  Created on: Apr 28, 2025
 *      Author: Lucy
 */

#ifndef INC_MUX_H_
#define INC_MUX_H_

#include "SEGGER_RTT.h"
#include <stdint.h>
#include "stm32l4xx_hal.h"

#define CH1 0
#define CH2 1
#define CH3 2
#define CH4 3

void set_mux_channel(int mux_channel);


#endif /* MUX_H_ */
