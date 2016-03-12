// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont
//============================================================
//
//  video.h - SDL implementation of MAME video routines
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================

#ifndef __SDLVIDEO__
#define __SDLVIDEO__

#include "osdsdl.h"
#include "modules/osdwindow.h"

//============================================================
//  CONSTANTS
//============================================================

#define MAX_VIDEO_WINDOWS           (4)

#define VIDEO_SCALE_MODE_NONE       (0)

#define GLSL_SHADER_MAX 10

//============================================================
//  TYPE DEFINITIONS
//============================================================

inline osd_rect SDL_Rect_to_osd_rect(const SDL_Rect &r)
{
	return osd_rect(r.x, r.y, r.w, r.h);
}

class sdl_monitor_info : public osd_monitor_info
{
public:
#if 0
	sdl_monitor_info()
	: m_next(NULL), m_handle(0), m_aspect(0.0f)
		{}
#endif
	sdl_monitor_info(const UINT64 handle, const char *monitor_device, float aspect)
	: osd_monitor_info(&m_handle, monitor_device, aspect), m_handle(handle)
	{
		refresh();
	}

	// STATIC
	static void init();
	static void exit();
private:
	void virtual refresh() override;

	UINT64              m_handle;                 // handle to the monitor
};

struct osd_video_config
{
	// global configuration
	int                 windowed;               // start windowed?
	int                 prescale;                   // prescale factor
	int                 keepaspect;                 // keep aspect ratio
	int                 numscreens;             // number of screens

	// hardware options
	int                 mode;           // output mode
	int                 waitvsync;      // spin until vsync
	int                 syncrefresh;    // sync only to refresh rate
	int                 switchres;      // switch resolutions

	int                 fullstretch;    // FXIME: implement in windows!

	// d3d, accel, opengl
	int                 filter;                     // enable filtering
	//int                 filter;         // enable filtering, disabled if glsl_filter>0

	// OpenGL options
	int                 glsl;
	int                 glsl_filter;        // glsl filtering, >0 disables filter
	char *              glsl_shader_mamebm[GLSL_SHADER_MAX]; // custom glsl shader set, mame bitmap
	int                 glsl_shader_mamebm_num; // custom glsl shader set number, mame bitmap
	char *              glsl_shader_scrn[GLSL_SHADER_MAX]; // custom glsl shader set, screen bitmap
	int                 glsl_shader_scrn_num; // custom glsl shader number, screen bitmap
	int                 pbo;
	int                 vbo;
	int                 allowtexturerect;   // allow GL_ARB_texture_rectangle, default: no
	int                 forcepow2texture;   // force power of two textures, default: no

	// dd, d3d
	int                 triplebuf;                  // triple buffer

	//============================================================
	// SDL - options
	//============================================================
	int                 novideo;                // don't draw, for pure CPU benchmarking

	int                 centerh;
	int                 centerv;

	// vector options
	float               beamwidth;      // beam width

	// perftest
	int                 perftest;       // print out real video fps

	// X11 options
	int                 restrictonemonitor; // in fullscreen, confine to Xinerama monitor 0

	// YUV options
	int                 scale_mode;
};

//============================================================
//  GLOBAL VARIABLES
//============================================================

extern osd_video_config video_config;

#endif
