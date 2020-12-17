// license:BSD-3-Clause
// copyright-holders:Aaron Giles
//============================================================
//
//  window.c - Win32 window handling
//
//============================================================

#define LOG_TEMP_PAUSE      0

// Needed for RAW Input
#define WM_INPUT 0x00FF

// standard C headers
#include <process.h>

#include <atomic>
#include <chrono>
#include <list>
#include <memory>

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

#include "modules/monitor/monitor_common.h"

#include <agile.h>
using namespace Windows::UI::Core;

#include "modules/render/drawbgfx.h"
#include "modules/render/drawnone.h"

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
#define WM_USER_REDRAW                  (WM_USER + 2)
#define WM_USER_SET_FULLSCREEN          (WM_USER + 3)
#define WM_USER_SET_MAXSIZE             (WM_USER + 4)
#define WM_USER_SET_MINSIZE             (WM_USER + 5)



//============================================================
//  GLOBAL VARIABLES
//============================================================

static DWORD main_threadid;

// actual physical resolution
static int win_physical_width;
static int win_physical_height;



//============================================================
//  LOCAL VARIABLES
//============================================================

// event handling
static std::chrono::system_clock::time_point last_event_check;

// debugger
static int in_background;

static int ui_temp_pause;
static int ui_temp_was_paused;

static HANDLE window_thread;
static DWORD window_threadid;

static std::chrono::time_point<std::chrono::high_resolution_clock> last_update_time;

static HANDLE ui_pause_event;

//============================================================
//  window_init
//  (main thread)
//============================================================

bool windows_osd_interface::window_init()
{
	// get the main thread ID before anything else
	main_threadid = GetCurrentThreadId();

	// create an event to signal UI pausing
	ui_pause_event = CreateEvent(nullptr, TRUE, FALSE, nullptr);
	if (!ui_pause_event)
		fatalerror("Failed to create pause event\n");

	window_thread = GetCurrentThread();
	window_threadid = main_threadid;

	const int fallbacks[VIDEO_MODE_COUNT] = {
		-1,                 // NONE -> no fallback
		VIDEO_MODE_NONE,    // GDI -> NONE
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
			case VIDEO_MODE_BGFX:
				error = renderer_bgfx::init(machine());
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

	return true;
}

void windows_osd_interface::update_slider_list()
{
	for (auto window : osd_common_t::s_window_list)
	{
		// check if any window has dirty sliders
		if (window->has_renderer() && window->renderer().sliders_dirty())
		{
			build_slider_list();
			return;
		}
	}
}

int windows_osd_interface::window_count()
{
	return osd_common_t::s_window_list.size();
}

void windows_osd_interface::build_slider_list()
{
	m_sliders.clear();

	for (auto window : osd_common_t::s_window_list)
	{
		if (window->has_renderer())
		{
			// take the sliders of the first window
			std::vector<ui::menu_item> window_sliders = window->renderer().get_slider_list();
			m_sliders.insert(m_sliders.end(), window_sliders.begin(), window_sliders.end());
		}
	}
}

void windows_osd_interface::add_audio_to_recording(const int16_t *buffer, int samples_this_frame)
{
	auto window = osd_common_t::s_window_list.front(); // We only record on the first window
	if (window != nullptr)
	{
		window->renderer().add_audio_to_recording(buffer, samples_this_frame);
	}
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
		auto window = osd_common_t::s_window_list.front();

		// Destroy removes it from the list also
		window->destroy();
	}

	switch(video_config.mode)
	{
		case VIDEO_MODE_NONE:
			renderer_none::exit();
			break;
		case VIDEO_MODE_BGFX:
			renderer_bgfx::exit();
			break;
		default:
			break;
	}

	// kill the UI pause event
	if (ui_pause_event)
		CloseHandle(ui_pause_event);
}

