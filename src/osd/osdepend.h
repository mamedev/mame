// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    osdepend.h

    OS-dependent code interface.

*******************************************************************c********/

#pragma once

#ifndef __OSDEPEND_H__
#define __OSDEPEND_H__

#include "emucore.h"
#include "osdcore.h"
#include "unicode.h"
#include "cliopts.h"

// forward references
class input_type_entry;     // FIXME: including emu.h does not work because emu.h includes osdepend.h


//============================================================
//  TYPE DEFINITIONS
//============================================================

// FIXME: We can do better than this
typedef void *osd_font;

// ======================> osd_interface

// description of the currently-running machine
class osd_interface
{
public:

	// general overridables
	virtual void init(running_machine &machine) = 0;
	virtual void update(bool skip_redraw) = 0;

	// debugger overridables
	virtual void init_debugger() = 0;
	virtual void wait_for_debugger(device_t &device, bool firststop) = 0;

	// audio overridables
	virtual void update_audio_stream(const INT16 *buffer, int samples_this_frame) = 0;
	virtual void set_mastervolume(int attenuation) = 0;
	virtual bool no_sound() = 0;

	// input overridables
	virtual void customize_input_type_list(simple_list<input_type_entry> &typelist) = 0;

	// font overridables
	virtual osd_font font_open(const char *name, int &height) = 0;
	virtual void font_close(osd_font font) = 0;
	virtual bool font_get_bitmap(osd_font font, unicode_char chnum, bitmap_argb32 &bitmap, INT32 &width, INT32 &xoffs, INT32 &yoffs) = 0;

	// video overridables
	virtual void *get_slider_list() = 0; // FIXME: returns slider_state *

	// command option overrides
	virtual bool execute_command(const char *command) = 0;

};

#endif  /* __OSDEPEND_H__ */
