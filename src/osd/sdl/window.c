//============================================================
//
//  window.c - SDL window handling
//
//  Copyright (c) 1996-2014, Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================

// standard SDL headers
#include "sdlinc.h"

#if (SDLMAME_SDL2)
#include <SDL2/SDL_thread.h>
#else
#include <SDL/SDL_thread.h>
#endif

// standard C headers
#include <math.h>
#include <unistd.h>

// MAME headers

#include "emu.h"
#include "emuopts.h"
#include "ui/ui.h"


// OSD headers

#include "window.h"
#include "input.h"
#include "osdsdl.h"

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

#define OSDWORK_CALLBACK(name)  void *name(void *param, ATTR_UNUSED int threadid)

// minimum window dimension
#define MIN_WINDOW_DIM                  200

#ifndef SDLMAME_WIN32
#define WMSZ_TOP        (0)
#define WMSZ_BOTTOM     (1)
#define WMSZ_BOTTOMLEFT     (2)
#define WMSZ_BOTTOMRIGHT    (3)
#define WMSZ_LEFT       (4)
#define WMSZ_TOPLEFT        (5)
#define WMSZ_TOPRIGHT       (6)
#define WMSZ_RIGHT      (7)
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
static int multithreading_enabled;
static osd_work_queue *work_queue;

#if !(SDLMAME_SDL2) && (!defined(SDLMAME_EMSCRIPTEN))
typedef int SDL_threadID;
#endif

static SDL_threadID main_threadid;
static SDL_threadID window_threadid;

// debugger
//static int in_background;

static sdl_draw_info draw;

struct worker_param {
	worker_param()
	: m_window(NULL), m_list(NULL), m_machine(NULL), m_resize_new_width(0), m_resize_new_height(0)
	{
	}
	worker_param(running_machine &amachine, sdl_window_info *awindow)
	: m_window(awindow), m_list(NULL), m_machine(&amachine), m_resize_new_width(0), m_resize_new_height(0)
	{
	}
	worker_param(running_machine &amachine, sdl_window_info *awindow, render_primitive_list &alist)
	: m_window(awindow), m_list(&alist), m_machine(&amachine), m_resize_new_width(0), m_resize_new_height(0)
	{
	}
	worker_param(sdl_window_info *awindow, int anew_width, int anew_height)
	: m_window(awindow), m_list(NULL), m_machine(NULL), m_resize_new_width(anew_width), m_resize_new_height(anew_height)
	{
	}
	worker_param(sdl_window_info *awindow)
	: m_window(awindow), m_list(NULL), m_machine(NULL), m_resize_new_width(0), m_resize_new_height(0)
	{
	}
	running_machine &machine() const { assert(m_machine != NULL); return *m_machine; }
	sdl_window_info *window() const { assert(m_window != NULL); return m_window; }
	render_primitive_list *list() const { return m_list; }
	int new_width() const { return m_resize_new_width; }
	int new_height() const { return m_resize_new_height; }
	// FIXME: only needed for window set-up which returns an error.
	void set_window(sdl_window_info *window) { m_window = window; }
private:
	sdl_window_info *m_window;
	render_primitive_list *m_list;
	running_machine *m_machine;
	int m_resize_new_width;
	int m_resize_new_height;
};


//============================================================
//  PROTOTYPES
//============================================================

static OSDWORK_CALLBACK( draw_video_contents_wt );
static OSDWORK_CALLBACK( sdlwindow_video_window_destroy_wt );
static OSDWORK_CALLBACK( sdlwindow_resize_wt );
static OSDWORK_CALLBACK( sdlwindow_toggle_full_screen_wt );
static void sdlwindow_update_cursor_state(running_machine &machine, sdl_window_info *window);
static void sdlwindow_sync(void);

static void *complete_create_wt(void *param, int threadid);
static void set_starting_view(running_machine &machine, int index, sdl_window_info *window, const char *defview, const char *view);

//============================================================
//  execute_async
//============================================================


INLINE void execute_async(osd_work_callback callback, const worker_param &wp)
{
	worker_param *wp_temp = (worker_param *) osd_malloc(sizeof(worker_param));
	*wp_temp = wp;

	if (multithreading_enabled)
	{
		osd_work_item_queue(work_queue, callback, (void *) wp_temp, WORK_ITEM_FLAG_AUTO_RELEASE);
	} else
		callback((void *) wp_temp, 0);
}

INLINE void execute_sync(osd_work_callback callback, const worker_param &wp)
{
	worker_param *wp_temp = (worker_param *) osd_malloc(sizeof(worker_param));
	*wp_temp = wp;

	callback((void *) wp_temp, 0);
}


//============================================================
//  execute_async_wait
//============================================================

