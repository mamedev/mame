//============================================================
//
//  input.h - Win32 implementation of MAME input routines
//
//  Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
//
//============================================================

#ifndef __INPUT_H
#define __INPUT_H


//============================================================
//  PROTOTYPES
//============================================================

void wininput_init(running_machine *machine);
void wininput_poll(void);

void wininput_handle_mouse_button(int button, int down, int x, int y);
BOOL wininput_handle_raw(HANDLE device);

int wininput_should_hide_mouse(void);


#endif /* __INPUT_H */
