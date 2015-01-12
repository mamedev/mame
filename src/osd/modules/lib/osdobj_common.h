// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    osdepend.h

    OS-dependent code interface.

*******************************************************************c********/

#pragma once

#ifndef __OSDOBJ_COMMON_H__
#define __OSDOBJ_COMMON__

#include "osdepend.h"
#include "options.h"


#if 0
// forward references
class input_type_entry;
class device_t;
#endif

class osd_sound_interface;
class osd_debugger_interface;

// a osd_sound_type is simply a pointer to its alloc function
typedef osd_sound_interface *(*osd_sound_type)(const osd_interface &osd, running_machine &machine);

// a osd_sound_type is simply a pointer to its alloc function
typedef osd_debugger_interface *(*osd_debugger_type)(const osd_interface &osd);


// ======================> osd_interface

// description of the currently-running machine
class osd_common_t : public osd_interface
{
public:
	// construction/destruction
	osd_common_t();
	virtual ~osd_common_t();

	virtual void register_options(osd_options &options);

	// general overridables
	virtual void init(running_machine &machine);
	virtual void update(bool skip_redraw);

	// debugger overridables
	virtual void init_debugger();
	virtual void wait_for_debugger(device_t &device, bool firststop);

	// audio overridables
	virtual void update_audio_stream(const INT16 *buffer, int samples_this_frame);
	virtual void set_mastervolume(int attenuation);
	virtual bool no_sound();

	// input overridables
	virtual void customize_input_type_list(simple_list<input_type_entry> &typelist);

	// font overridables
	virtual osd_font font_open(const char *name, int &height);
	virtual void font_close(osd_font font);
	virtual bool font_get_bitmap(osd_font font, unicode_char chnum, bitmap_argb32 &bitmap, INT32 &width, INT32 &xoffs, INT32 &yoffs);

	// video overridables
	virtual void *get_slider_list();

	// midi overridables
	// FIXME: this should return a list of devices, not list them on stdout
	virtual void list_midi_devices(void);

	virtual void list_network_adapters()
    {
        network_init();
        osd_list_network_adapters();
        network_exit();
    }


	// FIXME: everything below seems to be osd specific and not part of
	//        this INTERFACE but part of the osd IMPLEMENTATION

    // getters
    running_machine &machine() { assert(m_machine != NULL); return *m_machine; }


    virtual void debugger_update();
    virtual void debugger_exit();
    virtual void debugger_register();

    virtual void init_subsystems();

    virtual bool video_init();
    virtual void video_register();
    virtual bool window_init();

    virtual bool sound_init();
    virtual void sound_register();

    virtual bool input_init();
    virtual void input_pause();
    virtual void input_resume();
    virtual bool output_init();
    virtual bool network_init();
    virtual bool midi_init();

    virtual void exit_subsystems();
    virtual void video_exit();
    virtual void window_exit();
    virtual void sound_exit();
    virtual void input_exit();
    virtual void output_exit();
    virtual void network_exit();
    virtual void midi_exit();

    virtual void osd_exit();

    virtual void video_options_add(const char *name, void *type);
    virtual void sound_options_add(const char *name, osd_sound_type type);
    virtual void debugger_options_add(const char *name, osd_debugger_type type);

private:
	// internal state
	running_machine *   m_machine;

	void update_option(osd_options &options, const char * key, dynamic_array<const char *> &values);

protected:
	osd_sound_interface* m_sound;
	osd_debugger_interface* m_debugger;
private:
	//tagmap_t<osd_video_type>  m_video_options;
	dynamic_array<const char *> m_video_names;
	tagmap_t<osd_sound_type>  m_sound_options;
	dynamic_array<const char *> m_sound_names;
	tagmap_t<osd_debugger_type>  m_debugger_options;
	dynamic_array<const char *> m_debugger_names;
};


class osd_sound_interface
{
public:
	// construction/destruction
	osd_sound_interface(const osd_interface &osd, running_machine &machine);
	virtual ~osd_sound_interface();

	virtual void update_audio_stream(const INT16 *buffer, int samples_this_frame) = 0;
	virtual void set_mastervolume(int attenuation) = 0;
protected:
	const osd_interface& m_osd;
	running_machine& m_machine;
};

// this template function creates a stub which constructs a sound subsystem
template<class _DeviceClass>
osd_sound_interface *osd_sound_creator(const osd_interface &osd, running_machine &machine)
{
	return global_alloc(_DeviceClass(osd, machine));
}

class osd_debugger_interface
{
public:
	// construction/destruction
	osd_debugger_interface(const osd_interface &osd);
	virtual ~osd_debugger_interface();

	virtual void init_debugger(running_machine &machine) = 0;
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

#endif  /* __OSDOBJ_COMMON_H__ */
