#include "main.h"
#include "buttons.h"
#include "app.h"
#include "menu.h"
#include "tasks.h"
#include "midi.h"

#include <string.h>


const GPIO_t btnSelect[4] = 
{
	{BTN_S0_GPIO_Port, BTN_S0_Pin},
	{BTN_S1_GPIO_Port, BTN_S1_Pin},
	{BTN_S2_GPIO_Port, BTN_S2_Pin},
	{BTN_S3_GPIO_Port, BTN_S3_Pin}
};

const GPIO_t btnDetect[8] = 
{
	{BTN_D0_GPIO_Port, BTN_D0_Pin},
	{BTN_D1_GPIO_Port, BTN_D1_Pin},
	{BTN_D2_GPIO_Port, BTN_D2_Pin},
	{BTN_D3_GPIO_Port, BTN_D3_Pin},
	{BTN_D4_GPIO_Port, BTN_D4_Pin},
	{BTN_D5_GPIO_Port, BTN_D5_Pin},
	{BTN_D6_GPIO_Port, BTN_D6_Pin},
	{BTN_D7_GPIO_Port, BTN_D7_Pin}
};

volatile uint8 btnDelay_ms[32] = {0};

volatile bool btnPressed[32] = {0};



void ButtonActivate(uint32 iBtn, bool shift, BtnEvent_e event)
{
	int32 offset = 12 * app.seqActive->pageSel->steps[app.iStepSel].octOffs;

	if(!shift)
	{
		if(iBtn < 16) // ------------------------------------------------------------------------
		{
			switch(app.modeCurr)
			{
				case MODE_DEFAULT:
				if(event == BTN_PUSHED)
				{
					if(menu.state == MENU_IDLE)
					{
						menu.state = MENU_LISTEN_ON_NOTE;
						menu.seqSel = app.seqActive;
						menu.stepSel = &app.seqActive->pageSel->steps[iBtn];
						menu.iStepSel = iBtn;
					}
				}
				else if(event == BTN_RELEASED)
				{
					// TODO: check if this could be differently (it shouldn"t)
					if(menu.stepSel == &app.seqActive->pageSel->steps[iBtn])
					{
						// no note received, step should be inverted
						if(menu.state == MENU_LISTEN_ON_NOTE)
						{
							menu.state = MENU_IDLE;
							menu.stepSel->on = !menu.stepSel->on;
							Menu_Reset();

						}
					}
				}
				break;

				case MODE_PITCH:
				if(event == BTN_PUSHED)
				{
					app.iStepSel = iBtn;
				}
				break;

				case MODE_SET_CHANNEL:
				if(event == BTN_PUSHED)
				{
					MIDI_SendAllNotesOff(app.seqActive->midiChannel);
					app.seqActive->midiChannel = iBtn;
				}
				break;

				case MODE_COPY:
				if(event == BTN_PUSHED)
				{
					if(menu.state == MENU_IDLE)
					{
						menu.state = MENU_COPY_STEPS;
						menu.seqSel = app.seqActive;
						menu.stepSel = &app.seqActive->pageSel->steps[iBtn];
						menu.iStepSel = iBtn;
					}
					// ending step selected
					else if(menu.state == MENU_COPY_STEPS)
					{
						if(menu.iStepSel < iBtn)
						{
							menu.numSelected = iBtn - menu.iStepSel + 1;
							menu.state = MENU_PASTE_STEPS;
						}
						else
						{
							Menu_Reset();
							app.modeCurr = MODE_DEFAULT;
						}
					}
					else if(menu.state == MENU_PASTE_STEPS)
					{
						memcpy(&app.seqActive->pageSel->steps[iBtn], menu.stepSel, menu.numSelected*sizeof(Step_t));
						Menu_Reset();
						app.modeCurr = MODE_DEFAULT;
					}
				}
				else if(event == BTN_RELEASED)
				{
					if(menu.state == MENU_COPY_STEPS)
					{
						menu.state = MENU_PASTE_STEPS;
						menu.numSelected = 1;
					}
				}
				break;

				default:{}
			}
		}
		else if(iBtn == BTN_SEQ1) // ------------------------------------------------------------
		{
			switch(app.modeCurr)
			{
				case MODE_DEFAULT:
				if(event == BTN_PUSHED)
				{
					app.seqActive->gateInSync = false;
					app.seqActive = &seq[0];
				}
				break;

				case MODE_PITCH:
				case MODE_SET_CHANNEL:
				case MODE_COPY:
				if(event == BTN_PUSHED)
				{
					app.modeCurr = MODE_DEFAULT;
					Menu_Reset();
				}
				break;

				default:{}
			}
		}
		else if(iBtn == BTN_SEQ2) // ------------------------------------------------------------
		{
			switch(app.modeCurr)
			{
				case MODE_DEFAULT:
				if(event == BTN_PUSHED)
				{
					app.seqActive->gateInSync = false;
					app.seqActive = &seq[1];
				}
				break;

				default:{}
			}
		}
		else if(iBtn == BTN_SEQ3) // ------------------------------------------------------------
		{
			switch(app.modeCurr)
			{
				case MODE_DEFAULT:
				if(event == BTN_PUSHED)
				{
					app.seqActive->gateInSync = false;
					app.seqActive = &seq[2];
				}
				break;

				case MODE_PITCH:
				if(event == BTN_PUSHED)
				{
					app.seqActive->pageSel->steps[app.iStepSel].octOffs++;
				}
				break;

				default:{}
			}
		}
		else if(iBtn == BTN_DRUM) // ------------------------------------------------------------
		{
			switch(app.modeCurr)
			{
				case MODE_DEFAULT:
				if(event == BTN_PUSHED)
				{
					app.seqActive->gateInSync = false;
					app.seqActive = &seq[3];
				}
				break;
				
				case MODE_PITCH:
				if(event == BTN_PUSHED)
				{
					app.seqActive->pageSel->steps[app.iStepSel].octOffs--;
				}
				break;

				default:{}
			}
		}
		else if(iBtn == BTN_PAGE1) // ------------------------------------------------------------
		{
			switch(app.modeCurr)
			{
				case MODE_DEFAULT:
				if(event == BTN_PUSHED)
				{
					app.seqActive->pageSel = &app.seqActive->patternCurr->pages[0];
				}
				break;
				
				case MODE_PITCH:
				if(event == BTN_RELEASED)
				{
					app.seqActive->pageSel->steps[app.iStepSel].pitch[0] = app.seqActive->rootNote + 6;
				}
				break;

				case MODE_COPY:
				if(event == BTN_PUSHED)
				{
					if(menu.state == MENU_IDLE)
					{
						menu.state = MENU_COPY_PAGES;
						menu.pageSel = &app.seqActive->patternCurr->pages[0];
					}
					else if(menu.state == MENU_COPY_PAGES)
					{
						if(app.seqActive->pageCurr == menu.pageSel)
						{
							MIDI_SendAllNotesOff(app.seqActive->midiChannel);
						}
						memcpy(app.seqActive->patternCurr->pages[0].steps, menu.pageSel->steps, NUM_STEPS*sizeof(Step_t));
						Menu_Reset();
						app.modeCurr = MODE_DEFAULT;
					}
				}
				break;

				default:{}
			}
			
		}
		else if(iBtn == BTN_PAGE2) // ------------------------------------------------------------
		{
			switch(app.modeCurr)
			{
				case MODE_DEFAULT:
				if(event == BTN_PUSHED)
				{
					app.seqActive->pageSel = &app.seqActive->patternCurr->pages[1];
				}
				break;
				
				case MODE_PITCH:
				if(event == BTN_RELEASED)
				{
					app.seqActive->pageSel->steps[app.iStepSel].pitch[0] = app.seqActive->rootNote + 7;
				}
				break;

				case MODE_COPY:
				if(event == BTN_PUSHED)
				{
					if(menu.state == MENU_IDLE)
					{
						menu.state = MENU_COPY_PAGES;
						menu.pageSel = &app.seqActive->patternCurr->pages[1];
					}
					else if(menu.state == MENU_COPY_PAGES)
					{
						if(app.seqActive->pageCurr == menu.pageSel)
						{
							MIDI_SendAllNotesOff(app.seqActive->midiChannel);
						}
						memcpy(app.seqActive->patternCurr->pages[1].steps, menu.pageSel->steps, NUM_STEPS*sizeof(Step_t));
						Menu_Reset();
						app.modeCurr = MODE_DEFAULT;
					}
				}
				break;

				default:{}
			}
		}
		else if(iBtn == BTN_PAGE3) // ------------------------------------------------------------
		{
			switch(app.modeCurr)
			{
				case MODE_DEFAULT:
				if(event == BTN_PUSHED)
				{
					app.seqActive->pageSel = &app.seqActive->patternCurr->pages[2];
				}
				break;
				
				case MODE_PITCH:
				if(event == BTN_RELEASED)
				{
					app.seqActive->pageSel->steps[app.iStepSel].pitch[0] = app.seqActive->rootNote + 8;
				}
				break;

				case MODE_COPY:
				if(event == BTN_PUSHED)
				{
					if(menu.state == MENU_IDLE)
					{
						menu.state = MENU_COPY_PAGES;
						menu.pageSel = &app.seqActive->patternCurr->pages[2];
					}
					else if(menu.state == MENU_COPY_PAGES)
					{
						if(app.seqActive->pageCurr == menu.pageSel)
						{
							MIDI_SendAllNotesOff(app.seqActive->midiChannel);
						}
						memcpy(app.seqActive->patternCurr->pages[2].steps, menu.pageSel->steps, NUM_STEPS*sizeof(Step_t));
						Menu_Reset();
						app.modeCurr = MODE_DEFAULT;
					}
				}
				break;

				default:{}
			}
		}
		else if(iBtn == BTN_PAGE4) // ------------------------------------------------------------
		{
			switch(app.modeCurr)
			{
				case MODE_DEFAULT:
				if(event == BTN_PUSHED)
				{
					app.seqActive->pageSel = &app.seqActive->patternCurr->pages[3];
				}
				break;
				
				case MODE_PITCH:
				if(event == BTN_RELEASED)
				{
					app.seqActive->pageSel->steps[app.iStepSel].pitch[0] = app.seqActive->rootNote + 9;
				}
				break;

				case MODE_COPY:
				if(event == BTN_PUSHED)
				{
					if(menu.state == MENU_IDLE)
					{
						menu.state = MENU_COPY_PAGES;
						menu.pageSel = &app.seqActive->patternCurr->pages[3];
					}
					else if(menu.state == MENU_COPY_PAGES)
					{
						if(app.seqActive->pageCurr == menu.pageSel)
						{
							MIDI_SendAllNotesOff(app.seqActive->midiChannel);
						}
						memcpy(app.seqActive->patternCurr->pages[3].steps, menu.pageSel->steps, NUM_STEPS*sizeof(Step_t));
						Menu_Reset();
						app.modeCurr = MODE_DEFAULT;
					}
				}
				break;

				default:{}
			}
		}
		else if(iBtn == 22) // ------------------------------------------------------------
		{
			switch(app.modeCurr)
			{
				case MODE_DEFAULT:
				if(event == BTN_PUSHED)
				{
				}
				break;
				
				case MODE_PITCH:
				if(event == BTN_RELEASED)
				{
					app.seqActive->pageSel->steps[app.iStepSel].pitch[0] = app.seqActive->rootNote + 10;
				}
				break;

				default:{}
			}
		}
		else if(iBtn == 23) // ------------------------------------------------------------
		{
			switch(app.modeCurr)
			{
				case MODE_DEFAULT:
				if(event == BTN_PUSHED)
				{
				}
				break;
				
				case MODE_PITCH:
				if(event == BTN_RELEASED)
				{
					app.seqActive->pageSel->steps[app.iStepSel].pitch[0] = app.seqActive->rootNote + 11;
				}
				break;

				default:{}
			}
		}
		else if(iBtn == BTN_SHIFT) // ------------------------------------------------------------
		{
			switch(app.modeCurr)
			{
				case MODE_DEFAULT:
				if(event == BTN_PUSHED)
				{
				}
				break;
				
				case MODE_PITCH:
				if(event == BTN_RELEASED)
				{
					app.seqActive->pageSel->steps[app.iStepSel].pitch[0] = app.seqActive->rootNote;
				}
				break;

				default:{}
			}
		}
		else if(iBtn == 27) // ------------------------------------------------------------
		{
			switch(app.modeCurr)
			{
				case MODE_DEFAULT:
				if(event == BTN_PUSHED)
				{
				}
				break;
				
				case MODE_PITCH:
				if(event == BTN_RELEASED)
				{
					app.seqActive->pageSel->steps[app.iStepSel].pitch[0] = app.seqActive->rootNote + 1;
				}
				break;

				default:{}
			}
		}
		else if(iBtn == 28) // ------------------------------------------------------------
		{
			switch(app.modeCurr)
			{
				case MODE_DEFAULT:
				if(event == BTN_PUSHED)
				{
					app.modeCurr = MODE_ERASE;
				}
				break;

				case MODE_PITCH:
				if(event == BTN_RELEASED)
				{
					app.seqActive->pageSel->steps[app.iStepSel].pitch[0] = app.seqActive->rootNote + 2;
				}
				break;

				default:{}
			}
		}
		else if(iBtn == 29) // ------------------------------------------------------------
		{
			switch(app.modeCurr)
			{
				case MODE_DEFAULT:
				if(event == BTN_PUSHED)
				{
					app.modeCurr = MODE_COPY;
				}
				break;

				case MODE_PITCH:
				if(event == BTN_RELEASED)
				{
					app.seqActive->pageSel->steps[app.iStepSel].pitch[0] = app.seqActive->rootNote + 3;
				}
				break;

				default:{}
			}
		}
		else if(BTN_PITCH == iBtn) // ------------------------------------------------------------
		{
			switch(app.modeCurr)
			{
				case MODE_DEFAULT:
				if(event == BTN_PUSHED)
				{
					app.modeCurr = MODE_PITCH;
					app.modeChanged = true;
				}
				break;
				
				case MODE_PITCH:
				if(event == BTN_RELEASED)
				{
					if(app.modeChanged)
					{
						app.modeChanged = false;
					}
					else
					{
						app.seqActive->pageSel->steps[app.iStepSel].pitch[0] = app.seqActive->rootNote + 4;
					}
				}
				break;

				default:{}
			}
		}
		else if(BTN_START_STOP == iBtn) // ------------------------------------------------------------
		{
			switch(app.modeCurr)
			{
				case MODE_DEFAULT:
				if(event == BTN_PUSHED)
				{
					xTaskNotify(os.mainTaskHandle, NOTIF_GLOB_START_STOP, eSetBits);
				}
				break;
				
				case MODE_PITCH:
				if(event == BTN_RELEASED)
				{
					app.seqActive->pageSel->steps[app.iStepSel].pitch[0] = app.seqActive->rootNote + 5;
				}
				break;

				default:{}
			}
		}
	}
	else if(shift) // ========================================   S H I F T     K E Y S   =====================================================
	{
		if(iBtn == 15) // ------------------------------------------------------------------------
		{
			switch(app.modeCurr)
			{
				case MODE_DEFAULT:
				if(event == BTN_PUSHED)
				{
					if(app.modeCurr == MODE_DEFAULT)
					{
						app.modeCurr = MODE_SET_CHANNEL;
					}
					if(app.modeCurr == MODE_SET_CHANNEL)
					{
						app.modeCurr == MODE_DEFAULT;
					}
				}
				break;

				default:{}
			}
		}
		else if(iBtn == BTN_SEQ1) // ------------------------------------------------------------
		{
			switch(app.modeCurr)
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
					app.seqActive->gateInSync = false;
					app.seqActive = &seq[0];
				}
				break;

				default:{}
			}
		}
		else if(iBtn == BTN_SEQ2) // ------------------------------------------------------------
		{
			switch(app.modeCurr)
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
					app.seqActive->gateInSync = false;
					app.seqActive = &seq[1];
				}
				break;

				default:{}
			}
		}
		else if(iBtn == BTN_SEQ3) // ------------------------------------------------------------
		{
			switch(app.modeCurr)
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
					app.seqActive->gateInSync = false;
					app.seqActive = &seq[2];
				}
				break;

				default:{}
			}
		}
		else if(iBtn == BTN_DRUM) // ------------------------------------------------------------
		{
			switch(app.modeCurr)
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
					app.seqActive->gateInSync = false;
					app.seqActive = &seq[3];
				}
				break;

				default:{}
			}
		}
		else if(iBtn == BTN_PAGE1) // ------------------------------------------------------------
		{
			switch(app.modeCurr)
			{
				case MODE_DEFAULT:
				if(event == BTN_PUSHED)
				{
					if(app.seqActive->patternCurr->pages[0].on)
					{
						if(app.seqActive->patternCurr->numPagesOn > 1)
						{
							app.seqActive->patternCurr->pages[0].on = false;
							app.seqActive->patternCurr->numPagesOn--;
						}
					}
					else
					{
						app.seqActive->patternCurr->pages[0].on = true;
						app.seqActive->patternCurr->numPagesOn++;
					}
				}
				break;

				default:{}
			}
		}
		else if(iBtn == BTN_PAGE2) // ------------------------------------------------------------
		{
			switch(app.modeCurr)
			{
				case MODE_DEFAULT:
				if(event == BTN_PUSHED)
				{
					if(app.seqActive->patternCurr->pages[1].on)
					{
						if(app.seqActive->patternCurr->numPagesOn > 1)
						{
							app.seqActive->patternCurr->pages[1].on = false;
							app.seqActive->patternCurr->numPagesOn--;
						}
					}
					else
					{
						app.seqActive->patternCurr->pages[1].on = true;
						app.seqActive->patternCurr->numPagesOn++;
					}
				}
				break;

				default:{}
			}
		}
		else if(iBtn == BTN_PAGE3) // ------------------------------------------------------------
		{
			switch(app.modeCurr)
			{
				case MODE_DEFAULT:
				if(event == BTN_PUSHED)
				{
					if(app.seqActive->patternCurr->pages[2].on)
					{
						if(app.seqActive->patternCurr->numPagesOn > 1)
						{
							app.seqActive->patternCurr->pages[2].on = false;
							app.seqActive->patternCurr->numPagesOn--;
						}
					}
					else
					{
						app.seqActive->patternCurr->pages[2].on = true;
						app.seqActive->patternCurr->numPagesOn++;
					}
				}
				break;

				default:{}
			}
		}
		else if(iBtn == BTN_PAGE4) // ------------------------------------------------------------
		{
			switch(app.modeCurr)
			{
				case MODE_DEFAULT:
				if(event == BTN_PUSHED)
				{
					if(app.seqActive->patternCurr->pages[3].on)
					{
						if(app.seqActive->patternCurr->numPagesOn > 1)
						{
							app.seqActive->patternCurr->pages[3].on = false;
							app.seqActive->patternCurr->numPagesOn--;
						}
					}
					else
					{
						app.seqActive->patternCurr->pages[3].on = true;
						app.seqActive->patternCurr->numPagesOn++;
					}
				}
				break;

				default:{}
			}
		}
		else if(iBtn == BTN_SHIFT) // ------------------------------------------------------------
		{
			switch(app.modeCurr)
			{
				case MODE_DEFAULT:
				if(event == BTN_PUSHED)
				{
				}
				break;
				
				case MODE_PITCH:
				if(event == BTN_RELEASED)
				{
					app.seqActive->pageSel->steps[app.iStepSel].pitch[0] = app.seqActive->rootNote;
				}
				break;

				default:{}
			}
		}
	}	
}