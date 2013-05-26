//============================================================
//
//  window.c - Win32 window handling
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

#define LOG_THREADS         0
#define LOG_TEMP_PAUSE      0

// Needed for RAW Input
#define WM_INPUT 0x00FF

// standard windows headers
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>

// standard C headers
#include <process.h>

// MAME headers
#include "emu.h"
#include "emuopts.h"
#include "uiinput.h"

// MAMEOS headers
#include "winmain.h"
#include "window.h"
#include "video.h"
#include "input.h"
#include "debugwin.h"
#include "strconv.h"
#include "config.h"
#include "winutf8.h"

extern int drawnone_init(running_machine &machine, win_draw_callbacks *callbacks);
extern int drawgdi_init(running_machine &machine, win_draw_callbacks *callbacks);
extern int drawdd_init(running_machine &machine, win_draw_callbacks *callbacks);
extern int drawd3d_init(running_machine &machine, win_draw_callbacks *callbacks);


//============================================================
//  PARAMETERS
//============================================================

// window styles
#define WINDOW_STYLE                    WS_OVERLAPPEDWINDOW
#define WINDOW_STYLE_EX                 0

// debugger window styles
#define DEBUG_WINDOW_STYLE              WS_OVERLAPPED
#define DEBUG_WINDOW_STYLE_EX           0

// full screen window styles
#define FULLSCREEN_STYLE                WS_POPUP
#define FULLSCREEN_STYLE_EX             WS_EX_TOPMOST

// minimum window dimension
#define MIN_WINDOW_DIM                  200

// custom window messages
#define WM_USER_FINISH_CREATE_WINDOW    (WM_USER + 0)
#define WM_USER_SELF_TERMINATE          (WM_USER + 1)
#define WM_USER_REDRAW                  (WM_USER + 2)
#define WM_USER_SET_FULLSCREEN          (WM_USER + 3)
#define WM_USER_SET_MAXSIZE             (WM_USER + 4)
#define WM_USER_SET_MINSIZE             (WM_USER + 5)
#define WM_USER_UI_TEMP_PAUSE           (WM_USER + 6)
#define WM_USER_EXEC_FUNC               (WM_USER + 7)



//============================================================
//  GLOBAL VARIABLES
//============================================================

// actual physical resolution
static int win_physical_width;
static int win_physical_height;



//============================================================
//  LOCAL VARIABLES
//============================================================

// event handling
static DWORD last_event_check;

// debugger
static int in_background;

static DWORD last_update_time;

static win_draw_callbacks draw;


//============================================================
//  PROTOTYPES
//============================================================

static void winwindow_video_window_destroy(win_window_info *window);
static void draw_video_contents(win_window_info *window, HDC dc, int update);

static unsigned __stdcall thread_entry(void *param);
static void create_window_class(void);
static void set_starting_view(int index, win_window_info *window, const char *view);

static void update_minmax_state(win_window_info *window);
static void minimize_window(win_window_info *window);
static void maximize_window(win_window_info *window);

static void adjust_window_position_after_major_change(win_window_info *window);
static void set_fullscreen(win_window_info *window, int fullscreen);


// temporary hacks
#if LOG_THREADS
struct mtlog
{
	osd_ticks_t timestamp;
	const char *event;
};

static mtlog mtlog[100000];
static volatile LONG mtlogindex;

void mtlog_add(const char *event)
{
	int index = atomic_increment32((LONG *) &mtlogindex) - 1;
	if (index < ARRAY_LENGTH(mtlog))
	{
		mtlog[index].timestamp = osd_ticks();
		mtlog[index].event = event;
	}
}

static void mtlog_dump(void)
{
	osd_ticks_t cps = osd_ticks_per_second();
	osd_ticks_t last = mtlog[0].timestamp * 1000000 / cps;
	int i;

	FILE *f = fopen("mt.log", "w");
	for (i = 0; i < mtlogindex; i++)
	{
		osd_ticks_t curr = mtlog[i].timestamp * 1000000 / cps;
		fprintf(f, "%20I64d %10I64d %s\n", curr, curr - last, mtlog[i].event);
		last = curr;
	}
	fclose(f);
}
#else
void mtlog_add(const char *event) { }
static void mtlog_dump(void) { }
#endif

