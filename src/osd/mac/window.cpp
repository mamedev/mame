// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont
//============================================================
//
//  window.cpp - Mac window handling
//
//  Mac OSD by R. Belmont
//
//============================================================

// standard C headers
#include <cmath>
#ifndef _MSC_VER
#include <unistd.h>
#endif
#include <list>
#include <memory>

// MAME headers
#include "emu.h"
#include "emuopts.h"
#include "render.h"
#include "screen.h"
#include "ui/uimain.h"

// OSD headers

#include "window.h"
#include "osdmac.h"
#include "modules/monitor/monitor_common.h"

//============================================================
//  PROTOTYPES (windowcontroller.mm)
//============================================================

extern void *CreateMAMEWindow(const char *title, int x, int y, int w, int h, bool isFullscreen, uint32_t displayID);
extern void *GetOSWindow(void *wincontroller);
extern void MacWindowGetSize(void *wincontroller, int *width, int *height);
extern void MacWindowGetSizePixels(void *wincontroller, int *width, int *height);
extern void MacSetFullscreen(void *wincontroller, bool fullscreen, uint32_t displayID);
extern uint32_t MacDisplayIDForWindow(void *wincontroller);
extern void MacDestroyWindow(void *wincontroller);
extern void MacSetWindowAspectRatio(void *wincontroller, int width, int height);
extern void MacSetWindowMinSize(void *wincontroller, int width, int height);
extern void MacSetWindowContentSize(void *wincontroller, int width, int height);
extern bool MacAppHasFocus();
extern int MacPointerInWindow();
extern void MacShowPointer(bool show);
extern void MacCapturePointer(void *wincontroller, bool capture);

//============================================================
//  PARAMETERS
//============================================================

// these are arbitrary values since AFAIK there's no way to make X/SDL tell you
#define WINDOW_DECORATION_WIDTH (8) // should be more than plenty
#define WINDOW_DECORATION_HEIGHT (48)   // title bar + bottom drag region

// minimum window dimension
#define MIN_WINDOW_DIM                  200

#define WMSZ_TOP            (0)
#define WMSZ_BOTTOM         (1)
#define WMSZ_BOTTOMLEFT     (2)
#define WMSZ_BOTTOMRIGHT    (3)
#define WMSZ_LEFT           (4)
#define WMSZ_TOPLEFT        (5)
#define WMSZ_TOPRIGHT       (6)
#define WMSZ_RIGHT          (7)

// debugger
//static int in_background;


//============================================================
//  PROTOTYPES
//============================================================


//============================================================
//  window_init
//  (main thread)
//============================================================

bool mac_osd_interface::window_init()
{
	osd_printf_verbose("Enter macwindow_init\n");

	// set up the window list
	osd_printf_verbose("Leave macwindow_init\n");
	return true;
}


extern void MacPollInputs(); // in windowcontroller.mm

void mac_osd_interface::process_events()
{
	MacPollInputs();
}

//============================================================
//  MacRequestMachineExit
//  called from the Cocoa side when the user clicks the close
//  box or quits from the menu bar; returns 1 if a running
//  machine accepted the request
//============================================================

extern "C" int MacRequestMachineExit()
{
	if (osd_common_t::window_list().empty())
	{
		return 0;
	}

	osd_common_t::window_list().front()->machine().schedule_exit();
	return 1;
}

//============================================================
//  MacOrderWindowsFront
//  called when the application becomes active; windows shown
//  before activation completed (fullscreen startup) may not
//  have been ordered in, so re-assert all of them
//============================================================

extern void MacOrderWindowFront(void *wincontroller); // in windowcontroller.mm

extern "C" void MacOrderWindowsFront()
{
	for (const auto &w : osd_common_t::window_list())
	{
		auto &win = static_cast<mac_window_info &>(*w);
		if (win.platform_window() != nullptr)
		{
			MacOrderWindowFront(win.platform_window());
		}
	}
}

bool mac_osd_interface::has_focus() const
{
	return MacAppHasFocus();
}


//============================================================
//  macwindow_exit
//  (main thread)
//============================================================

