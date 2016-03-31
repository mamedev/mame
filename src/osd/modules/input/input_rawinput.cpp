// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Brad Hughes
//============================================================
//
//  input_rawinput.cpp - Windows RawInput input implementation
//
//============================================================

#include "input_module.h"
#include "modules/osdmodule.h"

#if defined(OSD_WINDOWS)

// standard windows headers
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tchar.h>
#undef interface

#include <mutex>
#include <functional>

// MAME headers
#include "emu.h"
#include "osdepend.h"
#include "strconv.h"

// MAMEOS headers
#include "winutil.h"
#include "winmain.h"
#include "window.h"

#include "input_common.h"
#include "input_windows.h"

//============================================================
//  MACROS
//============================================================

#ifdef UNICODE
#define UNICODE_SUFFIX      "W"
#else
#define UNICODE_SUFFIX      "A"
#endif

// RawInput APIs
typedef lazy_loaded_function_p3<INT, PRAWINPUTDEVICELIST, PINT, UINT> get_rawinput_device_list_ptr;
typedef lazy_loaded_function_p5<INT, HRAWINPUT, UINT, LPVOID, PINT, UINT> get_rawinput_data_ptr;
typedef lazy_loaded_function_p4<INT, HANDLE, UINT, LPVOID, PINT> get_rawinput_device_info_ptr;
typedef lazy_loaded_function_p3<BOOL, PCRAWINPUTDEVICE, UINT, UINT> register_rawinput_devices_ptr;

class safe_regkey
{
private:
	HKEY m_key;

public:
	safe_regkey()
		: m_key(nullptr)
	{
	}

	explicit safe_regkey(HKEY key)
		: m_key(key)
	{
	}

	bool valid() const { return m_key != nullptr; }

	void close()
	{
		if (m_key != nullptr)
		{
			RegCloseKey(m_key);
			m_key = nullptr;
		}
	}

	~safe_regkey()
	{
		close();
	}

	operator HKEY() const { return m_key; }
};

//============================================================
//  reg_open_key
//============================================================

static safe_regkey reg_open_key(HKEY basekey, const std::wstring &subkey)
{
	HKEY key;
	if (RegOpenKeyEx(basekey, subkey.c_str(), 0, KEY_READ, &key) == ERROR_SUCCESS)
		return safe_regkey(key);

	return safe_regkey();

}

//============================================================
//  reg_enum_key
//============================================================

static std::wstring reg_enum_key(HKEY key, int index)
{
	WCHAR keyname[MAX_PATH];
	DWORD namelen;
	if (RegEnumKeyEx(key, index, keyname, &namelen, nullptr, nullptr, nullptr, nullptr) == ERROR_SUCCESS)
		return std::wstring(keyname, namelen);

	return std::wstring();
}

//============================================================
//  reg_query_string
//============================================================

static std::wstring reg_query_string(HKEY key, const TCHAR *path)
{
	DWORD datalen;
	LONG result;

	// first query to get the length
	result = RegQueryValueEx(key, path, nullptr, nullptr, nullptr, &datalen);
	if (result != ERROR_SUCCESS)
		return std::wstring();

	// allocate a buffer
	auto buffer = std::make_unique<TCHAR[]>(datalen + sizeof(TCHAR));
	buffer[datalen / sizeof(TCHAR)] = 0;

	// now get the actual data
	result = RegQueryValueEx(key, path, nullptr, nullptr, reinterpret_cast<LPBYTE>(buffer.get()), &datalen);
	if (result == ERROR_SUCCESS)
		return std::wstring(buffer.get());

	// otherwise return an empty string
	return std::wstring();
}

static std::wstring trim_prefix(const std::wstring &devicename)
{
	// remove anything prior to the final semicolon
	auto semicolon_index = devicename.find_last_of(';');
	if (semicolon_index != std::wstring::npos)
		return devicename.substr(semicolon_index + 1);

	return devicename;
}

