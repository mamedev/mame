// license:BSD-3-Clause
// copyright-holders:Brad Hughes
/*
* input_rawinput.cpp
*
*/

#include "input_module.h"
#include "modules/osdmodule.h"

#if defined(OSD_WINDOWS) || defined(SDLMAME_WIN32)

// standard windows headers
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winioctl.h>
#include <tchar.h>
#undef interface

#include <mutex>

// MAME headers
#include "emu.h"
#include "osdepend.h"
#include "ui/ui.h"

// MAMEOS headers
#include "winmain.h"
#include "window.h"
#include "../../windows/input.h"

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
typedef /*WINUSERAPI*/ INT(WINAPI *get_rawinput_device_list_ptr)(OUT PRAWINPUTDEVICELIST pRawInputDeviceList, IN OUT PINT puiNumDevices, IN UINT cbSize);
typedef /*WINUSERAPI*/ INT(WINAPI *get_rawinput_data_ptr)(IN HRAWINPUT hRawInput, IN UINT uiCommand, OUT LPVOID pData, IN OUT PINT pcbSize, IN UINT cbSizeHeader);
typedef /*WINUSERAPI*/ INT(WINAPI *get_rawinput_device_info_ptr)(IN HANDLE hDevice, IN UINT uiCommand, OUT LPVOID pData, IN OUT PINT pcbSize);
typedef /*WINUSERAPI*/ BOOL(WINAPI *register_rawinput_devices_ptr)(IN PCRAWINPUTDEVICE pRawInputDevices, IN UINT uiNumDevices, IN UINT cbSize);

//============================================================
//  reg_query_string
//============================================================

static TCHAR *reg_query_string(HKEY key, const TCHAR *path)
{
	TCHAR *buffer;
	DWORD datalen;
	LONG result;

	// first query to get the length
	result = RegQueryValueEx(key, path, NULL, NULL, NULL, &datalen);
	if (result != ERROR_SUCCESS)
		return NULL;

	// allocate a buffer
	buffer = global_alloc_array(TCHAR, datalen + sizeof(*buffer));
	buffer[datalen / sizeof(*buffer)] = 0;

	// now get the actual data
	result = RegQueryValueEx(key, path, NULL, NULL, (LPBYTE)buffer, &datalen);
	if (result == ERROR_SUCCESS)
		return buffer;

	// otherwise return a NULL buffer
	global_free_array(buffer);
	return NULL;
}

//============================================================
//  rawinput_device_improve_name
//============================================================