void mac_osd_interface::window_exit()
{
	osd_printf_verbose("Enter macwindow_exit\n");

	// free all the windows
	while (!osd_common_t::s_window_list.empty())
	{
		auto window = std::move(osd_common_t::s_window_list.back());
		s_window_list.pop_back();
		window->destroy();
	}

	osd_printf_verbose("Leave macwindow_exit\n");
}

void mac_window_info::capture_pointer()
{
	if (!m_mouse_captured)
	{
		MacCapturePointer(platform_window(), true);
		m_mouse_captured = true;
	}
}

void mac_window_info::release_pointer()
{
	if (m_mouse_captured)
	{
		MacCapturePointer(platform_window(), false);
		m_mouse_captured = false;
	}
}

void mac_window_info::hide_pointer()
{
	if (!m_mouse_hidden)
	{
		MacShowPointer(false);
		m_mouse_hidden = true;
	}
}

void mac_window_info::show_pointer()
{
	if (m_mouse_hidden)
	{
		MacShowPointer(true);
		m_mouse_hidden = false;
	}
}


//============================================================
//  macwindow_resize
//============================================================

void mac_window_info::resize(int32_t width, int32_t height)
{
	osd_dim cd = get_size();

	if (width != cd.width() || height != cd.height())
	{
		// TODO: change window size here
		renderer().notify_changed();
	}
}


//============================================================
//  notify_changed
//============================================================

void mac_window_info::notify_changed()
{
	renderer().notify_changed();
}


//============================================================
//  update_aspect_ratio
//============================================================

void mac_window_info::update_aspect_ratio()
{
	if (platform_window() == nullptr || target() == nullptr)
	{
		return;
	}

	if (keepaspect() && (target()->scale_mode() == SCALE_FRACTIONAL))
	{
		// derive the ideal shape from the render target; only the ratio matters
		int32_t width, height;
		target()->compute_visible_area(10000, 10000, monitor()->pixel_aspect(), target()->orientation(), width, height);
		MacSetWindowAspectRatio(platform_window(), width, height);

		if (!fullscreen())
		{
			const osd_dim current = get_size();
			if ((current.width() > 0) && (current.height() > 0))
			{
				target()->compute_visible_area(current.width(), current.height(), monitor()->pixel_aspect(), target()->orientation(), width, height);
				if (osd_dim(width, height) != current)
				{
					MacSetWindowContentSize(platform_window(), width, height);
				}
			}
		}
	}
	else
	{
		// integer scaling modes and -nokeepaspect resize freely
		MacSetWindowAspectRatio(platform_window(), 0, 0);
	}

	// keep the window from being shrunk to nothing
	osd_dim minimum = get_min_bounds(keepaspect());
	MacSetWindowMinSize(platform_window(), minimum.width(), minimum.height());
}


//============================================================
//  toggle_full_screen
//============================================================

void mac_window_info::toggle_full_screen()
{
	// if we are in debug mode, never go full screen
	if (machine().debug_flags & DEBUG_FLAG_OSD_ENABLED)
	{
		return;
	}

	// If we are going fullscreen (leaving windowed) remember our windowed size
	if (!fullscreen())
	{
		m_windowed_dim = get_size();
	}

	// kill off the drawers
	renderer_reset();

	downcast<mac_osd_interface &>(machine().osd()).release_keys();

	// toggle the window mode; complete_create switches the existing window
	// pair and recreates the renderer on the new drawable
	set_fullscreen(!fullscreen());

	complete_create();
}

void mac_window_info::modify_prescale(int dir)
{
	int new_prescale = prescale();

	if (dir > 0 && prescale() < 8)
	{
		new_prescale = prescale() + 1;
	}
	if (dir < 0 && prescale() > 1)
	{
		new_prescale = prescale() - 1;
	}

	if (new_prescale != prescale())
	{
		if (m_fullscreen && video_config.switchres)
		{
			complete_destroy();

			m_prescale = new_prescale;

			complete_create();
		}
		else
		{
			notify_changed();
			m_prescale = new_prescale;
		}
	}
	machine().ui().popup_time(1, "Prescale %d", prescale());
}

//============================================================
//  update_cursor_state
//  (main or window thread)
//============================================================

