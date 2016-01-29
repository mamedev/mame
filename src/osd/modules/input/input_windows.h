#ifndef INPUT_WIN_H_
#define INPUT_WIN_H_

#include "strconv.h"

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

// default axis names
const TCHAR *const default_axis_name[] =
{
	TEXT("X"), TEXT("Y"), TEXT("Z"), TEXT("RX"),
	TEXT("RY"), TEXT("RZ"), TEXT("SL1"), TEXT("SL2")
};

INT32 normalize_absolute_axis(INT32 raw, INT32 rawmin, INT32 rawmax);

//============================================================
//  TYPEDEFS
//============================================================

// state information for a keyboard
struct keyboard_state
{
	UINT8                   state[MAX_KEYS];
	INT8                    oldkey[MAX_KEYS];
	INT8                    currkey[MAX_KEYS];
};

// state information for a mouse (matches DIMOUSESTATE exactly)
struct mouse_state
{
	LONG                    lX;
	LONG                    lY;
	LONG                    lZ;
	BYTE                    rgbButtons[8];
};


//============================================================
//  device_info
//============================================================

struct osd_deleter
{
public:
	void operator()(void* obj) const
	{
		if (obj != nullptr)
		{
			osd_free(obj);
		}
	}
};

typedef std::unique_ptr<char, osd_deleter> osd_string_ptr;

class input_device_list;

class device_info
{
	friend input_device_list;

private:
	osd_string_ptr          m_name;
	input_device *          m_device;
	running_machine &       m_machine;
	input_module &          m_module;
	input_device_class      m_deviceclass;

public:
	// Constructor
	device_info(running_machine &machine, const TCHAR *name, input_device_class deviceclass, input_module &module)
		: m_device(nullptr),
			m_machine(machine),
			m_module(module),
			m_deviceclass(deviceclass)
	{
		m_name = osd_string_ptr(utf8_from_tstring(name));
	}

	// Destructor
	virtual ~device_info() {}

	// Getters
	running_machine &         machine() const { return m_machine; }
	const char *              name() { return m_name.get(); }
	input_device *            device() { return m_device; }
	input_module &            module() const { return m_module; }
	input_device_class        deviceclass() { return m_deviceclass; }

	// Poll and reset methods
	virtual void poll() {};
	virtual void reset() = 0;
};

struct key_trans_entry {
	input_item_id   mame_key;
	INT32           scan_code;
	unsigned char   virtual_key;
	char            ascii_key;
	char const  *   mame_key_name;
};

class keyboard_trans_table
{
private:
	keyboard_trans_table();
	int lookup_mame_index(const char * scode);
	static const key_trans_entry s_table[];

	UINT32 m_table_size;

public:
	UINT32 size() { return m_table_size; }
	input_item_id lookup_mame_code(const char * scode);
	input_item_id map_scancode_to_itemid(int scancode);
	int vkey_for_mame_code(input_code code);

	static keyboard_trans_table& instance()
	{
		static keyboard_trans_table s_instance;
		return s_instance;
	}

	key_trans_entry const & operator [](int i) { return s_table[i]; }
};

//============================================================
//  mouse_device
//============================================================

class mouse_device : public device_info
{
public:
	mouse_device(running_machine &machine, const TCHAR *name, input_module &module)
		: device_info(machine, name, DEVICE_CLASS_MOUSE, module),
		mouse({ 0 }) {}

	void reset() override
	{
		memset(&mouse, 0, sizeof(mouse));
	}

	mouse_state mouse;
};

//============================================================
//  keyboard_device
//============================================================

class keyboard_device : public device_info
{
public:
	keyboard_device(running_machine &machine, const TCHAR *name, input_module &module)
		: device_info(machine, name, DEVICE_CLASS_KEYBOARD, module),
		keyboard({ 0 }) {}

	keyboard_state keyboard;

	void reset() override
	{
		memset(&keyboard, 0, sizeof(keyboard));
	}
};

//============================================================
//  joystick_device
//============================================================

class joystick_device : public device_info
{
public:
	joystick_device(running_machine &machine, const TCHAR *name, input_module &module)
		: device_info(machine, name, DEVICE_CLASS_JOYSTICK, module)
	{
	}
};

//============================================================
//  lightgun_device
//============================================================

class lightgun_device : public device_info
{
public:
	lightgun_device(running_machine &machine, const TCHAR *name, input_module &module)
		: device_info(machine, name, DEVICE_CLASS_LIGHTGUN, module),
		  mouse({0})
	{
	}

	void reset() override
	{
		memset(&mouse, 0, sizeof(mouse));
	}

	mouse_state mouse;
};

//============================================================
//  input_device_list class
//============================================================

class input_device_list
{
protected:
	std::vector<std::unique_ptr<device_info>> m_list;

public:
	input_device_list()
	{
	}

	void poll_devices()
	{
		for (auto iter = m_list.begin(); iter != m_list.end(); iter++)
			iter->get()->poll();
	}

