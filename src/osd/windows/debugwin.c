//============================================================
//
//  debugwin.c - Win32 debug window handling
//
//============================================================
//
//  Copyright Aaron Giles
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

// standard windows headers
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <tchar.h>
#include <commdlg.h>
#ifdef _MSC_VER
#include <zmouse.h>
#endif

// MAME headers
#include "emu.h"
#include "uiinput.h"
#include "imagedev/cassette.h"
#include "debugger.h"
#include "debug/debugvw.h"
#include "debug/dvdisasm.h"
#include "debug/dvmemory.h"
#include "debug/dvstate.h"
#include "debug/debugvw.h"
#include "debug/debugcon.h"
#include "debug/debugcpu.h"

// MAMEOS headers
#include "debugwin.h"
#include "winmain.h"
#include "window.h"
#include "video.h"
#include "input.h"
#include "config.h"
#include "strconv.h"
#include "winutf8.h"



//============================================================
//  PARAMETERS
//============================================================

#define MAX_VIEWS				4
#define EDGE_WIDTH				3
#define MAX_EDIT_STRING			256
#define HISTORY_LENGTH			20
#define MAX_OTHER_WND			4

// debugger window styles
#define DEBUG_WINDOW_STYLE		(WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN) & (~WS_MINIMIZEBOX & ~WS_MAXIMIZEBOX)
#define DEBUG_WINDOW_STYLE_EX	0

// debugger view styles
#define DEBUG_VIEW_STYLE		WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN
#define DEBUG_VIEW_STYLE_EX		0

// edit box styles
#define EDIT_BOX_STYLE			WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL
#define EDIT_BOX_STYLE_EX		0

// combo box styles
#define COMBO_BOX_STYLE			WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL
#define COMBO_BOX_STYLE_EX		0

// horizontal scroll bar styles
#define HSCROLL_STYLE			WS_CHILD | WS_VISIBLE | SBS_HORZ
#define HSCROLL_STYLE_EX		0

// vertical scroll bar styles
#define VSCROLL_STYLE			WS_CHILD | WS_VISIBLE | SBS_VERT
#define VSCROLL_STYLE_EX		0


enum
{
	ID_NEW_MEMORY_WND = 1,
	ID_NEW_DISASM_WND,
	ID_NEW_LOG_WND,
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
	ID_LOGICAL_ADDRESSES,
	ID_PHYSICAL_ADDRESSES,
	ID_REVERSE_VIEW,
	ID_INCREASE_MEM_WIDTH,
	ID_DECREASE_MEM_WIDTH,

	ID_SHOW_RAW,
	ID_SHOW_ENCRYPTED,
	ID_SHOW_COMMENTS,
	ID_RUN_TO_CURSOR,
	ID_TOGGLE_BREAKPOINT,

	ID_DEVICE_OPTIONS  // keep this always at the end
};



//============================================================
//  TYPES
//============================================================

struct debugview_info;
class debugwin_info;


struct debugview_info
{
	debugwin_info *			owner;
	debug_view *			view;
	HWND					wnd;
	HWND					hscroll;
	HWND					vscroll;
};


class debugwin_info
{
public:
	debugwin_info(running_machine &machine)
		: m_machine(machine) { }

	running_machine &machine() const { return m_machine; }

	debugwin_info *			next;
	HWND					wnd;
	HWND					focuswnd;
	WNDPROC					handler;

	UINT32					minwidth, maxwidth;
	UINT32					minheight, maxheight;
	void					(*recompute_children)(debugwin_info *);
	void					(*update_menu)(debugwin_info *);

	int						(*handle_command)(debugwin_info *, WPARAM, LPARAM);
	int						(*handle_key)(debugwin_info *, WPARAM, LPARAM);
	UINT16					ignore_char_lparam;

	debugview_info			view[MAX_VIEWS];

	HWND					editwnd;
	char					edit_defstr[256];
	void					(*process_string)(debugwin_info *, const char *);
	WNDPROC 				original_editproc;
	TCHAR					history[HISTORY_LENGTH][MAX_EDIT_STRING];
	int						history_count;
	int						last_history;

	HWND					otherwnd[MAX_OTHER_WND];

private:
	running_machine &		m_machine;
};


//============================================================
//  LOCAL VARIABLES
//============================================================

static debugwin_info *window_list;
static debugwin_info *main_console;
static UINT32 main_console_regwidth;

static UINT8 waiting_for_debugger;

static HFONT debug_font;
static UINT32 debug_font_height;
static UINT32 debug_font_width;
static UINT32 debug_font_ascent;

static UINT32 hscroll_height;
static UINT32 vscroll_width;



//============================================================
//  PROTOTYPES
//============================================================

static debugwin_info *debugwin_window_create(running_machine &machine, LPCSTR title, WNDPROC handler);
static void debugwin_window_free(debugwin_info *info);
static LRESULT CALLBACK debugwin_window_proc(HWND wnd, UINT message, WPARAM wparam, LPARAM lparam);

static void debugwin_view_draw_contents(debugview_info *view, HDC dc);
static LRESULT CALLBACK debugwin_view_proc(HWND wnd, UINT message, WPARAM wparam, LPARAM lparam);
static void debugwin_view_update(debug_view &view, void *osdprivate);
static int debugwin_view_create(debugwin_info *info, int which, debug_view_type type);

static LRESULT CALLBACK debugwin_edit_proc(HWND wnd, UINT message, WPARAM wparam, LPARAM lparam);

//static void generic_create_window(int type);
static void generic_recompute_children(debugwin_info *info);

static void memory_create_window(running_machine &machine);
static void memory_recompute_children(debugwin_info *info);
static void memory_process_string(debugwin_info *info, const char *string);
static void memory_update_menu(debugwin_info *info);
static int memory_handle_command(debugwin_info *info, WPARAM wparam, LPARAM lparam);
static int memory_handle_key(debugwin_info *info, WPARAM wparam, LPARAM lparam);
static void memory_update_caption(running_machine &machine, HWND wnd);

static void disasm_create_window(running_machine &machine);
static void disasm_recompute_children(debugwin_info *info);
static void disasm_process_string(debugwin_info *info, const char *string);
static void disasm_update_menu(debugwin_info *info);
static int disasm_handle_command(debugwin_info *info, WPARAM wparam, LPARAM lparam);
static int disasm_handle_key(debugwin_info *info, WPARAM wparam, LPARAM lparam);
static void disasm_update_caption(running_machine &machine, HWND wnd);

static void console_create_window(running_machine &machine);
static void console_recompute_children(debugwin_info *info);
static void console_process_string(debugwin_info *info, const char *string);
static void console_set_cpu(device_t *device);

static HMENU create_standard_menubar(void);
static int global_handle_command(debugwin_info *info, WPARAM wparam, LPARAM lparam);
static int global_handle_key(debugwin_info *info, WPARAM wparam, LPARAM lparam);
static void smart_set_window_bounds(HWND wnd, HWND parent, RECT *bounds);
static void smart_show_window(HWND wnd, BOOL show);
static void smart_show_all(BOOL show);

static void image_update_menu(debugwin_info *info);

//============================================================
//  wait_for_debugger
//============================================================

void windows_osd_interface::wait_for_debugger(device_t &device, bool firststop)
{
	MSG message;

	// create a console window
	if (main_console == NULL)
		console_create_window(machine());

	// update the views in the console to reflect the current CPU
	if (main_console != NULL)
		console_set_cpu(&device);

	// when we are first stopped, adjust focus to us
	if (firststop && main_console != NULL)
	{
		SetForegroundWindow(main_console->wnd);
		if (winwindow_has_focus())
			SetFocus(main_console->editwnd);
	}

	// make sure the debug windows are visible
	waiting_for_debugger = TRUE;
	smart_show_all(TRUE);

	// run input polling to ensure that our status is in sync
	wininput_poll(machine());

	// get and process messages
	GetMessage(&message, NULL, 0, 0);

	switch (message.message)
	{
		// check for F10 -- we need to capture that ourselves
		case WM_SYSKEYDOWN:
		case WM_SYSKEYUP:
			if (message.wParam == VK_F4 && message.message == WM_SYSKEYDOWN)
				SendMessage(GetAncestor(GetFocus(), GA_ROOT), WM_CLOSE, 0, 0);
			if (message.wParam == VK_F10)
				SendMessage(GetAncestor(GetFocus(), GA_ROOT), (message.message == WM_SYSKEYDOWN) ? WM_KEYDOWN : WM_KEYUP, message.wParam, message.lParam);
			break;

		// process everything else
		default:
			winwindow_dispatch_message(machine(), &message);
			break;
	}

	// mark the debugger as active
	waiting_for_debugger = FALSE;
}



//============================================================
//  debugwin_seq_pressed
//============================================================

static int debugwin_seq_pressed(running_machine &machine)
{
	const input_seq &seq = machine.ioport().type_seq(IPT_UI_DEBUG_BREAK);
	int result = FALSE;
	int invert = FALSE;
	int first = TRUE;
	int codenum;

	// iterate over all of the codes
	int length = seq.length();
	for (codenum = 0; codenum < length; codenum++)
	{
		input_code code = seq[codenum];

		// handle NOT
		if (code == input_seq::not_code)
			invert = TRUE;

		// handle OR and END
		else if (code == input_seq::or_code || code == input_seq::end_code)
		{
			// if we have a positive result from the previous set, we're done
			if (result || code == input_seq::end_code)
				break;

			// otherwise, reset our state
			result = FALSE;
			invert = FALSE;
			first = TRUE;
		}

		// handle everything else as a series of ANDs
		else
		{
			int vkey = wininput_vkey_for_mame_code(code);
			int pressed = (vkey != 0 && (GetAsyncKeyState(vkey) & 0x8000) != 0);

			// if this is the first in the sequence, result is set equal
			if (first)
				result = pressed ^ invert;

			// further values are ANDed
			else if (result)
				result &= pressed ^ invert;

			// no longer first, and clear the invert flag
			first = invert = FALSE;
		}
	}

	// return the result if we queried at least one switch
	return result;
}



//============================================================
//  debugwin_init_windows
//============================================================

void debugwin_init_windows(running_machine &machine)
{
	static int class_registered;

	// register the window classes
	if (!class_registered)
	{
		WNDCLASS wc = { 0 };

		// initialize the description of the window class
		wc.lpszClassName	= TEXT("MAMEDebugWindow");
		wc.hInstance		= GetModuleHandle(NULL);
		wc.lpfnWndProc		= debugwin_window_proc;
		wc.hCursor			= LoadCursor(NULL, IDC_ARROW);
		wc.hIcon			= LoadIcon(wc.hInstance, MAKEINTRESOURCE(2));
		wc.lpszMenuName		= NULL;
		wc.hbrBackground	= NULL;
		wc.style			= 0;
		wc.cbClsExtra		= 0;
		wc.cbWndExtra		= 0;

		// register the class; fail if we can't
		if (!RegisterClass(&wc))
			fatalerror("Unable to register debug window class\n");

		// initialize the description of the view class
		wc.lpszClassName	= TEXT("MAMEDebugView");
		wc.lpfnWndProc		= debugwin_view_proc;

		// register the class; fail if we can't
		if (!RegisterClass(&wc))
			fatalerror("Unable to register debug view class\n");

		class_registered = TRUE;
	}

	// create the font
	if (debug_font == NULL)
	{
		// create a temporary DC
		HDC temp_dc = GetDC(NULL);
		TEXTMETRIC metrics;
		HGDIOBJ old_font;

		if (temp_dc != NULL)
		{
			windows_options &options = downcast<windows_options &>(machine.options());
			int size = options.debugger_font_size();
			TCHAR *t_face;

			// create a standard font
			t_face = tstring_from_utf8(options.debugger_font());
			debug_font = CreateFont(-MulDiv(size, GetDeviceCaps(temp_dc, LOGPIXELSY), 72), 0, 0, 0, FW_MEDIUM, FALSE, FALSE, FALSE,
						ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH, t_face);
			osd_free(t_face);
			t_face = NULL;

			if (debug_font == NULL)
				fatalerror("Unable to create debugger font\n");

			// get the metrics
			old_font = SelectObject(temp_dc, debug_font);
			if (GetTextMetrics(temp_dc, &metrics))
			{
				debug_font_width = metrics.tmAveCharWidth;
				debug_font_height = metrics.tmHeight;
				debug_font_ascent = metrics.tmAscent + metrics.tmExternalLeading;
			}
			SelectObject(temp_dc, old_font);
			ReleaseDC(NULL, temp_dc);
		}
	}

	// get other metrics
	hscroll_height = GetSystemMetrics(SM_CYHSCROLL);
	vscroll_width = GetSystemMetrics(SM_CXVSCROLL);
}



//============================================================
//  debugwin_destroy_windows
//============================================================

