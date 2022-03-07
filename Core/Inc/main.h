/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
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
#include "stm32g0xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
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
void SystemClock_Config(void);

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define AIN1_Pin GPIO_PIN_11
#define AIN1_GPIO_Port GPIOC
#define AIN1_EXTI_IRQn EXTI4_15_IRQn
#define CAN2_RX_Pin GPIO_PIN_2
#define CAN2_RX_GPIO_Port GPIOC
#define CAN2_RX_EXTI_IRQn EXTI2_3_IRQn
#define GSM_POW_Pin GPIO_PIN_7
#define GSM_POW_GPIO_Port GPIOC
#define WIFI_LED_SW_Pin GPIO_PIN_0
#define WIFI_LED_SW_GPIO_Port GPIOD
#define GPS_LED_SW_Pin GPIO_PIN_1
#define GPS_LED_SW_GPIO_Port GPIOD
#define GPRS_LED_SW_Pin GPIO_PIN_2
#define GPRS_LED_SW_GPIO_Port GPIOD
#define CAN_LED_SW_Pin GPIO_PIN_3
#define CAN_LED_SW_GPIO_Port GPIOD
#define AIN2_Pin GPIO_PIN_10
#define AIN2_GPIO_Port GPIOC
#define AIN2_EXTI_IRQn EXTI4_15_IRQn
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
