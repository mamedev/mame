// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Brad Hughes
//============================================================
//
//  input_rawinput.cpp - Windows RawInput input implementation
//
//============================================================

#include "modules/osdmodule.h"

#if defined(OSD_WINDOWS)

#include "input_windows.h"

#include "input_wincommon.h"

#include "winmain.h"
#include "window.h"

#include "modules/lib/osdlib.h"
#include "strconv.h"

// MAME headers
#include "inpttype.h"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <functional>
#include <mutex>
#include <new>
#include <utility>

// standard windows headers
#include <windows.h>
#include <tchar.h>


namespace osd {

namespace {

class safe_regkey
{
private:
	HKEY m_key;

public:
	safe_regkey() : m_key(nullptr) { }
	safe_regkey(safe_regkey const &) = delete;
	safe_regkey(safe_regkey &&key) : m_key(key.m_key) { key.m_key = nullptr; }
	explicit safe_regkey(HKEY key) : m_key(key) { }

	~safe_regkey() { close(); }

	safe_regkey &operator=(safe_regkey const &) = delete;

	safe_regkey &operator=(safe_regkey &&key)
	{
		close();
		m_key = key.m_key;
		key.m_key = nullptr;
		return *this;
	}

	explicit operator bool() const { return m_key != nullptr; }

	void close()
	{
		if (m_key != nullptr)
		{
			RegCloseKey(m_key);
			m_key = nullptr;
		}
	}

	operator HKEY() const { return m_key; }

	safe_regkey open(std::wstring const &subkey) const { return open(m_key, subkey); }

	std::wstring enum_key(int index) const
	{
		WCHAR keyname[256];
		DWORD namelen = std::size(keyname);
		if (RegEnumKeyEx(m_key, index, keyname, &namelen, nullptr, nullptr, nullptr, nullptr) == ERROR_SUCCESS)
			return std::wstring(keyname, namelen);
		else
			return std::wstring();
	}

	std::wstring query_string(WCHAR const *path) const
	{
		// first query to get the length
		DWORD datalen;
		if (RegQueryValueExW(m_key, path, nullptr, nullptr, nullptr, &datalen) != ERROR_SUCCESS)
			return std::wstring();

		// allocate a buffer
		auto buffer = std::make_unique<WCHAR []>((datalen + (sizeof(WCHAR) * 2) - 1) / sizeof(WCHAR));

		// now get the actual data
		if (RegQueryValueExW(m_key, path, nullptr, nullptr, reinterpret_cast<LPBYTE>(buffer.get()), &datalen) != ERROR_SUCCESS)
			return std::wstring();

		buffer[datalen / sizeof(WCHAR)] = 0;
		return std::wstring(buffer.get());
	}

	template <typename T> void foreach_subkey(T &&action) const
	{
		std::wstring name;
		for (int i = 0; ; i++)
		{
			name = enum_key(i);
			if (name.empty())
				break;

			safe_regkey const subkey = open(name);
			if (!subkey)
				break;

			bool const shouldcontinue = action(subkey);
			if (!shouldcontinue)
				break;
		}
	}

