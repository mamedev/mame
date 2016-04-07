// license:BSD-3-Clause
// copyright-holders:Aaron Giles
//============================================================
//
//  window.h - Win32 window handling
//
//============================================================

#ifndef __WIN_WINDOW__
#define __WIN_WINDOW__

// standard windows headers
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>

#include <mutex>
#include "video.h"
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

class win_window_info  : public osd_window
{
public:
	win_window_info(running_machine &machine);
	virtual ~win_window_info();

	running_machine &machine() const override { return m_machine; }

	virtual render_target *target() override { return m_target; }
	int fullscreen() const override { return m_fullscreen; }

	void update();

	virtual osd_monitor_info *winwindow_video_window_monitor(const osd_rect *proposed) override;

	virtual bool win_has_menu() override
	{
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
		return GetMenu(m_hwnd) ? true : false;
#else
		return false;
#endif
	}

	virtual osd_dim get_size() override
	{
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
		RECT client;
		GetClientRect(m_hwnd, &client);
		return osd_dim(client.right - client.left, client.bottom - client.top);
#else
		throw ref new Platform::NotImplementedException();
#endif
	}

	virtual osd_monitor_info *monitor() const override { return m_monitor; }

	void destroy();

	// static

	static void create(running_machine &machine, int index, osd_monitor_info *monitor, const osd_window_config *config);

	// static callbacks

	static LRESULT CALLBACK video_window_proc(HWND wnd, UINT message, WPARAM wparam, LPARAM lparam);
	static unsigned __stdcall thread_entry(void *param);

	// member variables

	win_window_info *   m_next;
	volatile int        m_init_state;

	// window handle and info
	char                m_title[256];
	RECT                m_non_fullscreen_bounds;
	int                 m_startmaximized;
	int                 m_isminimized;
	int                 m_ismaximized;

	// monitor info
	osd_monitor_info *  m_monitor;
	int                 m_fullscreen;
	int                 m_fullscreen_safe;
	float               m_aspect;

	// rendering info
	std::mutex          m_render_lock;
	render_target *     m_target;
	int                 m_targetview;
	int                 m_targetorient;
	render_layer_config m_targetlayerconfig;

	// input info
	DWORD               m_lastclicktime;
	int                 m_lastclickx;
	int                 m_lastclicky;

	// drawing data
	osd_renderer *      m_renderer;

private:
	void draw_video_contents(HDC dc, int update);
	int complete_create();
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

	running_machine &   m_machine;
};

struct osd_draw_callbacks
{
	osd_renderer *(*create)(osd_window *window);
	void (*exit)(void);
};

//============================================================
//  GLOBAL VARIABLES
//============================================================

// windows
extern win_window_info *win_window_list;



//============================================================
//  PROTOTYPES
//============================================================

BOOL winwindow_has_focus(void);
void winwindow_update_cursor_state(running_machine &machine);

extern LRESULT CALLBACK winwindow_video_window_proc_ui(HWND wnd, UINT message, WPARAM wparam, LPARAM lparam);

void winwindow_toggle_full_screen(void);
void winwindow_take_snap(void);
void winwindow_take_video(void);
void winwindow_toggle_fsfx(void);

void winwindow_process_events_periodic(running_machine &machine);
void winwindow_process_events(running_machine &machine, int ingame, bool nodispatch);

void winwindow_ui_pause_from_window_thread(running_machine &machine, int pause);
void winwindow_ui_pause_from_main_thread(running_machine &machine, int pause);
int winwindow_ui_is_paused(running_machine &machine);

void winwindow_ui_exec_on_main_thread(void (*func)(void *), void *param);
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

#endif
