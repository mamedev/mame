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
		m_start_viewscreen(0),
#else
		m_hwnd(0), m_focus_hwnd(0), m_monitor(NULL), m_resize_state(0),
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
	virtual bool win_has_menu() = 0;
	virtual win_monitor_info *winwindow_video_window_monitor(const RECT *proposed) = 0;

	// window handle and info
	HWND					m_hwnd;
	// FIXME: this is the same as win_window_list->m_hwnd, i.e. first window.
	// During modularization, this should be passed in differently
	HWND         	 		m_focus_hwnd;

	// monitor info
	win_monitor_info *  	m_monitor;
	int                 	m_resize_state;
	int                 	m_maxwidth, m_maxheight;
	int                 	m_refresh;
#endif

	int						m_prescale;
	render_primitive_list *	m_primlist;
};

class osd_renderer
{
public:

	/* Generic flags */
	static const int FLAG_NONE 					= 0x0000;
	static const int FLAG_NEEDS_OPENGL 			= 0x0001;
	static const int FLAG_HAS_VECTOR_SCREEN		= 0x0002;

	/* SDL 1.2 flags */
	static const int FLAG_NEEDS_DOUBLEBUF 		= 0x0100;
	static const int FLAG_NEEDS_ASYNCBLIT 		= 0x0200;

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

class win_window_info  : public osd_window
{
public:
	win_window_info(running_machine &machine);
	virtual ~win_window_info();

	running_machine &machine() const { return m_machine; }

	render_target *target() { return m_target; }
	int fullscreen() const { return m_fullscreen; }

	void update();

	win_monitor_info *winwindow_video_window_monitor(const RECT *proposed);

	bool win_has_menu()
	{
		return GetMenu(m_hwnd) ? true : false;
	}

	win_window_info *   m_next;
	volatile int        m_init_state;

	// window handle and info
	char                m_title[256];
	RECT                m_non_fullscreen_bounds;
	int                 m_startmaximized;
	int                 m_isminimized;
	int                 m_ismaximized;

	// monitor info
	int                 m_fullscreen;
	int                 m_fullscreen_safe;
	float               m_aspect;

	// rendering info
	osd_lock *          m_render_lock;
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

// creation/deletion of windows
void winwindow_video_window_create(running_machine &machine, int index, win_monitor_info *monitor, const win_window_config *config);

BOOL winwindow_has_focus(void);
void winwindow_update_cursor_state(running_machine &machine);

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
