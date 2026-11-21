// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont
//============================================================
//
//  window.h - SDL window handling
//
//  Mac OSD by R. Belmont
//
//============================================================

#ifndef MAME_OSD_MAC_WINDOW_H
#define MAME_OSD_MAC_WINDOW_H

#pragma once

#include "osdmac.h"
#include "osdsync.h"

#include "modules/osdwindow.h"

#include <cstdint>
#include <memory>
#include <list>
#include <vector>

//============================================================
//  TYPE DEFINITIONS
//============================================================

class render_target;

typedef uintptr_t HashT;

class mac_window_info : public osd_window_t<void *>
{
public:
	mac_window_info(running_machine &a_machine, render_module &renderprovider, int index, std::shared_ptr<osd_monitor_info> a_monitor,
			const osd_window_config *config);

	~mac_window_info();

	int window_init();

	void update() override;
	void toggle_full_screen();
	void modify_prescale(int dir);
	void resize(int32_t width, int32_t height);
	void complete_destroy() override;

	void capture_pointer() override;
	void release_pointer() override;
	void show_pointer() override;
	void hide_pointer() override;

	void notify_changed();

	osd_dim get_size() override;
	osd_dim get_size_pixels() override;

	int xy_to_render_target(int x, int y, int *xt, int *yt);

	// direct touch input from the window's gesture recognizer
	void touch_down(uint32_t touch, int x, int y);
	void touch_moved(uint32_t touch, int x, int y);
	void touch_up(uint32_t touch, int x, int y);
	void touch_cancelled(uint32_t touch);

	// -video none: keep the native window hidden, or omit it if no display is available
	bool headless() const { return m_headless; }

	// the window that owns a native NSWindow, or nullptr
	static mac_window_info *from_nswindow(void *nswindow);

private:
	struct mac_pointer_info : public pointer_info
	{
		static constexpr bool compare(const mac_pointer_info &info, uint32_t touch) { return info.touch < touch; }

		mac_pointer_info(uint32_t t, uint32_t i, uint32_t d) : pointer_info(i, d), touch(t) { }

		uint32_t touch;
	};

	// window handle and info
	int                 m_startmaximized;
	bool                m_headless;

	// dimensions
	osd_dim             m_minimum_dim;
	osd_dim             m_windowed_dim;

	// last aspect-constraint state pushed to the window
	bool                m_last_keepaspect;
	int                 m_last_scale_mode;

	// rendering info
	osd_event           m_rendered_event;

	//int                 m_extra_flags;

	// returns 0 on success, else 1
	int complete_create();

private:
	int wnd_extra_width();
	int wnd_extra_height();
	void update_aspect_ratio();
	osd_rect constrain_to_aspect_ratio(const osd_rect &rect, int adjustment);
	osd_dim get_min_bounds(int constrain);
	osd_dim get_max_bounds(int constrain);
	void update_cursor_state();
	osd_dim pick_best_mode();
	void set_fullscreen(int afullscreen) { m_fullscreen = afullscreen; }

	// monitor info
	bool                               m_mouse_captured;
	bool                               m_mouse_hidden;

	// active touches; pointer 0 belongs to the mouse
	uint64_t                           m_pointer_mask;
	uint32_t                           m_next_pointer;
	std::vector<pointer_dev_info>      m_ptrdev_info;
	std::vector<mac_pointer_info>      m_active_pointers;

	void measure_fps(int update);
	int tap_distance();
	void touch_lightgun(uint32_t touch, int x, int y, bool contact);
	std::vector<mac_pointer_info>::iterator find_touch(uint32_t touch);
	void remove_touch(std::vector<mac_pointer_info>::iterator info);
	void abort_touches();
};

#endif // MAME_OSD_MAC_WINDOW_H
