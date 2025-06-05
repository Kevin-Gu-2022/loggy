/*
 * lcd.c
 *
 *  Created on: Mar 21, 2025
 *      Author: kevin
 */
#include <stdlib.h>  // For random num
#include <stdio.h>
#include <time.h>

#include <stdio.h>
#include "u8g2.h"
#include "u8g2_callbacks.h"
#include "lcd.h"
#include "serial.h"
#include "lcd_control.h"
#include "spi.h"
#include "state_control.h"

#define LCD_REFRESH_DELAY 400

#define DEBUGGING
#ifdef DEBUGGING
#include "SEGGER_RTT.h"
#endif /* DEBUGGING */

/* Globals */
u8g2_t u8g2;  // Global struct that contains all info used by u8g2 library

uint8_t softwareConnectedFlag = 0;

uint32_t lcdLastTick = 0;

struct CellPosition {
	uint8_t row;
	uint8_t col;
};

static struct CellPosition cellPos = {0};


#define WINDOW_SIZE 4
static int channels[NUM_CHANNELS] = {1, 2, 3, 4, 5, 6, 7, 8};
static int row_window_start = 0; // Starting index of the window
static int row_local_index = 0;  // Index within the window

static const uint8_t disconnectedIcon[] = {
            0b00001110,
            0b00010001,
            0b00000001,
            0b01000010,
            0b10000000,
            0b10001000,
            0b01110000,
        };

static const uint8_t connectedIcon[] = {
            0b00001110,
            0b00010001,
            0b00100001,
            0b01000010,
            0b10000100,
            0b10001000,
            0b01110000,
        };

static const uint8_t alarmOnIcon[] = {
            0b01000010,
            0b10011001,
            0b10111101,
            0b00111100,
			0b00111100,
            0b01111110,
            0b01111110,
            0b00000000,
        };

static const uint8_t alarmOffIcon[] = {
            0b00000000,
            0b00011000,
            0b00111100,
            0b00111100,
			0b00111100,
            0b01111110,
            0b01111110,
            0b00000000,
        };

static const uint8_t pcNoConnectIcon[] = {
            0b00000001,
            0b00000010,
            0b11111111,
            0b10001001,
			0b10010001,
            0b11111111,
            0b01011000,
            0b10111100,
        };

static const uint8_t pcConnectIcon[] = {
			0b00000000,
			0b00000000,
			0b11111111,
			0b10000001,
			0b10000001,
			0b11111111,
			0b00011000,
			0b00111100,
		};

static const uint8_t recordOffIcon[] = {
			0b00000000,
			0b00111100,
			0b01000010,
			0b01000010,
			0b01000010,
			0b01000010,
			0b00111100,
			0b00000000,
		};

static const uint8_t recordOnIcon[] = {
			0b00000000,
			0b00111100,
			0b01000010,
			0b01011010,
			0b01011010,
			0b01000010,
			0b00111100,
			0b00000000,
		};

uint8_t separators[] = {CHANNEL_READING_SEP_LINE,
						READING_RANGE_SEP_LINE,
						RANGE_ALARM_SEP_LINE,
						ALARM_STATE_SEP_LINE,
						STATE_LO_SEP_LINE,
						LO_HI_SEP_LINE,
						HI_RECORD_SEP_LINE,
						128}; // 8 columns

/**
 * Setup for the LCD using the u8g2 library
 */
void u8g2_setup(void) {
	// Full frame buffer mode
	u8g2_Setup_st7920_s_128x64_f(
        &u8g2,
        U8G2_R0,
        u8x8_byte_stm32_hal_spi,
        u8x8_gpio_and_delay_stm32
    );

    u8g2_InitDisplay(&u8g2);
    u8g2_SetPowerSave(&u8g2, 0);

    // Temporary random number setup
    srand(100);
}




/**
 * Zeroth row, zeroth col is the first reading
 * Returns 1 if error, 0 otherwise
 */
uint8_t highlight_cell(uint8_t row, uint8_t col) {

	// Limit row to max of 3 and col to max of 6
	col = (col > 7) ? 7 : col + 1;
	row = (row >= 4) ? 3 : row;

	uint8_t x_start = (col == 0) ? 0 : separators[col - 1] + 1;
	uint8_t x_end = separators[col] - 1;

	uint8_t x = x_start;
	uint8_t w = x_end - x_start + 1;
	uint8_t y = 17 + row * 11;

	u8g2_SetDrawColor(&u8g2, 2);  // XOR mode
	u8g2_DrawBox(&u8g2, x, y, w, 10);
	u8g2_SetDrawColor(&u8g2, 1);  // Normal draw mode
	return 0;

}

