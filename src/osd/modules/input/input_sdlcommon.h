// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont, Brad Hughes
//============================================================
//
//  input_sdlcommon.h - SDL Common code shared by SDL modules
//
//    Note: this code is also used by the X11 input modules
//
//============================================================

#ifndef MAME_OSD_INPUT_INPUT_SDLCOMMON_H
#define MAME_OSD_INPUT_INPUT_SDLCOMMON_H

#pragma once

#include <algorithm>
#include <unordered_map>

#define MAX_DEVMAP_ENTRIES  16
#define SDL_MODULE_EVENT_BUFFER_SIZE 5


struct device_map_t
{
	struct {
		std::string    name;
		int            physical;
	} map[MAX_DEVMAP_ENTRIES];
	int     logical[MAX_DEVMAP_ENTRIES];
	int     initialized;
};

//============================================================
//  event_manager_t
//============================================================

class sdl_event_subscriber
{
public:
	virtual ~sdl_event_subscriber() {}
	virtual void handle_event(SDL_Event &sdlevent) = 0;
};

template <class TSubscriber>
class event_manager_t
{
protected:
	std::mutex                                   m_lock;
	std::unordered_multimap<int, TSubscriber*>   m_subscription_index;
	event_manager_t()
	{
	}

public:
	virtual ~event_manager_t()
	{
	}

	template <size_t N>
	void subscribe(int const (&event_types)[N], TSubscriber *subscriber)
	{
		std::lock_guard<std::mutex> scope_lock(m_lock);

		// Add the subscription
		for (int i : event_types)
			m_subscription_index.emplace(i, subscriber);
	}

	void unsubscribe(TSubscriber *subscriber)
	{
		std::lock_guard<std::mutex> scope_lock(m_lock);

		// Remove the events that match the subscriber
		for (auto it = begin(m_subscription_index); it != end(m_subscription_index);)
		{
			if (it->second == subscriber)
			{
				it = m_subscription_index.erase(it);
			}
			else
			{
				++it;
			}
		}
	}

	virtual void process_events(running_machine &machine) = 0;
};

class sdl_window_info;

class sdl_event_manager : public event_manager_t<sdl_event_subscriber>
{
private:
	bool                                 m_mouse_over_window;
	std::shared_ptr<sdl_window_info>     m_focus_window;

	sdl_event_manager()
		: m_mouse_over_window(true),
		  m_focus_window(nullptr)
	{
	}

public:
	bool mouse_over_window() const { return m_mouse_over_window; }
	bool has_focus() const { return m_focus_window != nullptr; }
	std::shared_ptr<sdl_window_info> focus_window() const { return m_focus_window; }

	static sdl_event_manager& instance()
	{
		static sdl_event_manager s_instance;
		return s_instance;
	}

	void process_events(running_machine &machine) override;

private:
	void process_window_event(running_machine &machine, SDL_Event &sdlevent);
};

//============================================================
//  INLINE FUNCTIONS
//============================================================

static inline std::string remove_spaces(const char *s)
{
	// Remove the spaces
	auto output = std::string(s);
	output.erase(std::remove_if(output.begin(), output.end(), isspace), output.end());

	return output;
}

//============================================================
//  devmap_init - initializes a device_map based on
//   an input option prefix and max number of devices
//============================================================

static inline void devmap_init(running_machine &machine, device_map_t *devmap, const char *opt, int max_devices, const char *label)
{
	int dev;
	char defname[20];

	// The max devices the user specified, better not be bigger than the max the arrays can old
	assert(max_devices <= MAX_DEVMAP_ENTRIES);

	// Initialize the map to default uninitialized values
	for (dev = 0; dev < MAX_DEVMAP_ENTRIES; dev++)
	{
		devmap->map[dev].name.clear();
		devmap->map[dev].physical = -1;
		devmap->logical[dev] = -1;
	}
	devmap->initialized = 0;

	// populate the device map up to the max number of devices
	for (dev = 0; dev < max_devices; dev++)
	{
		const char *dev_name;

		// derive the parameter name from the option name and index. For instance: lightgun_index1 to lightgun_index8
		sprintf(defname, "%s%d", opt, dev + 1);

		// Get the user-specified name that matches the parameter
		dev_name = machine.options().value(defname);

		// If they've specified a name and it's not "auto", treat it as a custom mapping
		if (dev_name && *dev_name && strcmp(dev_name, OSDOPTVAL_AUTO))
		{
			// remove the spaces from the name store it in the index
			devmap->map[dev].name = remove_spaces(dev_name);
			osd_printf_verbose("%s: Logical id %d: %s\n", label, dev + 1, devmap->map[dev].name);
			devmap->initialized = 1;
		}
	}
}

#endif // MAME_OSD_INPUT_INPUT_SDLCOMMON_H
