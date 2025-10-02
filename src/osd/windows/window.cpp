// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb
//============================================================
//
//  window.cpp - Win32 window handling
//
//============================================================

#define LOG_TEMP_PAUSE      0

// standard C headers
#include <process.h>

#include <algorithm>
#include <atomic>
#include <cstring>

// MAME headers
#include "emu.h"
#include "uiinput.h"
#include "ui/uimain.h"

// MAMEOS headers
#include "winmain.h"
#include "window.h"
#include "video.h"
#include "winutf8.h"

#include "winutil.h"
#include "strconv.h"

#include "modules/monitor/monitor_common.h"

#define NOT_ALREADY_DOWN(x) ((x & 0x40000000) == 0)
#define SCAN_CODE(x) ((x >> 16) & 0xff)
#define IS_EXTENDED(x) ((x >> 24) & 0x01)
#define MAKE_DI_SCAN(scan, isextended) ((scan & 0x7f) | (isextended ? 0x80 : 0x00))
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
#define MIN_WINDOW_DIMX                 200
#define MIN_WINDOW_DIMY                 50

// custom window messages
#define WM_USER_REDRAW                  (WM_USER + 2)
#define WM_USER_SET_FULLSCREEN          (WM_USER + 3)
#define WM_USER_SET_MAXSIZE             (WM_USER + 4)
#define WM_USER_SET_MINSIZE             (WM_USER + 5)


namespace {

// If legacy mouse to pointer event translation is enabled, translated
// WM_POINTER* events have pointer ID zero.  Assume this will never be
// seen for "real" pointer events.
constexpr WORD MOUSE_POINTER_ID = 0;

constexpr unsigned get_pointer_buttons(WPARAM wparam)
{
	return
			(IS_POINTER_FIRSTBUTTON_WPARAM(wparam) ? 0x01 : 0x00) |
			(IS_POINTER_SECONDBUTTON_WPARAM(wparam) ? 0x02 : 0x00) |
			(IS_POINTER_THIRDBUTTON_WPARAM(wparam) ? 0x04 : 0x00) |
			(IS_POINTER_FOURTHBUTTON_WPARAM(wparam) ? 0x08 : 0x00) |
			(IS_POINTER_FIFTHBUTTON_WPARAM(wparam) ? 0x10 : 0x00);
}

constexpr osd::ui_event_handler::pointer convert_pointer_type(POINTER_INPUT_TYPE type)
{
	// PT_POINTER isn't a real type, and we'll leave PT_TOUCHPAD as unknown for now
	return
			(PT_TOUCH == type) ? osd::ui_event_handler::pointer::TOUCH :
			(PT_PEN == type) ? osd::ui_event_handler::pointer::PEN :
			(PT_MOUSE == type) ? osd::ui_event_handler::pointer::MOUSE :
			osd::ui_event_handler::pointer::UNKNOWN;
}

} // anonymous namespace

//============================================================
//  GLOBAL VARIABLES
//============================================================

static DWORD main_threadid;



//============================================================
//  LOCAL VARIABLES
//============================================================

// event handling
static int ui_temp_pause;
static int ui_temp_was_paused;

static HANDLE window_thread;
static DWORD window_threadid;

static DWORD last_update_time;

static HANDLE ui_pause_event;

static bool s_aggressive_focus;



//============================================================
//  PROTOTYPES
//============================================================


static void create_window_class();

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

	return true;
}


int windows_osd_interface::window_count()
{
	return osd_common_t::window_list().size();
}

void windows_osd_interface::add_audio_to_recording(const int16_t *buffer, int samples_this_frame)
{
	auto const &window = osd_common_t::window_list().front(); // We only record on the first window
	if (window)
		window->renderer().add_audio_to_recording(buffer, samples_this_frame);
}


//============================================================
//  winwindow_exit
//  (main thread)
//============================================================

void windows_osd_interface::window_exit()
{
	assert(GetCurrentThreadId() == main_threadid);

	// if we hid the cursor during the emulation, show it
	if (!osd_common_t::s_window_list.empty())
		osd_common_t::s_window_list.front()->show_pointer();

	// free all the windows
	while (!osd_common_t::s_window_list.empty())
	{
		auto window = std::move(osd_common_t::s_window_list.back());
		s_window_list.pop_back();
		window->destroy();
	}

	// kill the UI pause event
	if (ui_pause_event)
		CloseHandle(ui_pause_event);
}


inline win_window_info::win_pointer_info::win_pointer_info(WORD p, POINTER_INPUT_TYPE t, unsigned i, unsigned d)
	: pointer_info(i, d)
	, ptrid(p)
	, type(t)
{
}


win_window_info::win_window_info(
		running_machine &machine,
		render_module &renderprovider,
		int index,
		const std::shared_ptr<osd_monitor_info> &monitor,
		const osd_window_config *config)
	: osd_window_t(machine, renderprovider, index, std::move(monitor), *config)
	, m_init_state(0)
	, m_startmaximized(0)
	, m_isminimized(0)
	, m_ismaximized(0)
	, m_fullscreen_safe(0)
	, m_aspect(0)
	, m_targetview(0)
	, m_targetorient(0)
	, m_targetvismask(0)
	, m_targetscalemode(0)
	, m_targetkeepaspect(machine.options().keep_aspect())
	, m_lastclicktime(std::chrono::steady_clock::time_point::min())
	, m_lastclickx(0)
	, m_lastclicky(0)
	, m_last_surrogate(0)
	, m_dc(nullptr)
	, m_resize_state(RESIZE_STATE_NORMAL)
	, m_main(nullptr)
	, m_attached_mode(false)
	, m_pointer_mask(0)
	, m_next_pointer(0)
	, m_next_ptrdev(0)
	, m_tracking_mouse(false)
{
	m_non_fullscreen_bounds.left = 0;
	m_non_fullscreen_bounds.top = 0;
	m_non_fullscreen_bounds.right  = 0;
	m_non_fullscreen_bounds.bottom = 0;

	m_fullscreen = !video_config.windowed;
	m_prescale = video_config.prescale;

	m_ptrdev_map.reserve(8);
	m_ptrdev_info.reserve(1);
	m_active_pointers.reserve(16);
}

POINT win_window_info::s_saved_cursor_pos = { -1, -1 };

void win_window_info::capture_pointer()
{
	RECT bounds;
	GetClientRect(platform_window(), &bounds);
	ClientToScreen(platform_window(), &reinterpret_cast<POINT *>(&bounds)[0]);
	ClientToScreen(platform_window(), &reinterpret_cast<POINT *>(&bounds)[1]);
	ClipCursor(&bounds);
}

void win_window_info::release_pointer()
{
	ClipCursor(nullptr);
}

void win_window_info::hide_pointer()
{
	GetCursorPos(&s_saved_cursor_pos);

	while (ShowCursor(FALSE) >= -1) { }
	ShowCursor(TRUE);
}

void win_window_info::show_pointer()
{
	if (s_saved_cursor_pos.x != -1 || s_saved_cursor_pos.y != -1)
	{
		SetCursorPos(s_saved_cursor_pos.x, s_saved_cursor_pos.y);
		s_saved_cursor_pos.x = s_saved_cursor_pos.y = -1;
	}

	while (ShowCursor(TRUE) < 1) { }
	ShowCursor(FALSE);
}


bool windows_osd_interface::has_focus() const
{
	return winwindow_has_focus();
}


//============================================================
//  winwindow_process_events_periodic
//  (main thread)
//============================================================

void windows_osd_interface::process_events()
{
	assert(GetCurrentThreadId() == main_threadid);

	auto const currticks = std::chrono::steady_clock::now();

	// update once every 1/8th of a second
	if (currticks < (m_last_event_check + std::chrono::milliseconds(1000 / 8)))
		return;

	process_events(true, false);
}



