// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Brad Hughes, Vas Crabb
//============================================================
//
//  input_dinput.h - Windows DirectInput support
//
//============================================================
#ifndef MAME_OSD_INPUT_INPUT_DINPUT_H
#define MAME_OSD_INPUT_INPUT_DINPUT_H

#pragma once

#include "assignmenthelper.h"
#include "input_wincommon.h"

#include "modules/lib/osdlib.h"
#include "modules/lib/osdobj_common.h"

#include "window.h"

#include "strconv.h"

#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

#include <windows.h>
#include <dinput.h>
#include <wrl/client.h>


namespace osd {

//============================================================
//  dinput_device - base directinput device
//============================================================

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

		// allocate memory for the device object
		return std::make_unique<TDevice>(
				text::from_tstring(instance->tszInstanceName),
				make_id(instance),
				module,
				std::move(device),
				caps,
				format);
	}

	template <typename T>
	HRESULT enum_attached_devices(int devclass, T &&callback) const
	{
		return m_dinput->EnumDevices(
				devclass,
				&di_enum_devices_cb<std::remove_reference_t<T> >,
				LPVOID(&callback),
				DIEDFL_ATTACHEDONLY);
	}

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

private:
	std::pair<Microsoft::WRL::ComPtr<IDirectInputDevice8>, LPCDIDATAFORMAT> open_device(
			LPCDIDEVICEINSTANCE instance,
			LPCDIDATAFORMAT format1,
			LPCDIDATAFORMAT format2,
			dinput_cooperative_level cooperative_level);

	static std::string make_id(LPCDIDEVICEINSTANCE instance);

	template <typename T>
	static BOOL CALLBACK di_enum_devices_cb(LPCDIDEVICEINSTANCE instance, LPVOID ref)
	{
		return (*reinterpret_cast<T *>(ref))(instance);
	}

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


class dinput_joystick_device : public dinput_device, protected joystick_assignment_helper
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
	void poll(bool relative_reset) override;
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
