//============================================================
//
//  osdwindow.h - SDL window handling
//
//  Copyright (c) 1996-2015, Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
//
//============================================================

// There is no way this code is correct or sensible - it just gets it to build on OSX
// Wait for official fix from couriersud

#ifndef __OSDWINDOW__
#define __OSDWINDOW__

#include "render.h"


class osd_window_config
{
public:
	int	width;
	int	height;
	int	depth;
	int	refresh;
};


class osd_window
{
public:
	virtual void get_size(int &w, int &h) = 0;

	virtual running_machine &machine() const = 0;

	virtual render_target *target() = 0;
#if (SDLMAME_SDL2)
	virtual SDL_Window *sdl_window() = 0;
#else
	virtual SDL_Surface *sdl_surface() = 0;
#endif

	virtual void blit_surface_size(int &blitwidth, int &blitheight) = 0;
	virtual int prescale() const = 0;
	virtual float aspect() const { return 0; }

	render_primitive_list	*m_primlist;

protected:
	osd_window_config		m_win_config;
	int						m_prescale;
};


class osd_renderer
{
public:
	enum
	{
		FLAG_NEEDS_DOUBLEBUF	= 1,
		FLAG_NEEDS_ASYNCBLIT	= 2,
		FLAG_NEEDS_OPENGL		= 4,
		FLAG_HAS_VECTOR_SCREEN	= 8
	};

	void notify_changed();

	virtual int create() = 0;
	virtual int draw(const int update) = 0;
	virtual int xy_to_render_target(const int x, const int y, int *xt, int *yt) = 0;
	virtual void destroy() = 0;
	virtual render_primitive_list *get_primitives() = 0;

	bool has_flags(int f) const { return (m_flags & f) == f; }
	void set_flags(int f) { m_flags |= f; }
	void clear_flags(int f) { m_flags &= ~f; }

protected:
	enum
	{
		FI_CHANGED				= 16
	};

	osd_renderer(osd_window *w, int extra_flags) :
		m_window(*w),
		m_flags(extra_flags)
	{
	}

	osd_window &window() const { return m_window; }

private:
	osd_window	&m_window;
	int			m_flags;
};

inline void osd_renderer::notify_changed() { set_flags(FI_CHANGED); }

#endif