uwp_window_info::uwp_window_info(
	running_machine &machine,
	int index,
	std::shared_ptr<osd_monitor_info> monitor,
	const osd_window_config *config) : osd_window_t(*config),
		m_next(nullptr),
		m_init_state(0),
		m_startmaximized(0),
		m_isminimized(0),
		m_ismaximized(0),
		m_monitor(monitor),
		m_fullscreen(!video_config.windowed),
		m_fullscreen_safe(0),
		m_aspect(0),
		m_target(nullptr),
		m_targetview(0),
		m_targetorient(0),
		m_targetvismask(0),
		m_lastclicktime(std::chrono::system_clock::time_point::min()),
		m_lastclickx(0),
		m_lastclicky(0),
		m_machine(machine)
{
	memset(m_title,0,sizeof(m_title));
	m_non_fullscreen_bounds.left = 0;
	m_non_fullscreen_bounds.top = 0;
	m_non_fullscreen_bounds.right  = 0;
	m_non_fullscreen_bounds.bottom = 0;
	m_prescale = video_config.prescale;
}

POINT uwp_window_info::s_saved_cursor_pos = { -1, -1 };

CoreCursor^ uwp_window_info::s_cursor = nullptr;

void uwp_window_info::capture_pointer()
{
	platform_window()->SetPointerCapture();
}

void uwp_window_info::release_pointer()
{
	platform_window()->ReleasePointerCapture();
}

void uwp_window_info::hide_pointer()
{
	auto window = platform_window();
	uwp_window_info::s_cursor = window->PointerCursor;
	window->PointerCursor = nullptr;
}

void uwp_window_info::show_pointer()
{
	auto window = platform_window();
	window->PointerCursor = uwp_window_info::s_cursor;
}

//============================================================
//  winwindow_process_events_periodic
//  (main thread)
//============================================================

void winwindow_process_events_periodic(running_machine &machine)
{
	auto currticks = std::chrono::system_clock::now();

	assert(GetCurrentThreadId() == main_threadid);

	// update once every 1/8th of a second
	if (currticks - last_event_check < std::chrono::milliseconds(1000 / 8))
		return;
	winwindow_process_events(machine, true, false);
}

//============================================================
//  winwindow_has_focus
//  (main or window thread)
//============================================================

bool winwindow_has_focus(void)
{
	// For now always act like we have focus
	return true;
}

//============================================================
//  winwindow_process_events
//  (main thread)
//============================================================

void winwindow_process_events(running_machine &machine, bool ingame, bool nodispatch)
{
//  MSG message;

	assert(GetCurrentThreadId() == main_threadid);

	// remember the last time we did this
	last_event_check = std::chrono::system_clock::now();

	try
	{
		CoreWindow^ window = CoreWindow::GetForCurrentThread();
		window->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);
	}
	catch (Platform::DisconnectedException^)
	{
		// This can get thrown when the window is being destroyed
	}
}


//============================================================
//  winwindow_video_window_create
//  (main thread)
//============================================================

void uwp_window_info::create(running_machine &machine, int index, std::shared_ptr<osd_monitor_info> monitor, const osd_window_config *config)
{
	assert(GetCurrentThreadId() == main_threadid);

	// allocate a new window object
	auto window = std::make_shared<uwp_window_info>(machine, index, monitor, config);

	// set main window
	if (window->m_index > 0)
	{
		for (auto w : osd_common_t::s_window_list)
		{
			if (w->m_index == 0)
			{
				window->set_main_window(std::static_pointer_cast<osd_window>(w));
				break;
			}
		}
	}
	else
	{
		// We must be the main window
		window->set_main_window(window);
	}

	// see if we are safe for fullscreen
	window->m_fullscreen_safe = TRUE;
	for (auto win : osd_common_t::s_window_list)
		if (win->monitor() == monitor.get())
			window->m_fullscreen_safe = FALSE;

	// add us to the list
	osd_common_t::s_window_list.push_back(window);

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
	window->m_targetvismask = window->m_target->visibility_mask();

	// make the window title
	if (video_config.numscreens == 1)
		sprintf(window->m_title, "%s: %s [%s]", emulator_info::get_appname(), machine.system().type.fullname(), machine.system().name);
	else
		sprintf(window->m_title, "%s: %s [%s] - Screen %d", emulator_info::get_appname(), machine.system().type.fullname(), machine.system().name, index);

	// set the initial maximized state
	window->m_startmaximized = options.maximize();

	window->m_init_state = window->complete_create() ? -1 : 1;

	// handle error conditions
	if (window->m_init_state == -1)
		fatalerror("Unable to complete window creation\n");
}

