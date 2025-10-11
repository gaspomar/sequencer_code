#ifndef MENU_H
#define MENU_H

#include "misc.h"
#include "app.h"

typedef enum
{
	MENU_IDLE,
	MENU_LISTEN_ON_NOTE,
	MENU_COPY_STEPS,
	MENU_PASTE_STEPS,
	MENU_ERASE_STEPS,
	MENU_COPY_PAGES,
	MENU_PASTE_PAGES
} MenuState_e;

typedef struct
{
	bool listenOnNote;
	bool copySelected;
	MenuState_e state;

	uint8 numSelected;

	Page_t* pageSel;
	SeqData_t* seqSel;
	Step_t* stepSel;

	int16 iStepSel;
} Menu_t;

extern volatile Menu_t menu;

void Menu_Reset();

#endif /*MENU_H*/