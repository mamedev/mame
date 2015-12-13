// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb
//============================================================
//
//  debugbaseinfo.c - Win32 debug window handling
//
//============================================================

#include "debugbaseinfo.h"


debugbase_info::debugbase_info(debugger_windows_interface &debugger) :
	m_debugger(debugger),
	m_machine(debugger.machine()),
	m_metrics(debugger.metrics()),
	m_waiting_for_debugger(debugger.waiting_for_debugger())
{
}


void debugbase_info::smart_set_window_bounds(HWND wnd, HWND parent, RECT const &bounds)
{
	RECT curbounds;
	int flags = 0;

	// first get the current bounds, relative to the parent
	GetWindowRect(wnd, &curbounds);
	if (parent != nullptr)
	{
		RECT parentbounds;
		GetWindowRect(parent, &parentbounds);
		curbounds.top -= parentbounds.top;
		curbounds.bottom -= parentbounds.top;
		curbounds.left -= parentbounds.left;
		curbounds.right -= parentbounds.left;
	}

	// if the position matches, don't change it
	if (curbounds.top == bounds.top && curbounds.left == bounds.left)
		flags |= SWP_NOMOVE;
	if ((curbounds.bottom - curbounds.top) == (bounds.bottom - bounds.top) &&
		(curbounds.right - curbounds.left) == (bounds.right - bounds.left))
		flags |= SWP_NOSIZE;

	// if we need to, reposition the window
	if (flags != (SWP_NOMOVE | SWP_NOSIZE))
		SetWindowPos(wnd, nullptr,
					bounds.left, bounds.top,
					bounds.right - bounds.left, bounds.bottom - bounds.top,
					SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOZORDER | flags);
}



void debugbase_info::smart_show_window(HWND wnd, bool show)
{
	BOOL const visible = IsWindowVisible(wnd);
	if ((visible && !show) || (!visible && show))
		ShowWindow(wnd, show ? SW_SHOW : SW_HIDE);
}
