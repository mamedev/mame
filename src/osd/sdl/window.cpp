// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont
//============================================================
//
//  window.c - SDL window handling
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================

#ifdef SDLMAME_WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

// standard SDL headers
#include "sdlinc.h"

#include <SDL2/SDL_thread.h>

// standard C headers
#include <math.h>
#ifndef _MSC_VER
#include <unistd.h>
#endif

// MAME headers

#include "emu.h"
#include "emuopts.h"
#include "ui/ui.h"


// OSD headers

#include "window.h"
#include "osdsdl.h"
#include "modules/render/drawbgfx.h"
#include "modules/render/drawsdl.h"
#include "modules/render/draw13.h"
#if (USE_OPENGL)
#include "modules/render/drawogl.h"
#endif

//============================================================
//  PARAMETERS
//============================================================

// these are arbitrary values since AFAIK there's no way to make X/SDL tell you
#define WINDOW_DECORATION_WIDTH (8) // should be more than plenty
#define WINDOW_DECORATION_HEIGHT (48)   // title bar + bottom drag region

#ifdef MAME_DEBUG
//#define ASSERT_USE(x) do { printf("%x %x\n", (int) SDL_ThreadID(), x); assert_always(SDL_ThreadID() == x, "Wrong Thread"); } while (0)
#define ASSERT_USE(x)   do { SDL_threadID _thid = SDL_ThreadID(); assert_always( _thid == x, "Wrong Thread"); } while (0)
#else
#define ASSERT_USE(x)   do {} while (0)
//#define ASSERT_USE(x) assert(SDL_ThreadID() == x)
#endif

#define ASSERT_REDRAW_THREAD()  ASSERT_USE(window_threadid)
#define ASSERT_WINDOW_THREAD()  ASSERT_USE(window_threadid)
#define ASSERT_MAIN_THREAD()    ASSERT_USE(main_threadid)

// minimum window dimension
#define MIN_WINDOW_DIM                  200

#ifndef SDLMAME_WIN32
#define WMSZ_TOP            (0)
#define WMSZ_BOTTOM         (1)
#define WMSZ_BOTTOMLEFT     (2)
#define WMSZ_BOTTOMRIGHT    (3)
#define WMSZ_LEFT           (4)
#define WMSZ_TOPLEFT        (5)
#define WMSZ_TOPRIGHT       (6)
#define WMSZ_RIGHT          (7)
#endif

//============================================================
//  GLOBAL VARIABLES
//============================================================

sdl_window_info *sdl_window_list;


//============================================================
//  LOCAL VARIABLES
//============================================================

static sdl_window_info **last_window_ptr;

// event handling
static SDL_threadID main_threadid;
static SDL_threadID window_threadid;

// debugger
//static int in_background;

struct worker_param {
	worker_param()
	: m_window(nullptr), m_list(nullptr), m_resize_new_width(0), m_resize_new_height(0)
	{
	}
	worker_param(sdl_window_info *awindow, render_primitive_list &alist)
	: m_window(awindow), m_list(&alist), m_resize_new_width(0), m_resize_new_height(0)
	{
	}
	worker_param(sdl_window_info *awindow, int anew_width, int anew_height)
	: m_window(awindow), m_list(nullptr), m_resize_new_width(anew_width), m_resize_new_height(anew_height)
	{
	}
	worker_param(sdl_window_info *awindow)
	: m_window(awindow), m_list(nullptr), m_resize_new_width(0), m_resize_new_height(0)
	{
	}
	sdl_window_info *window() const { assert(m_window != nullptr); return m_window; }
	render_primitive_list *list() const { return m_list; }
	int new_width() const { return m_resize_new_width; }
	int new_height() const { return m_resize_new_height; }
	// FIXME: only needed for window set-up which returns an error.
	void set_window(sdl_window_info *window) { m_window = window; }
private:
	sdl_window_info *m_window;
	render_primitive_list *m_list;
	int m_resize_new_width;
	int m_resize_new_height;
};


//============================================================
//  PROTOTYPES
//============================================================

static void sdlwindow_sync(void);

//============================================================
//  execute_async
//============================================================


static inline void execute_async(osd_work_callback callback, const worker_param &wp)
{
	worker_param *wp_temp = (worker_param *) osd_malloc(sizeof(worker_param));
	*wp_temp = wp;
	callback((void *) wp_temp, 0);
}