/**
 * Update globals every time this function is called with the supplied direction
 */
int navigate_lcd(uint8_t direction) {

	if (direction == DOWN) {
		row_local_index++;
		if (row_local_index >= WINDOW_SIZE) {
			// Shift window right if possible
			if (row_window_start + WINDOW_SIZE < NUM_CHANNELS) {
				row_window_start++;
				row_local_index = WINDOW_SIZE - 1;
			} else {
				// Can't move further
				row_local_index = WINDOW_SIZE - 1;
			}
		}

	} else if (direction == UP) {
		row_local_index--;
		if (row_local_index < 0) {
			// Shift window left if possible
			if (row_window_start > 0) {
				row_window_start--;
				row_local_index = 0;
			} else {
				// Can't move further
				row_local_index = 0;
			}
		}

	} else if (direction == RIGHT) {

		if (cellPos.col < 6) {
			cellPos.col++;
		}


	} else if (direction == LEFT) {

		if (cellPos.col > 0) {
			cellPos.col--;
		}

	}

	// Update global cell pos
	cellPos.row = row_window_start + row_local_index;

	return 0;
}

/**
 * Function that processes a button press
 */
void process_push() {
	if (cellPos.col == RECORDING_COL) {
		toggle_recording();
	} else if (cellPos.col == INPUT_RNG_COL) {
		toggle_input_range(cellPos.row);
	} else if (cellPos.col == ALARM_ON_COL) {
		unlatch_alarm(cellPos.row);
	}
}

static void setup_grid() {
	// Line underneath status bar
	u8g2_DrawHLine(&u8g2, 0, TOP_BANNER_BOTTOM_Y, 128);

	// Header
	u8g2_DrawStr(&u8g2, 0, TOP_BANNER_BOTTOM_Y + HEADER_HEIGHT, "C");
	u8g2_DrawStr(&u8g2, CHANNEL_READING_SEP_LINE + 2, TOP_BANNER_BOTTOM_Y + HEADER_HEIGHT, "Reading");
	u8g2_DrawStr(&u8g2, READING_RANGE_SEP_LINE + 2, TOP_BANNER_BOTTOM_Y + HEADER_HEIGHT, "Rg");
	u8g2_DrawStr(&u8g2, RANGE_ALARM_SEP_LINE + 2, TOP_BANNER_BOTTOM_Y + HEADER_HEIGHT, "A");
	u8g2_DrawStr(&u8g2, ALARM_STATE_SEP_LINE + 2, TOP_BANNER_BOTTOM_Y + HEADER_HEIGHT, "S");
	u8g2_DrawStr(&u8g2, HI_RECORD_SEP_LINE + 2, TOP_BANNER_BOTTOM_Y + HEADER_HEIGHT, "R");
	u8g2_DrawStr(&u8g2, STATE_LO_SEP_LINE + 2, TOP_BANNER_BOTTOM_Y + HEADER_HEIGHT, "Low");
	u8g2_DrawStr(&u8g2, LO_HI_SEP_LINE + 2, TOP_BANNER_BOTTOM_Y + HEADER_HEIGHT, "High");

	// Line underneath header
	u8g2_DrawHLine(&u8g2, 0, TOP_BANNER_BOTTOM_Y + HEADER_HEIGHT + 1, 125);

	// Draw the vertical lines
	for (int i = 0; i < 7; i++) {
//		u8g2_DrawVLine(&u8g2, separators[i], TOP_BANNER_BOTTOM_Y, SCREEN_MAX_ACTUAL_Y - TOP_BANNER_BOTTOM_Y);
	}

}