void debugwin_destroy_windows(void)
{
	// loop over windows and free them
	while (window_list != NULL)
	{
		// clear the view list because they will be freed by the core
		memset(window_list->view, 0, sizeof(window_list->view));
		DestroyWindow(window_list->wnd);
	}

	main_console = NULL;
}



//============================================================
//  debugwin_show
//============================================================

void debugwin_show(int type)
{
	debugwin_info *info;

	// loop over windows and show/hide them
	for (info = window_list; info != NULL; info = info->next)
		ShowWindow(info->wnd, type);
}



//============================================================
//  debugwin_update_during_game
//============================================================

void debugwin_update_during_game(running_machine &machine)
{
	// if we're running live, do some checks
	if (!winwindow_has_focus() && !debug_cpu_is_stopped(machine) && machine.phase() == MACHINE_PHASE_RUNNING)
	{
		// see if the interrupt key is pressed and break if it is
		if (debugwin_seq_pressed(machine))
		{
			HWND focuswnd = GetFocus();
			debugwin_info *info;

			debug_cpu_get_visible_cpu(machine)->debug()->halt_on_next_instruction("User-initiated break\n");

			// if we were focused on some window's edit box, reset it to default
			for (info = window_list; info != NULL; info = info->next)
				if (focuswnd == info->editwnd)
				{
					SendMessage(focuswnd, WM_SETTEXT, (WPARAM)0, (LPARAM)info->edit_defstr);
					SendMessage(focuswnd, EM_SETSEL, (WPARAM)0, (LPARAM)-1);
				}
		}
	}
}



//============================================================
//  debugwin_window_create
//============================================================

static debugwin_info *debugwin_window_create(running_machine &machine, LPCSTR title, WNDPROC handler)
{
	debugwin_info *info = NULL;
	RECT work_bounds;

	// allocate memory
	info = global_alloc_clear(debugwin_info(machine));

	// create the window
	info->handler = handler;
	info->wnd = win_create_window_ex_utf8(DEBUG_WINDOW_STYLE_EX, "MAMEDebugWindow", title, DEBUG_WINDOW_STYLE,
			0, 0, 100, 100, win_window_list->hwnd, create_standard_menubar(), GetModuleHandle(NULL), info);
	if (info->wnd == NULL)
		goto cleanup;

	// fill in some defaults
	SystemParametersInfo(SPI_GETWORKAREA, 0, &work_bounds, 0);
	info->minwidth = 200;
	info->minheight = 200;
	info->maxwidth = work_bounds.right - work_bounds.left;
	info->maxheight = work_bounds.bottom - work_bounds.top;

	// set the default handlers
	info->handle_command = global_handle_command;
	info->handle_key = global_handle_key;
	strcpy(info->edit_defstr, "");

	// hook us in
	info->next = window_list;
	window_list = info;

	return info;

cleanup:
	if (info->wnd != NULL)
		DestroyWindow(info->wnd);
	global_free(info);
	return NULL;
}



//============================================================
//  debugwin_window_free
//============================================================

static void debugwin_window_free(debugwin_info *info)
{
	debugwin_info **scanptr;
	int viewnum;

	// first unlink us from the list
	for (scanptr = &window_list; *scanptr != NULL; scanptr = &(*scanptr)->next)
		if (*scanptr == info)
		{
			*scanptr = info->next;
			break;
		}

	// free any views
	for (viewnum = 0; viewnum < ARRAY_LENGTH(info->view); viewnum++)
		if (info->view[viewnum].view != NULL)
		{
			info->machine().debug_view().free_view(*info->view[viewnum].view);
			info->view[viewnum].view = NULL;
		}

	// free our memory
	global_free(info);
}



//============================================================
//  debugwin_window_draw_contents
//============================================================

static void debugwin_window_draw_contents(debugwin_info *info, HDC dc)
{
	RECT bounds, parent;
	int curview, curwnd;

	// fill the background with light gray
	GetClientRect(info->wnd, &parent);
	FillRect(dc, &parent, (HBRUSH)GetStockObject(LTGRAY_BRUSH));

	// get the parent bounds in screen coords
	ClientToScreen(info->wnd, &((POINT *)&parent)[0]);
	ClientToScreen(info->wnd, &((POINT *)&parent)[1]);

	// draw edges around all views
	for (curview = 0; curview < MAX_VIEWS; curview++)
		if (info->view[curview].wnd)
		{
			GetWindowRect(info->view[curview].wnd, &bounds);
			bounds.top -= parent.top;
			bounds.bottom -= parent.top;
			bounds.left -= parent.left;
			bounds.right -= parent.left;
			InflateRect(&bounds, EDGE_WIDTH, EDGE_WIDTH);
			DrawEdge(dc, &bounds, EDGE_SUNKEN, BF_RECT);
		}

	// draw edges around all children
	if (info->editwnd)
	{
		GetWindowRect(info->editwnd, &bounds);
		bounds.top -= parent.top;
		bounds.bottom -= parent.top;
		bounds.left -= parent.left;
		bounds.right -= parent.left;
		InflateRect(&bounds, EDGE_WIDTH, EDGE_WIDTH);
		DrawEdge(dc, &bounds, EDGE_SUNKEN, BF_RECT);
	}

	for (curwnd = 0; curwnd < MAX_OTHER_WND; curwnd++)
		if (info->otherwnd[curwnd])
		{
			GetWindowRect(info->otherwnd[curwnd], &bounds);
			bounds.top -= parent.top;
			bounds.bottom -= parent.top;
			bounds.left -= parent.left;
			bounds.right -= parent.left;
			InflateRect(&bounds, EDGE_WIDTH, EDGE_WIDTH);
			DrawEdge(dc, &bounds, EDGE_SUNKEN, BF_RECT);
		}
}



//============================================================
//  debugwin_window_proc
//============================================================

static LRESULT CALLBACK debugwin_window_proc(HWND wnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	debugwin_info *info = (debugwin_info *)(FPTR)GetWindowLongPtr(wnd, GWLP_USERDATA);

	// handle a few messages
	switch (message)
	{
		// set the info pointer
		case WM_CREATE:
		{
			CREATESTRUCT *createinfo = (CREATESTRUCT *)lparam;
			info = (debugwin_info *)createinfo->lpCreateParams;
			SetWindowLongPtr(wnd, GWLP_USERDATA, (LONG_PTR)createinfo->lpCreateParams);
			if (info->handler)
				SetWindowLongPtr(wnd, GWLP_WNDPROC, (LONG_PTR)info->handler);
			break;
		}

		// paint: draw bezels as necessary
		case WM_PAINT:
		{
			PAINTSTRUCT pstruct;
			HDC dc = BeginPaint(wnd, &pstruct);
			debugwin_window_draw_contents(info, dc);
			EndPaint(wnd, &pstruct);
			break;
		}

		// keydown: handle debugger keys
		case WM_KEYDOWN:
			if ((*info->handle_key)(info, wparam, lparam))
				info->ignore_char_lparam = lparam >> 16;
			break;

		// char: ignore chars associated with keys we've handled
		case WM_CHAR:
			if (info->ignore_char_lparam == (lparam >> 16))
				info->ignore_char_lparam = 0;
			else if (waiting_for_debugger || !debugwin_seq_pressed(info->machine()))
				return DefWindowProc(wnd, message, wparam, lparam);
			break;

		// activate: set the focus
		case WM_ACTIVATE:
			if (wparam != WA_INACTIVE && info->focuswnd != NULL)
				SetFocus(info->focuswnd);
			break;

		// get min/max info: set the minimum window size
		case WM_GETMINMAXINFO:
		{
			MINMAXINFO *minmax = (MINMAXINFO *)lparam;
			if (info)
			{
				minmax->ptMinTrackSize.x = info->minwidth;
				minmax->ptMinTrackSize.y = info->minheight;
				minmax->ptMaxSize.x = minmax->ptMaxTrackSize.x = info->maxwidth;
				minmax->ptMaxSize.y = minmax->ptMaxTrackSize.y = info->maxheight;
			}
			break;
		}

		// sizing: recompute child window locations
		case WM_SIZE:
		case WM_SIZING:
			if (info->recompute_children != NULL)
				(*info->recompute_children)(info);
			InvalidateRect(wnd, NULL, FALSE);
			break;

		// mouse wheel: forward to the first view
		case WM_MOUSEWHEEL:
		{
			static int units_carryover = 0;

			UINT lines_per_click;
			if (!SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &lines_per_click, 0))
				lines_per_click = 3;

			int units = GET_WHEEL_DELTA_WPARAM(wparam) + units_carryover;
			int clicks = units / WHEEL_DELTA;
			units_carryover = units % WHEEL_DELTA;

			int delta = clicks * lines_per_click;
			int viewnum = 0;
			POINT point;
			HWND child;

			// figure out which view we are hovering over
			GetCursorPos(&point);
			ScreenToClient(info->wnd, &point);
			child = ChildWindowFromPoint(info->wnd, point);
			if (child)
			{
				for (viewnum = 0; viewnum < MAX_VIEWS; viewnum++)
					if (info->view[viewnum].wnd == child)
						break;
				if (viewnum == MAX_VIEWS)
					break;
			}

			// send the appropriate message to this view's scrollbar
			if (info->view[viewnum].wnd && info->view[viewnum].vscroll)
			{
				int message_type = SB_LINEUP;
				if (delta < 0)
				{
					message_type = SB_LINEDOWN;
					delta = -delta;
				}
				while (delta > 0)
				{
					SendMessage(info->view[viewnum].wnd, WM_VSCROLL, message_type, (LPARAM)info->view[viewnum].vscroll);
					delta--;
				}
			}
			break;
		}

		// activate: set the focus
		case WM_INITMENU:
			if (info->update_menu != NULL)
				(*info->update_menu)(info);
			break;

		// command: handle a comment
		case WM_COMMAND:
			if (!(*info->handle_command)(info, wparam, lparam))
				return DefWindowProc(wnd, message, wparam, lparam);
			break;

		// close: close the window if it's not the main console
		case WM_CLOSE:
			if (main_console && main_console->wnd == wnd)
			{
				smart_show_all(FALSE);
				debug_cpu_get_visible_cpu(info->machine())->debug()->go();
			}
			else
				DestroyWindow(wnd);
			break;

		// destroy: close down the window
		case WM_NCDESTROY:
			debugwin_window_free(info);
			break;

		// everything else: defaults
		default:
			return DefWindowProc(wnd, message, wparam, lparam);
	}

	return 0;
}



//============================================================
//  debugwin_view_create
//============================================================

static int debugwin_view_create(debugwin_info *info, int which, debug_view_type type)
{
	debugview_info *view = &info->view[which];

	// set the owner
	view->owner = info;

	// create the child view
	view->wnd = CreateWindowEx(DEBUG_VIEW_STYLE_EX, TEXT("MAMEDebugView"), NULL, DEBUG_VIEW_STYLE,
			0, 0, 100, 100, info->wnd, NULL, GetModuleHandle(NULL), view);
	if (view->wnd == NULL)
		goto cleanup;

	// create the scroll bars
	view->hscroll = CreateWindowEx(HSCROLL_STYLE_EX, TEXT("SCROLLBAR"), NULL, HSCROLL_STYLE,
			0, 0, 100, CW_USEDEFAULT, view->wnd, NULL, GetModuleHandle(NULL), view);
	view->vscroll = CreateWindowEx(VSCROLL_STYLE_EX, TEXT("SCROLLBAR"), NULL, VSCROLL_STYLE,
			0, 0, CW_USEDEFAULT, 100, view->wnd, NULL, GetModuleHandle(NULL), view);
	if (view->hscroll == NULL || view->vscroll == NULL)
		goto cleanup;

	// create the debug view
	view->view = info->machine().debug_view().alloc_view(type, debugwin_view_update, view);
	if (view->view == NULL)
		goto cleanup;

	return 1;

cleanup:
	if (view->view)
		info->machine().debug_view().free_view(*view->view);
	if (view->hscroll)
		DestroyWindow(view->hscroll);
	if (view->vscroll)
		DestroyWindow(view->vscroll);
	if (view->wnd)
		DestroyWindow(view->wnd);
	return 0;
}



//============================================================
//  debugwin_view_set_bounds
//============================================================

static void debugwin_view_set_bounds(debugview_info *info, HWND parent, const RECT *newbounds)
{
	RECT bounds = *newbounds;

	// account for the edges and set the bounds
	if (info->wnd)
		smart_set_window_bounds(info->wnd, parent, &bounds);

	// update
	debugwin_view_update(*info->view, info);
}



//============================================================
//  debugwin_view_draw_contents
//============================================================

