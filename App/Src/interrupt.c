#include "main.h"
#include "tim.h"
#include "usart.h"
#include "FreeRTOS.h"
#include "task.h"

#include "misc.h"
#include "tasks.h"

bool pinStateA = true;
bool pinStateB = true;

// ------- placed in main.c --------
//void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
//{
//}








// Rotary event
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if(GPIO_Pin == ROT_A_Pin)
    {
        if(!rotA_cooldownMs)
        {
            rotA_cooldownMs = ROT_COOLDOWN_MS;
            pinStateB = (bool)HAL_GPIO_ReadPin(ROT_B_GPIO_Port, ROT_B_Pin);
            if((false == pinStateA) && (true == pinStateB))
            {
                xTaskNotifyFromISR(mainTaskHandle, NOTIF_ROT_INPUT_LEFT, eSetBits, NULL);
            }
            pinStateA = HAL_GPIO_ReadPin(ROT_A_GPIO_Port, ROT_A_Pin);
        }
    }
    if(GPIO_Pin == ROT_B_Pin)
    {
        if(!rotB_cooldownMs)
        {
            rotB_cooldownMs = ROT_COOLDOWN_MS;
            pinStateA = (bool)HAL_GPIO_ReadPin(ROT_A_GPIO_Port, ROT_A_Pin);
            if((true == pinStateA) && (false == pinStateB))
            {
                xTaskNotifyFromISR(mainTaskHandle, NOTIF_ROT_INPUT_RIGHT, eSetBits, NULL);
            }
            pinStateB = HAL_GPIO_ReadPin(ROT_B_GPIO_Port, ROT_B_Pin);
        }
    }
}