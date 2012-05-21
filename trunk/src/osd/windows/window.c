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

#define LOG_THREADS			0
#define LOG_TEMP_PAUSE		0

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
#define WINDOW_STYLE					WS_OVERLAPPEDWINDOW
#define WINDOW_STYLE_EX					0

// debugger window styles
#define DEBUG_WINDOW_STYLE				WS_OVERLAPPED
#define DEBUG_WINDOW_STYLE_EX			0

// full screen window styles
#define FULLSCREEN_STYLE				WS_POPUP
#define FULLSCREEN_STYLE_EX				WS_EX_TOPMOST

// minimum window dimension
#define MIN_WINDOW_DIM					200

// custom window messages
#define WM_USER_FINISH_CREATE_WINDOW	(WM_USER + 0)
#define WM_USER_SELF_TERMINATE			(WM_USER + 1)
#define WM_USER_REDRAW					(WM_USER + 2)
#define WM_USER_SET_FULLSCREEN			(WM_USER + 3)
#define WM_USER_SET_MAXSIZE				(WM_USER + 4)
#define WM_USER_SET_MINSIZE				(WM_USER + 5)
#define WM_USER_UI_TEMP_PAUSE			(WM_USER + 6)
#define WM_USER_EXEC_FUNC				(WM_USER + 7)



//============================================================
//  GLOBAL VARIABLES
//============================================================

win_window_info *win_window_list;
static win_window_info **last_window_ptr;
static DWORD main_threadid;

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

static int ui_temp_pause;
static int ui_temp_was_paused;

static int multithreading_enabled;

static HANDLE window_thread;
static DWORD window_threadid;

static DWORD last_update_time;

static win_draw_callbacks draw;

static HANDLE ui_pause_event;
static HANDLE window_thread_ready_event;



//============================================================
//  PROTOTYPES
//============================================================

static void winwindow_exit(running_machine &machine);
static void winwindow_video_window_destroy(win_window_info *window);
static void draw_video_contents(win_window_info *window, HDC dc, int update);

static unsigned __stdcall thread_entry(void *param);
static int complete_create(win_window_info *window);
static void create_window_class(void);
static void set_starting_view(int index, win_window_info *window, const char *view);

static void constrain_to_aspect_ratio(win_window_info *window, RECT *rect, int adjustment);
static void get_min_bounds(win_window_info *window, RECT *bounds, int constrain);
static void get_max_bounds(win_window_info *window, RECT *bounds, int constrain);
static void update_minmax_state(win_window_info *window);
static void minimize_window(win_window_info *window);
static void maximize_window(win_window_info *window);

static void adjust_window_position_after_major_change(win_window_info *window);
static void set_fullscreen(win_window_info *window, int fullscreen);


// temporary hacks
#if LOG_THREADS
struct _mtlog
{
	osd_ticks_t	timestamp;
	const char *event;
};

static struct _mtlog mtlog[100000];
static volatile LONG mtlogindex;