//============================================================
//  winwindow_video_window_proc_ui
//  (window thread)
//============================================================

static LRESULT CALLBACK winwindow_video_window_proc_ui(HWND wnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	return win_window_info::video_window_proc(wnd, message, wparam, lparam);
}

//============================================================
//  is_mame_window
//============================================================

inline bool is_mame_window(HWND hwnd)
{
	for (const auto &window : osd_common_t::window_list())
		if (dynamic_cast<win_window_info &>(*window).platform_window() == hwnd)
			return true;

	return false;
}

inline BOOL handle_mouse_button(windows_osd_interface &osd, int button, int down, LPARAM lparam)
{
	MouseUpdateEventArgs args;
	args.pressed = (down ? 1 : 0) << button;
	args.released = (down ? 0 : 1) << button;
	args.vdelta = 0;
	args.hdelta = 0;
	args.xpos = GET_X_LPARAM(lparam);
	args.ypos = GET_Y_LPARAM(lparam);

	bool const handled = osd.handle_input_event(INPUT_EVENT_MOUSE_BUTTON, &args);

	// When in lightgun mode or mouse mode, the mouse click may be routed to the input system
	// because the mouse interactions in the UI are routed from the video_window_proc below
	// we need to make sure they aren't suppressed in these cases.
	return handled && !osd.options().lightgun() && !osd.options().mouse();
}

inline BOOL handle_mouse_wheel(windows_osd_interface &osd, int v, int h, LPARAM lparam)
{
	MouseUpdateEventArgs args;
	args.pressed = 0;
	args.released = 0;
	args.vdelta = v;
	args.hdelta = h;
	args.xpos = GET_X_LPARAM(lparam);
	args.ypos = GET_Y_LPARAM(lparam);

	bool const handled = osd.handle_input_event(INPUT_EVENT_MOUSE_WHEEL, &args);

	// When in lightgun mode or mouse mode, the mouse wheel may be routed to the input system
	// because the mouse interactions in the UI are routed from the video_window_proc below
	// we need to make sure they aren't suppressed in these cases.
	return handled && !osd.options().lightgun() && !osd.options().mouse();
}

inline BOOL handle_keypress(windows_osd_interface &osd, int vkey, int down, LPARAM lparam)
{
	KeyPressEventArgs args;
	args.event_id = down ? INPUT_EVENT_KEYDOWN : INPUT_EVENT_KEYUP;
	args.scancode = MAKE_DI_SCAN(SCAN_CODE(lparam), IS_EXTENDED(lparam));
	args.vkey = vkey;

	return osd.handle_input_event(args.event_id, &args);
}

//============================================================
//  winwindow_process_events
//  (main thread)
//============================================================

void windows_osd_interface::process_events(bool ingame, bool nodispatch)
{
	assert(GetCurrentThreadId() == main_threadid);

	// remember the last time we did this
	m_last_event_check = std::chrono::steady_clock::now();

	do
	{
		// if we are paused, lets wait for a message
		if (ui_temp_pause > 0)
			WaitMessage();

		// loop over all messages in the queue
		MSG message;
		while (PeekMessage(&message, nullptr, 0, 0, PM_REMOVE))
		{
			// prevent debugger windows from getting messages during reset
			bool dispatch = !nodispatch;

			if (!message.hwnd || is_mame_window(message.hwnd))
			{
				dispatch = true;
				switch (message.message)
				{
					// ignore keyboard messages
					case WM_SYSKEYUP:
					case WM_SYSKEYDOWN:
						dispatch = false;
						break;

					// forward mouse button downs to the input system
					case WM_LBUTTONDOWN:
						dispatch = !handle_mouse_button(*this, 0, TRUE, message.lParam);
						break;

					case WM_RBUTTONDOWN:
						dispatch = !handle_mouse_button(*this, 1, TRUE, message.lParam);
						break;

					case WM_MBUTTONDOWN:
						dispatch = !handle_mouse_button(*this, 2, TRUE, message.lParam);
						break;

					case WM_XBUTTONDOWN:
						dispatch = !handle_mouse_button(*this, (GET_XBUTTON_WPARAM(message.wParam) == XBUTTON1) ? 3 : 4, TRUE, message.lParam);
						break;

					// forward mouse button ups to the input system
					case WM_LBUTTONUP:
						dispatch = !handle_mouse_button(*this, 0, FALSE, message.lParam);
						break;

					case WM_RBUTTONUP:
						dispatch = !handle_mouse_button(*this, 1, FALSE, message.lParam);
						break;

					case WM_MBUTTONUP:
						dispatch = !handle_mouse_button(*this, 2, FALSE, message.lParam);
						break;

					case WM_XBUTTONUP:
						dispatch = !handle_mouse_button(*this, (GET_XBUTTON_WPARAM(message.wParam) == XBUTTON1) ? 3 : 4, FALSE, message.lParam);
						break;

					// forward mouse wheel movement to the input system
					case WM_MOUSEWHEEL:
						dispatch = !handle_mouse_wheel(*this, GET_WHEEL_DELTA_WPARAM(message.wParam), 0, message.lParam);
						break;

					case WM_MOUSEHWHEEL:
						dispatch = !handle_mouse_wheel(*this, 0, GET_WHEEL_DELTA_WPARAM(message.wParam), message.lParam);
						break;

					// forward keystrokes to the input system
					case WM_KEYDOWN:
						if (NOT_ALREADY_DOWN(message.lParam))
							dispatch = !handle_keypress(*this, message.wParam, TRUE, message.lParam);
						break;

					case WM_KEYUP:
						dispatch = !handle_keypress(*this, message.wParam, FALSE, message.lParam);
						break;
				}
			}

			// dispatch if necessary
			if (dispatch)
				winwindow_dispatch_message(machine(), message);
		}
	}
	while (ui_temp_pause > 0);

	// update the cursor state after processing events
	winwindow_update_cursor_state(machine());
}



//============================================================
//  winwindow_dispatch_message
//  (main thread)
//============================================================

void winwindow_dispatch_message(running_machine &machine, MSG const &message)
{
	assert(GetCurrentThreadId() == main_threadid);

	// dispatch our special communication messages
	switch (message.message)
	{
		// special case for quit
		case WM_QUIT:
			machine.schedule_exit();
			break;

		// everything else dispatches normally
		default:
			TranslateMessage(&message);
			DispatchMessage(&message);
			break;
	}
}



//============================================================
//  winwindow_take_snap
//  (main thread)
//============================================================

void winwindow_take_snap()
{
	assert(GetCurrentThreadId() == main_threadid);

	// iterate over windows and request a snap
	for (const auto &window : osd_common_t::window_list())
		window->renderer().save();
}



//============================================================
//  winwindow_toggle_fsfx
//  (main thread)
//============================================================

void winwindow_toggle_fsfx()
{
	assert(GetCurrentThreadId() == main_threadid);

	// iterate over windows and request a snap
	for (const auto &window : osd_common_t::window_list())
		window->renderer().toggle_fsfx();
}



//============================================================
//  winwindow_take_video
//  (main thread)
//============================================================

void winwindow_take_video()
{
	assert(GetCurrentThreadId() == main_threadid);

	// iterate over windows and request a snap
	for (const auto &window : osd_common_t::window_list())
		window->renderer().record();
}



//============================================================
//  winwindow_toggle_full_screen
//  (main thread)
//============================================================

