// license:BSD-3-Clause
// copyright-holders:Aaron Giles
//============================================================
//
//  window.c - Win32 window handling
//
//============================================================

#define LOG_THREADS         0
#define LOG_TEMP_PAUSE      0

// Needed for RAW Input
#define WM_INPUT 0x00FF

// standard C headers
#include <process.h>

#include <atomic>

// MAME headers
#include "emu.h"
#include "uiinput.h"

// MAMEOS headers
#include "winmain.h"
#include "window.h"
#include "video.h"
#include "winutf8.h"

#include "winutil.h"

#include "modules/render/drawbgfx.h"
#include "modules/render/drawnone.h"
#include "modules/render/drawd3d.h"
#include "modules/render/drawgdi.h"
#if (USE_OPENGL)
#include "modules/render/drawogl.h"
#endif

#define NOT_ALREADY_DOWN(x) (x & 0x40000000) == 0
#define SCAN_CODE(x) ((x >> 16) & 0xff)
#define IS_EXTENDED(x) (0x01000000 & x)
#define MAKE_DI_SCAN(scan, isextended) (scan & 0x7f) | (isextended ? 0x80 : 0x00)
#define WINOSD(machine) downcast<windows_osd_interface*>(&machine.osd())

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

static HANDLE window_thread;
static DWORD window_threadid;

static DWORD last_update_time;

static HANDLE ui_pause_event;
static HANDLE window_thread_ready_event;



//============================================================
//  PROTOTYPES
//============================================================


static void create_window_class(void);

// temporary hacks
#if LOG_THREADS
struct mtlog
{
	osd_ticks_t timestamp;
	const char *event;
};

static mtlog mtlog[100000];
static std::atomic<INT32> mtlogindex;

void mtlog_add(const char *event)
{
	int index = mtlogindex++;
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
		fprintf(f, "%s",string_format("%20I64d %10I64d %s\n", (UINT64)curr, (UINT64)(curr - last), mtlog[i].event).c_str());
		last = curr;
	}
	fclose(f);
}
#else
void mtlog_add(const char *event) { }
#endif



//============================================================
//  window_init
//  (main thread)
//============================================================

bool windows_osd_interface::window_init()
{
	// get the main thread ID before anything else
	main_threadid = GetCurrentThreadId();

	// set up window class and register it
	create_window_class();

	// create an event to signal UI pausing
	ui_pause_event = CreateEvent(nullptr, TRUE, FALSE, nullptr);
	if (!ui_pause_event)
		fatalerror("Failed to create pause event\n");

	window_thread = GetCurrentThread();
	window_threadid = main_threadid;

	const int fallbacks[VIDEO_MODE_COUNT] = {
		-1,                 // NONE -> no fallback
		VIDEO_MODE_NONE,    // GDI -> NONE
		VIDEO_MODE_D3D,     // BGFX -> D3D
#if (USE_OPENGL)
		VIDEO_MODE_GDI,     // OPENGL -> GDI
#endif
		-1,                 // No SDL2ACCEL on Windows OSD
#if (USE_OPENGL)
		VIDEO_MODE_OPENGL,  // D3D -> OPENGL
#else
		VIDEO_MODE_GDI,     // D3D -> GDI
#endif
		-1                  // No SOFT on Windows OSD
	};

	int current_mode = video_config.mode;
	while (current_mode != VIDEO_MODE_NONE)
	{
		bool error = false;
		switch(current_mode)
	{
			case VIDEO_MODE_NONE:
				error = renderer_none::init(machine());
				break;
			case VIDEO_MODE_GDI:
				error = renderer_gdi::init(machine());
				break;
			case VIDEO_MODE_BGFX:
				error = renderer_bgfx::init(machine());
				break;
#if (USE_OPENGL)
			case VIDEO_MODE_OPENGL:
				error = renderer_ogl::init(machine());
				break;
#endif
			case VIDEO_MODE_SDL2ACCEL:
				fatalerror("SDL2-Accel renderer unavailable on Windows OSD.");
				break;
			case VIDEO_MODE_D3D:
				error = renderer_d3d9::init(machine());
				break;
			case VIDEO_MODE_SOFT:
				fatalerror("SDL1 renderer unavailable on Windows OSD.");
				break;
			default:
				fatalerror("Unknown video mode.");
				break;
		}
		if (error)
		{
			current_mode = fallbacks[current_mode];
		}
		else
		{
			break;
		}
	}
	video_config.mode = current_mode;

	// set up the window list
	last_window_ptr = &win_window_list;

	return true;
}

void windows_osd_interface::update_slider_list()
{
	for (win_window_info *window = win_window_list; window != nullptr; window = window->m_next)
	{
		// check if any window has dirty sliders
		if (window->m_renderer && window->m_renderer->sliders_dirty())
		{
			build_slider_list();
			return;
		}
	}
}

int windows_osd_interface::window_count()
{
	int count = 0;
	for (win_window_info *info = win_window_list; info != nullptr; info = info->m_next)
	{
		count++;
	}
	return count;
}

void windows_osd_interface::build_slider_list()
{
	m_sliders = nullptr;
	slider_state* full_list = nullptr;
	slider_state* curr = nullptr;
	for (win_window_info *window = win_window_list; window != nullptr; window = window->m_next)
	{
		// take the sliders of the first window
		slider_state* window_sliders = window->m_renderer->get_slider_list();
		if (window_sliders == nullptr)
		{
			continue;
		}

		if (full_list == nullptr)
		{
			full_list = curr = window_sliders;
		}
		else
		{
			curr->next = window_sliders;
		}

		while (curr->next != nullptr) {
			curr = curr->next;
		}
	}

	m_sliders = full_list;
}

//============================================================
//  winwindow_exit
//  (main thread)
//============================================================

