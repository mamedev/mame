// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb
//============================================================
//
//  window.h - Win32 window handling
//
//============================================================
#ifndef MAME_OSD_WINDOWS_WINDOW_H
#define MAME_OSD_WINDOWS_WINDOW_H

#pragma once

#include "emucore.h"
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
	enum
	{
		RESIZE_STATE_NORMAL,
		RESIZE_STATE_RESIZING,
		RESIZE_STATE_PENDING
	};

	win_window_info(running_machine &machine, render_module &renderprovider, int index, const std::shared_ptr<osd_monitor_info> &monitor, const osd_window_config *config);

	bool attached_mode() const { return m_attached_mode; }
	win_window_focus focus() const;

	void update() override;

	virtual osd_dim get_size() override
	{
		RECT client;
		GetClientRect(platform_window(), &client);
		return osd_dim(client.right - client.left, client.bottom - client.top);
	}

	win_window_info *main_window() const { return m_main; }
	void set_main_window(win_window_info &main) { m_main = &main; }

	void capture_pointer() override;
	void release_pointer() override;
	void show_pointer() override;
	void hide_pointer() override;

	void complete_destroy() override;

	// static

	static std::unique_ptr<win_window_info> create(
			running_machine &machine,
			render_module &renderprovider,
			int index,
			const std::shared_ptr<osd_monitor_info> &monitor,
			const osd_window_config *config);

	// static callbacks

	static LRESULT CALLBACK video_window_proc(HWND wnd, UINT message, WPARAM wparam, LPARAM lparam);

	// member variables

	volatile int        m_init_state;

	// window handle and info
	RECT                m_non_fullscreen_bounds;
	int                 m_startmaximized;
	int                 m_isminimized;
	int                 m_ismaximized;

	// monitor info
	int                 m_fullscreen_safe;
	float               m_aspect;

	// rendering info
	std::mutex          m_render_lock;
	unsigned            m_targetview;
	int                 m_targetorient;
	render_layer_config m_targetlayerconfig;
	u32                 m_targetvismask;
	int                 m_targetscalemode;
	bool                m_targetkeepaspect;

	// input info
	std::chrono::steady_clock::time_point  m_lastclicktime;
	int                 m_lastclickx;
	int                 m_lastclicky;
	char16_t            m_last_surrogate;

	HDC                 m_dc;       // only used by GDI renderer!
	int                 m_resize_state;

private:
	struct win_pointer_info : public pointer_info
	{
		static constexpr bool compare(win_pointer_info const &info, WORD ptrid) { return info.ptrid < ptrid; }

		win_pointer_info(win_pointer_info const &) = default;
		win_pointer_info(win_pointer_info &&) = default;
		win_pointer_info &operator=(win_pointer_info const &) = default;
		win_pointer_info &operator=(win_pointer_info &&) = default;

		win_pointer_info(WORD p, POINTER_INPUT_TYPE t, unsigned i, unsigned d);

		WORD ptrid;
		POINTER_INPUT_TYPE type;
	};

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

	// pointer handling helpers
	void pointer_entered(WPARAM wparam, LPARAM lparam);
	void pointer_left(WPARAM wparam, LPARAM lparam);
	void pointer_updated(WPARAM wparam, LPARAM lparam);
	void pointer_capture_changed(WPARAM wparam, LPARAM lparam);
	void mouse_left(WPARAM wparam, LPARAM lparam);
	void mouse_updated(WPARAM wparam, LPARAM lparam);
	void expire_pointer(std::vector<win_pointer_info>::iterator info, POINT const &where, bool canceled);
	void update_pointer(win_pointer_info &info, POINT const &where, unsigned buttons, bool canceled);
	std::vector<win_pointer_info>::iterator map_pointer(WPARAM wparam);
	std::vector<win_pointer_info>::iterator find_pointer(WPARAM wparam);
	std::vector<win_pointer_info>::iterator map_mouse_pointer();
	std::vector<win_pointer_info>::iterator find_mouse_pointer();

	win_window_info *   m_main;
	bool                m_attached_mode;
	bool                m_cursor_clipped;

	// these functions first appear in Windows 8/Server 2012
	OSD_DYNAMIC_API(user32, "User32.dll", "User32.dll");
	OSD_DYNAMIC_API_FN(user32, BOOL, WINAPI, GetPointerType, UINT32, POINTER_INPUT_TYPE *);
	OSD_DYNAMIC_API_FN(user32, BOOL, WINAPI, GetPointerInfo, UINT32, POINTER_INFO *);
	OSD_DYNAMIC_API_FN(user32, BOOL, WINAPI, GetPointerPenInfo, UINT32, POINTER_PEN_INFO *);
	OSD_DYNAMIC_API_FN(user32, BOOL, WINAPI, GetPointerTouchInfo, UINT32, POINTER_TOUCH_INFO *);

	// info on currently active pointers - 64 pointers ought to be enough for anyone
	uint64_t m_pointer_mask;
	unsigned m_next_pointer, m_next_ptrdev;
	bool m_tracking_mouse;
	std::vector<std::pair<HANDLE, unsigned> > m_ptrdev_map;
	std::vector<pointer_dev_info> m_ptrdev_info;
	std::vector<win_pointer_info> m_active_pointers;

	static POINT        s_saved_cursor_pos;
};


//============================================================
//  PROTOTYPES
//============================================================

bool winwindow_has_focus(void);
void winwindow_update_cursor_state(running_machine &machine);

void winwindow_toggle_full_screen(void);
void winwindow_take_snap(void);
void winwindow_take_video(void);
void winwindow_toggle_fsfx(void);

void winwindow_ui_pause(running_machine &machine, int pause);
int winwindow_ui_is_paused(running_machine &machine);

void winwindow_dispatch_message(running_machine &machine, MSG const &message);



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
