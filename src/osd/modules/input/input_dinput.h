// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Brad Hughes
//============================================================
//
//  input_dinput.h - Windows DirectInput support
//
//============================================================
#ifndef MAME_OSD_INPUT_INPUT_DINPUT_H
#define MAME_OSD_INPUT_INPUT_DINPUT_H

#pragma once

#include "input_windows.h"

#include "modules/lib/osdlib.h"
#include "modules/lib/osdobj_common.h"

#include "window.h"

#include "strconv.h"

#include <dinput.h>
#include <windows.h>
#include <wrl/client.h>


//============================================================
//  dinput_device - base directinput device
//============================================================

class device_enum_interface
{
public:
	struct dinput_callback_context
	{
		device_enum_interface *     self;
		void *                      state;
	};

	virtual ~device_enum_interface()
	{
	}

	static BOOL CALLBACK enum_callback(LPCDIDEVICEINSTANCE instance, LPVOID ref)
	{
		auto context = static_cast<dinput_callback_context*>(ref);
		return context->self->device_enum_callback(instance, context->state);
	}

	virtual BOOL device_enum_callback(LPCDIDEVICEINSTANCE instance, LPVOID ref) = 0;
};

// Typedef for dynamically loaded function
typedef HRESULT (WINAPI *dinput_create_fn)(HINSTANCE, DWORD, LPDIRECTINPUT8 *, LPUNKNOWN);

enum class dinput_cooperative_level
{
	FOREGROUND,
	BACKGROUND
};

class dinput_api_helper
{
private:
	Microsoft::WRL::ComPtr<IDirectInput8> m_dinput;
	osd::dynamic_module::ptr              m_dinput_dll;

public:
	dinput_api_helper();
	virtual ~dinput_api_helper();
	int initialize();

	template <class TDevice>
	TDevice *create_device(
			running_machine &machine,
			input_module_base &module,
			LPCDIDEVICEINSTANCE instance,
			LPCDIDATAFORMAT format1,
			LPCDIDATAFORMAT format2,
			dinput_cooperative_level cooperative_level)
	{
		HRESULT result;
		std::shared_ptr<win_window_info> window;
		HWND hwnd;

		// convert instance name to utf8
		std::string utf8_instance_name = osd::text::from_tstring(instance->tszInstanceName);

		// set device id to name + product unique identifier + instance unique identifier
		std::string utf8_instance_id = utf8_instance_name + " product_" + guid_to_string(instance->guidProduct) + " instance_" + guid_to_string(instance->guidInstance);

		// allocate memory for the device object
		TDevice &devinfo = module.devicelist().create_device<TDevice>(machine, std::move(utf8_instance_name), std::move(utf8_instance_id), module);

		// attempt to create a device
		result = m_dinput->CreateDevice(instance->guidInstance, devinfo.dinput.device.GetAddressOf(), nullptr);
		if (result != DI_OK)
			goto error;

		// get the caps
		devinfo.dinput.caps.dwSize = sizeof(devinfo.dinput.caps);
		result = devinfo.dinput.device->GetCapabilities(&devinfo.dinput.caps);
		if (result != DI_OK)
			goto error;

		// attempt to set the data format
		devinfo.dinput.format = format1;
		result = devinfo.dinput.device->SetDataFormat(devinfo.dinput.format);
		if (result != DI_OK)
		{
			// use the secondary format if available
			if (format2)
			{
				devinfo.dinput.format = format2;
				result = devinfo.dinput.device->SetDataFormat(devinfo.dinput.format);
			}
			if (result != DI_OK)
				goto error;
		}

		// default window to the first window in the list
		window = std::static_pointer_cast<win_window_info>(osd_common_t::s_window_list.front());
		DWORD di_cooperative_level;
		if (window->attached_mode())
		{
			// in attached mode we have to ignore the caller and hook up to the desktop window
			hwnd = GetDesktopWindow();
			di_cooperative_level = DISCL_BACKGROUND | DISCL_NONEXCLUSIVE;
		}
		else
		{
			hwnd = window->platform_window();
			switch (cooperative_level)
			{
			case dinput_cooperative_level::BACKGROUND:
				di_cooperative_level = DISCL_BACKGROUND | DISCL_NONEXCLUSIVE;
				break;
			case dinput_cooperative_level::FOREGROUND:
				di_cooperative_level = DISCL_FOREGROUND | DISCL_NONEXCLUSIVE;
				break;
			default:
				throw false;
			}
		}

		// set the cooperative level
		result = devinfo.dinput.device->SetCooperativeLevel(hwnd, di_cooperative_level);
		if (result != DI_OK)
			goto error;

		return &devinfo;

	error:
		module.devicelist().free_device(devinfo);
		return nullptr;
	}

	HRESULT enum_attached_devices(int devclass, device_enum_interface *enumerate_interface, void *state) const;

	static std::string guid_to_string(const GUID& guid)
	{
		// Size of a GUID string with dashes plus null terminator
		char guid_string[37];

		snprintf(
			guid_string, std::size(guid_string),
			"%08lx-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
			guid.Data1, guid.Data2, guid.Data3,
			guid.Data4[0], guid.Data4[1], guid.Data4[2],
			guid.Data4[3], guid.Data4[4], guid.Data4[5],
			guid.Data4[6], guid.Data4[7]);

		return guid_string;
	}
};

class dinput_device : public device_info
{
public:
	// DirectInput-specific information about a device
	struct dinput_api_state
	{
		Microsoft::WRL::ComPtr<IDirectInputDevice8>  device;
		DIDEVCAPS                                    caps;
		LPCDIDATAFORMAT                              format;
	};

	dinput_api_state dinput;

	dinput_device(running_machine &machine, std::string &&name, std::string &&id, input_device_class deviceclass, input_module &module);
	virtual ~dinput_device();

protected:
	HRESULT poll_dinput(LPVOID pState) const;
};

class dinput_keyboard_device : public dinput_device
{
private:
	std::mutex m_device_lock;

public:
	keyboard_state  keyboard;

	dinput_keyboard_device(running_machine &machine, std::string &&name, std::string &&id, input_module &module);

	void poll() override;
	void reset() override;
};

class dinput_mouse_device : public dinput_device
{
public:
	mouse_state mouse;

public:
	dinput_mouse_device(running_machine &machine, std::string &&name, std::string &&id, input_module &module);
	void poll() override;
	void reset() override;
};

// state information for a joystick; DirectInput state must be first element
struct dinput_joystick_state
{
	DIJOYSTATE              state;
	LONG                    rangemin[8];
	LONG                    rangemax[8];
};

class dinput_joystick_device : public dinput_device
{
public:
	dinput_joystick_state   joystick;
public:
	dinput_joystick_device(running_machine &machine, std::string &&name, std::string &&id, input_module &module);
	void reset() override;
	void poll() override;
	int configure();
};

#endif // MAME_OSD_INPUT_INPUT_DINPUT_H
