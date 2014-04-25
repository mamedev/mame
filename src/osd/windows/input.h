// license:BSD-3-Clause
// copyright-holders:Aaron Giles
//============================================================
//
//  input.h - Win32 implementation of MAME input routines
//
//============================================================

#ifndef __INPUT_H
#define __INPUT_H


//============================================================
//  PROTOTYPES
//============================================================

void wininput_poll(running_machine &machine);

BOOL wininput_handle_mouse_button(int button, int down, int x, int y);
BOOL wininput_handle_raw(HANDLE device);

bool wininput_should_hide_mouse(void);

int wininput_vkey_for_mame_code(input_code code);


#endif /* __INPUT_H */