void winwindow_toggle_full_screen()
{
	assert(GetCurrentThreadId() == main_threadid);

	// if we are in debug mode, never go full screen
	for (const auto &window : osd_common_t::window_list())
	{
		if (window->machine().debug_flags & DEBUG_FLAG_OSD_ENABLED)
			return;
	}

	// toggle the window mode
	video_config.windowed = !video_config.windowed;

	// destroy the renderers first so that the render module can bounce if it depends on having a window handle
	for (auto it = osd_common_t::window_list().rbegin(); osd_common_t::window_list().rend() != it; ++it)
		(*it)->renderer_reset();

	// iterate over windows and toggle their fullscreen state
	for (const auto &window : osd_common_t::window_list())
	{
		SendMessage(
				dynamic_cast<win_window_info &>(*window).platform_window(),
				WM_USER_SET_FULLSCREEN,
				!video_config.windowed,
				0);
	}

	// Set the first window as foreground
	SetForegroundWindow(dynamic_cast<win_window_info &>(*osd_common_t::window_list().front()).platform_window());
}



//============================================================
//  winwindow_has_focus
//  (main or window thread)
//============================================================

bool winwindow_has_focus()
{
	// see if one of the video windows has focus
	for (const auto &window : osd_common_t::window_list())
	{
		switch (dynamic_cast<win_window_info &>(*window).focus())
		{
		case win_window_focus::NONE:
			break;

		case win_window_focus::THREAD:
			if (s_aggressive_focus)
				return true;
			break;

		case win_window_focus::WINDOW:
			return true;

		default:
			throw false;
		}
	}
	return false;
}


//============================================================
//  osd_set_aggressive_input_focus
//============================================================

void osd_set_aggressive_input_focus(bool aggressive_focus)
{
	s_aggressive_focus = aggressive_focus;
}


//============================================================
//  winwindow_update_cursor_state
//  (main thread)
//============================================================

void winwindow_update_cursor_state(running_machine &machine)
{
	assert(GetCurrentThreadId() == main_threadid);

	// If no windows, just return
	if (osd_common_t::window_list().empty())
		return;

	auto &window = static_cast<win_window_info &>(*osd_common_t::window_list().front());

	// if we should hide the mouse cursor, then do it
	// rules are:
	//   1. we must have focus before hiding the cursor
	//   2. we also hide the cursor in full screen mode and when the window doesn't have a menu
	//   3. we also hide the cursor in windowed mode if we're not paused and
	//      the input system requests it
	if (winwindow_has_focus() && (
		(window.fullscreen() && !GetMenu(window.platform_window()))
		|| (!machine.paused() && WINOSD(machine)->should_hide_mouse())))
	{
		// hide cursor
		window.hide_pointer();

		// clip pointer to game video window
		window.capture_pointer();
	}
	else
	{
		// show cursor
		window.show_pointer();

		// allow cursor to move freely
		window.release_pointer();
	}
}



//============================================================
//  winwindow_video_window_create
//  (main thread)
//============================================================

std::unique_ptr<win_window_info> win_window_info::create(
		running_machine &machine,
		render_module &renderprovider,
		int index,
		const std::shared_ptr<osd_monitor_info> &monitor,
		const osd_window_config *config)
{
	assert(GetCurrentThreadId() == main_threadid);

	// allocate a new window object
	auto window = std::make_unique<win_window_info>(machine, renderprovider, index, monitor, config);

	// set main window
	if (window->index() > 0)
	{
		for (const auto &w : osd_common_t::window_list())
		{
			if (w->index() == 0)
			{
				window->set_main_window(dynamic_cast<win_window_info &>(*w));
				break;
			}
		}
	}
	else
	{
		// We must be the main window
		window->set_main_window(*window);
	}

	// see if we are safe for fullscreen
	window->m_fullscreen_safe = TRUE;
	for (const auto &win : osd_common_t::window_list())
		if (win->monitor() == monitor.get())
			window->m_fullscreen_safe = FALSE;

	window->create_target();

	// remember the current values in case they change
	window->m_targetview = window->target()->view();
	window->m_targetorient = window->target()->orientation();
	window->m_targetlayerconfig = window->target()->layer_config();
	window->m_targetvismask = window->target()->visibility_mask();

	// set the initial maximized state
	window->m_startmaximized = downcast<windows_options &>(machine.options()).maximize();

	window->m_init_state = window->complete_create() ? -1 : 1;

	// handle error conditions
	if (window->m_init_state == -1)
		fatalerror("Unable to complete window creation\n");

	return window;
}

//============================================================
//  winwindow_video_window_destroy
//  (main thread)
//============================================================

void win_window_info::complete_destroy()
{
	assert(GetCurrentThreadId() == main_threadid);

	// destroy the window
	if (platform_window() != nullptr)
		DestroyWindow(platform_window());
}



//============================================================
//  winwindow_video_window_update
//  (main thread)
//============================================================

void win_window_info::update()
{
	assert(GetCurrentThreadId() == main_threadid);

	// see if the target has changed significantly in window mode
	unsigned const targetview = target()->view();
	int const targetorient = target()->orientation();
	render_layer_config const targetlayerconfig = target()->layer_config();
	u32 const targetvismask = target()->visibility_mask();
	int const targetscalemode = target()->scale_mode();
	bool const targetkeepaspect = target()->keepaspect();
	if (targetview != m_targetview || targetorient != m_targetorient || targetlayerconfig != m_targetlayerconfig || targetvismask != m_targetvismask ||
		targetscalemode != m_targetscalemode || targetkeepaspect != m_targetkeepaspect)
	{
		m_targetview = targetview;
		m_targetorient = targetorient;
		m_targetlayerconfig = targetlayerconfig;
		m_targetvismask = targetvismask;
		m_targetscalemode = targetscalemode;
		m_targetkeepaspect = targetkeepaspect;

		// in window mode, reminimize/maximize
		if (!fullscreen())
		{
			if (m_isminimized)
				SendMessage(platform_window(), WM_USER_SET_MINSIZE, 0, 0);
			if (m_ismaximized)
				SendMessage(platform_window(), WM_USER_SET_MAXSIZE, 0, 0);
		}
	}

	// if we're visible and running and not in the middle of a resize, draw
	if (platform_window() != nullptr && target() != nullptr && has_renderer())
	{
		bool got_lock = true;

		// only block if we're throttled
		if (machine().video().throttled() || timeGetTime() - last_update_time > 250)
			m_render_lock.lock();
		else
			got_lock = m_render_lock.try_lock();

		// only render if we were able to get the lock
		if (got_lock)
		{
			render_primitive_list *primlist;

			// don't hold the lock; we just used it to see if rendering was still happening
			m_render_lock.unlock();

			// ensure the target bounds are up-to-date, and then get the primitives
			primlist = renderer().get_primitives();

			// post a redraw request with the primitive list as a parameter
			last_update_time = timeGetTime();

			if (attached_mode())
			{
				HDC hdc = GetDC(platform_window());

				m_primlist = primlist;
				draw_video_contents(hdc, FALSE);

				ReleaseDC(platform_window(), hdc);
			}
			else
			{
				SendMessage(platform_window(), WM_USER_REDRAW, 0, (LPARAM)primlist);
			}
		}
	}
}

//============================================================
//  create_window_class
//  (main thread)
//============================================================

static void create_window_class()
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
//  winwindow_ui_pause
//  (main thread)
//============================================================