	static safe_regkey open(HKEY basekey, std::wstring const &subkey)
	{
		HKEY key(nullptr);
		if (RegOpenKeyEx(basekey, subkey.c_str(), 0, KEY_READ, &key) == ERROR_SUCCESS)
			return safe_regkey(key);
		else
			return safe_regkey();
	}
};

std::wstring trim_prefix(const std::wstring &devicename)
{
	// remove anything prior to the final semicolon
	auto semicolon_index = devicename.find_last_of(';');
	if (semicolon_index != std::wstring::npos)
		return devicename.substr(semicolon_index + 1);

	return devicename;
}

std::wstring compute_device_regpath(const std::wstring &name)
{
	static const std::wstring basepath(L"SYSTEM\\CurrentControlSet\\Enum\\");

	// allocate a temporary string and concatenate the base path plus the name
	auto regpath_buffer = std::make_unique<WCHAR []>(basepath.length() + 1 + name.length());
	wcscpy(regpath_buffer.get(), basepath.c_str());
	WCHAR *chdst = regpath_buffer.get() + basepath.length();

	// convert all # to \ in the name
	for (int i = 4; i < name.length(); i++)
		*chdst++ = (name[i] == '#') ? L'\\' : name[i];
	*chdst = 0;

	// remove the final chunk
	chdst = wcsrchr(regpath_buffer.get(), L'\\');
	if (chdst == nullptr)
		return std::wstring();

	*chdst = 0;

	return std::wstring(regpath_buffer.get());
}

std::wstring improve_name_from_base_path(const std::wstring &regpath, bool *hid)
{
	// now try to open the registry key
	auto device_key = safe_regkey::open(HKEY_LOCAL_MACHINE, regpath);
	if (!device_key)
		return std::wstring();

	// fetch the device description; if it exists, we are finished
	auto regstring = device_key.query_string(L"DeviceDesc");
	if (!regstring.empty())
		return trim_prefix(regstring);

	// if the key name does not contain "HID", it's not going to be in the USB tree; give up
	*hid = regpath.find(L"HID") != std::string::npos;
	return std::wstring();
}

std::wstring improve_name_from_usb_path(const std::wstring &regpath)
{
	static const std::wstring usbbasepath(L"SYSTEM\\CurrentControlSet\\Enum\\USB");

	// extract the expected parent ID from the regpath
	size_t last_slash_index = regpath.find_last_of('\\');
	if (last_slash_index == std::wstring::npos)
		return std::wstring();

	std::wstring parentid = regpath.substr(last_slash_index + 1);

	// open the USB key
	auto usb_key = safe_regkey::open(HKEY_LOCAL_MACHINE, usbbasepath);
	if (!usb_key)
		return std::wstring();

	std::wstring regstring;

	usb_key.foreach_subkey(
			[&regstring, &parentid] (safe_regkey const &subkey)
			{
				subkey.foreach_subkey(
						[&regstring, &parentid] (safe_regkey const &endkey)
						{
							std::wstring endparentid = endkey.query_string(L"ParentIdPrefix");

							// This key doesn't have a ParentIdPrefix
							if (endparentid.empty())
								return true;

							// do we have a match?
							if (parentid.find(endparentid) == 0)
								regstring = endkey.query_string(L"DeviceDesc");

							return regstring.empty();
						});

				return regstring.empty();
			});

	return trim_prefix(regstring);
}


//============================================================
//  rawinput_device_improve_name
//============================================================

std::wstring rawinput_device_improve_name(const std::wstring &name)
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
public:
	rawinput_device(std::string &&name, std::string &&id, input_module &module, HANDLE handle) :
		event_based_device(std::move(name), std::move(id), module),
		m_handle(handle)
	{
	}

	HANDLE device_handle() const { return m_handle; }

	bool reconnect_candidate(std::string_view i) const { return !m_handle && (id() == i); }

	void detach_device()
	{
		assert(m_handle);
		m_handle = nullptr;
		osd_printf_verbose("RawInput: %s [ID %s] disconnected\n", name(), id());
	}

	void attach_device(HANDLE handle)
	{
		assert(!m_handle);
		m_handle = handle;
		osd_printf_verbose("RawInput: %s [ID %s] reconnected\n", name(), id());
	}

private:
	HANDLE m_handle;
};


//============================================================
//  rawinput_keyboard_device
//============================================================

class rawinput_keyboard_device : public rawinput_device
{
public:
	rawinput_keyboard_device(std::string &&name, std::string &&id, input_module &module, HANDLE handle) :
		rawinput_device(std::move(name), std::move(id), module, handle),
		m_pause_pressed(std::chrono::steady_clock::time_point::min()),
		m_e1(0xffff),
		m_keyboard({ { 0 } })
	{
	}

	virtual void reset() override
	{
		rawinput_device::reset();
		m_pause_pressed = std::chrono::steady_clock::time_point::min();
		memset(&m_keyboard, 0, sizeof(m_keyboard));
		m_e1 = 0xffff;
	}

