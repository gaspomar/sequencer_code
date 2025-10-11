#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "main.h"
#include "tim.h"
#include "usart.h"
#include "adc.h"

#include "app.h"
#include "misc.h"
#include "midi.h"
#include "led.h"
#include "tm1637.h"
#include "tasks.h"
#include "uart_buffer.h"
#include "menu.h"
#include "buttons.h"

#include <string.h>
#include <stdio.h>
#include <math.h>


void MainTask(void *);
void LedUpdateTask(void *);
void DispUpdateTask(void *);
void PotUpdateTask(void *);
void ButtonReadTask(void *);
void UartRxProcessTask(void *);


OSData_t os;


/************************************************************************************************************************
 * Functions
 ***********************************************************************************************************************/

 void InitOSData()
 {
	// semaphores and events
	os.uartSem = xSemaphoreCreateBinary();
	os.rsrcMutex = xSemaphoreCreateMutex();
	
	// tasks
	BaseType_t retVal;
	
	retVal = xTaskCreate(MainTask, "mainTask", MAIN_TASK_STACK_SIZE, NULL, 3, &os.mainTaskHandle);
	if(pdPASS != retVal) { /*error*/ }
	
	retVal = xTaskCreate(LedUpdateTask, "ledUpdateTask", LED_UPDATE_TASK_STACK_SIZE, NULL, 1, &os.ledUpdateTaskHandle);
	if(pdPASS != retVal) { /*error*/ }

	retVal = xTaskCreate(DispUpdateTask, "dispUpdateTask", DISP_UPDATE_TASK_STACK_SIZE, NULL, 1, &os.dispUpdateTaskHandle);
	if(pdPASS != retVal) { /*error*/ }
	
	retVal = xTaskCreate(PotUpdateTask, "potUpdateTask", POT_UPDATE_TASK_STACK_SIZE, NULL, 2, &os.potUpdateTaskHandle);
	if(pdPASS != retVal) { /*error*/ }

	retVal = xTaskCreate(ButtonReadTask, "btnReadTask", BTN_READ_TASK_STACK_SIZE, NULL, 2, &os.buttonReadTaskHandle);
	if(pdPASS != retVal) { /*error*/ }

	retVal = xTaskCreate(UartRxProcessTask, "uartRxProcTask", UART_RX_PROCESS_TASK_STACK_SIZE, NULL, 2, &os.uartRxProcessTaskHandle);
 }


