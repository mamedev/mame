// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont, Couriersud
//============================================================
//
//  osdwindow.h - SDL window handling
//
//============================================================

#ifndef MAME_OSD_MODULES_OSDWINDOW_H
#define MAME_OSD_MODULES_OSDWINDOW_H

#pragma once

#include "emucore.h"
#include "osdhelper.h"
#include "../frontend/mame/ui/menuitem.h"

#include <cassert>
#include <chrono>
#include <memory>
#include <string>
#include <vector>

// standard windows headers
#ifdef OSD_WINDOWS
#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#endif

//============================================================
//  TYPE DEFINITIONS
//============================================================

class osd_monitor_info;
class render_module;
class render_primitive_list;

class osd_renderer;


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
	virtual ~osd_window();

	render_target *target() const { return m_target; }
	int fullscreen() const { return m_fullscreen; }
	running_machine &machine() const { return m_machine; }
	const std::string &title() const { return m_title; }

	bool has_renderer() const { return m_renderer != nullptr; }
	osd_renderer &renderer() const { return *m_renderer; }
	void renderer_reset() { m_renderer.reset(); } // public because OSD object calls it directly during teardown

	int index() const { return m_index; }
	int prescale() const { return m_prescale; }

	float pixel_aspect() const;

	bool swap_xy() const;

	bool keepaspect() const;

	virtual osd_dim get_size() = 0;

	osd_monitor_info *monitor() const { return m_monitor.get(); }
	std::shared_ptr<osd_monitor_info> monitor_from_rect(const osd_rect *proposed) const;

	void create_target();
	void destroy();

	// Clips the pointer to the bounds of this window
	virtual void capture_pointer() = 0;

	// Releases the pointer from a previously captured state
	virtual void release_pointer() = 0;

	virtual void show_pointer() = 0;
	virtual void hide_pointer() = 0;

	virtual void update() = 0;
	virtual void complete_destroy() = 0;

protected:
	static inline constexpr int CLICK_DISTANCE = 16; // in pointer units squared
	static inline constexpr int TAP_DISTANCE = 49; // taps are less repeatable than clicks

	struct prev_touch
	{
		prev_touch() = default;
		prev_touch(prev_touch const &) = default;
		prev_touch(prev_touch &&) = default;
		prev_touch &operator=(prev_touch const &) = default;
		prev_touch &operator=(prev_touch &&) = default;

		std::chrono::steady_clock::time_point when = std::chrono::steady_clock::time_point::min();
		int x = 0, y = 0, cnt = 0;
	};

	struct pointer_dev_info
	{
		pointer_dev_info() = default;
		pointer_dev_info(pointer_dev_info const &) = default;
		pointer_dev_info(pointer_dev_info &&) = default;
		pointer_dev_info &operator=(pointer_dev_info const &) = default;
		pointer_dev_info &operator=(pointer_dev_info &&) = default;

		unsigned clear_expired_touches(std::chrono::steady_clock::time_point const &now, std::chrono::steady_clock::duration const &time);
		unsigned consume_touch(unsigned i);

		prev_touch touches[8];
	};

	struct pointer_info
	{
		pointer_info(pointer_info const &) = default;
		pointer_info(pointer_info &&) = default;
		pointer_info &operator=(pointer_info const &) = default;
		pointer_info &operator=(pointer_info &&) = default;

		pointer_info(unsigned i, unsigned d);

		void primary_down(
				int cx,
				int cy,
				std::chrono::steady_clock::duration const &time,
				int tolerance,
				bool checkprev,
				std::vector<pointer_dev_info> &devices);
		void check_primary_hold_drag(
				int cx,
				int cy,
				std::chrono::steady_clock::duration const &time,
				int tolerance);

		std::chrono::steady_clock::time_point pressed;
		unsigned index, device;
		int x, y;
		unsigned buttons;
		int pressedx, pressedy;
		int clickcnt;
	};

	osd_window(
			running_machine &machine,
			render_module &renderprovider,
			int index,
			const std::shared_ptr<osd_monitor_info> &monitor,
			const osd_window_config &config);

	bool renderer_interactive() const;
	bool renderer_sdl_needs_opengl() const;
	void renderer_create();

private:
	void set_starting_view(int index, const char *defview, const char *view);

private:
	render_target           *m_target;
public:
	render_primitive_list   *m_primlist;
	osd_window_config       m_win_config;
private:
	int                     m_index;
protected:
	bool                    m_fullscreen;
	int                     m_prescale;

private:
	running_machine                     &m_machine;
	render_module                       &m_renderprovider;
	std::shared_ptr<osd_monitor_info>   m_monitor;
	std::unique_ptr<osd_renderer>       m_renderer;
	const std::string                   m_title;
};


inline unsigned osd_window::pointer_dev_info::clear_expired_touches(
		std::chrono::steady_clock::time_point const &now,
		std::chrono::steady_clock::duration const &time)
{
	// find first non-expired touch (if any)
	unsigned end(0);
	while ((std::size(touches) > end) && touches[end].cnt && ((touches[end].when + time) < now))
		touches[end++].cnt = 0;

	// shift non-expired touches back if necessary
	if (end)
	{
		unsigned pos(0);
		while ((std::size(touches) > end) && touches[end].cnt)
		{
			touches[pos++] = std::move(touches[end]);
			touches[end++].cnt = 0;
		}
		return pos;
	}
	else
	{
		while ((std::size(touches) > end) && touches[end].cnt)
			++end;
		return end;
	}
}

inline unsigned osd_window::pointer_dev_info::consume_touch(unsigned i)
{
	assert(std::size(touches) > i);
	touches[i].cnt = 0;
	unsigned pos(i + 1);
	while ((std::size(touches) > pos) && touches[pos].cnt)
	{
		touches[i++] = std::move(touches[pos]);
		touches[pos++].cnt = 0;
	}
	return i;
}

