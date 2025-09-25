#ifndef TASKS_H
#define TASKS_H

#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"


/************************************************************************************************************************
 * Settings
 ***********************************************************************************************************************/


#define BUTTON_DEBOUNCE_DELAY_MS 10
#define TEMPO_CHANGE_COOLDOWN_MS 50
#define N_POLYPHONY 6
#define NUM_PAGES 4
#define NUM_PATTERNS 8
#define NUM_STEPS 16
#define BLINK_ON_INTERVAL_MS 100
#define BLINK_OFF_INTERVAL_MS 500


/************************************************************************************************************************
 * ----------
 ***********************************************************************************************************************/

typedef struct 
{
    GPIO_TypeDef* port;
    uint32 pin;
} GPIO_t;

typedef enum
{
	BTN_PUSHED,
	BTN_RELEASED
} BtnEvent_e;

typedef enum
{
	RES_1_3 = 3,
	RES_1_6 = 6,
	RES_1_4 = 4,
	RES_1_8 = 8,
	RES_1_16 = 16
} Resolution_e;

typedef enum
{
	MODE_DEFAULT,
	MODE_PITCH,
	MODE_SET_CHANNEL,
	MODE_COPY
} Mode_e;

typedef struct
{
	bool on;
	uint8 n_poly;
	int16 pitch[N_POLYPHONY];
	int8 octOffs;				// octave offset
} Step_t;

typedef struct
{
	uint8 id;
	bool on;
	Step_t steps[NUM_STEPS];
} Page_t;

typedef struct
{
	uint8 id;
	bool empty;
	Page_t pages[NUM_PAGES];
	bool varPagesOn[NUM_PAGES];	// alternative pages selected
} Pattern_t;

	
typedef struct
{
	uint8 id;
	bool on;
	bool startFlag;			// sequencer about to be started
	bool offFlag;
	Resolution_e stepRes;	// step resolution
	uint8 midiChannel;
	uint8 rootNote;	//

	Pattern_t patterns[NUM_PATTERNS];
	Pattern_t* patternCurr;
	Page_t* pageCurr;	// page where the current step is located
    Page_t* pageSel;	// selected page
	uint8 iStepCurr;	// current step index inside current page
	bool noteOn;
	int16 notesOn[N_POLYPHONY];

	uint8 gatePercent;
	bool gateInSync;
	uint16 syncEventsPerStep;
	uint16 stepTime_ms;
	uint16 gateTime_ms;
	volatile uint16 stepTimeCnt_ms;
}__attribute__((packed)) SeqData_t;

typedef struct
{
	bool copySelected;
	Page_t* pageSel;
	SeqData_t* seqSel;
	Step_t* stepSel;
} Menu_t;

// task notification bits ---------------

#define NOTIF_NEW_STEP_REACHED 0x0001
#define NOTIF_ROT_INPUT_RIGHT 0x0002
#define NOTIF_ROT_INPUT_LEFT 0x0004
#define NOTIF_GLOB_START_STOP 0x0008
#define NOTIF_LED_UPDATE_REQUIRED 0x0010

extern volatile bool globPlaying;
extern volatile bool globStopFlag;
extern volatile bool globStartFlag;
extern volatile bool bpmIncreased;

extern volatile SeqData_t seq[4];

extern volatile uint16 blinkCnt_ms;
extern volatile uint16 blinkSyncCnt;
extern volatile uint32 syncTimestamps_100us[25];
extern volatile uint16 syncEventsPerStep_seq1;

extern TaskHandle_t mainTaskHandle;

void InitApp();

#endif /*TASK_H*/