INLINE void execute_async_wait(osd_work_callback callback, const worker_param &wp)
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
#if (SDLMAME_SDL2)
		if (SDL_InitSubSystem(SDL_INIT_TIMER|SDL_INIT_AUDIO| SDL_INIT_VIDEO| SDL_INIT_JOYSTICK|SDL_INIT_NOPARACHUTE))
#else
		if (SDL_Init(SDL_INIT_TIMER|SDL_INIT_AUDIO| SDL_INIT_VIDEO| SDL_INIT_JOYSTICK|SDL_INIT_NOPARACHUTE))
#endif
		{
			osd_printf_error("Could not initialize SDL: %s.\n", SDL_GetError());
			exit(-1);
		}
	}
	return NULL;
}


//============================================================
//  window_init
//  (main thread)
//============================================================

bool sdl_osd_interface::window_init()
{
	osd_printf_verbose("Enter sdlwindow_init\n");
	// determine if we are using multithreading or not
	multithreading_enabled = options().multithreading();

	// get the main thread ID before anything else
	main_threadid = SDL_ThreadID();

	// if multithreading, create a thread to run the windows
	if (multithreading_enabled)
	{
		// create a thread to run the windows from
		work_queue = osd_work_queue_alloc(WORK_QUEUE_FLAG_IO);
		if (work_queue == NULL)
			return false;
		osd_work_item_queue(work_queue, &sdlwindow_thread_id, NULL, WORK_ITEM_FLAG_AUTO_RELEASE);
		sdlwindow_sync();
	}
	else
	{
		// otherwise, treat the window thread as the main thread
		//window_threadid = main_threadid;
		sdlwindow_thread_id(NULL, 0);
	}

	// initialize the drawers
#if USE_OPENGL
	if (video_config.mode == VIDEO_MODE_OPENGL)
	{
		if (drawogl_init(machine(), &draw))
			video_config.mode = VIDEO_MODE_SOFT;
	}
#endif
#if SDLMAME_SDL2
	if (video_config.mode == VIDEO_MODE_SDL2ACCEL)
	{
		if (drawsdl2_init(machine(), &draw))
			video_config.mode = VIDEO_MODE_SOFT;
	}
#endif
#ifdef USE_BGFX
	if (video_config.mode == VIDEO_MODE_BGFX)
	{
		if (drawbgfx_init(machine(), &draw))
			video_config.mode = VIDEO_MODE_SOFT;
	}
#endif
	if (video_config.mode == VIDEO_MODE_SOFT)
	{
		if (drawsdl_init(&draw))
			return false;
	}

#if SDLMAME_SDL2
	/* We may want to set a number of the hints SDL2 provides.
	 * The code below will document which hints were set.
	 */
	const char * hints[] = { SDL_HINT_FRAMEBUFFER_ACCELERATION,
			SDL_HINT_RENDER_DRIVER, SDL_HINT_RENDER_OPENGL_SHADERS,
			SDL_HINT_RENDER_SCALE_QUALITY,
			SDL_HINT_RENDER_VSYNC,
			SDL_HINT_VIDEO_X11_XVIDMODE, SDL_HINT_VIDEO_X11_XINERAMA,
			SDL_HINT_VIDEO_X11_XRANDR, SDL_HINT_GRAB_KEYBOARD,
			SDL_HINT_MOUSE_RELATIVE_MODE_WARP,
			SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, SDL_HINT_IDLE_TIMER_DISABLED,
			SDL_HINT_ORIENTATIONS,
			SDL_HINT_XINPUT_ENABLED, SDL_HINT_GAMECONTROLLERCONFIG,
			SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, SDL_HINT_ALLOW_TOPMOST,
			SDL_HINT_TIMER_RESOLUTION,
#if SDL_VERSION_ATLEAST(2, 0, 2)
			SDL_HINT_RENDER_DIRECT3D_THREADSAFE, SDL_HINT_VIDEO_ALLOW_SCREENSAVER,
			SDL_HINT_ACCELEROMETER_AS_JOYSTICK, SDL_HINT_MAC_CTRL_CLICK_EMULATE_RIGHT_CLICK,
			SDL_HINT_VIDEO_WIN_D3DCOMPILER, SDL_HINT_VIDEO_WINDOW_SHARE_PIXEL_FORMAT,
			SDL_HINT_VIDEO_MAC_FULLSCREEN_SPACES,
#endif
#if SDL_VERSION_ATLEAST(2, 0, 3)
			SDL_HINT_RENDER_DIRECT3D11_DEBUG, SDL_HINT_VIDEO_HIGHDPI_DISABLED,
			SDL_HINT_WINRT_PRIVACY_POLICY_URL, SDL_HINT_WINRT_PRIVACY_POLICY_LABEL,
			SDL_HINT_WINRT_HANDLE_BACK_BUTTON,
#endif
			NULL
	};


	osd_printf_verbose("\nHints:\n");
	for (int i = 0; hints[i] != NULL; i++)
		osd_printf_verbose("\t%-40s %s\n", hints[i], SDL_GetHint(hints[i]));
#endif

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
	if (multithreading_enabled)
	{
		// Fallback
		while (!osd_work_queue_wait(work_queue, osd_ticks_per_second()*10))
		{
			osd_printf_warning("sdlwindow_sync: Sleeping...\n");
			osd_sleep(100000);
		}
	}
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
	return NULL;
}