inline osd_window::pointer_info::pointer_info(unsigned i, unsigned d) :
	pressed(std::chrono::steady_clock::time_point::min()),
	index(i),
	device(d),
	x(-1),
	y(-1),
	buttons(0),
	pressedx(0),
	pressedy(0),
	clickcnt(0)
{
}

inline void osd_window::pointer_info::primary_down(
		int cx,
		int cy,
		std::chrono::steady_clock::duration const &time,
		int tolerance,
		bool checkprev,
		std::vector<pointer_dev_info> &devices)
{
	auto const now(std::chrono::steady_clock::now());
	auto const exp(time + pressed);
	if (0 > clickcnt)
	{
		// previous click turned into a hold/drag
		clickcnt = 1;
	}
	else if (clickcnt)
	{
		// potential multi-click action
		int const dx(cx - pressedx);
		int const dy(cy - pressedy);
		int const distance((dx * dx) + (dy * dy));
		if ((exp < now) || (tolerance < distance))
			clickcnt = 1;
		else
			++clickcnt;
	}
	else
	{
		// first click for this pointer, but may need to check previous touches
		clickcnt = 1;
		if (checkprev)
		{
			if (devices.size() > device)
			{
				auto &devinfo(devices[device]);
				unsigned const end(devinfo.clear_expired_touches(now, time));
				for (int i = 0; end > i; ++i)
				{
					int const dx(cx - devinfo.touches[i].x);
					int const dy(cy - devinfo.touches[i].y);
					int const distance((dx * dx) + (dy * dy));
					if (tolerance >= distance)
					{
						// close match - consume it
						clickcnt = devinfo.touches[i].cnt + 1;
						devinfo.consume_touch(i);
						break;
					}
				}
			}
		}
	}

	// record the time/location where the pointer was pressed
	pressed = now;
	pressedx = cx;
	pressedy = cy;
}

inline void osd_window::pointer_info::check_primary_hold_drag(
		int cx,
		int cy,
		std::chrono::steady_clock::duration const &time,
		int tolerance)
{
	// check for conversion to a (multi-)click-and-hold/drag
	if (0 < clickcnt)
	{
		auto const now(std::chrono::steady_clock::now());
		auto const exp(time + pressed);
		int const dx(cx - pressedx);
		int const dy(cy - pressedy);
		int const distance((dx * dx) + (dy * dy));
		if ((exp < now) || (tolerance < distance))
			clickcnt = -clickcnt;
	}
}


template <class TWindowHandle>
class osd_window_t : public osd_window
{
public:
	TWindowHandle platform_window() const { return m_platform_window; }

protected:
	using osd_window::osd_window;

	void set_platform_window(TWindowHandle window)
	{
		assert(window == nullptr || m_platform_window == nullptr);
		m_platform_window = window;
	}

private:
	TWindowHandle m_platform_window = nullptr;
};


class osd_renderer
{
public:

	/* Generic flags */
	static const int FLAG_NONE                  = 0x0000;
	static const int FLAG_HAS_VECTOR_SCREEN     = 0x0001;

	osd_renderer(osd_window &window) : m_sliders_dirty(false), m_window(window), m_flags(0) { }

	virtual ~osd_renderer() { }

	osd_window &window() const { return m_window; }

	bool has_flags(const int flag) const { return ((m_flags & flag)) == flag; }
	void set_flags(int aflag) { m_flags |= aflag; }
	void clear_flags(int aflag) { m_flags &= ~aflag; }

	void notify_changed() { set_flags(FI_CHANGED); }

	/* Interface to be implemented by render code */

	virtual int create() = 0;
	virtual render_primitive_list *get_primitives() = 0;

	virtual void add_audio_to_recording(const int16_t *buffer, int samples_this_frame) { }
	virtual std::vector<ui::menu_item> get_slider_list() { return m_sliders; }
	virtual int draw(const int update) = 0;
	virtual int xy_to_render_target(const int x, const int y, int *xt, int *yt) { return 0; }
	virtual void save() { }
	virtual void record() { }
	virtual void toggle_fsfx() { }
	virtual bool sliders_dirty() { return m_sliders_dirty; }

protected:
	virtual void build_slider_list() { }

	/* Internal flags */
	static const int FI_CHANGED                 = 0x010000;
	bool                        m_sliders_dirty;
	std::vector<ui::menu_item>   m_sliders;

private:
	osd_window &m_window;
	int         m_flags;
};



//============================================================
//  CONSTANTS
//============================================================

#define MAX_VIDEO_WINDOWS           (4)


//============================================================
//  TYPE DEFINITIONS
//============================================================

struct osd_video_config
{
	// global configuration
	int                 windowed;                   // start windowed?
	int                 prescale;                   // prescale factor
	int                 numscreens;                 // number of screens

	// hardware options
	int                 waitvsync;                  // spin until vsync
	int                 syncrefresh;                // sync only to refresh rate
	int                 switchres;                  // switch resolutions

	// d3d, accel, opengl
	int                 filter;                     // enable filtering

	// d3d
	int                 triplebuf;                  // triple buffer

	//============================================================
	// SDL - options
	//============================================================
	int                 centerh;
	int                 centerv;

	// vector options
	float               beamwidth;      // beam width

	// perftest
	int                 perftest;       // print out real video fps

	// X11 options
	int                 restrictonemonitor; // in fullscreen, confine to Xinerama monitor 0
};

//============================================================
//  GLOBAL VARIABLES
//============================================================

extern osd_video_config video_config;

#endif // MAME_OSD_MODULES_OSDWINDOW_H
