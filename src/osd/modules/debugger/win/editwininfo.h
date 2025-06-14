// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb
//============================================================
//
//  editwininfo.h - Win32 debug window handling
//
//============================================================
#ifndef MAME_DEBUGGER_WIN_EDITWININFO_H
#define MAME_DEBUGGER_WIN_EDITWININFO_H

#pragma once

#include "debugwin.h"

#include "debugwininfo.h"

#include <deque>
#include <string>


namespace osd::debugger::win {

class editwin_info : public debugwin_info
{
public:
	editwin_info(debugger_windows_interface &debugger, bool is_main_console, int viewidx, LPCSTR title, WNDPROC handler);
	virtual ~editwin_info();

	virtual bool restore_field(HWND wnd) override;

	virtual bool set_default_focus() override;

	virtual void restore_configuration_from_node(util::xml::data_node const &node) override;

protected:
	constexpr static DWORD  COMBO_BOX_STYLE     = WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL;
	constexpr static DWORD  COMBO_BOX_STYLE_EX  = 0;

	void set_editwnd_bounds(RECT const &bounds);
	void set_editwnd_text(char const *text);
	void editwnd_select_all();
	void set_edit_defstr(const std::string &string) { m_edit_defstr = string; }

	virtual void draw_contents(HDC dc) override;

	virtual void save_configuration_to_node(util::xml::data_node &node) override;

private:
	typedef std::deque<std::basic_string<TCHAR> > history_deque;

	virtual void process_string(const std::string &string) = 0;

	LRESULT edit_proc(UINT message, WPARAM wparam, LPARAM lparam);

	static LRESULT CALLBACK static_edit_proc(HWND wnd, UINT message, WPARAM wparam, LPARAM lparam);

	HWND                    m_editwnd;
	int                     m_viewidx;
	std::string             m_edit_defstr;
	WNDPROC                 m_original_editproc;
	history_deque           m_history;
	int                     m_last_history;
};

} // namespace osd::debugger::win

#endif // MAME_DEBUGGER_WIN_EDITWININFO_H
