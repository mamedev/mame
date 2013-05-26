//============================================================
//
//  window.cpp - Windows window management
//
//============================================================
//
//  Copyright Nicola Salmoria and the MAME Team.
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

#include "render/window.h"

namespace render::windows
{

window_system::window_system(running_machine &machine, video_system *video) :
	render::window_system(machine, video)
{
	// get the main thread ID before anything else
	m_main_threadid = GetCurrentThreadId();

	// set up window class and register it
	create_window_class();

	// create an event to signal UI pausing
	m_ui_pause_event = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (!m_ui_pause_event)
		fatalerror("Failed to create pause event\n");

	// if multithreading, create a thread to run the windows
	if (m_multithreading_enabled)
	{
		// create an event to signal when the window thread is ready
		m_window_thread_ready_event = CreateEvent(NULL, TRUE, FALSE, NULL);
		if (!m_window_thread_ready_event)
			fatalerror("Failed to create window thread ready event\n");

		// create a thread to run the windows from
		size_t temp = _beginthreadex(NULL, 0, thread_entry, NULL, 0, (unsigned *)&m_window_threadid);
		m_window_thread = (HANDLE)temp;
		if (m_window_thread == NULL)
			fatalerror("Failed to create window thread\n");

		// set the thread priority equal to the main MAME thread
		SetThreadPriority(m_window_thread, GetThreadPriority(GetCurrentThread()));
	}

	// otherwise, treat the window thread as the main thread
	else
	{
		m_window_thread = GetCurrentThread();
		m_window_threadid = m_main_threadid;
	}

	// initialize the drawers
	if (m_video_config.mode == VIDEO_MODE_D3D)
	{
		if (drawd3d_init(machine, &draw))
			m_video_config.mode = VIDEO_MODE_GDI;
	}
	if (m_video_config.mode == VIDEO_MODE_DDRAW)
	{
		if (drawdd_init(machine, &draw))
			m_video_config.mode = VIDEO_MODE_GDI;
	}
	if (m_video_config.mode == VIDEO_MODE_GDI)
		drawgdi_init(machine, &draw);
	if (m_video_config.mode == VIDEO_MODE_NONE)
		drawnone_init(machine, &draw);
}

window_info *window_system::window_alloc(monitor_info *monitor)
{
	return global_alloc_clear(window_info(m_machine, m_main_threadid, m_window_threadid, monitor));
}

void window_system::window_create(int index, monitor_info *monitor, const window_config *config)
{
	assert(GetCurrentThreadId() == m_main_threadid);

	// allocate a new window object

	window_info *window = window_alloc(machine, m_main_threadid, m_window_threadid, monitor));
	window->set_maxdims(config->width, config->height);
	window->set_refresh(config->refresh);
	window->set_fullscreen(!m_video_config.windowed);

	// see if we are safe for fullscreen
	window->set_fullscreen_safe(TRUE);
	for (window_info *win = m_window_list; win != NULL; win = win->next())
		if (win->monitor() == monitor)
			window->set_fullscreen_safe(FALSE);

	// add us to the list
	*m_last_window_ptr = window;
	m_last_window_ptr = &window->next();

	// make the window title
	if (m_video_config.numscreens == 1)
		sprintf(window->title(), "%s: %s [%s]", emulator_info::get_appname(), m_machine.system().description, m_machine.system().name);
	else
		sprintf(window->title(), "%s: %s [%s] - Screen %d", emulator_info::get_appname(), m_machine.system().description, m_machine.system().name, index);

	// set the initial maximized state
	window->startmaximized = options.maximize();

	window->wait_for_ready();
}

//============================================================
//  toggle_full_screen
//  (main thread)
//============================================================

