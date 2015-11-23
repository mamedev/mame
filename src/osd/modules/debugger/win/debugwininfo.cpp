// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb
//============================================================
//
//  debugwininfo.c - Win32 debug window handling
//
//============================================================

#include "debugwininfo.h"

#include "debugviewinfo.h"

#include "debug/debugcpu.h"
#include "window.h"
#include "winutf8.h"

#include "winutil.h"


bool debugwin_info::s_window_class_registered = false;


debugwin_info::debugwin_info(debugger_windows_interface &debugger, bool is_main_console, LPCSTR title, WNDPROC handler) :
	debugbase_info(debugger),
	m_is_main_console(is_main_console),
	m_next(NULL),
	m_wnd(NULL),
	m_handler(handler),
	m_minwidth(200),
	m_maxwidth(0),
	m_minheight(200),
	m_maxheight(0),
	m_ignore_char_lparam(0)
{
	register_window_class();

	m_wnd = win_create_window_ex_utf8(DEBUG_WINDOW_STYLE_EX, "MAMEDebugWindow", title, DEBUG_WINDOW_STYLE,
			0, 0, 100, 100, win_window_list->m_hwnd, create_standard_menubar(), GetModuleHandleUni(), this);
	if (m_wnd == NULL)
		return;

	RECT work_bounds;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &work_bounds, 0);
	m_maxwidth = work_bounds.right - work_bounds.left;
	m_maxheight = work_bounds.bottom - work_bounds.top;
}


debugwin_info::~debugwin_info()
{
}


void debugwin_info::destroy()
{
	for (int curview = 0; curview < MAX_VIEWS; curview++)
		m_views[curview].reset();
	DestroyWindow(m_wnd);
}

bool debugwin_info::set_default_focus()
{
	return false;
}


void debugwin_info::prev_view(debugview_info *curview)
{
	// count the number of views
	int numviews;
	for (numviews = 0; numviews < MAX_VIEWS; numviews++)
	{
		if (m_views[numviews] == NULL)
			break;
	}

	// if we have a curview, find out its index
	int curindex = 1;
	if (curview)
	{
		for (curindex = numviews - 1; curindex > 0; curindex--)
		{
			if (m_views[curindex] == curview)
				break;
		}
		if (curindex < 0)
			curindex = 1;
	}

	// loop until we find someone to take focus
	while (1)
	{
		// advance to the previous index
		curindex--;
		if (curindex < -1)
			curindex = numviews - 1;

		if (curindex < 0 && set_default_focus())
		{
			// negative numbers mean the focuswnd
			break;
		}
		else if (curindex >= 0 && m_views[curindex] != NULL && m_views[curindex]->cursor_supported())
		{
			// positive numbers mean a view
			m_views[curindex]->set_focus();
			break;
		}
	}
}


void debugwin_info::next_view(debugview_info *curview)
{
	// count the number of views
	int numviews;
	for (numviews = 0; numviews < MAX_VIEWS; numviews++)
	{
		if (m_views[numviews] == NULL)
			break;
	}

	// if we have a curview, find out its index
	int curindex = -1;
	if (curview)
	{
		for (curindex = numviews - 1; curindex > 0; curindex--)
		{
			if (m_views[curindex] == curview)
				break;
		}
	}

	// loop until we find someone to take focus
	while (1)
	{
		// advance to the previous index
		curindex++;
		if (curindex >= numviews)
			curindex = -1;

		if (curindex < 0 && set_default_focus())
		{
			// negative numbers mean the focuswnd
			break;
		}
		else if (curindex >= 0 && m_views[curindex] != NULL && m_views[curindex]->cursor_supported())
		{
			// positive numbers mean a view
			m_views[curindex]->set_focus();
			break;
		}
	}
}


