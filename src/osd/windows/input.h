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

const TCHAR *default_button_name(int which);

INT32 generic_button_get_state(void *device_internal, void *item_internal);
INT32 generic_axis_get_state(void *device_internal, void *item_internal);

#endif /* __INPUT_H */


