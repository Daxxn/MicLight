/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include "stm32f0xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "MainWrapper.h"
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

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED6_Pin GPIO_PIN_0
#define LED6_GPIO_Port GPIOA
#define LED7_Pin GPIO_PIN_1
#define LED7_GPIO_Port GPIOA
#define BRIGHTNESS_Pin GPIO_PIN_2
#define BRIGHTNESS_GPIO_Port GPIOA
#define LED1_Pin GPIO_PIN_3
#define LED1_GPIO_Port GPIOA
#define MAX_BRIGHT_IND_Pin GPIO_PIN_4
#define MAX_BRIGHT_IND_GPIO_Port GPIOA
#define UNDER_PWR_IND_Pin GPIO_PIN_5
#define UNDER_PWR_IND_GPIO_Port GPIOA
#define LED2_Pin GPIO_PIN_6
#define LED2_GPIO_Port GPIOA
#define LED3_Pin GPIO_PIN_7
#define LED3_GPIO_Port GPIOA
#define LED4_Pin GPIO_PIN_0
#define LED4_GPIO_Port GPIOB
#define LED5_Pin GPIO_PIN_1
#define LED5_GPIO_Port GPIOB
#define STATUS_IND_Pin GPIO_PIN_2
#define STATUS_IND_GPIO_Port GPIOB
#define LED8_Pin GPIO_PIN_10
#define LED8_GPIO_Port GPIOB
#define ENABLE_Pin GPIO_PIN_11
#define ENABLE_GPIO_Port GPIOB
#define ENABLE_EXTI_IRQn EXTI4_15_IRQn
#define I2C_INT_Pin GPIO_PIN_12
#define I2C_INT_GPIO_Port GPIOB
#define I2C_INT_EXTI_IRQn EXTI4_15_IRQn
#define USB_FAULT_Pin GPIO_PIN_15
#define USB_FAULT_GPIO_Port GPIOB
#define MAX_BRIGHT_Pin GPIO_PIN_9
#define MAX_BRIGHT_GPIO_Port GPIOA
#define MAX_BRIGHT_EXTI_IRQn EXTI4_15_IRQn
#define USB_OVER_VOLT_Pin GPIO_PIN_9
#define USB_OVER_VOLT_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */
#define OVERTEMP_DEFAULT 56
#define SHUTDOWN_TEMP_DEFAULT 80
#define NORMAL_TEMP_DEFAULT 40
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