static std::wstring compute_device_regpath(const std::wstring &name)
{
	static const std::wstring basepath(L"SYSTEM\\CurrentControlSet\\Enum\\");

	// allocate a temporary string and concatenate the base path plus the name
	auto regpath_buffer = std::make_unique<TCHAR[]>(basepath.length() + 1 + name.length());
	wcscpy(regpath_buffer.get(), basepath.c_str());
	WCHAR * chdst = regpath_buffer.get() + basepath.length();

	// convert all # to \ in the name
	for (int i = 4; i < name.length(); i++)
		*chdst++ = (name[i] == '#') ? '\\' : name[i];
	*chdst = 0;

	// remove the final chunk
	chdst = wcsrchr(regpath_buffer.get(), '\\');
	if (chdst == nullptr)
		return std::wstring();

	*chdst = 0;

	return std::wstring(regpath_buffer.get());
}

static std::wstring improve_name_from_base_path(const std::wstring &regpath, bool *hid)
{
	// now try to open the registry key
	auto device_key = reg_open_key(HKEY_LOCAL_MACHINE, regpath);
	if (!device_key.valid())
		return std::wstring();

	// fetch the device description; if it exists, we are finished
	auto regstring = reg_query_string(device_key, L"DeviceDesc");
	if (!regstring.empty())
		return trim_prefix(regstring);

	// if the key name does not contain "HID", it's not going to be in the USB tree; give up
	*hid = regpath.find(L"HID") != std::string::npos;
	return std::wstring();
}

static void foreach_subkey(HKEY key, std::function<bool(HKEY)> action)
{
	for (int i = 0; ; i++)
	{
		std::wstring name = reg_enum_key(key, i);
		if (name.empty())
			break;

		safe_regkey subkey = reg_open_key(key, name);
		if (!subkey.valid())
			break;

		bool shouldcontinue = action(subkey);
		if (!shouldcontinue)
			break;
	}
}

static std::wstring improve_name_from_usb_path(const std::wstring &regpath)
{
	static const std::wstring usbbasepath(L"SYSTEM\\CurrentControlSet\\Enum\\USB");

	// extract the expected parent ID from the regpath
	size_t last_slash_index = regpath.find_last_of('\\');
	if (last_slash_index == std::wstring::npos)
		return std::wstring();

	std::wstring parentid = regpath.substr(last_slash_index + 1);

	// open the USB key
	auto usb_key = reg_open_key(HKEY_LOCAL_MACHINE, usbbasepath);
	if (!usb_key.valid())
		return std::wstring();

	std::wstring regstring;

	foreach_subkey(usb_key, [&regstring, &parentid](HKEY subkey)
	{
		foreach_subkey(subkey, [&regstring, &parentid](HKEY endkey)
		{
			std::wstring endparentid = reg_query_string(endkey, L"ParentIdPrefix");

			// This key doesn't have a ParentIdPrefix
			if (endparentid.empty())
				return true;

			// do we have a match?
			if (parentid.find(endparentid) == 0)
				regstring = reg_query_string(endkey, L"DeviceDesc");

			return regstring.empty();
		});

		return regstring.empty();
	});

	return trim_prefix(regstring);
}

//============================================================
//  rawinput_device_improve_name
//============================================================

static std::wstring rawinput_device_improve_name(const std::wstring &name)
{
	// The RAW name received is formatted as:
	//   \??\type-id#hardware-id#instance-id#{DeviceClasses-id}
	// XP starts with "\??\"
	// Vista64 starts with "\\?\"

	// ensure the name is something we can handle
	if (name.find(L"\\\\?\\") != 0 && name.find(L"\\??\\") != 0)
		return name;

	std::wstring regpath = compute_device_regpath(name);

	bool hid = false;
	auto improved = improve_name_from_base_path(regpath, &hid);
	if (!improved.empty())
		return improved;

	if (hid)
	{
		improved = improve_name_from_usb_path(regpath);
		if (!improved.empty())
			return improved;
	}

	// Fall back to the original name
	return name;
}