void winwindow_ui_pause(running_machine &machine, int pause)
{
	int old_temp_pause = ui_temp_pause;

	assert(GetCurrentThreadId() == main_threadid);

	if (pause)
	{
		// if we're pausing, increment the pause counter

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
	else
	{
		// if we're resuming, decrement the pause counter

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
		osd_printf_verbose("winwindow_ui_pause(): %d --> %d\n", old_temp_pause, ui_temp_pause);
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
	if (fullscreen())
		return 0;
	AdjustWindowRectEx(&temprect, WINDOW_STYLE, GetMenu(platform_window()) ? true : false, WINDOW_STYLE_EX);
	return rect_width(&temprect) - 100;
}



//============================================================
//  wnd_extra_height
//  (window thread)
//============================================================

int win_window_info::wnd_extra_height()
{
	RECT temprect = { 100, 100, 200, 200 };
	if (fullscreen())
		return 0;
	AdjustWindowRectEx(&temprect, WINDOW_STYLE, GetMenu(platform_window()) ? true : false, WINDOW_STYLE_EX);
	return rect_height(&temprect) - 100;
}


//============================================================
//  complete_create
//  (window thread)
//============================================================

int win_window_info::complete_create()
{
	RECT client;
	int tempwidth, tempheight;
	HDC dc;

	assert(GetCurrentThreadId() == window_threadid);

	// get the monitor bounds
	osd_rect monitorbounds = monitor()->position_size();

	// are we in worker UI mode?
	HWND hwnd;
	const char *attach_window_name = downcast<windows_options &>(machine().options()).attach_window();
	m_attached_mode = attach_window_name && *attach_window_name ? true : false;
	if (m_attached_mode)
	{
		// we are in worker UI mode; either this value is an HWND or a window name
		hwnd = (HWND)atoll(attach_window_name);
		if (!hwnd)
			hwnd = FindWindowEx(nullptr, nullptr, nullptr, osd::text::to_tstring(attach_window_name).c_str());
	}
	else
	{
		// create the window, but don't show it yet
		hwnd = win_create_window_ex_utf8(
				fullscreen() ? FULLSCREEN_STYLE_EX : WINDOW_STYLE_EX,
				"MAME",
				title().c_str(),
				fullscreen() ? FULLSCREEN_STYLE : WINDOW_STYLE,
				monitorbounds.left() + 20, monitorbounds.top() + 20,
				monitorbounds.left() + 100, monitorbounds.top() + 100,
				nullptr,//(osd_common_t::s_window_list != nullptr) ? osd_common_t::s_window_list->m_hwnd : nullptr,
				nullptr,
				GetModuleHandleUni(),
				nullptr);
	}

	if (hwnd == nullptr)
		return 1;

	set_platform_window(hwnd);

	// set a pointer back to us
	if (!attached_mode())
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)this);

	// skip the positioning stuff for '-video none' or '-attach_window'
	if (!renderer_interactive() || attached_mode())
	{
		renderer_create();
		if (renderer().create())
			return 1;
		return 0;
	}

	// adjust the window position to the initial width/height
	tempwidth = (m_win_config.width != 0) ? m_win_config.width : 640;
	tempheight = (m_win_config.height != 0) ? m_win_config.height : 480;
	SetWindowPos(platform_window(), nullptr, monitorbounds.left() + 20, monitorbounds.top() + 20,
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
	if (!fullscreen() || m_fullscreen_safe)
	{
		renderer_create();
		if (renderer().create())
			return 1;

		ShowWindow(platform_window(), SW_SHOW);
	}

	// clear the window
	dc = GetDC(platform_window());
	GetClientRect(platform_window(), &client);
	FillRect(dc, &client, (HBRUSH)GetStockObject(BLACK_BRUSH));
	ReleaseDC(platform_window(), dc);
	return 0;
}



//============================================================
//  winwindow_video_window_proc
//  (window thread)
//============================================================

LRESULT CALLBACK win_window_info::video_window_proc(HWND wnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	LONG_PTR ptr = GetWindowLongPtr(wnd, GWLP_USERDATA);
	auto *window = (win_window_info *)ptr;

	// we may get called before SetWindowLongPtr is called
	if (window)
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
			window->draw_video_contents(hdc, true);
			if (GetMenu(window->platform_window()))
				DrawMenuBar(window->platform_window());
			EndPaint(wnd, &pstruct);
		}
		break;

	// non-client paint: punt if full screen
	case WM_NCPAINT:
		if (!window->fullscreen() || GetMenu(window->platform_window()))
			return DefWindowProc(wnd, message, wparam, lparam);
		break;

	// input device change: handle RawInput device connection/disconnection
	case WM_INPUT_DEVICE_CHANGE:
		switch (wparam)
		{
		case GIDC_ARRIVAL:
			downcast<windows_osd_interface&>(window->machine().osd()).handle_input_event(INPUT_EVENT_ARRIVAL, &lparam);
			break;
		case GIDC_REMOVAL:
			downcast<windows_osd_interface&>(window->machine().osd()).handle_input_event(INPUT_EVENT_REMOVAL, &lparam);
			break;
		default:
			return DefWindowProc(wnd, message, wparam, lparam);
		}
		break;

	// input: handle the raw input
	case WM_INPUT:
		downcast<windows_osd_interface&>(window->machine().osd()).handle_input_event(INPUT_EVENT_RAWINPUT, &lparam);
		break;

	// syskeys - ignore
	case WM_SYSKEYUP:
	case WM_SYSKEYDOWN:
		break;

	// text input events
	case WM_CHAR:
		{
			char16_t const ch = char16_t(wparam);
			if ((0xd800 <= ch) && (0xdbff >= ch))
			{
				window->m_last_surrogate = ch;
			}
			else if ((0xdc00 <= ch) && (0xdfff >= ch))
			{
				if (window->m_last_surrogate)
				{
					char32_t const uch = 0x10000 + ((ch & 0x03ff) | ((window->m_last_surrogate & 0x03ff) << 10));
					window->machine().ui_input().push_char_event(window->target(), uch);
				}
				window->m_last_surrogate = 0;
			}
			else
			{
				window->machine().ui_input().push_char_event(window->target(), char32_t(ch));
				window->m_last_surrogate = 0;
			}
		}
		break;

	case WM_UNICHAR:
		if (UNICODE_NOCHAR == wparam)
			return TRUE;
		else
			window->machine().ui_input().push_char_event(window->target(), char32_t(wparam));
		break;

	// legacy mouse events
	case WM_MOUSEMOVE:
		window->mouse_updated(wparam, lparam);
		break;

	case WM_MOUSELEAVE:
		window->mouse_left(wparam, lparam);
		break;

	case WM_LBUTTONDOWN:
		window->mouse_updated(wparam, lparam);
		break;

	case WM_LBUTTONUP:
		window->mouse_updated(wparam, lparam);
		break;

	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
		window->mouse_updated(wparam, lparam);
		break;

	case WM_XBUTTONDOWN:
	case WM_XBUTTONUP:
		window->mouse_updated(wparam, lparam);
		return TRUE;

	case WM_MOUSEWHEEL:
		{
			POINT where{ GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam) };
			ScreenToClient(wnd, &where);
			UINT ucNumLines = 3; // default
			SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &ucNumLines, 0);
			window->machine().ui_input().push_mouse_wheel_event(window->target(), where.x, where.y, GET_WHEEL_DELTA_WPARAM(wparam), ucNumLines);
		}
		break;

	// new-style pointer handling (mouse/pen/touch)
	case WM_POINTERENTER:
		window->pointer_entered(wparam, lparam);
		break;
	case WM_POINTERLEAVE:
		window->pointer_left(wparam, lparam);
		break;
	case WM_POINTERDOWN:
	case WM_POINTERUP:
	case WM_POINTERUPDATE:
		window->pointer_updated(wparam, lparam);
		break;
	case WM_POINTERCAPTURECHANGED:
		window->pointer_capture_changed(wparam, lparam);
		break;
	// TODO: other pointer events?
	//case WM_POINTERACTIVATE:
	//case WM_POINTERDEVICECHANGE:
	//case WM_POINTERDEVICEINRANGE:
	//case WM_POINTERDEVICEOUTOFRANGE:
	//case WM_POINTERROUTEDAWAY:
	//case WM_POINTERROUTEDRELEASED:
	//case WM_POINTERROUTEDTO:
	//case WM_POINTERWHEEL:
	//case WM_POINTERHWHEEL:

	// pause the system when we start a menu or resize
	case WM_ENTERSIZEMOVE:
		window->m_resize_state = RESIZE_STATE_RESIZING;
		[[fallthrough]];
	case WM_ENTERMENULOOP:
		winwindow_ui_pause(window->machine(), TRUE);
		break;

	// unpause the system when we stop a menu or resize and force a redraw
	case WM_EXITSIZEMOVE:
		window->m_resize_state = RESIZE_STATE_PENDING;
		[[fallthrough]];
	case WM_EXITMENULOOP:
		winwindow_ui_pause(window->machine(), FALSE);
		InvalidateRect(wnd, nullptr, FALSE);
		break;

	// get min/max info: set the minimum window size
	case WM_GETMINMAXINFO:
		{
			auto *minmax = (MINMAXINFO *)lparam;
			minmax->ptMinTrackSize.x = MIN_WINDOW_DIMX;
			minmax->ptMinTrackSize.y = MIN_WINDOW_DIMY;
			break;
		}
		break;

	// sizing: constrain to the aspect ratio unless control key is held down
	case WM_SIZING:
		{
			RECT *rect = (RECT *)lparam;
			if (window->keepaspect() && (window->target()->scale_mode() == SCALE_FRACTIONAL) && !(GetAsyncKeyState(VK_CONTROL) & 0x8000))
			{
				osd_rect r = window->constrain_to_aspect_ratio(RECT_to_osd_rect(*rect), wparam);
				rect->top = r.top();
				rect->left = r.left();
				rect->bottom = r.bottom();
				rect->right = r.right();
			}
			InvalidateRect(wnd, nullptr, FALSE);
		}
		break;

	// syscommands: catch win_start_maximized
	case WM_SYSCOMMAND:
		{
			uint16_t cmd = wparam & 0xfff0;

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
		}
		return DefWindowProc(wnd, message, wparam, lparam);

	case WM_ACTIVATE:
		if (window->has_renderer())
		{
			if (window->fullscreen())
			{
				if ((wparam == WA_ACTIVE) || (wparam == WA_CLICKACTIVE))
				{
					for (const auto &w : osd_common_t::window_list())
						ShowWindow(dynamic_cast<win_window_info &>(*w).platform_window(), SW_RESTORE);
				}
				else if ((wparam == WA_INACTIVE) && !is_mame_window(HWND(lparam)))
				{
					for (const auto &w : osd_common_t::window_list())
						ShowWindow(dynamic_cast<win_window_info &>(*w).platform_window(), SW_MINIMIZE);
				}
			}

			if ((wparam == WA_ACTIVE) || (wparam == WA_CLICKACTIVE))
				window->machine().ui_input().push_window_focus_event(window->target());
			else if (wparam == WA_INACTIVE)
				window->machine().ui_input().push_window_defocus_event(window->target());
		}
		return DefWindowProc(wnd, message, wparam, lparam);

	// close: cause MAME to exit
	case WM_CLOSE:
		window->machine().schedule_exit();
		break;

	// destroy: clean up all attached rendering bits and nullptr out our hwnd
	case WM_DESTROY:
		window->renderer_reset();
		window->set_platform_window(nullptr);
		return DefWindowProc(wnd, message, wparam, lparam);

	// self redraw: draw ourself in a non-painty way
	case WM_USER_REDRAW:
		{
			HDC hdc = GetDC(wnd);

			window->m_primlist = (render_primitive_list *)lparam;
			window->draw_video_contents(hdc, false);

			ReleaseDC(wnd, hdc);
		}
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
		window->monitor()->refresh();
		window->monitor()->update_resolution(LOWORD(lparam), HIWORD(lparam));
		break;

	// set focus: if we're not the primary window, switch back
	// commented out ATM because this prevents us from resizing secondary windows