void mac_window_info::update_cursor_state()
{
	// do not do mouse capture if the debugger's enabled to avoid
	// the possibility of losing control
	if (!(machine().debug_flags & DEBUG_FLAG_OSD_ENABLED))
	{
		auto &osd = downcast<mac_osd_interface &>(machine().osd());

		if (!osd.has_focus() || machine().ui().is_menu_active())
		{
			// the app is inactive or a menu wants the pointer
			show_pointer();
			release_pointer();
		}
		else if (osd.should_hide_mouse())
		{
			// a mouse-enabled machine is being pointed at - capture for
			// relative motion (this freezes the system cursor, so it's
			// strictly opt-in via -mouse/-lightgun)
			hide_pointer();
			capture_pointer();
		}
		else if (fullscreen() && MacPointerInWindow())
		{
			// plain fullscreen: hide the cursor over the game, but leave it
			// free so other displays stay usable
			hide_pointer();
			release_pointer();
		}
		else
		{
			show_pointer();
			release_pointer();
		}
	}
}

int mac_window_info::xy_to_render_target(int x, int y, int *xt, int *yt)
{
	return renderer().xy_to_render_target(x, y, xt, yt);
}

//============================================================
//  mac_window_info::window_init
//  (main thread)
//============================================================

int mac_window_info::window_init()
{
	int result;

	// set the initial maximized state
	// FIXME: Does not belong here
	m_startmaximized = downcast<mac_options &>(machine().options()).maximize();

	create_target();

	result = complete_create();

	// handle error conditions
	if (result == 1)
	{
		goto error;
	}

	return 0;

error:
	destroy();
	return 1;
}


//============================================================
//  mac_window_info::complete_destroy
//============================================================

void mac_window_info::complete_destroy()
{
	// Release pointer grab and hide if needed
	show_pointer();
	release_pointer();

	if (platform_window() != nullptr)
	{
		MacDestroyWindow(platform_window());
		set_platform_window(nullptr);
	}

	// release all keys ...
	downcast<mac_osd_interface &>(machine().osd()).release_keys();
}


//============================================================
//  pick_best_mode
//============================================================

osd_dim mac_window_info::pick_best_mode()
{
	// Modern macOS doesn't do exclusive-mode display switching, so the
	// desktop resolution is always the best (and only) choice
	return monitor()->position_size().dim();
}

//============================================================
//  sdlwindow_video_window_update
//  (main thread)
//============================================================

void mac_window_info::update()
{
	osd_ticks_t     event_wait_ticks;

	// adjust the cursor state
	//sdlwindow_update_cursor_state(machine, window);

	update_cursor_state();

	// if we're visible and running and not in the middle of a resize, draw
	if (target() != nullptr)
	{
		int tempwidth, tempheight;

		// see if the games video mode has changed
		// pick up runtime changes to the aspect settings from the UI
		const bool aspect = keepaspect();
		const int scale_mode = target()->scale_mode();
		if (aspect != m_last_keepaspect || scale_mode != m_last_scale_mode)
		{
			m_last_keepaspect = aspect;
			m_last_scale_mode = scale_mode;
			update_aspect_ratio();
		}

		target()->compute_minimum_size(tempwidth, tempheight);
		if (osd_dim(tempwidth, tempheight) != m_minimum_dim)
		{
			m_minimum_dim = osd_dim(tempwidth, tempheight);

			// the view or rotation changed - update the resize constraints
			update_aspect_ratio();

			if (!this->m_fullscreen)
			{
				//Don't resize window without user interaction;
				//window_resize(blitwidth, blitheight);
			}
			else if (video_config.switchres)
			{
				osd_dim tmp = this->pick_best_mode();
				resize(tmp.width(), tmp.height());
			}
		}

		if (video_config.waitvsync && video_config.syncrefresh)
		{
			event_wait_ticks = osd_ticks_per_second(); // block at most a second
		}
		else
		{
			event_wait_ticks = 0;
		}

		if (m_rendered_event.wait(event_wait_ticks))
		{
			const int update = 1;

			// ensure the target bounds are up-to-date, and then get the primitives

			render_primitive_list &primlist = *renderer().get_primitives();

			// and redraw now

			// Check whether window has vector screens

			{
				const screen_device *screen = screen_device_enumerator(machine().root_device()).byindex(index());
				if ((screen != nullptr) && (screen->screen_type() == SCREEN_TYPE_VECTOR))
				{
					renderer().set_flags(osd_renderer::FLAG_HAS_VECTOR_SCREEN);
				}
				else
				{
					renderer().clear_flags(osd_renderer::FLAG_HAS_VECTOR_SCREEN);
				}
			}

			m_primlist = &primlist;

			// if no bitmap, just fill
			if (m_primlist == nullptr)
			{
			}
			// otherwise, render with our drawing system
			else
			{
				if( video_config.perftest )
				{
					measure_fps(update);
				}
				else
				{
					renderer().draw(update);
				}
			}

			/* all done, ready for next */
			m_rendered_event.set();
		}
	}
}


