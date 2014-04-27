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


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// forward references
class input_type_entry;
class device_t;
class osd_interface;
class osd_sound_interface;
class osd_debugger_interface;

typedef void *osd_font;

// a osd_sound_type is simply a pointer to its alloc function
typedef osd_sound_interface *(*osd_sound_type)(const osd_interface &osd);

// a osd_sound_type is simply a pointer to its alloc function
typedef osd_debugger_interface *(*osd_debugger_type)(const osd_interface &osd);


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
	void init_debugger();
	void wait_for_debugger(device_t &device, bool firststop);
	void debugger_update();
	void debugger_exit();
	virtual void debugger_register();

	// audio overridables
	void update_audio_stream(const INT16 *buffer, int samples_this_frame);
	void set_mastervolume(int attenuation);

	// input overridables
	virtual void customize_input_type_list(simple_list<input_type_entry> &typelist);

	// font overridables
	virtual osd_font font_open(const char *name, int &height);
	virtual void font_close(osd_font font);
	virtual bool font_get_bitmap(osd_font font, unicode_char chnum, bitmap_argb32 &bitmap, INT32 &width, INT32 &xoffs, INT32 &yoffs);

	// video overridables
	virtual void *get_slider_list();

	void init_subsystems();	
	virtual bool video_init();
	
	bool sound_init();
	virtual void sound_register();
	
	virtual bool input_init();
	virtual void input_pause();
	virtual void input_resume();
	virtual bool output_init();
	virtual bool network_init();
	virtual bool midi_init();

	void exit_subsystems();	
	virtual void video_exit();
	void sound_exit();
	virtual void input_exit();
	virtual void output_exit();
	virtual void network_exit();
	virtual void midi_exit();

	virtual void osd_exit();

private:
	// internal state
	running_machine *   m_machine;	

protected:	
	osd_sound_interface* m_sound;
	osd_debugger_interface* m_debugger;
	
	tagmap_t<osd_sound_type>  m_sound_options;  
	tagmap_t<osd_debugger_type>  m_debugger_options;  
};

class osd_sound_interface
{
public:
	// construction/destruction
	osd_sound_interface(const osd_interface &osd);
	virtual ~osd_sound_interface();
	
	virtual void update_audio_stream(const INT16 *buffer, int samples_this_frame) = 0;
	virtual void set_mastervolume(int attenuation) = 0;
protected:	
	const osd_interface& m_osd;
};

// this template function creates a stub which constructs a sound subsystem
template<class _DeviceClass>
osd_sound_interface *osd_sound_creator(const osd_interface &osd)
{
	return global_alloc(_DeviceClass(osd));
}

class osd_debugger_interface
{
public:
	// construction/destruction
	osd_debugger_interface(const osd_interface &osd);
	virtual ~osd_debugger_interface();
	
	virtual void init_debugger() = 0;
	virtual void wait_for_debugger(device_t &device, bool firststop) = 0;
	virtual void debugger_update() = 0;
	virtual void debugger_exit() = 0;

protected:	
	const osd_interface& m_osd;
};

// this template function creates a stub which constructs a debugger
template<class _DeviceClass>
osd_debugger_interface *osd_debugger_creator(const osd_interface &osd)
{
	return global_alloc(_DeviceClass(osd));
}

#endif  /* __OSDEPEND_H__ */