static TCHAR *rawinput_device_improve_name(TCHAR *name)
{
	static const TCHAR usbbasepath[] = TEXT("SYSTEM\\CurrentControlSet\\Enum\\USB");
	static const TCHAR basepath[] = TEXT("SYSTEM\\CurrentControlSet\\Enum\\");
	TCHAR *regstring = NULL;
	TCHAR *parentid = NULL;
	TCHAR *regpath = NULL;
	const TCHAR *chsrc;
	HKEY regkey = NULL;
	int usbindex;
	TCHAR *chdst;
	LONG result;

	// The RAW name received is formatted as:
	//   \??\type-id#hardware-id#instance-id#{DeviceClasses-id}
	// XP starts with "\??\"
	// Vista64 starts with "\\?\"

	// ensure the name is something we can handle
	if (_tcsncmp(name, TEXT("\\\\?\\"), 4) != 0 && _tcsncmp(name, TEXT("\\??\\"), 4) != 0)
		return name;

	// allocate a temporary string and concatenate the base path plus the name
	regpath = global_alloc_array(TCHAR, _tcslen(basepath) + 1 + _tcslen(name));
	_tcscpy(regpath, basepath);
	chdst = regpath + _tcslen(regpath);

	// convert all # to \ in the name
	for (chsrc = name + 4; *chsrc != 0; chsrc++)
		*chdst++ = (*chsrc == '#') ? '\\' : *chsrc;
	*chdst = 0;

	// remove the final chunk
	chdst = _tcsrchr(regpath, '\\');
	if (chdst == NULL)
		goto exit;
	*chdst = 0;

	// now try to open the registry key
	result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, regpath, 0, KEY_READ, &regkey);
	if (result != ERROR_SUCCESS)
		goto exit;

	// fetch the device description; if it exists, we are finished
	regstring = reg_query_string(regkey, TEXT("DeviceDesc"));
	if (regstring != NULL)
		goto convert;

	// close this key
	RegCloseKey(regkey);
	regkey = NULL;

	// if the key name does not contain "HID", it's not going to be in the USB tree; give up
	if (_tcsstr(regpath, TEXT("HID")) == NULL)
		goto exit;

	// extract the expected parent ID from the regpath
	parentid = _tcsrchr(regpath, '\\');
	if (parentid == NULL)
		goto exit;
	parentid++;

	// open the USB key
	result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, usbbasepath, 0, KEY_READ, &regkey);
	if (result != ERROR_SUCCESS)
		goto exit;

	// enumerate the USB key
	for (usbindex = 0; result == ERROR_SUCCESS && regstring == NULL; usbindex++)
	{
		TCHAR keyname[MAX_PATH];
		DWORD namelen;

		// get the next enumerated subkey and scan it
		namelen = ARRAY_LENGTH(keyname) - 1;
		result = RegEnumKeyEx(regkey, usbindex, keyname, &namelen, NULL, NULL, NULL, NULL);
		if (result == ERROR_SUCCESS)
		{
			LONG subresult;
			int subindex;
			HKEY subkey;

			// open the subkey
			subresult = RegOpenKeyEx(regkey, keyname, 0, KEY_READ, &subkey);
			if (subresult != ERROR_SUCCESS)
				continue;

			// enumerate the subkey
			for (subindex = 0; subresult == ERROR_SUCCESS && regstring == NULL; subindex++)
			{
				// get the next enumerated subkey and scan it
				namelen = ARRAY_LENGTH(keyname) - 1;
				subresult = RegEnumKeyEx(subkey, subindex, keyname, &namelen, NULL, NULL, NULL, NULL);
				if (subresult == ERROR_SUCCESS)
				{
					TCHAR *endparentid;
					LONG endresult;
					HKEY endkey;

					// open this final key
					endresult = RegOpenKeyEx(subkey, keyname, 0, KEY_READ, &endkey);
					if (endresult != ERROR_SUCCESS)
						continue;

					// do we have a match?
					endparentid = reg_query_string(endkey, TEXT("ParentIdPrefix"));
					if (endparentid != NULL && _tcsncmp(parentid, endparentid, _tcslen(endparentid)) == 0)
						regstring = reg_query_string(endkey, TEXT("DeviceDesc"));

					// free memory and close the key
					if (endparentid != NULL)
						global_free_array(endparentid);
					RegCloseKey(endkey);
				}
			}

			// close the subkey
			RegCloseKey(subkey);
		}
	}

	// if we didn't find anything, go to the exit
	if (regstring == NULL)
		goto exit;

convert:
	// replace the name with the nicer one
	global_free_array(name);

	// remove anything prior to the final semicolon
	chsrc = _tcsrchr(regstring, ';');
	if (chsrc != NULL)
		chsrc++;
	else
		chsrc = regstring;
	name = global_alloc_array(TCHAR, _tcslen(chsrc) + 1);
	_tcscpy(name, chsrc);

exit:
	if (regstring != NULL)
		global_free_array(regstring);
	if (regpath != NULL)
		global_free_array(regpath);
	if (regkey != NULL)
		RegCloseKey(regkey);

	return name;
}

//============================================================
//  rawinput_device class
//============================================================

class rawinput_device
{
private:
	HANDLE  m_handle;
public:

	rawinput_device()
	{
	}

	HANDLE device_handle() { return m_handle; }
	void set_handle(HANDLE handle) { m_handle = handle; }

	virtual void update(LPVOID data) = 0;
};

//============================================================
//  rawinput_keyboard_device
//============================================================

class rawinput_keyboard_device : public keyboard_device, public rawinput_device
{
public:
	rawinput_keyboard_device(running_machine& machine, const TCHAR* name, input_module& module)
		: keyboard_device(machine, name, module)
	{
	}

	void update(LPVOID data) override
	{
		RAWKEYBOARD *raw = (RAWKEYBOARD*)data;
		
		// determine the full DIK-compatible scancode
		UINT8 scancode = (raw->MakeCode & 0x7f) | ((raw->Flags & RI_KEY_E0) ? 0x80 : 0x00);

		// scancode 0xaa is a special shift code we need to ignore
		if (scancode == 0xaa)
			return;

		// set or clear the key
		keyboard.state[scancode] = (raw->Flags & RI_KEY_BREAK) ? 0x00 : 0x80;
	}
};

//============================================================
//  rawinput_mouse_device
//============================================================

struct rawinput_mouse_state
{
	int raw_x;
	int raw_y;
	int raw_z;
};

class rawinput_mouse_device : public mouse_device, public rawinput_device
{
private:
	std::mutex  m_device_lock;
public:
	rawinput_mouse_state raw_mouse;

	rawinput_mouse_device(running_machine& machine, const TCHAR* name, input_module& module)
		: mouse_device(machine, name, module), raw_mouse({0})
	{
	}

