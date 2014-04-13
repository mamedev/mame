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
#include "emutempl.h"
#include "osdcore.h"
#include "unicode.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// forward references
class input_type_entry;
class device_t;
typedef void *osd_font;


// ======================> osd_interface

// description of the currently-running machine
class osd_interface
{
public:
	// construction/destruction
	osd_interface();
	virtual ~osd_interface();

	// getters
	running_machine &machine() const { assert(m_machine != NULL); return *m_machine; }

	// general overridables
	virtual void init(running_machine &machine);
	virtual void update(bool skip_redraw);

	// debugger overridables
	virtual void init_debugger();
	virtual void wait_for_debugger(device_t &device, bool firststop);

	// audio overridables
	virtual void update_audio_stream(const INT16 *buffer, int samples_this_frame);
	virtual void set_mastervolume(int attenuation);

	// input overridables
	virtual void customize_input_type_list(simple_list<input_type_entry> &typelist);

	// font overridables
	virtual osd_font font_open(const char *name, int &height);
	virtual void font_close(osd_font font);
	virtual bool font_get_bitmap(osd_font font, unicode_char chnum, bitmap_argb32 &bitmap, INT32 &width, INT32 &xoffs, INT32 &yoffs);

	// video overridables
	virtual void *get_slider_list();

private:
	// internal state
	running_machine *   m_machine;
};


#endif  /* __OSDEPEND_H__ */
