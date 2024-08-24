// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont, Brad Hughes
//============================================================
//
//  input_common.h - Common code for all MAME input modules
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================
#ifndef MAME_OSD_INPUT_INPUT_COMMON_H
#define MAME_OSD_INPUT_INPUT_COMMON_H

#pragma once

#include "input_module.h"

#include "interface/inputdev.h"
#include "interface/inputman.h"
#include "modules/osdmodule.h"

#include "util/strformat.h"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>


//============================================================
//  PARAMETERS
//============================================================

enum
{
	POVDIR_LEFT = 0,
	POVDIR_RIGHT,
	POVDIR_UP,
	POVDIR_DOWN
};

#define MAX_KEYS            256
#define MAX_AXES            32
#define MAX_BUTTONS         32
#define MAX_HATS            8
#define MAX_POV             4


//============================================================
//  device_info
//============================================================

class device_info
{
private:
	const std::string       m_name;
	const std::string       m_id;
	input_module &          m_module;

public:
	// Constructor
	device_info(std::string &&name, std::string &&id, input_module &module) :
		m_name(std::move(name)),
		m_id(std::move(id)),
		m_module(module)
	{
	}

	// Destructor
	virtual ~device_info() = default;

	// Getters
	const std::string &name() const { return m_name; }
	const std::string &id() const { return m_id; }
	input_module &module() const { return m_module; }

	// Poll and reset methods
	virtual void poll(bool relative_reset) = 0;
	virtual void reset() = 0;
	virtual void configure(osd::input_device &device) = 0;
};


//============================================================
//  event_based_device
//============================================================

template <class TEvent>
class event_based_device : public device_info
{
private:
	static inline constexpr unsigned DEFAULT_EVENT_QUEUE_SIZE = 64;

	std::queue<TEvent> m_event_queue;

protected:
	std::mutex           m_device_lock;

	virtual void process_event(TEvent const &ev) = 0;

public:
	event_based_device(std::string &&name, std::string &&id, input_module &module) :
		device_info(std::move(name), std::move(id), module)
	{
	}

	void queue_events(TEvent const *events, int count)
	{
		std::lock_guard<std::mutex> scope_lock(m_device_lock);
		for (int i = 0; i < count; i++)
			m_event_queue.push(events[i]);

		// If we've gone over the size, remove old events from the queue
		while (m_event_queue.size() > DEFAULT_EVENT_QUEUE_SIZE)
			m_event_queue.pop();
	}

	virtual void poll(bool relative_reset) override
	{
		std::lock_guard<std::mutex> scope_lock(m_device_lock);

		// Process each event until the queue is empty
		while (!m_event_queue.empty())
		{
			process_event(m_event_queue.front());
			m_event_queue.pop();
		}
	}

	virtual void reset() override
	{
		std::lock_guard<std::mutex> scope_lock(m_device_lock);
		std::queue<TEvent>().swap(m_event_queue);
	}
};


//============================================================
//  input_device_list class
//============================================================

template <typename Info>
class input_device_list
{
private:
	std::vector<std::unique_ptr<Info> > m_list;

public:
	auto size() const { return m_list.size(); }
	auto empty() const { return m_list.empty(); }
	auto begin() const { return m_list.begin(); }
	auto end() const { return m_list.end(); }

	void poll_devices(bool relative_reset)
	{
		for (auto &device: m_list)
			device->poll(relative_reset);
	}

	void reset_devices()
	{
		for (auto &device: m_list)
			device->reset();
	}

	template <typename T>
	void for_each_device(T &&action)
	{
		for (auto &device: m_list)
			action(*device);
	}

	void free_all_devices()
	{
		while (!m_list.empty())
			m_list.pop_back();
	}

	template <typename Actual>
	Actual &add_device(std::unique_ptr<Actual> &&devinfo)
	{
		// append us to the list and return reference
		Actual &result = *devinfo;
		m_list.emplace_back(std::move(devinfo));
		return result;
	}
};


// keyboard translation table

struct key_trans_entry
{
	input_item_id   mame_key;

#if defined(OSD_SDL) || defined(SDLMAME_WIN32)
	int             sdl_scancode;
#endif
#if defined(OSD_WINDOWS) || defined(SDLMAME_WIN32)
	uint16_t        scan_code;
	unsigned char   virtual_key;
#endif

	char            ascii_key;
	char const *    mame_key_name;
	char const *    ui_name;
};

class keyboard_trans_table
{
private:
	// default constructor is private
	keyboard_trans_table();

	static key_trans_entry              s_default_table[];
	std::unique_ptr<key_trans_entry[]>  m_custom_table;

	key_trans_entry *                   m_table;
	uint32_t                            m_table_size;

public:
	// constructor
	keyboard_trans_table(std::unique_ptr<key_trans_entry[]> table, unsigned int size);

	// getters/setters
	uint32_t size() const { return m_table_size; }

	// public methods
	input_item_id lookup_mame_code(const char * scode) const;
	int lookup_mame_index(const char * scode) const;

#if defined(OSD_WINDOWS) || defined(SDLMAME_WIN32)
	input_item_id map_di_scancode_to_itemid(int di_scancode) const;
	int vkey_for_mame_code(input_code code) const;
#endif