	void poll() override
	{
		downcast<wininput_module &>(module()).poll_if_necessary(machine());

		std::lock_guard<std::mutex> scope_lock(m_device_lock);

		// copy the accumulated raw state to the actual state
		mouse.lX = raw_mouse.raw_x;
		mouse.lY = raw_mouse.raw_y;
		mouse.lZ = raw_mouse.raw_z;
		raw_mouse.raw_x = 0;
		raw_mouse.raw_y = 0;
		raw_mouse.raw_z = 0;
	}

	void update(LPVOID void_data) override
	{
		std::lock_guard<std::mutex> scope_lock(m_device_lock);

		RAWMOUSE *data = (RAWMOUSE*)void_data;
		if (!(data->usFlags & MOUSE_MOVE_ABSOLUTE))
		{
			raw_mouse.raw_x += data->lLastX * INPUT_RELATIVE_PER_PIXEL;
			raw_mouse.raw_y += data->lLastY * INPUT_RELATIVE_PER_PIXEL;

			// update zaxis
			if (data->usButtonFlags & RI_MOUSE_WHEEL)
				raw_mouse.raw_z += (INT16)data->usButtonData * INPUT_RELATIVE_PER_PIXEL;
		}

		// update the button states; always update the corresponding mouse buttons
		if (data->usButtonFlags & RI_MOUSE_BUTTON_1_DOWN) mouse.rgbButtons[0] = 0x80;
		if (data->usButtonFlags & RI_MOUSE_BUTTON_1_UP)   mouse.rgbButtons[0] = 0x00;
		if (data->usButtonFlags & RI_MOUSE_BUTTON_2_DOWN) mouse.rgbButtons[1] = 0x80;
		if (data->usButtonFlags & RI_MOUSE_BUTTON_2_UP)   mouse.rgbButtons[1] = 0x00;
		if (data->usButtonFlags & RI_MOUSE_BUTTON_3_DOWN) mouse.rgbButtons[2] = 0x80;
		if (data->usButtonFlags & RI_MOUSE_BUTTON_3_UP)   mouse.rgbButtons[2] = 0x00;
		if (data->usButtonFlags & RI_MOUSE_BUTTON_4_DOWN) mouse.rgbButtons[3] = 0x80;
		if (data->usButtonFlags & RI_MOUSE_BUTTON_4_UP)   mouse.rgbButtons[3] = 0x00;
		if (data->usButtonFlags & RI_MOUSE_BUTTON_5_DOWN) mouse.rgbButtons[4] = 0x80;
		if (data->usButtonFlags & RI_MOUSE_BUTTON_5_UP)   mouse.rgbButtons[4] = 0x00;
	}
};

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
			get_rawinput_device_list(nullptr),
			get_rawinput_data(nullptr),
			get_rawinput_device_info(nullptr),
			register_rawinput_devices(nullptr)
	{
	}

	void input_init(running_machine &machine) override
	{
		// get the number of devices, allocate a device list, and fetch it
		int device_count = 0;
		if ((*get_rawinput_device_list)(NULL, &device_count, sizeof(RAWINPUTDEVICELIST)) != 0)
			return;

		if (device_count == 0)
			return;

		auto rawinput_devices = std::make_unique<RAWINPUTDEVICELIST[]>(device_count);
		if ((*get_rawinput_device_list)(rawinput_devices.get(), &device_count, sizeof(RAWINPUTDEVICELIST)) == -1)
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
		(*register_rawinput_devices)(&registration, 1, sizeof(registration));
	}

