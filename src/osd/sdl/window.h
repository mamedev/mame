// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont
//============================================================
//
//  window.h - SDL window handling
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================

#ifndef __SDLWINDOW__
#define __SDLWINDOW__

#include "sdlinc.h"
#include "osdsdl.h"
#include "video.h"

#include "modules/osdwindow.h"

#include <cstdint>


//============================================================
//  TYPE DEFINITIONS
//============================================================

typedef uintptr_t HashT;

#define OSDWORK_CALLBACK(name)  void *name(void *param, ATTR_UNUSED int threadid)

class sdl_window_info : public osd_window
{
public:
	sdl_window_info(running_machine &a_machine, int index, osd_monitor_info *a_monitor,
			const osd_window_config *config)
	: osd_window(), m_next(NULL),
		// Following three are used by input code to defer resizes
		m_resize_width(0),
		m_resize_height(0),
		m_last_resize(0),
		m_minimum_dim(0,0),
		m_windowed_dim(0,0),
		m_rendered_event(0), m_target(0),
		m_sdl_window(NULL),
		m_machine(a_machine), m_monitor(a_monitor), m_fullscreen(0)
	{
		m_win_config = *config;
		m_index = index;

		//FIXME: these should be per_window in config-> or even better a bit set
		m_fullscreen = !video_config.windowed;
		m_prescale = video_config.prescale;

		m_windowed_dim = osd_dim(config->width, config->height);
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

	osd_dim get_size() override
	{
		int w=0; int h=0;
		SDL_GetWindowSize(m_sdl_window, &w, &h);
		return osd_dim(w,h);
	}

	int xy_to_render_target(int x, int y, int *xt, int *yt);

	running_machine &machine() const override { return m_machine; }
	osd_monitor_info *monitor() const override { return m_monitor; }
	int fullscreen() const override { return m_fullscreen; }

	render_target *target() override { return m_target; }
	SDL_Window *sdl_window() override { return m_sdl_window; }

	int prescale() const { return m_prescale; }
	osd_renderer &renderer() const { return *m_renderer; }

	// Pointer to next window
	sdl_window_info *   m_next;

	// These are used in combine resizing events ... #if SDL13_COMBINE_RESIZE
	int                 m_resize_width;
	int                 m_resize_height;
	osd_ticks_t         m_last_resize;

private:
	// window handle and info
	char                m_title[256];
	int                 m_startmaximized;

	// dimensions
	osd_dim             m_minimum_dim;
	osd_dim             m_windowed_dim;

	// rendering info
	osd_event *         m_rendered_event;
	render_target *     m_target;

	// Needs to be here as well so we can identify window
	SDL_Window          *m_sdl_window;
	// Original display_mode
	SDL_DisplayMode m_original_mode;

	int                 m_extra_flags;

	void set_renderer(osd_renderer *renderer)
	{
		m_renderer = renderer;
	}

	static OSDWORK_CALLBACK( complete_create_wt );

private:
	int wnd_extra_width();
	int wnd_extra_height();
	void set_starting_view(int index, const char *defview, const char *view);
	osd_rect constrain_to_aspect_ratio(const osd_rect &rect, int adjustment);
	osd_dim get_min_bounds(int constrain);
	osd_dim get_max_bounds(int constrain);
	void update_cursor_state();
	osd_dim pick_best_mode();
	void set_fullscreen(int afullscreen) { m_fullscreen = afullscreen; }

	// Pointer to machine
	running_machine &   m_machine;
	// monitor info
	osd_monitor_info *  m_monitor;
	int                 m_fullscreen;
	osd_renderer *      m_renderer;

	// static callbacks ...

	static OSDWORK_CALLBACK( sdlwindow_resize_wt );
	static OSDWORK_CALLBACK( draw_video_contents_wt );
	static OSDWORK_CALLBACK( sdlwindow_video_window_destroy_wt );
	static OSDWORK_CALLBACK( sdlwindow_toggle_full_screen_wt );
	static OSDWORK_CALLBACK( notify_changed_wt );
	static OSDWORK_CALLBACK( update_cursor_state_wt );

	void measure_fps(int update);

};

struct osd_draw_callbacks
{
	osd_renderer *(*create)(osd_window *window);
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
