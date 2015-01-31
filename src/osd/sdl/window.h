//============================================================
//
//  window.h - SDL window handling
//
//  Copyright (c) 1996-2014, Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================

#ifndef __SDLWINDOW__
#define __SDLWINDOW__

#include "sdlinc.h"
#include "video.h"
#include "render.h"
#include "modules/sync/osdsync.h"

#include "osd_opengl.h"
#include "osdsdl.h"

// I don't like this, but we're going to get spurious "cast to integer of different size" warnings on
// at least one architecture without doing it this way.
#ifdef PTR64
typedef UINT64 HashT;
#else
typedef UINT32 HashT;
#endif

//============================================================
//  TYPE DEFINITIONS
//============================================================

struct sdl_window_info
{
	sdl_window_info(running_machine *a_machine, sdl_monitor_info *a_monitor,
			int index, const sdl_window_config *config)
	: next(NULL), m_minwidth(0), m_minheight(0),
		startmaximized(0),
		rendered_event(0), target(0), primlist(NULL), dxdata(NULL),
		width(0), height(0), blitwidth(0), blitheight(0),
		start_viewscreen(0),
#if (SDLMAME_SDL2)
		sdl_window(NULL),
		resize_width(0),
		resize_height(0),
		last_resize(0),
#else
		screen_width(0), screen_height(0),
#endif
		m_machine(a_machine), m_monitor(a_monitor), m_fullscreen(0), m_index(0)
	{
		m_maxwidth = config->width;
		m_maxheight = config->height;
		depth = config->depth;
		refresh = config->refresh;
		m_index = index;

		//FIXME: these should be per_window in config-> or even better a bit set
		m_fullscreen = !video_config.windowed;
		prescale = video_config.prescale;

		windowed_width = config->width;
		windowed_height = config->height;
	}

	void video_window_update(running_machine &machine);
	void blit_surface_size(int window_width, int window_height);
	void toggle_full_screen(running_machine &machine);
	void modify_prescale(running_machine &machine, int dir);
	void window_resize(INT32 width, INT32 height);
	void window_clear();

	void video_window_destroy(running_machine &machine);
	void pick_best_mode(int *fswidth, int *fsheight);
	void get_min_bounds(int *window_width, int *window_height, int constrain);
	void get_max_bounds(int *window_width, int *window_height, int constrain);

	// Pointer to next window
	sdl_window_info *   next;

	running_machine &machine() const { assert(m_machine != NULL); return *m_machine; }
	sdl_monitor_info *monitor() const { return m_monitor; }
	int fullscreen() const { return m_fullscreen; }
	int index() const { return m_index; }

	void set_fullscreen(int afullscreen) { m_fullscreen = afullscreen; }

	// Draw Callbacks
	int (*create)(sdl_window_info *window, int width, int height);
	void (*resize)(sdl_window_info *window, int width, int height);
	int (*draw)(sdl_window_info *window, UINT32 dc, int update);
	void (*set_target_bounds)(sdl_window_info *window);
	int (*xy_to_render_target)(sdl_window_info *window, int x, int y, int *xt, int *yt);
	void (*destroy_all_textures)(sdl_window_info *window);
	void (*destroy)(sdl_window_info *window);
	void (*clear)(sdl_window_info *window);

	// window handle and info
	char                title[256];

	// diverse flags
	int                 m_minwidth, m_minheight;
	int                 m_maxwidth, m_maxheight;
	int                 depth;
	int                 refresh;
	int                 windowed_width;
	int                 windowed_height;
	int                 startmaximized;

	// rendering info
	osd_event *         rendered_event;
	render_target *     target;
	render_primitive_list *primlist;

	// drawing data
	void *              dxdata;

	// cache of physical width and height
	int                 width;
	int                 height;

	// current blitwidth and height
	int                 blitwidth;
	int                 blitheight;

	int                 start_viewscreen;

	// GL specific
	int                 prescale;

#if (SDLMAME_SDL2)
	// Needs to be here as well so we can identify window
	SDL_Window          *sdl_window;
	// These are used in combine resizing events ... #if SDL13_COMBINE_RESIZE
	int                 resize_width;
	int                 resize_height;
	osd_ticks_t         last_resize;
#else
	int                 screen_width;
	int                 screen_height;
#endif

private:
	void constrain_to_aspect_ratio(int *window_width, int *window_height, int adjustment);

	// Pointer to machine
	running_machine *   m_machine;
	// monitor info
	sdl_monitor_info *  m_monitor;
	int                 m_fullscreen;
	int                 m_index;

};

struct sdl_draw_info
{
	void (*exit)(void);
	void (*attach)(sdl_draw_info *info, sdl_window_info *window);
};

//============================================================
//  GLOBAL VARIABLES
//============================================================

// window - list
extern sdl_window_info *sdl_window_list;

//============================================================
//  PROTOTYPES
//============================================================

// creation/deletion of windows
int sdlwindow_video_window_create(running_machine &machine, int index, sdl_monitor_info *monitor, const sdl_window_config *config);

//============================================================
// PROTOTYPES - drawsdl.c
//============================================================

int drawsdl_init(sdl_draw_info *callbacks);
const char *drawsdl_scale_mode_str(int index);
int drawsdl_scale_mode(const char *s);

//============================================================
// PROTOTYPES - drawogl.c
//============================================================

int drawogl_init(running_machine &machine, sdl_draw_info *callbacks);

//============================================================
// PROTOTYPES - draw13.c
//============================================================

int drawsdl2_init(running_machine &machine, sdl_draw_info *callbacks);

#endif /* __SDLWINDOW__ */
