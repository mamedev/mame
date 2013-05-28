//============================================================
//
//  video.c - Windows video system
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

#include "render/windows/video.h"
#include "osd/windows/debugwin.h"

namespace render
{

namespace windows
{

video_system::video_system(running_machine &machine) :
	render::video_system(machine)
{
	if (m_video_config.mode != VIDEO_MODE_NONE)
		SetForegroundWindow(m_window_list->hwnd());

	// possibly create the debug window, but don't show it yet
	if (m_machine.debug_flags & DEBUG_FLAG_OSD_ENABLED)
		debugwin_init_windows(m_machine, this);
}

void video_system::update()
{
	for (window_info *window = m_window_list; window != NULL; window = window->next())
	{
		window->update();
	}
}

void video_system::exit()
{
	// free all of our monitor information
	while (m_monitor_list != NULL)
	{
		monitor_info *temp = m_monitor_list;
		m_monitor_list = temp->next();
		global_free(temp);
	}
}

//============================================================
//  extract_video_config
//============================================================

void video_system::extract_video_config()
{
	render::video_system::extract_video_config();
	windows_options &options = downcast<windows_options &>(machine.options());
	const char *stemp;

	m_video_config.prescale      = options.prescale();

	// per-window options: extract the data
	const char *default_resolution = options.resolution();
	get_resolution(default_resolution, options.resolution(0), &m_video_config.window[0], TRUE);
	get_resolution(default_resolution, options.resolution(1), &m_video_config.window[1], TRUE);
	get_resolution(default_resolution, options.resolution(2), &m_video_config.window[2], TRUE);
	get_resolution(default_resolution, options.resolution(3), &m_video_config.window[3], TRUE);

	// video options: extract the data
	stemp = options.video();
	if (strcmp(stemp, "d3d") == 0)
		m_video_config.mode = VIDEO_MODE_D3D;
	else if (strcmp(stemp, "ddraw") == 0)
		m_video_config.mode = VIDEO_MODE_DDRAW;
	else if (strcmp(stemp, "gdi") == 0)
		m_video_config.mode = VIDEO_MODE_GDI;
	else if (strcmp(stemp, "none") == 0)
	{
		m_video_config.mode = VIDEO_MODE_NONE;
		if (options.seconds_to_run() == 0)
			mame_printf_warning("Warning: -video none doesn't make much sense without -seconds_to_run\n");
	}
	else
	{
		mame_printf_warning("Invalid video value %s; reverting to gdi\n", stemp);
		m_video_config.mode = VIDEO_MODE_GDI;
	}
	m_video_config.waitvsync     = options.wait_vsync();
	m_video_config.syncrefresh   = options.sync_refresh();
	m_video_config.triplebuf     = options.triple_buffer();
	m_video_config.switchres     = options.switch_res();

	// ddraw options: extract the data
	m_video_config.hwstretch     = options.hwstretch();

	// d3d options: extract the data
	m_video_config.filter        = options.filter();
	if (m_video_config.prescale == 0)
		m_video_config.prescale = 1;
}


//============================================================
//  monitor_from_handle
//============================================================

monitor_info *video_system::monitor_from_handle(HMONITOR hmonitor)
{
	// find the matching monitor
	for (monitor_info *monitor = win_monitor_list; monitor != NULL; monitor = monitor->next())
	{
		if (monitor->handle() == hmonitor)
		{
			return monitor;
		}
	}
	return NULL;
}


//============================================================
//  init_monitors
//============================================================

void video_system::init_monitors()
{
	// make a list of monitors
	m_monitor_list = NULL;
	win_monitor_info **tailptr = &m_monitor_list;
	EnumDisplayMonitors(NULL, NULL, monitor_enum_callback, (LPARAM)&tailptr);

	// if we're verbose, print the list of monitors
	for (monitor_info *monitor = m_monitor_list; monitor != NULL; monitor = monitor->next())
	{
		char *utf8_device = utf8_from_tstring(monitor->info().szDevice);
		if (utf8_device != NULL)
		{
			mame_printf_verbose("Video: Monitor %p = \"%s\" %s\n", monitor->handle(), utf8_device, (monitor == m_primary_monitor) ? "(primary)" : "");
			osd_free(utf8_device);
		}
	}
}


//============================================================
//  monitor_enum_callback
//============================================================

BOOL CALLBACK video_system::monitor_enum_callback(HMONITOR handle, HDC dc, LPRECT rect, LPARAM data)
{
	// get the monitor info
	MONITORINFOEX info;
	info.cbSize = sizeof(info);
	BOOL result = GetMonitorInfo(handle, (LPMONITORINFO)&info);
	assert(result);
	(void)result; // to silence gcc 4.6

	// guess the aspect ratio assuming square pixels
	float aspect = (float)(info.rcMonitor.right - info.rcMonitor.left) / (float)(info.rcMonitor.bottom - info.rcMonitor.top);

	// allocate a new monitor info
	monitor_info *monitor = global_alloc_clear(monitor_info(aspect, handle, info));

	// save the primary monitor handle
	if (monitor->info().dwFlags & MONITORINFOF_PRIMARY)
		m_primary_monitor = monitor;

	// hook us into the list
	monitor_info ***tailptr = (monitor_info ***)data;
	**tailptr = monitor;
	*tailptr = &monitor->next();

	// enumerate all the available monitors so to list their names in verbose mode
	return TRUE;
}


//============================================================
//  video_system::ui_pause_from_main_thread
//  (main thread)
//============================================================

void video_system::ui_pause_from_main_thread(bool pause)
{
	assert(GetCurrentThreadId() == m_main_threadid);

	bool old_temp_pause = ui_temp_pause;

	// if we're pausing, increment the pause counter
	if (pause)
	{
		// if we're the first to pause, we have to actually initiate it
		if (m_ui_temp_pause++ == 0)
		{
			// only call mame_pause if we weren't already paused due to some external reason
			m_ui_temp_was_paused = m_machine.paused();
			if (!m_ui_temp_was_paused)
			{
				m_machine.pause();
			}

			set_pause_event();
		}
	}
	else // if we're resuming, decrement the pause counter
	{
		// if we're the last to resume, unpause MAME
		if (--m_ui_temp_pause == 0)
		{
			// but only do it if we were the ones who initiated it
			if (!m_ui_temp_was_paused)
			{
				m_machine.resume();
			}

			reset_pause_event();
		}
	}
}

void video_system::set_pause_event()
{
	SetEvent(m_ui_pause_event);
}

void video_system::reset_pause_event()
{
	ResetEvent(m_ui_pause_event);
}

//============================================================
//  video_system::ui_pause_from_window_thread
//  (window thread)
//============================================================

void video_system::ui_pause_from_window_thread(bool pause)
{
	assert(GetCurrentThreadId() == m_window_threadid);

	// if we're multithreaded, we have to request a pause on the main thread
	if (m_multithreading_enabled)
	{
		// request a pause from the main thread
		PostThreadMessage((DWORD)m_main_threadid, WM_USER_UI_TEMP_PAUSE, pause, 0);

		// if we're pausing, block until it happens
		if (pause)
		{
			WaitForSingleObject(m_ui_pause_event, INFINITE);
		}
	}
	else // otherwise, we just do it directly
	{
		ui_pause_from_main_thread(pause);
	}
}


//============================================================
//  get_aspect
//============================================================

float video_system::get_aspect(const char *defdata, const char *data, int report_error)
{
	int num = 0, den = 1;

	if (strcmp(data, "auto") == 0)
	{
		if (strcmp(defdata, "auto") == 0)
			return 0;
		data = defdata;
	}
	if (sscanf(data, "%d:%d", &num, &den) != 2 && report_error)
		mame_printf_error("Illegal aspect ratio value = %s\n", data);
	return (float)num / (float)den;
}



//============================================================
//  get_resolution
//============================================================

void video_system::get_resolution(const char *defdata, const char *data, window_system::window_config *config, int report_error)
{
	config->width = config->height = config->refresh = 0;
	if (strcmp(data, "auto") == 0)
	{
		if (strcmp(defdata, "auto") == 0)
			return;
		data = defdata;
	}
	if (sscanf(data, "%dx%d@%d", &config->width, &config->height, &config->refresh) < 2 && report_error)
		mame_printf_error("Illegal resolution value = %s\n", data);
}

//============================================================
//  window_has_focus
//  (main or window thread)
//============================================================

bool video_system::window_has_focus()
{
	HWND focuswnd = GetFocus();

	// see if one of the video windows has focus
	for (window_info *window = m_window_list; window != NULL; window = window->next())
		if (focuswnd == window->hwnd())
			return TRUE;

	return FALSE;
}


//============================================================
//  video_system::update_cursor_state
//  (main thread)
//============================================================

void video_system::update_cursor_state()
{
	assert(GetCurrentThreadId() == m_main_threadid);

	static POINT saved_cursor_pos = { -1, -1 };

	// if we should hide the mouse cursor, then do it
	// rules are:
	//   1. we must have focus before hiding the cursor
	//   2. we also hide the cursor in full screen mode and when the window doesn't have a menu
	//   3. we also hide the cursor in windowed mode if we're not paused and
	//      the input system requests it
	if (window_has_focus() && ((!m_video_config.windowed && !has_menu()) || (!m_machine.paused() && wininput_should_hide_mouse())))
	{
		window_info *window = win_window_list;
		RECT bounds;

		// hide cursor
		while (ShowCursor(FALSE) >= -1) ;
		ShowCursor(TRUE);

		// store the cursor position
		GetCursorPos(&saved_cursor_pos);

		// clip cursor to game video window
		GetClientRect(window->hwnd(), &bounds);
		ClientToScreen(window->hwnd(), &((POINT *)&bounds)[0]);
		ClientToScreen(window->hwnd(), &((POINT *)&bounds)[1]);
		ClipCursor(&bounds);
	}
	else
	{
		// show cursor
		while (ShowCursor(TRUE) < 1) ;
		ShowCursor(FALSE);

		// allow cursor to move freely
		ClipCursor(NULL);
		if (saved_cursor_pos.x != -1 || saved_cursor_pos.y != -1)
		{
			SetCursorPos(saved_cursor_pos.x, saved_cursor_pos.y);
			saved_cursor_pos.x = saved_cursor_pos.y = -1;
		}
	}
}


}; // namespace render::windows