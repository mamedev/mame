// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb
//============================================================
//
//  debugviewinfo.h - Win32 debug window handling
//
//============================================================
#ifndef MAME_DEBUGGER_WIN_DEBUGVIEWINFO_H
#define MAME_DEBUGGER_WIN_DEBUGVIEWINFO_H

#pragma once

#include "debugwin.h"

#include "debugbaseinfo.h"

#include "debug/debugvw.h"


namespace osd::debugger::win {

class debugview_info : protected debugbase_info
{
public:
	debugview_info(debugger_windows_interface &debugger, debugwin_info &owner, HWND parent, debug_view_type type);
	virtual ~debugview_info();

	bool is_valid() const;

	bool owns_window(HWND wnd) const { return m_wnd == wnd; }

	uint32_t prefwidth() const;
	uint32_t maxwidth();
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

	int source_index() const;
	char const *source_name() const;
	device_t *source_device() const;
	bool source_is_visible_cpu() const;
	bool set_source_index(int index);
	bool set_source_for_device(device_t &device);
	bool set_source_for_visible_cpu();

	HWND create_source_combobox(HWND parent, LONG_PTR userdata);

	void show() const { smart_show_window(m_wnd, true); }
	void hide() const { smart_show_window(m_wnd, false); }
	bool is_visible() const {   return IsWindowVisible(m_wnd); }

	virtual void restore_configuration_from_node(util::xml::data_node const &node);
	virtual void save_configuration_to_node(util::xml::data_node &node);

protected:
	enum
	{
		ID_CONTEXT_COPY_VISIBLE = 1,
		ID_CONTEXT_PASTE
	};

	template <typename T> T *view() const { return downcast<T *>(m_view); }

	virtual void add_items_to_context_menu(HMENU menu);
	virtual void update_context_menu(HMENU menu);
	virtual void handle_context_menu(unsigned command);
	virtual void update();

private:
	void draw_contents(HDC windc);
	uint32_t process_scroll(WORD type, HWND wnd);
	bool process_context_menu(int x, int y);
	LRESULT view_proc(UINT message, WPARAM wparam, LPARAM lparam);

	static void static_update(debug_view &view, void *osdprivate);
	static LRESULT CALLBACK static_view_proc(HWND wnd, UINT message, WPARAM wparam, LPARAM lparam);

	static void register_window_class();

	debugwin_info   &m_owner;
	debug_view      *m_view;
	HWND            m_wnd;
	HWND            m_hscroll;
	HWND            m_vscroll;
	HMENU           m_contextmenu;

	static bool     s_window_class_registered;
};

} // namespace osd::debugger::win

#endif // MAME_DEBUGGER_WIN_DEBUGVIEWINFO_H
