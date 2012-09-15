//============================================================
//
//  video.h - Win32 implementation of MAME video routines
//
//============================================================
//
//  Copyright Aaron Giles
//  All rights reserved.
//
//  Redistribution and use in source and binary forms, with or
//  without modification, are permitted provided that the
//  following conditions are met:
//
//    * Redistributions of source code must retain the above
//      copyright notice, this list of conditions and the
//      following disclaimer.
//    * Redistributions in binary form must reproduce the
//      above copyright notice, this list of conditions and
//      the following disclaimer in the documentation and/or
//      other materials provided with the distribution.
//    * Neither the name 'MAME' nor the names of its
//      contributors may be used to endorse or promote
//      products derived from this software without specific
//      prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND
//  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
//  FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
//  EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
//  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
//  DAMAGE (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
//  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
//  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
//  ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
//  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
//  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
//  IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//============================================================

#ifndef __WIN_VIDEO__
#define __WIN_VIDEO__

#include "render.h"


//============================================================
//  CONSTANTS
//============================================================

#define MAX_WINDOWS			4

#define VIDEO_MODE_NONE		0
#define VIDEO_MODE_GDI		1
#define VIDEO_MODE_DDRAW	2
#define VIDEO_MODE_D3D		3



//============================================================
//  TYPE DEFINITIONS
//============================================================

struct win_monitor_info
{
	win_monitor_info  *	next;					// pointer to next monitor in list
	HMONITOR			handle;					// handle to the monitor
	MONITORINFOEX		info;					// most recently retrieved info
	float				aspect;					// computed/configured aspect ratio of the physical device
	int					reqwidth;				// requested width for this monitor
	int					reqheight;				// requested height for this monitor
};


struct win_window_config
{
	float				aspect;						// decoded aspect ratio
	int					width;						// decoded width
	int					height;						// decoded height
	int					refresh;					// decoded refresh
};


struct win_video_config
{
	// global configuration
	int					windowed;					// start windowed?
	int					prescale;					// prescale factor
	int					keepaspect;					// keep aspect ratio
	int					numscreens;					// number of screens
	render_layer_config	layerconfig;				// default configuration of layers

	// per-window configuration
	win_window_config	window[MAX_WINDOWS];		// configuration data per-window

	// hardware options
	int					mode;						// output mode
	int					waitvsync;					// spin until vsync
	int					syncrefresh;				// sync only to refresh rate
	int					triplebuf;					// triple buffer
	int					switchres;					// switch resolutions

	// ddraw options
	int					hwstretch;					// stretch using the hardware

	// d3d options
	int					filter;						// enable filtering
};



//============================================================
//  GLOBAL VARIABLES
//============================================================

extern win_monitor_info *win_monitor_list;
extern win_video_config video_config;


//============================================================
//  PROTOTYPES
//============================================================

void winvideo_init(running_machine &machine);

void winvideo_monitor_refresh(win_monitor_info *monitor);
float winvideo_monitor_get_aspect(win_monitor_info *monitor);
win_monitor_info *winvideo_monitor_from_handle(HMONITOR monitor);

#endif
