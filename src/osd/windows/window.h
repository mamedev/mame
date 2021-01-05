// license:BSD-3-Clause
// copyright-holders:Aaron Giles
//============================================================
//
//  window.h - Win32 window handling
//
//============================================================
#ifndef MAME_OSD_WINDOWS_WINDOW_H
#define MAME_OSD_WINDOWS_WINDOW_H

#pragma once

#include "render.h"

#include "modules/osdwindow.h"
#include "modules/lib/osdlib.h"

#include <chrono>
#include <list>
#include <memory>
#include <mutex>
#include <utility>
#include <vector>

// standard windows headers
#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>


//============================================================
//  CONSTANTS
//============================================================

#define RESIZE_STATE_NORMAL     0
#define RESIZE_STATE_RESIZING   1
#define RESIZE_STATE_PENDING    2



//============================================================
//  TYPE DEFINITIONS
//============================================================

enum class win_window_focus
{
	NONE,       // neither this window nor this thread have focus
	THREAD,     // a window in this thread has focus
	WINDOW      // this window has focus directly
};


class win_window_info  : public osd_window_t<HWND>
{
public:
	win_window_info(running_machine &machine, int index, std::shared_ptr<osd_monitor_info> monitor, const osd_window_config *config);

	bool attached_mode() const { return m_attached_mode; }
	win_window_focus focus() const;

	void update() override;

	virtual bool win_has_menu() override
	{
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
		return GetMenu(platform_window()) ? true : false;
#else
		return false;
#endif
	}

	virtual osd_dim get_size() override
	{
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
		RECT client;
		GetClientRect(platform_window(), &client);
		return osd_dim(client.right - client.left, client.bottom - client.top);
#else
		throw ref new Platform::NotImplementedException();
#endif
	}

	void capture_pointer() override;
	void release_pointer() override;
	void show_pointer() override;
	void hide_pointer() override;

	void complete_destroy() override;

	// static

	static void create(running_machine &machine, int index, std::shared_ptr<osd_monitor_info> monitor, const osd_window_config *config);

	// static callbacks

	static LRESULT CALLBACK video_window_proc(HWND wnd, UINT message, WPARAM wparam, LPARAM lparam);

	// member variables

	volatile int        m_init_state;

	// window handle and info
	char                m_title[256];
	RECT                m_non_fullscreen_bounds;
	int                 m_startmaximized;
	int                 m_isminimized;
	int                 m_ismaximized;

	// monitor info
	int                                m_fullscreen_safe;
	float                              m_aspect;

	// rendering info
	std::mutex          m_render_lock;
	unsigned            m_targetview;
	int                 m_targetorient;
	render_layer_config m_targetlayerconfig;
	u32                 m_targetvismask;

	// input info
	std::chrono::steady_clock::time_point  m_lastclicktime;
	int                                    m_lastclickx;
	int                                    m_lastclicky;

private:
	void draw_video_contents(HDC dc, bool update);
	int complete_create();
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

	static POINT        s_saved_cursor_pos;

#if !WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
	static Windows::UI::Core::CoreCursor^ s_cursor;
#endif

	bool                m_attached_mode;
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
void winwindow_update_cursor_state(running_machine &machine);

extern LRESULT CALLBACK winwindow_video_window_proc_ui(HWND wnd, UINT message, WPARAM wparam, LPARAM lparam);

void winwindow_toggle_full_screen(void);
void winwindow_take_snap(void);
void winwindow_take_video(void);
void winwindow_toggle_fsfx(void);

void winwindow_process_events_periodic(running_machine &machine);
void winwindow_process_events(running_machine &machine, bool ingame, bool nodispatch);

void winwindow_ui_pause(running_machine &machine, int pause);
int winwindow_ui_is_paused(running_machine &machine);

void winwindow_dispatch_message(running_machine &machine, MSG *message);

extern int win_create_menu(running_machine &machine, HMENU *menus);



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

#endif // MAME_OSD_WINDOWS_WINDOW_H