//============================================================
//  complete_create
//============================================================

int mac_window_info::complete_create()
{
	osd_dim temp(0,0);

	// clear out original mode. Needed on OSX
	if (fullscreen())
	{
		// default to the current mode exactly
		temp = monitor()->position_size().dim();

		// if we're allowed to switch resolutions, override with something better
		if (video_config.switchres)
		{
			temp = pick_best_mode();
		}
	}
	else if (m_windowed_dim.width() > 0)
	{
		// if we have a remembered size force the new window size to it
		temp = m_windowed_dim;
	}
	else if (m_startmaximized)
	{
		temp = get_max_bounds(keepaspect());
	}
	else
	{
		temp = get_min_bounds(keepaspect());
	}

	// create the window .....
	osd_printf_verbose("Enter mac_window_info::complete_create\n");

	if (platform_window() == nullptr)
	{
		// get monitor work area for centering
		osd_rect work = monitor()->usuable_position_size();

		// starting in fullscreen mode, keep the hidden standard window small
		// enough to fit the display's work area so AppKit doesn't relocate it
		osd_dim window_dim = temp;
		if (fullscreen())
		{
			window_dim = get_min_bounds(keepaspect());
		}

		// Multiple windows on one monitor tile as a centered grid
		int slot = 0;
		for (const auto &other : osd_common_t::window_list())
		{
			auto &win = static_cast<mac_window_info &>(*other);
			if ((&win != this) && (win.monitor()->oshandle() == monitor()->oshandle()))
			{
				slot++;
			}
		}

		int expected = 0;
		auto &options = downcast<osd_options &>(machine().options());
		for (int i = 0; i < video_config.numscreens; i++)
		{
			const auto mon = monitor()->module().pick_monitor(options, i);
			if (mon && (mon->oshandle() == monitor()->oshandle()))
			{
				expected++;
			}
		}
		expected = std::max(expected, slot + 1);

		const int gap = 16;
		const int titlebar = 28;		// guesstimated value
		const int cols_fit = std::max(1, (work.width() + gap) / (window_dim.width() + gap));
		const int cols = std::min(expected, cols_fit);
		const int rows = (expected + cols - 1) / cols;
		const int col = slot % cols;
		const int row = slot / cols;

		// center the whole grid in the work area
		const int grid_width = (cols * window_dim.width()) + ((cols - 1) * gap);
		const int grid_height = (rows * window_dim.height()) + ((rows - 1) * (titlebar + gap));
		int x = work.left() + ((work.width() - grid_width) / 2) + (col * (window_dim.width() + gap));
		int y = work.top() + ((work.height() - grid_height) / 2) + (row * (window_dim.height() + titlebar + gap));
		x = std::max(x, work.left());
		y = std::max(y, work.top());

		auto window = CreateMAMEWindow(title().c_str(),
				x, y,
				window_dim.width(), window_dim.height(), fullscreen(),
				uint32_t(monitor()->oshandle()));

		if  (window == nullptr)
		{
			osd_printf_error("Window creation failed!\n");
			return 1;
		}

		set_platform_window(window);
	}
	else
	{
		uint32_t display = MacDisplayIDForWindow(platform_window());
		if (display == 0)
		{
			display = uint32_t(monitor()->oshandle());
		}
		MacSetFullscreen(platform_window(), fullscreen(), display);
	}

	// create the renderer now that the platform window exists; some render
	// modules (bgfx) need a valid native window handle at creation time
	if (!has_renderer())
	{
		renderer_create();
	}

	// update monitor resolution after mode change to ensure proper pixel aspect
	monitor()->refresh();
	if (fullscreen() && video_config.switchres)
	{
		monitor()->update_resolution(temp.width(), temp.height());
	}

	// initialize the drawing backend
	if (renderer().create())
	{
		osd_printf_verbose("Exiting mac_window_info::complete_create\n");
		return 1;
	}

	// apply the resize constraints to the (windowed) presentation
	update_aspect_ratio();

	return 0;
}


