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

/* ------------------------------------------------------
 *
 * All types named osd_* will ultimately be located in
 * the modules tree. They are temporarily maintained in
 * window.h until basic code simplification is finished.
 *
 */

class win_window_info;

class osd_window
{
public:
	osd_window()
	:
#ifdef OSD_SDL
#else
		m_hwnd(0), m_focus_hwnd(0), m_resize_state(0),
		m_maxwidth(0), m_maxheight(0),
		m_refresh(0),
#endif
		m_prescale(1),
		m_primlist(NULL)
  	  {}
	virtual ~osd_window() { }

	virtual render_target *target() = 0;
	virtual int fullscreen() const = 0;
	virtual running_machine &machine() const = 0;

	int prescale() const { return m_prescale; };

	float aspect() const { return monitor()->aspect(); }

#ifdef OSD_SDL
	virtual void blit_surface_size(int &blitwidth, int &blitheight) = 0;
	virtual sdl_monitor_info *monitor() const = 0;
	virtual void get_size(int &w, int &h) = 0;
#if (SDLMAME_SDL2)
	virtual SDL_Window *sdl_window() = 0;
#else
	virtual SDL_Surface *sdl_surface() = 0;
#endif
#else
	virtual win_monitor_info *monitor() const = 0;
	virtual bool win_has_menu() = 0;
	// FIXME: cann we replace winwindow_video_window_monitor(NULL) with monitor() ?
	virtual win_monitor_info *winwindow_video_window_monitor(const RECT *proposed) = 0;

	// window handle and info
	HWND					m_hwnd;
	// FIXME: this is the same as win_window_list->m_hwnd, i.e. first window.
	// During modularization, this should be passed in differently
	HWND         	 		m_focus_hwnd;

	int                 	m_resize_state;
	int                 	m_maxwidth, m_maxheight;
	int                 	m_refresh;
#endif

	int						m_prescale;
	render_primitive_list 	*m_primlist;
};

class osd_renderer
{
public:

	/* Generic flags */
	static const int FLAG_NONE 					= 0x0000;
	static const int FLAG_NEEDS_OPENGL 			= 0x0001;
	static const int FLAG_HAS_VECTOR_SCREEN		= 0x0002;

#if (!(SDLMAME_SDL2))
	/* SDL 1.2 flags */
	static const int FLAG_NEEDS_DOUBLEBUF 		= 0x0100;
	static const int FLAG_NEEDS_ASYNCBLIT 		= 0x0200;
#endif

	osd_renderer(osd_window *window, const int flags)
	: m_window(window), m_flags(flags) { }

	virtual ~osd_renderer() { }

	osd_window &window() { return *m_window; }
	bool has_flags(const int flag) { return ((m_flags & flag)) == flag; }
	void set_flags(int aflag) { m_flags |= aflag; }
	void clear_flags(int aflag) { m_flags &= ~aflag; }


	void notify_changed() { set_flags(FI_CHANGED); }

	/* Interface to be implemented by render code */

	virtual int create() = 0;
	virtual render_primitive_list *get_primitives() = 0;

#ifdef OSD_SDL
	virtual int draw(const UINT32 dc, const int update) = 0;
	virtual int xy_to_render_target(const int x, const int y, int *xt, int *yt) = 0;
#else
	virtual int draw(HDC dc, int update) = 0;
	virtual void save() = 0;
	virtual void record() = 0;
	virtual void toggle_fsfx() = 0;
#endif

	virtual void destroy() = 0;

protected:
	/* Internal flags */
	static const int FI_CHANGED	 				= 0x010000;

private:

	osd_window		*m_window;
	int m_flags;
};

#define OSDWORK_CALLBACK(name)  void *name(void *param, ATTR_UNUSED int threadid)

class sdl_window_info : public osd_window
{
public:
	sdl_window_info(running_machine &a_machine, int index, sdl_monitor_info *a_monitor,
			const sdl_window_config *config)
	: osd_window(), m_next(NULL),
		// Following three are used by input code to defer resizes
#if (SDLMAME_SDL2)
		m_resize_width(0),
		m_resize_height(0),
		m_last_resize(0),
#endif
		 m_minwidth(0), m_minheight(0),
		m_rendered_event(0), m_target(0),
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

	void update();
	void toggle_full_screen();
	void modify_prescale(int dir);
	void resize(INT32 width, INT32 height);
	void destroy();

	void notify_changed();

	void get_size(int &w, int &h)
	{
#if (SDLMAME_SDL2)
		SDL_GetWindowSize(m_sdl_window, &w, &h);
#else
		w = m_sdlsurf->w; h = m_sdlsurf->h;
#endif
	}

	int xy_to_render_target(int x, int y, int *xt, int *yt);

	running_machine &machine() const { return m_machine; }
	sdl_monitor_info *monitor() const { return m_monitor; }
	int fullscreen() const { return m_fullscreen; }

	render_target *target() { return m_target; }
#if (SDLMAME_SDL2)
	SDL_Window *sdl_window() { return m_sdl_window; }
#else
	SDL_Surface *sdl_surface() { return m_sdlsurf; }
#endif

	void blit_surface_size(int &blitwidth, int &blitheight);
	int prescale() const { return m_prescale; }

	// Pointer to next window
	sdl_window_info *   m_next;

#if (SDLMAME_SDL2)
	// These are used in combine resizing events ... #if SDL13_COMBINE_RESIZE
	int                 m_resize_width;
	int                 m_resize_height;
	osd_ticks_t         m_last_resize;
#endif

private:
	// window handle and info
	char                m_title[256];
	int                 m_startmaximized;

	// diverse flags
	int                 m_minwidth, m_minheight;
	int                 m_maxwidth, m_maxheight;
	int                 m_refresh;
	int                 m_depth;
	int                 m_windowed_width;
	int                 m_windowed_height;

	// rendering info
	osd_event *         m_rendered_event;
	render_target *     m_target;

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
	void update_cursor_state();
	void pick_best_mode(int *fswidth, int *fsheight);
	void set_starting_view(running_machine &machine, int index, const char *defview, const char *view);
	void get_min_bounds(int *window_width, int *window_height, int constrain);
	void get_max_bounds(int *window_width, int *window_height, int constrain);
	void set_fullscreen(int afullscreen) { m_fullscreen = afullscreen; }

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
	static OSDWORK_CALLBACK( notify_changed_wt );
	static OSDWORK_CALLBACK( update_cursor_state_wt );

	void measure_fps(UINT32 dc, int update);

};

struct osd_draw_callbacks
{
	osd_renderer *(*create)(osd_window *window);
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

int drawsdl_init(osd_draw_callbacks *callbacks);
const char *drawsdl_scale_mode_str(int index);
int drawsdl_scale_mode(const char *s);

//============================================================
// PROTOTYPES - drawogl.c
//============================================================

int drawogl_init(running_machine &machine, osd_draw_callbacks *callbacks);

//============================================================
// PROTOTYPES - draw13.c
//============================================================

int drawsdl2_init(running_machine &machine, osd_draw_callbacks *callbacks);

//============================================================
// PROTOTYPES - drawbgfx.c
//============================================================

int drawbgfx_init(running_machine &machine, osd_draw_callbacks *callbacks);

#endif /* __SDLWINDOW__ */
