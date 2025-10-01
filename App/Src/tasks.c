#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "main.h"
#include "tim.h"
#include "usart.h"
#include "adc.h"

#include "misc.h"
#include "midi.h"
#include "led.h"
#include "tm1637.h"
#include "tasks.h"
#include "uart_buffer.h"

#include <string.h>
#include <stdio.h>


/************************************************************************************************************************
 * Macros
 ***********************************************************************************************************************/

#define NUM_SEQUENCERS 4


 /************************************************************************************************************************
 * Types
 ***********************************************************************************************************************/

typedef struct
{
    uint16 tempo;
    uint16 gate;
	uint16 swing;
    uint16 dummy1;
    uint16 dummy2;
	uint16 dummy3;
} __attribute__((packed)) AnalogBuffer_t;


#define	BTN_SEQ1 16
#define	BTN_SEQ2 17
#define	BTN_SEQ3 18
#define	BTN_DRUM 19

#define BTN_LEFT 16
#define BTN_RIGHT 17
#define BTN_UP 18
#define BTN_DOWN 19

#define BTN_PAGE1 24
#define BTN_PAGE2 25
#define BTN_PAGE3 20
#define BTN_PAGE4 21
#define BTN_PAGE5 22
#define BTN_PAGE6 23

#define BTN_SHIFT 26
#define BTN_PITCH 30
#define	BTN_START_STOP 31

#define BTN_1 	26
#define BTN_2b 	27
#define BTN_2 	28
#define BTN_3b 	29
#define BTN_3 	30
#define BTN_4 	31
#define BTN_5b 	24
#define BTN_5 	25
#define BTN_6b 	20
#define BTN_6 	21
#define BTN_7b 	22
#define BTN_7 	23

#define NOTE_C3		60
#define NOTE_Db3	61
#define NOTE_D3		60
#define NOTE_Eb3	60
#define NOTE_E3		60
#define NOTE_F3		60
#define NOTE_Gb3	60
#define NOTE_G3		60
#define NOTE_Ab3	60
#define NOTE_A3		60
#define NOTE_Bb3	60
#define NOTE_B3		60


static const GPIO_t btnSelect[4] = {
	{BTN_S0_GPIO_Port, BTN_S0_Pin},
	{BTN_S1_GPIO_Port, BTN_S1_Pin},
	{BTN_S2_GPIO_Port, BTN_S2_Pin},
	{BTN_S3_GPIO_Port, BTN_S3_Pin}
};

static const GPIO_t btnDetect[8] = {
	{BTN_D0_GPIO_Port, BTN_D0_Pin},
	{BTN_D1_GPIO_Port, BTN_D1_Pin},
	{BTN_D2_GPIO_Port, BTN_D2_Pin},
	{BTN_D3_GPIO_Port, BTN_D3_Pin},
	{BTN_D4_GPIO_Port, BTN_D4_Pin},
	{BTN_D5_GPIO_Port, BTN_D5_Pin},
	{BTN_D6_GPIO_Port, BTN_D6_Pin},
	{BTN_D7_GPIO_Port, BTN_D7_Pin}
};

/************************************************************************************************************************
 * Variables
 ***********************************************************************************************************************/

// safe resources (no mutex needed) ---------------------------------------



// buffers ----------------------------------------------------------------

static volatile AnalogBuffer_t potBuffer;

static volatile uint8 gatePercent;

// states ----------------------------------------------------------------

volatile SeqData_t seq[4];

static volatile SeqData_t* seqActive;
static volatile Mode_e modeCurr = MODE_DEFAULT;

static volatile bool btnPressed[32] = {0};

static volatile Menu_t menu;

// flags ----------------------------------------------------------------

static volatile bool noteOn = false;

volatile bool globPlaying = false;

// timing ----------------------------------------------------------------

static volatile uint16 tempoBpm = 100;

volatile uint16 blinkCnt_ms = 0;
volatile uint16 blinkSyncCnt = 0;

volatile uint32 syncTimestamps_100us[25];	// stores the timestamps where a midi sync event should happen up to the 1/4 of the beat

// synchronization -------------------------------------------------------

SemaphoreHandle_t uartSem;

SemaphoreHandle_t rsrcMutex;

volatile bool globStopFlag = false;
volatile bool globStartFlag = false;	// all sequencers are about to be started
volatile bool bpmIncreased = false;
volatile bool modeChanged = false;

// navigation ------------------------------------------------------------

static volatile uint8 iStepSel = 0;

// task data -------------------------------------------------------------

#define MAIN_TASK_STACK_SIZE 128
#define LED_UPDATE_TASK_STACK_SIZE 64
#define DISP_UPDATE_TASK_STACK_SIZE 64
#define POT_UPDATE_TASK_STACK_SIZE 64
#define BTN_READ_TASK_STACK_SIZE 128


void MainTask(void *);
void LedUpdateTask(void *);
void DispUpdateTask(void *);
void PotUpdateTask(void *);
void ButtonReadTask(void *);