//============================================================
//  draw_video_contents
//  (window thread)
//============================================================

void mac_window_info::measure_fps(int update)
{
	const unsigned long frames_skip4fps = 100;
	static int64_t lastTime=0, sumdt=0, startTime=0;
	static unsigned long frames = 0;
	int64_t currentTime, t0;
	double dt;
	double tps;
	osd_ticks_t tps_t;

	tps_t = osd_ticks_per_second();
	tps = (double) tps_t;

	t0 = osd_ticks();

	renderer().draw(update);

	frames++;
	currentTime = osd_ticks();
	if(startTime==0||frames==frames_skip4fps)
	{
		startTime=currentTime;
	}
	if( frames>=frames_skip4fps )
	{
		sumdt+=currentTime-t0;
	}
	if( (currentTime-lastTime)>1L*osd_ticks_per_second() && frames>frames_skip4fps )
	{
		dt = (double) (currentTime-startTime) / tps; // in decimale sec.
		osd_printf_info("%6.2lfs, %4lu F, "
				"avrg game: %5.2lf FPS %.2lf ms/f, "
				"avrg video: %5.2lf FPS %.2lf ms/f, "
				"last video: %5.2lf FPS %.2lf ms/f\n",
			dt, frames-frames_skip4fps,
			(double)(frames-frames_skip4fps)/dt,                             // avrg game fps
			( (currentTime-startTime) / ((frames-frames_skip4fps)) ) * 1000.0 / osd_ticks_per_second(),
			(double)(frames-frames_skip4fps)/((double)(sumdt) / tps), // avrg vid fps
			( sumdt / ((frames-frames_skip4fps)) ) * 1000.0 / tps,
			1.0/((currentTime-t0) / osd_ticks_per_second()), // this vid fps
			(currentTime-t0) * 1000.0 / tps
		);
		lastTime = currentTime;
	}
}

int mac_window_info::wnd_extra_width()
{
	return m_fullscreen ? 0 : WINDOW_DECORATION_WIDTH;
}

int mac_window_info::wnd_extra_height()
{
	return m_fullscreen ? 0 : WINDOW_DECORATION_HEIGHT;
}


//============================================================
//  constrain_to_aspect_ratio
//  (window thread)
//============================================================