void mtlog_add(const char *event)
{
	int index = InterlockedIncrement((LONG *) &mtlogindex) - 1;
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
//  winwindow_init
//  (main thread)
//============================================================

void winwindow_init(running_machine &machine)
{
	size_t temp;

	// determine if we are using multithreading or not
	multithreading_enabled = downcast<windows_options &>(machine.options()).multithreading();

	// get the main thread ID before anything else
	main_threadid = GetCurrentThreadId();

	// ensure we get called on the way out
	machine.add_notifier(MACHINE_NOTIFY_EXIT, machine_notify_delegate(FUNC(winwindow_exit), &machine));

	// set up window class and register it
	create_window_class();

	// create an event to signal UI pausing
	ui_pause_event = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (!ui_pause_event)
		fatalerror("Failed to create pause event");

	// if multithreading, create a thread to run the windows
	if (multithreading_enabled)
	{
		// create an event to signal when the window thread is ready
		window_thread_ready_event = CreateEvent(NULL, TRUE, FALSE, NULL);
		if (!window_thread_ready_event)
			fatalerror("Failed to create window thread ready event");

		// create a thread to run the windows from
		temp = _beginthreadex(NULL, 0, thread_entry, NULL, 0, (unsigned *)&window_threadid);
		window_thread = (HANDLE)temp;
		if (window_thread == NULL)
			fatalerror("Failed to create window thread");

		// set the thread priority equal to the main MAME thread
		SetThreadPriority(window_thread, GetThreadPriority(GetCurrentThread()));
	}

	// otherwise, treat the window thread as the main thread
	else
	{
		window_thread = GetCurrentThread();
		window_threadid = main_threadid;
	}

	// initialize the drawers
	if (video_config.mode == VIDEO_MODE_D3D)
	{
		if (drawd3d_init(machine, &draw))
			video_config.mode = VIDEO_MODE_GDI;
	}
	if (video_config.mode == VIDEO_MODE_DDRAW)
	{
		if (drawdd_init(machine, &draw))
			video_config.mode = VIDEO_MODE_GDI;
	}
	if (video_config.mode == VIDEO_MODE_GDI)
		drawgdi_init(machine, &draw);
	if (video_config.mode == VIDEO_MODE_NONE)
		drawnone_init(machine, &draw);

	// set up the window list
	last_window_ptr = &win_window_list;
}



//============================================================
//  winwindow_exit
//  (main thread)
//============================================================

static void winwindow_exit(running_machine &machine)
{
	assert(GetCurrentThreadId() == main_threadid);

	// possibly kill the debug window
	if (machine.debug_flags & DEBUG_FLAG_OSD_ENABLED)
		debugwin_destroy_windows();

	// free all the windows
	while (win_window_list != NULL)
	{
		win_window_info *temp = win_window_list;
		win_window_list = temp->next;
		winwindow_video_window_destroy(temp);
	}

	// kill the drawers
	(*draw.exit)();

	// if we're multithreaded, clean up the window thread
	if (multithreading_enabled)
	{
		PostThreadMessage(window_threadid, WM_USER_SELF_TERMINATE, 0, 0);
		WaitForSingleObject(window_thread, INFINITE);

		mtlog_dump();
	}

	// kill the UI pause event
	if (ui_pause_event)
		CloseHandle(ui_pause_event);

	// kill the window thread ready event
	if (window_thread_ready_event)
		CloseHandle(window_thread_ready_event);

	// if we hid the cursor during the emulation, show it
	while (ShowCursor(TRUE) < 0) ;
}



//============================================================
//  winwindow_process_events_periodic
//  (main thread)
//============================================================

void winwindow_process_events_periodic(running_machine &machine)
{
	DWORD currticks = GetTickCount();

	assert(GetCurrentThreadId() == main_threadid);

	// update once every 1/8th of a second
	if (currticks - last_event_check < 1000 / 8)
		return;
	winwindow_process_events(machine, TRUE);
}



//============================================================
//  is_mame_window
//============================================================

static BOOL is_mame_window(HWND hwnd)
{
	win_window_info *window;

	for (window = win_window_list; window != NULL; window = window->next)
		if (window->hwnd == hwnd)
			return TRUE;

	return FALSE;
}



//============================================================
//  winwindow_process_events
//  (main thread)
//============================================================

void winwindow_process_events(running_machine &machine, int ingame)
{
	MSG message;

	assert(GetCurrentThreadId() == main_threadid);

	// if we're running, disable some parts of the debugger
	if (ingame && (machine.debug_flags & DEBUG_FLAG_OSD_ENABLED) != 0)
		debugwin_update_during_game(machine);

	// remember the last time we did this
	last_event_check = GetTickCount();

	do
	{
		// if we are paused, lets wait for a message
		if (ui_temp_pause > 0)
			WaitMessage();

		// loop over all messages in the queue
		while (PeekMessage(&message, NULL, 0, 0, PM_REMOVE))
		{
			int dispatch = TRUE;

			if (message.hwnd == NULL || is_mame_window(message.hwnd))
			{
				switch (message.message)
				{
					// ignore keyboard messages
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
				}
			}

			// dispatch if necessary
			if (dispatch)
				winwindow_dispatch_message(machine, &message);
		}
	} while (ui_temp_pause > 0);

	// update the cursor state after processing events
	winwindow_update_cursor_state(machine);
}



//============================================================
//  winwindow_dispatch_message
//  (main thread)
//============================================================

void winwindow_dispatch_message(running_machine &machine, MSG *message)
{
	assert(GetCurrentThreadId() == main_threadid);

	// dispatch our special communication messages
	switch (message->message)
	{
		// special case for quit
		case WM_QUIT:
			machine.schedule_exit();
			break;

		// temporary pause from the window thread
		case WM_USER_UI_TEMP_PAUSE:
			winwindow_ui_pause_from_main_thread(machine, message->wParam);
			break;

		// execute arbitrary function
		case WM_USER_EXEC_FUNC:
			{
				void (*func)(void *) = (void (*)(void *)) message->wParam;
				void *param = (void *) message->lParam;
				func(param);
			}
			break;

		// everything else dispatches normally
		default:
			TranslateMessage(message);
			DispatchMessage(message);
			break;
	}
}



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
	for (window = win_window_list; window != NULL; window = window->next)
	{
		(*draw.window_save)(window);
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
	for (window = win_window_list; window != NULL; window = window->next)
	{
		(*draw.window_record)(window);
	}
}



//============================================================
//  winwindow_toggle_full_screen
//  (main thread)
//============================================================

void winwindow_toggle_full_screen(void)
{
	win_window_info *window;

	assert(GetCurrentThreadId() == main_threadid);

	// if we are in debug mode, never go full screen
	for (window = win_window_list; window != NULL; window = window->next)
		if (window->machine().debug_flags & DEBUG_FLAG_OSD_ENABLED)
			return;

	// toggle the window mode
	video_config.windowed = !video_config.windowed;

	// iterate over windows and toggle their fullscreen state
	for (window = win_window_list; window != NULL; window = window->next)
		SendMessage(window->hwnd, WM_USER_SET_FULLSCREEN, !video_config.windowed, 0);
	SetForegroundWindow(win_window_list->hwnd);
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
	for (window = win_window_list; window != NULL; window = window->next)
		if (focuswnd == window->hwnd)
			return TRUE;

	return FALSE;
}


//============================================================
//  winwindow_update_cursor_state
//  (main thread)
//============================================================

void winwindow_update_cursor_state(running_machine &machine)
{
	static POINT saved_cursor_pos = { -1, -1 };

	assert(GetCurrentThreadId() == main_threadid);

	// if we should hide the mouse cursor, then do it
	// rules are:
	//   1. we must have focus before hiding the cursor
	//   2. we also hide the cursor in full screen mode and when the window doesn't have a menu
	//   3. we also hide the cursor in windowed mode if we're not paused and
	//      the input system requests it
	if (downcast<windows_options &>(machine.options()).hide_cursor() &&
		winwindow_has_focus() && ((!video_config.windowed && !win_has_menu(win_window_list)) || (!machine.paused() && wininput_should_hide_mouse())))
	{
		win_window_info *window = win_window_list;
		RECT bounds;

		// hide cursor
		while (ShowCursor(FALSE) >= -1) ;
		ShowCursor(TRUE);

		// store the cursor position
		GetCursorPos(&saved_cursor_pos);

		// clip cursor to game video window
		GetClientRect(window->hwnd, &bounds);
		ClientToScreen(window->hwnd, &((POINT *)&bounds)[0]);
		ClientToScreen(window->hwnd, &((POINT *)&bounds)[1]);
		ClipCursor(&bounds);
	}
	else
	{
		// show cursor
		while (ShowCursor(TRUE) < 1) ;
		ShowCursor(FALSE);

		// allow cursor to move freely
		ClipCursor(NULL);
		if (saved_cursor_pos.x != -1 || saved_cursor_pos.y != -1)
		{
			SetCursorPos(saved_cursor_pos.x, saved_cursor_pos.y);
			saved_cursor_pos.x = saved_cursor_pos.y = -1;
		}
	}
}



//============================================================
//  winwindow_video_window_create
//  (main thread)
//============================================================

void winwindow_video_window_create(running_machine &machine, int index, win_monitor_info *monitor, const win_window_config *config)
{
	win_window_info *window, *win;

	assert(GetCurrentThreadId() == main_threadid);

	// allocate a new window object
	window = global_alloc_clear(win_window_info(machine));
	window->maxwidth = config->width;
	window->maxheight = config->height;
	window->refresh = config->refresh;
	window->monitor = monitor;
	window->fullscreen = !video_config.windowed;

	// see if we are safe for fullscreen
	window->fullscreen_safe = TRUE;
	for (win = win_window_list; win != NULL; win = win->next)
		if (win->monitor == monitor)
			window->fullscreen_safe = FALSE;

	// add us to the list
	*last_window_ptr = window;
	last_window_ptr = &window->next;

	// create a lock that we can use to skip blitting
	window->render_lock = osd_lock_alloc();

	// load the layout
	window->target = machine.render().target_alloc();

	// set the specific view
	windows_options &options = downcast<windows_options &>(machine.options());
	set_starting_view(index, window, options.view(index));

	// remember the current values in case they change
	window->targetview = window->target->view();
	window->targetorient = window->target->orientation();
	window->targetlayerconfig = window->target->layer_config();

	// make the window title
	if (video_config.numscreens == 1)
		sprintf(window->title, "%s: %s [%s]", emulator_info::get_appname(), machine.system().description, machine.system().name);
	else
		sprintf(window->title, "%s: %s [%s] - Screen %d", emulator_info::get_appname(), machine.system().description, machine.system().name, index);

	// set the initial maximized state
	window->startmaximized = options.maximize();

	// finish the window creation on the window thread
	if (multithreading_enabled)
	{
		// wait until the window thread is ready to respond to events
		WaitForSingleObject(window_thread_ready_event, INFINITE);

		PostThreadMessage(window_threadid, WM_USER_FINISH_CREATE_WINDOW, 0, (LPARAM)window);
		while (window->init_state == 0)
			Sleep(1);
	}
	else
		window->init_state = complete_create(window) ? -1 : 1;

	// handle error conditions
	if (window->init_state == -1)
		fatalerror("Unable to complete window creation");
}



//============================================================
//  winwindow_video_window_destroy
//  (main thread)
//============================================================

static void winwindow_video_window_destroy(win_window_info *window)
{
	win_window_info **prevptr;

	assert(GetCurrentThreadId() == main_threadid);

	// remove us from the list
	for (prevptr = &win_window_list; *prevptr != NULL; prevptr = &(*prevptr)->next)
		if (*prevptr == window)
		{
			*prevptr = window->next;
			break;
		}

	// destroy the window
	if (window->hwnd != NULL)
		SendMessage(window->hwnd, WM_USER_SELF_TERMINATE, 0, 0);

	// free the render target
	window->machine().render().target_free(window->target);

	// free the lock
	osd_lock_free(window->render_lock);

	// free the window itself
	global_free(window);
}



//============================================================
//  winwindow_video_window_update
//  (main thread)
//============================================================

void winwindow_video_window_update(win_window_info *window)
{
	int targetview, targetorient;
	render_layer_config targetlayerconfig;

	assert(GetCurrentThreadId() == main_threadid);

	mtlog_add("winwindow_video_window_update: begin");

	// see if the target has changed significantly in window mode
	targetview = window->target->view();
	targetorient = window->target->orientation();
	targetlayerconfig = window->target->layer_config();
	if (targetview != window->targetview || targetorient != window->targetorient || targetlayerconfig != window->targetlayerconfig)
	{
		window->targetview = targetview;
		window->targetorient = targetorient;
		window->targetlayerconfig = targetlayerconfig;

		// in window mode, reminimize/maximize
		if (!window->fullscreen)
		{
			if (window->isminimized)
				SendMessage(window->hwnd, WM_USER_SET_MINSIZE, 0, 0);
			if (window->ismaximized)
				SendMessage(window->hwnd, WM_USER_SET_MAXSIZE, 0, 0);
		}
	}

	// if we're visible and running and not in the middle of a resize, draw
	if (window->hwnd != NULL && window->target != NULL && window->drawdata != NULL)
	{
		int got_lock = TRUE;

		mtlog_add("winwindow_video_window_update: try lock");

		// only block if we're throttled
		if (window->machine().video().throttled() || timeGetTime() - last_update_time > 250)
			osd_lock_acquire(window->render_lock);
		else
			got_lock = osd_lock_try(window->render_lock);

		// only render if we were able to get the lock
		if (got_lock)
		{
			render_primitive_list *primlist;

			mtlog_add("winwindow_video_window_update: got lock");

			// don't hold the lock; we just used it to see if rendering was still happening
			osd_lock_release(window->render_lock);

			// ensure the target bounds are up-to-date, and then get the primitives
			primlist = (*draw.window_get_primitives)(window);

			// post a redraw request with the primitive list as a parameter
			last_update_time = timeGetTime();
			mtlog_add("winwindow_video_window_update: PostMessage start");
			if (multithreading_enabled)
				PostMessage(window->hwnd, WM_USER_REDRAW, 0, (LPARAM)primlist);
			else
				SendMessage(window->hwnd, WM_USER_REDRAW, 0, (LPARAM)primlist);
			mtlog_add("winwindow_video_window_update: PostMessage end");
		}
	}

	mtlog_add("winwindow_video_window_update: end");
}



//============================================================
//  winwindow_video_window_monitor
//  (window thread)
//============================================================

win_monitor_info *winwindow_video_window_monitor(win_window_info *window, const RECT *proposed)
{
	win_monitor_info *monitor;

	// in window mode, find the nearest
	if (!window->fullscreen)
	{
		if (proposed != NULL)
			monitor = winvideo_monitor_from_handle(MonitorFromRect(proposed, MONITOR_DEFAULTTONEAREST));
		else
			monitor = winvideo_monitor_from_handle(MonitorFromWindow(window->hwnd, MONITOR_DEFAULTTONEAREST));
	}

	// in full screen, just use the configured monitor
	else
		monitor = window->monitor;

	// make sure we're up-to-date
	winvideo_monitor_refresh(monitor);
	return monitor;
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
		wc.lpszClassName	= TEXT("MAME");
		wc.hInstance		= GetModuleHandle(NULL);
		wc.lpfnWndProc		= winwindow_video_window_proc_ui;
		wc.hCursor			= LoadCursor(NULL, IDC_ARROW);
		wc.hIcon			= LoadIcon(wc.hInstance, MAKEINTRESOURCE(2));

		// register the class; fail if we can't
		if (!RegisterClass(&wc))
			fatalerror("Failed to create window class");
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
//  winwindow_ui_pause_from_main_thread
//  (main thread)
//============================================================

void winwindow_ui_pause_from_main_thread(running_machine &machine, int pause)
{
	int old_temp_pause = ui_temp_pause;

	assert(GetCurrentThreadId() == main_threadid);

	// if we're pausing, increment the pause counter
	if (pause)
	{
		// if we're the first to pause, we have to actually initiate it
		if (ui_temp_pause++ == 0)
		{
			// only call mame_pause if we weren't already paused due to some external reason
			ui_temp_was_paused = machine.paused();
			if (!ui_temp_was_paused)
				machine.pause();

			SetEvent(ui_pause_event);
		}
	}

	// if we're resuming, decrement the pause counter
	else
	{
		// if we're the last to resume, unpause MAME
		if (--ui_temp_pause == 0)
		{
			// but only do it if we were the ones who initiated it
			if (!ui_temp_was_paused)
				machine.resume();

			ResetEvent(ui_pause_event);
		}
	}

	if (LOG_TEMP_PAUSE)
		logerror("winwindow_ui_pause_from_main_thread(): %d --> %d\n", old_temp_pause, ui_temp_pause);
}



//============================================================
//  winwindow_ui_pause_from_window_thread
//  (window thread)
//============================================================

void winwindow_ui_pause_from_window_thread(running_machine &machine, int pause)
{
	assert(GetCurrentThreadId() == window_threadid);

	// if we're multithreaded, we have to request a pause on the main thread
	if (multithreading_enabled)
	{
		// request a pause from the main thread
		PostThreadMessage(main_threadid, WM_USER_UI_TEMP_PAUSE, pause, 0);

		// if we're pausing, block until it happens
		if (pause)
			WaitForSingleObject(ui_pause_event, INFINITE);
	}

	// otherwise, we just do it directly
	else
		winwindow_ui_pause_from_main_thread(machine, pause);
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
//  wnd_extra_width
//  (window thread)
//============================================================

INLINE int wnd_extra_width(win_window_info *window)
{
	RECT temprect = { 100, 100, 200, 200 };
	if (window->fullscreen)
		return 0;
	AdjustWindowRectEx(&temprect, WINDOW_STYLE, win_has_menu(window), WINDOW_STYLE_EX);
	return rect_width(&temprect) - 100;
}



//============================================================
//  wnd_extra_height
//  (window thread)
//============================================================

INLINE int wnd_extra_height(win_window_info *window)
{
	RECT temprect = { 100, 100, 200, 200 };
	if (window->fullscreen)
		return 0;
	AdjustWindowRectEx(&temprect, WINDOW_STYLE, win_has_menu(window), WINDOW_STYLE_EX);
	return rect_height(&temprect) - 100;
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
					window->init_state = complete_create(window) ? -1 : 1;
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
//  complete_create
//  (window thread)
//============================================================

static int complete_create(win_window_info *window)
{
	RECT monitorbounds, client;
	int tempwidth, tempheight;
	HMENU menu = NULL;
	HDC dc;

	assert(GetCurrentThreadId() == window_threadid);

	// get the monitor bounds
	monitorbounds = window->monitor->info.rcMonitor;

	// create the window menu if needed
	if (downcast<windows_options &>(window->machine().options()).menu())
	{
		if (win_create_menu(window->machine(), &menu))
			return 1;
	}

	// create the window, but don't show it yet
	window->hwnd = win_create_window_ex_utf8(
						window->fullscreen ? FULLSCREEN_STYLE_EX : WINDOW_STYLE_EX,
						"MAME",
						window->title,
						window->fullscreen ? FULLSCREEN_STYLE : WINDOW_STYLE,
						monitorbounds.left + 20, monitorbounds.top + 20,
						monitorbounds.left + 100, monitorbounds.top + 100,
						NULL,//(win_window_list != NULL) ? win_window_list->hwnd : NULL,
						menu,
						GetModuleHandle(NULL),
						NULL);
	if (window->hwnd == NULL)
		return 1;

	// set a pointer back to us
	SetWindowLongPtr(window->hwnd, GWLP_USERDATA, (LONG_PTR)window);

	// skip the positioning stuff for -video none */
	if (video_config.mode == VIDEO_MODE_NONE)
		return 0;

	// adjust the window position to the initial width/height
	tempwidth = (window->maxwidth != 0) ? window->maxwidth : 640;
	tempheight = (window->maxheight != 0) ? window->maxheight : 480;
	SetWindowPos(window->hwnd, NULL, monitorbounds.left + 20, monitorbounds.top + 20,
			monitorbounds.left + tempwidth + wnd_extra_width(window),
			monitorbounds.top + tempheight + wnd_extra_height(window),
			SWP_NOZORDER);

	// maximum or minimize as appropriate
	if (window->startmaximized)
		maximize_window(window);
	else
		minimize_window(window);
	adjust_window_position_after_major_change(window);

	// show the window
	if (!window->fullscreen || window->fullscreen_safe)
	{
		// finish off by trying to initialize DirectX; if we fail, ignore it
		if ((*draw.window_init)(window))
			return 1;
		ShowWindow(window->hwnd, SW_SHOW);
	}

	// clear the window
	dc = GetDC(window->hwnd);
	GetClientRect(window->hwnd, &client);
	FillRect(dc, &client, (HBRUSH)GetStockObject(BLACK_BRUSH));
	ReleaseDC(window->hwnd, dc);
	return 0;
}



//============================================================
//  winwindow_video_window_proc
//  (window thread)
//============================================================

LRESULT CALLBACK winwindow_video_window_proc(HWND wnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	LONG_PTR ptr = GetWindowLongPtr(wnd, GWLP_USERDATA);
	win_window_info *window = (win_window_info *)ptr;

	// we may get called before SetWindowLongPtr is called
	if (window != NULL)
	{
		assert(GetCurrentThreadId() == window_threadid);
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
			if (win_has_menu(window))
				DrawMenuBar(window->hwnd);
			EndPaint(wnd, &pstruct);
			break;
		}

		// non-client paint: punt if full screen
		case WM_NCPAINT:
			if (!window->fullscreen || win_has_menu(window))
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
			ui_input_push_mouse_move_event(window->machine(), window->target, GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam));
			break;

		case WM_MOUSELEAVE:
			ui_input_push_mouse_leave_event(window->machine(), window->target);
			break;

		case WM_LBUTTONDOWN:
		{
			DWORD ticks = GetTickCount();
			ui_input_push_mouse_down_event(window->machine(), window->target, GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam));

			// check for a double-click
			if (ticks - window->lastclicktime < GetDoubleClickTime() &&
				GET_X_LPARAM(lparam) >= window->lastclickx - 4 && GET_X_LPARAM(lparam) <= window->lastclickx + 4 &&
				GET_Y_LPARAM(lparam) >= window->lastclicky - 4 && GET_Y_LPARAM(lparam) <= window->lastclicky + 4)
			{
				window->lastclicktime = 0;
				ui_input_push_mouse_double_click_event(window->machine(), window->target, GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam));
			}
			else
			{
				window->lastclicktime = ticks;
				window->lastclickx = GET_X_LPARAM(lparam);
				window->lastclicky = GET_Y_LPARAM(lparam);
			}
			break;
		}

		case WM_LBUTTONUP:
			ui_input_push_mouse_up_event(window->machine(), window->target, GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam));
			break;

		case WM_CHAR:
			ui_input_push_char_event(window->machine(), window->target, (unicode_char) wparam);
			break;

		// pause the system when we start a menu or resize
		case WM_ENTERSIZEMOVE:
			window->resize_state = RESIZE_STATE_RESIZING;
		case WM_ENTERMENULOOP:
			winwindow_ui_pause_from_window_thread(window->machine(), TRUE);
			break;

		// unpause the system when we stop a menu or resize and force a redraw
		case WM_EXITSIZEMOVE:
			window->resize_state = RESIZE_STATE_PENDING;
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
			set_fullscreen(window, wparam);
			break;

		// minimum size set
		case WM_USER_SET_MINSIZE:
			minimize_window(window);
			break;

		// maximum size set
		case WM_USER_SET_MAXSIZE:
			maximize_window(window);
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
//  constrain_to_aspect_ratio
//  (window thread)
//============================================================

static void constrain_to_aspect_ratio(win_window_info *window, RECT *rect, int adjustment)
{
	win_monitor_info *monitor = winwindow_video_window_monitor(window, rect);
	INT32 extrawidth = wnd_extra_width(window);
	INT32 extraheight = wnd_extra_height(window);
	INT32 propwidth, propheight;
	INT32 minwidth, minheight;
	INT32 maxwidth, maxheight;
	INT32 viswidth, visheight;
	INT32 adjwidth, adjheight;
	float pixel_aspect;

	assert(GetCurrentThreadId() == window_threadid);

	// get the pixel aspect ratio for the target monitor
	pixel_aspect = winvideo_monitor_get_aspect(monitor);

	// determine the proposed width/height
	propwidth = rect_width(rect) - extrawidth;
	propheight = rect_height(rect) - extraheight;

	// based on which edge we are adjusting, take either the width, height, or both as gospel
	// and scale to fit using that as our parameter
	switch (adjustment)
	{
		case WMSZ_BOTTOM:
		case WMSZ_TOP:
			window->target->compute_visible_area(10000, propheight, pixel_aspect, window->target->orientation(), propwidth, propheight);
			break;

		case WMSZ_LEFT:
		case WMSZ_RIGHT:
			window->target->compute_visible_area(propwidth, 10000, pixel_aspect, window->target->orientation(), propwidth, propheight);
			break;

		default:
			window->target->compute_visible_area(propwidth, propheight, pixel_aspect, window->target->orientation(), propwidth, propheight);
			break;
	}

	// get the minimum width/height for the current layout
	window->target->compute_minimum_size(minwidth, minheight);

	// clamp against the absolute minimum
	propwidth = MAX(propwidth, MIN_WINDOW_DIM);
	propheight = MAX(propheight, MIN_WINDOW_DIM);

	// clamp against the minimum width and height
	propwidth = MAX(propwidth, minwidth);
	propheight = MAX(propheight, minheight);

	// clamp against the maximum (fit on one screen for full screen mode)
	if (window->fullscreen)
	{
		maxwidth = rect_width(&monitor->info.rcMonitor) - extrawidth;
		maxheight = rect_height(&monitor->info.rcMonitor) - extraheight;
	}
	else
	{
		maxwidth = rect_width(&monitor->info.rcWork) - extrawidth;
		maxheight = rect_height(&monitor->info.rcWork) - extraheight;

		// further clamp to the maximum width/height in the window
		if (window->maxwidth != 0)
			maxwidth = MIN(maxwidth, window->maxwidth + extrawidth);
		if (window->maxheight != 0)
			maxheight = MIN(maxheight, window->maxheight + extraheight);
	}

	// clamp to the maximum
	propwidth = MIN(propwidth, maxwidth);
	propheight = MIN(propheight, maxheight);

	// compute the visible area based on the proposed rectangle
	window->target->compute_visible_area(propwidth, propheight, pixel_aspect, window->target->orientation(), viswidth, visheight);

	// compute the adjustments we need to make
	adjwidth = (viswidth + extrawidth) - rect_width(rect);
	adjheight = (visheight + extraheight) - rect_height(rect);

	// based on which corner we're adjusting, constrain in different ways
	switch (adjustment)
	{
		case WMSZ_BOTTOM:
		case WMSZ_BOTTOMRIGHT:
		case WMSZ_RIGHT:
			rect->right += adjwidth;
			rect->bottom += adjheight;
			break;

		case WMSZ_BOTTOMLEFT:
			rect->left -= adjwidth;
			rect->bottom += adjheight;
			break;

		case WMSZ_LEFT:
		case WMSZ_TOPLEFT:
		case WMSZ_TOP:
			rect->left -= adjwidth;
			rect->top -= adjheight;
			break;

		case WMSZ_TOPRIGHT:
			rect->right += adjwidth;
			rect->top -= adjheight;
			break;
	}
}



//============================================================
//  get_min_bounds
//  (window thread)
//============================================================

static void get_min_bounds(win_window_info *window, RECT *bounds, int constrain)
{
	INT32 minwidth, minheight;

	assert(GetCurrentThreadId() == window_threadid);

	// get the minimum target size
	window->target->compute_minimum_size(minwidth, minheight);

	// expand to our minimum dimensions
	if (minwidth < MIN_WINDOW_DIM)
		minwidth = MIN_WINDOW_DIM;
	if (minheight < MIN_WINDOW_DIM)
		minheight = MIN_WINDOW_DIM;

	// account for extra window stuff
	minwidth += wnd_extra_width(window);
	minheight += wnd_extra_height(window);

	// if we want it constrained, figure out which one is larger
	if (constrain)
	{
		RECT test1, test2;

		// first constrain with no height limit
		test1.top = test1.left = 0;
		test1.right = minwidth;
		test1.bottom = 10000;
		constrain_to_aspect_ratio(window, &test1, WMSZ_BOTTOMRIGHT);

		// then constrain with no width limit
		test2.top = test2.left = 0;
		test2.right = 10000;
		test2.bottom = minheight;
		constrain_to_aspect_ratio(window, &test2, WMSZ_BOTTOMRIGHT);

		// pick the larger
		if (rect_width(&test1) > rect_width(&test2))
		{
			minwidth = rect_width(&test1);
			minheight = rect_height(&test1);
		}
		else
		{
			minwidth = rect_width(&test2);
			minheight = rect_height(&test2);
		}
	}

	// get the window rect
	GetWindowRect(window->hwnd, bounds);

	// now adjust
	bounds->right = bounds->left + minwidth;
	bounds->bottom = bounds->top + minheight;
}



//============================================================
//  get_max_bounds
//  (window thread)
//============================================================

static void get_max_bounds(win_window_info *window, RECT *bounds, int constrain)
{
	RECT maximum;

	assert(GetCurrentThreadId() == window_threadid);

	// compute the maximum client area
	winvideo_monitor_refresh(window->monitor);
	maximum = window->monitor->info.rcWork;

	// clamp to the window's max
	if (window->maxwidth != 0)
	{
		int temp = window->maxwidth + wnd_extra_width(window);
		if (temp < rect_width(&maximum))
			maximum.right = maximum.left + temp;
	}
	if (window->maxheight != 0)
	{
		int temp = window->maxheight + wnd_extra_height(window);
		if (temp < rect_height(&maximum))
			maximum.bottom = maximum.top + temp;
	}

	// constrain to fit
	if (constrain)
		constrain_to_aspect_ratio(window, &maximum, WMSZ_BOTTOMRIGHT);
	else
	{
		maximum.right -= wnd_extra_width(window);
		maximum.bottom -= wnd_extra_height(window);
	}

	// center within the work area
	bounds->left = window->monitor->info.rcWork.left + (rect_width(&window->monitor->info.rcWork) - rect_width(&maximum)) / 2;
	bounds->top = window->monitor->info.rcWork.top + (rect_height(&window->monitor->info.rcWork) - rect_height(&maximum)) / 2;
	bounds->right = bounds->left + rect_width(&maximum);
	bounds->bottom = bounds->top + rect_height(&maximum);
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



//============================================================
//  minimize_window
//  (window thread)
//============================================================

static void minimize_window(win_window_info *window)
{
	RECT newsize;

	assert(GetCurrentThreadId() == window_threadid);

	get_min_bounds(window, &newsize, video_config.keepaspect);
	SetWindowPos(window->hwnd, NULL, newsize.left, newsize.top, rect_width(&newsize), rect_height(&newsize), SWP_NOZORDER);
}



//============================================================
//  maximize_window
//  (window thread)
//============================================================

static void maximize_window(win_window_info *window)
{
	RECT newsize;

	assert(GetCurrentThreadId() == window_threadid);

	get_max_bounds(window, &newsize, video_config.keepaspect);
	SetWindowPos(window->hwnd, NULL, newsize.left, newsize.top, rect_width(&newsize), rect_height(&newsize), SWP_NOZORDER);
}



//============================================================
//  adjust_window_position_after_major_change
//  (window thread)
//============================================================

static void adjust_window_position_after_major_change(win_window_info *window)
{
	RECT oldrect, newrect;

	assert(GetCurrentThreadId() == window_threadid);

	// get the current size
	GetWindowRect(window->hwnd, &oldrect);

	// adjust the window size so the client area is what we want
	if (!window->fullscreen)
	{
		// constrain the existing size to the aspect ratio
		newrect = oldrect;
		if (video_config.keepaspect)
			constrain_to_aspect_ratio(window, &newrect, WMSZ_BOTTOMRIGHT);
	}

	// in full screen, make sure it covers the primary display
	else
	{
		win_monitor_info *monitor = winwindow_video_window_monitor(window, NULL);
		newrect = monitor->info.rcMonitor;
	}

	// adjust the position if different
	if (oldrect.left != newrect.left || oldrect.top != newrect.top ||
		oldrect.right != newrect.right || oldrect.bottom != newrect.bottom)
		SetWindowPos(window->hwnd, window->fullscreen ? HWND_TOPMOST : HWND_TOP,
				newrect.left, newrect.top,
				rect_width(&newrect), rect_height(&newrect), 0);

	// take note of physical window size (used for lightgun coordinate calculation)
	if (window == win_window_list)
	{
		win_physical_width = rect_width(&newrect);
		win_physical_height = rect_height(&newrect);
		logerror("Physical width %d, height %d\n",win_physical_width,win_physical_height);
	}
}



//============================================================
//  set_fullscreen
//  (window thread)
//============================================================

static void set_fullscreen(win_window_info *window, int fullscreen)
{
	assert(GetCurrentThreadId() == window_threadid);

	// if we're in the right state, punt
	if (window->fullscreen == fullscreen)
		return;
	window->fullscreen = fullscreen;

	// kill off the drawers
	(*draw.window_destroy)(window);

	// hide ourself
	ShowWindow(window->hwnd, SW_HIDE);

	// configure the window if non-fullscreen
	if (!fullscreen)
	{
		// adjust the style
		SetWindowLong(window->hwnd, GWL_STYLE, WINDOW_STYLE);
		SetWindowLong(window->hwnd, GWL_EXSTYLE, WINDOW_STYLE_EX);
		SetWindowPos(window->hwnd, 0, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

		// force to the bottom, then back on top
		SetWindowPos(window->hwnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		SetWindowPos(window->hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

		// if we have previous non-fullscreen bounds, use those
		if (window->non_fullscreen_bounds.right != window->non_fullscreen_bounds.left)
		{
			SetWindowPos(window->hwnd, HWND_TOP, window->non_fullscreen_bounds.left, window->non_fullscreen_bounds.top,
						rect_width(&window->non_fullscreen_bounds), rect_height(&window->non_fullscreen_bounds),
						SWP_NOZORDER);
		}

		// otherwise, set a small size and maximize from there
		else
		{
			SetWindowPos(window->hwnd, HWND_TOP, 0, 0, MIN_WINDOW_DIM, MIN_WINDOW_DIM, SWP_NOZORDER);
			maximize_window(window);
		}
	}

	// configure the window if fullscreen
	else
	{
		// save the bounds
		GetWindowRect(window->hwnd, &window->non_fullscreen_bounds);

		// adjust the style
		SetWindowLong(window->hwnd, GWL_STYLE, FULLSCREEN_STYLE);
		SetWindowLong(window->hwnd, GWL_EXSTYLE, FULLSCREEN_STYLE_EX);
		SetWindowPos(window->hwnd, 0, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

		// set topmost
		SetWindowPos(window->hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	}

	// adjust the window to compensate for the change
	adjust_window_position_after_major_change(window);

	// show ourself
	if (!window->fullscreen || window->fullscreen_safe)
	{
		if (video_config.mode != VIDEO_MODE_NONE)
			ShowWindow(window->hwnd, SW_SHOW);
		if ((*draw.window_init)(window))
			exit(1);
	}

	// ensure we're still adjusted correctly
	adjust_window_position_after_major_change(window);
}
