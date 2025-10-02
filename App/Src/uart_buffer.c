#include "misc.h"
#include "main.h"
#include <string.h>
#include "usart.h"
#include "uart_buffer.h"


volatile static UART_Buf_Element_t txBuffer[UART_TX_BUF_SIZE];
volatile static uint8 rxBuffer[UART_RX_BUF_SIZE];

volatile uint8 noteOnMsg[3];
volatile bool noteOnReceived = false;

volatile static uint8 top = 0;
volatile static uint8 btm = 0;
volatile static uint8 num = 0;
volatile static bool transferInProgress = false;

uint8 iRxBuff = 0;  // index of current buffer slot
uint8 iNoteOnMsg = 0;


void UART_Buf_Init()
{
    for(int i=0; i<UART_RX_BUF_SIZE; i++)
    {
        rxBuffer[i] = 0xFF;
    }
}


void UART_Buf_AddToQueue(const uint8* data, uint8 size)
{
    __disable_irq();
    if((size <= UART_ELEMENT_SIZE) && (num < UART_TX_BUF_SIZE))
    {
        memset(txBuffer[top].buf, 0, UART_ELEMENT_SIZE);
        memcpy(txBuffer[top].buf, data, size);
        txBuffer[top].size = size;
        if(++top == UART_TX_BUF_SIZE)
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
        HAL_UART_Transmit_IT(&huart1, txBuffer[btm].buf, txBuffer[btm].size);
    }
    __enable_irq();
}


void UART_Buf_Receive()
{
    HAL_UART_Receive_DMA(&huart1, rxBuffer, UART_RX_BUF_SIZE);
}


void UART_Buf_ProcessRxBuffer()
{
    // check if data has arrived
    if(rxBuffer[iRxBuff] != 0xFF)
    {
        // check if msg is active sensing
        if(rxBuffer[iRxBuff] != 0xFE)
        {
            // check is msg is note on
            if((rxBuffer[iRxBuff] & 0xF0) == 0x90)
            {
                iNoteOnMsg = 0;
                noteOnMsg[iNoteOnMsg] = rxBuffer[iRxBuff];
                iNoteOnMsg++;
            }
            // check if byte is data byte (first bit is 0)
            else if((rxBuffer[iRxBuff] & 0x80) == 0)
            {
                if(iNoteOnMsg > 0)
                {
                    noteOnMsg[iNoteOnMsg] = rxBuffer[iRxBuff];
                    
                    if(iNoteOnMsg < 2)
                    {
                        iNoteOnMsg++;
                    }
                    else
                    {
                        iNoteOnMsg = 0;
                        noteOnReceived = true;
                    }
                }
            }
        }
        
        rxBuffer[iRxBuff] = 0xFF;   // byte processed, set to default (0xFF)

        if(iRxBuff == UART_RX_BUF_SIZE - 1)
        {
            iRxBuff = 0;
        }
        else
        {
            iRxBuff++;
        }
    }
}





void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    
    memset(txBuffer[btm].buf, 0, UART_ELEMENT_SIZE);
    txBuffer[btm].size = 0;
    if(++btm == UART_TX_BUF_SIZE)
    {
        btm = 0;
    }
    num--;
    transferInProgress = false;
    UART_Buf_Send();
}


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    //
}