static void debugwin_view_draw_contents(debugview_info *view, HDC windc)
{
	const debug_view_char *viewdata = view->view->viewdata();
	debug_view_xy visarea = view->view->visible_size();
	HGDIOBJ oldfont, oldbitmap;
	COLORREF oldfgcolor;
	UINT32 col, row;
	HBITMAP bitmap;
	int oldbkmode;
	RECT client;
	HDC dc;

	// get the client rect
	GetClientRect(view->wnd, &client);

	// create a compatible DC and an offscreen bitmap
	dc = CreateCompatibleDC(windc);
	if (dc == NULL)
		return;
	bitmap = CreateCompatibleBitmap(windc, client.right, client.bottom);
	if (bitmap == NULL)
	{
		DeleteDC(dc);
		return;
	}
	oldbitmap = SelectObject(dc, bitmap);

	// set the font
	oldfont = SelectObject(dc, debug_font);
	oldfgcolor = GetTextColor(dc);
	oldbkmode = GetBkMode(dc);
	SetBkMode(dc, TRANSPARENT);

	// iterate over rows and columns
	for (row = 0; row < visarea.y; row++)
	{
		int iter;

		// loop twice; once to fill the background and once to draw the text
		for (iter = 0; iter < 2; iter++)
		{
			COLORREF fgcolor = RGB(0x00,0x00,0x00);
			COLORREF bgcolor = RGB(0xff,0xff,0xff);
			HBRUSH bgbrush = NULL;
			int last_attrib = -1;
			TCHAR buffer[256];
			int count = 0;
			RECT bounds;

			// initialize the text bounds
			bounds.left = bounds.right = 0;
			bounds.top = row * debug_font_height;
			bounds.bottom = bounds.top + debug_font_height;

			// start with a brush on iteration #0
			if (iter == 0)
				bgbrush = CreateSolidBrush(bgcolor);

			// iterate over columns
			for (col = 0; col < visarea.x; col++)
			{
				// if the attribute changed, adjust the colors
				if (viewdata[col].attrib != last_attrib)
				{
					COLORREF oldbg = bgcolor;

					// reset to standard colors
					fgcolor = RGB(0x00,0x00,0x00);
					bgcolor = RGB(0xff,0xff,0xff);

					// pick new fg/bg colors
					if (viewdata[col].attrib & DCA_ANCILLARY) bgcolor = RGB(0xe0,0xe0,0xe0);
					if (viewdata[col].attrib & DCA_SELECTED) bgcolor = RGB(0xff,0x80,0x80);
					if (viewdata[col].attrib & DCA_CURRENT) bgcolor = RGB(0xff,0xff,0x00);
					if ((viewdata[col].attrib & DCA_SELECTED) && (viewdata[col].attrib & DCA_CURRENT)) bgcolor = RGB(0xff,0xc0,0x80);
					if (viewdata[col].attrib & DCA_CHANGED) fgcolor = RGB(0xff,0x00,0x00);
					if (viewdata[col].attrib & DCA_INVALID) fgcolor = RGB(0x00,0x00,0xff);
					if (viewdata[col].attrib & DCA_DISABLED) fgcolor = RGB((GetRValue(fgcolor) + GetRValue(bgcolor)) / 2, (GetGValue(fgcolor) + GetGValue(bgcolor)) / 2, (GetBValue(fgcolor) + GetBValue(bgcolor)) / 2);
					if (viewdata[col].attrib & DCA_COMMENT) fgcolor = RGB(0x00,0x80,0x00);

					// flush any pending drawing
					if (count > 0)
					{
						bounds.right = bounds.left + count * debug_font_width;
						if (iter == 0)
							FillRect(dc, &bounds, bgbrush);
						else
							ExtTextOut(dc, bounds.left, bounds.top, 0, NULL, buffer, count, NULL);
						bounds.left = bounds.right;
						count = 0;
					}

					// set the new colors
					if (iter == 0 && oldbg != bgcolor)
					{
						DeleteObject(bgbrush);
						bgbrush = CreateSolidBrush(bgcolor);
					}
					else if (iter == 1)
						SetTextColor(dc, fgcolor);
					last_attrib = viewdata[col].attrib;
				}

				// add this character to the buffer
				buffer[count++] = viewdata[col].byte;
			}

			// flush any remaining stuff
			if (count > 0)
			{
				bounds.right = bounds.left + count * debug_font_width;
				if (iter == 0)
					FillRect(dc, &bounds, bgbrush);
				else
					ExtTextOut(dc, bounds.left, bounds.top, 0, NULL, buffer, count, NULL);
			}

			// erase to the end of the line
			if (iter == 0)
			{
				bounds.left = bounds.right;
				bounds.right = client.right;
				FillRect(dc, &bounds, bgbrush);
				DeleteObject(bgbrush);
			}
		}

		// advance viewdata
		viewdata += visarea.x;
	}

	// erase anything beyond the bottom with white
	GetClientRect(view->wnd, &client);
	client.top = visarea.y * debug_font_height;
	FillRect(dc, &client, (HBRUSH)GetStockObject(WHITE_BRUSH));

	// reset the font
	SetBkMode(dc, oldbkmode);
	SetTextColor(dc, oldfgcolor);
	SelectObject(dc, oldfont);

	// blit the final results
	BitBlt(windc, 0, 0, client.right, client.bottom, dc, 0, 0, SRCCOPY);

	// undo the offscreen stuff
	SelectObject(dc, oldbitmap);
	DeleteObject(bitmap);
	DeleteDC(dc);
}



//============================================================
//  debugwin_view_update
//============================================================

static void debugwin_view_update(debug_view &view, void *osdprivate)
{
	debugview_info *info = (debugview_info *)osdprivate;
	RECT bounds, vscroll_bounds, hscroll_bounds;
	debug_view_xy totalsize, visiblesize, topleft;
	int show_vscroll, show_hscroll;
	SCROLLINFO scrollinfo;

	assert(info->view == &view);

	// get the view window bounds
	GetClientRect(info->wnd, &bounds);
	visiblesize.x = (bounds.right - bounds.left) / debug_font_width;
	visiblesize.y = (bounds.bottom - bounds.top) / debug_font_height;

	// get the updated total rows/cols and left row/col
	totalsize = view.total_size();
	topleft = view.visible_position();

	// determine if we need to show the scrollbars
	show_vscroll = show_hscroll = FALSE;
	if (totalsize.x > visiblesize.x && bounds.bottom >= hscroll_height)
	{
		bounds.bottom -= hscroll_height;
		visiblesize.y = (bounds.bottom - bounds.top) / debug_font_height;
		show_hscroll = TRUE;
	}
	if (totalsize.y > visiblesize.y && bounds.right >= vscroll_width)
	{
		bounds.right -= vscroll_width;
		visiblesize.x = (bounds.right - bounds.left) / debug_font_width;
		show_vscroll = TRUE;
	}
	if (!show_vscroll && totalsize.y > visiblesize.y && bounds.right >= vscroll_width)
	{
		bounds.right -= vscroll_width;
		visiblesize.x = (bounds.right - bounds.left) / debug_font_width;
		show_vscroll = TRUE;
	}

	// compute the bounds of the scrollbars
	GetClientRect(info->wnd, &vscroll_bounds);
	vscroll_bounds.left = vscroll_bounds.right - vscroll_width;
	if (show_hscroll)
		vscroll_bounds.bottom -= hscroll_height;

	GetClientRect(info->wnd, &hscroll_bounds);
	hscroll_bounds.top = hscroll_bounds.bottom - hscroll_height;
	if (show_vscroll)
		hscroll_bounds.right -= vscroll_width;

	// if we hid the scrollbars, make sure we reset the top/left corners
	if (topleft.y + visiblesize.y > totalsize.y)
		topleft.y = MAX(totalsize.y - visiblesize.y, 0);
	if (topleft.x + visiblesize.x > totalsize.x)
		topleft.x = MAX(totalsize.x - visiblesize.x, 0);

	// fill out the scroll info struct for the vertical scrollbar
	scrollinfo.cbSize = sizeof(scrollinfo);
	scrollinfo.fMask = SIF_PAGE | SIF_POS | SIF_RANGE;
	scrollinfo.nMin = 0;
	scrollinfo.nMax = totalsize.y - 1;
	scrollinfo.nPage = visiblesize.y;
	scrollinfo.nPos = topleft.y;
	SetScrollInfo(info->vscroll, SB_CTL, &scrollinfo, TRUE);

	// fill out the scroll info struct for the horizontal scrollbar
	scrollinfo.cbSize = sizeof(scrollinfo);
	scrollinfo.fMask = SIF_PAGE | SIF_POS | SIF_RANGE;
	scrollinfo.nMin = 0;
	scrollinfo.nMax = totalsize.x - 1;
	scrollinfo.nPage = visiblesize.x;
	scrollinfo.nPos = topleft.x;
	SetScrollInfo(info->hscroll, SB_CTL, &scrollinfo, TRUE);

	// update window info
	visiblesize.y++;
	visiblesize.x++;
	view.set_visible_size(visiblesize);
	view.set_visible_position(topleft);

	// invalidate the bounds
	InvalidateRect(info->wnd, NULL, FALSE);

	// adjust the bounds of the scrollbars and show/hide them
	if (info->vscroll)
	{
		if (show_vscroll)
			smart_set_window_bounds(info->vscroll, info->wnd, &vscroll_bounds);
		smart_show_window(info->vscroll, show_vscroll);
	}
	if (info->hscroll)
	{
		if (show_hscroll)
			smart_set_window_bounds(info->hscroll, info->wnd, &hscroll_bounds);
		smart_show_window(info->hscroll, show_hscroll);
	}
}



//============================================================
//  debugwin_view_process_scroll
//============================================================

static UINT32 debugwin_view_process_scroll(debugview_info *info, WORD type, HWND wnd)
{
	SCROLLINFO scrollinfo;
	INT32 maxval;
	INT32 result;

	// get the current info
	scrollinfo.cbSize = sizeof(scrollinfo);
	scrollinfo.fMask = SIF_PAGE | SIF_POS | SIF_RANGE | SIF_TRACKPOS;
	GetScrollInfo(wnd, SB_CTL, &scrollinfo);

	// by default we stay put
	result = scrollinfo.nPos;

	// determine the maximum value
	maxval = (scrollinfo.nMax > scrollinfo.nPage) ? (scrollinfo.nMax - scrollinfo.nPage + 1) : 0;

	// handle the message
	switch (type)
	{
		case SB_THUMBTRACK:
			result = scrollinfo.nTrackPos;
			break;

		case SB_LEFT:
			result = 0;
			break;

		case SB_RIGHT:
			result = maxval;
			break;

		case SB_LINELEFT:
			result -= 1;
			break;

		case SB_LINERIGHT:
			result += 1;
			break;

		case SB_PAGELEFT:
			result -= scrollinfo.nPage - 1;
			break;

		case SB_PAGERIGHT:
			result += scrollinfo.nPage - 1;
			break;
	}

	// generic rangecheck
	if (result < 0)
		result = 0;
	if (result > maxval)
		result = maxval;

	// set the new position
	scrollinfo.fMask = SIF_POS;
	scrollinfo.nPos = result;
	SetScrollInfo(wnd, SB_CTL, &scrollinfo, TRUE);

	return (UINT32)result;
}



//============================================================
//  debugwin_view_prev_view
//============================================================

static void debugwin_view_prev_view(debugwin_info *info, debugview_info *curview)
{
	int curindex = 1;
	int numviews;

	// count the number of views
	for (numviews = 0; numviews < MAX_VIEWS; numviews++)
		if (info->view[numviews].wnd == NULL)
			break;

	// if we have a curview, find out its index
	if (curview)
		curindex = curview - &info->view[0];

	// loop until we find someone to take focus
	while (1)
	{
		// advance to the previous index
		curindex--;
		if (curindex < -1)
			curindex = numviews - 1;

		// negative numbers mean the focuswnd
		if (curindex < 0 && info->focuswnd != NULL)
		{
			SetFocus(info->focuswnd);
			break;
		}

		// positive numbers mean a view
		else if (curindex >= 0 && info->view[curindex].wnd != NULL && info->view[curindex].view->cursor_supported())
		{
			SetFocus(info->view[curindex].wnd);
			break;
		}
	}
}



//============================================================
//  debugwin_view_next_view
//============================================================

static void debugwin_view_next_view(debugwin_info *info, debugview_info *curview)
{
	int curindex = -1;
	int numviews;

	// count the number of views
	for (numviews = 0; numviews < MAX_VIEWS; numviews++)
		if (info->view[numviews].wnd == NULL)
			break;

	// if we have a curview, find out its index
	if (curview)
		curindex = curview - &info->view[0];

	// loop until we find someone to take focus
	while (1)
	{
		// advance to the previous index
		curindex++;
		if (curindex >= numviews)
			curindex = -1;

		// negative numbers mean the focuswnd
		if (curindex < 0 && info->focuswnd != NULL)
		{
			SetFocus(info->focuswnd);
			break;
		}

		// positive numbers mean a view
		else if (curindex >= 0 && info->view[curindex].wnd != NULL && info->view[curindex].view->cursor_supported())
		{
			SetFocus(info->view[curindex].wnd);
			InvalidateRect(info->view[curindex].wnd, NULL, FALSE);
			break;
		}
	}
}



