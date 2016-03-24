// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont
#ifndef _osdsdl_h_
#define _osdsdl_h_

#include "sdlinc.h"

#include "watchdog.h"
#include "clifront.h"
#include "modules/lib/osdobj_common.h"
#include "modules/osdmodule.h"
#include "modules/font/font_module.h"

//============================================================
//  System dependent defines
//============================================================


#if defined(SDLMAME_WIN32)
		#define SDLMAME_EVENTS_IN_WORKER_THREAD (0)
		#define SDLMAME_INIT_IN_WORKER_THREAD   (0)
		#define SDL13_COMBINE_RESIZE (0) //(1) no longer needed
#else
	#define SDLMAME_EVENTS_IN_WORKER_THREAD (0)
	#define SDLMAME_INIT_IN_WORKER_THREAD   (0)
	#define SDL13_COMBINE_RESIZE (0)
#endif

//============================================================
//  Defines
//============================================================

#define SDLOPTION_INIPATH               "inipath"
#define SDLOPTION_SDLVIDEOFPS           "sdlvideofps"
#define SDLOPTION_USEALLHEADS           "useallheads"
#define SDLOPTION_CENTERH               "centerh"
#define SDLOPTION_CENTERV               "centerv"

#define SDLOPTION_SCALEMODE             "scalemode"

#define SDLOPTION_WAITVSYNC             "waitvsync"
#define SDLOPTION_SYNCREFRESH           "syncrefresh"
#define SDLOPTION_KEYMAP                "keymap"
#define SDLOPTION_KEYMAP_FILE           "keymap_file"

#define SDLOPTION_SIXAXIS               "sixaxis"
#define SDLOPTION_JOYINDEX              "joy_idx"
#define SDLOPTION_KEYBINDEX             "keyb_idx"
#define SDLOPTION_MOUSEINDEX            "mouse_index"
#if (USE_XINPUT)
#define SDLOPTION_LIGHTGUNINDEX         "lightgun_index"
#endif

#define SDLOPTION_AUDIODRIVER           "audiodriver"
#define SDLOPTION_VIDEODRIVER           "videodriver"
#define SDLOPTION_RENDERDRIVER          "renderdriver"
#define SDLOPTION_GL_LIB                "gl_lib"

#define SDLOPTVAL_OPENGL                "opengl"
#define SDLOPTVAL_SOFT                  "soft"
#define SDLOPTVAL_SDL2ACCEL             "accel"
#define SDLOPTVAL_BGFX                  "bgfx"

#define SDLMAME_LED(x)                  "led" #x

// read by sdlmame

#define SDLENV_DESKTOPDIM               "SDLMAME_DESKTOPDIM"
#define SDLENV_VMWARE                   "SDLMAME_VMWARE"

// set by sdlmame

#define SDLENV_VISUALID                 "SDL_VIDEO_X11_VISUALID"
#define SDLENV_VIDEODRIVER              "SDL_VIDEODRIVER"
#define SDLENV_AUDIODRIVER              "SDL_AUDIODRIVER"
#define SDLENV_RENDERDRIVER             "SDL_VIDEO_RENDERER"

#define SDLMAME_SOUND_LOG               "sound.log"

#ifdef SDLMAME_MACOSX
/* Vas Crabb: Default GL-lib for MACOSX */
#define SDLOPTVAL_GLLIB                 "/System/Library/Frameworks/OpenGL.framework/Libraries/libGL.dylib"
#else
#define SDLOPTVAL_GLLIB                 OSDOPTVAL_AUTO
#endif


//============================================================
//  TYPE DEFINITIONS
//============================================================

class sdl_options : public osd_options
{
public:
	// construction/destruction
	sdl_options();

	// performance options
	bool video_fps() const { return bool_value(SDLOPTION_SDLVIDEOFPS); }

	// video options
	bool centerh() const { return bool_value(SDLOPTION_CENTERH); }
	bool centerv() const { return bool_value(SDLOPTION_CENTERV); }
	const char *scale_mode() const { return value(SDLOPTION_SCALEMODE); }

	// full screen options
#ifdef SDLMAME_X11
	bool use_all_heads() const { return bool_value(SDLOPTION_USEALLHEADS); }
#endif

	// keyboard mapping
	bool keymap() const { return bool_value(SDLOPTION_KEYMAP); }
	const char *keymap_file() const { return value(SDLOPTION_KEYMAP_FILE); }

	// joystick mapping
	const char *joy_index(int index) const { return value(string_format("%s%d", SDLOPTION_JOYINDEX, index).c_str()); }
	bool sixaxis() const { return bool_value(SDLOPTION_SIXAXIS); }

	const char *mouse_index(int index) const { return value(string_format("%s%d", SDLOPTION_MOUSEINDEX, index).c_str()); }
	const char *keyboard_index(int index) const { return value(string_format("%s%d", SDLOPTION_KEYBINDEX, index).c_str()); }

	const char *video_driver() const { return value(SDLOPTION_VIDEODRIVER); }
	const char *render_driver() const { return value(SDLOPTION_RENDERDRIVER); }
	const char *audio_driver() const { return value(SDLOPTION_AUDIODRIVER); }
#if USE_OPENGL
	const char *gl_lib() const { return value(SDLOPTION_GL_LIB); }
#endif

private:
	static const options_entry s_option_entries[];
};


class sdl_osd_interface : public osd_common_t
{
public:
	// construction/destruction
	sdl_osd_interface(sdl_options &options);
	virtual ~sdl_osd_interface();

	// general overridables
	virtual void init(running_machine &machine) override;
	virtual void update(bool skip_redraw) override;

	// video overridables
	virtual slider_state *get_slider_list() override;

	// input overridables
	virtual void customize_input_type_list(simple_list<input_type_entry> &typelist) override;

	virtual void video_register() override;

	virtual bool video_init() override;
	virtual bool window_init() override;
	virtual bool output_init() override;
	//virtual bool midi_init();

	virtual void video_exit() override;
	virtual void window_exit() override;
	virtual void output_exit() override;
	//virtual void midi_exit();

	// sdl specific
	void poll_inputs(running_machine &machine);
	void release_keys();
	bool should_hide_mouse();
	void process_events_buf();

	sdl_options &options() { return m_options; }

protected:
	virtual void build_slider_list() override;
	virtual void update_slider_list() override;

private:
	virtual void osd_exit() override;

	void extract_video_config();

	sdl_options &m_options;

	watchdog *m_watchdog;
	slider_state *      m_sliders;
};

//============================================================
//  sdlwork.c
//============================================================

extern int osd_num_processors;

#endif
