// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Brad Hughes
//============================================================
//
//  window.h - UWP window handling
//
//============================================================

#ifndef __UWP_WINDOW__
#define __UWP_WINDOW__

// standard windows headers
#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <inspectable.h>
#include <agile.h>

#include <chrono>
#include <mutex>
#include <memory>
#include <list>

#include "render.h"

#include "modules/osdwindow.h"

//============================================================
//  PARAMETERS
//============================================================


//============================================================
//  CONSTANTS
//============================================================

#define RESIZE_STATE_NORMAL     0
#define RESIZE_STATE_RESIZING   1
#define RESIZE_STATE_PENDING    2



//============================================================
//  TYPE DEFINITIONS
//============================================================

class uwp_window_info  : public osd_window_t<Platform::Agile<Windows::UI::Core::CoreWindow>>
{
public:
	uwp_window_info(running_machine &machine, int index, std::shared_ptr<osd_monitor_info> monitor, const osd_window_config *config);

	running_machine &machine() const override { return m_machine; }

	virtual render_target *target() override { return m_target; }
	int fullscreen() const override { return m_fullscreen; }

	void update() override;

	virtual osd_dim get_size() override
	{
		auto bounds = platform_window()->Bounds;
		return osd_dim(bounds.Width, bounds.Height);
	}

	bool win_has_menu() override
	{
		return false;
	}

	void capture_pointer() override;
	void release_pointer() override;
	void show_pointer() override;
	void hide_pointer() override;

	virtual osd_monitor_info *monitor() const override { return m_monitor.get(); }

	void destroy() override;

	// static

	static void create(running_machine &machine, int index, std::shared_ptr<osd_monitor_info> monitor, const osd_window_config *config);

	// member variables

	uwp_window_info *   m_next;
	volatile int        m_init_state;

	// window handle and info
	char                m_title[256];
	RECT                m_non_fullscreen_bounds;
	int                 m_startmaximized;
	int                 m_isminimized;
	int                 m_ismaximized;

	// monitor info
	std::shared_ptr<osd_monitor_info>  m_monitor;
	int                                m_fullscreen;
	int                                m_fullscreen_safe;
	float                              m_aspect;

	// rendering info
	std::mutex          m_render_lock;
	render_target *     m_target;
	unsigned            m_targetview;
	int                 m_targetorient;
	render_layer_config m_targetlayerconfig;
	u32                 m_targetvismask;

	// input info
	std::chrono::system_clock::time_point  m_lastclicktime;
	int                                    m_lastclickx;
	int                                    m_lastclicky;

private:
	int complete_create();
	void draw_video_contents(bool update);
	void set_starting_view(int index, const char *defview, const char *view);
	int wnd_extra_width();
	int wnd_extra_height();
	osd_rect constrain_to_aspect_ratio(const osd_rect &rect, int adjustment);
	osd_dim get_min_bounds(int constrain);
	osd_dim get_max_bounds(int constrain);
	void update_minmax_state();
	void minimize_window();
	void maximize_window();
	void adjust_window_position_after_major_change();
	void set_fullscreen(int fullscreen);
	std::shared_ptr<osd_monitor_info> monitor_from_rect(const osd_rect* proposed) const;

	static POINT        s_saved_cursor_pos;

	static Windows::UI::Core::CoreCursor^ s_cursor;

	running_machine &   m_machine;
};

struct osd_draw_callbacks
{
	osd_renderer *(*create)(osd_window *window);
	void (*exit)(void);
};


//============================================================
//  PROTOTYPES
//============================================================

bool winwindow_has_focus(void);
void winwindow_process_events(running_machine &machine, bool ingame, bool nodispatch);
void winwindow_process_events_periodic(running_machine &machine);

//============================================================
//  rect_width / rect_height
//============================================================

static inline int rect_width(const RECT *rect)
{
	return rect->right - rect->left;
}


static inline int rect_height(const RECT *rect)
{
	return rect->bottom - rect->top;
}

#endif
