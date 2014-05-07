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
#include "options.h"

//============================================================
//  Defines
//============================================================

#define OSDOPTION_LOG                   "log"
#define OSDOPTION_VERBOSE               "verbose"
#define OSDOPTION_DEBUG                 "debug"
#define OSDOPTION_DEBUGGER			    "debugger"
#define OSDOPTION_OSLOG                 "oslog"
#define OSDOPTION_WATCHDOG              "watchdog"

#define OSDOPTION_MULTITHREADING        "multithreading"
#define OSDOPTION_NUMPROCESSORS         "numprocessors"
#define OSDOPTION_BENCH                 "bench"

#define OSDOPTION_VIDEO                 "video"
#define OSDOPTION_NUMSCREENS            "numscreens"
#define OSDOPTION_WINDOW                "window"
#define OSDOPTION_MAXIMIZE              "maximize"
#define OSDOPTION_KEEPASPECT            "keepaspect"
#define OSDOPTION_UNEVENSTRETCH         "unevenstretch"
#define OSDOPTION_WAITVSYNC             "waitvsync"
#define OSDOPTION_SYNCREFRESH           "syncrefresh"

#define OSDOPTION_SCREEN                "screen"
#define OSDOPTION_ASPECT                "aspect"
#define OSDOPTION_RESOLUTION            "resolution"
#define OSDOPTION_VIEW                  "view"

#define OSDOPTION_SWITCHRES             "switchres"

#define OSDOPTION_SOUND			        "sound"
#define OSDOPTION_AUDIO_LATENCY         "audio_latency"

#define OSDOPTVAL_AUTO                  "auto"

//============================================================
//  TYPE DEFINITIONS
//============================================================

class osd_options : public core_options
{
public:
	// construction/destruction
	osd_options();

	// debugging options
	bool verbose() const { return bool_value(OSDOPTION_VERBOSE); }
	bool log() const { return bool_value(OSDOPTION_LOG); }
	bool debug() const { return bool_value(OSDOPTION_DEBUG); }	
	const char *debugger() const { return value(OSDOPTION_DEBUGGER); }	
	bool oslog() const { return bool_value(OSDOPTION_OSLOG); }
	int watchdog() const { return int_value(OSDOPTION_WATCHDOG); }

	// performance options
	bool multithreading() const { return bool_value(OSDOPTION_MULTITHREADING); }
	const char *numprocessors() const { return value(OSDOPTION_NUMPROCESSORS); }
	int bench() const { return int_value(OSDOPTION_BENCH); }

	// video options
	const char *video() const { return value(OSDOPTION_VIDEO); }
	int numscreens() const { return int_value(OSDOPTION_NUMSCREENS); }
	bool window() const { return bool_value(OSDOPTION_WINDOW); }
	bool maximize() const { return bool_value(OSDOPTION_MAXIMIZE); }
	bool keep_aspect() const { return bool_value(OSDOPTION_KEEPASPECT); }
	bool uneven_stretch() const { return bool_value(OSDOPTION_UNEVENSTRETCH); }
	bool wait_vsync() const { return bool_value(OSDOPTION_WAITVSYNC); }
	bool sync_refresh() const { return bool_value(OSDOPTION_SYNCREFRESH); }

	// per-window options
	const char *screen() const { return value(OSDOPTION_SCREEN); }
	const char *aspect() const { return value(OSDOPTION_ASPECT); }
	const char *resolution() const { return value(OSDOPTION_RESOLUTION); }
	const char *view() const { return value(OSDOPTION_VIEW); }
	const char *screen(int index) const { astring temp; return value(temp.format("%s%d", OSDOPTION_SCREEN, index)); }
	const char *aspect(int index) const { astring temp; return value(temp.format("%s%d", OSDOPTION_ASPECT, index)); }
	const char *resolution(int index) const { astring temp; return value(temp.format("%s%d", OSDOPTION_RESOLUTION, index)); }
	const char *view(int index) const { astring temp; return value(temp.format("%s%d", OSDOPTION_VIEW, index)); }

	// full screen options
	bool switch_res() const { return bool_value(OSDOPTION_SWITCHRES); }

	// sound options
	const char *sound() const { return value(OSDOPTION_SOUND); }
	int audio_latency() const { return int_value(OSDOPTION_AUDIO_LATENCY); }

	void add_osd_options();
private:
	static const options_entry s_option_entries[];
};


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

	void register_options(osd_options &options);
	
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
	virtual void video_register();
	
	bool sound_init();
	virtual void sound_register();
	bool no_sound();
	
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
	
	void video_options_add(const char *name, void *type);
	void sound_options_add(const char *name, osd_sound_type type);
	void debugger_options_add(const char *name, osd_debugger_type type);

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
