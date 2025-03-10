// license:BSD-3-Clause
// copyright-holders:David Broman
//============================================================
//
//  sourcewininfo.h - Win32 debug window handling
//
//============================================================
#ifndef MAME_DEBUGGER_WIN_SOURCEWININFO_H
#define MAME_DEBUGGER_WIN_SOURCEWININFO_H

#pragma once

#include "disasmbasewininfo.h"
#include "debugwininfo.h"

namespace osd::debugger::win {

// Manages source-code-level debugging window.  Derives from disasmbasewin_info to reuse
// breakpoint handling and more, though with customizations.
class sourcewin_info : public disasmbasewin_info
{
public:
	sourcewin_info(debugger_windows_interface &debugger, bool is_main_console, LPCSTR title, WNDPROC handler);
	virtual ~sourcewin_info();
	virtual bool handle_key(WPARAM wparam, LPARAM lparam) override;

protected:
	virtual void draw_contents(HDC dc) override;
	virtual bool handle_command(WPARAM wparam, LPARAM lparam) override;
	void set_srcwnd_bounds(RECT const &bounds);
	bool show_src_window();
	void hide_src_window();

private:
	bool handle_sourcewin_command(WPARAM wparam, LPARAM lparam);
	virtual void process_string(const std::string &string) override {};

	HWND    m_filecombownd;
};

} // namespace osd::debugger::win

#endif // MAME_DEBUGGER_WIN_SOURCEWININFO_H
