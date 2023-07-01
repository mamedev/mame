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

/****************************************************************************
*      DirectInput compatible keyboard scan codes
****************************************************************************/
#define KEY_UNKNOWN         0x00
#define KEY_ESCAPE          0x01
#define KEY_1               0x02
#define KEY_2               0x03
#define KEY_3               0x04
#define KEY_4               0x05
#define KEY_5               0x06
#define KEY_6               0x07
#define KEY_7               0x08
#define KEY_8               0x09
#define KEY_9               0x0A
#define KEY_0               0x0B
#define KEY_MINUS           0x0C    /* - on main keyboard */
#define KEY_EQUALS          0x0D
#define KEY_BACK            0x0E    /* backspace */
#define KEY_TAB             0x0F
#define KEY_Q               0x10
#define KEY_W               0x11
#define KEY_E               0x12
#define KEY_R               0x13
#define KEY_T               0x14
#define KEY_Y               0x15
#define KEY_U               0x16
#define KEY_I               0x17
#define KEY_O               0x18
#define KEY_P               0x19
#define KEY_LBRACKET        0x1A
#define KEY_RBRACKET        0x1B
#define KEY_RETURN          0x1C    /* Enter on main keyboard */
#define KEY_LCONTROL        0x1D
#define KEY_A               0x1E
#define KEY_S               0x1F
#define KEY_D               0x20
#define KEY_F               0x21
#define KEY_G               0x22
#define KEY_H               0x23
#define KEY_J               0x24
#define KEY_K               0x25
#define KEY_L               0x26
#define KEY_SEMICOLON       0x27
#define KEY_APOSTROPHE      0x28
#define KEY_GRAVE           0x29    /* accent grave */
#define KEY_LSHIFT          0x2A
#define KEY_BACKSLASH       0x2B
#define KEY_Z               0x2C
#define KEY_X               0x2D
#define KEY_C               0x2E
#define KEY_V               0x2F
#define KEY_B               0x30
#define KEY_N               0x31
#define KEY_M               0x32
#define KEY_COMMA           0x33
#define KEY_PERIOD          0x34    /* . on main keyboard */
#define KEY_SLASH           0x35    /* / on main keyboard */
#define KEY_RSHIFT          0x36
#define KEY_MULTIPLY        0x37    /* * on numeric keypad */
#define KEY_LMENU           0x38    /* left Alt */
#define KEY_SPACE           0x39
#define KEY_CAPITAL         0x3A
#define KEY_F1              0x3B
#define KEY_F2              0x3C
#define KEY_F3              0x3D
#define KEY_F4              0x3E
#define KEY_F5              0x3F
#define KEY_F6              0x40
#define KEY_F7              0x41
#define KEY_F8              0x42
#define KEY_F9              0x43
#define KEY_F10             0x44
#define KEY_NUMLOCK         0x45
#define KEY_SCROLL          0x46    /* Scroll Lock */
#define KEY_NUMPAD7         0x47
#define KEY_NUMPAD8         0x48
#define KEY_NUMPAD9         0x49
#define KEY_SUBTRACT        0x4A    /* - on numeric keypad */
#define KEY_NUMPAD4         0x4B
#define KEY_NUMPAD5         0x4C
#define KEY_NUMPAD6         0x4D
#define KEY_ADD             0x4E    /* + on numeric keypad */
#define KEY_NUMPAD1         0x4F
#define KEY_NUMPAD2         0x50
#define KEY_NUMPAD3         0x51
#define KEY_NUMPAD0         0x52
#define KEY_DECIMAL         0x53    /* . on numeric keypad */
#define KEY_OEM_102         0x56    /* <> or \| on RT 102-key keyboard (Non-U.S.) */
#define KEY_F11             0x57
#define KEY_F12             0x58
#define KEY_F13             0x64    /*                     (NEC PC98) */
#define KEY_F14             0x65    /*                     (NEC PC98) */
#define KEY_F15             0x66    /*                     (NEC PC98) */
#define KEY_KANA            0x70    /* (Japanese keyboard)            */
#define KEY_ABNT_C1         0x73    /* /? on Brazilian keyboard */
#define KEY_CONVERT         0x79    /* (Japanese keyboard)            */
#define KEY_NOCONVERT       0x7B    /* (Japanese keyboard)            */
#define KEY_YEN             0x7D    /* (Japanese keyboard)            */
#define KEY_ABNT_C2         0x7E    /* Numpad . on Brazilian keyboard */
#define KEY_NUMPADEQUALS    0x8D    /* = on numeric keypad (NEC PC98) */
#define KEY_PREVTRACK       0x90    /* Previous Track (DIK_CIRCUMFLEX on Japanese keyboard) */
#define KEY_AT              0x91    /*                     (NEC PC98) */
#define KEY_COLON           0x92    /*                     (NEC PC98) */
#define KEY_UNDERLINE       0x93    /*                     (NEC PC98) */
#define KEY_KANJI           0x94    /* (Japanese keyboard)            */
#define KEY_STOP            0x95    /*                     (NEC PC98) */
#define KEY_AX              0x96    /*                     (Japan AX) */
#define KEY_UNLABELED       0x97    /*                        (J3100) */
#define KEY_NEXTTRACK       0x99    /* Next Track */
#define KEY_NUMPADENTER     0x9C    /* Enter on numeric keypad */
#define KEY_RCONTROL        0x9D
#define KEY_MUTE            0xA0    /* Mute */
#define KEY_CALCULATOR      0xA1    /* Calculator */
#define KEY_PLAYPAUSE       0xA2    /* Play / Pause */
#define KEY_MEDIASTOP       0xA4    /* Media Stop */
#define KEY_VOLUMEDOWN      0xAE    /* Volume - */
#define KEY_VOLUMEUP        0xB0    /* Volume + */
#define KEY_WEBHOME         0xB2    /* Web home */
#define KEY_NUMPADCOMMA     0xB3    /* , on numeric keypad (NEC PC98) */
#define KEY_DIVIDE          0xB5    /* / on numeric keypad */
#define KEY_SYSRQ           0xB7
#define KEY_RMENU           0xB8    /* right Alt */
#define KEY_PAUSE           0xC5    /* Pause */
#define KEY_HOME            0xC7    /* Home on arrow keypad */
#define KEY_UP              0xC8    /* UpArrow on arrow keypad */
#define KEY_PRIOR           0xC9    /* PgUp on arrow keypad */
#define KEY_LEFT            0xCB    /* LeftArrow on arrow keypad */
#define KEY_RIGHT           0xCD    /* RightArrow on arrow keypad */
#define KEY_END             0xCF    /* End on arrow keypad */
#define KEY_DOWN            0xD0    /* DownArrow on arrow keypad */
#define KEY_NEXT            0xD1    /* PgDn on arrow keypad */
#define KEY_INSERT          0xD2    /* Insert on arrow keypad */
#define KEY_DELETE          0xD3    /* Delete on arrow keypad */
#define KEY_LWIN            0xDB    /* Left Windows key */
#define KEY_RWIN            0xDC    /* Right Windows key */
#define KEY_APPS            0xDD    /* AppMenu key */
#define KEY_POWER           0xDE    /* System Power */
#define KEY_SLEEP           0xDF    /* System Sleep */
#define KEY_WAKE            0xE3    /* System Wake */
#define KEY_WEBSEARCH       0xE5    /* Web Search */
#define KEY_WEBFAVORITES    0xE6    /* Web Favorites */
#define KEY_WEBREFRESH      0xE7    /* Web Refresh */
#define KEY_WEBSTOP         0xE8    /* Web Stop */
#define KEY_WEBFORWARD      0xE9    /* Web Forward */
#define KEY_WEBBACK         0xEA    /* Web Back */
#define KEY_MYCOMPUTER      0xEB    /* My Computer */
#define KEY_MAIL            0xEC    /* Mail */
#define KEY_MEDIASELECT     0xED    /* Media Select */


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
	static inline constexpr unsigned DEFAULT_EVENT_QUEUE_SIZE = 20;

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

	void virtual poll(bool relative_reset) override
	{
		std::lock_guard<std::mutex> scope_lock(m_device_lock);

		// Process each event until the queue is empty
		while (!m_event_queue.empty())
		{
			process_event(m_event_queue.front());
			m_event_queue.pop();
		}
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
	int             scan_code;
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
