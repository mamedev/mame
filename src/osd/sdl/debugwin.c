//============================================================
//
//  debugwin.c - stubs for linking when NO_DEBUGGER is defined
//
//  Copyright (c) 1996-2010, Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================

#if defined(NO_DEBUGGER)

#include "sdlinc.h"

#include "emu.h"
#include "osdepend.h"
#include "osdsdl.h"

// win32 stubs for linking
void sdl_osd_interface::init_debugger()
{
}

void sdl_osd_interface::wait_for_debugger(device_t &device, bool firststop)
{
}

// win32 stubs for linking
void debugwin_update_during_game(running_machine &machine)
{
}

#endif