//  case WM_SETFOCUS:
//      if (window != osd_common_t::s_window_list && osd_common_t::s_window_list != nullptr)
//          SetFocus(osd_common_t::s_window_list->m_hwnd);
//      break;

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

void win_window_info::draw_video_contents(HDC dc, bool update)
{
	assert(GetCurrentThreadId() == window_threadid);

	std::lock_guard<std::mutex> lock(m_render_lock);

	// if we're iconic, don't bother
	if (platform_window() != nullptr && !IsIconic(platform_window()))
	{
		// if no bitmap, just fill
		if (m_primlist == nullptr)
		{
			RECT fill;
			GetClientRect(platform_window(), &fill);
			FillRect(dc, &fill, (HBRUSH)GetStockObject(BLACK_BRUSH));
		}

		// otherwise, render with our drawing system
		else
		{
			// update DC
			m_dc = dc;
			if (has_renderer())
			{
				renderer().draw(update);
			}
		}
	}
}


//============================================================
//  constrain_to_aspect_ratio
//  (window thread)
//============================================================

osd_rect win_window_info::constrain_to_aspect_ratio(const osd_rect &rect, int adjustment)
{
	assert(GetCurrentThreadId() == window_threadid);

	int32_t extrawidth = wnd_extra_width();
	int32_t extraheight = wnd_extra_height();
	int32_t propwidth, propheight;
	int32_t minwidth, minheight;
	int32_t maxwidth, maxheight;
	int32_t viswidth, visheight;
	int32_t adjwidth, adjheight;
	float pixel_aspect;

	auto monitor = monitor_from_rect(&rect);

	// Sometimes this gets called when monitors have already been torn down
	// In that the case, just return the unmodified rect
	if (monitor == nullptr)
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
			target()->compute_visible_area(10000, propheight, pixel_aspect, target()->orientation(), propwidth, propheight);
			break;

		case WMSZ_LEFT:
		case WMSZ_RIGHT:
			target()->compute_visible_area(propwidth, 10000, pixel_aspect, target()->orientation(), propwidth, propheight);
			break;

		default:
			target()->compute_visible_area(propwidth, propheight, pixel_aspect, target()->orientation(), propwidth, propheight);
			break;
	}

	// get the minimum width/height for the current layout
	target()->compute_minimum_size(minwidth, minheight);

	// clamp against the maximum (fit on one screen for full screen mode)
	if (fullscreen())
	{
		maxwidth = monitor->position_size().width() - extrawidth;
		maxheight = monitor->position_size().height() - extraheight;
	}
	else
	{
		maxwidth = monitor->usuable_position_size().width() - extrawidth;
		maxheight = monitor->usuable_position_size().height() - extraheight;

		// clamp minimum against half of maximum to allow resizing very large targets (eg. SVG screen)
		minwidth = std::min(minwidth, maxwidth / 2);
		minheight = std::min(minheight, maxheight / 2);

		// clamp maximum to the maximum width/height in the window
		if (m_win_config.width != 0)
			maxwidth = std::min(maxwidth, m_win_config.width + extrawidth);
		if (m_win_config.height != 0)
			maxheight = std::min(maxheight, m_win_config.height + extraheight);
	}

	// clamp against the absolute minimum
	propwidth = std::max(propwidth, MIN_WINDOW_DIMX);
	propheight = std::max(propheight, MIN_WINDOW_DIMY);

	// clamp against the minimum width and height
	propwidth = std::max(propwidth, minwidth);
	propheight = std::max(propheight, minheight);

	// clamp to the maximum
	propwidth = std::min(propwidth, maxwidth);
	propheight = std::min(propheight, maxheight);

	// compute the visible area based on the proposed rectangle
	target()->compute_visible_area(propwidth, propheight, pixel_aspect, target()->orientation(), viswidth, visheight);

	// clamp visable area to the proposed rectangle
	viswidth = std::min(viswidth, propwidth);
	visheight = std::min(visheight, propheight);

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
	int32_t minwidth, minheight;

	//assert(GetCurrentThreadId() == window_threadid);

	// get the minimum target size
	target()->compute_minimum_size(minwidth, minheight);

	// check if visible area is bigger
	int32_t viswidth, visheight;
	target()->compute_visible_area(minwidth, minheight, monitor()->aspect(), target()->orientation(), viswidth, visheight);
	minwidth = std::max(viswidth, minwidth);
	minheight = std::max(visheight, minheight);

	// expand to our minimum dimensions
	if (minwidth < MIN_WINDOW_DIMX)
		minwidth = MIN_WINDOW_DIMX;
	if (minheight < MIN_WINDOW_DIMY)
		minheight = MIN_WINDOW_DIMY;

	// account for extra window stuff
	minwidth += wnd_extra_width();
	minheight += wnd_extra_height();

	// if we want it constrained, figure out which one is larger
	if (constrain)
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
	//monitor()->refresh();
	osd_rect maximum = monitor()->usuable_position_size();

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
	if (constrain)
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

	if (!fullscreen())
	{
		RECT bounds;

		// compare the maximum bounds versus the current bounds
		const bool keep_aspect = keepaspect();
		osd_dim minbounds = get_min_bounds(keep_aspect);
		osd_dim maxbounds = get_max_bounds(keep_aspect);
		GetWindowRect(platform_window(), &bounds);

		// if either the width or height matches, we were maximized
		m_isminimized = (rect_width(&bounds) == minbounds.width()) ||
								(rect_height(&bounds) == minbounds.height());
		m_ismaximized = (rect_width(&bounds) == maxbounds.width()) ||
								(rect_height(&bounds) == maxbounds.height());

		// We can't be maximized and minimized simultaneously
		if (m_ismaximized)
			m_isminimized = FALSE;
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

	osd_dim newsize = get_min_bounds(keepaspect());

	// get the window rect
	RECT bounds;
	GetWindowRect(platform_window(), &bounds);

	osd_rect newrect(bounds.left, bounds.top, newsize);


	SetWindowPos(platform_window(), nullptr, newrect.left(), newrect.top(), newrect.width(), newrect.height(), SWP_NOZORDER);
}



