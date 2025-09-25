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
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "misc.h"

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

#define ROT_COOLDOWN_MS 1

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

extern volatile uint8 rotA_cooldownMs;
extern volatile uint8 rotB_cooldownMs;

extern volatile uint8 tempoChangeCooldown;

extern volatile uint16 syncCnt;

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define DBG_Pin GPIO_PIN_0
#define DBG_GPIO_Port GPIOC
#define DISP_SCK_Pin GPIO_PIN_1
#define DISP_SCK_GPIO_Port GPIOC
#define DISP_DIO_Pin GPIO_PIN_2
#define DISP_DIO_GPIO_Port GPIOC
#define POT1_Pin GPIO_PIN_0
#define POT1_GPIO_Port GPIOA
#define POT2_Pin GPIO_PIN_1
#define POT2_GPIO_Port GPIOA
#define POT3_Pin GPIO_PIN_2
#define POT3_GPIO_Port GPIOA
#define POT4_Pin GPIO_PIN_3
#define POT4_GPIO_Port GPIOA
#define POT5_Pin GPIO_PIN_4
#define POT5_GPIO_Port GPIOA
#define POT6_Pin GPIO_PIN_5
#define POT6_GPIO_Port GPIOA
#define DBG_A6_Pin GPIO_PIN_6
#define DBG_A6_GPIO_Port GPIOA
#define BTN_D7_Pin GPIO_PIN_11
#define BTN_D7_GPIO_Port GPIOB
#define BTN_D6_Pin GPIO_PIN_12
#define BTN_D6_GPIO_Port GPIOB
#define BTN_D5_Pin GPIO_PIN_13
#define BTN_D5_GPIO_Port GPIOB
#define BTN_D4_Pin GPIO_PIN_14
#define BTN_D4_GPIO_Port GPIOB
#define BTN_D3_Pin GPIO_PIN_15
#define BTN_D3_GPIO_Port GPIOB
#define BTN_D2_Pin GPIO_PIN_6
#define BTN_D2_GPIO_Port GPIOC
#define BTN_D1_Pin GPIO_PIN_7
#define BTN_D1_GPIO_Port GPIOC
#define BTN_D0_Pin GPIO_PIN_8
#define BTN_D0_GPIO_Port GPIOC
#define BTN_S3_Pin GPIO_PIN_9
#define BTN_S3_GPIO_Port GPIOC
#define BTN_S2_Pin GPIO_PIN_8
#define BTN_S2_GPIO_Port GPIOA
#define MIDI_TX_Pin GPIO_PIN_9
#define MIDI_TX_GPIO_Port GPIOA
#define MIDI_RX_Pin GPIO_PIN_10
#define MIDI_RX_GPIO_Port GPIOA
#define BTN_S1_Pin GPIO_PIN_15
#define BTN_S1_GPIO_Port GPIOA
#define BTN_S0_Pin GPIO_PIN_10
#define BTN_S0_GPIO_Port GPIOC
#define LED_SER_Pin GPIO_PIN_11
#define LED_SER_GPIO_Port GPIOC
#define LED_SRCK_Pin GPIO_PIN_12
#define LED_SRCK_GPIO_Port GPIOC
#define LED_RCK_Pin GPIO_PIN_2
#define LED_RCK_GPIO_Port GPIOD
#define LED_CLR__Pin GPIO_PIN_3
#define LED_CLR__GPIO_Port GPIOB
#define LED_GNOT_Pin GPIO_PIN_4
#define LED_GNOT_GPIO_Port GPIOB
#define MEM_WCN_Pin GPIO_PIN_5
#define MEM_WCN_GPIO_Port GPIOB
#define MEM_SCL_Pin GPIO_PIN_6
#define MEM_SCL_GPIO_Port GPIOB
#define MEM_SDA_Pin GPIO_PIN_7
#define MEM_SDA_GPIO_Port GPIOB
#define ROT_A_Pin GPIO_PIN_8
#define ROT_A_GPIO_Port GPIOB
#define ROT_A_EXTI_IRQn EXTI9_5_IRQn
#define ROT_B_Pin GPIO_PIN_9
#define ROT_B_GPIO_Port GPIOB
#define ROT_B_EXTI_IRQn EXTI9_5_IRQn

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