//============================================================
//  winwindow_take_snap
//  (main thread)
//============================================================

void winwindow_take_snap(void)
{
	if (draw.window_record == NULL)
		return;

	win_window_info *window;

	assert(GetCurrentThreadId() == main_threadid);

	// iterate over windows and request a snap
	for (window = win_window_list; window != NULL; window = window->get_next())
	{
		(*draw.window_save)(window);
	}
}



//============================================================
//  winwindow_toggle_fsfx
//  (main thread)
//============================================================

void winwindow_toggle_fsfx(void)
{
	if (draw.window_toggle_fsfx == NULL)
		return;

	windows::window_info *window;

	assert(GetCurrentThreadId() == main_threadid);

	// iterate over windows and request a snap
	for (window = window_info::window_list(); window != NULL; window = window->next())
	{
		(*draw.window_toggle_fsfx)(window);
	}
}


//============================================================
//  winwindow_take_video
//  (main thread)
//============================================================

void winwindow_take_video(void)
{
	if (draw.window_record == NULL)
		return;

	win_window_info *window;

	assert(GetCurrentThreadId() == main_threadid);

	// iterate over windows and request a snap
	for (window = win_window_list; window != NULL; window = window->get_next())
	{
		(*draw.window_record)(window);
	}
}


//============================================================
//  winwindow_has_focus
//  (main or window thread)
//============================================================

BOOL winwindow_has_focus(void)
{
	HWND focuswnd = GetFocus();
	win_window_info *window;

	// see if one of the video windows has focus
	for (window = win_window_list; window != NULL; window = window->next())
		if (focuswnd == window->hwnd())
			return TRUE;

	return FALSE;
}

//============================================================
//  create_window_class
//  (main thread)
//============================================================

static void create_window_class(void)
{
	static int classes_created = FALSE;

	assert(GetCurrentThreadId() == main_threadid);

	if (!classes_created)
	{
		WNDCLASS wc = { 0 };

		// initialize the description of the window class
		wc.lpszClassName    = TEXT("MAME");
		wc.hInstance        = GetModuleHandle(NULL);
		wc.lpfnWndProc      = winwindow_video_window_proc_ui;
		wc.hCursor          = LoadCursor(NULL, IDC_ARROW);
		wc.hIcon            = LoadIcon(wc.hInstance, MAKEINTRESOURCE(2));

		// register the class; fail if we can't
		if (!RegisterClass(&wc))
			fatalerror("Failed to create window class\n");
		classes_created = TRUE;
	}
}



//============================================================
//  set_starting_view
//  (main thread)
//============================================================

static void set_starting_view(int index, win_window_info *window, const char *view)
{
	const char *defview = downcast<windows_options &>(window->machine().options()).view();
	int viewindex;

	assert(GetCurrentThreadId() == main_threadid);

	// choose non-auto over auto
	if (strcmp(view, "auto") == 0 && strcmp(defview, "auto") != 0)
		view = defview;

	// query the video system to help us pick a view
	viewindex = window->target->configured_view(view, index, video_config.numscreens);

	// set the view
	window->target->set_view(viewindex);
}


//============================================================
//  winwindow_ui_exec_on_main_thread
//  (window thread)
//============================================================

void winwindow_ui_exec_on_main_thread(void (*func)(void *), void *param)
{
	assert(GetCurrentThreadId() == window_threadid);

	// if we're multithreaded, we have to request a pause on the main thread
	if (multithreading_enabled)
	{
		// request a pause from the main thread
		PostThreadMessage(main_threadid, WM_USER_EXEC_FUNC, (WPARAM) func, (LPARAM) param);
	}

	// otherwise, we just do it directly
	else
		(*func)(param);
}



//============================================================
//  winwindow_ui_is_paused
//============================================================

int winwindow_ui_is_paused(running_machine &machine)
{
	return machine.paused() && ui_temp_was_paused;
}