static inline void execute_sync(osd_work_callback callback, const worker_param &wp)
{
	worker_param *wp_temp = (worker_param *) osd_malloc(sizeof(worker_param));
	*wp_temp = wp;

	callback((void *) wp_temp, 0);
}


//============================================================
//  execute_async_wait
//============================================================

static inline void execute_async_wait(osd_work_callback callback, const worker_param &wp)
{
	execute_async(callback, wp);
	sdlwindow_sync();
}


//============================================================
//  sdlwindow_thread_id
//  (window thread)
//============================================================

static OSDWORK_CALLBACK(sdlwindow_thread_id)
{
	window_threadid = SDL_ThreadID();

	if (SDLMAME_INIT_IN_WORKER_THREAD)
	{
		if (SDL_InitSubSystem(SDL_INIT_VIDEO))
		{
			osd_printf_error("Could not initialize SDL: %s.\n", SDL_GetError());
			exit(-1);
		}
	}
	return nullptr;
}


//============================================================
//  window_init
//  (main thread)
//============================================================

bool sdl_osd_interface::window_init()
{
	osd_printf_verbose("Enter sdlwindow_init\n");

	// get the main thread ID before anything else
	main_threadid = SDL_ThreadID();

	// otherwise, treat the window thread as the main thread
	//window_threadid = main_threadid;
	sdlwindow_thread_id(nullptr, 0);

	// initialize the drawers
	if (video_config.mode == VIDEO_MODE_BGFX)
	{
		if (renderer_bgfx::init(machine()))
		{
#if (USE_OPENGL)
			video_config.mode = VIDEO_MODE_OPENGL;
		}
	}
	if (video_config.mode == VIDEO_MODE_OPENGL)
	{
		if (renderer_ogl::init(machine()))
		{
			video_config.mode = VIDEO_MODE_SOFT;
#else
			video_config.mode = VIDEO_MODE_SOFT;
#endif
		}
	}
	if (video_config.mode == VIDEO_MODE_SDL2ACCEL)
	{
		if (renderer_sdl2::init(machine()))
		{
			video_config.mode = VIDEO_MODE_SOFT;
	}
	}
	if (video_config.mode == VIDEO_MODE_SOFT)
	{
		if (renderer_sdl1::init(machine()))
		{
			return false;
	}
	}

	/* We may want to set a number of the hints SDL2 provides.
	 * The code below will document which hints were set.
	 */
	const char * hints[] = { SDL_HINT_FRAMEBUFFER_ACCELERATION,
			SDL_HINT_RENDER_DRIVER, SDL_HINT_RENDER_OPENGL_SHADERS,
			SDL_HINT_RENDER_SCALE_QUALITY,
			SDL_HINT_RENDER_VSYNC,
			SDL_HINT_VIDEO_X11_XVIDMODE, SDL_HINT_VIDEO_X11_XINERAMA,
			SDL_HINT_VIDEO_X11_XRANDR, SDL_HINT_GRAB_KEYBOARD,
			SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, SDL_HINT_IDLE_TIMER_DISABLED,
			SDL_HINT_ORIENTATIONS,
			SDL_HINT_XINPUT_ENABLED, SDL_HINT_GAMECONTROLLERCONFIG,
			SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, SDL_HINT_ALLOW_TOPMOST,
			SDL_HINT_TIMER_RESOLUTION,
#if SDL_VERSION_ATLEAST(2, 0, 2)
			SDL_HINT_RENDER_DIRECT3D_THREADSAFE, SDL_HINT_VIDEO_ALLOW_SCREENSAVER,
			SDL_HINT_ACCELEROMETER_AS_JOYSTICK, SDL_HINT_MAC_CTRL_CLICK_EMULATE_RIGHT_CLICK,
			SDL_HINT_VIDEO_WIN_D3DCOMPILER, SDL_HINT_VIDEO_WINDOW_SHARE_PIXEL_FORMAT,
			SDL_HINT_VIDEO_MAC_FULLSCREEN_SPACES, SDL_HINT_MOUSE_RELATIVE_MODE_WARP,
#endif
#if SDL_VERSION_ATLEAST(2, 0, 3)
			SDL_HINT_RENDER_DIRECT3D11_DEBUG, SDL_HINT_VIDEO_HIGHDPI_DISABLED,
			SDL_HINT_WINRT_PRIVACY_POLICY_URL, SDL_HINT_WINRT_PRIVACY_POLICY_LABEL,
			SDL_HINT_WINRT_HANDLE_BACK_BUTTON,
#endif
			nullptr
	};

	osd_printf_verbose("\nHints:\n");
	for (int i = 0; hints[i] != nullptr; i++)
		osd_printf_verbose("\t%-40s %s\n", hints[i], SDL_GetHint(hints[i]));

	// set up the window list
	last_window_ptr = &sdl_window_list;
	osd_printf_verbose("Leave sdlwindow_init\n");
	return true;
}


