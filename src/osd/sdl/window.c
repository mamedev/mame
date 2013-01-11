//============================================================
//
//  window.c - SDL window handling
//
//  Copyright (c) 1996-2010, Nicola Salmoria and the MAME Team.
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
#include "ui.h"


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

#if !(SDLMAME_SDL2)
typedef int SDL_threadID;
#endif

static SDL_threadID main_threadid;
static SDL_threadID window_threadid;

// debugger
//static int in_background;

static sdl_draw_info draw;

struct worker_param {
	running_machine &machine() const { assert(m_machine != NULL); return *m_machine; }
	sdl_window_info *window;
	render_primitive_list *list;
	running_machine *m_machine;
	int resize_new_width;
	int resize_new_height;
};


//============================================================
//  PROTOTYPES
//============================================================

static void sdlwindow_exit(running_machine &machine);
static void sdlwindow_video_window_destroy(running_machine &machine, sdl_window_info *window);
static OSDWORK_CALLBACK( draw_video_contents_wt );
static OSDWORK_CALLBACK( sdlwindow_video_window_destroy_wt );
static OSDWORK_CALLBACK( sdlwindow_resize_wt );
static OSDWORK_CALLBACK( sdlwindow_toggle_full_screen_wt );
static void sdlwindow_update_cursor_state(running_machine &machine, sdl_window_info *window);
static void sdlwindow_sync(void);

static void get_min_bounds(sdl_window_info *window, int *window_width, int *window_height, int constrain);
static void get_max_bounds(sdl_window_info *window, int *window_width, int *window_height, int constrain);

static void *complete_create_wt(void *param, int threadid);
static void set_starting_view(running_machine &machine, int index, sdl_window_info *window, const char *defview, const char *view);

//============================================================
//  clear the worker_param structure, inline - faster than memset
//============================================================

INLINE void clear_worker_param(worker_param *wp)
{
	wp->window=NULL;
	wp->list=NULL;
	wp->m_machine=NULL;
	wp->resize_new_width=0;
	wp->resize_new_height=0;
}


//============================================================
//  execute_async
//============================================================


INLINE void execute_async(osd_work_callback callback, worker_param *wp)
{
	worker_param *wp_temp = NULL;

	if (wp)
	{
		wp_temp = (worker_param *) osd_malloc(sizeof(worker_param));
		*wp_temp = *wp;
	}
	if (multithreading_enabled)
	{
		osd_work_item_queue(work_queue, callback, (void *) wp_temp, WORK_ITEM_FLAG_AUTO_RELEASE);
	} else
		callback((void *) wp_temp, 0);
}


//============================================================
//  execute_async_wait
//============================================================

INLINE void execute_async_wait(osd_work_callback callback, worker_param *wp)
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
			mame_printf_error("Could not initialize SDL: %s.\n", SDL_GetError());
			exit(-1);
		}
	}
	return NULL;
}


//============================================================
//  win_init_window
//  (main thread)
//============================================================