void sdl_osd_interface::window_exit()
{
	worker_param wp_dummy;

	ASSERT_MAIN_THREAD();

	osd_printf_verbose("Enter sdlwindow_exit\n");

	// free all the windows
	while (sdl_window_list != NULL)
	{
		sdl_window_info *temp = sdl_window_list;
		sdl_window_list = temp->m_next;
		temp->video_window_destroy(machine());
		// free the window itself
		global_free(temp);
	}

	// if we're multithreaded, clean up the window thread
	if (multithreading_enabled)
	{
		sdlwindow_sync();
	}

	// kill the drawers
	(*draw.exit)();

	execute_async_wait(&sdlwindow_exit_wt, wp_dummy);

	if (multithreading_enabled)
	{
		osd_work_queue_wait(work_queue, 1000000);
		osd_work_queue_free(work_queue);
	}
	osd_printf_verbose("Leave sdlwindow_exit\n");

}


//============================================================
//  sdlwindow_blit_surface_size
//============================================================

INLINE int better_mode(int width0, int height0, int width1, int height1, float desired_aspect)
{
	float aspect0 = (float)width0 / (float)height0;
	float aspect1 = (float)width1 / (float)height1;
	return (fabs(desired_aspect - aspect0) < fabs(desired_aspect - aspect1)) ? 0 : 1;
}

void sdl_window_info::blit_surface_size(int window_width, int window_height)
{
	INT32 newwidth, newheight;
	int xscale = 1, yscale = 1;
	float desired_aspect = 1.0f;
	INT32 target_width = window_width;
	INT32 target_height = window_height;

	// start with the minimum size
	m_target->compute_minimum_size(newwidth, newheight);

	// compute the appropriate visible area if we're trying to keepaspect
	if (video_config.keepaspect)
	{
		// make sure the monitor is up-to-date
		m_target->compute_visible_area(target_width, target_height, m_monitor->aspect(), m_target->orientation(), target_width, target_height);
		desired_aspect = (float)target_width / (float)target_height;
	}

	// non-integer scaling - often gives more pleasing results in full screen
	if (!video_config.fullstretch)
	{
		// compute maximum integral scaling to fit the window
		xscale = (target_width + 2) / newwidth;
		yscale = (target_height + 2) / newheight;

		// try a little harder to keep the aspect ratio if desired
		if (video_config.keepaspect)
		{
			// if we could stretch more in the X direction, and that makes a better fit, bump the xscale
			while (newwidth * (xscale + 1) <= window_width &&
				better_mode(newwidth * xscale, newheight * yscale, newwidth * (xscale + 1), newheight * yscale, desired_aspect))
				xscale++;

			// if we could stretch more in the Y direction, and that makes a better fit, bump the yscale
			while (newheight * (yscale + 1) <= window_height &&
				better_mode(newwidth * xscale, newheight * yscale, newwidth * xscale, newheight * (yscale + 1), desired_aspect))
				yscale++;

			// now that we've maxed out, see if backing off the maximally stretched one makes a better fit
			if (window_width - newwidth * xscale < window_height - newheight * yscale)
			{
				while (better_mode(newwidth * xscale, newheight * yscale, newwidth * (xscale - 1), newheight * yscale, desired_aspect) && (xscale >= 0))
					xscale--;
			}
			else
			{
				while (better_mode(newwidth * xscale, newheight * yscale, newwidth * xscale, newheight * (yscale - 1), desired_aspect) && (yscale >= 0))
					yscale--;
			}
		}

		// ensure at least a scale factor of 1
		if (xscale <= 0) xscale = 1;
		if (yscale <= 0) yscale = 1;

		// apply the final scale
		newwidth *= xscale;
		newheight *= yscale;
	}
	else
	{
		newwidth = target_width;
		newheight = target_height;
	}

	//FIXME: really necessary to distinguish for yuv_modes ?
	if (m_target->zoom_to_screen()
		&& (video_config.scale_mode == VIDEO_SCALE_MODE_NONE ))
		newwidth = window_width;

	if ((m_blitwidth != newwidth) || (m_blitheight != newheight))
		window_clear();

	m_blitwidth = newwidth;
	m_blitheight = newheight;
}


