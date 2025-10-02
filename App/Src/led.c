#include "led.h"
#include "main.h"
#include "misc.h"


void LED_Set(bool steps[16], bool misc[6], bool pots[3], bool seq[4])
{
    HAL_GPIO_WritePin(LED_SRCK_GPIO_Port, LED_SRCK_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED_RCK_GPIO_Port, LED_RCK_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED_GNOT_GPIO_Port, LED_GNOT_Pin, GPIO_PIN_SET);

    // steps
    for(int i = 15; i >= 0; i--)
    {
        HAL_GPIO_WritePin(LED_SER_GPIO_Port, LED_SER_Pin, steps[i]);
        HAL_GPIO_WritePin(LED_SRCK_GPIO_Port, LED_SRCK_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(LED_SRCK_GPIO_Port, LED_SRCK_Pin, GPIO_PIN_RESET);
    }

    // misc
    for(int i = 5; i >= 0; i--)
    {
        HAL_GPIO_WritePin(LED_SER_GPIO_Port, LED_SER_Pin, misc[i]);
        HAL_GPIO_WritePin(LED_SRCK_GPIO_Port, LED_SRCK_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(LED_SRCK_GPIO_Port, LED_SRCK_Pin, GPIO_PIN_RESET);
    }

    // pots
    for(int i = 2; i >= 0; i--)
    {
        HAL_GPIO_WritePin(LED_SER_GPIO_Port, LED_SER_Pin, pots[i]);
        HAL_GPIO_WritePin(LED_SRCK_GPIO_Port, LED_SRCK_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(LED_SRCK_GPIO_Port, LED_SRCK_Pin, GPIO_PIN_RESET);
    }
    for(int i = 2; i >= 0; i--)
    {
        HAL_GPIO_WritePin(LED_SER_GPIO_Port, LED_SER_Pin, false);
        HAL_GPIO_WritePin(LED_SRCK_GPIO_Port, LED_SRCK_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(LED_SRCK_GPIO_Port, LED_SRCK_Pin, GPIO_PIN_RESET);
    }

    // sequence select
    for(int i = 3; i >= 0; i--)
    {
        HAL_GPIO_WritePin(LED_SER_GPIO_Port, LED_SER_Pin, seq[i]);
        HAL_GPIO_WritePin(LED_SRCK_GPIO_Port, LED_SRCK_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(LED_SRCK_GPIO_Port, LED_SRCK_Pin, GPIO_PIN_RESET);
    }

    HAL_GPIO_WritePin(LED_RCK_GPIO_Port, LED_RCK_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LED_GNOT_GPIO_Port, LED_GNOT_Pin, GPIO_PIN_RESET);
}


void LED_SetAll()
{
    HAL_GPIO_WritePin(LED_SRCK_GPIO_Port, LED_SRCK_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED_RCK_GPIO_Port, LED_RCK_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED_GNOT_GPIO_Port, LED_GNOT_Pin, GPIO_PIN_SET);

    for(int i = 31; i >= 0; i--)
    {
        HAL_GPIO_WritePin(LED_SER_GPIO_Port, LED_SER_Pin, true);
        HAL_GPIO_WritePin(LED_SRCK_GPIO_Port, LED_SRCK_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(LED_SRCK_GPIO_Port, LED_SRCK_Pin, GPIO_PIN_RESET);
    }

    HAL_GPIO_WritePin(LED_RCK_GPIO_Port, LED_RCK_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LED_GNOT_GPIO_Port, LED_GNOT_Pin, GPIO_PIN_RESET);
}


void LED_ClearAll()
{
    HAL_GPIO_WritePin(LED_SRCK_GPIO_Port, LED_SRCK_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED_RCK_GPIO_Port, LED_RCK_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED_GNOT_GPIO_Port, LED_GNOT_Pin, GPIO_PIN_SET);

    for(int i = 31; i >= 0; i--)
    {
        HAL_GPIO_WritePin(LED_SER_GPIO_Port, LED_SER_Pin, false);
        HAL_GPIO_WritePin(LED_SRCK_GPIO_Port, LED_SRCK_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(LED_SRCK_GPIO_Port, LED_SRCK_Pin, GPIO_PIN_RESET);
    }

    HAL_GPIO_WritePin(LED_RCK_GPIO_Port, LED_RCK_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LED_GNOT_GPIO_Port, LED_GNOT_Pin, GPIO_PIN_RESET);
}


void LED_AnimationWelcome()
{
    bool steps[16] = {false};
    bool misc[6] = {false};
    bool pots[3] = {false};
    bool seq[4] = {false};
    
    int i_misc = 0;

    for(int i=0; i < (16+4); i++)
    {

        if(i < 16)
        {
            steps[i] = true;
        }
        if(i >= 4)
        {
            //steps[i-4] = false;
        }
        if(i % 3 == 0)
        {
            misc[i_misc++] = true;
        }
        LED_Set(steps, misc, pots, seq);
        HAL_Delay(50);
    }
}


void SetDisplay()
{
    
}