//============================================================
//  thread_entry
//  (window thread)
//============================================================

static unsigned __stdcall thread_entry(void *param)
{
	MSG message;

	// make a bogus user call to make us a message thread
	PeekMessage(&message, NULL, 0, 0, PM_NOREMOVE);

	// attach our input to the main thread
	AttachThreadInput(main_threadid, window_threadid, TRUE);

	// signal to the main thread that we are ready to receive events
	SetEvent(window_thread_ready_event);

	// run the message pump
	while (GetMessage(&message, NULL, 0, 0))
	{
		int dispatch = TRUE;

		if ((message.hwnd == NULL) || is_mame_window(message.hwnd))
		{
			switch (message.message)
			{
				// ignore input messages here
				case WM_SYSKEYUP:
				case WM_SYSKEYDOWN:
					dispatch = FALSE;
					break;

				// forward mouse button downs to the input system
				case WM_LBUTTONDOWN:
					dispatch = !wininput_handle_mouse_button(0, TRUE, GET_X_LPARAM(message.lParam), GET_Y_LPARAM(message.lParam));
					break;

				case WM_RBUTTONDOWN:
					dispatch = !wininput_handle_mouse_button(1, TRUE, GET_X_LPARAM(message.lParam), GET_Y_LPARAM(message.lParam));
					break;

				case WM_MBUTTONDOWN:
					dispatch = !wininput_handle_mouse_button(2, TRUE, GET_X_LPARAM(message.lParam), GET_Y_LPARAM(message.lParam));
					break;

				case WM_XBUTTONDOWN:
					dispatch = !wininput_handle_mouse_button(3, TRUE, GET_X_LPARAM(message.lParam), GET_Y_LPARAM(message.lParam));
					break;

				// forward mouse button ups to the input system
				case WM_LBUTTONUP:
					dispatch = !wininput_handle_mouse_button(0, FALSE, GET_X_LPARAM(message.lParam), GET_Y_LPARAM(message.lParam));
					break;

				case WM_RBUTTONUP:
					dispatch = !wininput_handle_mouse_button(1, FALSE, GET_X_LPARAM(message.lParam), GET_Y_LPARAM(message.lParam));
					break;

				case WM_MBUTTONUP:
					dispatch = !wininput_handle_mouse_button(2, FALSE, GET_X_LPARAM(message.lParam), GET_Y_LPARAM(message.lParam));
					break;

				case WM_XBUTTONUP:
					dispatch = !wininput_handle_mouse_button(3, FALSE, GET_X_LPARAM(message.lParam), GET_Y_LPARAM(message.lParam));
					break;

				// a terminate message to the thread posts a quit
				case WM_USER_SELF_TERMINATE:
					PostQuitMessage(0);
					dispatch = FALSE;
					break;

				// handle the "complete create" message
				case WM_USER_FINISH_CREATE_WINDOW:
				{
					win_window_info *window = (win_window_info *)message.lParam;
					window->complete_create();
					dispatch = FALSE;
					break;
				}
			}
		}

		// dispatch if necessary
		if (dispatch)
		{
			TranslateMessage(&message);
			DispatchMessage(&message);
		}
	}
	return 0;
}

//============================================================
//  winwindow_video_window_proc
//  (window thread)
//============================================================