	virtual void poll(bool relative_reset) override
	{
		rawinput_device::poll(relative_reset);
		if (m_keyboard.state[0x80 | 0x45] && (std::chrono::steady_clock::now() > (m_pause_pressed + std::chrono::milliseconds(30))))
			m_keyboard.state[0x80 | 0x45] = 0x00;
	}

	virtual void process_event(RAWINPUT const &rawinput) override
	{
		// determine the full DIK-compatible scancode
		uint8_t scancode;

		// the only thing that uses this is Pause
		if (rawinput.data.keyboard.Flags & RI_KEY_E1)
		{
			m_e1 = rawinput.data.keyboard.MakeCode;
			return;
		}
		else if (0xffff != m_e1)
		{
			auto const e1 = std::exchange(m_e1, 0xffff);
			if (!(rawinput.data.keyboard.Flags & RI_KEY_E0))
			{
				if (((e1 & ~USHORT(0x80)) == 0x1d) && ((rawinput.data.keyboard.MakeCode & ~USHORT(0x80)) == 0x45))
				{
					if (rawinput.data.keyboard.Flags & RI_KEY_BREAK)
						return; // RawInput generates a fake break immediately after the make - ignore it

					m_pause_pressed = std::chrono::steady_clock::now();
					scancode = 0x80 | 0x45;
				}
				else
				{
					return; // no idea
				}
			}
			else
			{
				return; // shouldn't happen, ignore it
			}
		}
		else
		{
			// strip bit 7 of the make code to work around dodgy drivers that set it for key up events
			if (rawinput.data.keyboard.MakeCode & ~USHORT(0xff))
			{
				// won't fit in a byte along with the E0 flag
				return;
			}
			scancode = (rawinput.data.keyboard.MakeCode & 0x7f) | ((rawinput.data.keyboard.Flags & RI_KEY_E0) ? 0x80 : 0x00);

			// fake shift generated with cursor control and Ins/Del for compatibility with very old DOS software
			if (scancode == 0xaa)
				return;
		}

		// set or clear the key
		m_keyboard.state[scancode] = (rawinput.data.keyboard.Flags & RI_KEY_BREAK) ? 0x00 : 0x80;
	}

	virtual void configure(input_device &device) override
	{
		keyboard_trans_table const &table = keyboard_trans_table::instance();

		// FIXME: GetKeyNameTextW is for scan codes from WM_KEYDOWN, which aren't quite the same as DIK_* keycodes
		// in particular, NumLock and Pause are reversed for US-style keyboard systems
		for (unsigned keynum = 0; keynum < MAX_KEYS; keynum++)
		{
			input_item_id itemid = table.map_di_scancode_to_itemid(keynum);
			WCHAR keyname[100];

			// generate the name
			// FIXME: GetKeyNameText gives bogus names for media keys and various other things
			// in many cases it ignores the "extended" bit and returns the key name corresponding to the scan code alone
			LONG lparam = ((keynum & 0x7f) << 16) | ((keynum & 0x80) << 17);
			if ((keynum & 0x7f) == 0x45)
				lparam ^= 0x0100'0000; // horrid hack
			if (GetKeyNameTextW(lparam, keyname, std::size(keyname)) == 0)
				_snwprintf(keyname, std::size(keyname), L"Scan%03d", keynum);
			std::string name = text::from_wstring(keyname);

			// add the item to the device
			device.add_item(
					name,
					util::string_format("SCAN%03d", keynum),
					itemid,
					generic_button_get_state<std::uint8_t>,
					&m_keyboard.state[keynum]);
		}
	}

private:
	std::chrono::steady_clock::time_point m_pause_pressed;
	uint16_t m_e1;
	keyboard_state m_keyboard;
};


//============================================================
//  rawinput_mouse_device
//============================================================

class rawinput_mouse_device : public rawinput_device
{
public:
	rawinput_mouse_device(std::string &&name, std::string &&id, input_module &module, HANDLE handle) :
		rawinput_device(std::move(name), std::move(id), module, handle),
		m_mouse({0}),
		m_x(0),
		m_y(0),
		m_v(0),
		m_h(0)
	{
	}