void LedUpdateTask(void *)
{
	static uint32 notVal;
	static bool stepBlinkInProg = false;		// blinking due to reaching new step is in progress
	static uint16 blinkCntLoc_ms = 0;			// local copy of blink counter

	static uint16 blinkSyncCntLoc = 0;
	static uint16 blinkSyncCntPrev = 0;

	static bool stepLEDs[16];
	static bool miscLEDs[6];
	static bool potLEDs[3];
	static bool seqLEDs[4];

	static bool stepLEDsBlink[16];
	static bool miscLEDsBlink[6];
	static bool potLEDsBlink[3];
	static bool seqLEDsBlink[4];

	// ------------ stepping ---------
	while(1)
	{
		memset(stepLEDsBlink, 0, 16*sizeof(bool));
		memset(miscLEDsBlink, 0, 6*sizeof(bool));
		memset(potLEDsBlink, 0, 3*sizeof(bool));
		memset(seqLEDsBlink, 0, 4*sizeof(bool));
		
		if(pdTRUE == xSemaphoreTake(os.rsrcMutex, portMAX_DELAY))
		{
			__disable_irq();
			blinkSyncCntLoc = app.blinkSyncCnt;
			blinkCntLoc_ms = app.blinkCnt_ms;
			__enable_irq();

			
			xTaskNotifyWait(0x0000, 0xFFFF, &notVal, 0);
			
			if (notVal & NOTIF_NEW_STEP_REACHED)
			{
				if(app.seqActive->pageCurr == app.seqActive->pageSel)
				{
					stepBlinkInProg = true;
				}
			}
			
			
			if((blinkSyncCntLoc != blinkSyncCntPrev) && (blinkSyncCntLoc % 24 == 0))
			{
				__disable_irq();
				app.blinkCnt_ms = 0;
				blinkCntLoc_ms = 0;
				__enable_irq();
			}
			
			memset(potLEDs, 0, 3*sizeof(bool));
			memset(seqLEDs, 0, 4*sizeof(bool));
			memset(miscLEDs, 0, 6*sizeof(bool));

			switch(app.modeCurr)
			{
				case MODE_DEFAULT:// -----------------------------------------------------------
				{
					// sequencer select 
					if(btnPressed[BTN_SHIFT])
					{
						for(int i=0; i<NUM_SEQUENCERS; i++)
						{
							seqLEDs[i] = seq[i].on;
						}
					}
					else
					{
						seqLEDs[app.seqActive->id] = true;
					}

					// page select / misc
					miscLEDsBlink[app.seqActive->pageCurr->id] = true;
					for(int i=0; i<NUM_PAGES; i++)
					{
						if(btnPressed[BTN_SHIFT])
						{
							miscLEDs[i] = app.seqActive->patternCurr->pages[i].on;
						}
						else
						{
							miscLEDs[i] = app.seqActive->pageSel->id == app.seqActive->patternCurr->pages[i].id;
						}
					}

					if(stepBlinkInProg)
					{
						stepLEDs[app.seqActive->iStepCurr] = !app.seqActive->pageSel->steps[app.seqActive->iStepCurr].on;
					}
					
					for(int i=0; i<16; i++)
					{
						if(!stepBlinkInProg)
						{
							stepLEDs[i] = app.seqActive->pageSel->steps[i].on;
						}
					}
				}
				break;

				case MODE_PITCH: // -----------------------------------------------------------
				{
					// page select / misc
					switch(app.seqActive->pageSel->steps[app.iStepSel].octOffs)
					{
						case -3:
						{
							miscLEDs[0] = true;
							miscLEDs[1] = true;
							miscLEDs[2] = true;
							break;
						}
						case -2:
						{
							miscLEDs[1] = true;
							miscLEDs[2] = true;
							break;
						}
						case -1:
						{
							miscLEDs[2] = true;
							break;
						}
						case 1:
						{
							miscLEDs[3] = true;
							break;
						}
						case 2:
						{
							miscLEDs[3] = true;
							miscLEDs[4] = true;
							break;
						}
						case 3:
						{
							miscLEDs[3] = true;
							miscLEDs[4] = true;
							miscLEDs[5] = true;
							break;
						}
					}

					// steps
					stepLEDsBlink[app.iStepSel] = true;
					if(stepBlinkInProg)
					{
						stepLEDs[app.seqActive->iStepCurr] = !app.seqActive->pageSel->steps[app.seqActive->iStepCurr].on;
					}
					for(int i=0; i<16; i++)
					{
						if(!stepBlinkInProg)
						{
							stepLEDs[i] = app.seqActive->pageSel->steps[i].on;
						}
					}
				}
				break;

				case MODE_SET_CHANNEL: // -----------------------------------------------------------
				{
					// sequencer select 
					seqLEDs[app.seqActive->id] = true;

					// steps
					memset(stepLEDs, 0, NUM_STEPS);
					stepLEDs[app.seqActive->midiChannel] = true;
				}
				break;

				case MODE_COPY: // --------------------------------------------------------------------
				{
					
					if(menu.state == MENU_COPY_PAGES)
					{
						miscLEDsBlink[menu.pageSel->id] = true;
					}
					else if(menu.state == MENU_PASTE_STEPS)
					{
						for(int i=menu.iStepSel; i<(menu.iStepSel + menu.numSelected); i++)
						{
							stepLEDsBlink[i] = true;
						}
					}
					// steps
					for(int i=0; i<16; i++)
					{
						stepLEDs[i] = app.seqActive->pageSel->steps[i].on;
					}
				}
				break;
			}


			// potentiometer leds
			if(false == app.seqActive->gateInSync)
			{
				if(app.seqActive->gatePercent > app.gatePercent)
				{
					potLEDsBlink[1] = true;
				}
				else
				{
					potLEDs[1] = true;
				}
			}


			// ------- gate reached -----------
			if (app.seqActive->stepTimeCnt_ms >= 30)
			{
				if(stepBlinkInProg)
				{
					stepBlinkInProg = false;
				}
			}
				
			// blinking ---------------------------------------------
			if(blinkCntLoc_ms < BLINK_ON_INTERVAL_MS)
			{
				for(int i=0; i<16; i++)
				{
					if((app.seqActive->pageCurr != app.seqActive->pageSel) || (app.iStepSel != app.seqActive->iStepCurr))
					{
						stepLEDs[i] = stepLEDs[i] ^ stepLEDsBlink[i];
					}
				}
				for(int i=0; i<6; i++)
				{
					miscLEDs[i] = miscLEDs[i] ^ miscLEDsBlink[i];
				}
				for(int i=0; i<4; i++)
				{
					seqLEDs[i] = seqLEDs[i] ^ seqLEDsBlink[i];
				}
				for(int i=0; i<3; i++)
				{
					potLEDs[i] = potLEDs[i] ^ potLEDsBlink[i];
				}
			}

			LED_Set(stepLEDs, miscLEDs, potLEDs, seqLEDs);
		}

		__disable_irq();
		blinkSyncCntPrev = blinkSyncCntLoc;
		__enable_irq();

		xSemaphoreGive(os.rsrcMutex);

		vTaskDelay(1);
    }
}