void window_system::toggle_full_screen()
{
	assert(GetCurrentThreadId() == m_main_threadid);

	// if we are in debug mode, never go full screen
	for (window_info *window = m_window_list; window != NULL; window = window->next())
	{
		if (m_machine.debug_flags & DEBUG_FLAG_OSD_ENABLED)
		{
			return;
		}
	}

	// toggle the window mode
	video_system::video_config &video_config = m_video->video_config();
	video_config.windowed = !video_config.windowed;

	// iterate over windows and toggle their fullscreen state
	for (window_info *window = win_window_list; window != NULL; window = window->next())
	{
		SendMessage(window->hwnd(), WM_USER_SET_FULLSCREEN, !video_config.windowed, 0);
	}
	SetForegroundWindow(m_window_list->hwnd());
}

//============================================================
//  exit
//  (main thread)
//============================================================

void window_system::exit()
{
	assert(GetCurrentThreadId() == m_main_threadid);

	// possibly kill the debug window
	if (m_machine.debug_flags & DEBUG_FLAG_OSD_ENABLED)
		debugwin_destroy_windows();

	// free all the windows
	window_info *curr = m_window_list;
	while (curr != NULL)
	{
		window_info *temp = curr;
		curr = temp->next();
		global_free(temp);
	}

	// kill the drawers
	m_hal->exit();

	// if we're multithreaded, clean up the window thread
	if (m_multithreading_enabled)
	{
		PostThreadMessage(m_window_threadid, WM_USER_SELF_TERMINATE, 0, 0);
		WaitForSingleObject(m_window_thread, INFINITE);

		mtlog_dump();
	}

	// kill the UI pause event
	if (m_ui_pause_event)
		CloseHandle(m_ui_pause_event);

	// kill the window thread ready event
	if (m_window_thread_ready_event)
		CloseHandle(m_window_thread_ready_event);

	// if we hid the cursor during the emulation, show it
	while (ShowCursor(TRUE) < 0);
}

void window_info::remove_from_list()
{
	window_info *curr;

	// remove us from the list
	for (curr = m_window_list; curr != NULL; curr = curr->next())
	{
		if (curr->next() == this)
		{
			curr->set_next(m_next);
			break;
		}
	}

	if (m_window_list == this)
	{
		m_window_list = m_next;
	}
}

window_info::~window_info()
{
	assert(GetCurrentThreadId() == main_threadid);

	// destroy the window
	if (m_hwnd != NULL)
		SendMessage(m_hwnd, WM_USER_SELF_TERMINATE, 0, 0);
}


//============================================================
//  window_info::update_bounds
//  (main thread)
//============================================================

void window_info::update_bounds()
{
	return m_hal->update_bounds();
}


//============================================================
//  window_info::update
//  (main thread)
//============================================================

void window_info::update()
{
	assert(GetCurrentThreadId() == main_threadid);

	mtlog_add("winwindow_video_window_update: begin");

	// see if the target has changed significantly in window mode
	int targetview = m_target->view();
	int targetorient = m_target->orientation();
	render_layer_config targetlayerconfig = m_target->layer_config();
	if (targetview != m_targetview || targetorient != m_targetorient || targetlayerconfig != m_targetlayerconfig)
	{
		m_targetview = targetview;
		m_targetorient = targetorient;
		m_targetlayerconfig = targetlayerconfig;

		// in window mode, reminimize/maximize
		if (!m_fullscreen)
		{
			if (m_isminimized)
			{
				SendMessage(m_hwnd, WM_USER_SET_MINSIZE, 0, 0);
			}
			if (m_ismaximized)
			{
				SendMessage(m_hwnd, WM_USER_SET_MAXSIZE, 0, 0);
			}
		}
	}

	// if we're visible and running and not in the middle of a resize, draw
	if (m_hwnd != NULL && m_target != NULL && m_drawdata != NULL)
	{
		int got_lock = TRUE;

		mtlog_add("winwindow_video_window_update: try lock");

		// only block if we're throttled
		if (m_machine.video().throttled() || timeGetTime() - last_update_time > 250)
		{
			osd_lock_acquire(m_render_lock);
		}
		else
		{
			got_lock = osd_lock_try(m_render_lock);
		}

		// only render if we were able to get the lock
		if (got_lock)
		{
			render_primitive_list *primlist;

			mtlog_add("winwindow_video_window_update: got lock");

			// don't hold the lock; we just used it to see if rendering was still happening
			osd_lock_release(m_render_lock);

			// ensure the target bounds are up-to-date, and then get the primitives
			primlist = get_primitives();

			// post a redraw request with the primitive list as a parameter
			last_update_time = timeGetTime();
			mtlog_add("winwindow_video_window_update: PostMessage start");
			if (multithreading_enabled)
				PostMessage(m_hwnd, WM_USER_REDRAW, 0, (LPARAM)primlist);
			else
				SendMessage(m_hwnd, WM_USER_REDRAW, 0, (LPARAM)primlist);
			mtlog_add("winwindow_video_window_update: PostMessage end");
		}
	}

	mtlog_add("winwindow_video_window_update: end");
}