	virtual void poll(bool relative_reset) override
	{
		rawinput_device::poll(relative_reset);
		if (relative_reset)
		{
			m_mouse.lX = std::exchange(m_x, 0);
			m_mouse.lY = std::exchange(m_y, 0);
			m_mouse.lV = std::exchange(m_v, 0);
			m_mouse.lH = std::exchange(m_h, 0);
		}
	}

	virtual void reset() override
	{
		rawinput_device::reset();
		memset(&m_mouse, 0, sizeof(m_mouse));
		m_x = m_y = m_v = m_h = 0;
	}

	virtual void configure(input_device &device) override
	{
		// populate the axes
		device.add_item(
				"X",
				std::string_view(),
				ITEM_ID_XAXIS,
				generic_axis_get_state<LONG>,
				&m_mouse.lX);
		device.add_item(
				"Y",
				std::string_view(),
				ITEM_ID_YAXIS,
				generic_axis_get_state<LONG>,
				&m_mouse.lY);
		device.add_item(
				"Scroll V",
				std::string_view(),
				ITEM_ID_ZAXIS,
				generic_axis_get_state<LONG>,
				&m_mouse.lV);
		device.add_item(
				"Scroll H",
				std::string_view(),
				ITEM_ID_RZAXIS,
				generic_axis_get_state<LONG>,
				&m_mouse.lH);

		// populate the buttons
		for (int butnum = 0; butnum < 5; butnum++)
		{
			device.add_item(
					default_button_name(butnum),
					std::string_view(),
					input_item_id(ITEM_ID_BUTTON1 + butnum),
					generic_button_get_state<BYTE>,
					&m_mouse.rgbButtons[butnum]);
		}
	}

	virtual void process_event(RAWINPUT const &rawinput) override
	{
		// If this data was intended for a rawinput mouse
		if (rawinput.data.mouse.usFlags == MOUSE_MOVE_RELATIVE)
		{
			m_x += rawinput.data.mouse.lLastX * input_device::RELATIVE_PER_PIXEL;
			m_y += rawinput.data.mouse.lLastY * input_device::RELATIVE_PER_PIXEL;

			// update Z/Rz axes (vertical/horizontal scroll)
			if (rawinput.data.mouse.usButtonFlags & RI_MOUSE_WHEEL)
				m_v += int16_t(rawinput.data.mouse.usButtonData) * input_device::RELATIVE_PER_PIXEL;
			if (rawinput.data.mouse.usButtonFlags & RI_MOUSE_HWHEEL)
				m_h += int16_t(rawinput.data.mouse.usButtonData) * input_device::RELATIVE_PER_PIXEL;

			// update the button states; always update the corresponding mouse buttons
			if (rawinput.data.mouse.usButtonFlags & RI_MOUSE_BUTTON_1_DOWN) m_mouse.rgbButtons[0] = 0x80;
			if (rawinput.data.mouse.usButtonFlags & RI_MOUSE_BUTTON_1_UP)   m_mouse.rgbButtons[0] = 0x00;
			if (rawinput.data.mouse.usButtonFlags & RI_MOUSE_BUTTON_2_DOWN) m_mouse.rgbButtons[1] = 0x80;
			if (rawinput.data.mouse.usButtonFlags & RI_MOUSE_BUTTON_2_UP)   m_mouse.rgbButtons[1] = 0x00;
			if (rawinput.data.mouse.usButtonFlags & RI_MOUSE_BUTTON_3_DOWN) m_mouse.rgbButtons[2] = 0x80;
			if (rawinput.data.mouse.usButtonFlags & RI_MOUSE_BUTTON_3_UP)   m_mouse.rgbButtons[2] = 0x00;
			if (rawinput.data.mouse.usButtonFlags & RI_MOUSE_BUTTON_4_DOWN) m_mouse.rgbButtons[3] = 0x80;
			if (rawinput.data.mouse.usButtonFlags & RI_MOUSE_BUTTON_4_UP)   m_mouse.rgbButtons[3] = 0x00;
			if (rawinput.data.mouse.usButtonFlags & RI_MOUSE_BUTTON_5_DOWN) m_mouse.rgbButtons[4] = 0x80;
			if (rawinput.data.mouse.usButtonFlags & RI_MOUSE_BUTTON_5_UP)   m_mouse.rgbButtons[4] = 0x00;
		}
	}

private:
	mouse_state m_mouse;
	LONG m_x, m_y, m_v, m_h;
};


//============================================================
//  rawinput_lightgun_device
//============================================================

class rawinput_lightgun_device : public rawinput_device
{
public:
	rawinput_lightgun_device(std::string &&name, std::string &&id, input_module &module, HANDLE handle) :
		rawinput_device(std::move(name), std::move(id), module, handle),
		m_lightgun({0}),
		m_v(0),
		m_h(0)
	{
	}