std::shared_ptr<osd_monitor_info> uwp_window_info::monitor_from_rect(const osd_rect* proposed) const
{
	std::shared_ptr<osd_monitor_info> monitor;

	// in window mode, find the nearest
	if (!fullscreen())
	{
		if (proposed != nullptr)
		{
			monitor = m_monitor->module().monitor_from_rect(*proposed);
		}
		else
			monitor = m_monitor->module().monitor_from_window(*this);
	}
	else
	{
		// in full screen, just use the configured monitor
		monitor = m_monitor;
	}

	return monitor;
}

//============================================================
//  winwindow_video_window_destroy
//  (main thread)
//============================================================

void uwp_window_info::destroy()
{
	assert(GetCurrentThreadId() == main_threadid);

	// remove us from the list
	osd_common_t::s_window_list.remove(shared_from_this());

	// destroy the window
//  if (platform_window<HWND>() != nullptr)
	//  DestroyWindow(platform_window<HWND>());

	// free the render target
	machine().render().target_free(m_target);
	m_target = nullptr;
}



//============================================================
//  winwindow_video_window_update
//  (main thread)
//============================================================

void uwp_window_info::update()
{
	assert(GetCurrentThreadId() == main_threadid);

	// see if the target has changed significantly in window mode
	unsigned const targetview = m_target->view();
	int const targetorient = m_target->orientation();
	render_layer_config const targetlayerconfig = m_target->layer_config();
	u32 const targetvismask = m_target->visibility_mask();
	if (targetview != m_targetview || targetorient != m_targetorient || targetlayerconfig != m_targetlayerconfig || targetvismask != m_targetvismask)
	{
		m_targetview = targetview;
		m_targetorient = targetorient;
		m_targetlayerconfig = targetlayerconfig;
		m_targetvismask = targetvismask;

		// in window mode, reminimize/maximize
		if (!fullscreen())
		{
			//if (m_isminimized)
				//SendMessage(platform_window<HWND>(), WM_USER_SET_MINSIZE, 0, 0);
			//if (m_ismaximized)
//              SendMessage(platform_window<HWND>(), WM_USER_SET_MAXSIZE, 0, 0);
		}
	}

	// if we're visible and running and not in the middle of a resize, draw
	if (platform_window() != nullptr && m_target != nullptr && has_renderer())
	{
		bool got_lock = true;
		auto clock = std::chrono::high_resolution_clock();

		// only block if we're throttled
		if (machine().video().throttled() || clock.now() - last_update_time > std::chrono::milliseconds(250))
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
			last_update_time = clock.now();

			// Actually perform the redraw
			m_primlist = primlist;
			draw_video_contents(false);
		}
	}
}

//============================================================
//  draw_video_contents
//  (window thread)
//============================================================

void uwp_window_info::draw_video_contents(bool update)
{
	assert(GetCurrentThreadId() == window_threadid);

	std::lock_guard<std::mutex> lock(m_render_lock);

	// if we're iconic, don't bother
	if (platform_window() != nullptr)
	{
		// if no bitmap, just fill
		if (m_primlist == nullptr)
		{
			// For now do nothing, eventually we need to clear the window
		}

		// otherwise, render with our drawing system
		else
		{
			renderer().draw(update);
		}
	}
}


//============================================================
//  set_starting_view
//  (main thread)
//============================================================

void uwp_window_info::set_starting_view(int index, const char *defview, const char *view)
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
//  winwindow_ui_pause
//  (main thread)
//============================================================

void winwindow_ui_pause(running_machine &machine, int pause)
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

int uwp_window_info::wnd_extra_width()
{
	RECT temprect = { 100, 100, 200, 200 };
	if (fullscreen())
		return 0;
	//AdjustWindowRectEx(&temprect, WINDOW_STYLE, win_has_menu(), WINDOW_STYLE_EX);
	return rect_width(&temprect) - 100;
}



//============================================================
//  wnd_extra_height
//  (window thread)
//============================================================

int uwp_window_info::wnd_extra_height()
{
	RECT temprect = { 100, 100, 200, 200 };
	if (fullscreen())
		return 0;
	//AdjustWindowRectEx(&temprect, WINDOW_STYLE, win_has_menu(), WINDOW_STYLE_EX);
	return rect_height(&temprect) - 100;
}