void windows_osd_interface::window_exit()
{
	assert(GetCurrentThreadId() == main_threadid);

	// free all the windows
	while (win_window_list != nullptr)
	{
		win_window_info *temp = win_window_list;
		win_window_list = temp->m_next;
		temp->destroy();
		global_free(temp);
	}

	switch(video_config.mode)
	{
		case VIDEO_MODE_NONE:
			renderer_none::exit();
			break;
		case VIDEO_MODE_GDI:
			renderer_gdi::exit();
			break;
		case VIDEO_MODE_BGFX:
			renderer_bgfx::exit();
			break;
#if (USE_OPENGL)
		case VIDEO_MODE_OPENGL:
			renderer_ogl::exit();
			break;
#endif
		case VIDEO_MODE_D3D:
			renderer_d3d9::exit();
			break;
		default:
			break;
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

win_window_info::win_window_info(running_machine &machine)
		: osd_window(), m_next(nullptr),
		m_init_state(0),
		m_startmaximized(0),
		m_isminimized(0),
		m_ismaximized(0),
		m_monitor(nullptr),
		m_fullscreen(0),
		m_fullscreen_safe(0),
		m_aspect(0),
		m_target(nullptr),
		m_targetview(0),
		m_targetorient(0),
		m_lastclicktime(0),
		m_lastclickx(0),
		m_lastclicky(0),
		m_renderer(nullptr),
		m_machine(machine)
{
	memset(m_title,0,sizeof(m_title));
	m_non_fullscreen_bounds.left = 0;
	m_non_fullscreen_bounds.top = 0;
	m_non_fullscreen_bounds.right  = 0;
	m_non_fullscreen_bounds.bottom = 0;
	m_prescale = video_config.prescale;
}

win_window_info::~win_window_info()
{
	if (m_renderer != nullptr)
	{
		delete m_renderer;
}
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
	winwindow_process_events(machine, TRUE, FALSE);
}



//============================================================
//  is_mame_window
//============================================================

static BOOL is_mame_window(HWND hwnd)
{
	for (win_window_info *window = win_window_list; window != nullptr; window = window->m_next)
		if (window->m_hwnd == hwnd)
			return TRUE;

	return FALSE;
}

inline static BOOL handle_mouse_button(windows_osd_interface *osd, int button, int down, int x, int y)
{
	MouseButtonEventArgs args;
	args.button = button;
	args.keydown = down;
	args.xpos = x;
	args.ypos = y;

	bool handled = osd->handle_input_event(INPUT_EVENT_MOUSE_BUTTON, &args);

	// When in lightgun mode or mouse mode, the mouse click may be routed to the input system
	// because the mouse interactions in the UI are routed from the video_window_proc below
	// we need to make sure they aren't suppressed in these cases.
	return handled && !osd->options().lightgun() && !osd->options().mouse();
}

inline static BOOL handle_keypress(windows_osd_interface *osd, int vkey, int down, int scancode, BOOL extended_key)
{
	KeyPressEventArgs args;
	args.event_id = down ? INPUT_EVENT_KEYDOWN : INPUT_EVENT_KEYUP;
	args.scancode = MAKE_DI_SCAN(scancode, extended_key);
	args.vkey = vkey;

	return osd->handle_input_event(args.event_id, &args);
}

//============================================================
//  winwindow_process_events
//  (main thread)
//============================================================

void winwindow_process_events(running_machine &machine, int ingame, bool nodispatch)
{
	MSG message;

	assert(GetCurrentThreadId() == main_threadid);

	// remember the last time we did this
	last_event_check = GetTickCount();

	do
	{
		// if we are paused, lets wait for a message
		if (ui_temp_pause > 0)
			WaitMessage();

		// loop over all messages in the queue
		while (PeekMessage(&message, nullptr, 0, 0, PM_REMOVE))
		{
			// prevent debugger windows from getting messages during reset
			int dispatch = TRUE && !nodispatch;

			if (message.hwnd == nullptr || is_mame_window(message.hwnd))
			{
				dispatch = TRUE;
				switch (message.message)
				{
					// ignore keyboard messages
					case WM_SYSKEYUP:
					case WM_SYSKEYDOWN:
						dispatch = FALSE;
						break;

					// forward mouse button downs to the input system
					case WM_LBUTTONDOWN:
						dispatch = !handle_mouse_button(WINOSD(machine), 0, TRUE, GET_X_LPARAM(message.lParam), GET_Y_LPARAM(message.lParam));
						break;

					case WM_RBUTTONDOWN:
						dispatch = !handle_mouse_button(WINOSD(machine), 1, TRUE, GET_X_LPARAM(message.lParam), GET_Y_LPARAM(message.lParam));
						break;

					case WM_MBUTTONDOWN:
						dispatch = !handle_mouse_button(WINOSD(machine), 2, TRUE, GET_X_LPARAM(message.lParam), GET_Y_LPARAM(message.lParam));
						break;

					case WM_XBUTTONDOWN:
						dispatch = !handle_mouse_button(WINOSD(machine), 3, TRUE, GET_X_LPARAM(message.lParam), GET_Y_LPARAM(message.lParam));
						break;

					// forward mouse button ups to the input system
					case WM_LBUTTONUP:
						dispatch = !handle_mouse_button(WINOSD(machine), 0, FALSE, GET_X_LPARAM(message.lParam), GET_Y_LPARAM(message.lParam));
						break;

					case WM_RBUTTONUP:
						dispatch = !handle_mouse_button(WINOSD(machine), 1, FALSE, GET_X_LPARAM(message.lParam), GET_Y_LPARAM(message.lParam));
						break;

					case WM_MBUTTONUP:
						dispatch = !handle_mouse_button(WINOSD(machine), 2, FALSE, GET_X_LPARAM(message.lParam), GET_Y_LPARAM(message.lParam));
						break;

					case WM_XBUTTONUP:
						dispatch = !handle_mouse_button(WINOSD(machine), 3, FALSE, GET_X_LPARAM(message.lParam), GET_Y_LPARAM(message.lParam));
						break;

					case WM_KEYDOWN:
						if (NOT_ALREADY_DOWN(message.lParam))
							dispatch = !handle_keypress(WINOSD(machine), message.wParam, TRUE, SCAN_CODE(message.lParam), IS_EXTENDED(message.lParam));
						break;

					case WM_KEYUP:
						dispatch = !handle_keypress(WINOSD(machine), message.wParam, FALSE, SCAN_CODE(message.lParam), IS_EXTENDED(message.lParam));
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
	win_window_info *window;

	assert(GetCurrentThreadId() == main_threadid);

	// iterate over windows and request a snap
	for (window = win_window_list; window != nullptr; window = window->m_next)
	{
		window->m_renderer->save();
	}
}



//============================================================
//  winwindow_toggle_fsfx
//  (main thread)
//============================================================

void winwindow_toggle_fsfx(void)
{
	win_window_info *window;

	assert(GetCurrentThreadId() == main_threadid);

	// iterate over windows and request a snap
	for (window = win_window_list; window != nullptr; window = window->m_next)
	{
		window->m_renderer->toggle_fsfx();
	}
}



//============================================================
//  winwindow_take_video
//  (main thread)
//============================================================

void winwindow_take_video(void)
{
	win_window_info *window;

	assert(GetCurrentThreadId() == main_threadid);

	// iterate over windows and request a snap
	for (window = win_window_list; window != nullptr; window = window->m_next)
	{
		window->m_renderer->record();
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
	for (window = win_window_list; window != nullptr; window = window->m_next)
		if (window->machine().debug_flags & DEBUG_FLAG_OSD_ENABLED)
			return;

	// toggle the window mode
	video_config.windowed = !video_config.windowed;

	// iterate over windows and toggle their fullscreen state
	for (window = win_window_list; window != nullptr; window = window->m_next)
		SendMessage(window->m_hwnd, WM_USER_SET_FULLSCREEN, !video_config.windowed, 0);
	SetForegroundWindow(win_window_list->m_hwnd);
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
	for (window = win_window_list; window != nullptr; window = window->m_next)
		if (focuswnd == window->m_hwnd)
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
	if (winwindow_has_focus() && ((!video_config.windowed && !win_window_list->win_has_menu()) || (!machine.paused() && downcast<windows_osd_interface&>(machine.osd()).should_hide_mouse())))
	{
		win_window_info *window = win_window_list;
		RECT bounds;

		// hide cursor
		while (ShowCursor(FALSE) >= -1) { };
		ShowCursor(TRUE);

		// store the cursor position
		GetCursorPos(&saved_cursor_pos);

		// clip cursor to game video window
		GetClientRect(window->m_hwnd, &bounds);
		ClientToScreen(window->m_hwnd, &((POINT *)&bounds)[0]);
		ClientToScreen(window->m_hwnd, &((POINT *)&bounds)[1]);
		ClipCursor(&bounds);
	}
	else
	{
		// show cursor
		while (ShowCursor(TRUE) < 1) { };
		ShowCursor(FALSE);

		// allow cursor to move freely
		ClipCursor(nullptr);
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

void win_window_info::create(running_machine &machine, int index, osd_monitor_info *monitor, const osd_window_config *config)
{
	win_window_info *win;

	assert(GetCurrentThreadId() == main_threadid);

	// allocate a new window object
	win_window_info *window = global_alloc(win_window_info(machine));
	window->m_win_config = *config;
	window->m_monitor = monitor;
	window->m_fullscreen = !video_config.windowed;
	window->m_index = index;

	// set main window
	if (index > 0)
	{
		for (auto w = win_window_list; w != nullptr; w = w->m_next)
		{
			if (w->m_index == 0)
			{
				window->m_main = w;
				break;
			}
		}
	}
	// see if we are safe for fullscreen
	window->m_fullscreen_safe = TRUE;
	for (win = win_window_list; win != nullptr; win = win->m_next)
		if (win->m_monitor == monitor)
			window->m_fullscreen_safe = FALSE;

	// add us to the list
	*last_window_ptr = window;
	last_window_ptr = &window->m_next;

	// load the layout
	window->m_target = machine.render().target_alloc();

	// set the specific view
	windows_options &options = downcast<windows_options &>(machine.options());

	const char *defview = options.view();
	window->set_starting_view(index, defview, options.view(index));

	// remember the current values in case they change
	window->m_targetview = window->m_target->view();
	window->m_targetorient = window->m_target->orientation();
	window->m_targetlayerconfig = window->m_target->layer_config();

	// make the window title
	if (video_config.numscreens == 1)
		sprintf(window->m_title, "%s: %s [%s]", emulator_info::get_appname(), machine.system().description, machine.system().name);
	else
		sprintf(window->m_title, "%s: %s [%s] - Screen %d", emulator_info::get_appname(), machine.system().description, machine.system().name, index);

	// set the initial maximized state
	window->m_startmaximized = options.maximize();

	window->m_init_state = window->complete_create() ? -1 : 1;

	// handle error conditions
	if (window->m_init_state == -1)
		fatalerror("Unable to complete window creation\n");
}



//============================================================
//  winwindow_video_window_destroy
//  (main thread)
//============================================================

void win_window_info::destroy()
{
	win_window_info **prevptr;

	assert(GetCurrentThreadId() == main_threadid);

	// remove us from the list
	for (prevptr = &win_window_list; *prevptr != nullptr; prevptr = &(*prevptr)->m_next)
		if (*prevptr == this)
		{
			*prevptr = this->m_next;
			break;
		}

	// destroy the window
	if (m_hwnd != nullptr)
		SendMessage(m_hwnd, WM_USER_SELF_TERMINATE, 0, 0);

	// free the render target
	machine().render().target_free(m_target);
}



//============================================================
//  winwindow_video_window_update
//  (main thread)
//============================================================

void win_window_info::update()
{
	int targetview, targetorient;
	render_layer_config targetlayerconfig;

	assert(GetCurrentThreadId() == main_threadid);

	mtlog_add("winwindow_video_window_update: begin");

	// see if the target has changed significantly in window mode
	targetview = m_target->view();
	targetorient = m_target->orientation();
	targetlayerconfig = m_target->layer_config();
	if (targetview != m_targetview || targetorient != m_targetorient || targetlayerconfig != m_targetlayerconfig)
	{
		m_targetview = targetview;
		m_targetorient = targetorient;
		m_targetlayerconfig = targetlayerconfig;

		// in window mode, reminimize/maximize
		if (!m_fullscreen)
		{
			if (m_isminimized)
				SendMessage(m_hwnd, WM_USER_SET_MINSIZE, 0, 0);
			if (m_ismaximized)
				SendMessage(m_hwnd, WM_USER_SET_MAXSIZE, 0, 0);
		}
	}

	// if we're visible and running and not in the middle of a resize, draw
	if (m_hwnd != nullptr && m_target != nullptr && m_renderer != nullptr)
	{
		bool got_lock = true;

		mtlog_add("winwindow_video_window_update: try lock");

		// only block if we're throttled
		if (machine().video().throttled() || timeGetTime() - last_update_time > 250)
			m_render_lock.lock();
		else
			got_lock = m_render_lock.try_lock();

		// only render if we were able to get the lock
		if (got_lock)
		{
			render_primitive_list *primlist;

			mtlog_add("winwindow_video_window_update: got lock");

			// don't hold the lock; we just used it to see if rendering was still happening
			m_render_lock.unlock();

			// ensure the target bounds are up-to-date, and then get the primitives
			primlist = m_renderer->get_primitives();

			// post a redraw request with the primitive list as a parameter
			last_update_time = timeGetTime();
			mtlog_add("winwindow_video_window_update: PostMessage start");
			SendMessage(m_hwnd, WM_USER_REDRAW, 0, (LPARAM)primlist);
			mtlog_add("winwindow_video_window_update: PostMessage end");
		}
	}

	mtlog_add("winwindow_video_window_update: end");
}



//============================================================
//  winwindow_video_window_monitor
//  (window thread)
//============================================================

osd_monitor_info *win_window_info::winwindow_video_window_monitor(const osd_rect *proposed)
{
	osd_monitor_info *monitor;

	// in window mode, find the nearest
	if (!m_fullscreen)
	{
		if (proposed != nullptr)
		{
			RECT p;
			p.top = proposed->top();
			p.left = proposed->left();
			p.bottom = proposed->bottom();
			p.right = proposed->right();
			monitor = win_monitor_info::monitor_from_handle(MonitorFromRect(&p, MONITOR_DEFAULTTONEAREST));
		}
		else
			monitor = win_monitor_info::monitor_from_handle(MonitorFromWindow(m_hwnd, MONITOR_DEFAULTTONEAREST));
	}

	// in full screen, just use the configured monitor
	else
		monitor = m_monitor;

	// make sure we're up-to-date
	//monitor->refresh();
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
		wc.lpszClassName    = TEXT("MAME");
		wc.hInstance        = GetModuleHandleUni();
		wc.lpfnWndProc      = winwindow_video_window_proc_ui;
		wc.hCursor          = LoadCursor(nullptr, IDC_ARROW);
		wc.hIcon            = LoadIcon(wc.hInstance, MAKEINTRESOURCE(2));

		UnregisterClass(wc.lpszClassName, wc.hInstance);

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

void win_window_info::set_starting_view(int index, const char *defview, const char *view)
{
	int viewindex;

	assert(GetCurrentThreadId() == main_threadid);

	// choose non-auto over auto
	if (strcmp(view, "auto") == 0 && strcmp(defview, "auto") != 0)
		view = defview;

	// query the video system to help us pick a view
	viewindex = target()->configured_view(view, index, video_config.numscreens);

	// set the view
	target()->set_view(viewindex);
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
		osd_printf_verbose("winwindow_ui_pause_from_main_thread(): %d --> %d\n", old_temp_pause, ui_temp_pause);
}



//============================================================
//  winwindow_ui_pause_from_window_thread
//  (window thread)
//============================================================

void winwindow_ui_pause_from_window_thread(running_machine &machine, int pause)
{
	assert(GetCurrentThreadId() == window_threadid);
	winwindow_ui_pause_from_main_thread(machine, pause);
}



//============================================================
//  winwindow_ui_exec_on_main_thread
//  (window thread)
//============================================================

void winwindow_ui_exec_on_main_thread(void (*func)(void *), void *param)
{
	assert(GetCurrentThreadId() == window_threadid);

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

int win_window_info::wnd_extra_width()
{
	RECT temprect = { 100, 100, 200, 200 };
	if (m_fullscreen)
		return 0;
	AdjustWindowRectEx(&temprect, WINDOW_STYLE, win_has_menu(), WINDOW_STYLE_EX);
	return rect_width(&temprect) - 100;
}



//============================================================
//  wnd_extra_height
//  (window thread)
//============================================================

int win_window_info::wnd_extra_height()
{
	RECT temprect = { 100, 100, 200, 200 };
	if (m_fullscreen)
		return 0;
	AdjustWindowRectEx(&temprect, WINDOW_STYLE, win_has_menu(), WINDOW_STYLE_EX);
	return rect_height(&temprect) - 100;
}

//============================================================
//  thread_entry
//  (window thread)
//============================================================

unsigned __stdcall win_window_info::thread_entry(void *param)
{
	MSG message;
	windows_osd_interface *osd = static_cast<windows_osd_interface*>(param);

	// make a bogus user call to make us a message thread
	PeekMessage(&message, nullptr, 0, 0, PM_NOREMOVE);

	// attach our input to the main thread
	AttachThreadInput(main_threadid, window_threadid, TRUE);

	// signal to the main thread that we are ready to receive events
	SetEvent(window_thread_ready_event);

	// run the message pump
	while (GetMessage(&message, nullptr, 0, 0))
	{
		int dispatch = TRUE;

		if ((message.hwnd == nullptr) || is_mame_window(message.hwnd))
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
					dispatch = !handle_mouse_button(osd, 0, TRUE, GET_X_LPARAM(message.lParam), GET_Y_LPARAM(message.lParam));
					break;

				case WM_RBUTTONDOWN:
					dispatch = !handle_mouse_button(osd, 1, TRUE, GET_X_LPARAM(message.lParam), GET_Y_LPARAM(message.lParam));
					break;

				case WM_MBUTTONDOWN:
					dispatch = !handle_mouse_button(osd, 2, TRUE, GET_X_LPARAM(message.lParam), GET_Y_LPARAM(message.lParam));
					break;

				case WM_XBUTTONDOWN:
					dispatch = !handle_mouse_button(osd, 3, TRUE, GET_X_LPARAM(message.lParam), GET_Y_LPARAM(message.lParam));
					break;

				// forward mouse button ups to the input system
				case WM_LBUTTONUP:
					dispatch = !handle_mouse_button(osd, 0, FALSE, GET_X_LPARAM(message.lParam), GET_Y_LPARAM(message.lParam));
					break;

				case WM_RBUTTONUP:
					dispatch = !handle_mouse_button(osd, 1, FALSE, GET_X_LPARAM(message.lParam), GET_Y_LPARAM(message.lParam));
					break;

				case WM_MBUTTONUP:
					dispatch = !handle_mouse_button(osd, 2, FALSE, GET_X_LPARAM(message.lParam), GET_Y_LPARAM(message.lParam));
					break;

				case WM_XBUTTONUP:
					dispatch = !handle_mouse_button(osd, 3, FALSE, GET_X_LPARAM(message.lParam), GET_Y_LPARAM(message.lParam));
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
					window->m_init_state = window->complete_create() ? -1 : 1;
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

int win_window_info::complete_create()
{
	RECT client;
	int tempwidth, tempheight;
	HMENU menu = nullptr;
	HDC dc;

	assert(GetCurrentThreadId() == window_threadid);

	// get the monitor bounds
	osd_rect monitorbounds = m_monitor->position_size();

	// create the window menu if needed
	if (downcast<windows_options &>(machine().options()).menu())
	{
		if (win_create_menu(machine(), &menu))
			return 1;
	}

	// create the window, but don't show it yet
	m_hwnd = win_create_window_ex_utf8(
						m_fullscreen ? FULLSCREEN_STYLE_EX : WINDOW_STYLE_EX,
						"MAME",
						m_title,
						m_fullscreen ? FULLSCREEN_STYLE : WINDOW_STYLE,
						monitorbounds.left() + 20, monitorbounds.top() + 20,
						monitorbounds.left() + 100, monitorbounds.top() + 100,
						nullptr,//(win_window_list != nullptr) ? win_window_list->m_hwnd : nullptr,
						menu,
						GetModuleHandleUni(),
						nullptr);
	if (m_hwnd == nullptr)
		return 1;

	// set window #0 as the focus window for all windows, required for D3D & multimonitor
	m_focus_hwnd = win_window_list->m_hwnd;

	// set a pointer back to us
	SetWindowLongPtr(m_hwnd, GWLP_USERDATA, (LONG_PTR)this);

	// skip the positioning stuff for -video none */
	if (video_config.mode == VIDEO_MODE_NONE)
		return 0;

	// adjust the window position to the initial width/height
	tempwidth = (m_win_config.width != 0) ? m_win_config.width : 640;
	tempheight = (m_win_config.height != 0) ? m_win_config.height : 480;
	SetWindowPos(m_hwnd, nullptr, monitorbounds.left() + 20, monitorbounds.top() + 20,
			monitorbounds.left() + tempwidth + wnd_extra_width(),
			monitorbounds.top() + tempheight + wnd_extra_height(),
			SWP_NOZORDER);

	// maximum or minimize as appropriate
	if (m_startmaximized)
		maximize_window();
	else
		minimize_window();
	adjust_window_position_after_major_change();

	// show the window
	if (!m_fullscreen || m_fullscreen_safe)
	{
		// finish off by trying to initialize DirectX; if we fail, ignore it
		if (m_renderer != nullptr)
		{
			delete m_renderer;
		}
		m_renderer = osd_renderer::make_for_type(video_config.mode, reinterpret_cast<osd_window *>(this));
		if (m_renderer->create())
			return 1;

		ShowWindow(m_hwnd, SW_SHOW);
	}

	// clear the window
	dc = GetDC(m_hwnd);
	GetClientRect(m_hwnd, &client);
	FillRect(dc, &client, (HBRUSH)GetStockObject(BLACK_BRUSH));
	ReleaseDC(m_hwnd, dc);
	return 0;
}



//============================================================
//  winwindow_video_window_proc
//  (window thread)
//============================================================

LRESULT CALLBACK win_window_info::video_window_proc(HWND wnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	LONG_PTR ptr = GetWindowLongPtr(wnd, GWLP_USERDATA);
	win_window_info *window = (win_window_info *)ptr;

	// we may get called before SetWindowLongPtr is called
	if (window != nullptr)
	{
		assert(GetCurrentThreadId() == window_threadid);
		window->update_minmax_state();
	}

	// handle a few messages
	switch (message)
	{
		// paint: redraw the last bitmap
		case WM_PAINT:
		{
			PAINTSTRUCT pstruct;
			HDC hdc = BeginPaint(wnd, &pstruct);
			window->draw_video_contents(hdc, TRUE);
			if (window->win_has_menu())
				DrawMenuBar(window->m_hwnd);
			EndPaint(wnd, &pstruct);
			break;
		}

		// non-client paint: punt if full screen
		case WM_NCPAINT:
			if (!window->m_fullscreen || window->win_has_menu())
				return DefWindowProc(wnd, message, wparam, lparam);
			break;

		// input: handle the raw input
		case WM_INPUT:
			downcast<windows_osd_interface&>(window->machine().osd()).handle_input_event(INPUT_EVENT_RAWINPUT, &lparam);
			break;

		// syskeys - ignore
		case WM_SYSKEYUP:
		case WM_SYSKEYDOWN:
			break;

		// input events
		case WM_MOUSEMOVE:
			window->machine().ui_input().push_mouse_move_event(window->m_target, GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam));
			break;

		case WM_MOUSELEAVE:
			window->machine().ui_input().push_mouse_leave_event(window->m_target);
			break;

		case WM_LBUTTONDOWN:
		{
			DWORD ticks = GetTickCount();
			window->machine().ui_input().push_mouse_down_event(window->m_target, GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam));

			// check for a double-click
			if (ticks - window->m_lastclicktime < GetDoubleClickTime() &&
				GET_X_LPARAM(lparam) >= window->m_lastclickx - 4 && GET_X_LPARAM(lparam) <= window->m_lastclickx + 4 &&
				GET_Y_LPARAM(lparam) >= window->m_lastclicky - 4 && GET_Y_LPARAM(lparam) <= window->m_lastclicky + 4)
			{
				window->m_lastclicktime = 0;
				window->machine().ui_input().push_mouse_double_click_event(window->m_target, GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam));
			}
			else
			{
				window->m_lastclicktime = ticks;
				window->m_lastclickx = GET_X_LPARAM(lparam);
				window->m_lastclicky = GET_Y_LPARAM(lparam);
			}
			break;
		}

		case WM_LBUTTONUP:
			window->machine().ui_input().push_mouse_up_event(window->m_target, GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam));
			break;

		case WM_CHAR:
			window->machine().ui_input().push_char_event(window->m_target, (unicode_char) wparam);
			break;

		case WM_MOUSEWHEEL:
		{
			UINT ucNumLines = 3; // default
			SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &ucNumLines, 0);
			window->machine().ui_input().push_mouse_wheel_event(window->m_target, GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam), GET_WHEEL_DELTA_WPARAM(wparam), ucNumLines);
			break;
		}

		// pause the system when we start a menu or resize
		case WM_ENTERSIZEMOVE:
			window->m_resize_state = RESIZE_STATE_RESIZING;
		case WM_ENTERMENULOOP:
			winwindow_ui_pause_from_window_thread(window->machine(), TRUE);
			break;

		// unpause the system when we stop a menu or resize and force a redraw
		case WM_EXITSIZEMOVE:
			window->m_resize_state = RESIZE_STATE_PENDING;
		case WM_EXITMENULOOP:
			winwindow_ui_pause_from_window_thread(window->machine(), FALSE);
			InvalidateRect(wnd, nullptr, FALSE);
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
			{
				osd_rect r = window->constrain_to_aspect_ratio(RECT_to_osd_rect(*rect), wparam);
				rect->top = r.top();
				rect->left = r.left();
				rect->bottom = r.bottom();
				rect->right = r.right();
			}
			InvalidateRect(wnd, nullptr, FALSE);
			break;
		}

		// syscommands: catch win_start_maximized
		case WM_SYSCOMMAND:
		{
			UINT16 cmd = wparam & 0xfff0;

			// prevent screensaver or monitor power events
			if (cmd == SC_MONITORPOWER || cmd == SC_SCREENSAVE)
				return 1;

			// most SYSCOMMANDs require us to invalidate the window
			InvalidateRect(wnd, nullptr, FALSE);

			// handle maximize
			if (cmd == SC_MAXIMIZE)
			{
				window->update_minmax_state();
				if (window->m_ismaximized)
					window->minimize_window();
				else
					window->maximize_window();
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
			window->machine().schedule_exit();
			break;

		// destroy: clean up all attached rendering bits and nullptr out our hwnd
		case WM_DESTROY:
			global_free(window->m_renderer);
			window->m_renderer = nullptr;
			window->m_hwnd = nullptr;
			return DefWindowProc(wnd, message, wparam, lparam);

		// self redraw: draw ourself in a non-painty way
		case WM_USER_REDRAW:
		{
			HDC hdc = GetDC(wnd);

			mtlog_add("winwindow_video_window_proc: WM_USER_REDRAW begin");
			window->m_primlist = (render_primitive_list *)lparam;
			window->draw_video_contents(hdc, FALSE);
			mtlog_add("winwindow_video_window_proc: WM_USER_REDRAW end");

			ReleaseDC(wnd, hdc);
			break;
		}

		// self destruct
		case WM_USER_SELF_TERMINATE:
			DestroyWindow(window->m_hwnd);
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

		// maximum size set
		case WM_DISPLAYCHANGE:
			/* FIXME: The current codebase has an issue with setting aspect
			 * ratios correctly after display change. set_aspect should
			 * be set_forced_aspect and on a refresh this forced aspect should
			 * be preserved if set. If not, the standard aspect calculation
			 * should be used.
			 */
			window->m_monitor->refresh();
			window->m_monitor->update_resolution(LOWORD(lparam), HIWORD(lparam));
			break;

		// set focus: if we're not the primary window, switch back
		// commented out ATM because this prevents us from resizing secondary windows
//      case WM_SETFOCUS:
//          if (window != win_window_list && win_window_list != nullptr)
//              SetFocus(win_window_list->m_hwnd);
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

void win_window_info::draw_video_contents(HDC dc, int update)
{
	assert(GetCurrentThreadId() == window_threadid);

	mtlog_add("draw_video_contents: begin");

	mtlog_add("draw_video_contents: render lock acquire");
	std::lock_guard<std::mutex> lock(m_render_lock);
	mtlog_add("draw_video_contents: render lock acquired");

	// if we're iconic, don't bother
	if (m_hwnd != nullptr && !IsIconic(m_hwnd))
	{
		// if no bitmap, just fill
		if (m_primlist == nullptr)
		{
			RECT fill;
			GetClientRect(m_hwnd, &fill);
			FillRect(dc, &fill, (HBRUSH)GetStockObject(BLACK_BRUSH));
		}

		// otherwise, render with our drawing system
		else
		{
			// update DC
			m_dc = dc;
			m_renderer->draw(update);
			mtlog_add("draw_video_contents: drawing finished");
		}
	}

	mtlog_add("draw_video_contents: render lock released");

	mtlog_add("draw_video_contents: end");
}


//============================================================
//  constrain_to_aspect_ratio
//  (window thread)
//============================================================

osd_rect win_window_info::constrain_to_aspect_ratio(const osd_rect &rect, int adjustment)
{
	INT32 extrawidth = wnd_extra_width();
	INT32 extraheight = wnd_extra_height();
	INT32 propwidth, propheight;
	INT32 minwidth, minheight;
	INT32 maxwidth, maxheight;
	INT32 viswidth, visheight;
	INT32 adjwidth, adjheight;
	float pixel_aspect;
	osd_monitor_info *monitor = winwindow_video_window_monitor(&rect);

	assert(GetCurrentThreadId() == window_threadid);

	// do not constrain aspect ratio for integer scaled views
	if (m_target->scale_mode() != SCALE_FRACTIONAL)
		return rect;

	// get the pixel aspect ratio for the target monitor
	pixel_aspect = monitor->pixel_aspect();

	// determine the proposed width/height
	propwidth = rect.width() - extrawidth;
	propheight = rect.height() - extraheight;

	// based on which edge we are adjusting, take either the width, height, or both as gospel
	// and scale to fit using that as our parameter
	switch (adjustment)
	{
		case WMSZ_BOTTOM:
		case WMSZ_TOP:
			m_target->compute_visible_area(10000, propheight, pixel_aspect, m_target->orientation(), propwidth, propheight);
			break;

		case WMSZ_LEFT:
		case WMSZ_RIGHT:
			m_target->compute_visible_area(propwidth, 10000, pixel_aspect, m_target->orientation(), propwidth, propheight);
			break;

		default:
			m_target->compute_visible_area(propwidth, propheight, pixel_aspect, m_target->orientation(), propwidth, propheight);
			break;
	}

	// get the minimum width/height for the current layout
	m_target->compute_minimum_size(minwidth, minheight);

	// clamp against the absolute minimum
	propwidth = MAX(propwidth, MIN_WINDOW_DIM);
	propheight = MAX(propheight, MIN_WINDOW_DIM);

	// clamp against the minimum width and height
	propwidth = MAX(propwidth, minwidth);
	propheight = MAX(propheight, minheight);

	// clamp against the maximum (fit on one screen for full screen mode)
	if (m_fullscreen)
	{
		maxwidth = monitor->position_size().width() - extrawidth;
		maxheight = monitor->position_size().height() - extraheight;
	}
	else
	{
		maxwidth = monitor->usuable_position_size().width() - extrawidth;
		maxheight = monitor->usuable_position_size().height() - extraheight;

		// further clamp to the maximum width/height in the window
		if (m_win_config.width != 0)
			maxwidth = MIN(maxwidth, m_win_config.width + extrawidth);
		if (m_win_config.height != 0)
			maxheight = MIN(maxheight, m_win_config.height + extraheight);
	}

	// clamp to the maximum
	propwidth = MIN(propwidth, maxwidth);
	propheight = MIN(propheight, maxheight);

	// compute the visible area based on the proposed rectangle
	m_target->compute_visible_area(propwidth, propheight, pixel_aspect, m_target->orientation(), viswidth, visheight);

	// compute the adjustments we need to make
	adjwidth = (viswidth + extrawidth) - rect.width();
	adjheight = (visheight + extraheight) - rect.height();

	// based on which corner we're adjusting, constrain in different ways
	osd_rect ret(rect);

	switch (adjustment)
	{
		case WMSZ_BOTTOM:
		case WMSZ_BOTTOMRIGHT:
		case WMSZ_RIGHT:
			ret = rect.resize(rect.width() + adjwidth, rect.height() + adjheight);
			break;

		case WMSZ_BOTTOMLEFT:
			ret = rect.move_by(-adjwidth, 0).resize(rect.width() + adjwidth, rect.height() + adjheight);
			break;

		case WMSZ_LEFT:
		case WMSZ_TOPLEFT:
		case WMSZ_TOP:
			ret = rect.move_by(-adjwidth, -adjheight).resize(rect.width() + adjwidth, rect.height() + adjheight);
			break;

		case WMSZ_TOPRIGHT:
			ret = rect.move_by(0, -adjheight).resize(rect.width() + adjwidth, rect.height() + adjheight);
			break;
	}
	return ret;
}



//============================================================
//  get_min_bounds
//  (window thread)
//============================================================

osd_dim win_window_info::get_min_bounds(int constrain)
{
	INT32 minwidth, minheight;

	//assert(GetCurrentThreadId() == window_threadid);

	// get the minimum target size
	m_target->compute_minimum_size(minwidth, minheight);

	// expand to our minimum dimensions
	if (minwidth < MIN_WINDOW_DIM)
		minwidth = MIN_WINDOW_DIM;
	if (minheight < MIN_WINDOW_DIM)
		minheight = MIN_WINDOW_DIM;

	// account for extra window stuff
	minwidth += wnd_extra_width();
	minheight += wnd_extra_height();

	// if we want it constrained, figure out which one is larger
	if (constrain && m_target->scale_mode() == SCALE_FRACTIONAL)
	{
		// first constrain with no height limit
		osd_rect test1(0,0,minwidth,10000);
		test1 = constrain_to_aspect_ratio(test1, WMSZ_BOTTOMRIGHT);

		// then constrain with no width limit
		osd_rect test2(0,0,10000,minheight);
		test2 = constrain_to_aspect_ratio(test2, WMSZ_BOTTOMRIGHT);

		// pick the larger
		if (test1.width() > test2.width())
		{
			minwidth = test1.width();
			minheight = test1.height();
		}
		else
		{
			minwidth = test2.width();
			minheight = test2.height();
		}
	}

	return osd_dim(minwidth, minheight);
}



//============================================================
//  get_max_bounds
//  (window thread)
//============================================================

osd_dim win_window_info::get_max_bounds(int constrain)
{
	//assert(GetCurrentThreadId() == window_threadid);

	// compute the maximum client area
	//m_monitor->refresh();
	osd_rect maximum = m_monitor->usuable_position_size();

	// clamp to the window's max
	int tempw = maximum.width();
	int temph = maximum.height();
	if (m_win_config.width != 0)
	{
		int temp = m_win_config.width + wnd_extra_width();
		if (temp < maximum.width())
			tempw = temp;
	}
	if (m_win_config.height != 0)
	{
		int temp = m_win_config.height + wnd_extra_height();
		if (temp < maximum.height())
			temph = temp;
	}

	maximum = maximum.resize(tempw, temph);

	// constrain to fit
	if (constrain && m_target->scale_mode() == SCALE_FRACTIONAL)
		maximum = constrain_to_aspect_ratio(maximum, WMSZ_BOTTOMRIGHT);

	return maximum.dim();
}



//============================================================
//  update_minmax_state
//  (window thread)
//============================================================

void win_window_info::update_minmax_state()
{
	assert(GetCurrentThreadId() == window_threadid);

	if (!m_fullscreen)
	{
		RECT bounds;

		// compare the maximum bounds versus the current bounds
		osd_dim minbounds = get_min_bounds(video_config.keepaspect);
		osd_dim maxbounds = get_max_bounds(video_config.keepaspect);
		GetWindowRect(m_hwnd, &bounds);

		// if either the width or height matches, we were maximized
		m_isminimized = (rect_width(&bounds) == minbounds.width()) ||
								(rect_height(&bounds) == minbounds.height());
		m_ismaximized = (rect_width(&bounds) == maxbounds.width()) ||
								(rect_height(&bounds) == maxbounds.height());
	}
	else
	{
		m_isminimized = FALSE;
		m_ismaximized = TRUE;
	}
}



//============================================================
//  minimize_window
//  (window thread)
//============================================================

void win_window_info::minimize_window()
{
	assert(GetCurrentThreadId() == window_threadid);

	osd_dim newsize = get_min_bounds(video_config.keepaspect);

	// get the window rect
	RECT bounds;
	GetWindowRect(m_hwnd, &bounds);

	osd_rect newrect(bounds.left, bounds.top, newsize );


	SetWindowPos(m_hwnd, nullptr, newrect.left(), newrect.top(), newrect.width(), newrect.height(), SWP_NOZORDER);
}



//============================================================
//  maximize_window
//  (window thread)
//============================================================

void win_window_info::maximize_window()
{
	assert(GetCurrentThreadId() == window_threadid);

	osd_dim newsize = get_max_bounds(video_config.keepaspect);

	// center within the work area
	osd_rect work = m_monitor->usuable_position_size();
	osd_rect newrect = osd_rect(work.left() + (work.width() - newsize.width()) / 2,
			work.top() + (work.height() - newsize.height()) / 2,
			newsize);

	SetWindowPos(m_hwnd, nullptr, newrect.left(), newrect.top(), newrect.width(), newrect.height(), SWP_NOZORDER);
}



//============================================================
//  adjust_window_position_after_major_change
//  (window thread)
//============================================================

void win_window_info::adjust_window_position_after_major_change()
{
	RECT oldrect;

	assert(GetCurrentThreadId() == window_threadid);

	// get the current size
	GetWindowRect(m_hwnd, &oldrect);
	osd_rect newrect = RECT_to_osd_rect(oldrect);

	// adjust the window size so the client area is what we want
	if (!m_fullscreen)
	{
		// constrain the existing size to the aspect ratio
		if (video_config.keepaspect)
			newrect = constrain_to_aspect_ratio(newrect, WMSZ_BOTTOMRIGHT);
	}

	// in full screen, make sure it covers the primary display
	else
	{
		osd_monitor_info *monitor = winwindow_video_window_monitor(nullptr);
		newrect = monitor->position_size();
	}

	// adjust the position if different
	if (oldrect.left != newrect.left() || oldrect.top != newrect.top() ||
		oldrect.right != newrect.right() || oldrect.bottom != newrect.bottom())
		SetWindowPos(m_hwnd, m_fullscreen ? HWND_TOPMOST : HWND_TOP,
				newrect.left(), newrect.top(),
				newrect.width(), newrect.height(), 0);

	// take note of physical window size (used for lightgun coordinate calculation)
	if (this == win_window_list)
	{
		win_physical_width = newrect.width();
		win_physical_height = newrect.height();
		osd_printf_verbose("Physical width %d, height %d\n",win_physical_width,win_physical_height);
	}
}


//============================================================
//  set_fullscreen
//  (window thread)
//============================================================

void win_window_info::set_fullscreen(int fullscreen)
{
	assert(GetCurrentThreadId() == window_threadid);

	// if we're in the right state, punt
	if (m_fullscreen == fullscreen)
		return;
	m_fullscreen = fullscreen;

	// kill off the drawers
	delete m_renderer;
	m_renderer = nullptr;

	// hide ourself
	ShowWindow(m_hwnd, SW_HIDE);

	// configure the window if non-fullscreen
	if (!fullscreen)
	{
		// adjust the style
		SetWindowLong(m_hwnd, GWL_STYLE, WINDOW_STYLE);
		SetWindowLong(m_hwnd, GWL_EXSTYLE, WINDOW_STYLE_EX);
		SetWindowPos(m_hwnd, 0, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

		// force to the bottom, then back on top
		SetWindowPos(m_hwnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		SetWindowPos(m_hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

		// if we have previous non-fullscreen bounds, use those
		if (m_non_fullscreen_bounds.right != m_non_fullscreen_bounds.left)
		{
			SetWindowPos(m_hwnd, HWND_TOP, m_non_fullscreen_bounds.left, m_non_fullscreen_bounds.top,
						rect_width(&m_non_fullscreen_bounds), rect_height(&m_non_fullscreen_bounds),
						SWP_NOZORDER);
		}

		// otherwise, set a small size and maximize from there
		else
		{
			SetWindowPos(m_hwnd, HWND_TOP, 0, 0, MIN_WINDOW_DIM, MIN_WINDOW_DIM, SWP_NOZORDER);
			maximize_window();
		}
	}

	// configure the window if fullscreen
	else
	{
		// save the bounds
		GetWindowRect(m_hwnd, &m_non_fullscreen_bounds);

		// adjust the style
		SetWindowLong(m_hwnd, GWL_STYLE, FULLSCREEN_STYLE);
		SetWindowLong(m_hwnd, GWL_EXSTYLE, FULLSCREEN_STYLE_EX);
		SetWindowPos(m_hwnd, 0, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

		// set topmost
		SetWindowPos(m_hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	}

	// adjust the window to compensate for the change
	adjust_window_position_after_major_change();

	// show ourself
	if (!m_fullscreen || m_fullscreen_safe)
	{
		if (video_config.mode != VIDEO_MODE_NONE)
			ShowWindow(m_hwnd, SW_SHOW);

		m_renderer = reinterpret_cast<osd_renderer *>(osd_renderer::make_for_type(video_config.mode, reinterpret_cast<osd_window *>(this)));
		if (m_renderer->create())
			exit(1);
	}

	// ensure we're still adjusted correctly
	adjust_window_position_after_major_change();
}

#if (USE_QTDEBUG)
bool winwindow_qt_filter(void *message)
{
	MSG *msg = (MSG *)message;

	if(is_mame_window(msg->hwnd) || (!msg->hwnd && (msg->message >= WM_USER)))
	{
		LONG_PTR ptr;
		if(msg->hwnd) // get the machine associated with this window
			ptr = GetWindowLongPtr(msg->hwnd, GWLP_USERDATA);
		else // any one will have to do
			ptr = (LONG_PTR)win_window_list;

		winwindow_dispatch_message(((win_window_info *)ptr)->machine(), msg);
		return true;
	}
	return false;
}
#endif