//============================================================
//  rawinput_device class
//============================================================

class rawinput_device : public event_based_device<RAWINPUT>
{
private:
	HANDLE  m_handle;

public:
	rawinput_device(running_machine& machine, const char* name, input_device_class deviceclass, input_module& module)
		: event_based_device(machine, name, deviceclass, module),
			m_handle(nullptr)
	{
	}

	HANDLE device_handle() const { return m_handle; }
	void set_handle(HANDLE handle) { m_handle = handle; }
};

//============================================================
//  rawinput_keyboard_device
//============================================================

class rawinput_keyboard_device : public rawinput_device
{
public:
	keyboard_state keyboard;

	rawinput_keyboard_device(running_machine& machine, const char* name, input_module& module)
		: rawinput_device(machine, name, DEVICE_CLASS_KEYBOARD, module),
			keyboard({{0}})
	{
	}

	void reset() override
	{
		memset(&keyboard, 0, sizeof(keyboard));
	}

	void process_event(RAWINPUT &rawinput) override
	{
		// determine the full DIK-compatible scancode
		UINT8 scancode = (rawinput.data.keyboard.MakeCode & 0x7f) | ((rawinput.data.keyboard.Flags & RI_KEY_E0) ? 0x80 : 0x00);

		// scancode 0xaa is a special shift code we need to ignore
		if (scancode == 0xaa)
			return;

		// set or clear the key
		keyboard.state[scancode] = (rawinput.data.keyboard.Flags & RI_KEY_BREAK) ? 0x00 : 0x80;
	}
};

//============================================================
//  rawinput_mouse_device
//============================================================

class rawinput_mouse_device : public rawinput_device
{
private:
	std::mutex  m_device_lock;
public:
	mouse_state          mouse;

	rawinput_mouse_device(running_machine& machine, const char* name, input_module& module)
		: rawinput_device(machine, name, DEVICE_CLASS_MOUSE, module),
			mouse({0})
	{
	}

	void poll() override
	{
		mouse.lX = 0;
		mouse.lY = 0;
		mouse.lZ = 0;

		rawinput_device::poll();
	}

	void reset() override
	{
		memset(&mouse, 0, sizeof(mouse));
	}

	void process_event(RAWINPUT &rawinput) override
	{
		// If this data was intended for a rawinput lightgun, ignore it.
		if (rawinput.data.mouse.usFlags & MOUSE_MOVE_ABSOLUTE)
			return;

		mouse.lX += rawinput.data.mouse.lLastX * INPUT_RELATIVE_PER_PIXEL;
		mouse.lY += rawinput.data.mouse.lLastY * INPUT_RELATIVE_PER_PIXEL;

		// update zaxis
		if (rawinput.data.mouse.usButtonFlags & RI_MOUSE_WHEEL)
			mouse.lZ += static_cast<INT16>(rawinput.data.mouse.usButtonData) * INPUT_RELATIVE_PER_PIXEL;

		// update the button states; always update the corresponding mouse buttons
		if (rawinput.data.mouse.usButtonFlags & RI_MOUSE_BUTTON_1_DOWN) mouse.rgbButtons[0] = 0x80;
		if (rawinput.data.mouse.usButtonFlags & RI_MOUSE_BUTTON_1_UP)   mouse.rgbButtons[0] = 0x00;
		if (rawinput.data.mouse.usButtonFlags & RI_MOUSE_BUTTON_2_DOWN) mouse.rgbButtons[1] = 0x80;
		if (rawinput.data.mouse.usButtonFlags & RI_MOUSE_BUTTON_2_UP)   mouse.rgbButtons[1] = 0x00;
		if (rawinput.data.mouse.usButtonFlags & RI_MOUSE_BUTTON_3_DOWN) mouse.rgbButtons[2] = 0x80;
		if (rawinput.data.mouse.usButtonFlags & RI_MOUSE_BUTTON_3_UP)   mouse.rgbButtons[2] = 0x00;
		if (rawinput.data.mouse.usButtonFlags & RI_MOUSE_BUTTON_4_DOWN) mouse.rgbButtons[3] = 0x80;
		if (rawinput.data.mouse.usButtonFlags & RI_MOUSE_BUTTON_4_UP)   mouse.rgbButtons[3] = 0x00;
		if (rawinput.data.mouse.usButtonFlags & RI_MOUSE_BUTTON_5_DOWN) mouse.rgbButtons[4] = 0x80;
		if (rawinput.data.mouse.usButtonFlags & RI_MOUSE_BUTTON_5_UP)   mouse.rgbButtons[4] = 0x00;
	}
};

