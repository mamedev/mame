// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb
//============================================================
//
//  debugviewinfo.h - Win32 debug window handling
//
//============================================================

#ifndef __DEBUG_WIN_DEBUG_VIEW_INFO_H__
#define __DEBUG_WIN_DEBUG_VIEW_INFO_H__

#include "debugwin.h"

#include "debugbaseinfo.h"

#include "emu.h"
#include "debug/debugvw.h"


class debugview_info : protected debugbase_info
{
public:
	debugview_info(debugger_windows_interface &debugger, debugwin_info &owner, HWND parent, debug_view_type type);
	virtual ~debugview_info();

	bool is_valid() const;

	bool owns_window(HWND wnd) const { return m_wnd == wnd; }

	UINT32 prefwidth() const;
	UINT32 maxwidth();
	void get_bounds(RECT &bounds) const;
	void set_bounds(RECT const &newbounds);

	void send_vscroll(int delta);
	void send_pageup();
	void send_pagedown();
	void set_focus() const { SetFocus(m_wnd); }

	debug_view_type type() const { return m_view->type(); }
	debug_view_xy total_size() const { return m_view->total_size(); }
	bool cursor_supported() const { return m_view->cursor_supported(); }
	bool cursor_visible() const { return m_view->cursor_visible(); }

	char const *source_name() const;
	device_t *source_device() const;
	bool source_is_visible_cpu() const;
	bool set_source_index(int index);
	bool set_source_for_device(device_t &device);
	bool set_source_for_visible_cpu();

	HWND create_source_combobox(HWND parent, LONG_PTR userdata);

protected:
	template <typename T> T *view() const { return downcast<T *>(m_view); }

private:
	void draw_contents(HDC windc);
	void update();
	UINT32 process_scroll(WORD type, HWND wnd);
	LRESULT view_proc(UINT message, WPARAM wparam, LPARAM lparam);

	static void static_update(debug_view &view, void *osdprivate);
	static LRESULT CALLBACK static_view_proc(HWND wnd, UINT message, WPARAM wparam, LPARAM lparam);

	static void register_window_class();

	debugwin_info   &m_owner;
	debug_view      *m_view;
	HWND            m_wnd;
	HWND            m_hscroll;
	HWND            m_vscroll;

	static bool     s_window_class_registered;
};

#endif
