#include "menu.h"
#include "main.h"


volatile Menu_t menu;


void MenuReset()
{
	menu.pageSel = NULL;
	menu.seqSel = NULL;
	menu.stepSel = NULL;
	menu.copySelected = false;
	menu.listenOnNote = false;
	menu.numSelected = 0;
	menu.actionCurr = MENU_IDLE;
}