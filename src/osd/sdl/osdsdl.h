// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont
#ifndef MAME_OSD_SDL_OSDSDL_H
#define MAME_OSD_SDL_OSDSDL_H

#pragma once

#include "sdlopts.h"

#include "modules/lib/osdobj_common.h"
#include "modules/osdmodule.h"

#include <SDL2/SDL.h>

#include <cassert>
#include <chrono>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <utility>
#include <string>
#include <vector>


//============================================================
//  Defines
//============================================================

#define SDLMAME_LED(x)                  "led" #x

// read by sdlmame

#define SDLENV_DESKTOPDIM               "SDLMAME_DESKTOPDIM"
#define SDLENV_VMWARE                   "SDLMAME_VMWARE"

// set by sdlmame

#define SDLENV_VISUALID                 "SDL_VIDEO_X11_VISUALID"
#define SDLENV_VIDEODRIVER              "SDL_VIDEODRIVER"
#define SDLENV_AUDIODRIVER              "SDL_AUDIODRIVER"
#define SDLENV_RENDERDRIVER             "SDL_VIDEO_RENDERER"


//============================================================
//  TYPE DEFINITIONS
//============================================================

template <typename EventRecord, typename EventType>
class event_subscription_manager
{
public: // need extra public section for forward declaration
	class subscriber;

private:
	class impl
	{
	public:
		std::mutex m_mutex;
		std::unordered_multimap<EventType, subscriber *> m_subs;
	};

	std::shared_ptr<impl> m_impl;

protected:
	event_subscription_manager() : m_impl(new impl)
	{
	}

	~event_subscription_manager() = default;

	std::mutex &subscription_mutex()
	{
		return m_impl->m_mutex;
	}

	void dispatch_event(EventType const &type, EventRecord const &event)
	{
		auto const matches = m_impl->m_subs.equal_range(type);
		for (auto it = matches.first; matches.second != it; ++it)
			it->second->handle_event(event);
	}

public:
	class subscriber
	{
	public:
		virtual void handle_event(EventRecord const &event) = 0;

	protected:
		subscriber() = default;

		virtual ~subscriber()
		{
			unsubscribe();
		}

		template <typename T>
		void subscribe(event_subscription_manager &host, T &&types)
		{
			assert(!m_host.lock());
			assert(host.m_impl);

			m_host = host.m_impl;

			std::lock_guard<std::mutex> lock(host.m_impl->m_mutex);
			for (auto const &t : types)
				host.m_impl->m_subs.emplace(t, this);
		}

		void unsubscribe()
		{
			auto const host(m_host.lock());
			m_host.reset();
			if (host)
			{
				std::lock_guard<std::mutex> lock(host->m_mutex);
				auto it = host->m_subs.begin();
				while (host->m_subs.end() != it)
				{
					if (it->second == this)
						it = host->m_subs.erase(it);
					else
						++it;
				}
			}
		}

	private:
		std::weak_ptr<impl> m_host;
	};
};


using sdl_event_manager = event_subscription_manager<SDL_Event, uint32_t>;


class sdl_window_info;

class sdl_osd_interface : public osd_common_t, public sdl_event_manager
{
public:
	// construction/destruction
	sdl_osd_interface(sdl_options &options);
	virtual ~sdl_osd_interface();

	// general overridables
	virtual void init(running_machine &machine) override;
	virtual void update(bool skip_redraw) override;
	virtual void input_update(bool relative_reset) override;
	virtual void check_osd_inputs() override;

	// input overridables
	virtual void customize_input_type_list(std::vector<input_type_entry> &typelist) override;

	virtual bool video_init() override;
	virtual bool window_init() override;

	virtual void video_exit() override;
	virtual void window_exit() override;

	// SDL-specific
	virtual bool has_focus() const override { return bool(m_focus_window); }
	void release_keys();
	bool should_hide_mouse();
	void process_events_buf();

	virtual sdl_options &options() override { return m_options; }

	virtual void process_events() override;

private:
	enum
	{
		MODIFIER_KEY_LCTRL = 0x01,
		MODIFIER_KEY_RCTRL = 0x02,
		MODIFIER_KEY_LSHIFT = 0x04,
		MODIFIER_KEY_RSHIFT = 0x08,

		MODIFIER_KEY_CTRL = MODIFIER_KEY_LCTRL | MODIFIER_KEY_RCTRL,
		MODIFIER_KEY_SHIFT = MODIFIER_KEY_LSHIFT | MODIFIER_KEY_RSHIFT
	};

	virtual void osd_exit() override;

	void extract_video_config();
	void output_oslog(const char *buffer);

	void process_window_event(SDL_Event const &event);
	void process_textinput_event(SDL_Event const &event);

	bool mouse_over_window() const { return m_mouse_over_window > 0; }
	template <typename T> sdl_window_info *focus_window(T const &event) const;

	unsigned map_pointer_device(SDL_TouchID device);

	sdl_options &m_options;
	sdl_window_info *m_focus_window;
	int m_mouse_over_window;
	uint8_t m_modifier_keys;

	std::chrono::steady_clock::time_point m_last_click_time;
	int m_last_click_x;
	int m_last_click_y;

	bool m_enable_touch;
	unsigned m_next_ptrdev;
	std::vector<std::pair<SDL_TouchID, unsigned> > m_ptrdev_map;
};

//============================================================
//  sdlwork.c
//============================================================

extern int osd_num_processors;

#endif // MAME_OSD_SDL_OSDSDL_H
