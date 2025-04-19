// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb
//============================================================
//
//  debugwininfo.h - Win32 debug window handling
//
//============================================================
#ifndef MAME_DEBUGGER_WIN_DEBUGWININFO_H
#define MAME_DEBUGGER_WIN_DEBUGWININFO_H

#pragma once

#include "debugwin.h"

#include "debugbaseinfo.h"


namespace osd::debugger::win {

class debugwin_info : protected debugbase_info
{
public:
	virtual ~debugwin_info();

	bool is_valid() const { return m_wnd != nullptr; }

	void set_ignore_char_lparam(LPARAM value) { m_ignore_char_lparam = value >> 16; }
	bool check_ignore_char_lparam(LPARAM value)
	{
		if (m_ignore_char_lparam == (value >> 16))
		{
			m_ignore_char_lparam = 0;
			return false;
		}
		else
		{
			return true;
		}
	}

	void show() const { smart_show_window(m_wnd, true); }
	void hide() const { smart_show_window(m_wnd, false); }
	void set_foreground() const { SetForegroundWindow(m_wnd); }
	void redraw();
	void destroy();

	virtual bool set_default_focus();
	void prev_view(debugview_info *curview);
	void next_view(debugview_info *curview);
	virtual bool restore_field(HWND wnd) { return false; }

	virtual bool handle_key(WPARAM wparam, LPARAM lparam);

	void save_configuration(util::xml::data_node &parentnode);
	virtual void restore_configuration_from_node(util::xml::data_node const &node);

protected:
	static DWORD const  DEBUG_WINDOW_STYLE = (WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN) & (~WS_MINIMIZEBOX & ~WS_MAXIMIZEBOX);
	static DWORD const  DEBUG_WINDOW_STYLE_EX = 0;

	static int const    EDGE_WIDTH = 3;

	enum
	{
		ID_NEW_MEMORY_WND = 1,
		ID_NEW_DISASM_WND,
		ID_NEW_LOG_WND,
		ID_NEW_POINTS_WND,
		ID_RUN,
		ID_RUN_AND_HIDE,
		ID_RUN_VBLANK,
		ID_RUN_IRQ,
		ID_NEXT_CPU,
		ID_STEP,
		ID_STEP_OVER,
		ID_STEP_OUT,
		ID_REWIND_STEP,
		ID_HARD_RESET,
		ID_SOFT_RESET,
		ID_EXIT,

		ID_1_BYTE_CHUNKS_HEX,
		ID_2_BYTE_CHUNKS_HEX,
		ID_4_BYTE_CHUNKS_HEX,
		ID_8_BYTE_CHUNKS_HEX,
		ID_1_BYTE_CHUNKS_OCT,
		ID_2_BYTE_CHUNKS_OCT,
		ID_4_BYTE_CHUNKS_OCT,
		ID_8_BYTE_CHUNKS_OCT,
		ID_FLOAT_32BIT,
		ID_FLOAT_64BIT,
		ID_FLOAT_80BIT,
		ID_HEX_ADDRESSES,
		ID_DEC_ADDRESSES,
		ID_OCT_ADDRESSES,
		ID_LOGICAL_ADDRESSES,
		ID_PHYSICAL_ADDRESSES,
		ID_REVERSE_VIEW,
		ID_INCREASE_MEM_WIDTH,
		ID_DECREASE_MEM_WIDTH,

		ID_TOGGLE_BREAKPOINT,
		ID_DISABLE_BREAKPOINT,
		ID_RUN_TO_CURSOR,
		ID_SHOW_RAW,
		ID_SHOW_ENCRYPTED,
		ID_SHOW_COMMENTS,
		ID_SHOW_SOURCE,
		ID_SHOW_DISASM,

		ID_SHOW_BREAKPOINTS,
		ID_SHOW_WATCHPOINTS,
		ID_SHOW_REGISTERPOINTS,
		ID_SHOW_EXCEPTIONPOINTS,

		ID_CLEAR_LOG,

		ID_SAVE_WINDOWS,
		ID_GROUP_WINDOWS,
		ID_LIGHT_BACKGROUND,
		ID_DARK_BACKGROUND,

		ID_DEVICE_OPTIONS   // always keep this at the end
	};

	// Indices for use with m_views.  Each *win_info class that can appear
	// as a frame within the main consolewin_info should use these
	// indices to find its *view_info class, even when it's currently
	// shown as an independent free-floating window.  All other *win_info
	// classes (e.g., logwin_info, pointswin_info, etc.) just use m_views[0]
	enum
	{
		VIEW_IDX_SOURCE,
		VIEW_IDX_DISASM,
		VIEW_IDX_STATE,
		VIEW_IDX_CONSOLE,
		MAX_VIEWS
	};

	debugwin_info(debugger_windows_interface &debugger, bool is_main_console, LPCSTR title, WNDPROC handler);

	bool is_main_console() const { return m_is_main_console; }
	HWND window() const { return m_wnd; }
	uint32_t minwidth() const { return m_minwidth; }
	uint32_t maxwidth() const { return m_maxwidth; }
	void set_minwidth(uint32_t value) { m_minwidth = value; }
	void set_maxwidth(uint32_t value) { m_maxwidth = value; }

	virtual void recompute_children();
	virtual void update_menu() { }
	virtual bool handle_command(WPARAM wparam, LPARAM lparam);
	virtual void draw_contents(HDC dc);
	void draw_border(HDC dc, RECT &bounds);
	void draw_border(HDC dc, HWND child);

	virtual void save_configuration_to_node(util::xml::data_node &node);

	std::unique_ptr<debugview_info>    m_views[MAX_VIEWS];

private:
	LRESULT window_proc(UINT message, WPARAM wparam, LPARAM lparam);

	HMENU create_standard_menubar();

	static LRESULT CALLBACK static_window_proc(HWND wnd, UINT message, WPARAM wparam, LPARAM lparam);

	static void register_window_class();

	bool const      m_is_main_console;

	HWND            m_wnd;
	WNDPROC const   m_handler;

	uint32_t        m_minwidth, m_maxwidth;
	uint32_t        m_minheight, m_maxheight;

	uint16_t        m_ignore_char_lparam;

	static bool     s_window_class_registered;
};

} // namespace osd::debugger::win

#endif // MAME_DEBUGGER_WIN_DEBUGWININFO_H