//============================================================
//  complete_create
//  (window thread)
//============================================================

int uwp_window_info::complete_create()
{
	int tempwidth, tempheight;

	assert(GetCurrentThreadId() == window_threadid);

	// get the monitor bounds
	osd_rect monitorbounds = m_monitor->position_size();

	auto coreWindow = Platform::Agile<CoreWindow>(CoreWindow::GetForCurrentThread());
	set_platform_window(coreWindow);

	// skip the positioning stuff for -video none */
	if (video_config.mode == VIDEO_MODE_NONE)
	{
		set_renderer(osd_renderer::make_for_type(video_config.mode, shared_from_this()));
		renderer().create();
		return 0;
	}

	// adjust the window position to the initial width/height
	tempwidth = (m_win_config.width != 0) ? m_win_config.width : 640;
	tempheight = (m_win_config.height != 0) ? m_win_config.height : 480;

	// maximum or minimize as appropriate
	if (m_startmaximized)
		maximize_window();
	else
		minimize_window();
	adjust_window_position_after_major_change();

	// show the window
	if (!fullscreen() || m_fullscreen_safe)
	{
		set_renderer(osd_renderer::make_for_type(video_config.mode, shared_from_this()));
		if (renderer().create())
			return 1;

	}

	return 0;
}


//============================================================
//  constrain_to_aspect_ratio
//  (window thread)
//============================================================

osd_rect uwp_window_info::constrain_to_aspect_ratio(const osd_rect &rect, int adjustment)
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
	propwidth = std::max(propwidth, MIN_WINDOW_DIM);
	propheight = std::max(propheight, MIN_WINDOW_DIM);

	// clamp against the minimum width and height
	propwidth = std::max(propwidth, minwidth);
	propheight = std::max(propheight, minheight);

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

		// further clamp to the maximum width/height in the window
		if (m_win_config.width != 0)
			maxwidth = std::min(maxwidth, m_win_config.width + extrawidth);
		if (m_win_config.height != 0)
			maxheight = std::min(maxheight, m_win_config.height + extraheight);
	}

	// clamp to the maximum
	propwidth = std::min(propwidth, maxwidth);
	propheight = std::min(propheight, maxheight);

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