	void reset_devices()
	{
		for (auto iter = m_list.begin(); iter != m_list.end(); iter++)
			iter->get()->reset();
	}

	void free_device(device_info * devinfo)
	{
		// remove us from the list
		for (auto iter = m_list.begin(); iter != m_list.end(); iter++)
		{
			if (iter->get() == devinfo)
			{
				m_list.erase(iter);
				break;
			}
		}
	}

	int find_index(device_info* devinfo)
	{
		// remove us from the list
		int i = 0;
		for (auto iter = m_list.begin(); iter != m_list.end(); iter++)
		{
			if (iter->get() == devinfo)
			{
				break;
			}

			i++;
		}

		// return the index or -1 if we couldn't find it
		return i == m_list.size() ? -1 : i;
	}

	void free_all_devices()
	{
		while (!m_list.empty())
			m_list.pop_back();
	}

	int size()
	{
		return m_list.size();
	}

	device_info* at(int index)
	{
		return m_list.at(index).get();
	}

	template <typename TActual>
	TActual* create_device(running_machine &machine, const TCHAR *name, input_module &module)
	{
		// allocate the device object
		auto devinfo = std::make_unique<TActual>(machine, name, module);

		// Add the device to the machine
		devinfo->m_device = machine.input().device_class(devinfo->deviceclass()).add_device(devinfo->name(), devinfo.get());

		// append us to the list
		m_list.push_back(std::move(devinfo));

		return (TActual*)m_list.back().get();
	}

	template <class TActual>
	TActual* at(int index)
	{
		return (TActual*)m_list.at(index).get();
	}
};

//============================================================
//  wininput_module - base class for input modules
//============================================================

class wininput_module : public input_module
{
public:
	wininput_module(const char *type, const char* name)
		: input_module(type, name),
			m_last_poll(0),
			m_input_enabled(FALSE),
			m_mouse_enabled(FALSE),
			m_lightgun_enabled(FALSE),
			m_input_paused(FALSE),
			m_global_inputs_enabled(FALSE)
	{
	}

private:
	DWORD m_last_poll;
	BOOL m_input_enabled;
	BOOL m_mouse_enabled;
	BOOL m_lightgun_enabled;
	BOOL m_input_paused;
	const osd_options * m_options;
	input_device_list m_devicelist;

protected:
	BOOL m_global_inputs_enabled;

public:

	const osd_options *   options() { return m_options; }
	input_device_list *   devicelist() { return &m_devicelist; }
	bool                  input_enabled() { return m_input_enabled; }
	bool                  mouse_enabled() { return m_mouse_enabled; }
	bool                  lightgun_enabled() { return m_lightgun_enabled; }

	int init(const osd_options &options) override
	{
		m_options = &options;
		
		m_mouse_enabled = options.mouse();
		m_lightgun_enabled = options.lightgun();
		
		int result = init_internal();
		if (result != 0)
			return result;

		m_input_paused = FALSE;
		m_input_enabled = TRUE;

		return 0;
	}

	virtual int init_internal() { return 0; }

	void poll_if_necessary(running_machine &machine)
	{
		// make sure we poll at least once every 100ms
		if (GetTickCount() >= m_last_poll + 100)
			poll(machine);
	}

	virtual void poll(running_machine &machine)
	{
		// ignore if not enabled
		if (m_input_enabled)
		{
			// remember when this happened
			m_last_poll = GetTickCount();

			// periodically process events, in case they're not coming through
			// this also will make sure the mouse state is up-to-date
			winwindow_process_events_periodic(machine);

			// track if mouse/lightgun is enabled, for mouse hiding purposes
			m_mouse_enabled = machine.input().device_class(DEVICE_CLASS_MOUSE).enabled();
			m_lightgun_enabled = machine.input().device_class(DEVICE_CLASS_LIGHTGUN).enabled();
		}

		bool polldevices = m_input_enabled && (m_global_inputs_enabled || winwindow_has_focus());

		// poll all of the devices
		if (polldevices)
		{
			m_devicelist.poll_devices();
		}
		else
		{
			m_devicelist.reset_devices();
		}
	}

	virtual BOOL handle_input_event(input_event eventid, void* data)
	{
		return FALSE;
	}

	virtual BOOL should_hide_mouse()
	{
		if (winwindow_has_focus()  // has focus
			&& (!video_config.windowed || !win_window_list->win_has_menu()) // not windowed or doesn't have a menu
			&& (m_input_enabled && !m_input_paused) // input enabled and not paused
			&& (m_lightgun_enabled || m_mouse_enabled)) // either mouse or lightgun enabled in the core
		{
			return TRUE;
		}

		return FALSE;
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
};

//============================================================
//  INLINE FUNCTIONS
//============================================================

INT32 generic_button_get_state(void *device_internal, void *item_internal);
INT32 generic_axis_get_state(void *device_internal, void *item_internal);
const TCHAR *default_button_name(int which);
#endif