//============================================================
//  window_info::minimize_window
//  (window thread)
//============================================================

void window_info::minimize_window()
{
	assert(GetCurrentThreadId() == window_threadid);

	RECT newsize;
	get_min_bounds(window, &newsize, m_video_config.keepaspect);
	SetWindowPos(m_hwnd, NULL, newsize.left, newsize.top, rect_width(&newsize), rect_height(&newsize), SWP_NOZORDER);
}


//============================================================
//  window_info::maximize_window
//  (window thread)
//============================================================

void window_info::maximize_window()
{
	assert(GetCurrentThreadId() == window_threadid);

	RECT newsize;
	get_max_bounds(window, &newsize, m_video_config.keepaspect);
	SetWindowPos(m_hwnd, NULL, newsize.left, newsize.top, rect_width(&newsize), rect_height(&newsize), SWP_NOZORDER);
}


//============================================================
//  get_min_bounds
//  (window thread)
//============================================================

void window_info::get_min_bounds(RECT *bounds, bool constrain)
{
	assert(GetCurrentThreadId() == window_threadid);

	// get the minimum target size
	INT32 minwidth, minheight;
	m_target->compute_minimum_size(minwidth, minheight);

	// expand to our minimum dimensions
	if (minwidth < MIN_WINDOW_DIM)
		minwidth = MIN_WINDOW_DIM;
	if (minheight < MIN_WINDOW_DIM)
		minheight = MIN_WINDOW_DIM;

	// account for extra window stuff
	minwidth += extra_width();
	minheight += extra_height();

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
	GetWindowRect(m_hwnd, bounds);

	// now adjust
	bounds->right = bounds->left + minwidth;
	bounds->bottom = bounds->top + minheight;
}


//============================================================
//  get_max_bounds
//  (window thread)
//============================================================

void window_info::get_max_bounds(RECT *bounds, bool constrain)
{
	assert(GetCurrentThreadId() == window_threadid);

	// compute the maximum client area
	winvideo_monitor_refresh(m_monitor);
	RECT maximum = m_monitor->info.rcWork;

	// clamp to the window's max
	if (m_maxwidth != 0)
	{
		int temp = m_maxwidth + extra_width();
		if (temp < rect_width(&maximum))
			maximum.right = maximum.left + temp;
	}
	if (m_maxheight != 0)
	{
		int temp = m_maxheight + extra_height();
		if (temp < rect_height(&maximum))
			maximum.bottom = maximum.top + temp;
	}

	// constrain to fit
	if (constrain)
	{
		constrain_to_aspect_ratio(window, &maximum, WMSZ_BOTTOMRIGHT);
	}
	else
	{
		maximum.right -= extra_width();
		maximum.bottom -= extra_height();
	}

	// center within the work area
	bounds->left = m_monitor->info.rcWork.left + (rect_width(&m_monitor->info.rcWork) - rect_width(&maximum)) / 2;
	bounds->top = m_monitor->info.rcWork.top + (rect_height(&m_monitor->info.rcWork) - rect_height(&maximum)) / 2;
	bounds->right = bounds->left + rect_width(&maximum);
	bounds->bottom = bounds->top + rect_height(&maximum);
}