//============================================================
//  sdlwindow_resize
//  (main thread)
//============================================================

static OSDWORK_CALLBACK( sdlwindow_resize_wt )
{
	worker_param *      wp = (worker_param *) param;
	sdl_window_info *   window = wp->window();

	ASSERT_WINDOW_THREAD();

	window->renderer().destroy_all_textures();
	window->renderer().resize(wp->new_width(), wp->new_height());

	window->blit_surface_size(wp->new_width(), wp->new_height());

	window->window_clear();

	osd_free(wp);
	return NULL;
}

void sdl_window_info::window_resize(INT32 width, INT32 height)
{
	ASSERT_MAIN_THREAD();

	if (width == this->m_width && height == this->m_height)
		return;

	execute_async_wait(&sdlwindow_resize_wt, worker_param(this, width, height));
}


//============================================================
//  sdlwindow_clear_surface
//  (window thread)
//============================================================

static OSDWORK_CALLBACK( sdlwindow_clear_surface_wt )
{
	worker_param *wp = (worker_param *) param;
	sdl_window_info *window = wp->window();

	ASSERT_WINDOW_THREAD();

	window->renderer().clear();
	osd_free(wp);
	return NULL;
}

void sdl_window_info::window_clear()
{
	worker_param wp;

	if (SDL_ThreadID() == main_threadid)
	{
		execute_async_wait(&sdlwindow_clear_surface_wt, worker_param(this));
	}
	else
		execute_sync(&sdlwindow_clear_surface_wt, worker_param(this));
}


//============================================================
//  sdlwindow_toggle_full_screen
//  (main thread)
//============================================================

static OSDWORK_CALLBACK( sdlwindow_toggle_full_screen_wt )
{
	worker_param *wp = (worker_param *) param;
	sdl_window_info *window = wp->window();

	ASSERT_WINDOW_THREAD();

	// if we are in debug mode, never go full screen
	if (window->machine().debug_flags & DEBUG_FLAG_OSD_ENABLED)
		return NULL;

	// If we are going fullscreen (leaving windowed) remember our windowed size
	if (!window->fullscreen())
	{
		window->m_windowed_width = window->m_width;
		window->m_windowed_height = window->m_height;
	}

	window->renderer().destroy();
	sdlinput_release_keys(wp->machine());

	// toggle the window mode
	window->set_fullscreen(!window->fullscreen());

	complete_create_wt(param, 0);

	return NULL;
}

void sdl_window_info::toggle_full_screen(running_machine &machine)
{
	ASSERT_MAIN_THREAD();

	execute_async_wait(&sdlwindow_toggle_full_screen_wt, worker_param(machine, this));
}

static OSDWORK_CALLBACK( destroy_all_textures_wt )
{
	worker_param *wp = (worker_param *) param;

	sdl_window_info *window = wp->window();

	window->renderer().destroy_all_textures();

	osd_free(wp);
	return NULL;
}

void sdl_window_info::modify_prescale(running_machine &machine, int dir)
{
	worker_param wp = worker_param(machine, this);
	int new_prescale = m_prescale;

	if (dir > 0 && m_prescale < 3)
		new_prescale = m_prescale + 1;
	if (dir < 0 && m_prescale > 1)
		new_prescale = m_prescale - 1;

	if (new_prescale != m_prescale)
	{
		if (m_fullscreen && video_config.switchres)
		{
			execute_async_wait(&sdlwindow_video_window_destroy_wt, wp);

			m_prescale = new_prescale;

			execute_async_wait(&complete_create_wt, wp);

		}
		else
		{
			execute_async_wait(destroy_all_textures_wt, wp);
			m_prescale = new_prescale;
		}
		machine.ui().popup_time(1, "Prescale %d", m_prescale);
	}
}

//============================================================
//  sdlwindow_update_cursor_state
//  (main or window thread)
//============================================================

