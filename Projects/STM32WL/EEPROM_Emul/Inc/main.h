/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : InternOsc_Trimming/Inc/main.h
  * @author  		: MCD Application Team
  * @version 		: V2.0.0
  * @date    		: 24-August-2020
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
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
#include "stm32wlxx_hal.h"
#include "stm32wlxx_nucleo.h"
#include "eeprom_emul.h"

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
#ifdef LEDS_BUTTONS

/* The following constants can be defined as the clock, pin and port of GPIOs
 * to ease and improve the trimming procedure (it may be needed to also adjust the GPIO mode
 * in MX_GPIO_Init() in main.c).
 * During manual fine tuning (not mandatory but strongly advised):
 * -> Button 1 increases the trimming parameter,
 * -> Button 3 decreases the trimming parameter,
 * -> Button 2 saves the trimming parameter in memory at STORE_ADDRESS.
 * FINISH_LED lights up when the loading or saving of trimming parameters are successful.
 * ERROR_LED lights up in case of error.
 * TRIMMING_OK lights up after a successful trimming but before the saving of trimming paremeters */

#define __BUTTON_1_CLK_ENABLE() __HAL_RCC_GPIOA_CLK_ENABLE()
#define __BUTTON_2_CLK_ENABLE() __HAL_RCC_GPIOA_CLK_ENABLE()
#define __BUTTON_3_CLK_ENABLE() __HAL_RCC_GPIOC_CLK_ENABLE()

#define __FINISH_LED_CLK_ENABLE() 		__HAL_RCC_GPIOB_CLK_ENABLE()
#define __ERROR_LED_CLK_ENABLE() 		__HAL_RCC_GPIOB_CLK_ENABLE()
#define __TRIMMING_OK_LED_CLK_ENABLE() 	__HAL_RCC_GPIOB_CLK_ENABLE()

#define BUTTON_1_Pin 			GPIO_PIN_0
#define BUTTON_1_GPIO_Port 		GPIOA
#define BUTTON_1_ActiveState 	0
#define BUTTON_2_Pin 			GPIO_PIN_1
#define BUTTON_2_GPIO_Port 		GPIOA
#define BUTTON_2_ActiveState 	0
#define BUTTON_3_Pin 			GPIO_PIN_6
#define BUTTON_3_GPIO_Port 		GPIOC
#define BUTTON_3_ActiveState 	0

#define FINISH_LED_Pin 				GPIO_PIN_9
#define FINISH_LED_GPIO_Port 		GPIOB
#define ERROR_LED_Pin 				GPIO_PIN_11
#define ERROR_LED_GPIO_Port 		GPIOB
#define TRIMMING_OK_LED_Pin 		GPIO_PIN_15
#define TRIMMING_OK_LED_GPIO_Port 	GPIOB

#endif
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