//============================================================
//  maximize_window
//  (window thread)
//============================================================

void win_window_info::maximize_window()
{
	assert(GetCurrentThreadId() == window_threadid);

	osd_dim newsize = get_max_bounds(keepaspect());

	// center within the work area
	osd_rect work = monitor()->usuable_position_size();
	osd_rect newrect = osd_rect(work.left() + (work.width() - newsize.width()) / 2,
			work.top() + (work.height() - newsize.height()) / 2,
			newsize);

	SetWindowPos(platform_window(), nullptr, newrect.left(), newrect.top(), newrect.width(), newrect.height(), SWP_NOZORDER);
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
	GetWindowRect(platform_window(), &oldrect);
	osd_rect newrect = RECT_to_osd_rect(oldrect);

	// adjust the window size so the client area is what we want
	if (!fullscreen())
	{
		// constrain the existing size to the aspect ratio
		if (keepaspect())
			newrect = constrain_to_aspect_ratio(newrect, WMSZ_BOTTOMRIGHT);

		// restrict the window to one monitor and avoid toolbars if possible
		HMONITOR const nearest_monitor = MonitorFromWindow(platform_window(), MONITOR_DEFAULTTONEAREST);
		if (NULL != nearest_monitor)
		{
			MONITORINFO info;
			std::memset(&info, 0, sizeof(info));
			info.cbSize = sizeof(info);
			if (GetMonitorInfo(nearest_monitor, &info))
			{
				if (newrect.right() > info.rcWork.right)
					newrect = newrect.move_by(info.rcWork.right - newrect.right(), 0);
				if (newrect.bottom() > info.rcWork.bottom)
					newrect = newrect.move_by(0, info.rcWork.bottom - newrect.bottom());
				if (newrect.left() < info.rcWork.left)
					newrect = newrect.move_by(info.rcWork.left - newrect.left(), 0);
				if (newrect.top() < info.rcWork.top)
					newrect = newrect.move_by(0, info.rcWork.top - newrect.top());
			}
		}
	}
	else
	{
		// in full screen, make sure it covers the primary display
		std::shared_ptr<osd_monitor_info> monitor = monitor_from_rect(nullptr);
		newrect = monitor->position_size();
	}

	// adjust the position if different
	if (RECT_to_osd_rect(oldrect) != newrect)
		SetWindowPos(platform_window(), fullscreen() ? HWND_TOPMOST : HWND_TOP,
				newrect.left(), newrect.top(),
				newrect.width(), newrect.height(), 0);

	// take note of physical window size (used for lightgun coordinate calculation)
	if (index() == 0)
		osd_printf_verbose("Physical width %d, height %d\n", newrect.width(), newrect.height());
}


//============================================================
//  set_fullscreen
//  (window thread)
//============================================================

