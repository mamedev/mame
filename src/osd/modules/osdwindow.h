// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont, Couriersud
//============================================================
//
//  osdwindow.h - SDL window handling
//
//============================================================

#ifndef __OSDWINDOW__
#define __OSDWINDOW__

#include "video.h"
#include "render.h"

//============================================================
//  TYPE DEFINITIONS
//============================================================

class osd_window_config
{
public:
	osd_window_config() : aspect(0.0f), width(0), height(0), depth(0), refresh(0) {}

	float               aspect;                     // decoded aspect ratio FIXME: not used on windows
	int                 width;                      // decoded width
	int                 height;                     // decoded height
	int                 depth;                      // decoded depth - only SDL
	int                 refresh;                    // decoded refresh
};

class osd_window
{
public:
	osd_window()
	:
#ifdef OSD_SDL
#else
		m_hwnd(0), m_dc(0), m_focus_hwnd(0), m_resize_state(0),
#endif
		m_primlist(NULL),
		m_prescale(1)
		{}
	virtual ~osd_window() { }

	virtual render_target *target() = 0;
	virtual int fullscreen() const = 0;
	virtual running_machine &machine() const = 0;

	int prescale() const { return m_prescale; };

	float aspect() const { return monitor()->aspect(); }

	virtual osd_dim get_size() = 0;

#ifdef OSD_SDL
	virtual osd_dim blit_surface_size() = 0;
	virtual osd_monitor_info *monitor() const = 0;
#if (SDLMAME_SDL2)
	virtual SDL_Window *sdl_window() = 0;
#else
	virtual SDL_Surface *sdl_surface() = 0;
#endif
#else
	virtual osd_monitor_info *monitor() const = 0;
	virtual bool win_has_menu() = 0;
	// FIXME: cann we replace winwindow_video_window_monitor(NULL) with monitor() ?
	virtual osd_monitor_info *winwindow_video_window_monitor(const osd_rect *proposed) = 0;

	// window handle and info
	HWND                    m_hwnd;
	HDC                     m_dc;       // only used by GDI renderer!
	// FIXME: this is the same as win_window_list->m_hwnd, i.e. first window.
	// During modularization, this should be passed in differently
	HWND                    m_focus_hwnd;

	int                     m_resize_state;
#endif

	render_primitive_list   *m_primlist;
	osd_window_config       m_win_config;
protected:
	int                     m_prescale;
};

class osd_renderer
{
public:

	/* Generic flags */
	static const int FLAG_NONE                  = 0x0000;
	static const int FLAG_NEEDS_OPENGL          = 0x0001;
	static const int FLAG_HAS_VECTOR_SCREEN     = 0x0002;

	/* SDL 1.2 flags */
	static const int FLAG_NEEDS_DOUBLEBUF       = 0x0100;
	static const int FLAG_NEEDS_ASYNCBLIT       = 0x0200;

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

	virtual int draw(const int update) = 0;
#ifdef OSD_SDL
	virtual int xy_to_render_target(const int x, const int y, int *xt, int *yt) = 0;
#else
	virtual void save() = 0;
	virtual void record() = 0;
	virtual void toggle_fsfx() = 0;
#endif

	virtual void destroy() = 0;

protected:
	/* Internal flags */
	static const int FI_CHANGED                 = 0x010000;

private:

	osd_window      *m_window;
	int         m_flags;
};


#endif /* __OSDWINDOW__ */