static void sdlwindow_update_cursor_state(running_machine &machine, sdl_window_info *window)
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
#if (SDLMAME_SDL2)
	// do not do mouse capture if the debugger's enabled to avoid
	// the possibility of losing control
	if (!(machine.debug_flags & DEBUG_FLAG_OSD_ENABLED))
	{
		//FIXME: SDL1.3: really broken: the whole SDL code
		//       will only work correct with relative mouse movements ...
		if (!window->fullscreen() && !sdlinput_should_hide_mouse(machine))
		{
			SDL_ShowCursor(SDL_ENABLE);
			if (SDL_GetWindowGrab(window->m_sdl_window ))
				SDL_SetWindowGrab(window->m_sdl_window, SDL_FALSE);
			SDL_SetRelativeMouseMode(SDL_FALSE);
		}
		else
		{
			SDL_ShowCursor(SDL_DISABLE);
			if (!SDL_GetWindowGrab(window->m_sdl_window))
				SDL_SetWindowGrab(window->m_sdl_window, SDL_TRUE);
			SDL_SetRelativeMouseMode(SDL_TRUE);
		}
		SDL_SetCursor(NULL); // Force an update in case the underlying driver has changed visibility
	}

#else
	// do not do mouse capture if the debugger's enabled to avoid
	// the possibility of losing control
	if (!(machine.debug_flags & DEBUG_FLAG_OSD_ENABLED))
	{
		if ( window->fullscreen() || sdlinput_should_hide_mouse(machine) )
		{
			SDL_ShowCursor(SDL_DISABLE);
			if (!SDL_WM_GrabInput(SDL_GRAB_QUERY))
			{
				SDL_WM_GrabInput(SDL_GRAB_ON);
			}
		}
		else
		{
			SDL_ShowCursor(SDL_ENABLE);
			if (SDL_WM_GrabInput(SDL_GRAB_QUERY))
			{
				SDL_WM_GrabInput(SDL_GRAB_OFF);
			}
		}
	}
#endif
#endif
}

static OSDWORK_CALLBACK( sdlwindow_update_cursor_state_wt )
{
	worker_param *      wp = (worker_param *) param;
	//sdl_window_info *   window = wp->window;

	sdlwindow_update_cursor_state(wp->machine(), wp->window());

	osd_free(wp);
	return NULL;
}


//============================================================
//  sdlwindow_video_window_create
//  (main thread)
//============================================================

int sdlwindow_video_window_create(running_machine &machine, int index, sdl_monitor_info *monitor, const sdl_window_config *config)
{
	sdl_window_info *window;
	worker_param *wp = (worker_param *) osd_malloc(sizeof(worker_param));
	int result;

	ASSERT_MAIN_THREAD();

	// allocate a new window object
	window = global_alloc(sdl_window_info(&machine, monitor, index, config));

	// set the initial maximized state
	// FIXME: Does not belong here
	sdl_options &options = downcast<sdl_options &>(machine.options());
	window->m_startmaximized = options.maximize();

	// add us to the list
	*last_window_ptr = window;
	last_window_ptr = &window->m_next;

	window->set_renderer(draw.create(window));

	// create an event that we can use to skip blitting
	window->m_rendered_event = osd_event_alloc(FALSE, TRUE);

	// load the layout
	window->m_target = machine.render().target_alloc();

	// set the specific view
	set_starting_view(machine, index, window, options.view(), options.view(index));

	// make the window title
	if (video_config.numscreens == 1)
		sprintf(window->m_title, "%s: %s [%s]", emulator_info::get_appname(), machine.system().description, machine.system().name);
	else
		sprintf(window->m_title, "%s: %s [%s] - Screen %d", emulator_info::get_appname(), machine.system().description, machine.system().name, index);

	wp->set_window(window);

	// FIXME: pass error back in a different way
	if (multithreading_enabled)
	{
		osd_work_item *wi;

		wi = osd_work_item_queue(work_queue, &complete_create_wt, (void *) wp, 0);
		sdlwindow_sync();
		result = *((int *) (osd_work_item_result)(wi));
		osd_work_item_release(wi);
	}
	else
		result = *((int *) complete_create_wt((void *) wp, 0));

	// handle error conditions
	if (result == 1)
		goto error;

	return 0;

error:
	window->video_window_destroy(machine);
	// free the window itself
	global_free(window);
	return 1;
}


//============================================================
//  sdlwindow_video_window_destroy
//  (main thread)
//============================================================

static OSDWORK_CALLBACK( sdlwindow_video_window_destroy_wt )
{
	worker_param *      wp = (worker_param *) param;
	sdl_window_info *   window = wp->window();

	ASSERT_WINDOW_THREAD();

	// free the textures etc
	window->renderer().destroy();

	// release all keys ...
	sdlinput_release_keys(wp->machine());


	osd_free(wp);
	return NULL;
}

void sdl_window_info::video_window_destroy(running_machine &machine)
{
	sdl_window_info **prevptr;

	ASSERT_MAIN_THREAD();
	if (multithreading_enabled)
	{
		sdlwindow_sync();
	}

	//osd_event_wait(window->rendered_event, osd_ticks_per_second()*10);

	// remove us from the list
	for (prevptr = &sdl_window_list; *prevptr != NULL; prevptr = &(*prevptr)->m_next)
		if (*prevptr == this)
		{
			*prevptr = this->m_next;
			break;
		}

	// free the textures etc
	execute_async_wait(&sdlwindow_video_window_destroy_wt, worker_param(machine, this));

	// free the render target, after the textures!
	this->machine().render().target_free(m_target);

	// free the event
	osd_event_free(m_rendered_event);

}


