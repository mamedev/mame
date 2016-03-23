// license:BSD-3-Clause
// copyright-holders:Aaron Giles
//============================================================
//
//  video.h - Win32 implementation of MAME video routines
//
//============================================================

#ifndef __WIN_VIDEO__
#define __WIN_VIDEO__

#include "render.h"
#include "winmain.h"
#include "modules/osdwindow.h"

//============================================================
//  CONSTANTS
//============================================================

#define MAX_WINDOWS         4

#define GLSL_SHADER_MAX 10

//============================================================
//  TYPE DEFINITIONS
//============================================================

inline osd_rect RECT_to_osd_rect(const RECT &r)
{
	return osd_rect(r.left, r.top, r.right - r.left, r.bottom - r.top);
}

class win_monitor_info : public osd_monitor_info
{
public:
	win_monitor_info(const HMONITOR handle, const char *monitor_device, float aspect);
	virtual ~win_monitor_info();

	virtual void refresh() override;

	// static

	static BOOL CALLBACK monitor_enum_callback(HMONITOR handle, HDC dc, LPRECT rect, LPARAM data);
	static osd_monitor_info *monitor_from_handle(HMONITOR monitor);

	HMONITOR handle() { return m_handle; }

private:
	HMONITOR            m_handle;                 // handle to the monitor
	MONITORINFOEX       m_info;                   // most recently retrieved info
};

struct osd_video_config
{
	// global configuration
	int                 windowed;                   // start windowed?
	int                 prescale;                   // prescale factor
	int                 keepaspect;                 // keep aspect ratio
	int                 numscreens;                 // number of screens

	// hardware options
	int                 mode;                       // output mode
	int                 waitvsync;                  // spin until vsync
	int                 syncrefresh;                // sync only to refresh rate
	int                 switchres;                  // switch resolutions

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