void window_info::toggle_fullscreen(bool fullscreen)
{
	assert(GetCurrentThreadId() == window_threadid);

	// if we're in the right state, punt
	if (m_fullscreen == fullscreen)
		return;
	m_fullscreen = fullscreen;

	// kill off the drawers
	m_hal->shutdown();
	global_free(m_hal);

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
		else // otherwise, set a small size and maximize from there
		{
			SetWindowPos(m_hwnd, HWND_TOP, 0, 0, MIN_WINDOW_DIM, MIN_WINDOW_DIM, SWP_NOZORDER);
			maximize_window(window);
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
		if (m_video_config.mode != VIDEO_MODE_NONE)
			ShowWindow(m_hwnd, SW_SHOW);
		// TODO: HAL type selection (GDI, DDraw, D3D, GL)
		m_hal = global_alloc_clear(draw_hal());
		if (!m_hal->initialize())
			exit(1);
	}

	// ensure we're still adjusted correctly
	adjust_window_position_after_major_change();
}

//============================================================
//  adjust_window_position_after_major_change
//  (window thread)
//============================================================

void window_info::adjust_window_position_after_major_change()
{
	assert(GetCurrentThreadId() == m_window_threadid);

	// get the current size
	RECT oldrect;
	GetWindowRect(m_hwnd, &oldrect);

	// adjust the window size so the client area is what we want
	RECT newrect;
	if (!m_fullscreen)
	{
		// constrain the existing size to the aspect ratio
		newrect = oldrect;
		if (m_video_config.keepaspect)
			constrain_to_aspect_ratio(&newrect, WMSZ_BOTTOMRIGHT);
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
		SetWindowPos(m_hwnd, m_fullscreen ? HWND_TOPMOST : HWND_TOP,
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
//  constrain_to_aspect_ratio
//  (window thread)
//============================================================

void window_info::constrain_to_aspect_ratio(RECT *rect, int adjustment)
{
	assert(GetCurrentThreadId() == window_threadid);

	win_monitor_info *monitor = winwindow_video_window_monitor(window, rect);
	INT32 extrawidth = extra_width();
	INT32 extraheight = extra_height();

	// get the pixel aspect ratio for the target monitor
	float pixel_aspect = m_monitor->get_aspect();

	// determine the proposed width/height
	propwidth = rect_width(rect) - extrawidth;
	propheight = rect_height(rect) - extraheight;

	// based on which edge we are adjusting, take either the width, height, or both as gospel
	// and scale to fit using that as our parameter
	INT32 propwidth, propheight;
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
	INT32 minwidth, minheight;
	m_target->compute_minimum_size(minwidth, minheight);

	// clamp against the absolute minimum
	propwidth = MAX(propwidth, MIN_WINDOW_DIM);
	propheight = MAX(propheight, MIN_WINDOW_DIM);

	// clamp against the minimum width and height
	propwidth = MAX(propwidth, minwidth);
	propheight = MAX(propheight, minheight);

	// clamp against the maximum (fit on one screen for full screen mode)
	INT32 maxwidth, maxheight;
	if (m_fullscreen)
	{
		maxwidth = rect_width(&m_info.rcMonitor) - extrawidth;
		maxheight = rect_height(&m_info.rcMonitor) - extraheight;
	}
	else
	{
		maxwidth = rect_width(&m_info.rcWork) - extrawidth;
		maxheight = rect_height(&m_info.rcWork) - extraheight;

		// further clamp to the maximum width/height in the window
		if (m_maxwidth != 0)
			maxwidth = MIN(maxwidth, m_maxwidth + extrawidth);
		if (m_maxheight != 0)
			maxheight = MIN(maxheight, m_maxheight + extraheight);
	}

	// clamp to the maximum
	propwidth = MIN(propwidth, maxwidth);
	propheight = MIN(propheight, maxheight);

	// compute the visible area based on the proposed rectangle
	INT32 viswidth, visheight;
	m_target->compute_visible_area(propwidth, propheight, pixel_aspect, m_target->orientation(), viswidth, visheight);

	// compute the adjustments we need to make
	INT32 adjwidth = (viswidth + extrawidth) - rect_width(rect);
	INT32 adjheight = (visheight + extraheight) - rect_height(rect);

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
//  winwindow_video_window_monitor
//  (window thread)
//============================================================

monitor_info *window_info::monitor(const RECT *proposed)
{
	monitor_info *monitor;

	// in window mode, find the nearest
	if (!m_fullscreen)
	{
		if (proposed != NULL)
		{
			monitor = winvideo_monitor_from_handle(MonitorFromRect(proposed, MONITOR_DEFAULTTONEAREST));
		}
		else
		{
			monitor = winvideo_monitor_from_handle(MonitorFromWindow(m_hwnd, MONITOR_DEFAULTTONEAREST));
		}
	}
	else // in full screen, just use the configured monitor
	{
		monitor = m_monitor;
	}

	// make sure we're up-to-date
	monitor->refresh();
	return monitor;
}

int window_info::extra_width()
{
	RECT temprect = { 100, 100, 200, 200 };
	if (m_fullscreen)
	{
		return 0;
	}
	AdjustWindowRectEx(&temprect, WINDOW_STYLE, has_menu(), WINDOW_STYLE_EX);
	return rect_width(&temprect) - 100;
}


int window_info::extra_height()
{
	RECT temprect = { 100, 100, 200, 200 };
	if (m_fullscreen)
	{
		return 0;
	}
	AdjustWindowRectEx(&temprect, WINDOW_STYLE, has_menu(), WINDOW_STYLE_EX);
	return rect_height(&temprect) - 100;
}

void window_info::complete_create()
{
	RECT client;
	HDC dc;

	assert(GetCurrentThreadId() == m_window_threadid);

	// get the monitor bounds
	RECT monitorbounds = m_monitor->info.rcMonitor;

	// create the window, but don't show it yet
	m_hwnd = win_create_window_ex_utf8(
						m_fullscreen ? FULLSCREEN_STYLE_EX : WINDOW_STYLE_EX,
						"MAME",
						m_title,
						m_fullscreen ? FULLSCREEN_STYLE : WINDOW_STYLE,
						monitorbounds.left + 20, monitorbounds.top + 20,
						monitorbounds.left + 100, monitorbounds.top + 100,
						NULL,//(win_window_list != NULL) ? win_window_list->hwnd : NULL,
						NULL,
						GetModuleHandle(NULL),
						NULL);
	if (m_hwnd == NULL)
	{
		m_init_state = 1;
		return;
	}

	// set a pointer back to us
	SetWindowLongPtr(m_hwnd, GWLP_USERDATA, (LONG_PTR)window);

	// skip the positioning stuff for -video none */
	if (m_video_config.mode == VIDEO_MODE_NONE)
	{
		m_init_state = 0;
		return;
	}

	// adjust the window position to the initial width/height
	int tempwidth = (m_maxwidth != 0) ? m_maxwidth : 640;
	int tempheight = (m_maxheight != 0) ? m_maxheight : 480;
	SetWindowPos(m_hwnd, NULL, monitorbounds.left + 20, monitorbounds.top + 20,
			monitorbounds.left + tempwidth + extra_width(window),
			monitorbounds.top + tempheight + extra_height(window),
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
		if(!m_hal->initialize())
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

void window_info::wait_for_ready()
{
	// finish the window creation on the window thread
	if (m_multithreading_enabled)
	{
		// wait until the window thread is ready to respond to events
		// TODO:
		WaitForSingleObject(window_thread_ready_event, INFINITE);

		PostThreadMessage(window_threadid, WM_USER_FINISH_CREATE_WINDOW, 0, (LPARAM)window);
		while (m_init_state == 0)
			Sleep(1);
	}
	else
	{
		m_init_state = complete_create() ? -1 : 1;
	}

	// handle error conditions
	if (m_init_state == -1)
	{
		fatalerror("Unable to complete window creation\n");
	}
}

}; // namespace render::windows