//============================================================
//  pick_best_mode
//============================================================

#if SDLMAME_SDL2
void sdl_window_info::pick_best_mode(int *fswidth, int *fsheight)
{
	int minimum_width, minimum_height, target_width, target_height;
	int i;
	int num;
	float size_score, best_score = 0.0f;

	// determine the minimum width/height for the selected target
	m_target->compute_minimum_size(minimum_width, minimum_height);

	// use those as the target for now
	target_width = minimum_width * MAX(1, m_prescale);
	target_height = minimum_height * MAX(1, m_prescale);

	// if we're not stretching, allow some slop on the minimum since we can handle it
	{
		minimum_width -= 4;
		minimum_height -= 4;
	}

	num = SDL_GetNumDisplayModes(m_monitor->handle());

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
			SDL_GetDisplayMode(m_monitor->handle(), i, &mode);

			// compute initial score based on difference between target and current
			size_score = 1.0f / (1.0f + fabsf((INT32)mode.w - target_width) + fabsf((INT32)mode.h - target_height));

			// if the mode is too small, give a big penalty
			if (mode.w < minimum_width || mode.h < minimum_height)
				size_score *= 0.01f;

			// if mode is smaller than we'd like, it only scores up to 0.1
			if (mode.w < target_width || mode.h < target_height)
				size_score *= 0.1f;

			// if we're looking for a particular mode, that's a winner
			if (mode.w == m_maxwidth && mode.h == m_maxheight)
				size_score = 2.0f;

			// refresh adds some points
			if (m_refresh)
				size_score *= 1.0f / (1.0f + fabsf(m_refresh - mode.refresh_rate) / 10.0f);

			osd_printf_verbose("%4dx%4d@%2d -> %f\n", (int)mode.w, (int)mode.h, (int) mode.refresh_rate, size_score);

			// best so far?
			if (size_score > best_score)
			{
				best_score = size_score;
				*fswidth = mode.w;
				*fsheight = mode.h;
			}

		}
	}
}
#else
void sdl_window_info::pick_best_mode(int *fswidth, int *fsheight)
{
	int minimum_width, minimum_height, target_width, target_height;
	int i;
	float size_score, best_score = 0.0f;
	SDL_Rect **modes;

	// determine the minimum width/height for the selected target
	m_target->compute_minimum_size(minimum_width, minimum_height);

	// use those as the target for now
	target_width = minimum_width * MAX(1, m_prescale);
	target_height = minimum_height * MAX(1, m_prescale);

	// if we're not stretching, allow some slop on the minimum since we can handle it
	{
		minimum_width -= 4;
		minimum_height -= 4;
	}

#if 1 // defined(SDLMAME_WIN32)
	/*
	 *  We need to do this here. If SDL_ListModes is
	 * called in init_monitors, the call will crash
	 * on win32
	 */
	modes = SDL_ListModes(NULL, SDL_FULLSCREEN | SDL_DOUBLEBUF);
#else
	modes = window->m_monitor->modes;
#endif

	if (modes == (SDL_Rect **)0)
	{
		osd_printf_error("SDL: No modes available?!\n");
		exit(-1);
	}
	else if (modes == (SDL_Rect **)-1)  // all modes are possible
	{
		*fswidth = m_maxwidth;
		*fsheight = m_maxheight;
	}
	else
	{
		for (i = 0; modes[i]; ++i)
		{
			// compute initial score based on difference between target and current
			size_score = 1.0f / (1.0f + fabsf((INT32)modes[i]->w - target_width) + fabsf((INT32)modes[i]->h - target_height));

			// if the mode is too small, give a big penalty
			if (modes[i]->w < minimum_width || modes[i]->h < minimum_height)
				size_score *= 0.01f;

			// if mode is smaller than we'd like, it only scores up to 0.1
			if (modes[i]->w < target_width || modes[i]->h < target_height)
				size_score *= 0.1f;

			// if we're looking for a particular mode, that's a winner
			if (modes[i]->w == m_maxwidth && modes[i]->h == m_maxheight)
				size_score = 2.0f;

			osd_printf_verbose("%4dx%4d -> %f\n", (int)modes[i]->w, (int)modes[i]->h, size_score);

			// best so far?
			if (size_score > best_score)
			{
				best_score = size_score;
				*fswidth = modes[i]->w;
				*fsheight = modes[i]->h;
			}

		}
	}
}
#endif