/*
register_rawinput_devices = (register_rawinput_devices_ptr)GetProcAddress(user32, "RegisterRawInputDevices");
get_rawinput_device_list = (get_rawinput_device_list_ptr)GetProcAddress(user32, "GetRawInputDeviceList");
get_rawinput_device_info = (get_rawinput_device_info_ptr)GetProcAddress(user32, "GetRawInputDeviceInfo" UNICODE_SUFFIX);
get_rawinput_data = (get_rawinput_data_ptr)GetProcAddress(user32, "GetRawInputData");
*/

//============================================================
//  rawinput_module - base class for rawinput modules
//============================================================

class rawinput_module : public wininput_module
{
private:
	// RawInput variables
	get_rawinput_device_list_ptr     get_rawinput_device_list;
	get_rawinput_data_ptr            get_rawinput_data;
	get_rawinput_device_info_ptr     get_rawinput_device_info;
	register_rawinput_devices_ptr    register_rawinput_devices;
	std::mutex                       m_module_lock;

public:
	rawinput_module(const char *type, const char* name)
		: wininput_module(type, name),
			get_rawinput_device_list("GetRawInputDeviceList", L"user32.dll"),
			get_rawinput_data("GetRawInputData", L"user32.dll"),
			get_rawinput_device_info("GetRawInputDeviceInfoW", L"user32.dll"),
			register_rawinput_devices("RegisterRawInputDevices", L"user32.dll")
	{
	}

	bool probe() override
	{
		int status = get_rawinput_device_list.initialize();
		status |= get_rawinput_data.initialize();
		status |= get_rawinput_device_info.initialize();
		status |= register_rawinput_devices.initialize();

		if (status != 0)
			return false;

		return true;
	}

	void input_init(running_machine &machine) override
	{
		// get the number of devices, allocate a device list, and fetch it
		int device_count = 0;
		if (get_rawinput_device_list(nullptr, &device_count, sizeof(RAWINPUTDEVICELIST)) != 0)
			return;

		if (device_count == 0)
			return;

		auto rawinput_devices = std::make_unique<RAWINPUTDEVICELIST[]>(device_count);
		if (get_rawinput_device_list(rawinput_devices.get(), &device_count, sizeof(RAWINPUTDEVICELIST)) == -1)
			return;

		// iterate backwards through devices; new devices are added at the head
		for (int devnum = device_count - 1; devnum >= 0; devnum--)
		{
			RAWINPUTDEVICELIST *device = &rawinput_devices[devnum];
			add_rawinput_device(machine, device);
		}

		// don't enable global inputs when debugging
		if (!machine.options().debug())
		{
			m_global_inputs_enabled = downcast<windows_options &>(machine.options()).global_inputs();
		}

		// If we added no devices, no need to register for notifications
		if (devicelist()->size() == 0)
			return;

		// finally, register to receive raw input WM_INPUT messages if we found devices
		RAWINPUTDEVICE registration;
		registration.usUsagePage = usagepage();
		registration.usUsage = usage();
		registration.dwFlags = m_global_inputs_enabled ? 0x00000100 : 0;
		registration.hwndTarget = win_window_list->m_hwnd;

		// register the device
		register_rawinput_devices(&registration, 1, sizeof(registration));
	}

protected:
	virtual void add_rawinput_device(running_machine& machine, RAWINPUTDEVICELIST * device) = 0;
	virtual USHORT usagepage() = 0;
	virtual USHORT usage() = 0;