protected:
	virtual void add_rawinput_device(running_machine& machine, RAWINPUTDEVICELIST * device) = 0;
	virtual USHORT usagepage() = 0;
	virtual USHORT usage() = 0;

	int init_internal() override
	{
		HMODULE user32;

		// look in user32 for the raw input APIs
		user32 = LoadLibrary(TEXT("user32.dll"));
		if (user32 == NULL)
			return 1;

		// look up the entry points
		register_rawinput_devices = (register_rawinput_devices_ptr)GetProcAddress(user32, "RegisterRawInputDevices");
		get_rawinput_device_list = (get_rawinput_device_list_ptr)GetProcAddress(user32, "GetRawInputDeviceList");
		get_rawinput_device_info = (get_rawinput_device_info_ptr)GetProcAddress(user32, "GetRawInputDeviceInfo" UNICODE_SUFFIX);
		get_rawinput_data = (get_rawinput_data_ptr)GetProcAddress(user32, "GetRawInputData");
		if (register_rawinput_devices == NULL || get_rawinput_device_list == NULL || get_rawinput_device_info == NULL || get_rawinput_data == NULL)
			return 1;

		osd_printf_verbose("RawInput: APIs detected\n");

		return 0;
	}

	template<class TDevice>
	TDevice* create_rawinput_device(running_machine &machine, PRAWINPUTDEVICELIST rawinputdevice)
	{
		TDevice* devinfo = nullptr;
		INT name_length = 0;
		// determine the length of the device name, allocate it, and fetch it if not nameless
		if ((*get_rawinput_device_info)(rawinputdevice->hDevice, RIDI_DEVICENAME, NULL, &name_length) != 0)
			return nullptr;

		std::unique_ptr<TCHAR[]> tname = std::make_unique<TCHAR[]>(name_length + 1);
		if (name_length > 1 && (*get_rawinput_device_info)(rawinputdevice->hDevice, RIDI_DEVICENAME, tname.get(), &name_length) == -1)
			return nullptr;

		// if this is an RDP name, skip it
		if (_tcsstr(tname.get(), TEXT("Root#RDP_")) != NULL)
			return nullptr;

		// improve the name and then allocate a device
		tname = std::unique_ptr<TCHAR[]>(rawinput_device_improve_name(tname.release()));

		devinfo = devicelist()->create_device<TDevice>(machine, tname.get(), *this);

		// Add the handle
		devinfo->set_handle(rawinputdevice->hDevice);

		return devinfo;
	}

	BOOL handle_input_event(input_event eventid, void* eventdata) override
	{
		// Only handle raw input data
		if (!input_enabled() || eventid != INPUT_EVENT_RAWINPUT)
			return FALSE;

		HRAWINPUT rawinputdevice = *(HRAWINPUT*)eventdata;

		BYTE small_buffer[4096];
		std::unique_ptr<BYTE[]> larger_buffer;
		LPBYTE data = small_buffer;
		BOOL result = FALSE;
		int size;

		// ignore if not enabled
		if (!input_enabled())
			return FALSE;

		// determine the size of databuffer we need
		if ((*get_rawinput_data)(rawinputdevice, RID_INPUT, NULL, &size, sizeof(RAWINPUTHEADER)) != 0)
			return FALSE;

		// if necessary, allocate a temporary buffer and fetch the data
		if (size > sizeof(small_buffer))
		{
			larger_buffer = std::make_unique<BYTE[]>(size);
			data = larger_buffer.get();
			if (data == NULL)
				return FALSE;
		}

		// fetch the data and process the appropriate message types
		result = (*get_rawinput_data)((HRAWINPUT)rawinputdevice, RID_INPUT, data, &size, sizeof(RAWINPUTHEADER));
		if (result)
		{
			std::lock_guard<std::mutex> scope_lock(m_module_lock);

			rawinput_device *devinfo;
			// find the device in the list and update
			for (int i = 0; i < devicelist()->size(); i++)
			{
				devinfo = dynamic_cast<rawinput_device*>(devicelist()->at(i));
				RAWINPUT *input = reinterpret_cast<RAWINPUT*>(data);
				if (input->header.hDevice == devinfo->device_handle())
				{
					devinfo->update(&input->data);
					result = TRUE;
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
	USHORT usagepage() { return 1; }
	USHORT usage() { return 6; }
	void add_rawinput_device(running_machine& machine, RAWINPUTDEVICELIST * device) override
	{
		// make sure this is a keyboard
		if (device->dwType != RIM_TYPEKEYBOARD)
			return;

		// allocate and link in a new device
		rawinput_keyboard_device *devinfo = create_rawinput_device<rawinput_keyboard_device>(machine, device);
		if (devinfo == NULL)
			return;

		// populate it
		for (int keynum = 0; keynum < MAX_KEYS; keynum++)
		{
			input_item_id itemid = keyboard_trans_table::instance().map_scancode_to_itemid(keynum);
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
	USHORT usagepage() { return 1; }
	USHORT usage() { return 2; }
	void add_rawinput_device(running_machine& machine, RAWINPUTDEVICELIST * device) override
	{
		// make sure this is a mouse
		if (device->dwType != RIM_TYPEMOUSE)
			return;

		// allocate and link in a new device
		rawinput_mouse_device *devinfo = create_rawinput_device<rawinput_mouse_device>(machine, device);
		if (devinfo == NULL)
			return;

		// populate the axes
		for (int axisnum = 0; axisnum < 3; axisnum++)
		{
			char *name = utf8_from_tstring(default_axis_name[axisnum]);
			devinfo->device()->add_item(name, (input_item_id)(ITEM_ID_XAXIS + axisnum), generic_axis_get_state, &devinfo->mouse.lX + axisnum);
			osd_free(name);
		}

		// populate the buttons
		for (int butnum = 0; butnum < 5; butnum++)
		{
			char *name = utf8_from_tstring(default_button_name(butnum));
			devinfo->device()->add_item(name, (input_item_id)(ITEM_ID_BUTTON1 + butnum), generic_button_get_state, &devinfo->mouse.rgbButtons[butnum]);
			osd_free(name);
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