	virtual void poll(bool relative_reset) override
	{
		rawinput_device::poll(relative_reset);
		if (relative_reset)
		{
			m_lightgun.lV = std::exchange(m_v, 0);
			m_lightgun.lH = std::exchange(m_h, 0);
		}
	}

	virtual void reset() override
	{
		rawinput_device::reset();
		memset(&m_lightgun, 0, sizeof(m_lightgun));
		m_v = 0;
		m_h = 0;
	}

	virtual void configure(input_device &device) override
	{
		// populate the axes
		for (int axisnum = 0; axisnum < 2; axisnum++)
		{
			device.add_item(
					default_axis_name[axisnum],
					std::string_view(),
					input_item_id(ITEM_ID_XAXIS + axisnum),
					generic_axis_get_state<LONG>,
					&m_lightgun.lX + axisnum);
		}

		// scroll wheels are always relative if present
		device.add_item(
				"Scroll V",
				std::string_view(),
				ITEM_ID_ADD_RELATIVE1,
				generic_axis_get_state<LONG>,
				&m_lightgun.lV);
		device.add_item(
				"Scroll H",
				std::string_view(),
				ITEM_ID_ADD_RELATIVE2,
				generic_axis_get_state<LONG>,
				&m_lightgun.lH);

		// populate the buttons
		for (int butnum = 0; butnum < 5; butnum++)
		{
			device.add_item(
					default_button_name(butnum),
					std::string_view(),
					input_item_id(ITEM_ID_BUTTON1 + butnum),
					generic_button_get_state<BYTE>,
					&m_lightgun.rgbButtons[butnum]);
		}
	}

