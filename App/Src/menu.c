#include "menu.h"
#include "main.h"


volatile Menu_t menu;


void Menu_Reset()
{
	menu.pageSel = NULL;
	menu.seqSel = NULL;
	menu.stepSel = NULL;
	menu.iStepSel = -1;
	menu.copySelected = false;
	menu.listenOnNote = false;
	menu.numSelected = 0;
	menu.state = MENU_IDLE;
}
