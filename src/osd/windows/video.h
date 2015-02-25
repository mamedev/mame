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


//============================================================
//  CONSTANTS
//============================================================

#define MAX_WINDOWS         4

enum {
	VIDEO_MODE_NONE,
	VIDEO_MODE_GDI,
	VIDEO_MODE_DDRAW,
	VIDEO_MODE_D3D,
	VIDEO_MODE_BGFX,
#if (USE_OPENGL)
	VIDEO_MODE_OPENGL,
#endif
};

#define GLSL_SHADER_MAX 10

//============================================================
//  TYPE DEFINITIONS
//============================================================

class win_monitor_info
{
public:
	win_monitor_info();
	virtual ~win_monitor_info();

	void refresh();

	const HMONITOR handle() { return m_handle; }
	const RECT &position_size() { refresh(); return m_info.rcMonitor; }
	const RECT &usuable_position_size() { refresh(); return m_info.rcWork; }
	bool is_primary() { return (m_info.dwFlags & MONITORINFOF_PRIMARY) != 0; }
	const char *devicename() { refresh(); return (m_name != NULL) ? m_name : "UNKNOWN"; }

	float aspect();

	void set_aspect(const float a) { m_aspect = a; }

	win_monitor_info  * m_next;                   // pointer to next monitor in list

	// static

	static BOOL CALLBACK monitor_enum_callback(HMONITOR handle, HDC dc, LPRECT rect, LPARAM data);

private:
	HMONITOR            m_handle;                 // handle to the monitor
	MONITORINFOEX       m_info;                   // most recently retrieved info

	float               m_aspect;               // computed/configured aspect ratio of the physical device
	char *              m_name;
};


struct osd_window_config
{
	osd_window_config() : aspect(0.0f), width(0), height(0), depth(0), refresh(0) {}

	float               aspect;                     // decoded aspect ratio FIXME: not used on windows
	int                 width;                      // decoded width
	int                 height;                     // decoded height
	int                 depth;                      // decoded depth - only SDL
	int                 refresh;                    // decoded refresh
};


struct win_video_config
{
	// global configuration
	int                 windowed;                   // start windowed?
	int                 prescale;                   // prescale factor
	int                 keepaspect;                 // keep aspect ratio
	int                 numscreens;                 // number of screens
	render_layer_config layerconfig;                // default configuration of layers

	// per-window configuration
	osd_window_config   window[MAX_WINDOWS];        // configuration data per-window

	// hardware options
	int                 mode;                       // output mode
	int                 waitvsync;                  // spin until vsync
	int                 syncrefresh;                // sync only to refresh rate
	int                 triplebuf;                  // triple buffer
	int                 switchres;                  // switch resolutions

	// ddraw options
	int                 hwstretch;                  // stretch using the hardware

	// d3d options
	int                 filter;                     // enable filtering

	// OpenGL options
	//int                 filter;         // enable filtering, disabled if glsl_filter>0
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
};



//============================================================
//  GLOBAL VARIABLES
//============================================================

extern win_video_config video_config;


//============================================================
//  PROTOTYPES
//============================================================

win_monitor_info *winvideo_monitor_from_handle(HMONITOR monitor);

#endif