bool debugwin_info::handle_key(WPARAM wparam, LPARAM lparam)
{
	/* ignore any keys that are received while the debug key is down */
	if (!waiting_for_debugger() && seq_pressed())
		return true;

	switch (wparam)
	{
	case VK_F3:
		if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
			SendMessage(m_wnd, WM_COMMAND, ID_HARD_RESET, 0);
		else
			SendMessage(m_wnd, WM_COMMAND, ID_SOFT_RESET, 0);
		return true;

	case VK_F4:
		if (GetAsyncKeyState(VK_MENU) & 0x8000)
		{
			/* ajg - never gets here since 'alt' seems to be captured somewhere else - menu maybe? */
			SendMessage(m_wnd, WM_COMMAND, ID_EXIT, 0);
			return true;
		}
		break;

	case VK_F5:
		SendMessage(m_wnd, WM_COMMAND, ID_RUN, 0);
		return true;

	case VK_F6:
		SendMessage(m_wnd, WM_COMMAND, ID_NEXT_CPU, 0);
		return true;

	case VK_F7:
		SendMessage(m_wnd, WM_COMMAND, ID_RUN_IRQ, 0);
		return true;

	case VK_F8:
		SendMessage(m_wnd, WM_COMMAND, ID_RUN_VBLANK, 0);
		return true;

	case VK_F10:
		SendMessage(m_wnd, WM_COMMAND, ID_STEP_OVER, 0);
		return true;

	case VK_F11:
		if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
			SendMessage(m_wnd, WM_COMMAND, ID_STEP_OUT, 0);
		else
			SendMessage(m_wnd, WM_COMMAND, ID_STEP, 0);
		return true;

	case VK_F12:
		SendMessage(m_wnd, WM_COMMAND, ID_RUN_AND_HIDE, 0);
		return true;

	case 'M':
		if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
		{
			SendMessage(m_wnd, WM_COMMAND, ID_NEW_MEMORY_WND, 0);
			return true;
		}
		break;

	case 'D':
		if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
		{
			SendMessage(m_wnd, WM_COMMAND, ID_NEW_DISASM_WND, 0);
			return true;
		}
		break;

	case 'L':
		if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
		{
			SendMessage(m_wnd, WM_COMMAND, ID_NEW_LOG_WND, 0);
			return true;
		}
		break;

	case 'B':
		if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
		{
			SendMessage(m_wnd, WM_COMMAND, ID_NEW_POINTS_WND, 0);
			return true;
		}
		break;
	}

	return false;
}


void debugwin_info::recompute_children()
{
	if (m_views[0] != NULL)
	{
		// compute a client rect
		RECT bounds;
		bounds.top = bounds.left = 0;
		bounds.right = m_views[0]->prefwidth() + (2 * EDGE_WIDTH);
		bounds.bottom = 200;
		AdjustWindowRectEx(&bounds, DEBUG_WINDOW_STYLE, FALSE, DEBUG_WINDOW_STYLE_EX);

		// clamp the min/max size
		set_maxwidth(bounds.right - bounds.left);

		// get the parent's dimensions
		RECT parent;
		GetClientRect(window(), &parent);

		// view gets the remaining space
		InflateRect(&parent, -EDGE_WIDTH, -EDGE_WIDTH);
		m_views[0]->set_bounds(parent);
	}
}


bool debugwin_info::handle_command(WPARAM wparam, LPARAM lparam)
{
	if (HIWORD(wparam) == 0)
	{
		switch (LOWORD(wparam))
		{
		case ID_NEW_MEMORY_WND:
			debugger().create_memory_window();
			return true;

		case ID_NEW_DISASM_WND:
			debugger().create_disasm_window();
			return true;

		case ID_NEW_LOG_WND:
			debugger().create_log_window();
			return true;

		case ID_NEW_POINTS_WND:
			debugger().create_points_window();
			return true;

		case ID_RUN_AND_HIDE:
			debugger().hide_all();
		case ID_RUN:
			debug_cpu_get_visible_cpu(machine())->debug()->go();
			return true;

		case ID_NEXT_CPU:
			debug_cpu_get_visible_cpu(machine())->debug()->go_next_device();
			return true;

		case ID_RUN_VBLANK:
			debug_cpu_get_visible_cpu(machine())->debug()->go_vblank();
			return true;

		case ID_RUN_IRQ:
			debug_cpu_get_visible_cpu(machine())->debug()->go_interrupt();
			return true;

		case ID_STEP:
			debug_cpu_get_visible_cpu(machine())->debug()->single_step();
			return true;

		case ID_STEP_OVER:
			debug_cpu_get_visible_cpu(machine())->debug()->single_step_over();
			return true;

		case ID_STEP_OUT:
			debug_cpu_get_visible_cpu(machine())->debug()->single_step_out();
			return true;

		case ID_HARD_RESET:
			machine().schedule_hard_reset();
			return true;

		case ID_SOFT_RESET:
			machine().schedule_soft_reset();
			debug_cpu_get_visible_cpu(machine())->debug()->go();
			return true;

		case ID_EXIT:
			set_default_focus();
			machine().schedule_exit();
			return true;
		}
	}
	return false;
}


void debugwin_info::draw_contents(HDC dc)
{
	// fill the background with light gray
	RECT parent;
	GetClientRect(m_wnd, &parent);
	FillRect(dc, &parent, (HBRUSH)GetStockObject(LTGRAY_BRUSH));

	// draw edges around all views
	for (int curview = 0; curview < MAX_VIEWS; curview++)
	{
		if (m_views[curview] != NULL)
		{
			RECT bounds;
			m_views[curview]->get_bounds(bounds);
			draw_border(dc, bounds);
		}
	}
}


