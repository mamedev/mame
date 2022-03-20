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

#include "inputdev.h"

#include <algorithm>
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

class input_device_list;

class device_info
{
	friend input_device_list;

private:
	const std::string       m_name;
	const std::string       m_id;
	input_device *          m_device;
	running_machine &       m_machine;
	input_module &          m_module;
	input_device_class      m_deviceclass;

public:
	// Constructor
	device_info(running_machine &machine, std::string &&name, std::string &&id, input_device_class deviceclass, input_module &module) :
		m_name(std::move(name)),
		m_id(std::move(id)),
		m_device(nullptr),
		m_machine(machine),
		m_module(module),
		m_deviceclass(deviceclass)
	{
	}

	// Destructor
	virtual ~device_info() { }

	// Getters
	running_machine &         machine() const { return m_machine; }
	const std::string &       name() const { return m_name; }
	const std::string &       id() const { return m_id; }
	input_device *            device() const { return m_device; }
	input_module &            module() const { return m_module; }
	input_device_class        deviceclass() const { return m_deviceclass; }

	// Poll and reset methods
	virtual void poll() { }
	virtual void reset() = 0;
};

//============================================================
//  event_based_device
//============================================================

#define DEFAULT_EVENT_QUEUE_SIZE 20

template <class TEvent>
class event_based_device : public device_info
{
private:
	std::queue<TEvent>   m_event_queue;

protected:
	std::mutex           m_device_lock;

	virtual void process_event(TEvent &ev) = 0;

public:
	event_based_device(running_machine &machine, std::string &&name, std::string &&id, input_device_class deviceclass, input_module &module)
		: device_info(machine, std::move(name), std::move(id), deviceclass, module)
	{
	}

	void queue_events(const TEvent *events, int count)
	{
		std::lock_guard<std::mutex> scope_lock(m_device_lock);
		for (int i = 0; i < count; i++)
			m_event_queue.push(events[i]);

		// If we've gone over the size, remove old events from the queue
		while (m_event_queue.size() > DEFAULT_EVENT_QUEUE_SIZE)
			m_event_queue.pop();
	}

	void virtual poll() override
	{
		std::lock_guard<std::mutex> scope_lock(m_device_lock);

		// Process each event until the queue is empty
		while (!m_event_queue.empty())
		{
			TEvent &next_event = m_event_queue.front();
			process_event(next_event);
			m_event_queue.pop();
		}
	}
};

//============================================================
//  input_device_list class
//============================================================

class input_device_list
{
private:
	std::vector<std::unique_ptr<device_info>> m_list;

public:
	auto size() const { return m_list.size(); }
	auto empty() const { return m_list.empty(); }
	auto begin() { return m_list.begin(); }
	auto end() { return m_list.end(); }

	void poll_devices()
	{
		for (auto &device: m_list)
			device->poll();
	}

	void reset_devices()
	{
		for (auto &device: m_list)
			device->reset();
	}

	void free_device(device_info &devinfo)
	{
		// find the device to remove
		const auto device_matches = [&devinfo] (std::unique_ptr<device_info> &device) { return &devinfo == device.get(); };
		m_list.erase(std::remove_if(std::begin(m_list), std::end(m_list), device_matches), m_list.end());
	}

	template <typename T>
	void for_each_device(T &&action)
	{
		for (auto &device: m_list)
			action(device.get());
	}

	void free_all_devices()
	{
		while (!m_list.empty())
			m_list.pop_back();
	}

	template <typename TActual, typename... TArgs>
	TActual &create_device(running_machine &machine, std::string &&name, std::string &&id, input_module &module, TArgs&&... args)
	{
		// allocate the device object
		auto devinfo = std::make_unique<TActual>(machine, std::move(name), std::move(id), module, std::forward<TArgs>(args)...);

		return add_device(machine, std::move(devinfo));
	}

	template <typename TActual>
	TActual &add_device(running_machine &machine, std::unique_ptr<TActual> devinfo)
	{
		// Add the device to the machine
		devinfo->m_device = &machine.input().device_class(devinfo->deviceclass()).add_device(devinfo->name(), devinfo->id(), devinfo.get());

		// append us to the list
		return *static_cast<TActual *>(m_list.emplace_back(std::move(devinfo)).get());
	}
};

// keyboard translation table

struct key_trans_entry {
	input_item_id   mame_key;

#if defined(OSD_SDL)
	int             sdl_scancode;
#elif defined(OSD_WINDOWS)
	int             scan_code;
	unsigned char   virtual_key;
#endif

	char            ascii_key;
	char const  *   mame_key_name;
	char *          ui_name;
};

class keyboard_trans_table
{
private:
	// default constructor is private
	keyboard_trans_table();

	static key_trans_entry              s_default_table[];
	std::unique_ptr<key_trans_entry[]>  m_custom_table;

	key_trans_entry *                   m_table;
	uint32_t                              m_table_size;

public:
	// constructor
	keyboard_trans_table(std::unique_ptr<key_trans_entry[]> table, unsigned int size);

