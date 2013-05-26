//============================================================
//
//  drawhal.h - Windows render abstraction layer
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

#ifndef __RENDER_WINDOWS_WINDOW__
#define __RENDER_WINDOWS_WINDOW__

#include "video.h"
#include "render.h"
#include "render/window.h"

//============================================================
//  TYPE DEFINITIONS
//============================================================

namespace render
{

class draw_hal;

typedef DWORD render::threadid;

namespace windows
{

class window_system : public render::window_system
{
public:
	window_system(running_machine &machine, video_system *video);
	virtual ~window_system();

	virtual void		process_events_periodic();

	virtual void		toggle_full_screen();

	void				is_mame_window(HWND hwnd);

private:
	HANDLE 				m_window_thread;

	HANDLE 				m_window_thread_ready_event;
};

class window_info : public render::window_info
{
public:
	window_info(running_machine &machine, threadid main_threadid, threadid window_threadid, monitor_info *monitor)
		: render::window_info(machine, main_threadid, window_threadid), m_monitor(monitor) { }
	~window_info();

	draw_hal *			hal() { return m_hal; }

	DWORD				lastclicktime() { return m_lastclicktime; }
	int					lastclickx() { return m_lastclickx; }
	int					lastclicky() { return m_lastclicky; }

	void				set_lastclickx(int lastclickx) { m_lastclickx = lastclickx; }
	void				set_lastclicky(int lastclicky) { m_lastclicky = lastclicky; }
	void				set_lastclicktime(DWORD lastclicktime) { m_lastclicktime = lastclicktime; }

	void				set_resize_state(int resize_state) { m_resize_state = resize_state; }

	virtual void		update_bounds();

	virtual void		toggle_fullscreen(bool fullscreen);

	virtual int			extra_width();
	virtual int			extra_height();

	virtual bool		has_menu();

	virtual HWND		hwnd() { return m_hwnd; }

	virtual void		minimize_window();
	virtual void		maximize_window();

private:
	draw_hal *				m_hal;

	void					get_max_bounds(RECT *bounds, bool constrain);
	void					adjust_window_position_after_major_change();
	void					constrain_to_aspect_ratio(RECT *rect, int adjustment);

	// window handle and info
	HWND                	m_hwnd;
	char                	m_title[256];
	RECT                	m_non_fullscreen_bounds;
	int                 	m_startmaximized;
	int                 	m_isminimized;
	int                 	m_ismaximized;
	int                 	m_resize_state;

	// monitor info
	monitor_info *			m_monitor;

	// input info
	DWORD               	m_lastclicktime;
	int                 	m_lastclickx;
	int                 	m_lastclicky;
};

}}; // namespace render::windows

#endif // __RENDER_WINDOWS_WINDOW__