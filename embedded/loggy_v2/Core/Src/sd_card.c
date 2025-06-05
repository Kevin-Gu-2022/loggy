/*
 * sd_card.c
 *
 *  Created on: Apr 01, 2025
 *      Author: Lucy
 */
#include "main.h"
#include "fatfs.h"
#include <stdio.h>
#include "sd_card.h"
#include <string.h>
#include "rtc.h"
#include "state_control.h"

#define DEBUGGING
#ifdef DEBUGGING
#include "SEGGER_RTT.h"
#endif /* DEBUGGING */

FATFS fs;
FIL fil;

static int recording_state = NOT_RECORDING;
static int file_state;

void update_sd_card() {
	ControlState_t *controlState = get_current_state();

	// Check if control unit is recording
	if (controlState->recordingState == RECORDING) {
		// Update recording state
		if (recording_state == NOT_RECORDING) {
			//SEGGER_RTT_printf(0, "recording\n\r");
			// write headers only on first press
			initialise_headers(&fil, controlState->unit);
			recording_state = RECORDING;
		}
		// TODO: Print new line of data
		write_data(&fil, controlState->channelReading);
	} else {
		// Not recording
		// If file is open close it

		if (file_state && recording_state == RECORDING) {
			//SEGGER_RTT_printf(0, "close file\n\r");
			f_close(&fil);
			file_state = CLOSED;
			//SEGGER_RTT_printf(0, "file_state: %d\n\r", file_state);

		}
	}
	recording_state = controlState->recordingState;
//	SEGGER_RTT_printf(0, "recording_state: %d\n\r", recording_state);
}

// Function to initialize the headers
void initialise_headers(FIL *fil, uint8_t unit[]) {
	char header_line[200] = {0};  // Enough space for full line
	    char channel_label[32];

	    // Start with timestamp
	    strcat(header_line, "Timestamp,");

	    for (int i = 0; i < NUMBER_CHANNELS; i++) {
	        if (i < 4) {
	            // For CH1–CH4: check units
	        	if (unit[i] == 1) {
	        		snprintf(channel_label, sizeof(channel_label), "CH%d (deg C)", i + 1);
	            } else {
	                snprintf(channel_label, sizeof(channel_label), "CH%d (V)", i + 1);
	            }
	        } else if (i < 7) {
	            snprintf(channel_label, sizeof(channel_label), "CH%d (m/s^2)", i + 1);
	        } else {
	            snprintf(channel_label, sizeof(channel_label), "CH%d (deg C)", i + 1);
	        }

	        strcat(header_line, channel_label);

	        if (i < NUMBER_CHANNELS - 1) {
	            strcat(header_line, ",");
	        } else {
	            strcat(header_line, "\n");  // End the header line
	        }
	    }
	    // Write to file
	    f_puts(header_line, fil);
}

// Function to get the current timestamp as a string

void get_timestamp(char *timestamp) {
    RTC_DateTypeDef sDate;
    RTC_TimeTypeDef sTime;

    // Get the RTC current Time
    HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    // Get the RTC current Date
    HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

    uint32_t ms = HAL_GetTick() % 1000; // Approximate current ms

    snprintf(timestamp, 100, "%04d-%02d-%02d_%02d-%02d-%02d.%03lu",
                 2000 + sDate.Year, sDate.Month, sDate.Date,
                 sTime.Hours, sTime.Minutes, sTime.Seconds,
                 ms);
}

void write_data(FIL *fil, float channelReading[]) {
    char data_line[100];
    char timestamp[30];

    get_timestamp(timestamp);  // Use time stamp function

    // Prepare the data line in CSV format
    snprintf(data_line, sizeof(data_line), "%s,", timestamp);  // Add timestamp
    for (int i = 0; i < NUMBER_CHANNELS; i++) {
        snprintf(data_line + strlen(data_line), sizeof(data_line) - strlen(data_line), "%d", channelReading[i]);
        if (i < NUMBER_CHANNELS - 1) {
            strcat(data_line, ",");  // Add comma between channel values
        }
    }

    // Write the data line to the file
    f_puts(data_line, fil);
    f_puts("\n", fil);  // Add newline after each data entry
}

int initialise_sd_card() {
	char SDPath[4];

	  // Link the driver for the SD card
	  FATFS_LinkDriver(&USER_Driver, SDPath);

	  if ((disk_initialize(0) == 0) &&          // SD card initialisation successful
	      (disk_status(0) & STA_NODISK) == 0 &&  // SD card is not missing
	      (f_mount(&fs, "", 1) == FR_OK)) {
		  HAL_Delay(500); // Wait until after initialization

		  RTC_DateTypeDef sDate;
		  RTC_TimeTypeDef sTime;
		  char filename[32];

		  // Get current date and time from RTC
		  HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
		  HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

		  snprintf(filename, sizeof(filename), "%04d-%02d-%02d_%02d-%02d-%02d.csv",
	           2000 + sDate.Year, sDate.Month, sDate.Date,
	           sTime.Hours, sTime.Minutes, sTime.Seconds);

		  FRESULT res = f_open(&fil, filename, FA_OPEN_ALWAYS | FA_WRITE);
		  if (res == FR_OK) {
		      file_state = OPEN;
		      SEGGER_RTT_printf(0, "sd card initialised\n\r");
		      return 1; // success
		  } else {
		      return 0;
		      SEGGER_RTT_printf(0, "sd card not initialised\n\r");

		  }
	  } else {
		  // No SD card connected
	      return 0;
	      SEGGER_RTT_printf(0, "sd card not initialised\n\r");

	   }

}

