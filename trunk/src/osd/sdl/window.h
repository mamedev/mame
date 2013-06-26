//============================================================
//
//  window.h - SDL window handling
//
//  Copyright (c) 1996-2006, Nicola Salmoria and the MAME Team.
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
#include "sdlsync.h"

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
	// Pointer to next window
	sdl_window_info *   next;

	// Pointer to machine
	running_machine &machine() const { assert(m_machine != NULL); return *m_machine; }
	running_machine *   m_machine;

	// Draw Callbacks
	int (*create)(sdl_window_info *window, int width, int height);
	void (*resize)(sdl_window_info *window, int width, int height);
	int (*draw)(sdl_window_info *window, UINT32 dc, int update);
	render_primitive_list &(*get_primitives)(sdl_window_info *window);
	int (*xy_to_render_target)(sdl_window_info *window, int x, int y, int *xt, int *yt);
	void (*destroy_all_textures)(sdl_window_info *window);
	void (*destroy)(sdl_window_info *window);
	void (*clear)(sdl_window_info *window);

	// window handle and info
	char                title[256];

	// monitor info
	sdl_monitor_info *  monitor;
	int                 fullscreen;
	int         index;

	// diverse flags
	int                 minwidth, minheight;
	int                 maxwidth, maxheight;
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

	int                 totalColors;        // total colors from machine/sdl_window_config
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
#endif
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

// core initialization
int sdlwindow_init(running_machine &machine);

// creation/deletion of windows
int sdlwindow_video_window_create(running_machine &machine, int index, sdl_monitor_info *monitor, const sdl_window_config *config);
void sdlwindow_video_window_update(running_machine &machine, sdl_window_info *window);
void sdlwindow_blit_surface_size(sdl_window_info *window, int window_width, int window_height);
void sdlwindow_toggle_full_screen(running_machine &machine, sdl_window_info *window);
void sdlwindow_modify_prescale(running_machine &machine, sdl_window_info *window, int dir);
void sdlwindow_resize(sdl_window_info *window, INT32 width, INT32 height);
void sdlwindow_clear(sdl_window_info *window);


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

int draw13_init(running_machine &machine, sdl_draw_info *callbacks);

#endif /* __SDLWINDOW__ */
