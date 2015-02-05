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

class sdl_window_info;

class osd_renderer
{
public:

	/* Generic flags */
	static const int FLAG_NONE 					= 0x0000;
	static const int FLAG_NEEDS_OPENGL 			= 0x0001;

#if (!(SDLMAME_SDL2))
	/* SDL 1.2 flags */
	static const int FLAG_NEEDS_DOUBLEBUF 		= 0x0100;
	static const int FLAG_NEEDS_ASYNCBLIT 		= 0x0200;
#endif

	osd_renderer(sdl_window_info *window, const int flags)
	: m_window(window), m_flags(flags) { }

	virtual ~osd_renderer() { }

	sdl_window_info &window() { return *m_window; }
	int flags() const { return m_flags; }
	bool check_flag(const int flag) { return ((m_flags & flag)) == flag; }

	virtual int create(const int width, const int height) = 0;
	virtual void resize(const int width, const int height) = 0;
	virtual int draw(const UINT32 dc, const int update) = 0;
	virtual int xy_to_render_target(const int x, const int y, int *xt, int *yt) = 0;
	virtual void destroy_all_textures() = 0;
	virtual void destroy() = 0;
	virtual void clear() = 0;

private:
	sdl_window_info *m_window;
	int m_flags;
};

#define OSDWORK_CALLBACK(name)  void *name(void *param, ATTR_UNUSED int threadid)

class sdl_window_info
{
public:
	sdl_window_info(running_machine &a_machine, int index, sdl_monitor_info *a_monitor,
			const sdl_window_config *config)
	: m_next(NULL), m_primlist(NULL),
		m_start_viewscreen(0),
		// Following three are used by input code to defer resizes
#if (SDLMAME_SDL2)
		m_resize_width(0),
		m_resize_height(0),
		m_last_resize(0),
#endif
		 m_minwidth(0), m_minheight(0),
		m_rendered_event(0), m_target(0),
		m_width(0), m_height(0), m_blitwidth(0), m_blitheight(0),
#if (SDLMAME_SDL2)
		m_sdl_window(NULL),

#else
		m_sdlsurf(NULL),
#endif
		m_machine(a_machine), m_monitor(a_monitor), m_fullscreen(0), m_index(0)
	{
		m_maxwidth = config->width;
		m_maxheight = config->height;
		m_depth = config->depth;
		m_refresh = config->refresh;
		m_index = index;

		//FIXME: these should be per_window in config-> or even better a bit set
		m_fullscreen = !video_config.windowed;
		m_prescale = video_config.prescale;

		m_windowed_width = config->width;
		m_windowed_height = config->height;
	}

	~sdl_window_info()
	{
		global_free(m_renderer);
	}

	int window_init();
	void destroy();

	void update();
	void toggle_full_screen();
	void modify_prescale(int dir);
	void resize(INT32 width, INT32 height);
	void clear();
	int xy_to_render_target(int x, int y, int *xt, int *yt);

	void get_min_bounds(int *window_width, int *window_height, int constrain);
	void get_max_bounds(int *window_width, int *window_height, int constrain);

	running_machine &machine() const { return m_machine; }
	sdl_monitor_info *monitor() const { return m_monitor; }
	int fullscreen() const { return m_fullscreen; }

	void set_fullscreen(int afullscreen) { m_fullscreen = afullscreen; }
	void update_cursor_state();

	void blit_surface_size(int window_width, int window_height);
	void pick_best_mode(int *fswidth, int *fsheight);
	void set_starting_view(running_machine &machine, int index, const char *defview, const char *view);

	int index() const { return m_index; }



	render_target *target() { return m_target; }
#if (SDLMAME_SDL2)
	SDL_Window *sdl_window() { return m_sdl_window; }
#else
	SDL_Surface *sdl_surface() { return m_sdlsurf; }
#endif
	int	width() const { return m_width; }
	int height() const { return m_height; }

	int	blitwidth() const { return m_blitwidth; }
	int blitheight() const { return m_blitheight; }
	int prescale() const { return m_prescale; }

	// Pointer to next window
	sdl_window_info *   m_next;

	// FIXME: renderer should deal with this
	render_primitive_list *m_primlist;
	int                 m_start_viewscreen;
#if (SDLMAME_SDL2)
	// These are used in combine resizing events ... #if SDL13_COMBINE_RESIZE
	int                 m_resize_width;
	int                 m_resize_height;
	osd_ticks_t         m_last_resize;
#endif

private:
	// window handle and info
	char                m_title[256];

	// diverse flags
	int                 m_minwidth, m_minheight;
	int                 m_maxwidth, m_maxheight;
	int                 m_depth;
	int                 m_refresh;
	int                 m_windowed_width;
	int                 m_windowed_height;
	int                 m_startmaximized;

	// rendering info
	osd_event *         m_rendered_event;
	render_target *     m_target;

	// cache of physical width() and height()
	int                 m_width;
	int                 m_height;

	// current m_blitwidth and height()
	int                 m_blitwidth;
	int                 m_blitheight;


	int                 m_prescale;

#if (SDLMAME_SDL2)
	// Needs to be here as well so we can identify window
	SDL_Window          *m_sdl_window;
	// Original display_mode
	SDL_DisplayMode m_original_mode;
#else
	// SDL surface
	SDL_Surface         *m_sdlsurf;
#endif

	int 				m_extra_flags;

	void set_renderer(osd_renderer *renderer)
	{
		m_renderer = renderer;
	}

	static OSDWORK_CALLBACK( complete_create_wt );
protected:
	osd_renderer &renderer() { return *m_renderer; }
private:
	void constrain_to_aspect_ratio(int *window_width, int *window_height, int adjustment);

	// Pointer to machine
	running_machine &   m_machine;
	// monitor info
	sdl_monitor_info *  m_monitor;
	int                 m_fullscreen;
	int                 m_index;
	osd_renderer *		m_renderer;

	// static callbacks ...

	static OSDWORK_CALLBACK( sdlwindow_resize_wt );
	static OSDWORK_CALLBACK( draw_video_contents_wt );
	static OSDWORK_CALLBACK( sdlwindow_video_window_destroy_wt );
	static OSDWORK_CALLBACK( sdlwindow_toggle_full_screen_wt );
	static OSDWORK_CALLBACK( sdlwindow_clear_surface_wt );
	static OSDWORK_CALLBACK( destroy_all_textures_wt );

	void measure_fps(UINT32 dc, int update);

};

struct sdl_draw_info
{
	osd_renderer *(*create)(sdl_window_info *window);
	void (*exit)(void);
};

//============================================================
//  GLOBAL VARIABLES
//============================================================

// window - list
extern sdl_window_info *sdl_window_list;

//============================================================
//  PROTOTYPES
//============================================================

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

//============================================================
// PROTOTYPES - drawbgfx.c
//============================================================

int drawbgfx_init(running_machine &machine, sdl_draw_info *callbacks);

#endif /* __SDLWINDOW__ */
