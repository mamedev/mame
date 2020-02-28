// license:BSD-3-Clause
// copyright-holders:Aaron Giles
//============================================================
//
//  winmenu.cpp - Win32 OSD core dummy menu implementation
//
//============================================================

// standard windows headers
#include <windows.h>

#include "emu.h"

#include "window.h"

//============================================================
//  winwindow_video_window_proc_ui
//  (window thread)
//============================================================

LRESULT CALLBACK winwindow_video_window_proc_ui(HWND wnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	return win_window_info::video_window_proc(wnd, message, wparam, lparam);
}

//============================================================
//  win_create_menu
//============================================================

int win_create_menu(running_machine &machine, HMENU *menus)
{
	return 0;
}
