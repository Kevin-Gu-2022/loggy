/*
 * lcd_control.c
 *
 *  Created on: Apr 11, 2025
 *      Author: kevin
 */
#include "lcd_control.h"
#include "lcd.h"
#include "gpio.h"

#define DEBUGGING
#include "SEGGER_RTT.h"

static volatile uint32_t prevButtonTick = 0;



void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if (HAL_GetTick() - prevButtonTick > 500) {

		switch(GPIO_Pin)
		    {
		        case JS_LEFT_Pin:
		        	 SEGGER_RTT_printf(0, "Left activated!\n");
					 navigate_lcd(LEFT);
		            break;
		        case JS_RIGHT_Pin:
		        	SEGGER_RTT_printf(0, "Right activated\n");
		        	navigate_lcd(RIGHT);
		            break;
		        case JS_UP_Pin:
		        	SEGGER_RTT_printf(0, "Up activated\n");
					 navigate_lcd(UP);
		            break;
		        case JS_DOWN_Pin:
		        	SEGGER_RTT_printf(0, "Down activated\n");
					 navigate_lcd(DOWN);
					break;
		        case JS_PUSH_Pin:
		        	SEGGER_RTT_printf(0, "Push activated\n");
		        	process_push();
					break;

		        default:
		            break;
		    }

		//  Update previous tick
		prevButtonTick = HAL_GetTick();

	}



}