	static keyboard_trans_table& instance()
	{
		static keyboard_trans_table s_instance;
		return s_instance;
	}

	key_trans_entry & operator [](int i) const { return m_table[i]; }
};


//============================================================
//  input_module_base - base class for input modules
//============================================================

class input_module_base : public osd_module, public input_module
{
private:
	// 10 milliseconds polling interval
	static constexpr inline unsigned MIN_POLLING_INTERVAL = 2;

	using clock_type = std::chrono::high_resolution_clock;
	using timepoint_type =  std::chrono::time_point<std::chrono::high_resolution_clock>;

	clock_type            m_clock;
	timepoint_type        m_last_poll;
	bool                  m_background_input;
	const osd_options *   m_options;
	osd::input_manager *  m_manager;

	virtual void poll(bool relative_reset) = 0;

protected:
	input_module_base(char const *type, char const *name);

	osd::input_manager &  manager() { assert(m_manager); return *m_manager; }
	const osd_options *   options() const { return m_options; }
	bool                  background_input() const { return m_background_input; }

	virtual void before_poll() { }

public:
	virtual int init(osd_interface &osd, const osd_options &options) override;

	virtual void input_init(running_machine &machine) override;
	virtual void poll_if_necessary(bool relative_reset) override;

	virtual void reset_devices() = 0; // SDL OSD uses this to forcibly release keys
};


//============================================================
//  input_module_impl - base class for input modules
//============================================================

template <typename Info, typename OsdImpl>
class input_module_impl : public input_module_base
{
public:
	virtual void exit() override
	{
		devicelist().free_all_devices();
	}

	virtual int init(osd_interface &osd, const osd_options &options) override
	{
		m_osd = dynamic_cast<OsdImpl *>(&osd);
		if (!m_osd)
			return -1;

		return input_module_base::init(osd, options);
	}

	virtual void reset_devices() override { devicelist().reset_devices(); }

protected:
	using input_module_base::input_module_base;

	input_device_list<Info> &devicelist() { return m_devicelist; }
	OsdImpl &osd() { assert(m_osd); return *m_osd; }

	virtual void before_poll() override
	{
		// periodically process events, in case they're not coming through
		// this also will make sure the mouse state is up-to-date
		osd().process_events();
	}

	virtual bool should_poll_devices()
	{
		return background_input() || osd().has_focus();
	}

	template <typename Actual, typename... Params>
	Actual &create_device(input_device_class deviceclass, std::string &&name, std::string &&id, Params &&... args)
	{
		// allocate the device object and add it to the input manager
		return add_device(
				deviceclass,
				std::make_unique<Actual>(std::move(name), std::move(id), *this, std::forward<Params>(args)...));
	}

	template <typename Actual>
	Actual &add_device(input_device_class deviceclass, std::unique_ptr<Actual> &&devinfo)
	{
		// add it to the input manager and append it to the list
		osd::input_device &osddev = manager().add_device(deviceclass, devinfo->name(), devinfo->id(), devinfo.get());
		devinfo->configure(osddev);
		return devicelist().add_device(std::move(devinfo));
	}

private:
	virtual void poll(bool relative_reset) override final
	{
		// poll all of the devices
		if (should_poll_devices())
			m_devicelist.poll_devices(relative_reset);
		else
			m_devicelist.reset_devices();
	}

	input_device_list<Info> m_devicelist;
	OsdImpl *m_osd = nullptr;
};


template <class TItem>
int generic_button_get_state(void *device_internal, void *item_internal)
{
	// return the current state
	return *reinterpret_cast<TItem const *>(item_internal) >> 7;
}


template <class TItem>
int generic_axis_get_state(void *device_internal, void *item_internal)
{
	return *reinterpret_cast<TItem const *>(item_internal);
}


//============================================================
//  default_button_name
//============================================================

inline std::string default_button_name(int which)
{
	return util::string_format("Button %d", which + 1);
}

//============================================================
//  default_pov_name
//============================================================

inline std::string default_pov_name(int which)
{
	return util::string_format("Hat %d", which + 1);
}

// default axis names
const char *const default_axis_name[] =
{
	"X", "Y", "Z", "RX",
	"RY", "RZ", "SL1", "SL2"
};

inline int32_t normalize_absolute_axis(double raw, double rawmin, double rawmax)
{
	// make sure we have valid arguments
	if (rawmin >= rawmax)
		return int32_t(raw);

	double const center = (rawmax + rawmin) / 2.0;
	if (raw >= center)
	{
		// above center
		double const result = (raw - center) * double(osd::input_device::ABSOLUTE_MAX) / (rawmax - center);
		return int32_t(std::min(result, double(osd::input_device::ABSOLUTE_MAX)));
	}
	else
	{
		// below center
		double result = -((center - raw) * double(-osd::input_device::ABSOLUTE_MIN) / (center - rawmin));
		return int32_t(std::max(result, double(osd::input_device::ABSOLUTE_MIN)));
	}
}

#endif // MAME_OSD_INPUT_INPUT_COMMON_H
