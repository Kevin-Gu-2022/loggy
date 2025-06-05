/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    gpio.c
  * @brief   This file provides code for the configuration
  *          of all used GPIO pins.
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
#include "gpio.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/** Configure pins as
        * Analog
        * Input
        * Output
        * EVENT_OUT
        * EXTI
*/
void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, Channel_Select_A_Pin|Channel_Select_B_Pin|Channel_Select_C_Pin|CH7_LOW_Pin
                          |CH8_HIGH_Pin|CH4_LOW_Pin|CH4_HIGH_Pin|CH3_LOW_Pin
                          |CH3_HIGH_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, SAMPLING_LED_Pin|CH7_HIGH_Pin|CH2_LOW_Pin|CH2_HIGH_Pin
                          |CH1_LOW_Pin|CH1_HIGH_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, CH8_LOW_Pin|CH6_LOW_Pin|CH6_HIGH_Pin|CH5_LOW_Pin
                          |CH5_HIGH_Pin|RANGE_SWITCHING_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin : DRDY_Pin */
  GPIO_InitStruct.Pin = DRDY_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(DRDY_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : Channel_Select_A_Pin Channel_Select_B_Pin Channel_Select_C_Pin CH7_LOW_Pin
                           CH8_HIGH_Pin CH4_LOW_Pin CH4_HIGH_Pin CH3_LOW_Pin
                           CH3_HIGH_Pin */
  GPIO_InitStruct.Pin = Channel_Select_A_Pin|Channel_Select_B_Pin|Channel_Select_C_Pin|CH7_LOW_Pin
                          |CH8_HIGH_Pin|CH4_LOW_Pin|CH4_HIGH_Pin|CH3_LOW_Pin
                          |CH3_HIGH_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : SAMPLING_LED_Pin CH7_HIGH_Pin CH2_LOW_Pin CH2_HIGH_Pin
                           CH1_LOW_Pin CH1_HIGH_Pin */
  GPIO_InitStruct.Pin = SAMPLING_LED_Pin|CH7_HIGH_Pin|CH2_LOW_Pin|CH2_HIGH_Pin
                          |CH1_LOW_Pin|CH1_HIGH_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : CH8_LOW_Pin CH6_LOW_Pin CH6_HIGH_Pin CH5_LOW_Pin
                           CH5_HIGH_Pin RANGE_SWITCHING_Pin */
  GPIO_InitStruct.Pin = CH8_LOW_Pin|CH6_LOW_Pin|CH6_HIGH_Pin|CH5_LOW_Pin
                          |CH5_HIGH_Pin|RANGE_SWITCHING_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : CS_Pin */
  GPIO_InitStruct.Pin = CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(CS_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 2 */

/* USER CODE END 2 */