	int init_internal() override
	{
		// look up the entry points
		int status = get_rawinput_device_list.initialize();
		status |= get_rawinput_data.initialize();
		status |= get_rawinput_device_info.initialize();
		status |= register_rawinput_devices.initialize();
		if (status != 0)
			return 1;

		osd_printf_verbose("RawInput: APIs detected\n");
		return 0;
	}

	template<class TDevice>
	TDevice* create_rawinput_device(running_machine &machine, PRAWINPUTDEVICELIST rawinputdevice)
	{
		TDevice* devinfo;
		INT name_length = 0;
		// determine the length of the device name, allocate it, and fetch it if not nameless
		if (get_rawinput_device_info(rawinputdevice->hDevice, RIDI_DEVICENAME, nullptr, &name_length) != 0)
			return nullptr;

		std::unique_ptr<TCHAR[]> tname = std::make_unique<TCHAR[]>(name_length + 1);
		if (name_length > 1 && get_rawinput_device_info(rawinputdevice->hDevice, RIDI_DEVICENAME, tname.get(), &name_length) == -1)
			return nullptr;

		// if this is an RDP name, skip it
		if (_tcsstr(tname.get(), TEXT("Root#RDP_")) != nullptr)
			return nullptr;

		// improve the name and then allocate a device
		std::wstring name = rawinput_device_improve_name(tname.get());

		// convert name to utf8
		auto osd_deleter = [](void *ptr) { osd_free(ptr); };
		auto utf8_name = std::unique_ptr<char, decltype(osd_deleter)>(utf8_from_wstring(name.c_str()), osd_deleter);

		devinfo = devicelist()->create_device<TDevice>(machine, utf8_name.get(), *this);

		// Add the handle
		devinfo->set_handle(rawinputdevice->hDevice);

		return devinfo;
	}

	bool handle_input_event(input_event eventid, void* eventdata) override
	{
		// Only handle raw input data
		if (!input_enabled() || eventid != INPUT_EVENT_RAWINPUT)
			return FALSE;

		HRAWINPUT rawinputdevice = *static_cast<HRAWINPUT*>(eventdata);

		BYTE small_buffer[4096];
		std::unique_ptr<BYTE[]> larger_buffer;
		LPBYTE data = small_buffer;
		BOOL result;
		int size;

		// ignore if not enabled
		if (!input_enabled())
			return FALSE;

		// determine the size of databuffer we need
		if (get_rawinput_data(rawinputdevice, RID_INPUT, nullptr, &size, sizeof(RAWINPUTHEADER)) != 0)
			return FALSE;

		// if necessary, allocate a temporary buffer and fetch the data
		if (size > sizeof(small_buffer))
		{
			larger_buffer = std::make_unique<BYTE[]>(size);
			data = larger_buffer.get();
			if (data == nullptr)
				return FALSE;
		}

		// fetch the data and process the appropriate message types
		result = get_rawinput_data(static_cast<HRAWINPUT>(rawinputdevice), RID_INPUT, data, &size, sizeof(RAWINPUTHEADER));
		if (result)
		{
			std::lock_guard<std::mutex> scope_lock(m_module_lock);

			rawinput_device *devinfo;
			// find the device in the list and update
			for (int i = 0; i < devicelist()->size(); i++)
			{
				devinfo = dynamic_cast<rawinput_device*>(devicelist()->at(i));
				if (devinfo)
				{
					RAWINPUT *input = reinterpret_cast<RAWINPUT*>(data);
					if (input->header.hDevice == devinfo->device_handle())
					{
						devinfo->queue_events(input, 1);
						result = TRUE;
					}
				}
			}
		}

		return result;
	}
};