//============================================================
//  sdlwindow_sync
//============================================================

static void sdlwindow_sync(void)
{
	// haha, do nothing, losers
}


void sdl_osd_interface::update_slider_list()
{
	for (sdl_window_info *window = sdl_window_list; window != nullptr; window = window->m_next)
	{
		// check if any window has dirty sliders
		if (&window->renderer() && window->renderer().sliders_dirty())
		{
			build_slider_list();
			return;
		}
	}
}

void sdl_osd_interface::build_slider_list()
{
	m_sliders = nullptr;
	slider_state* full_list = nullptr;
	slider_state* curr = nullptr;
	for (sdl_window_info *window = sdl_window_list; window != nullptr; window = window->m_next)
	{
		// take the sliders of the first window
		slider_state* window_sliders = window->renderer().get_slider_list();
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
//  sdlwindow_exit
//  (main thread)
//============================================================

static OSDWORK_CALLBACK( sdlwindow_exit_wt )
{
	if (SDLMAME_INIT_IN_WORKER_THREAD)
		SDL_Quit();

	if (param)
		osd_free(param);
	return nullptr;
}


void sdl_osd_interface::window_exit()
{
	worker_param wp_dummy;

	ASSERT_MAIN_THREAD();

	osd_printf_verbose("Enter sdlwindow_exit\n");

	// free all the windows
	while (sdl_window_list != nullptr)
	{
		sdl_window_info *temp = sdl_window_list;
		sdl_window_list = temp->m_next;
		temp->destroy();
		// free the window itself
		global_free(temp);
	}

	switch(video_config.mode)
	{
		case VIDEO_MODE_SDL2ACCEL:
			renderer_sdl2::exit();
			break;
		case VIDEO_MODE_SOFT:
			renderer_sdl1::exit();
			break;
		case VIDEO_MODE_BGFX:
			renderer_bgfx::exit();
			break;
#if (USE_OPENGL)
		case VIDEO_MODE_OPENGL:
			renderer_ogl::exit();
			break;
#endif
		default:
			break;
	}

	execute_async_wait(&sdlwindow_exit_wt, wp_dummy);

	osd_printf_verbose("Leave sdlwindow_exit\n");

}


//============================================================
//  sdlwindow_resize
//  (main thread)
//============================================================

OSDWORK_CALLBACK( sdl_window_info::sdlwindow_resize_wt )
{
	worker_param *      wp = (worker_param *) param;
	sdl_window_info *   window = wp->window();
	int width = wp->new_width();
	int height = wp->new_height();

	ASSERT_WINDOW_THREAD();

	SDL_SetWindowSize(window->sdl_window(), width, height);
	window->renderer().notify_changed();

	osd_free(wp);
	return nullptr;
}

void sdl_window_info::resize(INT32 width, INT32 height)
{
	ASSERT_MAIN_THREAD();

	osd_dim cd = get_size();

	if (width != cd.width() || height != cd.height())
		execute_async_wait(&sdlwindow_resize_wt, worker_param(this, width, height));
}


//============================================================
//  sdlwindow_clear_surface
//  (window thread)
//============================================================

OSDWORK_CALLBACK( sdl_window_info::notify_changed_wt )
{
	worker_param *wp = (worker_param *) param;
	sdl_window_info *window = wp->window();

	ASSERT_WINDOW_THREAD();

	window->renderer().notify_changed();
	osd_free(wp);
	return nullptr;
}

void sdl_window_info::notify_changed()
{
	worker_param wp;

	if (SDL_ThreadID() == main_threadid)
	{
		execute_async_wait(&notify_changed_wt, worker_param(this));
	}
	else
		execute_sync(&notify_changed_wt, worker_param(this));
}


//============================================================
//  sdlwindow_toggle_full_screen
//  (main thread)
//============================================================

OSDWORK_CALLBACK( sdl_window_info::sdlwindow_toggle_full_screen_wt )
{
	worker_param *wp = (worker_param *) param;
	sdl_window_info *window = wp->window();

	ASSERT_WINDOW_THREAD();

	// if we are in debug mode, never go full screen
	if (window->machine().debug_flags & DEBUG_FLAG_OSD_ENABLED)
		return nullptr;

	// If we are going fullscreen (leaving windowed) remember our windowed size
	if (!window->fullscreen())
	{
		window->m_windowed_dim = window->get_size();
	}

	if (window->m_renderer != nullptr)
	{
		delete window->m_renderer;
		window->m_renderer = nullptr;
	}

	bool is_osx = false;
#ifdef SDLMAME_MACOSX
	// FIXME: This is weird behaviour and certainly a bug in SDL
	is_osx = true;
#endif
	if (window->fullscreen() && (video_config.switchres || is_osx))
	{
		SDL_SetWindowFullscreen(window->sdl_window(), 0);    // Try to set mode
		SDL_SetWindowDisplayMode(window->sdl_window(), &window->m_original_mode);    // Try to set mode
		SDL_SetWindowFullscreen(window->sdl_window(), SDL_WINDOW_FULLSCREEN);    // Try to set mode
	}
	SDL_DestroyWindow(window->sdl_window());

	downcast<sdl_osd_interface &>(window->machine().osd()).release_keys();

	window->set_renderer(osd_renderer::make_for_type(video_config.mode, reinterpret_cast<osd_window *>(window)));

	// toggle the window mode
	window->set_fullscreen(!window->fullscreen());

	complete_create_wt(param, 0);

	return nullptr;
}

void sdl_window_info::toggle_full_screen()
{
	ASSERT_MAIN_THREAD();

	execute_async_wait(&sdlwindow_toggle_full_screen_wt, worker_param(this));
}

void sdl_window_info::modify_prescale(int dir)
{
	worker_param wp = worker_param(this);
	int new_prescale = prescale();

	if (dir > 0 && prescale() < 3)
		new_prescale = prescale() + 1;
	if (dir < 0 && prescale() > 1)
		new_prescale = prescale() - 1;

	if (new_prescale != prescale())
	{
		if (m_fullscreen && video_config.switchres)
		{
			execute_async_wait(&sdlwindow_video_window_destroy_wt, wp);

			m_prescale = new_prescale;

			execute_async_wait(&complete_create_wt, wp);

		}
		else
		{
			notify_changed();
			m_prescale = new_prescale;
		}
		machine().ui().popup_time(1, "Prescale %d", prescale());
	}
}

//============================================================
//  sdlwindow_update_cursor_state
//  (main or window thread)
//============================================================

void sdl_window_info::update_cursor_state()
{
#if (USE_XINPUT)
	// Hack for wii-lightguns:
	// they stop working with a grabbed mouse;
	// even a ShowCursor(SDL_DISABLE) already does this.
	// To make the cursor disappear, we'll just set an empty cursor image.
	unsigned char data[]={0,0,0,0,0,0,0,0};
	SDL_Cursor *c;
	c=SDL_CreateCursor(data, data, 8, 8, 0, 0);
	SDL_SetCursor(c);
#else
	// do not do mouse capture if the debugger's enabled to avoid
	// the possibility of losing control
	if (!(machine().debug_flags & DEBUG_FLAG_OSD_ENABLED))
	{
		bool should_hide_mouse = downcast<sdl_osd_interface&>(machine().osd()).should_hide_mouse();

		if (!fullscreen() && !should_hide_mouse)
		{
			SDL_ShowCursor(SDL_ENABLE);
			if (SDL_GetWindowGrab(sdl_window() ))
				SDL_SetWindowGrab(sdl_window(), SDL_FALSE);
			SDL_SetRelativeMouseMode(SDL_FALSE);
		}
		else
		{
			SDL_ShowCursor(SDL_DISABLE);
			if (!SDL_GetWindowGrab(sdl_window()))
				SDL_SetWindowGrab(sdl_window(), SDL_TRUE);
			SDL_SetRelativeMouseMode(SDL_TRUE);
		}
		SDL_SetCursor(nullptr); // Force an update in case the underlying driver has changed visibility
			}
#endif
}

OSDWORK_CALLBACK( sdl_window_info::update_cursor_state_wt )
{
	worker_param *      wp = (worker_param *) param;
	sdl_window_info *   window = wp->window();

	window->update_cursor_state();

	osd_free(wp);
	return nullptr;
}

int sdl_window_info::xy_to_render_target(int x, int y, int *xt, int *yt)
{
	return renderer().xy_to_render_target(x, y, xt, yt);
}

//============================================================
//  sdlwindow_video_window_create
//  (main thread)
//============================================================

int sdl_window_info::window_init()
{
	worker_param *wp = (worker_param *) osd_malloc(sizeof(worker_param));
	int result;

	ASSERT_MAIN_THREAD();

	// set the initial maximized state
	// FIXME: Does not belong here
	sdl_options &options = downcast<sdl_options &>(m_machine.options());
	m_startmaximized = options.maximize();

	// add us to the list
	*last_window_ptr = this;
	last_window_ptr = &this->m_next;

	set_renderer(osd_renderer::make_for_type(video_config.mode, reinterpret_cast<osd_window *>(this)));

	// create an event that we can use to skip blitting
	m_rendered_event = osd_event_alloc(FALSE, TRUE);

	// load the layout
	m_target = m_machine.render().target_alloc();

	// set the specific view
	set_starting_view(m_index, options.view(), options.view(m_index));

	// make the window title
	if (video_config.numscreens == 1)
		sprintf(m_title, "%s: %s [%s]", emulator_info::get_appname(), m_machine.system().description, m_machine.system().name);
	else
		sprintf(m_title, "%s: %s [%s] - Screen %d", emulator_info::get_appname(), m_machine.system().description, m_machine.system().name, m_index);

	wp->set_window(this);

	result = *((int *) sdl_window_info::complete_create_wt((void *) wp, 0));

	// handle error conditions
	if (result == 1)
		goto error;

	return 0;

error:
	destroy();
	return 1;
}


//============================================================
//  sdlwindow_video_window_destroy
//  (main thread)
//============================================================

OSDWORK_CALLBACK( sdl_window_info::sdlwindow_video_window_destroy_wt )
{
	worker_param *      wp = (worker_param *) param;
	sdl_window_info *   window = wp->window();

	ASSERT_WINDOW_THREAD();

	// free the textures etc
	global_free(window->m_renderer);
	window->m_renderer = nullptr;

	if (window->fullscreen() && video_config.switchres)
	{
		SDL_SetWindowFullscreen(window->sdl_window(), 0);    // Try to set mode
		SDL_SetWindowDisplayMode(window->sdl_window(), &window->m_original_mode);    // Try to set mode
		SDL_SetWindowFullscreen(window->sdl_window(), SDL_WINDOW_FULLSCREEN);    // Try to set mode
	}
	SDL_DestroyWindow(window->sdl_window());
	// release all keys ...
	downcast<sdl_osd_interface &>(window->machine().osd()).release_keys();

	osd_free(wp);
	return nullptr;
}

void sdl_window_info::destroy()
{
	sdl_window_info **prevptr;

	ASSERT_MAIN_THREAD();

	//osd_event_wait(window->rendered_event, osd_ticks_per_second()*10);

	// remove us from the list
	for (prevptr = &sdl_window_list; *prevptr != nullptr; prevptr = &(*prevptr)->m_next)
		if (*prevptr == this)
		{
			*prevptr = this->m_next;
			break;
		}

	// free the textures etc
	execute_async_wait(&sdlwindow_video_window_destroy_wt, worker_param(this));

	// free the render target, after the textures!
	this->machine().render().target_free(m_target);

	// free the event
	osd_event_free(m_rendered_event);

}


//============================================================
//  pick_best_mode
//============================================================

osd_dim sdl_window_info::pick_best_mode()
{
	int minimum_width, minimum_height, target_width, target_height;
	int i;
	int num;
	float size_score, best_score = 0.0f;
	osd_dim ret(0,0);

	// determine the minimum width/height for the selected target
	m_target->compute_minimum_size(minimum_width, minimum_height);

	// use those as the target for now
	target_width = minimum_width * MAX(1, prescale());
	target_height = minimum_height * MAX(1, prescale());

	// if we're not stretching, allow some slop on the minimum since we can handle it
	{
		minimum_width -= 4;
		minimum_height -= 4;
	}

	// FIXME: this should be provided by monitor !
	num = SDL_GetNumDisplayModes(*((UINT64 *)m_monitor->oshandle()));

	if (num == 0)
	{
		osd_printf_error("SDL: No modes available?!\n");
		exit(-1);
	}
	else
	{
		for (i = 0; i < num; ++i)
		{
			SDL_DisplayMode mode;
			SDL_GetDisplayMode(*((UINT64 *)m_monitor->oshandle()), i, &mode);

			// compute initial score based on difference between target and current
			size_score = 1.0f / (1.0f + abs((INT32)mode.w - target_width) + abs((INT32)mode.h - target_height));

			// if the mode is too small, give a big penalty
			if (mode.w < minimum_width || mode.h < minimum_height)
				size_score *= 0.01f;

			// if mode is smaller than we'd like, it only scores up to 0.1
			if (mode.w < target_width || mode.h < target_height)
				size_score *= 0.1f;

			// if we're looking for a particular mode, that's a winner
			if (mode.w == m_win_config.width && mode.h == m_win_config.height)
				size_score = 2.0f;

			// refresh adds some points
			if (m_win_config.refresh)
				size_score *= 1.0f / (1.0f + abs(m_win_config.refresh - mode.refresh_rate) / 10.0f);

			osd_printf_verbose("%4dx%4d@%2d -> %f\n", (int)mode.w, (int)mode.h, (int) mode.refresh_rate, (double) size_score);

			// best so far?
			if (size_score > best_score)
			{
				best_score = size_score;
				ret = osd_dim(mode.w, mode.h);
			}

		}
	}
	return ret;
}

//============================================================
//  sdlwindow_video_window_update
//  (main thread)
//============================================================

void sdl_window_info::update()
{
	osd_ticks_t     event_wait_ticks;
	ASSERT_MAIN_THREAD();

	// adjust the cursor state
	//sdlwindow_update_cursor_state(machine, window);

	execute_async(&update_cursor_state_wt, worker_param(this));

	// if we're visible and running and not in the middle of a resize, draw
	if (m_target != nullptr)
	{
		int tempwidth, tempheight;

		// see if the games video mode has changed
		m_target->compute_minimum_size(tempwidth, tempheight);
		if (osd_dim(tempwidth, tempheight) != m_minimum_dim)
		{
			m_minimum_dim = osd_dim(tempwidth, tempheight);

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
			event_wait_ticks = osd_ticks_per_second(); // block at most a second
		else
			event_wait_ticks = 0;

		if (osd_event_wait(m_rendered_event, event_wait_ticks))
		{
			// ensure the target bounds are up-to-date, and then get the primitives

			render_primitive_list &primlist = *m_renderer->get_primitives();

			// and redraw now

			execute_async(&draw_video_contents_wt, worker_param(this, primlist));
		}
	}
}


//============================================================
//  set_starting_view
//  (main thread)
//============================================================

void sdl_window_info::set_starting_view(int index, const char *defview, const char *view)
{
	int viewindex;

	ASSERT_MAIN_THREAD();

	// choose non-auto over auto
	if (strcmp(view, "auto") == 0 && strcmp(defview, "auto") != 0)
		view = defview;

	// query the video system to help us pick a view
	viewindex = target()->configured_view(view, index, video_config.numscreens);

	// set the view
	target()->set_view(viewindex);
}


//============================================================
//  complete_create
//  (window thread)
//============================================================

OSDWORK_CALLBACK( sdl_window_info::complete_create_wt )
{
	worker_param *      wp = (worker_param *) param;
	sdl_window_info *   window = wp->window();

	osd_dim temp(0,0);
	static int result[2] = {0,1};

	ASSERT_WINDOW_THREAD();
	osd_free(wp);

	// clear out original mode. Needed on OSX
	if (window->fullscreen())
	{
		// default to the current mode exactly
		temp = window->monitor()->position_size().dim();

		// if we're allowed to switch resolutions, override with something better
		if (video_config.switchres)
			temp = window->pick_best_mode();
	}
	else if (window->m_windowed_dim.width() > 0)
	{
		// if we have a remembered size force the new window size to it
		temp = window->m_windowed_dim;
	}
	else if (window->m_startmaximized)
		temp = window->get_max_bounds(video_config.keepaspect );
	else
		temp = window->get_min_bounds(video_config.keepaspect );

	// create the window .....

	/* FIXME: On Ubuntu and potentially other Linux OS you should use
	 * to disable panning. This has to be done before every invocation of mame.
	 *
	 * xrandr --output HDMI-0 --panning 0x0+0+0 --fb 0x0
	 *
	 */
	osd_printf_verbose("Enter sdl_info::create\n");

	if (window->renderer().has_flags(osd_renderer::FLAG_NEEDS_OPENGL))
	{
		SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

		/* FIXME: A reminder that gamma is wrong throughout MAME. Currently, SDL2.0 doesn't seem to
		    * support the following attribute although my hardware lists GL_ARB_framebuffer_sRGB as an extension.
		    *
		    * SDL_GL_SetAttribute( SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, 1 );
		    *
		    */
		window->m_extra_flags = SDL_WINDOW_OPENGL;
	}
	else
		window->m_extra_flags = 0;

#ifdef SDLMAME_MACOSX
	/* FIMXE: On OSX, SDL_WINDOW_FULLSCREEN_DESKTOP seems to be more reliable.
	 *        It however creates issues with white borders, i.e. the screen clear
	 *        does not work. This happens both with opengl and accel.
	 */
#endif

	// create the SDL window
	// soft driver also used | SDL_WINDOW_INPUT_GRABBED | SDL_WINDOW_MOUSE_FOCUS
	window->m_extra_flags |= (window->fullscreen() ?
			/*SDL_WINDOW_BORDERLESS |*/ SDL_WINDOW_INPUT_FOCUS | SDL_WINDOW_FULLSCREEN : SDL_WINDOW_RESIZABLE);

#if defined(SDLMAME_WIN32)
	SDL_SetHint(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "0");
#endif

	// get monitor work area for centering
	osd_rect work = window->monitor()->usuable_position_size();

	// create the SDL window
	window->m_sdl_window = SDL_CreateWindow(window->m_title,
			work.left() + (work.width() - temp.width()) / 2,
			work.top() + (work.height() - temp.height()) / 2,
			temp.width(), temp.height(), window->m_extra_flags);
	//window().sdl_window() = SDL_CreateWindow(window().m_title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
	//      width, height, m_extra_flags);

	if  ( window->m_sdl_window == nullptr )
	{
		if (window->renderer().has_flags(osd_renderer::FLAG_NEEDS_OPENGL))
			osd_printf_error("OpenGL not supported on this driver: %s\n", SDL_GetError());
		else
			osd_printf_error("Window creation failed: %s\n", SDL_GetError());
		return (void *) &result[1];
	}

	if (window->fullscreen() && video_config.switchres)
	{
		SDL_DisplayMode mode;
		//SDL_GetCurrentDisplayMode(window().monitor()->handle, &mode);
		SDL_GetWindowDisplayMode(window->sdl_window(), &mode);
		window->m_original_mode = mode;
		mode.w = temp.width();
		mode.h = temp.height();
		if (window->m_win_config.refresh)
			mode.refresh_rate = window->m_win_config.refresh;

		SDL_SetWindowDisplayMode(window->sdl_window(), &mode);    // Try to set mode
#ifndef SDLMAME_WIN32
		/* FIXME: Warp the mouse to 0,0 in case a virtual desktop resolution
		 * is in place after the mode switch - which will most likely be the case
		 * This is a hack to work around a deficiency in SDL2
		 */
		SDL_WarpMouseInWindow(window->sdl_window(), 1, 1);
#endif
	}
	else
	{
		//SDL_SetWindowDisplayMode(window().sdl_window(), nullptr); // Use desktop
	}

	// show window

	SDL_ShowWindow(window->sdl_window());
	//SDL_SetWindowFullscreen(window->sdl_window(), 0);
	//SDL_SetWindowFullscreen(window->sdl_window(), window->fullscreen());
	SDL_RaiseWindow(window->sdl_window());

#ifdef SDLMAME_WIN32
	if (window->fullscreen())
		SDL_SetWindowGrab(window->sdl_window(), SDL_TRUE);
#endif

	// set main window
	if (window->m_index > 0)
	{
		for (auto w = sdl_window_list; w != nullptr; w = w->m_next)
		{
			if (w->m_index == 0)
	{
				window->m_main = w;
				break;
	}
	}
	}

	// update monitor resolution after mode change to ensure proper pixel aspect
	window->monitor()->refresh();
	if (window->fullscreen() && video_config.switchres)
		window->monitor()->update_resolution(temp.width(), temp.height());

	// initialize the drawing backend
	if (window->renderer().create())
		return (void *) &result[1];

	// Make sure we have a consistent state
	SDL_ShowCursor(0);
	SDL_ShowCursor(1);

	return (void *) &result[0];
}


//============================================================
//  draw_video_contents
//  (window thread)
//============================================================

void sdl_window_info::measure_fps(int update)
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
		startTime=currentTime;
	if( frames>=frames_skip4fps )
		sumdt+=currentTime-t0;
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

OSDWORK_CALLBACK( sdl_window_info::draw_video_contents_wt )
{
	int     update =    1;
	worker_param *wp = (worker_param *) param;
	sdl_window_info *window = wp->window();

	ASSERT_REDRAW_THREAD();

	// Some configurations require events to be polled in the worker thread
	downcast< sdl_osd_interface& >(window->machine().osd()).process_events_buf();

	// Check whether window has vector screens

	{
#if 1
		int scrnum = 0;
		int is_vector = 0;
		screen_device_iterator iter(window->machine().root_device());
		for (const screen_device *screen = iter.first(); screen != nullptr; screen = iter.next())
		{
			if (scrnum == window->m_index)
			{
				is_vector = (screen->screen_type() == SCREEN_TYPE_VECTOR) ? 1 : 0;
				break;
			}
			else
			{
				scrnum++;
			}
		}
		if (is_vector)
			window->renderer().set_flags(osd_renderer::FLAG_HAS_VECTOR_SCREEN);
		else
			window->renderer().clear_flags(osd_renderer::FLAG_HAS_VECTOR_SCREEN);
#endif
	}


	window->m_primlist = wp->list();

	// if no bitmap, just fill
	if (window->m_primlist == nullptr)
	{
	}
	// otherwise, render with our drawing system
	else
	{
		if( video_config.perftest )
			window->measure_fps(update);
		else
			window->renderer().draw(update);
	}

	/* all done, ready for next */
	osd_event_set(window->m_rendered_event);
	osd_free(wp);

	return nullptr;
}

int sdl_window_info::wnd_extra_width()
{
	return m_fullscreen ? 0 : WINDOW_DECORATION_WIDTH;
}

int sdl_window_info::wnd_extra_height()
{
	return m_fullscreen ? 0 : WINDOW_DECORATION_HEIGHT;
}


//============================================================
//  constrain_to_aspect_ratio
//  (window thread)
//============================================================

osd_rect sdl_window_info::constrain_to_aspect_ratio(const osd_rect &rect, int adjustment)
{
	INT32 extrawidth = wnd_extra_width();
	INT32 extraheight = wnd_extra_height();
	INT32 propwidth, propheight;
	INT32 minwidth, minheight;
	INT32 maxwidth, maxheight;
	INT32 viswidth, visheight;
	INT32 adjwidth, adjheight;
	float pixel_aspect;
	osd_monitor_info *monitor = m_monitor;

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

osd_dim sdl_window_info::get_min_bounds(int constrain)
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

	// remove extra window stuff
	minwidth -= wnd_extra_width();
	minheight -= wnd_extra_height();

	return osd_dim(minwidth, minheight);
}



//============================================================
//  get_max_bounds
//  (window thread)
//============================================================

osd_dim sdl_window_info::get_max_bounds(int constrain)
{
	//assert(GetCurrentThreadId() == window_threadid);

	// compute the maximum client area
	// m_monitor->refresh();
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

	// remove extra window stuff
	maximum = maximum.resize(maximum.width() - wnd_extra_width(), maximum.height() - wnd_extra_height());

	return maximum.dim();
}
