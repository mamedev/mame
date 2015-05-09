// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont
//============================================================
//
//  input.h - SDL implementation of MAME input routines
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================

#ifndef __SDLINPUT_H__
#define __SDLINPUT_H__

#include "window.h"

//============================================================
//  PROTOTYPES
//============================================================

void sdlinput_poll(running_machine &machine);
int  sdlinput_should_hide_mouse();

sdl_window_info *sdlinput_get_focus_window();

void  sdlinput_process_events_buf();
void  sdlinput_release_keys();

#endif /* __SDLINPUT_H__ */