int sdlwindow_init(running_machine &machine)
{
	mame_printf_verbose("Enter sdlwindow_init\n");
	// determine if we are using multithreading or not
	multithreading_enabled = downcast<sdl_options &>(machine.options()).multithreading();

	// get the main thread ID before anything else
	main_threadid = SDL_ThreadID();

	// ensure we get called on the way out
	machine.add_notifier(MACHINE_NOTIFY_EXIT, machine_notify_delegate(FUNC(sdlwindow_exit), &machine));

	// if multithreading, create a thread to run the windows
	if (multithreading_enabled)
	{
		// create a thread to run the windows from
		work_queue = osd_work_queue_alloc(WORK_QUEUE_FLAG_IO);
		if (work_queue == NULL)
			return 1;
		osd_work_item_queue(work_queue, &sdlwindow_thread_id, NULL, WORK_ITEM_FLAG_AUTO_RELEASE);
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
		if (drawogl_init(machine, &draw))
			video_config.mode = VIDEO_MODE_SOFT;
	}
#endif
#if SDLMAME_SDL2
	if (video_config.mode == VIDEO_MODE_SDL13)
	{
		if (draw13_init(machine, &draw))
			video_config.mode = VIDEO_MODE_SOFT;
	}
#endif
	if (video_config.mode == VIDEO_MODE_SOFT)
	{
		if (drawsdl_init(&draw))
			return 1;
	}

	// set up the window list
	last_window_ptr = &sdl_window_list;
	mame_printf_verbose("Leave sdlwindow_init\n");
	return 0;
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
			mame_printf_warning("sdlwindow_sync: Sleeping...\n");
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


static void sdlwindow_exit(running_machine &machine)
{
	ASSERT_MAIN_THREAD();

	mame_printf_verbose("Enter sdlwindow_exit\n");

	// free all the windows
	while (sdl_window_list != NULL)
	{
		sdl_window_info *temp = sdl_window_list;
		sdl_window_list = temp->next;
		sdlwindow_video_window_destroy(machine, temp);
	}

	// if we're multithreaded, clean up the window thread
	if (multithreading_enabled)
	{
		sdlwindow_sync();
	}

	// kill the drawers
	(*draw.exit)();

	execute_async_wait(&sdlwindow_exit_wt, NULL);

	if (multithreading_enabled)
	{
		osd_work_queue_wait(work_queue, 1000000);
		osd_work_queue_free(work_queue);
	}
	mame_printf_verbose("Leave sdlwindow_exit\n");

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

void sdlwindow_blit_surface_size(sdl_window_info *window, int window_width, int window_height)
{
	INT32 newwidth, newheight;
	int xscale = 1, yscale = 1;
	float desired_aspect = 1.0f;
	INT32 target_width = window_width;
	INT32 target_height = window_height;

	// start with the minimum size
	window->target->compute_minimum_size(newwidth, newheight);

	// compute the appropriate visible area if we're trying to keepaspect
	if (video_config.keepaspect)
	{
		// make sure the monitor is up-to-date
		sdlvideo_monitor_refresh(window->monitor);
		window->target->compute_visible_area(target_width, target_height, sdlvideo_monitor_get_aspect(window->monitor), window->target->orientation(), target_width, target_height);
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
	if (window->target->zoom_to_screen()
		&& (video_config.scale_mode == VIDEO_SCALE_MODE_NONE ))
		newwidth = window_width;

	if ((window->blitwidth != newwidth) || (window->blitheight != newheight))
		sdlwindow_clear(window);

	window->blitwidth = newwidth;
	window->blitheight = newheight;
}


//============================================================
//  sdlwindow_resize
//  (main thread)
//============================================================

static OSDWORK_CALLBACK( sdlwindow_resize_wt )
{
	worker_param *      wp = (worker_param *) param;
	sdl_window_info *   window = wp->window;

	ASSERT_WINDOW_THREAD();

	window->destroy_all_textures(window);
	window->resize(window, wp->resize_new_width, wp->resize_new_height);

	sdlwindow_blit_surface_size(window, wp->resize_new_width, wp->resize_new_height);

	sdlwindow_clear(window);

	osd_free(wp);
	return NULL;
}

void sdlwindow_resize(sdl_window_info *window, INT32 width, INT32 height)
{
	worker_param wp;

	ASSERT_MAIN_THREAD();

	if (width == window->width && height == window->height)
		return;

	clear_worker_param(&wp);
	wp.resize_new_width = width;
	wp.resize_new_height = height;
	wp.window = window;

	execute_async_wait(&sdlwindow_resize_wt, &wp);
}


//============================================================
//  sdlwindow_clear_surface
//  (window thread)
//============================================================

static OSDWORK_CALLBACK( sdlwindow_clear_surface_wt )
{
	worker_param *wp = (worker_param *) param;
	sdl_window_info *window = wp->window;

	ASSERT_WINDOW_THREAD();

	window->clear(window);
	osd_free(wp);
	return NULL;
}

void sdlwindow_clear(sdl_window_info *window)
{
	worker_param *wp = (worker_param *) osd_malloc(sizeof(worker_param));

	clear_worker_param(wp);
	wp->window = window;

	if (SDL_ThreadID() == main_threadid)
	{
		execute_async_wait(&sdlwindow_clear_surface_wt, wp);
		osd_free(wp);
	}
	else
		sdlwindow_clear_surface_wt( (void *) wp, 0);
}


//============================================================
//  sdlwindow_toggle_full_screen
//  (main thread)
//============================================================

static OSDWORK_CALLBACK( sdlwindow_toggle_full_screen_wt )
{
	worker_param *wp = (worker_param *) param;
	sdl_window_info *window = wp->window;

	ASSERT_WINDOW_THREAD();

	// if we are in debug mode, never go full screen
	if (window->machine().debug_flags & DEBUG_FLAG_OSD_ENABLED)
		return NULL;

	// If we are going fullscreen (leaving windowed) remember our windowed size
	if (!window->fullscreen)
	{
		window->windowed_width = window->width;
		window->windowed_height = window->height;
	}

	window->destroy(window);
	sdlinput_release_keys(wp->machine());

	// toggle the window mode
	window->fullscreen = !window->fullscreen;

	complete_create_wt(param, 0);

	return NULL;
}

void sdlwindow_toggle_full_screen(running_machine &machine, sdl_window_info *window)
{
	worker_param wp;

	ASSERT_MAIN_THREAD();

	clear_worker_param(&wp);
	wp.window = window;
	wp.m_machine = &machine;

	execute_async_wait(&sdlwindow_toggle_full_screen_wt, &wp);
}

static OSDWORK_CALLBACK( destroy_all_textures_wt )
{
	worker_param *wp = (worker_param *) param;

	sdl_window_info *window = wp->window;

	window->destroy_all_textures(window);

	osd_free(wp);
	return NULL;
}

void sdlwindow_modify_prescale(running_machine &machine, sdl_window_info *window, int dir)
{
	worker_param wp;
	int new_prescale = window->prescale;

	clear_worker_param(&wp);

	wp.window = window;
	wp.m_machine = &machine;

	if (dir > 0 && window->prescale < 3)
		new_prescale = window->prescale + 1;
	if (dir < 0 && window->prescale > 1)
		new_prescale = window->prescale - 1;

	if (new_prescale != window->prescale)
	{
		if (window->fullscreen && video_config.switchres)
		{
			execute_async_wait(&sdlwindow_video_window_destroy_wt, &wp);

			window->prescale = new_prescale;

			execute_async_wait(&complete_create_wt, &wp);

		}
		else
		{
			execute_async_wait(destroy_all_textures_wt, &wp);
			window->prescale = new_prescale;
		}
		ui_popup_time(1, "Prescale %d", window->prescale);
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
		//SDL_SetRelativeMouseMode
		if (!window->fullscreen && !sdlinput_should_hide_mouse(machine))
		{
			SDL_ShowCursor(SDL_ENABLE);
			if (SDL_GetWindowGrab(window->sdl_window ))
				SDL_SetWindowGrab(window->sdl_window, SDL_FALSE);
		}
		else
		{
			SDL_ShowCursor(SDL_DISABLE);
			if (!SDL_GetWindowGrab(window->sdl_window))
				SDL_SetWindowGrab(window->sdl_window, SDL_TRUE);
		}
		SDL_SetCursor(NULL); // Force an update in case the underlying driver has changed visibility
	}

#else
	// do not do mouse capture if the debugger's enabled to avoid
	// the possibility of losing control
	if (!(machine.debug_flags & DEBUG_FLAG_OSD_ENABLED))
	{
		if ( window->fullscreen || sdlinput_should_hide_mouse(machine) )
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

	clear_worker_param(wp);

	// allocate a new window object
	window = global_alloc_clear(sdl_window_info);
	window->maxwidth = config->width;
	window->maxheight = config->height;
	window->depth = config->depth;
	window->refresh = config->refresh;
	window->monitor = monitor;
	window->m_machine = &machine;
	window->index = index;

	//FIXME: these should be per_window in config-> or even better a bit set
	window->fullscreen = !video_config.windowed;
	window->prescale = video_config.prescale;

	// set the initial maximized state
	// FIXME: Does not belong here
	sdl_options &options = downcast<sdl_options &>(machine.options());
	window->startmaximized = options.maximize();

	if (!window->fullscreen)
	{
		window->windowed_width = config->width;
		window->windowed_height = config->height;
	}
	window->totalColors = config->totalColors;

	// add us to the list
	*last_window_ptr = window;
	last_window_ptr = &window->next;

	draw.attach(&draw, window);

	// create an event that we can use to skip blitting
	window->rendered_event = osd_event_alloc(FALSE, TRUE);

	// load the layout
	window->target = machine.render().target_alloc();

	// set the specific view
	set_starting_view(machine, index, window, options.view(), options.view(index));

	// make the window title
	if (video_config.numscreens == 1)
		sprintf(window->title, "%s: %s [%s]", emulator_info::get_appname(), machine.system().description, machine.system().name);
	else
		sprintf(window->title, "%s: %s [%s] - Screen %d", emulator_info::get_appname(), machine.system().description, machine.system().name, index);

	wp->window = window;

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
	sdlwindow_video_window_destroy(machine, window);
	return 1;
}


//============================================================
//  sdlwindow_video_window_destroy
//  (main thread)
//============================================================

static OSDWORK_CALLBACK( sdlwindow_video_window_destroy_wt )
{
	worker_param *      wp = (worker_param *) param;
	sdl_window_info *   window = wp->window;

	ASSERT_WINDOW_THREAD();

	// free the textures etc
	window->destroy(window);

	// release all keys ...
	sdlinput_release_keys(wp->machine());


	osd_free(wp);
	return NULL;
}

static void sdlwindow_video_window_destroy(running_machine &machine, sdl_window_info *window)
{
	sdl_window_info **prevptr;
	worker_param wp;

	ASSERT_MAIN_THREAD();
	if (multithreading_enabled)
	{
		sdlwindow_sync();
	}

	//osd_event_wait(window->rendered_event, osd_ticks_per_second()*10);

	// remove us from the list
	for (prevptr = &sdl_window_list; *prevptr != NULL; prevptr = &(*prevptr)->next)
		if (*prevptr == window)
		{
			*prevptr = window->next;
			break;
		}

	// free the textures etc
	clear_worker_param(&wp);
	wp.window = window;
	wp.m_machine = &machine;
	execute_async_wait(&sdlwindow_video_window_destroy_wt, &wp);

	// free the render target, after the textures!
	window->machine().render().target_free(window->target);

	// free the event
	osd_event_free(window->rendered_event);

	// free the window itself
	global_free(window);
}


//============================================================
//  pick_best_mode
//============================================================

#if SDLMAME_SDL2
static void pick_best_mode(sdl_window_info *window, int *fswidth, int *fsheight)
{
	int minimum_width, minimum_height, target_width, target_height;
	int i;
	int num;
	float size_score, best_score = 0.0f;

	// determine the minimum width/height for the selected target
	window->target->compute_minimum_size(minimum_width, minimum_height);

	// use those as the target for now
	target_width = minimum_width * MAX(1, window->prescale);
	target_height = minimum_height * MAX(1, window->prescale);

	// if we're not stretching, allow some slop on the minimum since we can handle it
	{
		minimum_width -= 4;
		minimum_height -= 4;
	}

	num = SDL_GetNumDisplayModes(window->monitor->handle);

	if (num == 0)
	{
		mame_printf_error("SDL: No modes available?!\n");
		exit(-1);
	}
	else
	{
		for (i = 0; i < num; ++i)
		{
			SDL_DisplayMode mode;
			SDL_GetDisplayMode(window->monitor->handle, i, &mode);

			// compute initial score based on difference between target and current
			size_score = 1.0f / (1.0f + fabsf((INT32)mode.w - target_width) + fabsf((INT32)mode.h - target_height));

			// if the mode is too small, give a big penalty
			if (mode.w < minimum_width || mode.h < minimum_height)
				size_score *= 0.01f;

			// if mode is smaller than we'd like, it only scores up to 0.1
			if (mode.w < target_width || mode.h < target_height)
				size_score *= 0.1f;

			// if we're looking for a particular mode, that's a winner
			if (mode.w == window->maxwidth && mode.h == window->maxheight)
				size_score = 2.0f;

			// refresh adds some points
			if (window->refresh)
				size_score *= 1.0f / (1.0f + fabsf(window->refresh - mode.refresh_rate) / 10.0f);

			mame_printf_verbose("%4dx%4d@%2d -> %f\n", (int)mode.w, (int)mode.h, (int) mode.refresh_rate, size_score);

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
static void pick_best_mode(sdl_window_info *window, int *fswidth, int *fsheight)
{
	int minimum_width, minimum_height, target_width, target_height;
	int i;
	float size_score, best_score = 0.0f;
	SDL_Rect **modes;

	// determine the minimum width/height for the selected target
	window->target->compute_minimum_size(minimum_width, minimum_height);

	// use those as the target for now
	target_width = minimum_width * MAX(1, window->prescale);
	target_height = minimum_height * MAX(1, window->prescale);

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
	modes = window->monitor->modes;
#endif

	if (modes == (SDL_Rect **)0)
	{
		mame_printf_error("SDL: No modes available?!\n");
		exit(-1);
	}
	else if (modes == (SDL_Rect **)-1)  // all modes are possible
	{
		*fswidth = window->maxwidth;
		*fsheight = window->maxheight;
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
			if (modes[i]->w == window->maxwidth && modes[i]->h == window->maxheight)
				size_score = 2.0f;

			mame_printf_verbose("%4dx%4d -> %f\n", (int)modes[i]->w, (int)modes[i]->h, size_score);

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

void sdlwindow_video_window_update(running_machine &machine, sdl_window_info *window)
{

	osd_ticks_t     event_wait_ticks;
	ASSERT_MAIN_THREAD();

	// adjust the cursor state
	sdlwindow_update_cursor_state(machine, window);

	// if we're visible and running and not in the middle of a resize, draw
	if (window->target != NULL)
	{
		int tempwidth, tempheight;

		// see if the games video mode has changed
		window->target->compute_minimum_size(tempwidth, tempheight);
		if (tempwidth != window->minwidth || tempheight != window->minheight)
		{
			window->minwidth = tempwidth;
			window->minheight = tempheight;
			if (!window->fullscreen)
			{
				sdlwindow_blit_surface_size(window, window->width, window->height);
				sdlwindow_resize(window, window->blitwidth, window->blitheight);
			}
			else if (video_config.switchres)
			{
				pick_best_mode(window, &tempwidth, &tempheight);
				sdlwindow_resize(window, tempwidth, tempheight);
			}
		}

		if (video_config.waitvsync && video_config.syncrefresh)
			event_wait_ticks = osd_ticks_per_second(); // block at most a second
		else
			event_wait_ticks = 0;

		if (osd_event_wait(window->rendered_event, event_wait_ticks))
		{
			worker_param wp;
			render_primitive_list *primlist;

			clear_worker_param(&wp);

			// ensure the target bounds are up-to-date, and then get the primitives
			primlist = &window->get_primitives(window);

			// and redraw now

			wp.list = primlist;
			wp.window = window;
			wp.m_machine = &machine;

			execute_async(&draw_video_contents_wt, &wp);
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
	viewindex = window->target->configured_view(view, index, video_config.numscreens);

	// set the view
	window->target->set_view(viewindex);
	window->start_viewscreen=viewindex;
}


//============================================================
//  complete_create
//  (window thread)
//============================================================

static OSDWORK_CALLBACK( complete_create_wt )
{
	worker_param *      wp = (worker_param *) param;
	sdl_window_info *   window = wp->window;

	int tempwidth, tempheight;
	static int result[2] = {0,1};

	ASSERT_WINDOW_THREAD();
	osd_free(wp);

	if (window->fullscreen)
	{
		// default to the current mode exactly
		tempwidth = window->monitor->monitor_width;
		tempheight = window->monitor->monitor_height;

		// if we're allowed to switch resolutions, override with something better
		if (video_config.switchres)
			pick_best_mode(window, &tempwidth, &tempheight);
	}
	else if (window->windowed_width)
	{
		// if we have a remembered size force the new window size to it
		tempwidth = window->windowed_width;
		tempheight = window->windowed_height;
	}
	else
	{
		if (window->startmaximized)
		{
			tempwidth = tempheight = 0;
			get_max_bounds(window, &tempwidth, &tempheight, video_config.keepaspect );
		}
		else
		{
			/* Create the window directly with the correct aspect
			   instead of letting sdlwindow_blit_surface_size() resize it
			   this stops the window from "flashing" from the wrong aspect
			   size to the right one at startup. */
			tempwidth = (window->maxwidth != 0) ? window->maxwidth : 640;
			tempheight = (window->maxheight != 0) ? window->maxheight : 480;

			get_min_bounds(window, &tempwidth, &tempheight, video_config.keepaspect );
		}
	}

	// initialize the drawing backend
	if (window->create(window, tempwidth, tempheight))
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

	window->draw(window, dc, update);

	frames++;
	currentTime = osd_ticks();
	if(startTime==0||frames==frames_skip4fps)
		startTime=currentTime;
	if( frames>=frames_skip4fps )
		sumdt+=currentTime-t0;
	if( (currentTime-lastTime)>1L*osd_ticks_per_second() && frames>frames_skip4fps )
	{
		dt = (double) (currentTime-startTime) / tps; // in decimale sec.
		mame_printf_info("%6.2lfs, %4lu F, "
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
	sdl_window_info *window = wp->window;

	ASSERT_REDRAW_THREAD();

	// Some configurations require events to be polled in the worker thread
	sdlinput_process_events_buf(wp->machine());

	window->primlist = wp->list;

	// if no bitmap, just fill
	if (window->primlist == NULL)
	{
	}
	// otherwise, render with our drawing system
	else
	{
		if( video_config.perftest )
			measure_fps(window, dc, update);
		else
			window->draw(window, dc, update);
	}

	/* all done, ready for next */
	osd_event_set(window->rendered_event);
	osd_free(wp);

	return NULL;
}


//============================================================
//  constrain_to_aspect_ratio
//  (window thread)
//============================================================

static void constrain_to_aspect_ratio(sdl_window_info *window, int *window_width, int *window_height, int adjustment)
{
	INT32 extrawidth = 0;
	INT32 extraheight = 0;
	INT32 propwidth, propheight;
	INT32 minwidth, minheight;
	INT32 maxwidth, maxheight;
	INT32 viswidth, visheight;
	float pixel_aspect;

	// make sure the monitor is up-to-date
	sdlvideo_monitor_refresh(window->monitor);

	// get the pixel aspect ratio for the target monitor
	pixel_aspect = sdlvideo_monitor_get_aspect(window->monitor);

	// determine the proposed width/height
	propwidth = *window_width - extrawidth;
	propheight = *window_height - extraheight;

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
		maxwidth = window->monitor->center_width - extrawidth;
		maxheight = window->monitor->center_height - extraheight;
	}
	else
	{
		maxwidth = window->monitor->center_width - extrawidth;
		maxheight = window->monitor->center_height - extraheight;

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

	*window_width = viswidth;
	*window_height = visheight;
}


//============================================================
//  get_min_bounds
//  (window thread)
//============================================================

static void get_min_bounds(sdl_window_info *window, int *window_width, int *window_height, int constrain)
{
	INT32 minwidth, minheight;

	// get the minimum target size
	window->target->compute_minimum_size(minwidth, minheight);

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
		constrain_to_aspect_ratio(window, &test1w, &test1h, WMSZ_BOTTOMRIGHT);

		// then constrain with no width limit
		test2w = 10000; test2h = minheight;
		constrain_to_aspect_ratio(window, &test2w, &test2h, WMSZ_BOTTOMRIGHT);

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

static void get_max_bounds(sdl_window_info *window, int *window_width, int *window_height, int constrain)
{
	INT32 maxwidth, maxheight;

	// compute the maximum client area
	maxwidth = window->monitor->center_width;
	maxheight = window->monitor->center_height;

	// clamp to the window's max
	if (window->maxwidth != 0)
	{
		int temp = window->maxwidth + WINDOW_DECORATION_WIDTH;
		if (temp < maxwidth)
			maxwidth = temp;
	}
	if (window->maxheight != 0)
	{
		int temp = window->maxheight + WINDOW_DECORATION_HEIGHT;
		if (temp < maxheight)
			maxheight = temp;
	}

	// constrain to fit
	if (constrain)
		constrain_to_aspect_ratio(window, &maxwidth, &maxheight, WMSZ_BOTTOMRIGHT);
	//else
	{
		maxwidth -= WINDOW_DECORATION_WIDTH;
		maxheight -= WINDOW_DECORATION_HEIGHT;
		*window_width = maxwidth;
		*window_height = maxheight;
	}
}