//============================================================
//  debugwin_view_proc
//============================================================

static LRESULT CALLBACK debugwin_view_proc(HWND wnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	debugview_info *info = (debugview_info *)(FPTR)GetWindowLongPtr(wnd, GWLP_USERDATA);

	// handle a few messages
	switch (message)
	{
		// set the info pointer
		case WM_CREATE:
		{
			CREATESTRUCT *createinfo = (CREATESTRUCT *)lparam;
			SetWindowLongPtr(wnd, GWLP_USERDATA, (LONG_PTR)createinfo->lpCreateParams);
			break;
		}

		// paint: redraw the last bitmap
		case WM_PAINT:
		{
			PAINTSTRUCT pstruct;
			HDC dc = BeginPaint(wnd, &pstruct);
			debugwin_view_draw_contents(info, dc);
			EndPaint(wnd, &pstruct);
			break;
		}

		// keydown: handle debugger keys
		case WM_SYSKEYDOWN:
			if (wparam != VK_F10)
				return DefWindowProc(wnd, message, wparam, lparam);
			// (fall through)
		case WM_KEYDOWN:
		{
			if ((*info->owner->handle_key)(info->owner, wparam, lparam))
				info->owner->ignore_char_lparam = lparam >> 16;
			else
			{
				switch (wparam)
				{
					case VK_UP:
						info->view->process_char(DCH_UP);
						info->owner->ignore_char_lparam = lparam >> 16;
						break;

					case VK_DOWN:
						info->view->process_char(DCH_DOWN);
						info->owner->ignore_char_lparam = lparam >> 16;
						break;

					case VK_LEFT:
						if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
							info->view->process_char(DCH_CTRLLEFT);
						else
							info->view->process_char(DCH_LEFT);
						info->owner->ignore_char_lparam = lparam >> 16;
						break;

					case VK_RIGHT:
						if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
							info->view->process_char(DCH_CTRLRIGHT);
						else
							info->view->process_char(DCH_RIGHT);
						info->owner->ignore_char_lparam = lparam >> 16;
						break;

					case VK_PRIOR:
						info->view->process_char(DCH_PUP);
						info->owner->ignore_char_lparam = lparam >> 16;
						break;

					case VK_NEXT:
						info->view->process_char(DCH_PDOWN);
						info->owner->ignore_char_lparam = lparam >> 16;
						break;

					case VK_HOME:
						if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
							info->view->process_char(DCH_CTRLHOME);
						else
							info->view->process_char(DCH_HOME);
						info->owner->ignore_char_lparam = lparam >> 16;
						break;

					case VK_END:
						if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
							info->view->process_char(DCH_CTRLEND);
						else
							info->view->process_char(DCH_END);
						info->owner->ignore_char_lparam = lparam >> 16;
						break;

					case VK_ESCAPE:
						if (info->owner->focuswnd != NULL)
							SetFocus(info->owner->focuswnd);
						info->owner->ignore_char_lparam = lparam >> 16;
						break;

					case VK_TAB:
						if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
							debugwin_view_prev_view(info->owner, info);
						else
							debugwin_view_next_view(info->owner, info);
						info->owner->ignore_char_lparam = lparam >> 16;
						break;
				}
			}
			break;
		}

		// char: ignore chars associated with keys we've handled
		case WM_CHAR:
		{
			if (info->owner->ignore_char_lparam == (lparam >> 16))
				info->owner->ignore_char_lparam = 0;
			else if (waiting_for_debugger || !debugwin_seq_pressed(info->owner->machine()))
			{
				if (wparam >= 32 && wparam < 127 && info->view->cursor_supported())
					info->view->process_char(wparam);
				else
					return DefWindowProc(wnd, message, wparam, lparam);
			}
			break;
		}

		// gaining focus
		case WM_SETFOCUS:
		{
			if (info->view->cursor_supported())
				info->view->set_cursor_visible(true);
			break;
		}

		// losing focus
		case WM_KILLFOCUS:
		{
			if (info->view->cursor_supported())
				info->view->set_cursor_visible(false);
			break;
		}

		// mouse click
		case WM_LBUTTONDOWN:
		{
			if (info->view->cursor_supported())
			{
				debug_view_xy topleft = info->view->visible_position();
				debug_view_xy newpos;
				newpos.x = topleft.x + GET_X_LPARAM(lparam) / debug_font_width;
				newpos.y = topleft.y + GET_Y_LPARAM(lparam) / debug_font_height;
				info->view->set_cursor_position(newpos);
				SetFocus(wnd);
			}
			break;
		}

		// hscroll
		case WM_HSCROLL:
		{
			debug_view_xy topleft = info->view->visible_position();
			topleft.x = debugwin_view_process_scroll(info, LOWORD(wparam), (HWND)lparam);
			info->view->set_visible_position(topleft);
			info->owner->machine().debug_view().flush_osd_updates();
			break;
		}

		// vscroll
		case WM_VSCROLL:
		{
			debug_view_xy topleft = info->view->visible_position();
			topleft.y = debugwin_view_process_scroll(info, LOWORD(wparam), (HWND)lparam);
			info->view->set_visible_position(topleft);
			info->owner->machine().debug_view().flush_osd_updates();
			break;
		}

		// everything else: defaults
		default:
			return DefWindowProc(wnd, message, wparam, lparam);
	}

	return 0;
}



//============================================================
//  debugwin_edit_proc
//============================================================

static LRESULT CALLBACK debugwin_edit_proc(HWND wnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	debugwin_info *info = (debugwin_info *)(FPTR)GetWindowLongPtr(wnd, GWLP_USERDATA);
	TCHAR buffer[MAX_EDIT_STRING];

	// handle a few messages
	switch (message)
	{
		// key down: handle navigation in the attached view
		case WM_SYSKEYDOWN:
			if (wparam != VK_F10)
				return CallWindowProc(info->original_editproc, wnd, message, wparam, lparam);
			// (fall through)
		case WM_KEYDOWN:
			switch (wparam)
			{
				case VK_UP:
					if (info->last_history < info->history_count - 1)
						info->last_history++;
					else
						info->last_history = 0;
					SendMessage(wnd, WM_SETTEXT, (WPARAM)0, (LPARAM)&info->history[info->last_history][0]);
					SendMessage(wnd, EM_SETSEL, (WPARAM)MAX_EDIT_STRING, (LPARAM)MAX_EDIT_STRING);
					break;

				case VK_DOWN:
					if (info->last_history > 0)
						info->last_history--;
					else if (info->history_count > 0)
						info->last_history = info->history_count - 1;
					else
						info->last_history = 0;
					SendMessage(wnd, WM_SETTEXT, (WPARAM)0, (LPARAM)&info->history[info->last_history][0]);
					SendMessage(wnd, EM_SETSEL, (WPARAM)MAX_EDIT_STRING, (LPARAM)MAX_EDIT_STRING);
					break;

				case VK_PRIOR:
					if (info->view[0].wnd && info->view[0].vscroll)
						SendMessage(info->view[0].wnd, WM_VSCROLL, SB_PAGELEFT, (LPARAM)info->view[0].vscroll);
					break;

				case VK_NEXT:
					if (info->view[0].wnd && info->view[0].vscroll)
						SendMessage(info->view[0].wnd, WM_VSCROLL, SB_PAGERIGHT, (LPARAM)info->view[0].vscroll);
					break;

				case VK_TAB:
					if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
						debugwin_view_prev_view(info, NULL);
					else
						debugwin_view_next_view(info, NULL);
					info->ignore_char_lparam = lparam >> 16;
					break;

				default:
					if ((*info->handle_key)(info, wparam, lparam))
						info->ignore_char_lparam = lparam >> 16;
					else
						return CallWindowProc(info->original_editproc, wnd, message, wparam, lparam);
					break;
			}
			break;

		// char: handle the return key
		case WM_CHAR:

			// ignore chars associated with keys we've handled
			if (info->ignore_char_lparam == (lparam >> 16))
				info->ignore_char_lparam = 0;
			else if (waiting_for_debugger || !debugwin_seq_pressed(info->machine()))
			{
				switch (wparam)
				{
					case 13:
					{
						// fetch the text
						SendMessage(wnd, WM_GETTEXT, (WPARAM)ARRAY_LENGTH(buffer), (LPARAM)buffer);

						// add to the history if it's not a repeat of the last one
						if (buffer[0] != 0 && _tcscmp(buffer, &info->history[0][0]))
						{
							memmove(&info->history[1][0], &info->history[0][0], (HISTORY_LENGTH - 1) * MAX_EDIT_STRING);
							_tcscpy(&info->history[0][0], buffer);
							if (info->history_count < HISTORY_LENGTH)
								info->history_count++;
						}
						info->last_history = info->history_count - 1;

						// process
						if (info->process_string)
						{
							char *utf8_buffer = utf8_from_tstring(buffer);
							if (utf8_buffer != NULL)
							{
								(*info->process_string)(info, utf8_buffer);
								osd_free(utf8_buffer);
							}
						}
						break;
					}

					case 27:
					{
						// fetch the text
						SendMessage(wnd, WM_GETTEXT, (WPARAM)sizeof(buffer), (LPARAM)buffer);

						// if it's not empty, clear the text
						if (_tcslen(buffer) > 0)
						{
							info->ignore_char_lparam = lparam >> 16;
							SendMessage(wnd, WM_SETTEXT, (WPARAM)0, (LPARAM)info->edit_defstr);
						}
						break;
					}

					default:
						return CallWindowProc(info->original_editproc, wnd, message, wparam, lparam);
				}
			}
			break;

		// everything else: defaults
		default:
			return CallWindowProc(info->original_editproc, wnd, message, wparam, lparam);
	}

	return 0;
}



//============================================================
//  generic_create_window
//============================================================

#ifdef UNUSED_FUNCTION
static void generic_create_window(running_machine &machine, debug_view_type type)
{
	debugwin_info *info;
	char title[256];

	// create the window
	_snprintf(title, ARRAY_LENGTH(title), "Debug: %s [%s]", machine.system().description, machine.system().name);
	info = debugwin_window_create(machine, title, NULL);
	if (info == NULL || !debugwin_view_create(info, 0, type))
		return;

	// set the child function
	info->recompute_children = generic_recompute_children;

	// recompute the children once to get the maxwidth
	generic_recompute_children(info);

	// position the window and recompute children again
	SetWindowPos(info->wnd, HWND_TOP, 100, 100, info->maxwidth, 200, SWP_SHOWWINDOW);
	generic_recompute_children(info);
}
#endif



//============================================================
//  generic_recompute_children
//============================================================

static void generic_recompute_children(debugwin_info *info)
{
	debug_view_xy totalsize = info->view[0].view->total_size();
	RECT parent;
	RECT bounds;

	// compute a client rect
	bounds.top = bounds.left = 0;
	bounds.right = totalsize.x * debug_font_width + vscroll_width + 2 * EDGE_WIDTH;
	bounds.bottom = 200;
	AdjustWindowRectEx(&bounds, DEBUG_WINDOW_STYLE, FALSE, DEBUG_WINDOW_STYLE_EX);

	// clamp the min/max size
	info->maxwidth = bounds.right - bounds.left;

	// get the parent's dimensions
	GetClientRect(info->wnd, &parent);

	// view gets the remaining space
	InflateRect(&parent, -EDGE_WIDTH, -EDGE_WIDTH);
	debugwin_view_set_bounds(&info->view[0], info->wnd, &parent);
}



//============================================================
//  log_create_window
//============================================================

static void log_create_window(running_machine &machine)
{
	debug_view_xy totalsize;
	debugwin_info *info;
	char title[256];
	RECT bounds;

	// create the window
	_snprintf(title, ARRAY_LENGTH(title), "Errorlog: %s [%s]", machine.system().description, machine.system().name);
	info = debugwin_window_create(machine, title, NULL);
	if (info == NULL || !debugwin_view_create(info, 0, DVT_LOG))
		return;

	// set the child function
	info->recompute_children = generic_recompute_children;

	// get the view width
	totalsize = info->view[0].view->total_size();

	// compute a client rect
	bounds.top = bounds.left = 0;
	bounds.right = totalsize.x * debug_font_width + vscroll_width + 2 * EDGE_WIDTH;
	bounds.bottom = 200;
	AdjustWindowRectEx(&bounds, DEBUG_WINDOW_STYLE, FALSE, DEBUG_WINDOW_STYLE_EX);

	// clamp the min/max size
	info->maxwidth = bounds.right - bounds.left;

	// position the window at the bottom-right
	SetWindowPos(info->wnd, HWND_TOP,
				100, 100,
				bounds.right - bounds.left, bounds.bottom - bounds.top,
				SWP_SHOWWINDOW);

	// recompute the children and set focus on the edit box
	generic_recompute_children(info);
}



//============================================================
//  memory_create_window
//============================================================