void debugwin_info::draw_border(HDC dc, RECT &bounds)
{
	ScreenToClient(m_wnd, &((POINT *)&bounds)[0]);
	ScreenToClient(m_wnd, &((POINT *)&bounds)[1]);
	InflateRect(&bounds, EDGE_WIDTH, EDGE_WIDTH);
	DrawEdge(dc, &bounds, EDGE_SUNKEN, BF_RECT);
}


void debugwin_info::draw_border(HDC dc, HWND child)
{
	RECT bounds;
	GetWindowRect(child, &bounds);
	draw_border(dc, bounds);
}


LRESULT debugwin_info::window_proc(UINT message, WPARAM wparam, LPARAM lparam)
{
	// handle a few messages
	switch (message)
	{
	// paint: draw bezels as necessary
	case WM_PAINT:
		{
			PAINTSTRUCT pstruct;
			HDC dc = BeginPaint(m_wnd, &pstruct);
			draw_contents(dc);
			EndPaint(m_wnd, &pstruct);
			break;
		}

	// keydown: handle debugger keys
	case WM_KEYDOWN:
		if (handle_key(wparam, lparam))
			set_ignore_char_lparam(lparam);
		break;

	// char: ignore chars associated with keys we've handled
	case WM_CHAR:
		if (check_ignore_char_lparam(lparam))
		{
			if (waiting_for_debugger() || !seq_pressed())
				return DefWindowProc(m_wnd, message, wparam, lparam);
		}
		break;

	// activate: set the focus
	case WM_ACTIVATE:
		if (wparam != WA_INACTIVE)
			set_default_focus();
		break;

	// get min/max info: set the minimum window size
	case WM_GETMINMAXINFO:
		{
			MINMAXINFO *minmax = (MINMAXINFO *)lparam;
			minmax->ptMinTrackSize.x = m_minwidth;
			minmax->ptMinTrackSize.y = m_minheight;
			minmax->ptMaxSize.x = minmax->ptMaxTrackSize.x = m_maxwidth;
			minmax->ptMaxSize.y = minmax->ptMaxTrackSize.y = m_maxheight;
			break;
		}

	// sizing: recompute child window locations
	case WM_SIZE:
	case WM_SIZING:
		recompute_children();
		InvalidateRect(m_wnd, NULL, FALSE);
		break;

	// mouse wheel: forward to the first view
	case WM_MOUSEWHEEL:
		{
			static int units_carryover = 0;

			UINT lines_per_click;
			if (!SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &lines_per_click, 0))
				lines_per_click = 3;

			int const units = GET_WHEEL_DELTA_WPARAM(wparam) + units_carryover;
			int const clicks = units / WHEEL_DELTA;
			units_carryover = units % WHEEL_DELTA;

			int const delta = clicks * lines_per_click;
			int viewnum = 0;
			POINT point;

			// figure out which view we are hovering over
			GetCursorPos(&point);
			ScreenToClient(m_wnd, &point);
			HWND const child = ChildWindowFromPoint(m_wnd, point);
			if (child)
			{
				for (viewnum = 0; viewnum < MAX_VIEWS; viewnum++)
				{
					if ((m_views[viewnum] != NULL) && m_views[viewnum]->owns_window(child))
						break;
				}
				if (viewnum == MAX_VIEWS)
					break;
			}

			// send the appropriate message to this view's scrollbar
			if (m_views[viewnum] != NULL)
				m_views[viewnum]->send_vscroll(delta);

			break;
		}

	// activate: set the focus
	case WM_INITMENU:
		update_menu();
		break;

	// command: handle a comment
	case WM_COMMAND:
		if (!handle_command(wparam, lparam))
			return DefWindowProc(m_wnd, message, wparam, lparam);
		break;

	// close: close the window if it's not the main console
	case WM_CLOSE:
		if (m_is_main_console)
		{
			debugger().hide_all();
			debug_cpu_get_visible_cpu(machine())->debug()->go();
		}
		else
		{
			destroy();
		}
		break;

	// destroy: close down the window
	case WM_NCDESTROY:
		m_wnd = NULL;
		debugger().remove_window(*this);
		break;

	// everything else: defaults
	default:
		return DefWindowProc(m_wnd, message, wparam, lparam);
	}

	return 0;
}


