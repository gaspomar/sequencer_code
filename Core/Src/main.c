/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
#include "main.h"
#include "cmsis_os.h"
#include "adc.h"
#include "dma.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "usb.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "led.h"
#include "tasks.h"
#include "midi.h"
#include "uart_buffer.h"
#include "app.h"
#include "buttons.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_FREERTOS_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

volatile uint8 rotA_cooldownMs = 0;
volatile uint8 rotB_cooldownMs = 0;

volatile uint8 tempoChangeCooldown = 0;

volatile uint16 counter_100us = 0;

volatile uint32 lastTimestamp = 0;
volatile uint32 nextTimestamp = 0;

volatile uint16 syncCnt = 0;            // midi sync event counter

const uint8_t midiClockMsg = 0xF8;


/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_USART1_UART_Init();
  MX_I2C1_Init();
  MX_USB_PCD_Init();
  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */
  
  //LED_SetAll();
  InitApp();
  
  /* USER CODE END 2 */

  /* Call init function for freertos objects (in cmsis_os2.c) */
  MX_FREERTOS_Init();

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  { 
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC|RCC_PERIPHCLK_USB;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL_DIV1_5;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM7 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */
  // 1 ms
    if (htim->Instance == TIM1)
    {
      if(tempoChangeCooldown != 0)
      {
        tempoChangeCooldown--;
      }
      seq[0].stepTimeCnt_ms++;
      seq[1].stepTimeCnt_ms++;
      seq[2].stepTimeCnt_ms++;
      seq[3].stepTimeCnt_ms++;
      app.blinkCnt_ms++;

      for(int i=0; i<32; i++)
      {
        if(btnDelay_ms[i] > 0)
        {
          btnDelay_ms[i]--;
        }
      }
    
      if(0 < rotA_cooldownMs)
      {
        rotA_cooldownMs--;
      }

      if(0 < rotB_cooldownMs)
      {
          rotB_cooldownMs--;
      }
    }
    // 100us
    if(htim->Instance == TIM3)
    {
      
      if(app.globStopFlag)
      {
        counter_100us = 0;
        syncCnt = 0;
        lastTimestamp = 0;
        nextTimestamp = app.syncTimestamps_100us[1];
      }
      if(app.glob_start_flag)
      {
        counter_100us = 0;
        syncCnt = 0;
        lastTimestamp = 0;
        nextTimestamp = app.syncTimestamps_100us[1];
      }
      else
      {
        if(app.bpmIncreased)
        {
          counter_100us = app.syncTimestamps_100us[syncCnt] + MIN(((app.syncTimestamps_100us[syncCnt+1] - app.syncTimestamps_100us[syncCnt]) - (nextTimestamp - counter_100us)), app.syncTimestamps_100us[1]);
          app.bpmIncreased = false;
        }
        if(counter_100us == app.syncTimestamps_100us[syncCnt+1])  // counter reached sync event
        {
          counter_100us++;

          HAL_GPIO_TogglePin(DBG_GPIO_Port, DBG_Pin);
          
          if(app.blinkSyncCnt == 23)
          {
            app.blinkSyncCnt = 0;
          }
          else
          {
            app.blinkSyncCnt++;
          }

          UART_Buf_AddToQueue(&midiClockMsg, 1);
          if(app.globPlaying)
          {
            // TODO: for some reason the timing is 3% inaccurate, check with scope!
            if(syncCnt == 23) // 1/4 note reached, counter resets
            {
              syncCnt = 0;
              counter_100us = 0;
              lastTimestamp = 0;
              nextTimestamp = app.syncTimestamps_100us[1];
            }
            else
            {
              syncCnt++;
              lastTimestamp = app.syncTimestamps_100us[syncCnt];
              nextTimestamp = app.syncTimestamps_100us[syncCnt+1];
            }
          }
          else  // not playing
          {
            counter_100us = 0;
          } 
        }
        else if(counter_100us > app.syncTimestamps_100us[syncCnt+1])
        {
          counter_100us = 0;
        }
        else
        {
          counter_100us++;
        }
      }
      UART_Buf_Send();
    }

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM7)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