void DispUpdateTask(void *)
{
	while(1)
	{
		switch(app.modeCurr)
		{
			case MODE_DEFAULT:
			{
				tm1637_SetNumber(app.tempoBpm);
				break;
			}
			case MODE_PITCH:
			{
				tm1637_SetWord("ptch", 4);
				break;
			}
			case MODE_SET_CHANNEL:
			{
				tm1637_SetWord("chan", 4);
				//tm1637_SetWordAndNum("ch", 2, midiCh);
				break;
			}
			case MODE_COPY:
			{
				tm1637_SetWord("copy", 4);
				break;
			}
			default:{}
		}
		vTaskDelay(pdMS_TO_TICKS(10));
	}
}


void PotUpdateTask(void *)
{
	while(1)
	{
		app.gatePercent = MAX((uint8)SATURATE((((fabs((float)SATURATE(app.potBuffer.gate, 4040) - 4040.0))/4040.0)*100.0), 99.0), 1);

		if((app.seqActive->gatePercent >= app.gatePercent-5) && (app.seqActive->gatePercent <= app.gatePercent+5))
		{
			app.seqActive->gateInSync = true;
		}

		if(app.seqActive->gateInSync)
		{
			app.seqActive->gatePercent = app.gatePercent;
			app.seqActive->gateTime_ms = CalculateGateTime(app.seqActive->stepTime_ms, app.seqActive->gatePercent);
		}

		vTaskDelay(pdMS_TO_TICKS(10));
	}
}


void ButtonReadTask(void *)
{
	uint8 iBtn = 0;

	while(1)
	{
		// sample current state
		if(pdTRUE == xSemaphoreTake(os.rsrcMutex, portMAX_DELAY))
		{
			for(int i=0; i<4; i++)
			{
				HAL_GPIO_WritePin(btnSelect[i].port, btnSelect[i].pin, GPIO_PIN_SET);
			
				for(int j=0; j<8; j++)
				{
					iBtn = i*8 + j;	// button index

					if(btnDelay_ms[iBtn] == 0)
					{
						if(GPIO_PIN_SET == HAL_GPIO_ReadPin(btnDetect[j].port, btnDetect[j].pin))
						{
							if(false == btnPressed[iBtn])
							{
								if(btnPressed[BTN_SHIFT])
								{
									ButtonActivate(iBtn, true, BTN_PUSHED);
								}
								else
								{
									ButtonActivate(iBtn, false, BTN_PUSHED);
								}
								btnPressed[iBtn] = true;
								btnDelay_ms[iBtn] = BUTTON_DEBOUNCE_DELAY_MS;
							}
						}
						else // GPIO_PIN_RESET
						{
							if(true == btnPressed[iBtn])
							{
								if(btnPressed[BTN_SHIFT])
								{
									ButtonActivate(iBtn, true, BTN_RELEASED);
								}
								else
								{
									ButtonActivate(iBtn, false, BTN_RELEASED);
								}
							}
							btnPressed[iBtn] = false;
							btnDelay_ms[iBtn] = BUTTON_DEBOUNCE_DELAY_MS;
						}
					}
				}
				HAL_GPIO_WritePin(btnSelect[i].port, btnSelect[i].pin, GPIO_PIN_RESET);
			}
			xSemaphoreGive(os.rsrcMutex);
		} 

		vTaskDelay(pdMS_TO_TICKS(2));
	}
}


void UartRxProcessTask(void *)
{
	while(1)
	{
		if(pdTRUE == xSemaphoreTake(os.rsrcMutex, portMAX_DELAY))
		{
			for(int i=0; i<UART_RX_BUF_SIZE; i++)
			{
				UART_Buf_ProcessRxBuffer();
			}
			xSemaphoreGive(os.rsrcMutex);
		}
		vTaskDelay(1);
	}
}