static void memory_create_window(running_machine &machine)
{
	device_t *curcpu = debug_cpu_get_visible_cpu(machine);
	debugwin_info *info;
	HMENU optionsmenu;

	// create the window
	info = debugwin_window_create(machine, "Memory", NULL);
	if (info == NULL || !debugwin_view_create(info, 0, DVT_MEMORY))
		return;

	// set the handlers
	info->handle_command = memory_handle_command;
	info->handle_key = memory_handle_key;
	info->update_menu = memory_update_menu;

	// create the options menu
	optionsmenu = CreatePopupMenu();
	AppendMenu(optionsmenu, MF_ENABLED, ID_1_BYTE_CHUNKS, TEXT("1-byte chunks\tCtrl+1"));
	AppendMenu(optionsmenu, MF_ENABLED, ID_2_BYTE_CHUNKS, TEXT("2-byte chunks\tCtrl+2"));
	AppendMenu(optionsmenu, MF_ENABLED, ID_4_BYTE_CHUNKS, TEXT("4-byte chunks\tCtrl+4"));
	AppendMenu(optionsmenu, MF_ENABLED, ID_8_BYTE_CHUNKS, TEXT("8-byte chunks\tCtrl+8"));
	AppendMenu(optionsmenu, MF_DISABLED | MF_SEPARATOR, 0, TEXT(""));
	AppendMenu(optionsmenu, MF_ENABLED, ID_LOGICAL_ADDRESSES, TEXT("Logical Addresses\tCtrl+L"));
	AppendMenu(optionsmenu, MF_ENABLED, ID_PHYSICAL_ADDRESSES, TEXT("Physical Addresses\tCtrl+Y"));
	AppendMenu(optionsmenu, MF_DISABLED | MF_SEPARATOR, 0, TEXT(""));
	AppendMenu(optionsmenu, MF_ENABLED, ID_REVERSE_VIEW, TEXT("Reverse View\tCtrl+R"));
	AppendMenu(optionsmenu, MF_DISABLED | MF_SEPARATOR, 0, TEXT(""));
	AppendMenu(optionsmenu, MF_ENABLED, ID_INCREASE_MEM_WIDTH, TEXT("Increase bytes per line\tCtrl+P"));
	AppendMenu(optionsmenu, MF_ENABLED, ID_DECREASE_MEM_WIDTH, TEXT("Decrease bytes per line\tCtrl+O"));
	AppendMenu(GetMenu(info->wnd), MF_ENABLED | MF_POPUP, (UINT_PTR)optionsmenu, TEXT("Options"));

	// set up the view to track the initial expression
	downcast<debug_view_memory *>(info->view[0].view)->set_expression("0");
	strcpy(info->edit_defstr, "0");

	// create an edit box and override its key handling
	info->editwnd = CreateWindowEx(EDIT_BOX_STYLE_EX, TEXT("EDIT"), NULL, EDIT_BOX_STYLE,
			0, 0, 100, 100, info->wnd, NULL, GetModuleHandle(NULL), NULL);
	info->original_editproc = (WNDPROC)(FPTR)GetWindowLongPtr(info->editwnd, GWLP_WNDPROC);
	SetWindowLongPtr(info->editwnd, GWLP_USERDATA, (LONG_PTR)info);
	SetWindowLongPtr(info->editwnd, GWLP_WNDPROC, (LONG_PTR)debugwin_edit_proc);
	SendMessage(info->editwnd, WM_SETFONT, (WPARAM)debug_font, (LPARAM)FALSE);
	SendMessage(info->editwnd, WM_SETTEXT, (WPARAM)0, (LPARAM)TEXT("0"));
	SendMessage(info->editwnd, EM_LIMITTEXT, (WPARAM)MAX_EDIT_STRING, (LPARAM)0);
	SendMessage(info->editwnd, EM_SETSEL, (WPARAM)0, (LPARAM)-1);

	// create a combo box
	info->otherwnd[0] = CreateWindowEx(COMBO_BOX_STYLE_EX, TEXT("COMBOBOX"), NULL, COMBO_BOX_STYLE,
			0, 0, 100, 1000, info->wnd, NULL, GetModuleHandle(NULL), NULL);
	SetWindowLongPtr(info->otherwnd[0], GWLP_USERDATA, (LONG_PTR)info);
	SendMessage(info->otherwnd[0], WM_SETFONT, (WPARAM)debug_font, (LPARAM)FALSE);

	// populate the combobox
	int maxlength = 0;
	for (const debug_view_source *source = info->view[0].view->source_list().head(); source != NULL; source = source->next())
	{
		int length = strlen(source->name());
		if (length > maxlength)
			maxlength = length;
		TCHAR *t_name = tstring_from_utf8(source->name());
		SendMessage(info->otherwnd[0], CB_ADDSTRING, 0, (LPARAM)t_name);
		osd_free(t_name);
	}
	const debug_view_source *source = info->view[0].view->source_list().match_device(curcpu);
	SendMessage(info->otherwnd[0], CB_SETCURSEL, info->view[0].view->source_list().index(*source), 0);
	SendMessage(info->otherwnd[0], CB_SETDROPPEDWIDTH, (maxlength + 2) * debug_font_width + vscroll_width, 0);
	info->view[0].view->set_source(*source);

	// set the child functions
	info->recompute_children = memory_recompute_children;
	info->process_string = memory_process_string;

	// set the caption
	memory_update_caption(machine, info->wnd);

	// recompute the children once to get the maxwidth
	memory_recompute_children(info);

	// position the window and recompute children again
	SetWindowPos(info->wnd, HWND_TOP, 100, 100, info->maxwidth, 200, SWP_SHOWWINDOW);
	memory_recompute_children(info);

	// mark the edit box as the default focus and set it
	info->focuswnd = info->editwnd;
	SetFocus(info->editwnd);
}



//============================================================
//  memory_recompute_children
//============================================================

static void memory_recompute_children(debugwin_info *info)
{
	debug_view_xy totalsize = info->view[0].view->total_size();
	RECT parent, memrect, editrect, comborect;
	RECT bounds;

	// compute a client rect
	bounds.top = bounds.left = 0;
	bounds.right = totalsize.x * debug_font_width + vscroll_width + 2 * EDGE_WIDTH;
	bounds.bottom = 200;
	AdjustWindowRectEx(&bounds, DEBUG_WINDOW_STYLE, FALSE, DEBUG_WINDOW_STYLE_EX);

	// clamp the min/max size
	info->maxwidth = bounds.right - bounds.left;

	// get the parent's dimensions
	GetClientRect(info->wnd, &parent);

	// edit box gets half of the width
	editrect.top = parent.top + EDGE_WIDTH;
	editrect.bottom = editrect.top + debug_font_height + 4;
	editrect.left = parent.left + EDGE_WIDTH;
	editrect.right = parent.left + (parent.right - parent.left) / 2 - EDGE_WIDTH;

	// combo box gets the other half of the width
	comborect.top = editrect.top;
	comborect.bottom = editrect.bottom;
	comborect.left = editrect.right + 2 * EDGE_WIDTH;
	comborect.right = parent.right - EDGE_WIDTH;

	// memory view gets the rest
	memrect.top = editrect.bottom + 2 * EDGE_WIDTH;
	memrect.bottom = parent.bottom - EDGE_WIDTH;
	memrect.left = parent.left + EDGE_WIDTH;
	memrect.right = parent.right - EDGE_WIDTH;

	// set the bounds of things
	debugwin_view_set_bounds(&info->view[0], info->wnd, &memrect);
	smart_set_window_bounds(info->editwnd, info->wnd, &editrect);
	smart_set_window_bounds(info->otherwnd[0], info->wnd, &comborect);
}



//============================================================
//  memory_process_string
//============================================================

static void memory_process_string(debugwin_info *info, const char *string)
{
	// set the string to the memory view
	downcast<debug_view_memory *>(info->view[0].view)->set_expression(string);

	// select everything in the edit text box
	SendMessage(info->editwnd, EM_SETSEL, (WPARAM)0, (LPARAM)-1);

	// update the default string to match
	strncpy(info->edit_defstr, string, sizeof(info->edit_defstr) - 1);
}



//============================================================
//  memory_update_menu
//============================================================