//============================================================
//  sdlwindow_video_window_update
//  (main thread)
//============================================================

void sdl_window_info::video_window_update(running_machine &machine)
{
	osd_ticks_t     event_wait_ticks;
	ASSERT_MAIN_THREAD();

	// adjust the cursor state
	//sdlwindow_update_cursor_state(machine, window);

	execute_async(&sdlwindow_update_cursor_state_wt, worker_param(machine, this));

	// if we're visible and running and not in the middle of a resize, draw
	if (m_target != NULL)
	{
		int tempwidth, tempheight;

		// see if the games video mode has changed
		m_target->compute_minimum_size(tempwidth, tempheight);
		if (tempwidth != m_minwidth || tempheight != m_minheight)
		{
			m_minwidth = tempwidth;
			m_minheight = tempheight;

			if (!this->m_fullscreen)
			{
				blit_surface_size(m_width, m_height);
				//Don't resize window without user interaction;
				//window_resize(blitwidth, blitheight);
			}
			else if (video_config.switchres)
			{
				this->pick_best_mode(&tempwidth, &tempheight);
				window_resize(tempwidth, tempheight);
			}
		}

		if (video_config.waitvsync && video_config.syncrefresh)
			event_wait_ticks = osd_ticks_per_second(); // block at most a second
		else
			event_wait_ticks = 0;

		if (osd_event_wait(m_rendered_event, event_wait_ticks))
		{
			if ((!fullscreen()) || (video_config.switchres))
			{
				blit_surface_size(m_width, m_height);
			}
			else
			{
				blit_surface_size(monitor()->center_width(), monitor()->center_height());
			}

			// ensure the target bounds are up-to-date, and then get the primitives
			renderer().set_target_bounds();

			render_primitive_list &primlist = m_target->get_primitives();

			// and redraw now

			execute_async(&draw_video_contents_wt, worker_param(machine, this, primlist));
		}
	}
}


//============================================================
//  set_starting_view
//  (main thread)
//============================================================

static void set_starting_view(running_machine &machine, int index, sdl_window_info *window, const char *defview, const char *view)
{
	int viewindex;

	ASSERT_MAIN_THREAD();

	// choose non-auto over auto
	if (strcmp(view, "auto") == 0 && strcmp(defview, "auto") != 0)
		view = defview;

	// query the video system to help us pick a view
	viewindex = window->m_target->configured_view(view, index, video_config.numscreens);

	// set the view
	window->m_target->set_view(viewindex);
	window->m_start_viewscreen=viewindex;
}


//============================================================
//  complete_create
//  (window thread)
//============================================================