LRESULT CALLBACK winwindow_video_window_proc(HWND wnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	LONG_PTR ptr = GetWindowLongPtr(wnd, GWLP_USERDATA);
	windowns::window_info *window = (windows::window_info *)ptr;

	// we may get called before SetWindowLongPtr is called
	if (window != NULL)
	{
		assert(GetCurrentThreadId() == (DWORD)window->window_threadid());
		update_minmax_state(window);
	}

	// handle a few messages
	switch (message)
	{
		// paint: redraw the last bitmap
		case WM_PAINT:
		{
			PAINTSTRUCT pstruct;
			HDC hdc = BeginPaint(wnd, &pstruct);
			draw_video_contents(window, hdc, TRUE);
			if (window->has_menu())
				DrawMenuBar(window->hwnd());
			EndPaint(wnd, &pstruct);
			break;
		}

		// non-client paint: punt if full screen
		case WM_NCPAINT:
			if (!window->fullscreen() || window->has_menu())
				return DefWindowProc(wnd, message, wparam, lparam);
			break;

		// input: handle the raw input
		case WM_INPUT:
			wininput_handle_raw((HRAWINPUT)lparam);
			break;

		// syskeys - ignore
		case WM_SYSKEYUP:
		case WM_SYSKEYDOWN:
			break;

		// input events
		case WM_MOUSEMOVE:
			ui_input_push_mouse_move_event(window->machine(), window->target(), GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam));
			break;

		case WM_MOUSELEAVE:
			ui_input_push_mouse_leave_event(window->machine(), window->target());
			break;

		case WM_LBUTTONDOWN:
		{
			DWORD ticks = GetTickCount();
			ui_input_push_mouse_down_event(window->machine(), window->target(), GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam));

			// check for a double-click
			if (ticks - window->lastclicktime() < GetDoubleClickTime() &&
				GET_X_LPARAM(lparam) >= window->lastclickx() - 4 && GET_X_LPARAM(lparam) <= window->lastclickx() + 4 &&
				GET_Y_LPARAM(lparam) >= window->lastclicky() - 4 && GET_Y_LPARAM(lparam) <= window->lastclicky() + 4)
			{
				window->set_lastclicktime(0);
				ui_input_push_mouse_double_click_event(window->machine(), window->target(), GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam));
			}
			else
			{
				window->set_lastclicktime(ticks);
				window->set_lastclickx(GET_X_LPARAM(lparam));
				window->set_lastclicky(GET_Y_LPARAM(lparam));
			}
			break;
		}

		case WM_LBUTTONUP:
			ui_input_push_mouse_up_event(window->machine(), window->target(), GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam));
			break;

		case WM_CHAR:
			ui_input_push_char_event(window->machine(), window->target(), (unicode_char) wparam);
			break;

		// pause the system when we start a menu or resize
		case WM_ENTERSIZEMOVE:
			window->set_resize_state(RESIZE_STATE_RESIZING);
		case WM_ENTERMENULOOP:
			winwindow_ui_pause_from_window_thread(window->machine(), TRUE);
			break;

		// unpause the system when we stop a menu or resize and force a redraw
		case WM_EXITSIZEMOVE:
			window->set_resize_state(RESIZE_STATE_PENDING);
		case WM_EXITMENULOOP:
			winwindow_ui_pause_from_window_thread(window->machine(), FALSE);
			InvalidateRect(wnd, NULL, FALSE);
			break;

		// get min/max info: set the minimum window size
		case WM_GETMINMAXINFO:
		{
			MINMAXINFO *minmax = (MINMAXINFO *)lparam;
			minmax->ptMinTrackSize.x = MIN_WINDOW_DIM;
			minmax->ptMinTrackSize.y = MIN_WINDOW_DIM;
			break;
		}

		// sizing: constrain to the aspect ratio unless control key is held down
		case WM_SIZING:
		{
			RECT *rect = (RECT *)lparam;
			if (video_config.keepaspect && !(GetAsyncKeyState(VK_CONTROL) & 0x8000))
				constrain_to_aspect_ratio(window, rect, wparam);
			InvalidateRect(wnd, NULL, FALSE);
			break;
		}

		// syscommands: catch win_start_maximized
		case WM_SYSCOMMAND:
		{
			// prevent screensaver or monitor power events
			if (wparam == SC_MONITORPOWER || wparam == SC_SCREENSAVE)
				return 1;

			// most SYSCOMMANDs require us to invalidate the window
			InvalidateRect(wnd, NULL, FALSE);

			// handle maximize
			if ((wparam & 0xfff0) == SC_MAXIMIZE)
			{
				update_minmax_state(window);
				if (window->ismaximized)
					minimize_window(window);
				else
					maximize_window(window);
				break;
			}
			return DefWindowProc(wnd, message, wparam, lparam);
		}

		// track whether we are in the foreground
		case WM_ACTIVATEAPP:
			in_background = !wparam;
			break;

		// close: cause MAME to exit
		case WM_CLOSE:
			if (multithreading_enabled)
				PostThreadMessage(main_threadid, WM_QUIT, 0, 0);
			else
				window->machine().schedule_exit();
			break;

		// destroy: clean up all attached rendering bits and NULL out our hwnd
		case WM_DESTROY:
			(*draw.window_destroy)(window);
			window->hwnd = NULL;
			return DefWindowProc(wnd, message, wparam, lparam);

		// self redraw: draw ourself in a non-painty way
		case WM_USER_REDRAW:
		{
			HDC hdc = GetDC(wnd);

			mtlog_add("winwindow_video_window_proc: WM_USER_REDRAW begin");
			window->primlist = (render_primitive_list *)lparam;
			draw_video_contents(window, hdc, FALSE);
			mtlog_add("winwindow_video_window_proc: WM_USER_REDRAW end");

			ReleaseDC(wnd, hdc);
			break;
		}

		// self destruct
		case WM_USER_SELF_TERMINATE:
			DestroyWindow(window->hwnd);
			break;

		// fullscreen set
		case WM_USER_SET_FULLSCREEN:
			window->set_fullscreen(wparam);
			break;

		// minimum size set
		case WM_USER_SET_MINSIZE:
			window->minimize_window();
			break;

		// maximum size set
		case WM_USER_SET_MAXSIZE:
			window->maximize_window();
			break;

		// set focus: if we're not the primary window, switch back
		// commented out ATM because this prevents us from resizing secondary windows