	virtual void process_event(RAWINPUT const &rawinput) override
	{
		// If this data was intended for a rawinput lightgun
		if (rawinput.data.mouse.usFlags & MOUSE_MOVE_ABSOLUTE)
		{
			// update the X/Y positions
			m_lightgun.lX = normalize_absolute_axis(rawinput.data.mouse.lLastX, 0, input_device::ABSOLUTE_MAX);
			m_lightgun.lY = normalize_absolute_axis(rawinput.data.mouse.lLastY, 0, input_device::ABSOLUTE_MAX);

			// update Z/Rz axes
			if (rawinput.data.mouse.usButtonFlags & RI_MOUSE_WHEEL)
				m_v += int16_t(rawinput.data.mouse.usButtonData) * input_device::RELATIVE_PER_PIXEL;
			if (rawinput.data.mouse.usButtonFlags & RI_MOUSE_HWHEEL)
				m_h += int16_t(rawinput.data.mouse.usButtonData) * input_device::RELATIVE_PER_PIXEL;

			// update the button states; always update the corresponding mouse buttons
			if (rawinput.data.mouse.usButtonFlags & RI_MOUSE_BUTTON_1_DOWN) m_lightgun.rgbButtons[0] = 0x80;
			if (rawinput.data.mouse.usButtonFlags & RI_MOUSE_BUTTON_1_UP)   m_lightgun.rgbButtons[0] = 0x00;
			if (rawinput.data.mouse.usButtonFlags & RI_MOUSE_BUTTON_2_DOWN) m_lightgun.rgbButtons[1] = 0x80;
			if (rawinput.data.mouse.usButtonFlags & RI_MOUSE_BUTTON_2_UP)   m_lightgun.rgbButtons[1] = 0x00;
			if (rawinput.data.mouse.usButtonFlags & RI_MOUSE_BUTTON_3_DOWN) m_lightgun.rgbButtons[2] = 0x80;
			if (rawinput.data.mouse.usButtonFlags & RI_MOUSE_BUTTON_3_UP)   m_lightgun.rgbButtons[2] = 0x00;
			if (rawinput.data.mouse.usButtonFlags & RI_MOUSE_BUTTON_4_DOWN) m_lightgun.rgbButtons[3] = 0x80;
			if (rawinput.data.mouse.usButtonFlags & RI_MOUSE_BUTTON_4_UP)   m_lightgun.rgbButtons[3] = 0x00;
			if (rawinput.data.mouse.usButtonFlags & RI_MOUSE_BUTTON_5_DOWN) m_lightgun.rgbButtons[4] = 0x80;
			if (rawinput.data.mouse.usButtonFlags & RI_MOUSE_BUTTON_5_UP)   m_lightgun.rgbButtons[4] = 0x00;
		}
	}

private:
	mouse_state m_lightgun;
	LONG m_v, m_h;
};


//============================================================
//  rawinput_module - base class for RawInput modules
//============================================================

class rawinput_module : public wininput_module<rawinput_device>
{
private:
	std::mutex  m_module_lock;

public:
	rawinput_module(const char *type, const char *name) : wininput_module<rawinput_device>(type, name)
	{
	}

	virtual bool probe() override
	{
		return true;
	}

	virtual void input_init(running_machine &machine) override
	{
		wininput_module<rawinput_device>::input_init(machine);

		// get initial number of devices
		UINT device_count = 0;
		if (GetRawInputDeviceList(nullptr, &device_count, sizeof(RAWINPUTDEVICELIST)) != 0)
		{
			osd_printf_error("Error getting initial number of RawInput devices.\n");
			return;
		}
		if (!device_count)
			return;

		std::unique_ptr<RAWINPUTDEVICELIST []> rawinput_devices;
		UINT retrieved;
		do
		{
			rawinput_devices.reset(new (std::nothrow) RAWINPUTDEVICELIST [device_count]);
			if (!rawinput_devices)
			{
				osd_printf_error("Error allocating buffer for RawInput device list.\n");
				return;
			}
			retrieved = GetRawInputDeviceList(rawinput_devices.get(), &device_count, sizeof(RAWINPUTDEVICELIST));
		}
		while ((UINT(-1) == retrieved) && (GetLastError() == ERROR_INSUFFICIENT_BUFFER));
		if (UINT(-1) == retrieved)
		{
			osd_printf_error("Error listing RawInput devices.\n");
			return;
		}

		// iterate backwards through devices; new devices are added at the head
		for (int devnum = retrieved - 1; devnum >= 0; devnum--)
			add_rawinput_device(rawinput_devices[devnum]);

		// If we added no devices, no need to register for notifications
		if (devicelist().empty())
			return;

		// finally, register to receive raw input WM_INPUT messages if we found devices
		RAWINPUTDEVICE registration;
		registration.usUsagePage = usagepage();
		registration.usUsage = usage();
		registration.dwFlags = RIDEV_DEVNOTIFY | RIDEV_INPUTSINK;
		registration.hwndTarget = dynamic_cast<win_window_info &>(*osd_common_t::window_list().front()).platform_window();

		// register the device
		RegisterRawInputDevices(&registration, 1, sizeof(registration));
	}

protected:
	virtual void add_rawinput_device(RAWINPUTDEVICELIST const &device) = 0;
	virtual USHORT usagepage() = 0;
	virtual USHORT usage() = 0;

