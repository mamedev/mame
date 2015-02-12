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

#define VIDEO_MODE_NONE     0
#define VIDEO_MODE_GDI      1
#define VIDEO_MODE_DDRAW    2
#define VIDEO_MODE_D3D      3
#define VIDEO_MODE_BGFX     4



//============================================================
//  TYPE DEFINITIONS
//============================================================

class win_monitor_info
{
public:
	win_monitor_info();
	virtual ~win_monitor_info();

	void refresh();
	float aspect();
	void set_aspect(float a) { m_aspect = a; }
	const char *devicename() { refresh(); return (m_name != NULL) ? m_name : "UNKNOWN"; }

	win_monitor_info  * next;                   // pointer to next monitor in list
	HMONITOR            handle;                 // handle to the monitor
	MONITORINFOEX       info;                   // most recently retrieved info
private:
	float               m_aspect;               // computed/configured aspect ratio of the physical device
	int                 reqwidth;               // requested width for this monitor
	int                 reqheight;              // requested height for this monitor
	char * 				m_name;
};


struct win_window_config
{
	float               aspect;                     // decoded aspect ratio FIXME:Not used!
	int                 width;                      // decoded width
	int                 height;                     // decoded height
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
	win_window_config   window[MAX_WINDOWS];        // configuration data per-window

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