HMENU debugwin_info::create_standard_menubar()
{
	// create the debug menu
	HMENU const debugmenu = CreatePopupMenu();
	if (debugmenu == NULL)
		return NULL;
	AppendMenu(debugmenu, MF_ENABLED, ID_NEW_MEMORY_WND, TEXT("New Memory Window\tCtrl+M"));
	AppendMenu(debugmenu, MF_ENABLED, ID_NEW_DISASM_WND, TEXT("New Disassembly Window\tCtrl+D"));
	AppendMenu(debugmenu, MF_ENABLED, ID_NEW_LOG_WND, TEXT("New Error Log Window\tCtrl+L"));
	AppendMenu(debugmenu, MF_ENABLED, ID_NEW_POINTS_WND, TEXT("New (Break|Watch)points Window\tCtrl+B"));
	AppendMenu(debugmenu, MF_DISABLED | MF_SEPARATOR, 0, TEXT(""));
	AppendMenu(debugmenu, MF_ENABLED, ID_RUN, TEXT("Run\tF5"));
	AppendMenu(debugmenu, MF_ENABLED, ID_RUN_AND_HIDE, TEXT("Run and Hide Debugger\tF12"));
	AppendMenu(debugmenu, MF_ENABLED, ID_NEXT_CPU, TEXT("Run to Next CPU\tF6"));
	AppendMenu(debugmenu, MF_ENABLED, ID_RUN_IRQ, TEXT("Run until Next Interrupt on This CPU\tF7"));
	AppendMenu(debugmenu, MF_ENABLED, ID_RUN_VBLANK, TEXT("Run until Next VBLANK\tF8"));
	AppendMenu(debugmenu, MF_DISABLED | MF_SEPARATOR, 0, TEXT(""));
	AppendMenu(debugmenu, MF_ENABLED, ID_STEP, TEXT("Step Into\tF11"));
	AppendMenu(debugmenu, MF_ENABLED, ID_STEP_OVER, TEXT("Step Over\tF10"));
	AppendMenu(debugmenu, MF_ENABLED, ID_STEP_OUT, TEXT("Step Out\tShift+F11"));
	AppendMenu(debugmenu, MF_DISABLED | MF_SEPARATOR, 0, TEXT(""));
	AppendMenu(debugmenu, MF_ENABLED, ID_SOFT_RESET, TEXT("Soft Reset\tF3"));
	AppendMenu(debugmenu, MF_ENABLED, ID_HARD_RESET, TEXT("Hard Reset\tShift+F3"));
	AppendMenu(debugmenu, MF_ENABLED, ID_EXIT, TEXT("Exit"));

	// create the menu bar
	HMENU const menubar = CreateMenu();
	if (menubar == NULL)
	{
		DestroyMenu(debugmenu);
		return NULL;
	}
	AppendMenu(menubar, MF_ENABLED | MF_POPUP, (UINT_PTR)debugmenu, TEXT("Debug"));

	return menubar;
}


LRESULT CALLBACK debugwin_info::static_window_proc(HWND wnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	if (message == WM_CREATE)
	{
		// set the info pointer
		CREATESTRUCT const *const createinfo = (CREATESTRUCT *)lparam;
		debugwin_info *const info = (debugwin_info *)createinfo->lpCreateParams;
		SetWindowLongPtr(wnd, GWLP_USERDATA, (LONG_PTR)createinfo->lpCreateParams);
		if (info->m_handler)
			SetWindowLongPtr(wnd, GWLP_WNDPROC, (LONG_PTR)info->m_handler);
		return 0;
	}

	debugwin_info *const info = (debugwin_info *)(FPTR)GetWindowLongPtr(wnd, GWLP_USERDATA);
	if (info == NULL)
		return DefWindowProc(wnd, message, wparam, lparam);

	assert((info->m_wnd == wnd) || (info->m_wnd == NULL));
	return info->window_proc(message, wparam, lparam);
}


void debugwin_info::register_window_class()
{
	if (!s_window_class_registered)
	{
		WNDCLASS wc = { 0 };

		// initialize the description of the window class
		wc.lpszClassName    = TEXT("MAMEDebugWindow");
		wc.hInstance        = GetModuleHandleUni();
		wc.lpfnWndProc      = &debugwin_info::static_window_proc;
		wc.hCursor          = LoadCursor(NULL, IDC_ARROW);
		wc.hIcon            = LoadIcon(wc.hInstance, MAKEINTRESOURCE(2));
		wc.lpszMenuName     = NULL;
		wc.hbrBackground    = NULL;
		wc.style            = 0;
		wc.cbClsExtra       = 0;
		wc.cbWndExtra       = 0;

		UnregisterClass(wc.lpszClassName, wc.hInstance);

		// register the class; fail if we can't
		if (!RegisterClass(&wc))
			fatalerror("Unable to register debug window class\n");

		s_window_class_registered = true;
	}
}