TaskHandle_t mainTaskHandle;
TaskHandle_t ledUpdateTaskHandle;
TaskHandle_t dispUpdateTaskHandle;
TaskHandle_t potUpdateTaskHandle;
TaskHandle_t buttonReadTaskHandle;

// local functions -------------------------------------------------------------

static void MenuReset();
static void IncrementBPM();
static void DecrementBPM();
static void Step(SeqData_t* seq);
static void SendNoteONs(SeqData_t* seq);
static void SendNoteOFFs(SeqData_t* seq);
static void ResetSequencer(SeqData_t* seq);
static void ButtonActivate(uint32 iBtn, bool shift, BtnEvent_e event);

/************************************************************************************************************************
 * Tasks
 ***********************************************************************************************************************/

void InitApp()
{
	seqActive = &seq[0];

	for(int i=0; i<NUM_SEQUENCERS; i++)
	{
		for(int l=0; l<NUM_PATTERNS; l++)
		{
			seq[i].patterns[l].id = l;
			seq[i].patterns[l].empty = (i==0) ? false : true;
			seq[i].patterns[l].numPagesOn = 1;
			for(int j=0; j<NUM_PAGES; j++)
			{
				seq[i].patterns[l].pages[j].id = j;
				if(j==0) 	seq[i].patterns[l].pages[j].on = true;
				else 		seq[i].patterns[l].pages[j].on = false;
				
				for(int k=0; k<NUM_STEPS; k++)
				{
					seq[i].patterns[l].pages[j].steps[k].on = false;
					seq[i].patterns[l].pages[j].steps[k].n_poly = 1;
					seq[i].patterns[l].pages[j].steps[k].pitch[0] = 60;	// middle C
					seq[i].patterns[l].pages[j].steps[k].octOffs = 0;
				}
			}
		}
		for(int j=0; j<N_POLYPHONY; j++)
		{
			seq[i].notesOn[j] = 0;
		}
		seq[i].id = i;
		seq[i].stepRes = RES_1_16;
		seq[i].syncEventsPerStep = CalculateSyncEventsPerStep(seq[i].stepRes);
		seq[i].stepTime_ms = CalculateStepTime(tempoBpm, seq[i].syncEventsPerStep);
		seq[i].patternCurr = &seq[i].patterns[0];
		seq[i].pageCurr = &seq[i].patternCurr->pages[0];
		seq[i].pageSel = &seq[i].patternCurr->pages[0];
		seq[i].iStepCurr = -1;
		seq[i].noteOn = false;
		seq[i].midiChannel = i;
		seq[i].rootNote = NOTE_C3;
		seq[i].gateInSync = false;
		seq[i].onFlag = false;
		seq[i].offFlag = false;
		seq[i].gatePercent = 50;
		seq[i].gateTime_ms = CalculateGateTime(seq[i].stepTime_ms, seq[i].gatePercent);                               
		
		if((i==0) || (i==0))	
		seq[i].on = true; 
		else		
		seq[i].on = false;
	}

	MenuReset();

	HAL_GPIO_WritePin(DBG_A6_GPIO_Port, DBG_A6_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(DBG_GPIO_Port, DBG_Pin, GPIO_PIN_RESET);

	// peripherals
	HAL_TIM_Base_Start(&htim2);
	
	UART_Buf_Init();
	UART_Buf_Receive();
	
	HAL_ADC_Start_DMA(&hadc1, (uint32*)&potBuffer, 3);
	
	tm1637_SetUsDelayFunction(DelayFuncUs);
	
	// welcome animation
	tm1637_SetDefault();
	
	LED_ClearAll();
	LED_AnimationWelcome();
	HAL_Delay(700);
	LED_ClearAll();
	HAL_Delay(500);
	
	tm1637_SetNumber(tempoBpm);
	
	// TODO: TM1637 timeout
	
	// semaphores and events
	uartSem = xSemaphoreCreateBinary();

	rsrcMutex = xSemaphoreCreateMutex();
	
	// TODO: check if above commands were successful
	
	// tasks
	BaseType_t retVal;
	
	retVal = xTaskCreate(MainTask, "mainTask", MAIN_TASK_STACK_SIZE, NULL, 3, &mainTaskHandle);
	if(pdPASS != retVal) { /*error*/ }
	
	retVal = xTaskCreate(LedUpdateTask, "ledUpdateTask", LED_UPDATE_TASK_STACK_SIZE, NULL, 1, &ledUpdateTaskHandle);
	if(pdPASS != retVal) { /*error*/ }

	retVal = xTaskCreate(DispUpdateTask, "dispUpdateTask", DISP_UPDATE_TASK_STACK_SIZE, NULL, 1, &dispUpdateTaskHandle);
	if(pdPASS != retVal) { /*error*/ }
	
	retVal = xTaskCreate(PotUpdateTask, "potUpdateTask", POT_UPDATE_TASK_STACK_SIZE, NULL, 2, &potUpdateTaskHandle);
	if(pdPASS != retVal) { /*error*/ }

	retVal = xTaskCreate(ButtonReadTask, "btnReadTask", BTN_READ_TASK_STACK_SIZE, NULL, 2, &buttonReadTaskHandle);
	if(pdPASS != retVal) { /*error*/ }

	CalculateSyncTimestamps(tempoBpm, syncTimestamps_100us);
	
	HAL_TIM_Base_Start_IT(&htim1);
	HAL_TIM_Base_Start_IT(&htim3);
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
		
		if(pdTRUE == xSemaphoreTake(rsrcMutex, portMAX_DELAY))
		{
			__disable_irq();
			blinkSyncCntLoc = blinkSyncCnt;
			blinkCntLoc_ms = blinkCnt_ms;
			__enable_irq();

			
			xTaskNotifyWait(0x0000, 0xFFFF, &notVal, 0);
			
			if (notVal & NOTIF_NEW_STEP_REACHED)
			{
				if(seqActive->pageCurr == seqActive->pageSel)
				{
					stepBlinkInProg = true;
				}
			}
			
			
			if((blinkSyncCntLoc != blinkSyncCntPrev) && (blinkSyncCntLoc % 24 == 0))
			{
				__disable_irq();
				blinkCnt_ms = 0;
				blinkCntLoc_ms = 0;
				__enable_irq();
			}
			
			memset(potLEDs, 0, 3*sizeof(bool));
			memset(seqLEDs, 0, 4*sizeof(bool));
			memset(miscLEDs, 0, 6*sizeof(bool));

			switch(modeCurr)
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
						seqLEDs[seqActive->id] = true;
					}

					// page select / misc
					miscLEDsBlink[seqActive->pageCurr->id] = true;
					for(int i=0; i<NUM_PAGES; i++)
					{
						if(btnPressed[BTN_SHIFT])
						{
							miscLEDs[i] = seqActive->patternCurr->pages[i].on;
						}
						else
						{
							miscLEDs[i] = seqActive->pageSel->id == seqActive->patternCurr->pages[i].id;
						}
					}

					// steps
					if(stepBlinkInProg)
					{
						stepLEDs[seqActive->iStepCurr] = !seqActive->pageSel->steps[seqActive->iStepCurr].on;
					}
					for(int i=0; i<16; i++)
					{
						if(!stepBlinkInProg)
						{
							stepLEDs[i] = seqActive->pageSel->steps[i].on;
						}
					}
					

				}
				break;

				case MODE_PITCH: // -----------------------------------------------------------
				{
					// page select / misc
					switch(seqActive->pageSel->steps[iStepSel].octOffs)
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
					stepLEDsBlink[iStepSel] = true;
					if(stepBlinkInProg)
					{
						stepLEDs[seqActive->iStepCurr] = !seqActive->pageSel->steps[seqActive->iStepCurr].on;
					}
					for(int i=0; i<16; i++)
					{
						if(!stepBlinkInProg)
						{
							stepLEDs[i] = seqActive->pageSel->steps[i].on;
						}
					}
				}
				break;

				case MODE_SET_CHANNEL: // -----------------------------------------------------------
				{
					// sequencer select 
					seqLEDs[seqActive->id] = true;

					// steps
					memset(stepLEDs, 0, NUM_STEPS);
					stepLEDs[seqActive->midiChannel] = true;
				}
				break;

				case MODE_COPY: // --------------------------------------------------------------------
				{
					// page select / misc
					if(menu.copySelected)
					{
						miscLEDsBlink[menu.pageSel->id] = true;
					}
				}
				break;
			}


			// potentiometer leds
			if(false == seqActive->gateInSync)
			{
				if(seqActive->gatePercent > gatePercent)
				{
					potLEDsBlink[1] = true;
				}
				else
				{
					potLEDs[1] = true;
				}
			}


			// ------- gate reached -----------
			if (seqActive->stepTimeCnt_ms >= 30)
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
					if((seqActive->pageCurr != seqActive->pageSel) || (iStepSel != seqActive->iStepCurr))
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

		xSemaphoreGive(rsrcMutex);

		vTaskDelay(1);
    }
}