	template<class TDevice>
	TDevice *create_rawinput_device(input_device_class deviceclass, RAWINPUTDEVICELIST const &rawinputdevice)
	{
		// determine the length of the device name, allocate it, and fetch it if not nameless
		UINT name_length = 0;
		if (GetRawInputDeviceInfoW(rawinputdevice.hDevice, RIDI_DEVICENAME, nullptr, &name_length) != 0)
			return nullptr;

		std::unique_ptr<WCHAR []> tname = std::make_unique<WCHAR []>(name_length + 1);
		if (name_length > 1 && GetRawInputDeviceInfoW(rawinputdevice.hDevice, RIDI_DEVICENAME, tname.get(), &name_length) == UINT(-1))
			return nullptr;

		// if this is an RDP name, skip it
		if (wcsstr(tname.get(), L"Root#RDP_") != nullptr)
			return nullptr;

		// improve the name
		std::string utf8_name = text::from_wstring(rawinput_device_improve_name(tname.get()));

		// set device ID to raw input name
		std::string utf8_id = text::from_wstring(tname.get());

		tname.reset();

		// allocate a device
		return &create_device<TDevice>(
				deviceclass,
				std::move(utf8_name),
				std::move(utf8_id),
				rawinputdevice.hDevice);
	}

	virtual bool handle_input_event(input_event eventid, void const *eventdata) override
	{
		switch (eventid)
		{
		// handle raw input data
		case INPUT_EVENT_RAWINPUT:
			{
				HRAWINPUT const rawinputdevice = *reinterpret_cast<HRAWINPUT const *>(eventdata);

				union { RAWINPUT r; BYTE b[4096]; } small_buffer;
				std::unique_ptr<BYTE []> larger_buffer;
				LPVOID data = &small_buffer;
				UINT size;

				// determine the size of data buffer we need
				if (GetRawInputData(rawinputdevice, RID_INPUT, nullptr, &size, sizeof(RAWINPUTHEADER)) != 0)
					return false;

				// if necessary, allocate a temporary buffer and fetch the data
				if (size > sizeof(small_buffer))
				{
					larger_buffer.reset(new (std::align_val_t(alignof(RAWINPUT)), std::nothrow) BYTE [size]);
					data = larger_buffer.get();
					if (!data)
						return false;
				}

				// fetch the data and process the appropriate message types
				UINT result = GetRawInputData(rawinputdevice, RID_INPUT, data, &size, sizeof(RAWINPUTHEADER));
				if (UINT(-1) == result)
				{
					return false;
				}
				else if (result)
				{
					std::lock_guard<std::mutex> scope_lock(m_module_lock);

					auto *const input = reinterpret_cast<RAWINPUT const *>(data);
					if (!input->header.hDevice)
						return false;

					// find the device in the list and update
					auto target_device = std::find_if(
							devicelist().begin(),
							devicelist().end(),
							[input] (auto const &device)
							{
								return input->header.hDevice == device->device_handle();
							});
					if (devicelist().end() == target_device)
						return false;

					(*target_device)->queue_events(input, 1);
					return true;
				}
			}
			break;

		case INPUT_EVENT_ARRIVAL:
			{
				HRAWINPUT const rawinputdevice = *reinterpret_cast<HRAWINPUT const *>(eventdata);

				// determine the length of the device name, allocate it, and fetch it if not nameless
				UINT name_length = 0;
				if (GetRawInputDeviceInfoW(rawinputdevice, RIDI_DEVICENAME, nullptr, &name_length) != 0)
					return false;

				std::unique_ptr<WCHAR []> tname = std::make_unique<WCHAR []>(name_length + 1);
				if (name_length > 1 && GetRawInputDeviceInfoW(rawinputdevice, RIDI_DEVICENAME, tname.get(), &name_length) == UINT(-1))
					return false;
				std::string utf8_id = text::from_wstring(tname.get());
				tname.reset();

				std::lock_guard<std::mutex> scope_lock(m_module_lock);

				// find the device in the list and update
				auto target_device = std::find_if(
						devicelist().begin(),
						devicelist().end(),
						[&utf8_id] (auto const &device)
						{
							return device->reconnect_candidate(utf8_id);
						});
				if (devicelist().end() == target_device)
					return false;

				(*target_device)->attach_device(rawinputdevice);
				return true;
			}
			break;

		case INPUT_EVENT_REMOVAL:
			{
				HRAWINPUT const rawinputdevice = *reinterpret_cast<HRAWINPUT const *>(eventdata);

				std::lock_guard<std::mutex> scope_lock(m_module_lock);

				// find the device in the list and update
				auto target_device = std::find_if(
						devicelist().begin(),
						devicelist().end(),
						[rawinputdevice] (auto const &device)
						{
							return rawinputdevice == device->device_handle();
						});

				if (devicelist().end() == target_device)
					return false;

				(*target_device)->reset();
				(*target_device)->detach_device();
				return true;
			}
			break;

		default:
			break;
		}

		// must have been unhandled
		return false;
	}
};


//============================================================
//  keyboard_input_rawinput - RawInput keyboard module
//============================================================

class keyboard_input_rawinput : public rawinput_module
{
public:
	keyboard_input_rawinput() : rawinput_module(OSD_KEYBOARDINPUT_PROVIDER, "rawinput")
	{
	}

protected:
	virtual USHORT usagepage() override { return 1; }
	virtual USHORT usage() override { return 6; }

