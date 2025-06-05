/*
 * lcd.h
 *
 *  Created on: Mar 21, 2025
 *      Author: kevin
 */

#ifndef INC_LCD_H_
#define INC_LCD_H_

#include "u8g2.h"

#define SCREEN_MAX_X 127
#define SCREEN_MAX_Y 63
#define SCREEN_MAX_ACTUAL_X 128
#define SCREEN_MAX_ACTUAL_Y 64

#define ICON_SIZE 8
#define ICON_PADDING 1
#define TOP_BANNER_BOTTOM_Y 8
#define HEADER_HEIGHT 7
#define CHANNEL_LABEL_WIDTH 29

#define HEADER_BOTTOM_BORDER_Y TOP_BANNER_BOTTOM_Y+HEADER_HEIGHT+1

// Vertical lines
#define CHANNEL_READING_SEP_LINE 4
#define READING_RANGE_SEP_LINE CHANNEL_READING_SEP_LINE+READING_GAP
#define RANGE_ALARM_SEP_LINE READING_RANGE_SEP_LINE+RNG_GAP
#define ALARM_STATE_SEP_LINE RANGE_ALARM_SEP_LINE+ALARM_GAP
#define STATE_LO_SEP_LINE ALARM_STATE_SEP_LINE+STATE_GAP
#define LO_HI_SEP_LINE STATE_LO_SEP_LINE+LO_GAP
#define HI_RECORD_SEP_LINE LO_HI_SEP_LINE+HI_GAP

//#define CHANNEL_GAP 9
// ONLY CHANGE THESE VALUES
#define READING_GAP 28
#define RNG_GAP 11
#define ALARM_GAP 5
#define STATE_GAP 6
#define LO_GAP 34
#define HI_GAP 34

#define CHANNEL_COL_WIDTH 10
#define CHANNEL_HEIGHT 10

#define RECORDING_COL 6
#define INPUT_RNG_COL 1
#define ALARM_ON_COL 2

// Global struct that contains all info used by u8g2 library
extern u8g2_t u8g2;

extern uint8_t softwareConnectedFlag;

// Initialisation function for LCD
extern void u8g2_setup();

extern void update_screen();

/* Functions that updates globals inside LCD file */
extern int navigate_lcd(uint8_t direction);
extern void process_push();

#endif /* INC_LCD_H_ */