//============================================================
//  keyboard_input_rawinput - rawinput keyboard module
//============================================================

class keyboard_input_rawinput : public rawinput_module
{
public:
	keyboard_input_rawinput()
		: rawinput_module(OSD_KEYBOARDINPUT_PROVIDER, "rawinput")
	{
	}

protected:
	USHORT usagepage() override { return 1; }
	USHORT usage() override { return 6; }
	void add_rawinput_device(running_machine& machine, RAWINPUTDEVICELIST * device) override
	{
		// make sure this is a keyboard
		if (device->dwType != RIM_TYPEKEYBOARD)
			return;

		// allocate and link in a new device
		rawinput_keyboard_device *devinfo = create_rawinput_device<rawinput_keyboard_device>(machine, device);
		if (devinfo == nullptr)
			return;

		keyboard_trans_table &table = keyboard_trans_table::instance();

		// populate it
		for (int keynum = 0; keynum < MAX_KEYS; keynum++)
		{
			input_item_id itemid = table.map_di_scancode_to_itemid(keynum);
			TCHAR keyname[100];
			char *name;

			// generate the name
			if (GetKeyNameText(((keynum & 0x7f) << 16) | ((keynum & 0x80) << 17), keyname, ARRAY_LENGTH(keyname)) == 0)
				_sntprintf(keyname, ARRAY_LENGTH(keyname), TEXT("Scan%03d"), keynum);
			name = utf8_from_tstring(keyname);

			// add the item to the device
			devinfo->device()->add_item(name, itemid, generic_button_get_state, &devinfo->keyboard.state[keynum]);
			osd_free(name);
		}
	}
};

//============================================================
//  mouse_input_rawinput - rawinput mouse module
//============================================================

class mouse_input_rawinput : public rawinput_module
{
public:
	mouse_input_rawinput()
		: rawinput_module(OSD_MOUSEINPUT_PROVIDER, "rawinput")
	{
	}

protected:
	USHORT usagepage() override { return 1; }
	USHORT usage() override { return 2; }
	void add_rawinput_device(running_machine& machine, RAWINPUTDEVICELIST * device) override
	{
		// make sure this is a mouse
		if (device->dwType != RIM_TYPEMOUSE)
			return;

		// allocate and link in a new device
		rawinput_mouse_device *devinfo = create_rawinput_device<rawinput_mouse_device>(machine, device);
		if (devinfo == nullptr)
			return;

		// populate the axes
		for (int axisnum = 0; axisnum < 3; axisnum++)
		{
			devinfo->device()->add_item(default_axis_name[axisnum], static_cast<input_item_id>(ITEM_ID_XAXIS + axisnum), generic_axis_get_state, &devinfo->mouse.lX + axisnum);
		}

		// populate the buttons
		for (int butnum = 0; butnum < 5; butnum++)
		{
			devinfo->device()->add_item(default_button_name(butnum), static_cast<input_item_id>(ITEM_ID_BUTTON1 + butnum), generic_button_get_state, &devinfo->mouse.rgbButtons[butnum]);
		}
	}
};

#else
MODULE_NOT_SUPPORTED(keyboard_input_rawinput, OSD_KEYBOARDINPUT_PROVIDER, "rawinput")
MODULE_NOT_SUPPORTED(mouse_input_rawinput, OSD_MOUSEINPUT_PROVIDER, "rawinput")
//MODULE_NOT_SUPPORTED(lightgun_input_rawinput, OSD_LIGHTGUNINPUT_PROVIDER, "rawinput")
#endif

MODULE_DEFINITION(KEYBOARDINPUT_RAWINPUT, keyboard_input_rawinput)
MODULE_DEFINITION(MOUSEINPUT_RAWINPUT, mouse_input_rawinput)
//MODULE_DEFINITION(LIGHTGUNINPUT_RAWINPUT, lightgun_input_rawinput)