//      case WM_SETFOCUS:
//          if (window != win_window_list && win_window_list != NULL)
//              SetFocus(win_window_list->hwnd);
//          break;

		// everything else: defaults
		default:
			return DefWindowProc(wnd, message, wparam, lparam);
	}

	return 0;
}



//============================================================
//  draw_video_contents
//  (window thread)
//============================================================

static void draw_video_contents(win_window_info *window, HDC dc, int update)
{
	assert(GetCurrentThreadId() == window_threadid);

	mtlog_add("draw_video_contents: begin");

	mtlog_add("draw_video_contents: render lock acquire");
	osd_lock_acquire(window->render_lock);
	mtlog_add("draw_video_contents: render lock acquired");

	// if we're iconic, don't bother
	if (window->hwnd != NULL && !IsIconic(window->hwnd))
	{
		// if no bitmap, just fill
		if (window->primlist == NULL)
		{
			RECT fill;
			GetClientRect(window->hwnd, &fill);
			FillRect(dc, &fill, (HBRUSH)GetStockObject(BLACK_BRUSH));
		}

		// otherwise, render with our drawing system
		else
		{
			(*draw.window_draw)(window, dc, update);
			mtlog_add("draw_video_contents: drawing finished");
		}
	}

	osd_lock_release(window->render_lock);
	mtlog_add("draw_video_contents: render lock released");

	mtlog_add("draw_video_contents: end");
}


//============================================================
//  update_minmax_state
//  (window thread)
//============================================================

static void update_minmax_state(win_window_info *window)
{
	assert(GetCurrentThreadId() == window_threadid);

	if (!window->fullscreen)
	{
		RECT bounds, minbounds, maxbounds;

		// compare the maximum bounds versus the current bounds
		get_min_bounds(window, &minbounds, video_config.keepaspect);
		get_max_bounds(window, &maxbounds, video_config.keepaspect);
		GetWindowRect(window->hwnd, &bounds);

		// if either the width or height matches, we were maximized
		window->isminimized = (rect_width(&bounds) == rect_width(&minbounds) ||
								rect_height(&bounds) == rect_height(&minbounds));
		window->ismaximized = (rect_width(&bounds) == rect_width(&maxbounds) ||
								rect_height(&bounds) == rect_height(&maxbounds));
	}
	else
	{
		window->isminimized = FALSE;
		window->ismaximized = TRUE;
	}
}
