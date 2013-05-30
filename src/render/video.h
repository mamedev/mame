//============================================================
//
//  video.h - Generic video system header
//
//============================================================
//
//  Copyright Nicola Salmoria and the MAME Team.
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

#ifndef __RENDER_VIDEO__
#define __RENDER_VIDEO__

#include "window.h"
#include "monitor.h"
#include "drawhal.h"

//============================================================
//  CONSTANTS
//============================================================

#define MAX_WINDOWS         4


//============================================================
//  TYPE DEFINITIONS
//============================================================

namespace render
{

class window_system;
class window_info;
//struct window_config;

class video_system
{
public:
	video_system(running_machine &machine);
	virtual ~video_system();

	virtual void			update() = 0;

	virtual bool			window_has_focus() = 0;

	virtual bool			has_menu() = 0;

	virtual monitor_info *	pick_monitor(int index);

	virtual void			init_monitors() { }

	virtual void			exit();

	virtual void			set_primary_monitor(monitor_info *primary_monitor) { m_primary_monitor = primary_monitor; }
	virtual void			set_last_monitor(monitor_info *last_monitor);
	struct video_config
	{
		// global configuration
		bool				windowed;                   // start windowed?
		int                 prescale;                   // prescale factor
		int                 keepaspect;                 // keep aspect ratio
		int                 numscreens;                 // number of screens
		render_layer_config layerconfig;                // default configuration of layers

		// per-window configuration
		window_config 		window[MAX_WINDOWS];		// configuration data per-window

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

	window_system *			window() { return m_window; }

	video_config &			get_video_config() { return m_video_config; }

	virtual window_info *	window_list() { return m_window->window_list(); }

	virtual void			update_cursor_state() { }

protected:
	virtual void 			extract_video_config();

	float					get_aspect(const char *defdata, const char *data, int report_error);
	void					get_resolution(const char *defdata, const char *data, window_config *config, int report_error);

protected:
	running_machine &		m_machine;

	video_config 			m_video_config;

	window_system *			m_window;

	UINT64					m_main_threadid;

	monitor_info *			m_monitor_list;
	monitor_info *			m_primary_monitor;
	window_info **			m_last_window_ptr;
};

};

#endif // __RENDER_VIDEO__