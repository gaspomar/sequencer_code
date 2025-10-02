#ifndef MENU_H
#define MENU_H

#include "misc.h"
#include "app.h"

typedef enum
{
	MENU_IDLE,
	MENU_COPY_STEPS,
	MENU_COPY_PAGES
} MenuAction_e;

typedef struct
{
	bool listenOnNote;
	bool copySelected;
	MenuAction_e actionCurr;

	uint8 numSelected;

	Page_t* pageSel;
	SeqData_t* seqSel;
	Step_t* stepSel;

	uint8 iStepSel;
} Menu_t;

extern volatile Menu_t menu;

void MenuReset();

#endif /*MENU_H*/