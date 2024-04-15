// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont
#ifndef MAME_OSD_SDL_SDLOPTS_H
#define MAME_OSD_SDL_SDLOPTS_H

#pragma once

#include "modules/lib/osdobj_common.h"


//============================================================
//  Option identifiers
//============================================================

#define SDLOPTION_INIPATH               "inipath"
#define SDLOPTION_SDLVIDEOFPS           "sdlvideofps"
#define SDLOPTION_USEALLHEADS           "useallheads"
#define SDLOPTION_ATTACH_WINDOW         "attach_window"
#define SDLOPTION_CENTERH               "centerh"
#define SDLOPTION_CENTERV               "centerv"

#define SDLOPTION_SCALEMODE             "scalemode"

#define SDLOPTION_WAITVSYNC             "waitvsync"
#define SDLOPTION_SYNCREFRESH           "syncrefresh"
#define SDLOPTION_KEYMAP                "keymap"
#define SDLOPTION_KEYMAP_FILE           "keymap_file"

#define SDLOPTION_ENABLE_TOUCH          "enable_touch"
#define SDLOPTION_SIXAXIS               "sixaxis"
#if defined(USE_XINPUT) && USE_XINPUT
#define SDLOPTION_LIGHTGUNINDEX         "lightgun_index"
#endif

#define SDLOPTION_AUDIODRIVER           "audiodriver"
#define SDLOPTION_VIDEODRIVER           "videodriver"
#define SDLOPTION_RENDERDRIVER          "renderdriver"
#define SDLOPTION_GL_LIB                "gl_lib"


//============================================================
//  Option values
//============================================================

#define SDLOPTVAL_OPENGL                "opengl"
#define SDLOPTVAL_SOFT                  "soft"
#define SDLOPTVAL_SDL2ACCEL             "accel"
#define SDLOPTVAL_BGFX                  "bgfx"

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
#if defined(SDLMAME_X11)
	bool use_all_heads() const { return bool_value(SDLOPTION_USEALLHEADS); }
	const char *attach_window() const { return value(SDLOPTION_ATTACH_WINDOW); }
#endif // SDLMAME_X11

	// keyboard mapping
	bool keymap() const { return bool_value(SDLOPTION_KEYMAP); }
	const char *keymap_file() const { return value(SDLOPTION_KEYMAP_FILE); }

	// input options
	bool enable_touch() const { return bool_value(SDLOPTION_ENABLE_TOUCH); }
	bool sixaxis() const { return bool_value(SDLOPTION_SIXAXIS); }

	const char *video_driver() const { return value(SDLOPTION_VIDEODRIVER); }
	const char *render_driver() const { return value(SDLOPTION_RENDERDRIVER); }
	const char *audio_driver() const { return value(SDLOPTION_AUDIODRIVER); }
#if USE_OPENGL
	const char *gl_lib() const { return value(SDLOPTION_GL_LIB); }
#endif
};

#endif // MAME_OSD_SDL_SDLOPTS_H
