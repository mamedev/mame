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

//class draw_hal;

namespace windows
{

class window_system : public render::window_system
{
public:
	window_system(running_machine &machine, video_system *video);
	virtual ~window_system();

	virtual draw_hal *	hal() { return m_hal; }

	virtual void		process_events_periodic();

	virtual void		toggle_full_screen();

	virtual render::window_info *window_alloc(render::monitor_info *monitor, int index);

	virtual void		take_snap();
	virtual void		toggle_fsfx();
	virtual void		take_video();

	virtual window_info *	window_list() { return m_window_list; }

	void				shutdown();

	void				create_window_class();
	bool				is_mame_window(HWND hwnd);
	void				ui_exec_on_main_thread(void (*func)(void *), void *param);
	static unsigned __stdcall thread_entry(void *param);

	void				ui_pause_from_main_thread(bool pause);
	void				ui_pause_from_window_thread(bool pause);

	bool				ui_is_paused() { return m_machine.paused() && m_ui_temp_was_paused; }

	HANDLE				window_thread_ready_event() { return m_window_thread_ready_event; }

	virtual void		process_events(bool ingame);
	void				dispatch_message(MSG *message);

	LRESULT CALLBACK 	window_proc_ui(HWND wnd, UINT message, WPARAM wparam, LPARAM lparam);
	LRESULT CALLBACK	window_proc(HWND wnd, UINT message, WPARAM wparam, LPARAM lparam);

protected:
	virtual void		set_pause_event();
	virtual void		reset_pause_event();

	draw_hal *			m_hal;

	int					m_ui_temp_pause;
	int					m_ui_temp_was_paused;

private:
	HANDLE 				m_window_thread;
	HANDLE 				m_window_thread_ready_event;
	HANDLE 				m_ui_pause_event;

	UINT32				m_last_event_check;

	bool				m_classes_created;

	bool				m_in_background;
};

class window_info : public render::window_info
{
public:
	window_info(running_machine &machine, int index, UINT64 main_threadid, UINT64 window_threadid,
				window_system *system, monitor_info *monitor)
		: render::window_info(machine, index, main_threadid, window_threadid, system, monitor) { }
	~window_info();

	virtual void		init_hal();

	virtual void		update();

	virtual window_info *next() { return (render::windows::window_info *)m_next; }

	virtual bool		complete_create();
	virtual void		wait_for_ready();

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

	monitor_info *		monitor() { return m_monitor; }

	HWND				hwnd() { return m_hwnd; }
	void				set_hwnd(HWND hwnd) { m_hwnd = hwnd; } // Hodor, hodor hodor!

	virtual void		minimize_window();
	virtual void		maximize_window();

	virtual int			resize_state() { return m_resize_state; }

	virtual void		get_window_rect(math::rectf *bounds);

	void				draw_video_contents(HDC dc, int update);

	void				constrain_to_aspect_ratio(RECT *rect, int adjustment);

private:
	void					get_min_bounds(math::rectf *bounds, bool constrain);
	void					get_max_bounds(math::rectf *bounds, bool constrain);
	void					adjust_window_position_after_major_change();

	monitor_info *			monitor(const RECT *proposed);

	// window handle and info
	HWND                	m_hwnd;
	RECT                	m_non_fullscreen_bounds;
	int                 	m_resize_state;

	DWORD 					m_last_update_time;

	// input info
	DWORD               	m_lastclicktime;
	int                 	m_lastclickx;
	int                 	m_lastclicky;

	int						m_win_physical_width;
	int						m_win_physical_height;
};

}}; // namespace render::windows

#endif // __RENDER_WINDOWS_WINDOW__