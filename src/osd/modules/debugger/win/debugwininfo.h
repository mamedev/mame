// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb
//============================================================
//
//  debugwininfo.h - Win32 debug window handling
//
//============================================================

#ifndef __DEBUG_WIN_DEBUG_WIN_INFO_H__
#define __DEBUG_WIN_DEBUG_WIN_INFO_H__

#include "debugwin.h"

#include "debugbaseinfo.h"

#include "emu.h"


class debugwin_info : protected debugbase_info
{
public:
	template<class U> friend class simple_list;

	debugwin_info(debugger_windows_interface &debugger, bool is_main_console, LPCSTR title, WNDPROC handler);
	virtual ~debugwin_info();

	bool is_valid() const { return m_wnd != nullptr; }
	debugwin_info *next() const { return m_next; }

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

	void show() { smart_show_window(m_wnd, true); }
	void hide() { smart_show_window(m_wnd, false); }
	void set_foreground() { SetForegroundWindow(m_wnd); }
	void destroy();

	virtual bool set_default_focus();
	void prev_view(debugview_info *curview);
	void next_view(debugview_info *curview);
	virtual bool restore_field(HWND wnd) { return false; }

	virtual bool handle_key(WPARAM wparam, LPARAM lparam);

protected:
	static DWORD const  DEBUG_WINDOW_STYLE = (WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN) & (~WS_MINIMIZEBOX & ~WS_MAXIMIZEBOX);
	static DWORD const  DEBUG_WINDOW_STYLE_EX = 0;

	static int const    MAX_VIEWS = 4;
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
		ID_HARD_RESET,
		ID_SOFT_RESET,
		ID_EXIT,

		ID_1_BYTE_CHUNKS,
		ID_2_BYTE_CHUNKS,
		ID_4_BYTE_CHUNKS,
		ID_8_BYTE_CHUNKS,
		ID_FLOATING_POINT_32BIT,
		ID_FLOATING_POINT_64BIT,
		ID_FLOATING_POINT_80BIT,
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

		ID_SHOW_BREAKPOINTS,
		ID_SHOW_WATCHPOINTS,

		ID_DEVICE_OPTIONS   // always keep this at the end
	};

	bool is_main_console() const { return m_is_main_console; }
	HWND window() const { return m_wnd; }
	UINT32 minwidth() const { return m_minwidth; }
	UINT32 maxwidth() const { return m_maxwidth; }
	void set_minwidth(UINT32 value) { m_minwidth = value; }
	void set_maxwidth(UINT32 value) { m_maxwidth = value; }

	virtual void recompute_children();
	virtual void update_menu() { }
	virtual bool handle_command(WPARAM wparam, LPARAM lparam);
	virtual void draw_contents(HDC dc);
	void draw_border(HDC dc, RECT &bounds);
	void draw_border(HDC dc, HWND child);

	std::unique_ptr<debugview_info>    m_views[MAX_VIEWS];

private:
	LRESULT window_proc(UINT message, WPARAM wparam, LPARAM lparam);

	HMENU create_standard_menubar();

	static LRESULT CALLBACK static_window_proc(HWND wnd, UINT message, WPARAM wparam, LPARAM lparam);

	static void register_window_class();

	bool const      m_is_main_console;

	debugwin_info   *m_next;
	HWND            m_wnd;
	WNDPROC const   m_handler;

	UINT32          m_minwidth, m_maxwidth;
	UINT32          m_minheight, m_maxheight;

	UINT16          m_ignore_char_lparam;

	static bool     s_window_class_registered;
};

#endif