void MainTask(void *)
{
	static uint32 notVal;	// notification value

	static uint16 syncCntLocal = 0;
	static int32 syncCntPrev = -1;

	static bool allSeqOff;
	static bool allNotesOff;

	while(1)
    {	
		// one mutex for all resources
		if(pdTRUE == xSemaphoreTake(os.rsrcMutex, portMAX_DELAY))
		{
			__disable_irq();
			syncCntLocal = syncCnt;
			__enable_irq();

			xTaskNotifyWait(0x0000, 0xFFFF, &notVal, 0);

			allSeqOff = !seq[0].on && !seq[1].on && !seq[2].on && !seq[3].on;
	
			if((notVal & NOTIF_GLOB_START_STOP) || allSeqOff)
			{
				if(app.globPlaying)
				{
					app.globStopFlag = true;
					DelayFuncUs(150);
				}
				else if(!app.globPlaying && (notVal & NOTIF_GLOB_START_STOP) && !allSeqOff)
				{
					ResetSequencer(&seq[0]);
					ResetSequencer(&seq[1]);
					ResetSequencer(&seq[2]);
					ResetSequencer(&seq[3]);
					
					app.globStartFlag = true;
					DelayFuncUs(150);	// wait for ISR to enter waiting loop
				}
			}
			if (notVal & NOTIF_ROT_INPUT_RIGHT)
			{
				switch(app.modeCurr)
				{
					case MODE_DEFAULT:
					case MODE_PITCH:
					IncrementBPM();
					break;

					case MODE_SET_CHANNEL:
					if(app.seqActive->id < 3)
					{
						app.seqActive = &seq[app.seqActive->id + 1];
					}
					break;

					default:{}
				}
			}
			if (notVal & NOTIF_ROT_INPUT_LEFT)
			{
				switch(app.modeCurr)
				{
					case MODE_DEFAULT:
					case MODE_PITCH:
					DecrementBPM();
					break;

					case MODE_SET_CHANNEL:
					if(app.seqActive->id > 0)
					{
						app.seqActive = &seq[app.seqActive->id - 1];
					}
					break;

					default:{}
				}
			}

			if(noteOnReceived)
			{
				noteOnReceived = false;
				switch(app.modeCurr)
				{
					case MODE_DEFAULT:
					if(menu.state == MENU_LISTEN_ON_NOTE)
					{
						//if(menu.seqSel->midiChannel == (noteOnMsg[0] & 0x0F))
						//{
							menu.stepSel->pitch[0] = noteOnMsg[1];
							menu.stepSel->on = true;
							Menu_Reset();
						//}
					}
					break;

					case MODE_PITCH:
					__disable_irq();
					app.seqActive->pageSel->steps[app.iStepSel].pitch[0] = noteOnMsg[1];
					__enable_irq();
					break;

					default:{}
				}
			}
			
			// ----------------------- STEPPING -----------------------------
			if(app.globPlaying || app.globStartFlag)
			{
				if(((syncCntLocal != syncCntPrev) || app.globStartFlag) && !app.globStopFlag)	// sync counter updated or start initiated
				{
					for(int i=0; i<NUM_SEQUENCERS; i++)
					{
						if(app.globStartFlag)
						{
							seq[i].stepTimeCnt_ms = 0;
						}

						if((seq[i].onFlag) && (syncCntLocal == 0))
						{
							seq[i].on = true;
							seq[i].onFlag = false;
						}

						if(seq[i].on && (syncCntLocal % seq[i].syncEventsPerStep == 0))	// new step reached
						{
							Step(&seq[i]);
							
							if (true == seq[i].pageCurr->steps[seq[i].iStepCurr].on)
							{
								SendNoteONs(&seq[i]);
								seq[i].noteOn = true;
							}
							if(app.seqActive == &seq[i])
							{
								xTaskNotify(os.ledUpdateTaskHandle, NOTIF_NEW_STEP_REACHED, eSetBits);
							}
						}
					}
				}
				
				for(int i=0; i<NUM_SEQUENCERS; i++)
				{
					if(seq[i].on && (seq[i].stepTimeCnt_ms >= seq[i].gateTime_ms))
					{
						if(seq[i].noteOn)
						{
							SendNoteOFFs(&seq[i]);
							seq[i].noteOn = false;
						}
						if(seq[i].offFlag)
						{
							seq[i].on = false;
							seq[i].offFlag = false;
							ResetSequencer(&seq[i]);
						}
					}
				}
				
				allNotesOff = !seq[0].noteOn && !seq[1].noteOn && !seq[2].noteOn && !seq[3].noteOn;
				
				if(app.globStartFlag)
				{
					app.globStartFlag = false;
					app.globPlaying = true;
				}
				
				if(app.globStopFlag && allNotesOff)
				{
					app.globPlaying = false;
					app.globStopFlag = false;
				}
			}
			else if(!app.globPlaying)
			{
				for(int i=0; i<NUM_SEQUENCERS; i++)
				{
					if(!seq[i].on && seq[i].onFlag)
					{
						seq[i].on = true;
						seq[i].onFlag = false;
					}
					if(seq[i].on && seq[i].offFlag)
					{
						seq[i].on = false;
						seq[i].offFlag = false;
					}
				}
			}

			xSemaphoreGive(os.rsrcMutex);
		}
		syncCntPrev = syncCntLocal;
		
		vTaskDelay(pdMS_TO_TICKS(1));
	}
}