	virtual void add_rawinput_device(RAWINPUTDEVICELIST const &device) override
	{
		// make sure this is a keyboard
		if (device.dwType != RIM_TYPEKEYBOARD)
			return;

		// allocate and link in a new device
		create_rawinput_device<rawinput_keyboard_device>(DEVICE_CLASS_KEYBOARD, device);
	}
};


//============================================================
//  mouse_input_rawinput - RawInput mouse module
//============================================================

class mouse_input_rawinput : public rawinput_module
{
public:
	mouse_input_rawinput() : rawinput_module(OSD_MOUSEINPUT_PROVIDER, "rawinput")
	{
	}

protected:
	virtual USHORT usagepage() override { return 1; }
	virtual USHORT usage() override { return 2; }

	virtual void add_rawinput_device(RAWINPUTDEVICELIST const &device) override
	{
		// make sure this is a mouse
		if (device.dwType != RIM_TYPEMOUSE)
			return;

		// allocate and link in a new device
		create_rawinput_device<rawinput_mouse_device>(DEVICE_CLASS_MOUSE, device);
	}
};


//============================================================
//  lightgun_input_rawinput - RawInput lightgun module
//============================================================

class lightgun_input_rawinput : public rawinput_module
{
public:
	lightgun_input_rawinput() : rawinput_module(OSD_LIGHTGUNINPUT_PROVIDER, "rawinput")
	{
	}

protected:
	virtual USHORT usagepage() override { return 1; }
	virtual USHORT usage() override { return 2; }

	virtual void add_rawinput_device(RAWINPUTDEVICELIST const &device) override
	{
		// make sure this is a mouse
		if (device.dwType != RIM_TYPEMOUSE)
			return;

		// allocate and link in a new device
		create_rawinput_device<rawinput_lightgun_device>(DEVICE_CLASS_LIGHTGUN, device);
	}
};

} // anonymous namespace

} // namespace osd

#else // defined(OSD_WINDOWS)

#include "input_module.h"

namespace osd {

namespace {

MODULE_NOT_SUPPORTED(keyboard_input_rawinput, OSD_KEYBOARDINPUT_PROVIDER, "rawinput")
MODULE_NOT_SUPPORTED(mouse_input_rawinput, OSD_MOUSEINPUT_PROVIDER, "rawinput")
MODULE_NOT_SUPPORTED(lightgun_input_rawinput, OSD_LIGHTGUNINPUT_PROVIDER, "rawinput")

} // anonymous namespace

} // namespace osd

#endif // defined(OSD_WINDOWS)

MODULE_DEFINITION(KEYBOARDINPUT_RAWINPUT, osd::keyboard_input_rawinput)
MODULE_DEFINITION(MOUSEINPUT_RAWINPUT, osd::mouse_input_rawinput)
MODULE_DEFINITION(LIGHTGUNINPUT_RAWINPUT, osd::lightgun_input_rawinput)