void DispUpdateTask(void *)
{
	static char text[4] = {'C', 'H', 0, 0};

	while(1)
	{
		switch(modeCurr)
		{
			case MODE_DEFAULT:
			{
				tm1637_SetNumber(tempoBpm);
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
		gatePercent = MAX((uint8)SATURATE((((fabs((float)SATURATE(potBuffer.gate, 4040) - 4040.0))/4040.0)*100.0), 99.0), 1);

		if((seqActive->gatePercent >= gatePercent-5) && (seqActive->gatePercent <= gatePercent+5))
		{
			seqActive->gateInSync = true;
		}

		if(seqActive->gateInSync)
		{
			seqActive->gatePercent = gatePercent;
			seqActive->gateTime_ms = CalculateGateTime(seqActive->stepTime_ms, seqActive->gatePercent);
		}

		vTaskDelay(pdMS_TO_TICKS(10));
	}
}


void ButtonReadTask(void *)
{
	uint8 btnDelay[32] = {0};
	uint8 iBtn = 0;

	while(1)
	{
		// sample current state
		if(pdTRUE == xSemaphoreTake(rsrcMutex, portMAX_DELAY))
		{
			for(int i=0; i<4; i++)
			{
				HAL_GPIO_WritePin(btnSelect[i].port, btnSelect[i].pin, GPIO_PIN_SET);
			
				for(int j=0; j<8; j++)
				{
					iBtn = i*8 + j;	// button index

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
							btnDelay[iBtn] = BUTTON_DEBOUNCE_DELAY_MS;
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
						btnDelay[iBtn] = BUTTON_DEBOUNCE_DELAY_MS;
					}
					btnPressed[iBtn] = (bool)(HAL_GPIO_ReadPin(btnDetect[j].port, btnDetect[j].pin));
				}
				HAL_GPIO_WritePin(btnSelect[i].port, btnSelect[i].pin, GPIO_PIN_RESET);
			}
			xSemaphoreGive(rsrcMutex);
		} 

		vTaskDelay(pdMS_TO_TICKS(2));
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
		if(pdTRUE == xSemaphoreTake(rsrcMutex, portMAX_DELAY))
		{
			__disable_irq();
			syncCntLocal = syncCnt;
			__enable_irq();

			xTaskNotifyWait(0x0000, 0xFFFF, &notVal, 0);

			allSeqOff = !seq[0].on && !seq[1].on && !seq[2].on && !seq[3].on;
	
			if((notVal & NOTIF_GLOB_START_STOP) || allSeqOff)
			{
				if(globPlaying)
				{
					globStopFlag = true;
					DelayFuncUs(150);
				}
				else if(!globPlaying && (notVal & NOTIF_GLOB_START_STOP) && !allSeqOff)
				{
					ResetSequencer(&seq[0]);
					ResetSequencer(&seq[1]);
					ResetSequencer(&seq[2]);
					ResetSequencer(&seq[3]);
					globStartFlag = true;
					DelayFuncUs(150);	// wait for ISR to enter waiting loop
				}
			}
			if (notVal & NOTIF_ROT_INPUT_RIGHT)
			{
				switch(modeCurr)
				{
					case MODE_DEFAULT:
					case MODE_PITCH:
					IncrementBPM();
					break;

					case MODE_SET_CHANNEL:
					if(seqActive->id < 3)
					{
						seqActive = &seq[seqActive->id + 1];
					}
					break;
				}
			}
			if (notVal & NOTIF_ROT_INPUT_LEFT)
			{
				switch(modeCurr)
				{
					case MODE_DEFAULT:
					case MODE_PITCH:
					DecrementBPM();
					break;

					case MODE_SET_CHANNEL:
					if(seqActive->id > 0)
					{
						seqActive = &seq[seqActive->id - 1];
					}
					break;
				}
			}

			UART_Buf_ProcessRxBuffer();

			if(noteOnReceived)
			{
				switch(modeCurr)
				{
					case MODE_PITCH:
					noteOnReceived = false;
					__disable_irq();
					seqActive->pageSel->steps[iStepSel].pitch[0] = noteOnMsg[1];
					__enable_irq();
					break;

					default:
					noteOnReceived = false;
				}
			}
			
			// ----------------------- STEPPING -----------------------------
			if(globPlaying || globStartFlag)
			{
				if(((syncCntLocal != syncCntPrev) || globStartFlag) && !globStopFlag)	// sync counter updated or start initiated
				{
					for(int i=0; i<NUM_SEQUENCERS; i++)
					{
						if(globStartFlag)
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
							if(seqActive == &seq[i])
							{
								xTaskNotify(ledUpdateTaskHandle, NOTIF_NEW_STEP_REACHED, eSetBits);
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
				
				if(globStartFlag)
				{
					globStartFlag = false;
					globPlaying = true;
				}
				
				if(globStopFlag && allNotesOff)
				{
					globPlaying = false;
					globStopFlag = false;
				}
			}
			else if(!globPlaying)
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

			xSemaphoreGive(rsrcMutex);
		}
		syncCntPrev = syncCntLocal;
		
		vTaskDelay(pdMS_TO_TICKS(1));
	}
}


static void MenuReset()
{
	menu.pageSel = NULL;
	menu.seqSel = NULL;
	menu.stepSel = NULL;
	menu.copySelected = false;
}


void IncrementBPM()
{
	if(tempoBpm < 300)
	{
		tempoBpm++;
		if(0 == tempoChangeCooldown)
		{
			// TODO: make sure timer stays in sync in case of tempo change! BUG ALERT!
			__disable_irq();
			CalculateSyncTimestamps(tempoBpm, syncTimestamps_100us);
			bpmIncreased = true;
			__enable_irq();

			DelayFuncUs(150);

			for(int i=0; i<NUM_SEQUENCERS; i++)
			{
				seq[i].stepTime_ms = CalculateStepTime(tempoBpm, seq[i].syncEventsPerStep);
				seq[i].gateTime_ms = CalculateGateTime(seq[i].stepTime_ms, seq[i].gatePercent);
			}

			__disable_irq();
			tempoChangeCooldown = TEMPO_CHANGE_COOLDOWN_MS;
			__enable_irq();
		}
	}
}


void DecrementBPM()
{
	if(tempoBpm > 10)
	{
		tempoBpm--;
		if(0 == tempoChangeCooldown)
		{
			// TODO: make sure timer stays in sync in case of tempo change! BUG ALERT!
			__disable_irq();
			CalculateSyncTimestamps(tempoBpm, syncTimestamps_100us);
			__enable_irq();
	
			for(int i=0; i<NUM_SEQUENCERS; i++)
			{
				seq[i].stepTime_ms = CalculateStepTime(tempoBpm, seq[i].syncEventsPerStep);
				seq[i].gateTime_ms = CalculateGateTime(seq[i].stepTime_ms, seq[i].gatePercent);
			}
	
			__disable_irq();
			tempoChangeCooldown = TEMPO_CHANGE_COOLDOWN_MS;
			__enable_irq();
		}
	}
}


static void Step(SeqData_t* seq)
{
	seq->stepTimeCnt_ms = 0;

	switch(seq->stepRes)
	{
		case RES_1_4:
		case RES_1_8:
		case RES_1_16:
		{
			// TODO: add capability to handle pages shorter than 16 steps
			if(seq->iStepCurr == 15)
			{
				for(int i=seq->pageCurr->id; i<NUM_PAGES; i)
				{
					if(i < NUM_PAGES-1)
					{
						i++;
					}
					else
					{
						i=0;
					}

					if(seq->patternCurr->pages[i].on)
					{
						seq->pageCurr = &seq->patternCurr->pages[i];
						seq->iStepCurr = 0;
						break;
					}
				}
			}
			else
			{
				seq->iStepCurr++;
			}
			break;
		}
		case RES_1_3:
		case RES_1_6:
		{
			break;
		}
		default:{}
	} 
}


static void SendNoteONs(SeqData_t* seq)
{
	for(int i=0; i<(seq->pageCurr->steps[seq->iStepCurr].n_poly); i++)
	{
		Step_t* thisStep = &seq->pageCurr->steps[seq->iStepCurr];
		MIDI_SendNoteOn(thisStep->pitch[i] + 12*thisStep->octOffs, seq->midiChannel, 60, 0);
	}
}


static void SendNoteOFFs(SeqData_t* seq)
{
	for(int i=0; i<(seq->pageCurr->steps[seq->iStepCurr].n_poly); i++)
	{
		Step_t* thisStep = &seq->pageCurr->steps[seq->iStepCurr];
		MIDI_SendAllNotesOff(seq->midiChannel);
	}
}


static void ResetSequencer(SeqData_t* seq)
{
	// find first activated page
	for(int i=0; i<NUM_PAGES; i++)
	{
		if(seq->patternCurr->pages[i].on)
		{
			seq->pageCurr = &seq->patternCurr->pages[i];
			break;
		}
	}
	seq->iStepCurr = -1;
}


static void ButtonActivate(uint32 iBtn, bool shift, BtnEvent_e event)
{
	int32 offset = 12 * seqActive->pageSel->steps[iStepSel].octOffs;

	if(!shift)
	{
		if(iBtn < 16) // ------------------------------------------------------------------------
		{
			switch(modeCurr)
			{
				case MODE_DEFAULT:
				if(event == BTN_PUSHED)
				{
					seqActive->pageSel->steps[iBtn].on = !seqActive->pageSel->steps[iBtn].on;
				}
				break;

				case MODE_PITCH:
				if(event == BTN_PUSHED)
				{
					iStepSel = iBtn;
				}
				break;

				case MODE_SET_CHANNEL:
				if(event == BTN_PUSHED)
				{
					MIDI_SendAllNotesOff(seqActive->midiChannel);
					seqActive->midiChannel = iBtn;
				}
				break;
			}
		}
		else if(iBtn == BTN_SEQ1) // ------------------------------------------------------------
		{
			switch(modeCurr)
			{
				case MODE_DEFAULT:
				if(event == BTN_PUSHED)
				{
					seqActive->gateInSync = false;
					seqActive = &seq[0];
				}
				break;

				case MODE_PITCH:
				case MODE_SET_CHANNEL:
				if(event == BTN_PUSHED)
				{
					modeCurr = MODE_DEFAULT;
				}
				break;
			}
		}
		else if(iBtn == BTN_SEQ2) // ------------------------------------------------------------
		{
			switch(modeCurr)
			{
				case MODE_DEFAULT:
				if(event == BTN_PUSHED)
				{
					seqActive->gateInSync = false;
					seqActive = &seq[1];
				}
				break;
			}
		}
		else if(iBtn == BTN_SEQ3) // ------------------------------------------------------------
		{
			switch(modeCurr)
			{
				case MODE_DEFAULT:
				if(event == BTN_PUSHED)
				{
					seqActive->gateInSync = false;
					seqActive = &seq[2];
				}
				break;

				case MODE_PITCH:
				if(event == BTN_PUSHED)
				{
					seqActive->pageSel->steps[iStepSel].octOffs++;
				}
				break;
			}
		}
		else if(iBtn == BTN_DRUM) // ------------------------------------------------------------
		{
			switch(modeCurr)
			{
				case MODE_DEFAULT:
				if(event == BTN_PUSHED)
				{
					seqActive->gateInSync = false;
					seqActive = &seq[3];
				}
				break;
				
				case MODE_PITCH:
				if(event == BTN_PUSHED)
				{
					seqActive->pageSel->steps[iStepSel].octOffs--;
				}
				break;
			}
		}
		else if(iBtn == BTN_PAGE1) // ------------------------------------------------------------
		{
			switch(modeCurr)
			{
				case MODE_DEFAULT:
				if(event == BTN_PUSHED)
				{
					seqActive->pageSel = &seqActive->patternCurr->pages[0];
				}
				break;
				
				case MODE_PITCH:
				if(event == BTN_RELEASED)
				{
					seqActive->pageSel->steps[iStepSel].pitch[0] = seqActive->rootNote + 6;
				}
				break;

				case MODE_COPY:
				if(event == BTN_PUSHED)
				{
					if(!menu.copySelected)
					{
						menu.pageSel = &seqActive->patternCurr->pages[0];
						menu.copySelected = true;
					}
					else
					{
						if(seqActive->pageCurr == menu.pageSel)
						{
							MIDI_SendAllNotesOff(seqActive->midiChannel);
						}
						memcpy(&seqActive->patternCurr->pages[0].steps, menu.pageSel->steps, NUM_STEPS*sizeof(Step_t));
						menu.pageSel = NULL;
						menu.copySelected = false;
						modeCurr = MODE_DEFAULT;
					}
				}
				break;
			}
			
		}
		else if(iBtn == BTN_PAGE2) // ------------------------------------------------------------
		{
			switch(modeCurr)
			{
				case MODE_DEFAULT:
				if(event == BTN_PUSHED)
				{
					seqActive->pageSel = &seqActive->patternCurr->pages[1];
				}
				break;
				
				case MODE_PITCH:
				if(event == BTN_RELEASED)
				{
					seqActive->pageSel->steps[iStepSel].pitch[0] = seqActive->rootNote + 7;
				}
				break;

				case MODE_COPY:
				if(event == BTN_PUSHED)
				{
					if(!menu.copySelected)
					{
						menu.pageSel = &seqActive->patternCurr->pages[1];
						menu.copySelected = true;
					}
					else
					{
						if(seqActive->pageCurr == menu.pageSel)
						{
							MIDI_SendAllNotesOff(seqActive->midiChannel);
						}
						memcpy(&seqActive->patternCurr->pages[1].steps, menu.pageSel->steps, NUM_STEPS*sizeof(Step_t));
						menu.pageSel = NULL;
						menu.copySelected = false;
						modeCurr = MODE_DEFAULT;
					}
				}
				break;
			}
		}
		else if(iBtn == BTN_PAGE3) // ------------------------------------------------------------
		{
			switch(modeCurr)
			{
				case MODE_DEFAULT:
				if(event == BTN_PUSHED)
				{
					seqActive->pageSel = &seqActive->patternCurr->pages[2];
					menu.copySelected = true;
				}
				break;
				
				case MODE_PITCH:
				if(event == BTN_RELEASED)
				{
					seqActive->pageSel->steps[iStepSel].pitch[0] = seqActive->rootNote + 8;
				}
				break;

				case MODE_COPY:
				if(event == BTN_PUSHED)
				{
					if(!menu.copySelected)
					{
						menu.pageSel = &seqActive->patternCurr->pages[2];
						menu.copySelected = true;
					}
					else
					{
						if(seqActive->pageCurr == menu.pageSel)
						{
							MIDI_SendAllNotesOff(seqActive->midiChannel);
						}
						memcpy(&seqActive->patternCurr->pages[2].steps, menu.pageSel->steps, NUM_STEPS*sizeof(Step_t));
						menu.pageSel = NULL;
						menu.copySelected = false;
						modeCurr = MODE_DEFAULT;
					}
				}
				break;
			}
		}
		else if(iBtn == BTN_PAGE4) // ------------------------------------------------------------
		{
			switch(modeCurr)
			{
				case MODE_DEFAULT:
				if(event == BTN_PUSHED)
				{
					seqActive->pageSel = &seqActive->patternCurr->pages[3];
				}
				break;
				
				case MODE_PITCH:
				if(event == BTN_RELEASED)
				{
					seqActive->pageSel->steps[iStepSel].pitch[0] = seqActive->rootNote + 9;
				}
				break;

				case MODE_COPY:
				if(event == BTN_PUSHED)
				{
					if(!menu.copySelected)
					{
						menu.pageSel = &seqActive->patternCurr->pages[3];
						menu.copySelected = true;
					}
					else
					{
						if(seqActive->pageCurr == menu.pageSel)
						{
							MIDI_SendAllNotesOff(seqActive->midiChannel);
						}
						memcpy(&seqActive->patternCurr->pages[3].steps, menu.pageSel->steps, NUM_STEPS*sizeof(Step_t));
						menu.pageSel = NULL;
						menu.copySelected = false;
						modeCurr = MODE_DEFAULT;
					}
				}
				break;
			}
		}
		else if(iBtn == 22) // ------------------------------------------------------------
		{
			switch(modeCurr)
			{
				case MODE_DEFAULT:
				if(event == BTN_PUSHED)
				{
				}
				break;
				
				case MODE_PITCH:
				if(event == BTN_RELEASED)
				{
					seqActive->pageSel->steps[iStepSel].pitch[0] = seqActive->rootNote + 10;
				}
				break;
			}
		}
		else if(iBtn == 23) // ------------------------------------------------------------
		{
			switch(modeCurr)
			{
				case MODE_DEFAULT:
				if(event == BTN_PUSHED)
				{
				}
				break;
				
				case MODE_PITCH:
				if(event == BTN_RELEASED)
				{
					seqActive->pageSel->steps[iStepSel].pitch[0] = seqActive->rootNote + 11;
				}
				break;
			}
		}
		else if(iBtn == BTN_SHIFT) // ------------------------------------------------------------
		{
			switch(modeCurr)
			{
				case MODE_DEFAULT:
				if(event == BTN_PUSHED)
				{
				}
				break;
				
				case MODE_PITCH:
				if(event == BTN_RELEASED)
				{
					seqActive->pageSel->steps[iStepSel].pitch[0] = seqActive->rootNote;
				}
				break;
			}
		}
		else if(iBtn == 27) // ------------------------------------------------------------
		{
			switch(modeCurr)
			{
				case MODE_DEFAULT:
				if(event == BTN_PUSHED)
				{
				}
				break;
				
				case MODE_PITCH:
				if(event == BTN_RELEASED)
				{
					seqActive->pageSel->steps[iStepSel].pitch[0] = seqActive->rootNote + 1;
				}
				break;
			}
		}
		else if(iBtn == 28) // ------------------------------------------------------------
		{
			switch(modeCurr)
			{
				case MODE_DEFAULT:
				if(event == BTN_PUSHED)
				{
				}
				break;

				case MODE_PITCH:
				if(event == BTN_RELEASED)
				{
					seqActive->pageSel->steps[iStepSel].pitch[0] = seqActive->rootNote + 2;
				}
			}
		}
		else if(iBtn == 29) // ------------------------------------------------------------
		{
			switch(modeCurr)
			{
				case MODE_DEFAULT:
				if(event == BTN_PUSHED)
				{
					modeCurr = MODE_COPY;
				}
				break;

				case MODE_PITCH:
				if(event == BTN_RELEASED)
				{
					seqActive->pageSel->steps[iStepSel].pitch[0] = seqActive->rootNote + 3;
				}
			}
		}
		else if(BTN_PITCH == iBtn) // ------------------------------------------------------------
		{
			switch(modeCurr)
			{
				case MODE_DEFAULT:
				if(event == BTN_PUSHED)
				{
					modeCurr = MODE_PITCH;
					modeChanged = true;
				}
				break;
				
				case MODE_PITCH:
				if(event == BTN_RELEASED)
				{
					if(modeChanged)
					{
						modeChanged = false;
					}
					else
					{
						seqActive->pageSel->steps[iStepSel].pitch[0] = seqActive->rootNote + 4;
					}
				}
				break;
			}
		}
		else if(BTN_START_STOP == iBtn) // ------------------------------------------------------------
		{
			switch(modeCurr)
			{
				case MODE_DEFAULT:
				if(event == BTN_PUSHED)
				{
					xTaskNotify(mainTaskHandle, NOTIF_GLOB_START_STOP, eSetBits);
				}
				break;
				
				case MODE_PITCH:
				if(event == BTN_RELEASED)
				{
					seqActive->pageSel->steps[iStepSel].pitch[0] = seqActive->rootNote + 5;
				}
				break;
			}
		}
	}
	else if(shift) // ========================================   S H I F T     K E Y S   =====================================================
	{
		if(iBtn == 15) // ------------------------------------------------------------------------
		{
			switch(modeCurr)
			{
				case MODE_DEFAULT:
				if(event == BTN_PUSHED)
				{
					if(modeCurr == MODE_DEFAULT)
					{
						modeCurr = MODE_SET_CHANNEL;
					}
					if(modeCurr == MODE_SET_CHANNEL)
					{
						modeCurr == MODE_DEFAULT;
					}
				}
				break;
			}
		}
		else if(iBtn == BTN_SEQ1) // ------------------------------------------------------------
		{
			switch(modeCurr)
			{
				case MODE_DEFAULT:
				if(event == BTN_PUSHED)
				{
					if(false == seq[0].on)	seq[0].onFlag = true;
					else					seq[0].offFlag = true;
				}
				break;

				case MODE_SET_CHANNEL:
				if(event == BTN_PUSHED)
				{
					seqActive->gateInSync = false;
					seqActive = &seq[0];
				}
				break;
			}
		}
		else if(iBtn == BTN_SEQ2) // ------------------------------------------------------------
		{
			switch(modeCurr)
			{
				case MODE_DEFAULT:
				if(event == BTN_PUSHED)
				{
					if(false == seq[1].on)	seq[1].onFlag = true;
					else					seq[1].offFlag = true;
				}
				break;

				case MODE_SET_CHANNEL:
				if(event == BTN_PUSHED)
				{
					seqActive->gateInSync = false;
					seqActive = &seq[1];
				}
				break;
			}
		}
		else if(iBtn == BTN_SEQ3) // ------------------------------------------------------------
		{
			switch(modeCurr)
			{
				case MODE_DEFAULT:
				if(event == BTN_PUSHED)
				{
					if(false == seq[2].on)	seq[2].onFlag = true;
					else					seq[2].offFlag = true;
				}
				break;

				case MODE_SET_CHANNEL:
				if(event == BTN_PUSHED)
				{
					seqActive->gateInSync = false;
					seqActive = &seq[2];
				}
				break;
			}
		}
		else if(iBtn == BTN_DRUM) // ------------------------------------------------------------
		{
			switch(modeCurr)
			{
				case MODE_DEFAULT:
				if(event == BTN_PUSHED)
				{
					if(false == seq[3].on)	seq[3].onFlag = true;
					else					seq[3].offFlag = true;
				}
				break;

				case MODE_SET_CHANNEL:
				if(event == BTN_PUSHED)
				{
					seqActive->gateInSync = false;
					seqActive = &seq[3];
				}
				break;
			}
		}
		else if(iBtn == BTN_PAGE1) // ------------------------------------------------------------
		{
			switch(modeCurr)
			{
				case MODE_DEFAULT:
				if(event == BTN_PUSHED)
				{
					if(seqActive->patternCurr->pages[0].on)
					{
						if(seqActive->patternCurr->numPagesOn > 1)
						{
							seqActive->patternCurr->pages[0].on = false;
							seqActive->patternCurr->numPagesOn--;
						}
					}
					else
					{
						seqActive->patternCurr->pages[0].on = true;
						seqActive->patternCurr->numPagesOn++;
					}
				}
				break;
			}
		}
		else if(iBtn == BTN_PAGE2) // ------------------------------------------------------------
		{
			switch(modeCurr)
			{
				case MODE_DEFAULT:
				if(event == BTN_PUSHED)
				{
					if(seqActive->patternCurr->pages[1].on)
					{
						if(seqActive->patternCurr->numPagesOn > 1)
						{
							seqActive->patternCurr->pages[1].on = false;
							seqActive->patternCurr->numPagesOn--;
						}
					}
					else
					{
						seqActive->patternCurr->pages[1].on = true;
						seqActive->patternCurr->numPagesOn++;
					}
				}
				break;
			}
		}
		else if(iBtn == BTN_PAGE3) // ------------------------------------------------------------
		{
			switch(modeCurr)
			{
				case MODE_DEFAULT:
				if(event == BTN_PUSHED)
				{
					if(seqActive->patternCurr->pages[2].on)
					{
						if(seqActive->patternCurr->numPagesOn > 1)
						{
							seqActive->patternCurr->pages[2].on = false;
							seqActive->patternCurr->numPagesOn--;
						}
					}
					else
					{
						seqActive->patternCurr->pages[2].on = true;
						seqActive->patternCurr->numPagesOn++;
					}
				}
				break;
			}
		}
		else if(iBtn == BTN_PAGE4) // ------------------------------------------------------------
		{
			switch(modeCurr)
			{
				case MODE_DEFAULT:
				if(event == BTN_PUSHED)
				{
					if(seqActive->patternCurr->pages[3].on)
					{
						if(seqActive->patternCurr->numPagesOn > 1)
						{
							seqActive->patternCurr->pages[3].on = false;
							seqActive->patternCurr->numPagesOn--;
						}
					}
					else
					{
						seqActive->patternCurr->pages[3].on = true;
						seqActive->patternCurr->numPagesOn++;
					}
				}
				break;
			}
		}
		else if(iBtn == BTN_SHIFT) // ------------------------------------------------------------
		{
			switch(modeCurr)
			{
				case MODE_DEFAULT:
				if(event == BTN_PUSHED)
				{
				}
				break;
				
				case MODE_PITCH:
				if(event == BTN_RELEASED)
				{
					seqActive->pageSel->steps[iStepSel].pitch[0] = seqActive->rootNote;
				}
				break;
			}
		}
	}	
}