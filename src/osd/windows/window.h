// license:BSD-3-Clause
// copyright-holders:Aaron Giles
//============================================================
//
//  window.h - Win32 window handling
//
//============================================================

#ifndef __WIN_WINDOW__
#define __WIN_WINDOW__

#include "video.h"
#include "render.h"


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

class win_window_info;

class osd_renderer
{
public:

	/* Generic flags */
	static const int FLAG_NONE 					= 0x0000;
	static const int FLAG_NEEDS_OPENGL 			= 0x0001;

	/* SDL 1.2 flags */
	static const int FLAG_NEEDS_DOUBLEBUF 		= 0x0100;
	static const int FLAG_NEEDS_ASYNCBLIT 		= 0x0200;

	osd_renderer(win_window_info *window, const int flags)
	: m_window(window), m_flags(flags) { }

	virtual ~osd_renderer() { }

	win_window_info &window() { return *m_window; }
	int flags() const { return m_flags; }
	bool check_flag(const int flag) { return ((m_flags & flag)) == flag; }

	virtual int init() = 0;
	virtual render_primitive_list *get_primitives() = 0;
	virtual int draw(HDC dc, int update) = 0;
	virtual void save() = 0;
	virtual void record() = 0;
	virtual void toggle_fsfx() = 0;
	virtual void destroy() = 0;

private:
	win_window_info *m_window;
	int m_flags;
};

class win_window_info
{
public:
	win_window_info(running_machine &machine);
	virtual ~win_window_info();

	running_machine &machine() const { return m_machine; }

	void update();

	win_window_info *   m_next;
	volatile int        m_init_state;

	// window handle and info
	HWND                m_hwnd;
	HWND                m_focus_hwnd;
	char                m_title[256];
	RECT                m_non_fullscreen_bounds;
	int                 m_startmaximized;
	int                 m_isminimized;
	int                 m_ismaximized;
	int                 m_resize_state;

	// monitor info
	win_monitor_info *  m_monitor;
	int                 m_fullscreen;
	int                 m_fullscreen_safe;
	int                 m_maxwidth, m_maxheight;
	int                 m_refresh;
	float               m_aspect;

	// rendering info
	osd_lock *          m_render_lock;
	render_target *     m_target;
	int                 m_targetview;
	int                 m_targetorient;
	render_layer_config m_targetlayerconfig;
	render_primitive_list *m_primlist;

	// input info
	DWORD               m_lastclicktime;
	int                 m_lastclickx;
	int                 m_lastclicky;

	// drawing data
	osd_renderer *      m_renderer;

private:
	running_machine &   m_machine;
};

struct osd_draw_callbacks
{
	osd_renderer *(*create)(win_window_info *window);
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

// creation/deletion of windows
void winwindow_video_window_create(running_machine &machine, int index, win_monitor_info *monitor, const win_window_config *config);

BOOL winwindow_has_focus(void);
void winwindow_update_cursor_state(running_machine &machine);
win_monitor_info *winwindow_video_window_monitor(win_window_info *window, const RECT *proposed);

LRESULT CALLBACK winwindow_video_window_proc(HWND wnd, UINT message, WPARAM wparam, LPARAM lparam);
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
//  win_has_menu
//============================================================

INLINE BOOL win_has_menu(win_window_info *window)
{
	return GetMenu(window->m_hwnd) ? TRUE : FALSE;
}


//============================================================
//  rect_width / rect_height
//============================================================

INLINE int rect_width(const RECT *rect)
{
	return rect->right - rect->left;
}


INLINE int rect_height(const RECT *rect)
{
	return rect->bottom - rect->top;
}

#endif