	// getters/setters
	uint32_t size() const { return m_table_size; }

	// public methods
	input_item_id lookup_mame_code(const char * scode) const;
	int lookup_mame_index(const char * scode) const;

#if defined(OSD_WINDOWS)
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

class osd_options;

typedef std::chrono::high_resolution_clock clock_type;
typedef std::chrono::time_point<std::chrono::high_resolution_clock> timepoint_type;

// 10 milliseconds polling interval
#define MIN_POLLING_INTERVAL 10

class input_module_base : public input_module
{
public:
	input_module_base(const char *type, const char* name) :
		input_module(type, name),
		m_input_enabled(false),
		m_mouse_enabled(false),
		m_lightgun_enabled(false),
		m_input_paused(false),
		m_options(nullptr)
	{
	}

private:
	bool                  m_input_enabled;
	bool                  m_mouse_enabled;
	bool                  m_lightgun_enabled;
	bool                  m_input_paused;
	const osd_options *   m_options;
	input_device_list     m_devicelist;
	clock_type            m_clock;
	timepoint_type        m_last_poll;

protected:
	void set_mouse_enabled(bool value) { m_mouse_enabled = value; }

public:

	const osd_options *   options() const { return m_options; }
	input_device_list *   devicelist() { return &m_devicelist; }
	bool                  input_enabled() const { return m_input_enabled; }
	bool                  input_paused() const { return m_input_paused; }
	bool                  mouse_enabled() const { return m_mouse_enabled; }
	bool                  lightgun_enabled() const { return m_lightgun_enabled; }

	int init(const osd_options &options) override;

	void poll_if_necessary(running_machine &machine) override
	{
		auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(m_clock.now() - m_last_poll);
		if (elapsed.count() >= MIN_POLLING_INTERVAL)
		{
			poll(machine);
		}
	}

	virtual void poll(running_machine &machine)
	{
		// ignore if not enabled
		if (m_input_enabled)
		{
			// grab the current time
			m_last_poll = m_clock.now();

			before_poll(machine);

			// track if mouse/lightgun is enabled, for mouse hiding purposes
			m_mouse_enabled = machine.input().device_class(DEVICE_CLASS_MOUSE).enabled();
			m_lightgun_enabled = machine.input().device_class(DEVICE_CLASS_LIGHTGUN).enabled();
		}

		// poll all of the devices
		if (should_poll_devices(machine))
		{
			m_devicelist.poll_devices();
		}
		else
		{
			m_devicelist.reset_devices();
		}
	}

	virtual void pause() override
	{
		// keep track of the paused state
		m_input_paused = true;
	}

	virtual void resume() override
	{
		// keep track of the paused state
		m_input_paused = false;
	}

	virtual void exit() override
	{
		devicelist()->free_all_devices();
	}

protected:
	virtual int init_internal() { return 0; }
	virtual bool should_poll_devices(running_machine &machine) = 0;
	virtual void before_poll(running_machine &machine) {}
};

template <class TItem>
int generic_button_get_state(void *device_internal, void *item_internal)
{
	device_info *devinfo = static_cast<device_info *>(device_internal);
	TItem *itemdata = static_cast<TItem*>(item_internal);

	// return the current state
	devinfo->module().poll_if_necessary(devinfo->machine());
	return *itemdata >> 7;
}

template <class TItem>
int generic_axis_get_state(void *device_internal, void *item_internal)
{
	device_info *devinfo = static_cast<device_info *>(device_internal);
	TItem *axisdata = static_cast<TItem*>(item_internal);

	// return the current state
	devinfo->module().poll_if_necessary(devinfo->machine());
	return *axisdata;
}

//============================================================
//  default_button_name
//============================================================

inline const char *default_button_name(int which)
{
	static char buffer[20];
	snprintf(buffer, std::size(buffer), "B%d", which);
	return buffer;
}

//============================================================
//  default_pov_name
//============================================================

inline const char *default_pov_name(int which)
{
	static char buffer[20];
	snprintf(buffer, std::size(buffer), "POV%d", which);
	return buffer;
}

// default axis names
const char *const default_axis_name[] =
{
	"X", "Y", "Z", "RX",
	"RY", "RZ", "SL1", "SL2"
};

inline int32_t normalize_absolute_axis(double raw, double rawmin, double rawmax)
{
	double center = (rawmax + rawmin) / 2.0;

	// make sure we have valid data
	if (rawmin >= rawmax)
		return int32_t(raw);

	if (raw >= center)
	{
		// above center
		double result = (raw - center) * INPUT_ABSOLUTE_MAX / (rawmax - center);
		return std::min(result, (double)INPUT_ABSOLUTE_MAX);
	}
	else
	{
		// below center
		double result = -((center - raw) * (double)-INPUT_ABSOLUTE_MIN / (center - rawmin));
		return std::max(result, (double)INPUT_ABSOLUTE_MIN);
	}
}

#endif // MAME_OSD_INPUT_INPUT_COMMON_H
