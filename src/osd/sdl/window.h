// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont
//============================================================
//
//  window.h - SDL window handling
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================

#ifndef MAME_OSD_SDL_WINDOW_H
#define MAME_OSD_SDL_WINDOW_H

#include "osdsdl.h"
#include "video.h"

#include "modules/osdwindow.h"

#include <cstdint>
#include <memory>
#include <list>


//============================================================
//  TYPE DEFINITIONS
//============================================================

class render_target;

// forward of SDL_DisplayMode not possible (typedef struct) - define wrapper

class SDL_DM_Wrapper;

typedef uintptr_t HashT;

#define OSDWORK_CALLBACK(name)  void *name(void *param, ATTR_UNUSED int threadid)

class sdl_window_info : public osd_window_t<SDL_Window*>
{
public:
	sdl_window_info(running_machine &a_machine, int index, std::shared_ptr<osd_monitor_info> a_monitor,
			const osd_window_config *config);

	~sdl_window_info();

	int window_init();

	void update() override;
	void toggle_full_screen();
	void modify_prescale(int dir);
	void resize(int32_t width, int32_t height);
	void complete_destroy() override;

	void capture_pointer() override;
	void release_pointer() override;
	void show_pointer() override;
	void hide_pointer() override;

	void notify_changed();

	osd_dim get_size() override;

	int xy_to_render_target(int x, int y, int *xt, int *yt);

private:
	// window handle and info
	char                m_title[256];
	int                 m_startmaximized;

	// dimensions
	osd_dim             m_minimum_dim;
	osd_dim             m_windowed_dim;

	// rendering info
	osd_event           m_rendered_event;

	// Original display_mode
	std::unique_ptr<SDL_DM_Wrapper> m_original_mode;

	int                 m_extra_flags;

	// returns 0 on success, else 1
	int complete_create();

private:
	int wnd_extra_width();
	int wnd_extra_height();
	osd_rect constrain_to_aspect_ratio(const osd_rect &rect, int adjustment);
	osd_dim get_min_bounds(int constrain);
	osd_dim get_max_bounds(int constrain);
	void update_cursor_state();
	osd_dim pick_best_mode();
	void set_fullscreen(int afullscreen) { m_fullscreen = afullscreen; }

	// monitor info
	bool                               m_mouse_captured;
	bool                               m_mouse_hidden;

	void measure_fps(int update);

};

struct osd_draw_callbacks
{
	osd_renderer *(*create)(osd_window *window);
};

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

#endif // MAME_OSD_SDL_WINDOW_H
