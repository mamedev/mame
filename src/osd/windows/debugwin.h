//============================================================
//
//  debugwin.h - Win32 debug window handling
//
//  Copyright Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
//
//============================================================

#ifndef __WIN_DEBUGWIN__
#define __WIN_DEBUGWIN__


//============================================================
//  GLOBAL VARIABLES
//============================================================

// windows



//============================================================
//  PROTOTYPES
//============================================================

void debugwin_init_windows(void);
void debugwin_destroy_windows(void);
void debugwin_show(int type);
void debugwin_update_during_game(running_machine *machine);

#endif
