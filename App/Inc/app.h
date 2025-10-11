#ifndef APP_H
#define APP_H

#include "misc.h"

#define NUM_SEQUENCERS 4
#define BUTTON_DEBOUNCE_DELAY_MS 20
#define TEMPO_CHANGE_COOLDOWN_MS 50
#define N_POLYPHONY 4
#define NUM_PAGES 4
#define NUM_PATTERNS 8
#define NUM_STEPS 16
#define BLINK_ON_INTERVAL_MS 100
#define BLINK_OFF_INTERVAL_MS 500

#define	BTN_SEQ1 16
#define	BTN_SEQ2 17
#define	BTN_SEQ3 18
#define	BTN_DRUM 19

#define BTN_PAGE1 24
#define BTN_PAGE2 25
#define BTN_PAGE3 20
#define BTN_PAGE4 21

#define BTN_SHIFT 26
#define BTN_PITCH 30
#define	BTN_START_STOP 31

#define NOTE_C3 60


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
	MODE_COPY,
	MODE_ERASE
} Mode_e;


typedef struct
{
    uint16 tempo;
    uint16 gate;
	uint16 swing;
    uint16 dummy1;
    uint16 dummy2;
	uint16 dummy3;
} __attribute__((packed)) AnalogBuffer_t;


typedef struct
{
	//uint8 index;
	bool on;							// is the step activated
	uint8 n_poly;						// number of polyphony [TODO: check if you need this]
	int16 pitch[N_POLYPHONY];			// pitch
	int8 octOffs;						// octave offset
} Step_t;


typedef struct
{
	uint8 id;							// ID (basically the index)
	bool on;							// is the page activated
	Step_t steps[NUM_STEPS];			// the steps found on the page
} Page_t;


typedef struct
{
	uint8 id;							// identifier
	bool empty;							// is the page empty [note: migh not be used]
	uint8 numPagesOn;					// number of pages activated
	Page_t pages[NUM_PAGES];			// pages inside pattern
} Pattern_t;


typedef struct
{
	uint8 id;
	bool on;
	bool onFlag;						// sequencer about to be started
	bool offFlag;						// sequencer about to be stopped
	Resolution_e stepRes;				// step resolution
	uint8 midiChannel;					// selected midi channel
	uint8 rootNote;						// selected root note of the sequencer [note: can be used for arpeggiator]

	Pattern_t patterns[NUM_PATTERNS];
	Pattern_t* patternCurr;				// pointer to selected pattern
	Page_t* pageCurr;					// pointer to page where the currently playing step is located
	uint8 iStepCurr;					// current step index inside currently played page
	
    Page_t* pageSel;					// pointer to the selected page
	bool noteOn;						// flag to indicate that some notes are on [LEGACY]
	int16 notesOn[N_POLYPHONY];			// which notes are currently on

	uint8 gatePercent;					// currently stored gate value
	bool gateInSync;					// gate knob is at the same position as stored value
	uint16 syncEventsPerStep;			// how many midi ticks it takes to reach next step
	uint16 stepTime_ms;					// time between two changes
	uint16 gateTime_ms;					// time between step change and gate events
	volatile uint16 stepTimeCnt_ms;		// how many miliseconds has passed since last change of steps
}__attribute__((packed)) SeqData_t;


typedef struct 
{
    uint16 tempoBpm;
    Mode_e modeCurr;
    bool globPlaying;
    
    SeqData_t* seqActive;
    
    bool globStopFlag;
    bool globStartFlag;					// all sequencers are about to be started
    bool bpmIncreased;
    bool modeChanged;
    
    AnalogBuffer_t potBuffer;
    uint8 gatePercent;
    
    bool noteOn;
    
    uint16 blinkCnt_ms;
    uint16 blinkSyncCnt;
    
    uint32 syncTimestamps_100us[25];	// stores the timestamps where a midi sync event should happen up to the 1/4 of the beat
    
    uint8 iStepSel;
} AppState_t;


void InitApp();
void IncrementBPM();
void DecrementBPM();
void Step(SeqData_t* seq);
void SendNoteONs(SeqData_t* seq);
void SendNoteOFFs(SeqData_t* seq);
void ResetSequencer(SeqData_t* seq);


extern volatile AppState_t app;
extern volatile SeqData_t seq[4];

#endif /*APP_H*/