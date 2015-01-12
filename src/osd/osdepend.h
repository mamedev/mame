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

// forward references
class input_type_entry;     // FIXME: including emu.h does not work because emu.h includes osdepend.h

//============================================================
//  Defines
//============================================================

/* FIXME: void cli_frontend::listnetworkadapters should be
 * moved here.
 */
#include "options.h"

#define OSDOPTION_LOG                   "log"
#define OSDOPTION_VERBOSE               "verbose"
#define OSDOPTION_DEBUG                 "debug"
#define OSDOPTION_DEBUGGER              "debugger"
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

#define OSDOPTION_SOUND                 "sound"
#define OSDOPTION_AUDIO_LATENCY         "audio_latency"

#define OSDOPTVAL_AUTO                  "auto"

//============================================================
//  TYPE DEFINITIONS
//============================================================

/* FIXME: core_options inherits from osd_options. This will force any
 * future osd implementation to use the options below. Actually, these
 * options are *private* to the osd_core. This object should actually be an
 * accessor object. Later ...
 */

class osd_options : public core_options
{
public:
    // construction/destruction
    osd_options() : core_options() {};

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

    void add_osd_options()
    {
        this->add_entries(s_option_entries);
    }
private:
    static const options_entry s_option_entries[];
};


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

	// midi overridables
	// FIXME: this should return a list of devices, not list them on stdout, even better
	// move this to OSD_OPTIONS
	virtual void list_midi_devices(void) = 0;

    virtual void list_network_adapters() = 0;

};

#endif  /* __OSDEPEND_H__ */