static void memory_update_menu(debugwin_info *info)
{
	debug_view_memory *memview = downcast<debug_view_memory *>(info->view[0].view);
	CheckMenuItem(GetMenu(info->wnd), ID_1_BYTE_CHUNKS, MF_BYCOMMAND | (memview->bytes_per_chunk() == 1 ? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem(GetMenu(info->wnd), ID_2_BYTE_CHUNKS, MF_BYCOMMAND | (memview->bytes_per_chunk() == 2 ? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem(GetMenu(info->wnd), ID_4_BYTE_CHUNKS, MF_BYCOMMAND | (memview->bytes_per_chunk() == 4 ? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem(GetMenu(info->wnd), ID_8_BYTE_CHUNKS, MF_BYCOMMAND | (memview->bytes_per_chunk() == 8 ? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem(GetMenu(info->wnd), ID_LOGICAL_ADDRESSES, MF_BYCOMMAND | (memview->physical() ? MF_UNCHECKED : MF_CHECKED));
	CheckMenuItem(GetMenu(info->wnd), ID_PHYSICAL_ADDRESSES, MF_BYCOMMAND | (memview->physical() ? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem(GetMenu(info->wnd), ID_REVERSE_VIEW, MF_BYCOMMAND | (memview->reverse() ? MF_CHECKED : MF_UNCHECKED));
}



//============================================================
//  memory_handle_command
//============================================================

static int memory_handle_command(debugwin_info *info, WPARAM wparam, LPARAM lparam)
{
	debug_view_memory *memview = downcast<debug_view_memory *>(info->view[0].view);
	switch (HIWORD(wparam))
	{
		// combo box selection changed
		case CBN_SELCHANGE:
		{
			int sel = SendMessage((HWND)lparam, CB_GETCURSEL, 0, 0);
			if (sel != CB_ERR)
			{
				memview->set_source(*memview->source_list().by_index(sel));
				memory_update_caption(info->machine(), info->wnd);

				// reset the focus
				SetFocus(info->focuswnd);
				return 1;
			}
			break;
		}

		// menu selections
		case 0:
			switch (LOWORD(wparam))
			{
				case ID_1_BYTE_CHUNKS:
					memview->set_bytes_per_chunk(1);
					return 1;

				case ID_2_BYTE_CHUNKS:
					memview->set_bytes_per_chunk(2);
					return 1;

				case ID_4_BYTE_CHUNKS:
					memview->set_bytes_per_chunk(4);
					return 1;

				case ID_8_BYTE_CHUNKS:
					memview->set_bytes_per_chunk(8);
					return 1;

				case ID_LOGICAL_ADDRESSES:
					memview->set_physical(false);
					return 1;

				case ID_PHYSICAL_ADDRESSES:
					memview->set_physical(true);
					return 1;

				case ID_REVERSE_VIEW:
					memview->set_reverse(!memview->reverse());
					return 1;

				case ID_INCREASE_MEM_WIDTH:
					memview->set_chunks_per_row(memview->chunks_per_row() + 1);
					return 1;

				case ID_DECREASE_MEM_WIDTH:
					memview->set_chunks_per_row(memview->chunks_per_row() - 1);
					return 1;
			}
			break;
	}
	return global_handle_command(info, wparam, lparam);
}



//============================================================
//  memory_handle_key
//============================================================

static int memory_handle_key(debugwin_info *info, WPARAM wparam, LPARAM lparam)
{
	if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
	{
		switch (wparam)
		{
			case '1':
				SendMessage(info->wnd, WM_COMMAND, ID_1_BYTE_CHUNKS, 0);
				return 1;

			case '2':
				SendMessage(info->wnd, WM_COMMAND, ID_2_BYTE_CHUNKS, 0);
				return 1;

			case '4':
				SendMessage(info->wnd, WM_COMMAND, ID_4_BYTE_CHUNKS, 0);
				return 1;

			case '8':
				SendMessage(info->wnd, WM_COMMAND, ID_8_BYTE_CHUNKS, 0);
				return 1;

			case 'L':
				SendMessage(info->wnd, WM_COMMAND, ID_LOGICAL_ADDRESSES, 0);
				return 1;

			case 'Y':
				SendMessage(info->wnd, WM_COMMAND, ID_PHYSICAL_ADDRESSES, 0);
				return 1;

			case 'R':
				SendMessage(info->wnd, WM_COMMAND, ID_REVERSE_VIEW, 0);
				return 1;

			case 'P':
				SendMessage(info->wnd, WM_COMMAND, ID_INCREASE_MEM_WIDTH, 0);
				return 1;

			case 'O':
				SendMessage(info->wnd, WM_COMMAND, ID_DECREASE_MEM_WIDTH, 0);
				return 1;
		}
	}
	return global_handle_key(info, wparam, lparam);
}



//============================================================
//  memory_update_caption
//============================================================

static void memory_update_caption(running_machine &machine, HWND wnd)
{
	debugwin_info *info = (debugwin_info *)(FPTR)GetWindowLongPtr(wnd, GWLP_USERDATA);
	astring title;

	title.printf("Memory: %s", info->view[0].view->source()->name());
	win_set_window_text_utf8(wnd, title);
}



//============================================================
//  disasm_create_window
//============================================================

static void disasm_create_window(running_machine &machine)
{
	device_t *curcpu = debug_cpu_get_visible_cpu(machine);
	debugwin_info *info;
	HMENU optionsmenu;

	// create the window
	info = debugwin_window_create(machine, "Disassembly", NULL);
	if (info == NULL || !debugwin_view_create(info, 0, DVT_DISASSEMBLY))
		return;

	// create the options menu
	optionsmenu = CreatePopupMenu();
	AppendMenu(optionsmenu, MF_ENABLED, ID_TOGGLE_BREAKPOINT, TEXT("Set breakpoint at cursor\tF9"));
	AppendMenu(optionsmenu, MF_ENABLED, ID_RUN_TO_CURSOR, TEXT("Run to cursor\tF4"));
	AppendMenu(optionsmenu, MF_DISABLED | MF_SEPARATOR, 0, TEXT(""));
	AppendMenu(optionsmenu, MF_ENABLED, ID_SHOW_RAW, TEXT("Raw opcodes\tCtrl+R"));
	AppendMenu(optionsmenu, MF_ENABLED, ID_SHOW_ENCRYPTED, TEXT("Encrypted opcodes\tCtrl+E"));
	AppendMenu(optionsmenu, MF_ENABLED, ID_SHOW_COMMENTS, TEXT("Comments\tCtrl+M"));
	AppendMenu(GetMenu(info->wnd), MF_ENABLED | MF_POPUP, (UINT_PTR)optionsmenu, TEXT("Options"));

	// set the handlers
	info->handle_command = disasm_handle_command;
	info->handle_key = disasm_handle_key;
	info->update_menu = disasm_update_menu;

	// set up the view to track the initial expression
	downcast<debug_view_disasm *>(info->view[0].view)->set_expression("curpc");
	strcpy(info->edit_defstr, "curpc");

	// create an edit box and override its key handling
	info->editwnd = CreateWindowEx(EDIT_BOX_STYLE_EX, TEXT("EDIT"), NULL, EDIT_BOX_STYLE,
			0, 0, 100, 100, info->wnd, NULL, GetModuleHandle(NULL), NULL);
	info->original_editproc = (WNDPROC)(FPTR)GetWindowLongPtr(info->editwnd, GWLP_WNDPROC);
	SetWindowLongPtr(info->editwnd, GWLP_USERDATA, (LONG_PTR)info);
	SetWindowLongPtr(info->editwnd, GWLP_WNDPROC, (LONG_PTR)debugwin_edit_proc);
	SendMessage(info->editwnd, WM_SETFONT, (WPARAM)debug_font, (LPARAM)FALSE);
	SendMessage(info->editwnd, WM_SETTEXT, (WPARAM)0, (LPARAM)TEXT("curpc"));
	SendMessage(info->editwnd, EM_LIMITTEXT, (WPARAM)MAX_EDIT_STRING, (LPARAM)0);
	SendMessage(info->editwnd, EM_SETSEL, (WPARAM)0, (LPARAM)-1);

	// create a combo box
	info->otherwnd[0] = CreateWindowEx(COMBO_BOX_STYLE_EX, TEXT("COMBOBOX"), NULL, COMBO_BOX_STYLE,
			0, 0, 100, 1000, info->wnd, NULL, GetModuleHandle(NULL), NULL);
	SetWindowLongPtr(info->otherwnd[0], GWLP_USERDATA, (LONG_PTR)info);
	SendMessage(info->otherwnd[0], WM_SETFONT, (WPARAM)debug_font, (LPARAM)FALSE);

	// populate the combobox
	int maxlength = 0;
	for (const debug_view_source *source = info->view[0].view->source_list().head(); source != NULL; source = source->next())
	{
		int length = strlen(source->name());
		if (length > maxlength)
			maxlength = length;
		TCHAR *t_name = tstring_from_utf8(source->name());
		SendMessage(info->otherwnd[0], CB_ADDSTRING, 0, (LPARAM)t_name);
		osd_free(t_name);
	}
	const debug_view_source *source = info->view[0].view->source_list().match_device(curcpu);
	SendMessage(info->otherwnd[0], CB_SETCURSEL, info->view[0].view->source_list().index(*source), 0);
	SendMessage(info->otherwnd[0], CB_SETDROPPEDWIDTH, (maxlength + 2) * debug_font_width + vscroll_width, 0);
	info->view[0].view->set_source(*source);

	// set the child functions
	info->recompute_children = disasm_recompute_children;
	info->process_string = disasm_process_string;

	// set the caption
	disasm_update_caption(machine, info->wnd);

	// recompute the children once to get the maxwidth
	disasm_recompute_children(info);

	// position the window and recompute children again
	SetWindowPos(info->wnd, HWND_TOP, 100, 100, info->maxwidth, 200, SWP_SHOWWINDOW);
	disasm_recompute_children(info);

	// mark the edit box as the default focus and set it
	info->focuswnd = info->editwnd;
	SetFocus(info->editwnd);
}



//============================================================
//  disasm_recompute_children
//============================================================

static void disasm_recompute_children(debugwin_info *info)
{
	debug_view_xy totalsize = info->view[0].view->total_size();
	RECT parent, dasmrect, editrect, comborect;
	RECT bounds;

	// compute a client rect
	bounds.top = bounds.left = 0;
	bounds.right = totalsize.x * debug_font_width + vscroll_width + 2 * EDGE_WIDTH;
	bounds.bottom = 200;
	AdjustWindowRectEx(&bounds, DEBUG_WINDOW_STYLE, FALSE, DEBUG_WINDOW_STYLE_EX);

	// clamp the min/max size
	info->maxwidth = bounds.right - bounds.left;

	// get the parent's dimensions
	GetClientRect(info->wnd, &parent);

	// edit box gets half of the width
	editrect.top = parent.top + EDGE_WIDTH;
	editrect.bottom = editrect.top + debug_font_height + 4;
	editrect.left = parent.left + EDGE_WIDTH;
	editrect.right = parent.left + (parent.right - parent.left) / 2 - EDGE_WIDTH;

	// combo box gets the other half of the width
	comborect.top = editrect.top;
	comborect.bottom = editrect.bottom;
	comborect.left = editrect.right + 2 * EDGE_WIDTH;
	comborect.right = parent.right - EDGE_WIDTH;

	// disasm view gets the rest
	dasmrect.top = editrect.bottom + 2 * EDGE_WIDTH;
	dasmrect.bottom = parent.bottom - EDGE_WIDTH;
	dasmrect.left = parent.left + EDGE_WIDTH;
	dasmrect.right = parent.right - EDGE_WIDTH;

	// set the bounds of things
	debugwin_view_set_bounds(&info->view[0], info->wnd, &dasmrect);
	smart_set_window_bounds(info->editwnd, info->wnd, &editrect);
	smart_set_window_bounds(info->otherwnd[0], info->wnd, &comborect);
}



//============================================================
//  disasm_process_string
//============================================================

static void disasm_process_string(debugwin_info *info, const char *string)
{
	// set the string to the disasm view
	downcast<debug_view_disasm *>(info->view[0].view)->set_expression(string);

	// select everything in the edit text box
	SendMessage(info->editwnd, EM_SETSEL, (WPARAM)0, (LPARAM)-1);

	// update the default string to match
	strncpy(info->edit_defstr, string, sizeof(info->edit_defstr) - 1);
}



//============================================================
//  disasm_update_menu
//============================================================

static void disasm_update_menu(debugwin_info *info)
{
	disasm_right_column rightcol = downcast<debug_view_disasm *>(info->view[0].view)->right_column();
	CheckMenuItem(GetMenu(info->wnd), ID_SHOW_RAW, MF_BYCOMMAND | (rightcol == DASM_RIGHTCOL_RAW ? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem(GetMenu(info->wnd), ID_SHOW_ENCRYPTED, MF_BYCOMMAND | (rightcol == DASM_RIGHTCOL_ENCRYPTED ? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem(GetMenu(info->wnd), ID_SHOW_COMMENTS, MF_BYCOMMAND | (rightcol == DASM_RIGHTCOL_COMMENTS ? MF_CHECKED : MF_UNCHECKED));
}



//============================================================
//  disasm_handle_command
//============================================================

static int disasm_handle_command(debugwin_info *info, WPARAM wparam, LPARAM lparam)
{
	debug_view_disasm *dasmview = downcast<debug_view_disasm *>(info->view[0].view);
	char command[64];

	switch (HIWORD(wparam))
	{
		// combo box selection changed
		case CBN_SELCHANGE:
		{
			int sel = SendMessage((HWND)lparam, CB_GETCURSEL, 0, 0);
			if (sel != CB_ERR)
			{
				dasmview->set_source(*dasmview->source_list().by_index(sel));
				disasm_update_caption(info->machine(), info->wnd);

				// reset the focus
				SetFocus(info->focuswnd);
				return 1;
			}
			break;
		}

		// menu selections
		case 0:
			switch (LOWORD(wparam))
			{
				case ID_SHOW_RAW:
					dasmview->set_right_column(DASM_RIGHTCOL_RAW);
					(*info->recompute_children)(info);
					return 1;

				case ID_SHOW_ENCRYPTED:
					dasmview->set_right_column(DASM_RIGHTCOL_ENCRYPTED);
					(*info->recompute_children)(info);
					return 1;

				case ID_SHOW_COMMENTS:
					dasmview->set_right_column(DASM_RIGHTCOL_COMMENTS);
					(*info->recompute_children)(info);
					return 1;

				case ID_RUN_TO_CURSOR:
					if (dasmview->cursor_visible() && debug_cpu_get_visible_cpu(info->machine()) == dasmview->source()->device())
					{
						offs_t address = dasmview->selected_address();
						sprintf(command, "go 0x%X", address);
						debug_console_execute_command(info->machine(), command, 1);
					}
					return 1;

				case ID_TOGGLE_BREAKPOINT:
					if (dasmview->cursor_visible() && debug_cpu_get_visible_cpu(info->machine()) == dasmview->source()->device())
					{
						offs_t address = dasmview->selected_address();
						device_debug *debug = dasmview->source()->device()->debug();
						INT32 bpindex = -1;

						/* first find an existing breakpoint at this address */
						for (device_debug::breakpoint *bp = debug->breakpoint_first(); bp != NULL; bp = bp->next())
							if (address == bp->address())
							{
								bpindex = bp->index();
								break;
							}

						/* if it doesn't exist, add a new one */
						if (bpindex == -1)
							sprintf(command, "bpset 0x%X", address);
						else
							sprintf(command, "bpclear 0x%X", bpindex);
						debug_console_execute_command(info->machine(), command, 1);
					}
					return 1;
			}
			break;
	}
	return global_handle_command(info, wparam, lparam);
}



//============================================================
//  disasm_handle_key
//============================================================

static int disasm_handle_key(debugwin_info *info, WPARAM wparam, LPARAM lparam)
{
	if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
	{
		switch (wparam)
		{
			case 'R':
				SendMessage(info->wnd, WM_COMMAND, ID_SHOW_RAW, 0);
				return 1;

			case 'E':
				SendMessage(info->wnd, WM_COMMAND, ID_SHOW_ENCRYPTED, 0);
				return 1;

			case 'N':
				SendMessage(info->wnd, WM_COMMAND, ID_SHOW_COMMENTS, 0);
				return 1;
		}
	}

	switch (wparam)
	{
		/* ajg - steals the F4 from the global key handler - but ALT+F4 didn't work anyways ;) */
		case VK_F4:
			SendMessage(info->wnd, WM_COMMAND, ID_RUN_TO_CURSOR, 0);
			return 1;

		case VK_F9:
			SendMessage(info->wnd, WM_COMMAND, ID_TOGGLE_BREAKPOINT, 0);
			return 1;

		case VK_RETURN:
			if (info->view[0].view->cursor_visible())
			{
				SendMessage(info->wnd, WM_COMMAND, ID_STEP, 0);
				return 1;
			}
			break;
	}

	return global_handle_key(info, wparam, lparam);
}



//============================================================
//  disasm_update_caption
//============================================================

static void disasm_update_caption(running_machine &machine, HWND wnd)
{
	debugwin_info *info = (debugwin_info *)(FPTR)GetWindowLongPtr(wnd, GWLP_USERDATA);
	astring title;

	title.printf("Disassembly: %s", info->view[0].view->source()->name());
	win_set_window_text_utf8(wnd, title);
}


enum
{
	DEVOPTION_OPEN,
	DEVOPTION_CREATE,
	DEVOPTION_CLOSE,
	DEVOPTION_CASSETTE_STOPPAUSE,
	DEVOPTION_CASSETTE_PLAY,
	DEVOPTION_CASSETTE_RECORD,
	DEVOPTION_CASSETTE_REWIND,
	DEVOPTION_CASSETTE_FASTFORWARD,
	DEVOPTION_MAX
};

//============================================================
//  memory_update_menu
//============================================================

static void image_update_menu(debugwin_info *info)
{
	UINT32 cnt = 0;
	HMENU devicesmenu;

	DeleteMenu(GetMenu(info->wnd), 2, MF_BYPOSITION);

	// create the image menu
	devicesmenu = CreatePopupMenu();
	image_interface_iterator iter(info->machine().root_device());
	for (device_image_interface *img = iter.first(); img != NULL; img = iter.next())
	{
		astring temp;
		UINT flags_for_exists;
		UINT flags_for_writing;
		HMENU devicesubmenu = CreatePopupMenu();

		UINT_PTR new_item = ID_DEVICE_OPTIONS + (cnt * DEVOPTION_MAX);

		flags_for_exists = MF_ENABLED | MF_STRING;

		if (!img->exists())
			flags_for_exists |= MF_GRAYED;

		flags_for_writing = flags_for_exists;
		if (img->is_readonly())
			flags_for_writing |= MF_GRAYED;

		AppendMenu(devicesubmenu, MF_STRING, new_item + DEVOPTION_OPEN, TEXT("Mount..."));

		/*if (img->is_creatable())
            AppendMenu(devicesubmenu, MF_STRING, new_item + DEVOPTION_CREATE, TEXT("Create..."));
        */
		AppendMenu(devicesubmenu, flags_for_exists, new_item + DEVOPTION_CLOSE, TEXT("Unmount"));

		if (img->device().type() == CASSETTE)
		{
			cassette_state state;
			state = (cassette_state)(img->exists() ? (dynamic_cast<cassette_image_device*>(&img->device())->get_state() & CASSETTE_MASK_UISTATE) : CASSETTE_STOPPED);
			AppendMenu(devicesubmenu, MF_SEPARATOR, 0, NULL);
			AppendMenu(devicesubmenu, flags_for_exists	| ((state == CASSETTE_STOPPED)	? MF_CHECKED : 0), new_item + DEVOPTION_CASSETTE_STOPPAUSE, TEXT("Pause/Stop"));
			AppendMenu(devicesubmenu, flags_for_exists	| ((state == CASSETTE_PLAY) ? MF_CHECKED : 0), new_item + DEVOPTION_CASSETTE_PLAY, TEXT("Play"));
			AppendMenu(devicesubmenu, flags_for_writing | ((state == CASSETTE_RECORD)	? MF_CHECKED : 0), new_item + DEVOPTION_CASSETTE_RECORD, TEXT("Record"));
			AppendMenu(devicesubmenu, flags_for_exists, new_item + DEVOPTION_CASSETTE_REWIND, TEXT("Rewind"));
			AppendMenu(devicesubmenu, flags_for_exists, new_item + DEVOPTION_CASSETTE_FASTFORWARD, TEXT("Fast Forward"));
		}

		temp.format("%s :%s", img->device().name(), img->exists() ? img->filename() : "[empty slot]");

		AppendMenu(devicesmenu, MF_ENABLED | MF_POPUP, (UINT_PTR)devicesubmenu,  tstring_from_utf8(temp.cstr()));

		cnt++;
	}
	AppendMenu(GetMenu(info->wnd), MF_ENABLED | MF_POPUP, (UINT_PTR)devicesmenu, TEXT("Images"));

}

//============================================================
//  console_create_window
//============================================================

void console_create_window(running_machine &machine)
{
	debugwin_info *info;
	int bestwidth, bestheight;
	RECT bounds, work_bounds;
	HMENU optionsmenu;
	UINT32 conchars;

	// create the window
	info = debugwin_window_create(machine, "Debug", NULL);
	if (info == NULL)
		return;
	main_console = info;

	// create the views
	if (!debugwin_view_create(info, 0, DVT_DISASSEMBLY))
		goto cleanup;
	if (!debugwin_view_create(info, 1, DVT_STATE))
		goto cleanup;
	if (!debugwin_view_create(info, 2, DVT_CONSOLE))
		goto cleanup;

	// create the options menu
	optionsmenu = CreatePopupMenu();
	AppendMenu(optionsmenu, MF_ENABLED, ID_TOGGLE_BREAKPOINT, TEXT("Set breakpoint at cursor\tF9"));
	AppendMenu(optionsmenu, MF_ENABLED, ID_RUN_TO_CURSOR, TEXT("Run to cursor\tF4"));
	AppendMenu(optionsmenu, MF_DISABLED | MF_SEPARATOR, 0, TEXT(""));
	AppendMenu(optionsmenu, MF_ENABLED, ID_SHOW_RAW, TEXT("Raw opcodes\tCtrl+R"));
	AppendMenu(optionsmenu, MF_ENABLED, ID_SHOW_ENCRYPTED, TEXT("Encrypted opcodes\tCtrl+E"));
	AppendMenu(optionsmenu, MF_ENABLED, ID_SHOW_COMMENTS, TEXT("Comments\tCtrl+N"));
	AppendMenu(GetMenu(info->wnd), MF_ENABLED | MF_POPUP, (UINT_PTR)optionsmenu, TEXT("Options"));

	// Add image menu only if image devices exist
	{
		image_interface_iterator iter(machine.root_device());
		if (iter.first() != NULL) {
			info->update_menu = image_update_menu;
			image_update_menu(info);
		}
	}

	// set the handlers
	info->handle_command = disasm_handle_command;
	info->handle_key = disasm_handle_key;

	// create an edit box and override its key handling
	info->editwnd = CreateWindowEx(EDIT_BOX_STYLE_EX, TEXT("EDIT"), NULL, EDIT_BOX_STYLE,
			0, 0, 100, 100, info->wnd, NULL, GetModuleHandle(NULL), NULL);
	info->original_editproc = (WNDPROC)(FPTR)GetWindowLongPtr(info->editwnd, GWLP_WNDPROC);
	SetWindowLongPtr(info->editwnd, GWLP_USERDATA, (LONG_PTR)info);
	SetWindowLongPtr(info->editwnd, GWLP_WNDPROC, (LONG_PTR)debugwin_edit_proc);
	SendMessage(info->editwnd, WM_SETFONT, (WPARAM)debug_font, (LPARAM)FALSE);
	SendMessage(info->editwnd, EM_LIMITTEXT, (WPARAM)MAX_EDIT_STRING, (LPARAM)0);

	// set the child functions
	info->recompute_children = console_recompute_children;
	info->process_string = console_process_string;

	// loop over all register views and get the maximum size
	main_console_regwidth = 0;
	for (const debug_view_source *source = info->view[1].view->source_list().head(); source != NULL; source = source->next())
	{
		UINT32 regchars;

		// set the view and fetch the width
		info->view[1].view->set_source(*source);
		regchars = info->view[1].view->total_size().x;

		// track the maximum
		main_console_regwidth = MAX(regchars, main_console_regwidth);
	}

	// determine the width of the console (this is fixed)
	conchars = info->view[2].view->total_size().x;

	// loop over all CPUs and compute the width range based on dasm width
	info->minwidth = 0;
	info->maxwidth = 0;
	for (const debug_view_source *source = info->view[0].view->source_list().head(); source != NULL; source = source->next())
	{
		UINT32 minwidth, maxwidth, dischars;

		// set the view and fetch the width
		info->view[0].view->set_source(*source);
		dischars = info->view[0].view->total_size().x;

		// compute the preferred width
		minwidth = EDGE_WIDTH + main_console_regwidth * debug_font_width + vscroll_width + 2 * EDGE_WIDTH + 100 + EDGE_WIDTH;
		maxwidth = EDGE_WIDTH + main_console_regwidth * debug_font_width + vscroll_width + 2 * EDGE_WIDTH + MAX(dischars, conchars) * debug_font_width + vscroll_width + EDGE_WIDTH;
		info->minwidth = MAX(info->minwidth, minwidth);
		info->maxwidth = MAX(info->maxwidth, maxwidth);
	}

	// get the work bounds
	SystemParametersInfo(SPI_GETWORKAREA, 0, &work_bounds, 0);

	// adjust the min/max sizes for the window style
	bounds.top = bounds.left = 0;
	bounds.right = bounds.bottom = info->minwidth;
	AdjustWindowRectEx(&bounds, DEBUG_WINDOW_STYLE, FALSE, DEBUG_WINDOW_STYLE_EX);
	info->minwidth = bounds.right - bounds.left;

	bounds.top = bounds.left = 0;
	bounds.right = bounds.bottom = info->maxwidth;
	AdjustWindowRectEx(&bounds, DEBUG_WINDOW_STYLE, FALSE, DEBUG_WINDOW_STYLE_EX);
	info->maxwidth = bounds.right - bounds.left;

	// position the window at the bottom-right
	bestwidth = (info->maxwidth < (work_bounds.right - work_bounds.left)) ? info->maxwidth : (work_bounds.right - work_bounds.left);
	bestheight = (500 < (work_bounds.bottom - work_bounds.top)) ? 500 : (work_bounds.bottom - work_bounds.top);
	SetWindowPos(info->wnd, HWND_TOP,
				work_bounds.right - bestwidth, work_bounds.bottom - bestheight,
				bestwidth, bestheight,
				SWP_SHOWWINDOW);

	// recompute the children
	console_set_cpu(debug_cpu_get_visible_cpu(machine));

	// set up the disassembly view to track the current pc
	downcast<debug_view_disasm *>(info->view[0].view)->set_expression("curpc");

	// mark the edit box as the default focus and set it
	info->focuswnd = info->editwnd;
	SetFocus(info->editwnd);
	return;

cleanup:
	if (info->view[2].view)
		machine.debug_view().free_view(*info->view[2].view);
	if (info->view[1].view)
		machine.debug_view().free_view(*info->view[1].view);
	if (info->view[0].view)
		machine.debug_view().free_view(*info->view[0].view);
}



//============================================================
//  console_recompute_children
//============================================================

static void console_recompute_children(debugwin_info *info)
{
	RECT parent, regrect, disrect, conrect, editrect;
	UINT32 regchars;//, dischars, conchars;

	// get the parent's dimensions
	GetClientRect(info->wnd, &parent);

	// get the total width of all three children
	//dischars = info->view[0].view->total_size().x;
	regchars = main_console_regwidth;
	//conchars = info->view[2].view->total_size().x;

	// registers always get their desired width, and span the entire height
	regrect.top = parent.top + EDGE_WIDTH;
	regrect.bottom = parent.bottom - EDGE_WIDTH;
	regrect.left = parent.left + EDGE_WIDTH;
	regrect.right = regrect.left + debug_font_width * regchars + vscroll_width;

	// edit box goes at the bottom of the remaining area
	editrect.bottom = parent.bottom - EDGE_WIDTH;
	editrect.top = editrect.bottom - debug_font_height - 4;
	editrect.left = regrect.right + EDGE_WIDTH * 2;
	editrect.right = parent.right - EDGE_WIDTH;

	// console and disassembly split the difference
	disrect.top = parent.top + EDGE_WIDTH;
	disrect.bottom = (editrect.top - parent.top) / 2 - EDGE_WIDTH;
	disrect.left = regrect.right + EDGE_WIDTH * 2;
	disrect.right = parent.right - EDGE_WIDTH;

	conrect.top = disrect.bottom + EDGE_WIDTH * 2;
	conrect.bottom = editrect.top - EDGE_WIDTH;
	conrect.left = regrect.right + EDGE_WIDTH * 2;
	conrect.right = parent.right - EDGE_WIDTH;

	// set the bounds of things
	debugwin_view_set_bounds(&info->view[0], info->wnd, &disrect);
	debugwin_view_set_bounds(&info->view[1], info->wnd, &regrect);
	debugwin_view_set_bounds(&info->view[2], info->wnd, &conrect);
	smart_set_window_bounds(info->editwnd, info->wnd, &editrect);
}



//============================================================
//  console_process_string
//============================================================

static void console_process_string(debugwin_info *info, const char *string)
{
	TCHAR buffer = 0;

	// an empty string is a single step
	if (string[0] == 0)
		debug_cpu_get_visible_cpu(info->machine())->debug()->single_step();

	// otherwise, just process the command
	else
		debug_console_execute_command(info->machine(), string, 1);

	// clear the edit text box
	SendMessage(info->editwnd, WM_SETTEXT, 0, (LPARAM)&buffer);
}



//============================================================
//  console_set_cpu
//============================================================

static void console_set_cpu(device_t *device)
{
	// first set all the views to the new cpu number
	main_console->view[0].view->set_source(*main_console->view[0].view->source_list().match_device(device));
	main_console->view[1].view->set_source(*main_console->view[1].view->source_list().match_device(device));

	// then update the caption
	char curtitle[256];
	astring title;

	title.printf("Debug: %s - %s '%s'", device->machine().system().name, device->name(), device->tag());
	win_get_window_text_utf8(main_console->wnd, curtitle, ARRAY_LENGTH(curtitle));
	if (title.cmp(curtitle) != 0)
		win_set_window_text_utf8(main_console->wnd, title);

	// and recompute the children
	console_recompute_children(main_console);
}



//============================================================
//  create_standard_menubar
//============================================================

static HMENU create_standard_menubar(void)
{
	HMENU menubar, debugmenu;

	// create the debug menu
	debugmenu = CreatePopupMenu();
	if (debugmenu == NULL)
		return NULL;
	AppendMenu(debugmenu, MF_ENABLED, ID_NEW_MEMORY_WND, TEXT("New Memory Window\tCtrl+M"));
	AppendMenu(debugmenu, MF_ENABLED, ID_NEW_DISASM_WND, TEXT("New Disassembly Window\tCtrl+D"));
	AppendMenu(debugmenu, MF_ENABLED, ID_NEW_LOG_WND, TEXT("New Error Log Window\tCtrl+L"));
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
	menubar = CreateMenu();
	if (menubar == NULL)
	{
		DestroyMenu(debugmenu);
		return NULL;
	}
	AppendMenu(menubar, MF_ENABLED | MF_POPUP, (UINT_PTR)debugmenu, TEXT("Debug"));

	return menubar;
}

//============================================================
//  copy_extension_list
//============================================================

static int copy_extension_list(char *dest, size_t dest_len, const char *extensions)
{
	const char *s;
	int pos = 0;

	// our extension lists are comma delimited; Win32 expects to see lists
	// delimited by semicolons
	s = extensions;
	while(*s)
	{
		// append a semicolon if not at the beginning
		if (s != extensions)
			pos += snprintf(&dest[pos], dest_len - pos, ";");

		// append ".*"
		pos += snprintf(&dest[pos], dest_len - pos, "*.");

		// append the file extension
		while(*s && (*s != ','))
		{
			pos += snprintf(&dest[pos], dest_len - pos, "%c", *s);
			s++;
		}

		// if we found a comma, advance
		while(*s == ',')
			s++;
	}
	return pos;
}

//============================================================
//  add_filter_entry
//============================================================

static int add_filter_entry(char *dest, size_t dest_len, const char *description, const char *extensions)
{
	int pos = 0;

	// add the description
	pos += snprintf(&dest[pos], dest_len - pos, "%s (", description);

	// add the extensions to the description
	pos += copy_extension_list(&dest[pos], dest_len - pos, extensions);

	// add the trailing rparen and '|' character
	pos += snprintf(&dest[pos], dest_len - pos, ")|");

	// now add the extension list itself
	pos += copy_extension_list(&dest[pos], dest_len - pos, extensions);

	// append a '|'
	pos += snprintf(&dest[pos], dest_len - pos, "|");

	return pos;
}
//============================================================
//  build_generic_filter
//============================================================

static void build_generic_filter(device_image_interface *img, int is_save, char *filter, size_t filter_len)
{
	char *s;

	const char *file_extension;

	/* copy the string */
	file_extension = img->file_extensions();

	// start writing the filter
	s = filter;

	// common image types
	s += add_filter_entry(filter, filter_len, "Common image types", file_extension);

	// all files
	s += sprintf(s, "All files (*.*)|*.*|");

	// compressed
	if (!is_save)
		s += sprintf(s, "Compressed Images (*.zip)|*.zip|");

	*(s++) = '\0';
}

//============================================================
//  global_handle_command
//============================================================

static int global_handle_command(debugwin_info *info, WPARAM wparam, LPARAM lparam)
{
	if (HIWORD(wparam) == 0)
		switch (LOWORD(wparam))
		{
			case ID_NEW_MEMORY_WND:
				memory_create_window(info->machine());
				return 1;

			case ID_NEW_DISASM_WND:
				disasm_create_window(info->machine());
				return 1;

			case ID_NEW_LOG_WND:
				log_create_window(info->machine());
				return 1;

			case ID_RUN_AND_HIDE:
				smart_show_all(FALSE);
			case ID_RUN:
				debug_cpu_get_visible_cpu(info->machine())->debug()->go();
				return 1;

			case ID_NEXT_CPU:
				debug_cpu_get_visible_cpu(info->machine())->debug()->go_next_device();
				return 1;

			case ID_RUN_VBLANK:
				debug_cpu_get_visible_cpu(info->machine())->debug()->go_vblank();
				return 1;

			case ID_RUN_IRQ:
				debug_cpu_get_visible_cpu(info->machine())->debug()->go_interrupt();
				return 1;

			case ID_STEP:
				debug_cpu_get_visible_cpu(info->machine())->debug()->single_step();
				return 1;

			case ID_STEP_OVER:
				debug_cpu_get_visible_cpu(info->machine())->debug()->single_step_over();
				return 1;

			case ID_STEP_OUT:
				debug_cpu_get_visible_cpu(info->machine())->debug()->single_step_out();
				return 1;

			case ID_HARD_RESET:
				info->machine().schedule_hard_reset();
				return 1;

			case ID_SOFT_RESET:
				info->machine().schedule_soft_reset();
				debug_cpu_get_visible_cpu(info->machine())->debug()->go();
				return 1;

			case ID_EXIT:
				if (info->focuswnd != NULL)
					SetFocus(info->focuswnd);
				info->machine().schedule_exit();
				return 1;
		}
		if (LOWORD(wparam) >= ID_DEVICE_OPTIONS) {
			UINT32 devid = (LOWORD(wparam) - ID_DEVICE_OPTIONS) / DEVOPTION_MAX;
			image_interface_iterator iter(info->machine().root_device());
			device_image_interface *img = iter.byindex(devid);
			if (img!=NULL) {
				switch ((LOWORD(wparam) - ID_DEVICE_OPTIONS) % DEVOPTION_MAX)
				{
					case DEVOPTION_OPEN :
											{
												char filter[2048];
												build_generic_filter(img, FALSE, filter, ARRAY_LENGTH(filter));

												TCHAR selectedFilename[MAX_PATH];
												selectedFilename[0] = '\0';
												LPTSTR buffer;
												LPTSTR t_filter = NULL;

												buffer = tstring_from_utf8(filter);

												// convert a pipe-char delimited string into a NUL delimited string
												t_filter = (LPTSTR) alloca((_tcslen(buffer) + 2) * sizeof(*t_filter));
												int i = 0;
												for (i = 0; buffer[i] != '\0'; i++)
													t_filter[i] = (buffer[i] != '|') ? buffer[i] : '\0';
												t_filter[i++] = '\0';
												t_filter[i++] = '\0';
												osd_free(buffer);


												OPENFILENAME  ofn;
												memset(&ofn,0,sizeof(ofn));
												ofn.lStructSize = sizeof(ofn);
												ofn.hwndOwner = NULL;
												ofn.lpstrFile = selectedFilename;
												ofn.lpstrFile[0] = '\0';
												ofn.nMaxFile = MAX_PATH;
												ofn.lpstrFilter = t_filter;
												ofn.nFilterIndex = 1;
												ofn.lpstrFileTitle = NULL;
												ofn.nMaxFileTitle = 0;
												ofn.lpstrInitialDir = NULL;
												ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

												if (GetOpenFileName(&ofn)) {
													img->load(utf8_from_tstring(selectedFilename));
												}
											}
											return 1;
/*                  case DEVOPTION_CREATE:
                                            return 1;*/
					case DEVOPTION_CLOSE:
											img->unload();
											return 1;
					default:
						if (img->device().type() == CASSETTE) {
							cassette_image_device* cassette = dynamic_cast<cassette_image_device*>(&img->device());
							switch ((LOWORD(wparam) - ID_DEVICE_OPTIONS) % DEVOPTION_MAX)
							{

								case DEVOPTION_CASSETTE_STOPPAUSE:
														cassette->change_state(CASSETTE_STOPPED, CASSETTE_MASK_UISTATE);
														return 1;
								case DEVOPTION_CASSETTE_PLAY:
														cassette->change_state(CASSETTE_PLAY, CASSETTE_MASK_UISTATE);
														return 1;
								case DEVOPTION_CASSETTE_RECORD:
														cassette->change_state(CASSETTE_RECORD, CASSETTE_MASK_UISTATE);
														return 1;
								case DEVOPTION_CASSETTE_REWIND:
														cassette->seek(-60.0, SEEK_CUR);
														return 1;
								case DEVOPTION_CASSETTE_FASTFORWARD:
														cassette->seek(+60.0, SEEK_CUR);
														return 1;
							}
						}
				}
			}
		}
	return 0;
}



//============================================================
//  global_handle_key
//============================================================

static int global_handle_key(debugwin_info *info, WPARAM wparam, LPARAM lparam)
{
	/* ignore any keys that are received while the debug key is down */
	if (!waiting_for_debugger && debugwin_seq_pressed(info->machine()))
		return 1;

	switch (wparam)
	{
		case VK_F3:
			if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
				SendMessage(info->wnd, WM_COMMAND, ID_HARD_RESET, 0);
			else
				SendMessage(info->wnd, WM_COMMAND, ID_SOFT_RESET, 0);
			return 1;

		case VK_F4:
			if (GetAsyncKeyState(VK_MENU) & 0x8000)
			{
				/* ajg - never gets here since 'alt' seems to be captured somewhere else - menu maybe? */
				SendMessage(info->wnd, WM_COMMAND, ID_EXIT, 0);
				return 1;
			}
			break;

		case VK_F5:
			SendMessage(info->wnd, WM_COMMAND, ID_RUN, 0);
			return 1;

		case VK_F6:
			SendMessage(info->wnd, WM_COMMAND, ID_NEXT_CPU, 0);
			return 1;

		case VK_F7:
			SendMessage(info->wnd, WM_COMMAND, ID_RUN_IRQ, 0);
			return 1;

		case VK_F8:
			SendMessage(info->wnd, WM_COMMAND, ID_RUN_VBLANK, 0);
			return 1;

		case VK_F10:
			SendMessage(info->wnd, WM_COMMAND, ID_STEP_OVER, 0);
			return 1;

		case VK_F11:
			if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
				SendMessage(info->wnd, WM_COMMAND, ID_STEP_OUT, 0);
			else
				SendMessage(info->wnd, WM_COMMAND, ID_STEP, 0);
			return 1;

		case VK_F12:
			SendMessage(info->wnd, WM_COMMAND, ID_RUN_AND_HIDE, 0);
			return 1;

		case 'M':
			if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
			{
				SendMessage(info->wnd, WM_COMMAND, ID_NEW_MEMORY_WND, 0);
				return 1;
			}
			break;

		case 'D':
			if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
			{
				SendMessage(info->wnd, WM_COMMAND, ID_NEW_DISASM_WND, 0);
				return 1;
			}
			break;

		case 'L':
			if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
			{
				SendMessage(info->wnd, WM_COMMAND, ID_NEW_LOG_WND, 0);
				return 1;
			}
			break;
	}

	return 0;
}



//============================================================
//  smart_set_window_bounds
//============================================================

static void smart_set_window_bounds(HWND wnd, HWND parent, RECT *bounds)
{
	RECT curbounds;
	int flags = 0;

	// first get the current bounds, relative to the parent
	GetWindowRect(wnd, &curbounds);
	if (parent != NULL)
	{
		RECT parentbounds;
		GetWindowRect(parent, &parentbounds);
		curbounds.top -= parentbounds.top;
		curbounds.bottom -= parentbounds.top;
		curbounds.left -= parentbounds.left;
		curbounds.right -= parentbounds.left;
	}

	// if the position matches, don't change it
	if (curbounds.top == bounds->top && curbounds.left == bounds->left)
		flags |= SWP_NOMOVE;
	if ((curbounds.bottom - curbounds.top) == (bounds->bottom - bounds->top) &&
		(curbounds.right - curbounds.left) == (bounds->right - bounds->left))
		flags |= SWP_NOSIZE;

	// if we need to, reposition the window
	if (flags != (SWP_NOMOVE | SWP_NOSIZE))
		SetWindowPos(wnd, NULL,
					bounds->left, bounds->top,
					bounds->right - bounds->left, bounds->bottom - bounds->top,
					SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOZORDER | flags);
}



//============================================================
//  smart_show_window
//============================================================

static void smart_show_window(HWND wnd, BOOL show)
{
	BOOL visible = IsWindowVisible(wnd);
	if ((visible && !show) || (!visible && show))
		ShowWindow(wnd, show ? SW_SHOW : SW_HIDE);
}



//============================================================
//  smart_show_all
//============================================================

static void smart_show_all(BOOL show)
{
	debugwin_info *info;
	if (!show)
		SetForegroundWindow(win_window_list->hwnd);
	for (info = window_list; info != NULL; info = info->next)
		smart_show_window(info->wnd, show);
}