void win_window_info::set_fullscreen(int fullscreen)
{
	assert(GetCurrentThreadId() == window_threadid);

	// if we're in the right state, punt
	if (this->fullscreen() == fullscreen)
		return;
	m_fullscreen = fullscreen;

	// kill off the renderer
	renderer_reset();

	// hide ourself
	ShowWindow(platform_window(), SW_HIDE);

	if (!fullscreen)
	{
		// configure the window if non-fullscreen

		// adjust the style
		SetWindowLong(platform_window(), GWL_STYLE, WINDOW_STYLE);
		SetWindowLong(platform_window(), GWL_EXSTYLE, WINDOW_STYLE_EX);
		SetWindowPos(platform_window(), nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

		// force to the bottom, then back on top
		SetWindowPos(platform_window(), HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		SetWindowPos(platform_window(), HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

		// if we have previous non-fullscreen bounds, use those
		if (m_non_fullscreen_bounds.right != m_non_fullscreen_bounds.left)
		{
			SetWindowPos(platform_window(), HWND_TOP, m_non_fullscreen_bounds.left, m_non_fullscreen_bounds.top,
						rect_width(&m_non_fullscreen_bounds), rect_height(&m_non_fullscreen_bounds),
						SWP_NOZORDER);
		}

		// otherwise, set a small size and maximize from there
		else
		{
			SetWindowPos(platform_window(), HWND_TOP, 0, 0, MIN_WINDOW_DIMX, MIN_WINDOW_DIMY, SWP_NOZORDER);
			maximize_window();
		}
	}
	else
	{
		// configure the window if fullscreen

		// save the bounds
		GetWindowRect(platform_window(), &m_non_fullscreen_bounds);

		// adjust the style
		SetWindowLong(platform_window(), GWL_STYLE, FULLSCREEN_STYLE);
		SetWindowLong(platform_window(), GWL_EXSTYLE, FULLSCREEN_STYLE_EX);
		SetWindowPos(platform_window(), nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

		// set topmost
		SetWindowPos(platform_window(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	}

	// adjust the window to compensate for the change
	adjust_window_position_after_major_change();

	// show ourself
	if (!this->fullscreen() || m_fullscreen_safe)
	{
		if (renderer_interactive())
			ShowWindow(platform_window(), SW_SHOW);

		renderer_create();
		if (renderer().create())
			exit(1); // FIXME: better error handling than just silently exiting on failure
	}

	// ensure we're still adjusted correctly
	adjust_window_position_after_major_change();
}


//============================================================
//  focus
//  (main or window thread)
//============================================================

win_window_focus win_window_info::focus() const
{
	HWND focuswnd = nullptr;
	if (attached_mode())
	{
		// if this window is in attached mode, we need to see if it has
		// focus in its context; first find out what thread owns it
		DWORD window_thread_id = GetWindowThreadProcessId(platform_window(), nullptr);

		// and then identify which window has focus in that thread's context
		GUITHREADINFO gti;
		gti.cbSize = sizeof(gti);
		focuswnd = GetGUIThreadInfo(window_thread_id, &gti)
			? gti.hwndFocus
			: nullptr;
	}
	else
	{
		// life is simpler in non-attached mode
		focuswnd = GetFocus();
	}

	if (focuswnd == platform_window())
		return win_window_focus::WINDOW;
	else if (focuswnd)
		return win_window_focus::THREAD;
	else
		return win_window_focus::NONE;
}


void win_window_info::pointer_entered(WPARAM wparam, LPARAM lparam)
{
	assert(!IS_POINTER_CANCELED_WPARAM(wparam));
	auto const info(map_pointer(wparam));
	if (m_active_pointers.end() != info)
	{
		POINT where{ GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam) };
		ScreenToClient(platform_window(), &where);
		info->x = where.x;
		info->y = where.y;
		machine().ui_input().push_pointer_update(
				target(),
				convert_pointer_type(info->type),
				info->index,
				info->device,
				where.x, where.y,
				0U, 0U, 0U, 0U);
	}
}

void win_window_info::pointer_left(WPARAM wparam, LPARAM lparam)
{
	auto const info(find_pointer(wparam));
	if (m_active_pointers.end() != info)
	{
		bool const canceled(IS_POINTER_CANCELED_WPARAM(wparam));
		POINT where{ GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam) };
		ScreenToClient(platform_window(), &where);
		expire_pointer(info, where, canceled);
	}
}

void win_window_info::pointer_updated(WPARAM wparam, LPARAM lparam)
{
	bool const canceled(IS_POINTER_CANCELED_WPARAM(wparam));
	auto const info(canceled ? find_pointer(wparam) : map_pointer(wparam));
	if (m_active_pointers.end() != info)
	{
		POINT where{ GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam) };
		ScreenToClient(platform_window(), &where);
		unsigned const buttons(canceled ? 0 : get_pointer_buttons(wparam));
		update_pointer(*info, where, buttons, canceled);
	}
}

void win_window_info::pointer_capture_changed(WPARAM wparam, LPARAM lparam)
{
	auto const info(find_pointer(wparam));
	if (m_active_pointers.end() != info)
	{
		// treat this as the pointer being stolen - fail any gestures
		if (BIT(info->buttons, 0) && (0 < info->clickcnt))
			info->clickcnt = -info->clickcnt;

		// push to UI manager and dump pointer data
		machine().ui_input().push_pointer_abort(
				target(),
				convert_pointer_type(info->type),
				info->index,
				info->device,
				info->x, info->y,
				info->buttons, info->clickcnt);
		m_pointer_mask &= ~(decltype(m_pointer_mask)(1) << info->index);
		if (info->index < m_next_pointer)
			m_next_pointer = info->index;
		m_active_pointers.erase(info);
	}
}

void win_window_info::mouse_left(WPARAM wparam, LPARAM lparam)
{
	m_tracking_mouse = false;
	auto const info(find_mouse_pointer());
	if (m_active_pointers.end() != info)
	{
		POINT where{ GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam) };
		expire_pointer(info, where, false);
	}
}

void win_window_info::mouse_updated(WPARAM wparam, LPARAM lparam)
{
	auto const info(map_mouse_pointer());
	if (m_active_pointers.end() == info)
		return;

	POINT const where{ GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam) };
	unsigned const buttons(
			((MK_LBUTTON  & wparam) ? 0x01 : 0x00) |
			((MK_RBUTTON  & wparam) ? 0x02 : 0x00) |
			((MK_MBUTTON  & wparam) ? 0x04 : 0x00) |
			((MK_XBUTTON1 & wparam) ? 0x08 : 0x00) |
			((MK_XBUTTON2 & wparam) ? 0x10 : 0x00));

	// continue to track the mouse outside the window if buttons are down
	if (buttons && !info->buttons)
	{
		SetCapture(platform_window());

		TRACKMOUSEEVENT tm;
		std::memset(&tm, 0, sizeof(tm));
		tm.cbSize = sizeof(tm);
		tm.dwFlags = TME_CANCEL | TME_LEAVE;
		tm.hwndTrack = platform_window();
		TrackMouseEvent(&tm);
		m_tracking_mouse = false;
	}
	else
	{
		if (!buttons && info->buttons)
		{
			if (GetCapture() == platform_window())
				ReleaseCapture();
		}

		if (!m_tracking_mouse)
		{
			TRACKMOUSEEVENT tm;
			std::memset(&tm, 0, sizeof(tm));
			tm.cbSize = sizeof(tm);
			tm.dwFlags = TME_LEAVE;
			tm.hwndTrack = platform_window();
			TrackMouseEvent(&tm);
			m_tracking_mouse = true;
		}
	}

	update_pointer(*info, where, buttons, false);
}

void win_window_info::expire_pointer(std::vector<win_pointer_info>::iterator info, POINT const &where, bool canceled)
{
	// leaving implicitly releases buttons, so check hold/drag if necessary
	if (BIT(info->buttons, 0) && (0 < info->clickcnt))
	{
		if (!canceled)
		{
			auto const now(std::chrono::steady_clock::now());
			auto const exp(std::chrono::milliseconds(GetDoubleClickTime()) + info->pressed);
			int const dx(where.x - info->pressedx);
			int const dy(where.y - info->pressedy);
			int const distance((dx * dx) + (dy * dy));
			int const tolerance((PT_TOUCH == info->type) ? TAP_DISTANCE : CLICK_DISTANCE);
			if ((exp < now) || (tolerance < distance))
				info->clickcnt = -info->clickcnt;
		}
		else
		{
			info->clickcnt = -info->clickcnt;
		}
	}

	// need to remember touches to recognise multi-tap gestures
	if (!canceled && (PT_TOUCH == info->type) && (0 < info->clickcnt))
	{
		auto const now(std::chrono::steady_clock::now());
		auto const time = std::chrono::milliseconds(GetDoubleClickTime());
		if ((time + info->pressed) >= now)
		{
			try
			{
				unsigned i(0);
				if (m_ptrdev_info.size() > info->device)
					i = m_ptrdev_info[info->device].clear_expired_touches(now, time);
				else
					m_ptrdev_info.resize(info->device + 1);

				if (std::size(m_ptrdev_info[info->device].touches) > i)
				{
					m_ptrdev_info[info->device].touches[i].when = info->pressed;
					m_ptrdev_info[info->device].touches[i].x = info->pressedx;
					m_ptrdev_info[info->device].touches[i].y = info->pressedy;
					m_ptrdev_info[info->device].touches[i].cnt = info->clickcnt;
				}
			}
			catch (std::bad_alloc const &)
			{
				osd_printf_error("win_window_info: error allocating pointer data\n");
			}
		}
	}

	// push to UI manager and dump pointer data
	if (!canceled)
	{
		machine().ui_input().push_pointer_leave(
				target(),
				convert_pointer_type(info->type),
				info->index,
				info->device,
				where.x, where.y,
				info->buttons, info->clickcnt);
	}
	else
	{
		machine().ui_input().push_pointer_abort(
				target(),
				convert_pointer_type(info->type),
				info->index,
				info->device,
				where.x, where.y,
				info->buttons, info->clickcnt);
	}
	m_pointer_mask &= ~(decltype(m_pointer_mask)(1) << info->index);
	if (info->index < m_next_pointer)
		m_next_pointer = info->index;
	m_active_pointers.erase(info);
}

void win_window_info::update_pointer(win_pointer_info &info, POINT const &where, unsigned buttons, bool canceled)
{
	if (!canceled && (where.x == info.x) && (where.y == info.y) && (buttons == info.buttons))
		return;

	// detect multi-click actions
	unsigned const pressed(canceled ? 0 : (buttons & ~info.buttons));
	unsigned const released(canceled ? ~info.buttons : (~buttons & info.buttons));
	if (BIT(pressed, 0))
	{
		info.primary_down(
				where.x, where.y,
				std::chrono::milliseconds(GetDoubleClickTime()),
				(PT_TOUCH == info.type) ? TAP_DISTANCE : CLICK_DISTANCE,
				PT_TOUCH == info.type,
				m_ptrdev_info);
	}
	else if (BIT(info.buttons, 0))
	{
		info.check_primary_hold_drag(
				where.x, where.y,
				std::chrono::milliseconds(GetDoubleClickTime()),
				(PT_TOUCH == info.type) ? TAP_DISTANCE : CLICK_DISTANCE);
	}

	// update info and push to UI manager
	info.x = where.x;
	info.y = where.y;
	info.buttons = buttons;
	if (!canceled)
	{
		machine().ui_input().push_pointer_update(
				target(),
				convert_pointer_type(info.type),
				info.index,
				info.device,
				where.x, where.y,
				buttons, pressed, released, info.clickcnt);
	}
	else
	{
		machine().ui_input().push_pointer_abort(
				target(),
				convert_pointer_type(info.type),
				info.index,
				info.device,
				where.x, where.y,
				released, info.clickcnt);
	}
}

std::vector<win_window_info::win_pointer_info>::iterator win_window_info::map_pointer(WPARAM wparam)
{
	WORD const ptrid(GET_POINTERID_WPARAM(wparam));
	auto found(std::lower_bound(m_active_pointers.begin(), m_active_pointers.end(), ptrid, &win_pointer_info::compare));
	if ((m_active_pointers.end() != found) && (found->ptrid == ptrid))
		return found;

	if ((sizeof(m_next_pointer) * 8) <= m_next_pointer)
	{
		assert(~decltype(m_pointer_mask)(0) == m_pointer_mask);
		osd_printf_warning("win_window_info: exceeded maximum number of active pointers\n");
		return m_active_pointers.end();
	}
	assert(!BIT(m_pointer_mask, m_next_pointer));

	POINTER_INFO info = { 0 };
	if (!OSD_DYNAMIC_CALL(GetPointerInfo, ptrid, &info))
	{
		osd_printf_error("win_window_info: failed to get info for pointer ID %u\n", ptrid);
		return m_active_pointers.end();
	}

	auto devpos(std::lower_bound(
			m_ptrdev_map.begin(),
			m_ptrdev_map.end(),
			info.sourceDevice,
			[] (std::pair<HANDLE, unsigned> const &mapping, HANDLE device)
			{
				return mapping.first < device;
			}));

	try
	{
		if ((m_ptrdev_map.end() == devpos) || (devpos->first != info.sourceDevice))
		{
			devpos = m_ptrdev_map.emplace(devpos, info.sourceDevice, m_next_ptrdev);
			++m_next_ptrdev;
		}

		found = m_active_pointers.emplace(
				found,
				win_pointer_info(ptrid, info.pointerType, m_next_pointer, devpos->second));
		m_pointer_mask |= decltype(m_pointer_mask)(1) << m_next_pointer;
		do
		{
			++m_next_pointer;
		}
		while (((sizeof(m_next_pointer) * 8) > m_next_pointer) && BIT(m_pointer_mask, m_next_pointer));

		return found;
	}
	catch (std::bad_alloc const &)
	{
		osd_printf_error("win_window_info: error allocating pointer data\n");
		return m_active_pointers.end();
	}
}

std::vector<win_window_info::win_pointer_info>::iterator win_window_info::find_pointer(WPARAM wparam)
{
	WORD const ptrid(GET_POINTERID_WPARAM(wparam));
	auto const found(std::lower_bound(m_active_pointers.begin(), m_active_pointers.end(), ptrid, &win_pointer_info::compare));
	if ((m_active_pointers.end() != found) && (found->ptrid == ptrid))
		return found;
	else
		return m_active_pointers.end();
}

std::vector<win_window_info::win_pointer_info>::iterator win_window_info::map_mouse_pointer()
{
	auto found(std::lower_bound(m_active_pointers.begin(), m_active_pointers.end(), MOUSE_POINTER_ID, &win_pointer_info::compare));
	if ((m_active_pointers.end() != found) && (found->ptrid == MOUSE_POINTER_ID))
		return found;

	if ((sizeof(m_next_pointer) * 8) <= m_next_pointer)
	{
		assert(~decltype(m_pointer_mask)(0) == m_pointer_mask);
		osd_printf_warning("win_window_info: exceeded maximum number of active pointers\n");
		return m_active_pointers.end();
	}
	assert(!BIT(m_pointer_mask, m_next_pointer));

	auto devpos(std::lower_bound(
			m_ptrdev_map.begin(),
			m_ptrdev_map.end(),
			INVALID_HANDLE_VALUE,
			[] (std::pair<HANDLE, unsigned> const &mapping, HANDLE device)
			{
				return mapping.first < device;
			}));

	try
	{
		if ((m_ptrdev_map.end() == devpos) || (devpos->first != INVALID_HANDLE_VALUE))
		{
			devpos = m_ptrdev_map.emplace(devpos, INVALID_HANDLE_VALUE, m_next_ptrdev);
			++m_next_ptrdev;
		}

		found = m_active_pointers.emplace(
				found,
				win_pointer_info(MOUSE_POINTER_ID, PT_MOUSE, m_next_pointer, devpos->second));
		m_pointer_mask |= decltype(m_pointer_mask)(1) << m_next_pointer;
		do
		{
			++m_next_pointer;
		}
		while (((sizeof(m_next_pointer) * 8) > m_next_pointer) && BIT(m_pointer_mask, m_next_pointer));

		return found;
	}
	catch (std::bad_alloc const &)
	{
		osd_printf_error("win_window_info: error allocating pointer data\n");
		return m_active_pointers.end();
	}
}

std::vector<win_window_info::win_pointer_info>::iterator win_window_info::find_mouse_pointer()
{
	auto const found(std::lower_bound(m_active_pointers.begin(), m_active_pointers.end(), MOUSE_POINTER_ID, &win_pointer_info::compare));
	if ((m_active_pointers.end() != found) && (found->ptrid == MOUSE_POINTER_ID))
		return found;
	else
		return m_active_pointers.end();
}


#if (USE_QTDEBUG)
//============================================================
//  winwindow_qt_filter
//============================================================

bool winwindow_qt_filter(void *message)
{
	MSG *const msg = reinterpret_cast<MSG *>(message);

	if (is_mame_window(msg->hwnd) || (!msg->hwnd && (msg->message >= WM_USER)))
	{
		LONG_PTR ptr;
		if (msg->hwnd) // get the machine associated with this window
			ptr = GetWindowLongPtr(msg->hwnd, GWLP_USERDATA);
		else // any one will have to do
			ptr = (LONG_PTR)osd_common_t::window_list().front().get();

		winwindow_dispatch_message(reinterpret_cast<win_window_info *>(ptr)->machine(), *msg);
		return true;
	}
	return false;
}
#endif
