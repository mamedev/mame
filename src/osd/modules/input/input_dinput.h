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

#include "input_wincommon.h"

#include "modules/lib/osdlib.h"
#include "modules/lib/osdobj_common.h"

#include "window.h"

#include "strconv.h"

#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <utility>

#include <dinput.h>
#include <windows.h>
#include <wrl/client.h>


namespace osd {

//============================================================
//  dinput_device - base directinput device
//============================================================

class device_enum_interface
{
public:
	virtual ~device_enum_interface() = default;

	virtual BOOL device_enum_callback(LPCDIDEVICEINSTANCE instance) = 0;
};

enum class dinput_cooperative_level
{
	FOREGROUND,
	BACKGROUND
};

class dinput_api_helper
{
public:
	dinput_api_helper();
	~dinput_api_helper();

	int initialize();

	template <typename TDevice, typename TCallback>
	std::unique_ptr<TDevice> create_device(
			input_module_base &module,
			LPCDIDEVICEINSTANCE instance,
			LPCDIDATAFORMAT format1,
			LPCDIDATAFORMAT format2,
			dinput_cooperative_level cooperative_level,
			TCallback &&callback)
	{
		auto [device, format] = open_device(instance, format1, format2, cooperative_level);
		if (!device)
			return nullptr;

		// get the capabilities
		DIDEVCAPS caps;
		caps.dwSize = sizeof(caps);
		HRESULT const result = device->GetCapabilities(&caps);
		if (result != DI_OK)
			return nullptr;

		if (!callback(device, format))
			return nullptr;

		// convert instance name to UTF-8
		std::string utf8_instance_name = text::from_tstring(instance->tszInstanceName);

		// set device id to name + product unique identifier + instance unique identifier
		std::string utf8_instance_id = utf8_instance_name + " product_" + guid_to_string(instance->guidProduct) + " instance_" + guid_to_string(instance->guidInstance);

		// allocate memory for the device object
		return std::make_unique<TDevice>(
				std::move(utf8_instance_name),
				std::move(utf8_instance_id),
				module,
				std::move(device),
				caps,
				format);
	}

	HRESULT enum_attached_devices(int devclass, device_enum_interface &enumerate_interface) const;

	template <typename T>
	static HRESULT set_dword_property(
			T &&device,
			REFGUID property_guid,
			DWORD object,
			DWORD how,
			DWORD value)
	{
		DIPROPDWORD dipdw;
		dipdw.diph.dwSize = sizeof(dipdw);
		dipdw.diph.dwHeaderSize = sizeof(dipdw.diph);
		dipdw.diph.dwObj = object;
		dipdw.diph.dwHow = how;
		dipdw.dwData = value;

		return device->SetProperty(property_guid, &dipdw.diph);
	}

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

private:
	std::pair<Microsoft::WRL::ComPtr<IDirectInputDevice8>, LPCDIDATAFORMAT> open_device(
			LPCDIDEVICEINSTANCE instance,
			LPCDIDATAFORMAT format1,
			LPCDIDATAFORMAT format2,
			dinput_cooperative_level cooperative_level);

	Microsoft::WRL::ComPtr<IDirectInput8> m_dinput;
	dynamic_module::ptr                   m_dinput_dll;
};


class dinput_device : public device_info
{
protected:
	dinput_device(
			std::string &&name,
			std::string &&id,
			input_module &module,
			Microsoft::WRL::ComPtr<IDirectInputDevice8> &&device,
			DIDEVCAPS const &caps,
			LPCDIDATAFORMAT format);

	HRESULT poll_dinput(LPVOID pState) const;

	std::string item_name(int offset, std::string_view defstr, char const *suffix) const;

	// DirectInput-specific information about a device
	Microsoft::WRL::ComPtr<IDirectInputDevice8> const   m_device;
	DIDEVCAPS                                           m_caps;
	LPCDIDATAFORMAT const                               m_format;
};


class dinput_keyboard_device : public dinput_device
{
public:
	dinput_keyboard_device(
			std::string &&name,
			std::string &&id,
			input_module &module,
			Microsoft::WRL::ComPtr<IDirectInputDevice8> &&device,
			DIDEVCAPS const &caps,
			LPCDIDATAFORMAT format);

	virtual void poll() override;
	virtual void reset() override;
	virtual void configure(input_device &device) override;

private:
	std::mutex      m_device_lock;
	keyboard_state  m_keyboard;
};


class dinput_mouse_device : public dinput_device
{
public:
	dinput_mouse_device(
			std::string &&name,
			std::string &&id,
			input_module &module,
			Microsoft::WRL::ComPtr<IDirectInputDevice8> &&device,
			DIDEVCAPS const &caps,
			LPCDIDATAFORMAT format);

	void poll() override;
	void reset() override;
	virtual void configure(input_device &device) override;

private:
	mouse_state m_mouse;
};


class dinput_joystick_device : public dinput_device
{
public:
	dinput_joystick_device(
			std::string &&name,
			std::string &&id,
			input_module &module,
			Microsoft::WRL::ComPtr<IDirectInputDevice8> &&device,
			DIDEVCAPS const &caps,
			LPCDIDATAFORMAT format);

	void reset() override;
	void poll() override;
	void configure(input_device &device) override;

private:
	// state information for a joystick; DirectInput state must be first element
	struct dinput_joystick_state
	{
		DIJOYSTATE  state;
		LONG        rangemin[8];
		LONG        rangemax[8];
	};

	static int32_t pov_get_state(void *device_internal, void *item_internal);

	dinput_joystick_state   m_joystick;
};

} // namespace osd

#endif // MAME_OSD_INPUT_INPUT_DINPUT_H