osd_dim uwp_window_info::get_min_bounds(int constrain)
{
	int32_t minwidth, minheight;

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

osd_dim uwp_window_info::get_max_bounds(int constrain)
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

void uwp_window_info::update_minmax_state()
{
	assert(GetCurrentThreadId() == window_threadid);

	if (!fullscreen())
	{
		RECT bounds;

		// compare the maximum bounds versus the current bounds
		const bool keep_aspect = keepaspect();
		osd_dim minbounds = get_min_bounds(keep_aspect);
		osd_dim maxbounds = get_max_bounds(keep_aspect);
		//GetWindowRect(platform_window<HWND>(), &bounds);

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

void uwp_window_info::minimize_window()
{
	assert(GetCurrentThreadId() == window_threadid);

	osd_dim newsize = get_min_bounds(keepaspect());

	// get the window rect
	//RECT bounds;
	//GetWindowRect(platform_window<HWND>(), &bounds);

	//osd_rect newrect(bounds.left, bounds.top, newsize );


	//SetWindowPos(platform_window<HWND>(), nullptr, newrect.left(), newrect.top(), newrect.width(), newrect.height(), SWP_NOZORDER);
}

//============================================================
//  maximize_window
//  (window thread)
//============================================================

void uwp_window_info::maximize_window()
{
	assert(GetCurrentThreadId() == window_threadid);

	osd_dim newsize = get_max_bounds(keepaspect());

	// center within the work area
	osd_rect work = m_monitor->usuable_position_size();
	osd_rect newrect = osd_rect(work.left() + (work.width() - newsize.width()) / 2,
			work.top() + (work.height() - newsize.height()) / 2,
			newsize);

	//SetWindowPos(platform_window<HWND>(), nullptr, newrect.left(), newrect.top(), newrect.width(), newrect.height(), SWP_NOZORDER);
}



//============================================================
//  adjust_window_position_after_major_change
//  (window thread)
//============================================================

void uwp_window_info::adjust_window_position_after_major_change()
{
	RECT oldrect;

	assert(GetCurrentThreadId() == window_threadid);

	// get the current size
	//GetWindowRect(platform_window<HWND>(), &oldrect);
	osd_rect newrect = RECT_to_osd_rect(oldrect);

	// adjust the window size so the client area is what we want
	if (!fullscreen())
	{
		// constrain the existing size to the aspect ratio
		if (keepaspect())
			newrect = constrain_to_aspect_ratio(newrect, WMSZ_BOTTOMRIGHT);
	}

	// in full screen, make sure it covers the primary display
	else
	{
		std::shared_ptr<osd_monitor_info> monitor = monitor_from_rect(nullptr);
		newrect = monitor->position_size();
	}

	// adjust the position if different
	if (oldrect.left != newrect.left() || oldrect.top != newrect.top() ||
		oldrect.right != newrect.right() || oldrect.bottom != newrect.bottom())
		//SetWindowPos(platform_window<HWND>(), fullscreen() ? HWND_TOPMOST : HWND_TOP,
			//  newrect.left(), newrect.top(),
				//newrect.width(), newrect.height(), 0);

	// take note of physical window size (used for lightgun coordinate calculation)
	if (m_index == 0)
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

void uwp_window_info::set_fullscreen(int fullscreen)
{
	assert(GetCurrentThreadId() == window_threadid);

	// if we're in the right state, punt
	if (this->fullscreen() == fullscreen)
		return;
	m_fullscreen = fullscreen;

	// reset UI to main menu
	machine().ui().menu_reset();

	// kill off the drawers
	renderer_reset();

	// hide ourself
	//ShowWindow(platform_window<HWND>(), SW_HIDE);

	// configure the window if non-fullscreen
	if (!fullscreen)
	{
		// adjust the style
		//SetWindowLong(platform_window<HWND>(), GWL_STYLE, WINDOW_STYLE);
		//SetWindowLong(platform_window<HWND>(), GWL_EXSTYLE, WINDOW_STYLE_EX);
		//SetWindowPos(platform_window<HWND>(), nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

		// force to the bottom, then back on top
		//SetWindowPos(platform_window<HWND>(), HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		//SetWindowPos(platform_window<HWND>(), HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

		//// if we have previous non-fullscreen bounds, use those
		//if (m_non_fullscreen_bounds.right != m_non_fullscreen_bounds.left)
		//{
		//  SetWindowPos(platform_window<HWND>(), HWND_TOP, m_non_fullscreen_bounds.left, m_non_fullscreen_bounds.top,
		//              rect_width(&m_non_fullscreen_bounds), rect_height(&m_non_fullscreen_bounds),
		//              SWP_NOZORDER);
		//}
		//
		//// otherwise, set a small size and maximize from there
		//else
		//{
		//  SetWindowPos(platform_window<HWND>(), HWND_TOP, 0, 0, MIN_WINDOW_DIM, MIN_WINDOW_DIM, SWP_NOZORDER);
		//  maximize_window();
		//}
	}

	// configure the window if fullscreen
	else
	{
		// save the bounds
		//GetWindowRect(platform_window<HWND>(), &m_non_fullscreen_bounds);
		//
		//// adjust the style
		//SetWindowLong(platform_window<HWND>(), GWL_STYLE, FULLSCREEN_STYLE);
		//SetWindowLong(platform_window<HWND>(), GWL_EXSTYLE, FULLSCREEN_STYLE_EX);
		//SetWindowPos(platform_window<HWND>(), nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
		//
		//// set topmost
		//SetWindowPos(platform_window<HWND>(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	}

	// adjust the window to compensate for the change
	adjust_window_position_after_major_change();

	// show ourself
	if (!this->fullscreen() || m_fullscreen_safe)
	{
		//if (video_config.mode != VIDEO_MODE_NONE)
			//ShowWindow(platform_window<HWND>(), SW_SHOW);

		set_renderer(osd_renderer::make_for_type(video_config.mode, shared_from_this()));
		if (renderer().create())
			exit(1);
	}

	// ensure we're still adjusted correctly
	adjust_window_position_after_major_change();
}

