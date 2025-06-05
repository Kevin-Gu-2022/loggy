/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    rtc.c
  * @brief   This file provides code for the configuration
  *          of the RTC instances.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "rtc.h"

/* USER CODE BEGIN 0 */

#include "SEGGER_RTT.h"
#include <time.h>
#include <stdio.h>
#include "state_control.h"

static uint32_t secondTicks = 0;

/* USER CODE END 0 */

RTC_HandleTypeDef hrtc;

/* RTC init function */
void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {0};

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */

  /* USER CODE END Check_RTC_BKUP */

  /** Initialize RTC and set the Time and Date
  */
  sTime.Hours = 0x13;
  sTime.Minutes = 0x31;
  sTime.Seconds = 0x0;
  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sTime.StoreOperation = RTC_STOREOPERATION_RESET;
  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  sDate.WeekDay = RTC_WEEKDAY_SUNDAY;
  sDate.Month = RTC_MONTH_MAY;
  sDate.Date = 0x27;
  sDate.Year = 0x25;

  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

void HAL_RTC_MspInit(RTC_HandleTypeDef* rtcHandle)
{

  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
  if(rtcHandle->Instance==RTC)
  {
  /* USER CODE BEGIN RTC_MspInit 0 */

  /* USER CODE END RTC_MspInit 0 */

  /** Initializes the peripherals clock
  */
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC;
    PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
      Error_Handler();
    }

    /* RTC clock enable */
    __HAL_RCC_RTC_ENABLE();
  /* USER CODE BEGIN RTC_MspInit 1 */

  /* USER CODE END RTC_MspInit 1 */
  }
}

void HAL_RTC_MspDeInit(RTC_HandleTypeDef* rtcHandle)
{

  if(rtcHandle->Instance==RTC)
  {
  /* USER CODE BEGIN RTC_MspDeInit 0 */

  /* USER CODE END RTC_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_RTC_DISABLE();
  /* USER CODE BEGIN RTC_MspDeInit 1 */

  /* USER CODE END RTC_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

/**
 * Get the total Unix time from RTC
 */
uint32_t get_unix_time_RTC() {
    RTC_TimeTypeDef sTime;
    RTC_DateTypeDef sDate;

    // Read time and date from RTC
    HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

    struct tm t;
    t.tm_year = sDate.Year + 100;  // Years since 1900
    t.tm_mon  = sDate.Month - 1;   // 0-11
    t.tm_mday = sDate.Date;
    t.tm_hour = sTime.Hours;
    t.tm_min  = sTime.Minutes;
    t.tm_sec  = sTime.Seconds;
    t.tm_isdst = 0;

    // Convert to UNIX timestamp (local time)
    return (uint32_t)mktime(&t);
}

/**
 * Set the RTC from a total seconds timestamp
 */
void set_RTC_from_unix_time(uint32_t unix_time) {
    time_t raw_time = (time_t)unix_time;
    struct tm *timeinfo = gmtime(&raw_time); // UTC time

    RTC_TimeTypeDef sTime = {0};
    RTC_DateTypeDef sDate = {0};

    sTime.Hours   = timeinfo->tm_hour;
    sTime.Minutes = timeinfo->tm_min;
    sTime.Seconds = timeinfo->tm_sec;
    sTime.TimeFormat = RTC_HOURFORMAT12_AM;
    sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    sTime.StoreOperation = RTC_STOREOPERATION_RESET;

    sDate.WeekDay = timeinfo->tm_wday + 1; // RTC uses 1=Monday
    sDate.Month   = timeinfo->tm_mon + 1;  // tm_mon is 0–11
    sDate.Date    = timeinfo->tm_mday;
    sDate.Year    = timeinfo->tm_year % 100; // tm_year = years since 1900

    HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
}

/**
 * Call this function to update RTC. Will only change the RTC if more than 1s between calls to reduce CPU load
 */
void update_rtc_time() {
	if (HAL_GetTick() - secondTicks > 1000) {
		ControlState_t *currentState = get_current_state();
		currentState->rtcTime = get_unix_time_RTC();
//		currentState->rtcTime++;
		secondTicks = HAL_GetTick();
	}
}

/**
 * Get the timestamps string
 */
void get_timestamp_string(char *timestamp) {
    RTC_DateTypeDef sDate;
    RTC_TimeTypeDef sTime;

    // Get the RTC current Time
    HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    // Get the RTC current Date
    HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

    // Format the timestamp string
    snprintf(timestamp, 100, "%04d-%02d-%02d %02d:%02d:%02d",
                 2000 + sDate.Year, sDate.Month, sDate.Date,
                 sTime.Hours, sTime.Minutes, sTime.Seconds);
}



/* USER CODE END 1 */
