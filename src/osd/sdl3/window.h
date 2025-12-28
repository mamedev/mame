// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont
//============================================================
//
//  window.h - SDL window handling
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================

#ifndef MAME_OSD_SDL_WINDOW_H
#define MAME_OSD_SDL_WINDOW_H

#include "modules/osdwindow.h"
#include "osdsync.h"

#include <SDL3/SDL.h>

#include <chrono>
#include <cstdint>
#include <memory>
#include <vector>


//============================================================
//  TYPE DEFINITIONS
//============================================================

class render_target;

typedef uintptr_t HashT;

#define OSDWORK_CALLBACK(name)  void *name(void *param, int threadid)

class sdl_window_info : public osd_window_t<SDL_Window*>
{
public:
	sdl_window_info(
			running_machine &a_machine,
			render_module &renderprovider,
			int index,
			const std::shared_ptr<osd_monitor_info> &a_monitor,
			const osd_window_config *config);

	~sdl_window_info();

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

	int xy_to_render_target(int x, int y, int *xt, int *yt);

	void mouse_entered(unsigned device);
	void mouse_left(unsigned device);
	void mouse_down(unsigned device, int x, int y, unsigned button);
	void mouse_up(unsigned device, int x, int y, unsigned button);
	void mouse_moved(unsigned device, int x, int y);
	void mouse_wheel(unsigned device, int y);
	void finger_down(SDL_FingerID finger, unsigned device, int x, int y);
	void finger_up(SDL_FingerID finger, unsigned device, int x, int y);
	void finger_moved(SDL_FingerID finger, unsigned device, int x, int y);

private:
	struct sdl_pointer_info : public pointer_info
	{
		static constexpr bool compare(sdl_pointer_info const &info, SDL_FingerID finger) { return info.finger < finger; }

		sdl_pointer_info(sdl_pointer_info const &) = default;
		sdl_pointer_info(sdl_pointer_info &&) = default;
		sdl_pointer_info &operator=(sdl_pointer_info const &) = default;
		sdl_pointer_info &operator=(sdl_pointer_info &&) = default;

		sdl_pointer_info(SDL_FingerID, unsigned i, unsigned d);

		SDL_FingerID finger;
	};

	// returns 0 on success, else 1
	int complete_create();

	int wnd_extra_width();
	int wnd_extra_height();
	osd_rect constrain_to_aspect_ratio(const osd_rect &rect, int adjustment);
	osd_dim get_min_bounds(int constrain);
	osd_dim get_max_bounds(int constrain);
	void update_cursor_state();
	osd_dim pick_best_mode();
	void set_fullscreen(int afullscreen) { m_fullscreen = afullscreen; }

	void measure_fps(int update);

	std::vector<sdl_pointer_info>::iterator map_pointer(SDL_FingerID finger, unsigned device);

	// window handle and info
	int                 m_startmaximized;

	// dimensions
	osd_dim             m_minimum_dim;
	osd_dim             m_windowed_dim;

	// rendering info
	osd_event           m_rendered_event;

	// Original display_mode
	SDL_DisplayMode     m_original_mode;

	int                 m_extra_flags;

	// monitor info
	bool                m_mouse_captured;
	bool                m_mouse_hidden;

	// info on currently active pointers - 64 pointers ought to be enough for anyone
	uint64_t m_pointer_mask;
	unsigned m_next_pointer;
	bool m_mouse_inside;
	std::vector<pointer_dev_info> m_ptrdev_info;
	std::vector<sdl_pointer_info> m_active_pointers;
};

#endif // MAME_OSD_SDL_WINDOW_H
