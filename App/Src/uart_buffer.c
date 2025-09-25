#include "mein.h"
#include "main.h"
#include <string.h>
#include "usart.h"
#include "uart_buffer.h"


volatile static UART_Buf_Element_t buffer[UART_BUF_SIZE];
volatile static uint8 top = 0;
volatile static uint8 btm = 0;
volatile static uint8 num = 0;
volatile static bool transferInProgress = false;


void UART_Buf_AddToQueue(uint8* data, uint8 size)
{
    __disable_irq();
    if((size <= UART_ELEMENT_SIZE) && (num < UART_BUF_SIZE))
    {
        memset(buffer[top].buf, 0, UART_ELEMENT_SIZE);
        memcpy(buffer[top].buf, data, size);
        buffer[top].size = size;
        if(++top == UART_BUF_SIZE)
        {
            top = 0;
        }
        num++;
    }
    __enable_irq();
}


bool UART_Buf_IsEmpty()
{
    __disable_irq();
    if(num == 0)
    {
        return true;
    }
    else
    {
        return false;
    }
    __enable_irq();
}


void UART_Buf_Send()
{
    __disable_irq();
    if((UART_Buf_IsEmpty()==false) && !transferInProgress)
    {
        transferInProgress = true;
        HAL_UART_Transmit_IT(&huart1, buffer[btm].buf, buffer[btm].size);
    }
    __enable_irq();
}


void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    
    memset(buffer[btm].buf, 0, UART_ELEMENT_SIZE);
    buffer[btm].size = 0;
    if(++btm == UART_BUF_SIZE)
    {
        btm = 0;
    }
    num--;
    transferInProgress = false;
    UART_Buf_Send();
}