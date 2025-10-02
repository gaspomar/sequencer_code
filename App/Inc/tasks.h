#ifndef TASKS_H
#define TASKS_H

#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"


#define MAIN_TASK_STACK_SIZE 128
#define LED_UPDATE_TASK_STACK_SIZE 64
#define DISP_UPDATE_TASK_STACK_SIZE 64
#define POT_UPDATE_TASK_STACK_SIZE 64
#define BTN_READ_TASK_STACK_SIZE 128
#define UART_RX_PROCESS_TASK_STACK_SIZE 64


typedef struct 
{
	SemaphoreHandle_t uartSem;
	SemaphoreHandle_t rsrcMutex;
	
	TaskHandle_t mainTaskHandle;
	TaskHandle_t ledUpdateTaskHandle;
	TaskHandle_t dispUpdateTaskHandle;
	TaskHandle_t potUpdateTaskHandle;
	TaskHandle_t buttonReadTaskHandle;
	TaskHandle_t uartRxProcessTaskHandle;
} OSData_t;


#define NOTIF_NEW_STEP_REACHED 0x0001
#define NOTIF_ROT_INPUT_RIGHT 0x0002
#define NOTIF_ROT_INPUT_LEFT 0x0004
#define NOTIF_GLOB_START_STOP 0x0008
#define NOTIF_LED_UPDATE_REQUIRED 0x0010

extern OSData_t os;

void InitOSData();

#endif /*TASK_H*/