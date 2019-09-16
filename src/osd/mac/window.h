// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont
//============================================================
//
//  window.h - SDL window handling
//
//  Mac OSD by R. Belmont
//
//============================================================

#ifndef __MACWINDOW__
#define __MACWINDOW__

#include "osdmac.h"
#include "video.h"

#include "modules/osdwindow.h"

#include <cstdint>
#include <memory>
#include <list>

//============================================================
//  TYPE DEFINITIONS
//============================================================

class render_target;

typedef uintptr_t HashT;

#define OSDWORK_CALLBACK(name)  void *name(void *param, ATTR_UNUSED int threadid)

class mac_window_info : public osd_window_t<void *>
{
public:
	mac_window_info(running_machine &a_machine, int index, std::shared_ptr<osd_monitor_info> a_monitor,
			const osd_window_config *config);

	~mac_window_info();

	int window_init();

	void update() override;
	void toggle_full_screen();
	void modify_prescale(int dir);
	void resize(int32_t width, int32_t height);
	void destroy() override;

	void capture_pointer() override;
	void release_pointer() override;
	void show_pointer() override;
	void hide_pointer() override;

	void notify_changed();

	osd_dim get_size() override;

	int xy_to_render_target(int x, int y, int *xt, int *yt);

	running_machine &machine() const override { return m_machine; }
	osd_monitor_info *monitor() const override { return m_monitor.get(); }
	int fullscreen() const override { return m_fullscreen; }

	render_target *target() override { return m_target; }

	int prescale() const { return m_prescale; }

	// Pointer to next window
	mac_window_info *   m_next;

private:
	// window handle and info
	char                m_title[256];
	int                 m_startmaximized;

	// dimensions
	osd_dim             m_minimum_dim;
	osd_dim             m_windowed_dim;

	// rendering info
	osd_event           m_rendered_event;
	render_target *     m_target;

	//int                 m_extra_flags;

	// returns 0 on success, else 1
	int complete_create();
	void complete_destroy();

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
	std::shared_ptr<osd_monitor_info>  m_monitor;
	int                                m_fullscreen;
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
// PROTOTYPES - drawogl.c
//============================================================

int drawogl_init(running_machine &machine, osd_draw_callbacks *callbacks);

//============================================================
// PROTOTYPES - drawbgfx.c
//============================================================

int drawbgfx_init(running_machine &machine, osd_draw_callbacks *callbacks);

#endif /* __MACWINDOW__ */