void draw_screen() {

//	receiveData();

	u8g2_ClearBuffer(&u8g2);

	setup_grid();

	char channelBuff[2] = {0};
	char readingBuff[10] = {0};

	uint8_t alarmOnFlag = 0;


	// get from state file
	ControlState_t *controlState = get_current_state();

	for (int i = 1; i < 5; i++) {
		// Use indexFirstReading to index into the correct value in array to display values
		uint8_t indexFirstReading = row_window_start + i - 1;
		// Bottom line of each iteration
		uint8_t bottomLine = HEADER_BOTTOM_BORDER_Y + i * (CHANNEL_HEIGHT + 1);
		u8g2_DrawHLine(&u8g2, 0, bottomLine, 128);

		/* Fill in the values from control state */

		// Get the unit and associated index into the array below
		char *unitStr[4] = {"V", "\xB0 C", "m|s2", "E"}; // Last letter is an internal failure
		uint8_t unitIndex = controlState->unit[indexFirstReading];

		snprintf(channelBuff, sizeof(channelBuff), "%d", channels[indexFirstReading]);
		u8g2_DrawStr(&u8g2, 0, bottomLine - 2, channelBuff);  // Write the channel number one px above line

		// Populate with values from control state
		snprintf(readingBuff, sizeof(readingBuff), "%.3f%s",
				controlState->channelReading[indexFirstReading],
				unitStr[unitIndex]);
		u8g2_DrawStr(&u8g2, CHANNEL_READING_SEP_LINE + 2, bottomLine - 2, readingBuff);

		// Draw either 1V or 10V depending on control state
		if (indexFirstReading < 4) {
			u8g2_DrawStr(&u8g2, READING_RANGE_SEP_LINE + 2, bottomLine - 2,
							controlState->inputRange[indexFirstReading] == ONE_VOLT ? "1V" : "10V");
		}

		// Determine the mode
		char *alarmMode[4] = {"A", "D", "L", "E"};  // Last letter is an internal failure
		uint8_t alarmModeIndex = controlState->alarmMode[indexFirstReading];
		u8g2_DrawStr(&u8g2, ALARM_STATE_SEP_LINE + 2, bottomLine - 2, alarmMode[alarmModeIndex]);

		// Draw if alarm active or not based on the 3rd bit in alarm status. Also update flag if any alarm was on
		u8g2_DrawStr(&u8g2, RANGE_ALARM_SEP_LINE + 2, bottomLine - 2,
				(controlState->alarmOn[indexFirstReading])
					? (alarmOnFlag = 1, "1")
					: "0");

		// Draw S if not recording, R if recording
		u8g2_DrawStr(&u8g2, HI_RECORD_SEP_LINE + 2, bottomLine - 2, controlState->recordingState ? "R" : "S");


		snprintf(readingBuff, sizeof(readingBuff), "%.3f%s",
				controlState->alarmLowThreshold[indexFirstReading], unitStr[unitIndex]);

		u8g2_DrawStr(&u8g2, STATE_LO_SEP_LINE + 2, bottomLine - 2, readingBuff);
		snprintf(readingBuff, sizeof(readingBuff), "%.3f%s",
				controlState->alarmHighThreshold[indexFirstReading], unitStr[unitIndex]);
		u8g2_DrawStr(&u8g2, LO_HI_SEP_LINE + 2, bottomLine - 2, readingBuff);
	}

	 // RTC Time
	 char timeStr[32];
	 get_timestamp_string(timeStr);

	 u8g2_DrawStr(&u8g2, 0, 7, timeStr);

	 // Draw alarm icons
	 u8g2_DrawXBM(&u8g2, SCREEN_MAX_X - (ICON_SIZE + 1) + 1, 0, ICON_SIZE, ICON_SIZE,
			 alarmOnFlag ? alarmOnIcon : alarmOffIcon);

	 // Optical link icon
	 u8g2_DrawXBM(&u8g2, SCREEN_MAX_X - 2 * (ICON_SIZE + 1) + 1, 0, ICON_SIZE, ICON_SIZE,
			 controlState->opticalLinkState ? connectedIcon : disconnectedIcon);

	 // Draw the corresponding PC connection icon
	 u8g2_DrawXBM(&u8g2, SCREEN_MAX_X - 3 * (ICON_SIZE + 1) + 1, 0, ICON_SIZE, ICON_SIZE,
			 (softwareConnectedFlag) ? pcConnectIcon : pcNoConnectIcon);

	 // Draw the corresponding recording icon
	 u8g2_DrawXBM(&u8g2, SCREEN_MAX_X - 4 * (ICON_SIZE + 1) + 1, 0, ICON_SIZE, ICON_SIZE,
			 (controlState->recordingState) ? recordOnIcon : recordOffIcon);

	highlight_cell(row_local_index, cellPos.col);

	// Send buffer to display
	u8g2_SendBuffer(&u8g2);

}


/**
 * Function to run in every cycle of loop
 */
void update_screen() {

	// Only run this every 400 ms
	if (HAL_GetTick() - lcdLastTick > LCD_REFRESH_DELAY) {
		u8g2_SetFont(&u8g2, u8g2_font_u8glib_4_tf );
		draw_screen();
		lcdLastTick = HAL_GetTick();
	}


}





