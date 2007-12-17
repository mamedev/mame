//============================================================
//
//  debugwin.h - Win32 debug window handling
//
//  Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
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
void debugwin_update_during_game(void);
int debugwin_is_debugger_visible(void);

#endif
