/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define DRDY_Pin GPIO_PIN_13
#define DRDY_GPIO_Port GPIOC
#define Channel_Select_A_Pin GPIO_PIN_1
#define Channel_Select_A_GPIO_Port GPIOC
#define Channel_Select_B_Pin GPIO_PIN_2
#define Channel_Select_B_GPIO_Port GPIOC
#define Channel_Select_C_Pin GPIO_PIN_3
#define Channel_Select_C_GPIO_Port GPIOC
#define SAMPLING_LED_Pin GPIO_PIN_5
#define SAMPLING_LED_GPIO_Port GPIOA
#define CH7_HIGH_Pin GPIO_PIN_7
#define CH7_HIGH_GPIO_Port GPIOA
#define CH7_LOW_Pin GPIO_PIN_4
#define CH7_LOW_GPIO_Port GPIOC
#define CH8_HIGH_Pin GPIO_PIN_5
#define CH8_HIGH_GPIO_Port GPIOC
#define CH8_LOW_Pin GPIO_PIN_0
#define CH8_LOW_GPIO_Port GPIOB
#define CH6_LOW_Pin GPIO_PIN_12
#define CH6_LOW_GPIO_Port GPIOB
#define CH6_HIGH_Pin GPIO_PIN_13
#define CH6_HIGH_GPIO_Port GPIOB
#define CH5_LOW_Pin GPIO_PIN_14
#define CH5_LOW_GPIO_Port GPIOB
#define CH5_HIGH_Pin GPIO_PIN_15
#define CH5_HIGH_GPIO_Port GPIOB
#define CH4_LOW_Pin GPIO_PIN_6
#define CH4_LOW_GPIO_Port GPIOC
#define CH4_HIGH_Pin GPIO_PIN_7
#define CH4_HIGH_GPIO_Port GPIOC
#define CH3_LOW_Pin GPIO_PIN_8
#define CH3_LOW_GPIO_Port GPIOC
#define CH3_HIGH_Pin GPIO_PIN_9
#define CH3_HIGH_GPIO_Port GPIOC
#define CH2_LOW_Pin GPIO_PIN_8
#define CH2_LOW_GPIO_Port GPIOA
#define CH2_HIGH_Pin GPIO_PIN_9
#define CH2_HIGH_GPIO_Port GPIOA
#define CH1_LOW_Pin GPIO_PIN_10
#define CH1_LOW_GPIO_Port GPIOA
#define CH1_HIGH_Pin GPIO_PIN_11
#define CH1_HIGH_GPIO_Port GPIOA
#define CS_Pin GPIO_PIN_6
#define CS_GPIO_Port GPIOB
#define RANGE_SWITCHING_Pin GPIO_PIN_9
#define RANGE_SWITCHING_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
