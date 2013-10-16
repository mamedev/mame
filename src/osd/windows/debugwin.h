// license:BSD-3-Clause
// copyright-holders:Aaron Giles
//============================================================
//
//  debugwin.h - Win32 debug window handling
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

void debugwin_destroy_windows(running_machine &machine);
void debugwin_update_during_game(running_machine &machine);

#endif
