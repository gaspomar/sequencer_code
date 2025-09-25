#ifndef UART_BUFFER_H
#define UART_BUFFER_H

#include "mein.h"

#define UART_BUF_SIZE 20
#define UART_ELEMENT_SIZE 3

typedef struct 
{
    uint8 size;
    uint8 buf[UART_ELEMENT_SIZE];
} UART_Buf_Element_t;


bool UART_Buf_IsEmpty();


void UART_Buf_Send();


void UART_Buf_AddToQueue(uint8* data, uint8 size);


UART_Buf_Element_t* UART_Buf_GetNextInLine();

#endif /* UART_BUFFER_H */