osd_rect mac_window_info::constrain_to_aspect_ratio(const osd_rect &rect, int adjustment)
{
	int32_t extrawidth = wnd_extra_width();
	int32_t extraheight = wnd_extra_height();
	int32_t propwidth, propheight;
	int32_t minwidth, minheight;
	int32_t maxwidth, maxheight;
	int32_t viswidth, visheight;
	int32_t adjwidth, adjheight;
	float pixel_aspect;

	// do not constrain aspect ratio for integer scaled views
	if (target()->scale_mode() != SCALE_FRACTIONAL)
	{
		return rect;
	}

	// get the pixel aspect ratio for the target monitor
	pixel_aspect = monitor()->pixel_aspect();

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

	// clamp against the absolute minimum
	propwidth = std::max(propwidth, MIN_WINDOW_DIM);
	propheight = std::max(propheight, MIN_WINDOW_DIM);

	// clamp against the minimum width and height
	propwidth = std::max(propwidth, minwidth);
	propheight = std::max(propheight, minheight);

	// clamp against the maximum (fit on one screen for full screen mode)
	if (m_fullscreen)
	{
		maxwidth = monitor()->position_size().width() - extrawidth;
		maxheight = monitor()->position_size().height() - extraheight;
	}
	else
	{
		maxwidth = monitor()->usuable_position_size().width() - extrawidth;
		maxheight = monitor()->usuable_position_size().height() - extraheight;

		// further clamp to the maximum width/height in the window
		if (m_win_config.width != 0)
		{
			maxwidth = std::min(maxwidth, m_win_config.width + extrawidth);
		}
		if (m_win_config.height != 0)
		{
			maxheight = std::min(maxheight, m_win_config.height + extraheight);
		}
	}

	// clamp to the maximum
	propwidth = std::min(propwidth, maxwidth);
	propheight = std::min(propheight, maxheight);

	// compute the visible area based on the proposed rectangle
	target()->compute_visible_area(propwidth, propheight, pixel_aspect, target()->orientation(), viswidth, visheight);

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

osd_dim mac_window_info::get_min_bounds(int constrain)
{
	int32_t minwidth, minheight;

	//assert(GetCurrentThreadId() == window_threadid);

	// get the minimum target size
	target()->compute_minimum_size(minwidth, minheight);

	// expand to our minimum dimensions
	if (minwidth < MIN_WINDOW_DIM)
	{
		minwidth = MIN_WINDOW_DIM;
	}
	if (minheight < MIN_WINDOW_DIM)
	{
		minheight = MIN_WINDOW_DIM;
	}

	// account for extra window stuff
	minwidth += wnd_extra_width();
	minheight += wnd_extra_height();

	// if we want it constrained, figure out which one is larger
	if (constrain && target()->scale_mode() == SCALE_FRACTIONAL)
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

	// remove extra window stuff
	minwidth -= wnd_extra_width();
	minheight -= wnd_extra_height();

	return osd_dim(minwidth, minheight);
}

//============================================================
//  get_size
//============================================================

osd_dim mac_window_info::get_size()
{
	int w=0; int h=0;
	if (platform_window() != nullptr)
	{
		MacWindowGetSize(platform_window(), &w, &h);
	}
	return osd_dim(w,h);
}

osd_dim mac_window_info::get_size_pixels()
{
	int w=0; int h=0;
	if (platform_window() != nullptr)
	{
		MacWindowGetSizePixels(platform_window(), &w, &h);
	}
	return osd_dim(w,h);
}


//============================================================
//  get_max_bounds
//  (window thread)
//============================================================

osd_dim mac_window_info::get_max_bounds(int constrain)
{
	//assert(GetCurrentThreadId() == window_threadid);

	// compute the maximum client area
	// monitor()->refresh();
	osd_rect maximum = monitor()->usuable_position_size();

	// clamp to the window's max
	int tempw = maximum.width();
	int temph = maximum.height();
	if (m_win_config.width != 0)
	{
		int temp = m_win_config.width + wnd_extra_width();
		if (temp < maximum.width())
		{
			tempw = temp;
		}
	}
	if (m_win_config.height != 0)
	{
		int temp = m_win_config.height + wnd_extra_height();
		if (temp < maximum.height())
		{
			temph = temp;
		}
	}

	maximum = maximum.resize(tempw, temph);

	// constrain to fit
	if (constrain && target()->scale_mode() == SCALE_FRACTIONAL)
	{
		maximum = constrain_to_aspect_ratio(maximum, WMSZ_BOTTOMRIGHT);
	}

	// remove extra window stuff
	maximum = maximum.resize(maximum.width() - wnd_extra_width(), maximum.height() - wnd_extra_height());

	return maximum.dim();
}

//============================================================
//  construction and destruction
//============================================================

mac_window_info::mac_window_info(
		running_machine &a_machine,
		render_module &renderprovider,
		int index,
		std::shared_ptr<osd_monitor_info> a_monitor,
		const osd_window_config *config)
	: osd_window_t(a_machine, renderprovider, index, a_monitor, *config)
	, m_startmaximized(0)
	// Following three are used by input code to defer resizes
	, m_minimum_dim(0, 0)
	, m_windowed_dim(0, 0)
	, m_last_keepaspect(false)
	, m_last_scale_mode(-1)
	, m_rendered_event(0, 1)
	, m_mouse_captured(false)
	, m_mouse_hidden(false)
{
	//FIXME: these should be per_window in config-> or even better a bit set
	m_fullscreen = !video_config.windowed;
	m_prescale = video_config.prescale;

	m_windowed_dim = osd_dim(config->width, config->height);
}

mac_window_info::~mac_window_info()
{
}


//============================================================
//  osd_set_aggressive_input_focus
//============================================================

void osd_set_aggressive_input_focus(bool aggressive_focus)
{
	// dummy implementation for now
}
