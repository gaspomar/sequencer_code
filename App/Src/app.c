#include "app.h"
#include "main.h"
#include "tim.h"
#include "adc.h"
#include "tasks.h"
#include "led.h"
#include "uart_buffer.h"
#include "midi.h"
#include "menu.h"
#include "tm1637.h"

volatile AppState_t app;
volatile SeqData_t seq[4];


void IncrementBPM()
{
	if(app.tempoBpm < 300)
	{
		app.tempoBpm++;
		if(0 == tempoChangeCooldown)
		{
			// TODO: make sure timer stays in sync in case of tempo change! BUG ALERT!
			__disable_irq();
			CalculateSyncTimestamps(app.tempoBpm, app.syncTimestamps_100us);
			app.bpmIncreased = true;
			__enable_irq();

			DelayFuncUs(150);

			for(int i=0; i<NUM_SEQUENCERS; i++)
			{
				seq[i].stepTime_ms = CalculateStepTime(app.tempoBpm, seq[i].syncEventsPerStep);
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
	if(app.tempoBpm > 10)
	{
		app.tempoBpm--;
		if(0 == tempoChangeCooldown)
		{
			// TODO: make sure timer stays in sync in case of tempo change! BUG ALERT!
			__disable_irq();
			CalculateSyncTimestamps(app.tempoBpm, app.syncTimestamps_100us);
			__enable_irq();
	
			for(int i=0; i<NUM_SEQUENCERS; i++)
			{
				seq[i].stepTime_ms = CalculateStepTime(app.tempoBpm, seq[i].syncEventsPerStep);
				seq[i].gateTime_ms = CalculateGateTime(seq[i].stepTime_ms, seq[i].gatePercent);
			}
	
			__disable_irq();
			tempoChangeCooldown = TEMPO_CHANGE_COOLDOWN_MS;
			__enable_irq();
		}
	}
}


void Step(SeqData_t* seq)
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


void SendNoteONs(SeqData_t* seq)
{
	for(int i=0; i<(seq->pageCurr->steps[seq->iStepCurr].n_poly); i++)
	{
		Step_t* thisStep = &seq->pageCurr->steps[seq->iStepCurr];
		MIDI_SendNoteOn(thisStep->pitch[i] + 12*thisStep->octOffs, seq->midiChannel, 60, 0);
	}
}


void SendNoteOFFs(SeqData_t* seq)
{
	for(int i=0; i<(seq->pageCurr->steps[seq->iStepCurr].n_poly); i++)
	{
		MIDI_SendAllNotesOff(seq->midiChannel);
	}
}


void ResetSequencer(SeqData_t* seq)
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


void InitApp()
{
    app.tempoBpm = 100;
    app.modeCurr = MODE_DEFAULT;
    app.globPlaying = false;

	app.seqActive = &seq[0];

    app.globStopFlag = false;
    app.glob_start_flag = false;
    app.bpmIncreased = false;
    app.modeChanged = false;
    app.noteOn = false;
    app.blinkCnt_ms = 0;
    app.blinkSyncCnt = 0;
    app.iStepSel = 0;

	Menu_Reset();

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
		seq[i].stepTime_ms = CalculateStepTime(app.tempoBpm, seq[i].syncEventsPerStep);
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

	Menu_Reset();

	HAL_GPIO_WritePin(DBG_A6_GPIO_Port, DBG_A6_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(DBG_GPIO_Port, DBG_Pin, GPIO_PIN_RESET);

	// peripherals
	HAL_TIM_Base_Start(&htim2);
	
	UART_Buf_Init();
	UART_Buf_Receive();
	
	HAL_ADC_Start_DMA(&hadc1, (uint32*)&app.potBuffer, 3);
	
	tm1637_SetUsDelayFunction(DelayFuncUs);
	
	// welcome animation
	tm1637_SetDefault();
	
	LED_ClearAll();
	LED_AnimationWelcome();
	HAL_Delay(700);
	LED_ClearAll();
	HAL_Delay(500);
	
	tm1637_SetNumber(app.tempoBpm);
	
	// TODO: TM1637 timeout
	InitOSData();

	CalculateSyncTimestamps(app.tempoBpm, app.syncTimestamps_100us);
	
	HAL_TIM_Base_Start_IT(&htim1);
	HAL_TIM_Base_Start_IT(&htim3);
}