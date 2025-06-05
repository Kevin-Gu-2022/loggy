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
#define LCD_ChipSelect_Pin GPIO_PIN_3
#define LCD_ChipSelect_GPIO_Port GPIOA
#define JS_UP_Pin GPIO_PIN_0
#define JS_UP_GPIO_Port GPIOB
#define JS_UP_EXTI_IRQn EXTI0_IRQn
#define JS_LEFT_Pin GPIO_PIN_1
#define JS_LEFT_GPIO_Port GPIOB
#define JS_LEFT_EXTI_IRQn EXTI1_IRQn
#define JS_DOWN_Pin GPIO_PIN_2
#define JS_DOWN_GPIO_Port GPIOB
#define JS_DOWN_EXTI_IRQn EXTI2_IRQn
#define JS_PUSH_Pin GPIO_PIN_10
#define JS_PUSH_GPIO_Port GPIOB
#define JS_PUSH_EXTI_IRQn EXTI15_10_IRQn
#define JS_RIGHT_Pin GPIO_PIN_11
#define JS_RIGHT_GPIO_Port GPIOB
#define JS_RIGHT_EXTI_IRQn EXTI15_10_IRQn
#define Sampling_Pin GPIO_PIN_12
#define Sampling_GPIO_Port GPIOB
#define SD_CS_Pin GPIO_PIN_8
#define SD_CS_GPIO_Port GPIOA

/* USER CODE BEGIN Private defines */
#define SD_SPI_HANDLE hspi3

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