static OSDWORK_CALLBACK( complete_create_wt )
{
	worker_param *      wp = (worker_param *) param;
	sdl_window_info *   window = wp->window();

	int tempwidth, tempheight;
	static int result[2] = {0,1};

	ASSERT_WINDOW_THREAD();
	osd_free(wp);

	if (window->fullscreen())
	{
		// default to the current mode exactly
		tempwidth = window->monitor()->position_size().w;
		tempheight = window->monitor()->position_size().h;

		// if we're allowed to switch resolutions, override with something better
		if (video_config.switchres)
			window->pick_best_mode(&tempwidth, &tempheight);
	}
	else if (window->m_windowed_width)
	{
		// if we have a remembered size force the new window size to it
		tempwidth = window->m_windowed_width;
		tempheight = window->m_windowed_height;
	}
	else
	{
		if (window->m_startmaximized)
		{
			tempwidth = tempheight = 0;
			window->get_max_bounds(&tempwidth, &tempheight, video_config.keepaspect );
		}
		else
		{
			/* Create the window directly with the correct aspect
			   instead of letting sdlwindow_blit_surface_size() resize it
			   this stops the window from "flashing" from the wrong aspect
			   size to the right one at startup. */
			tempwidth = (window->m_maxwidth != 0) ? window->m_maxwidth : 640;
			tempheight = (window->m_maxheight != 0) ? window->m_maxheight : 480;

			window->get_min_bounds(&tempwidth, &tempheight, video_config.keepaspect );
		}
	}

	// initialize the drawing backend
	if (window->renderer().create(tempwidth, tempheight))
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

static void measure_fps(sdl_window_info *window, UINT32 dc, int update)
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

	window->renderer().draw(dc, update);

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

static OSDWORK_CALLBACK( draw_video_contents_wt )
{
	UINT32  dc =        0;
	int     update =    1;
	worker_param *wp = (worker_param *) param;
	sdl_window_info *window = wp->window();

	ASSERT_REDRAW_THREAD();

	// Some configurations require events to be polled in the worker thread
	sdlinput_process_events_buf(wp->machine());

	window->m_primlist = wp->list();

	// if no bitmap, just fill
	if (window->m_primlist == NULL)
	{
	}
	// otherwise, render with our drawing system
	else
	{
		if( video_config.perftest )
			measure_fps(window, dc, update);
		else
			window->renderer().draw(dc, update);
	}

	/* all done, ready for next */
	osd_event_set(window->m_rendered_event);
	osd_free(wp);

	return NULL;
}


//============================================================
//  constrain_to_aspect_ratio
//  (window thread)
//============================================================

void sdl_window_info::constrain_to_aspect_ratio(int *window_width, int *window_height, int adjustment)
{
	INT32 extrawidth = 0;
	INT32 extraheight = 0;
	INT32 propwidth, propheight;
	INT32 minwidth, minheight;
	INT32 maxwidth, maxheight;
	INT32 viswidth, visheight;
	float pixel_aspect;

	// get the pixel aspect ratio for the target monitor
	pixel_aspect = m_monitor->aspect();

	// determine the proposed width/height
	propwidth = *window_width - extrawidth;
	propheight = *window_height - extraheight;

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
	if (this->m_fullscreen)
	{
		maxwidth = m_monitor->center_width() - extrawidth;
		maxheight = m_monitor->center_height() - extraheight;
	}
	else
	{
		maxwidth = m_monitor->center_width() - extrawidth;
		maxheight = m_monitor->center_height() - extraheight;

		// further clamp to the maximum width/height in the window
		if (this->m_maxwidth != 0)
			maxwidth = MIN(maxwidth, this->m_maxwidth + extrawidth);
		if (this->m_maxheight != 0)
			maxheight = MIN(maxheight, this->m_maxheight + extraheight);
	}

	// clamp to the maximum
	propwidth = MIN(propwidth, maxwidth);
	propheight = MIN(propheight, maxheight);

	// compute the visible area based on the proposed rectangle
	m_target->compute_visible_area(propwidth, propheight, pixel_aspect, m_target->orientation(), viswidth, visheight);

	*window_width = viswidth;
	*window_height = visheight;
}


//============================================================
//  get_min_bounds
//  (window thread)
//============================================================

void sdl_window_info::get_min_bounds(int *window_width, int *window_height, int constrain)
{
	INT32 minwidth, minheight;

	// get the minimum target size
	this->m_target->compute_minimum_size(minwidth, minheight);

	// expand to our minimum dimensions
	if (minwidth < MIN_WINDOW_DIM)
		minwidth = MIN_WINDOW_DIM;
	if (minheight < MIN_WINDOW_DIM)
		minheight = MIN_WINDOW_DIM;

	// if we want it constrained, figure out which one is larger
	if (constrain)
	{
		int test1w, test1h;
		int test2w, test2h;

		// first constrain with no height limit
		test1w = minwidth; test1h = 10000;
		this->constrain_to_aspect_ratio(&test1w, &test1h, WMSZ_BOTTOMRIGHT);

		// then constrain with no width limit
		test2w = 10000; test2h = minheight;
		this->constrain_to_aspect_ratio(&test2w, &test2h, WMSZ_BOTTOMRIGHT);

		// pick the larger
		if ( test1w > test2w )
		{
			minwidth = test1w;
			minheight = test1h;
		}
		else
		{
			minwidth = test2w;
			minheight = test2h;
		}
	}

	*window_width = minwidth;
	*window_height = minheight;
}


//============================================================
//  get_max_bounds
//  (window thread)
//============================================================

void sdl_window_info::get_max_bounds(int *window_width, int *window_height, int constrain)
{
	INT32 maxwidth, maxheight;

	// compute the maximum client area
	maxwidth = m_monitor->center_width();
	maxheight = m_monitor->center_height();

	// clamp to the window's max
	if (this->m_maxwidth != 0)
	{
		int temp = this->m_maxwidth + WINDOW_DECORATION_WIDTH;
		if (temp < maxwidth)
			maxwidth = temp;
	}
	if (this->m_maxheight != 0)
	{
		int temp = this->m_maxheight + WINDOW_DECORATION_HEIGHT;
		if (temp < maxheight)
			maxheight = temp;
	}

	// constrain to fit
	if (constrain)
		this->constrain_to_aspect_ratio(&maxwidth, &maxheight, WMSZ_BOTTOMRIGHT);
	//else
	{
		maxwidth -= WINDOW_DECORATION_WIDTH;
		maxheight -= WINDOW_DECORATION_HEIGHT;
		*window_width = maxwidth;
		*window_height = maxheight;
	}
}
