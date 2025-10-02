// license:BSD-3-Clause
// copyright-holders:Brad Hughes, Janko Stamenović
//============================================================
//
//  input_winhybrid.cpp - Windows hybrid DirectInput/Xinput
//
//============================================================

#include "modules/osdmodule.h"
#if defined(OSD_WINDOWS) || defined(SDLMAME_WIN32)

#include "input_dinput.h"
#include "input_xinput.h"

#include <vector>

#include <oleauto.h>

#include <setupapi.h>
#include <cfgmgr32.h>   // for MAX_DEVICE_ID_LEN

namespace osd {

namespace {

struct bstr_deleter
{
	void operator () (BSTR bstr) const
	{
		if (bstr)
			SysFreeString(bstr);
	}
};


typedef std::unique_ptr<OLECHAR, bstr_deleter> bstr_ptr;


//============================================================
//  winhybrid_joystick_module
//============================================================

class winhybrid_joystick_module : public input_module_impl<device_info, osd_common_t>
{
private:
	std::unique_ptr<xinput_api_helper> m_xinput_helper;
	std::unique_ptr<dinput_api_helper> m_dinput_helper;

public:
	winhybrid_joystick_module() :
		input_module_impl<device_info, osd_common_t>(OSD_JOYSTICKINPUT_PROVIDER, "winhybrid")
	{
	}

	virtual bool probe() override
	{
		int status = init_helpers();
		if (status != 0)
		{
			osd_printf_verbose("Hybrid joystick module isn't supported, falling back.\n");
			return false;
		}

		return true;
	}

	virtual int init(osd_interface &osd, const osd_options &options) override
	{
		int status;

		// Call the base
		status = input_module_impl<device_info, osd_common_t>::init(osd, options);
		if (status != 0)
			return status;

		// Create and initialize our helpers
		status = init_helpers();
		if (status != 0)
		{
			osd_printf_error("Hybrid joystick module helpers failed to initialize. Error 0x%X\n", static_cast<unsigned int>(status));
			return status;
		}

		return 0;
	}

	virtual void input_init(running_machine &machine) override
	{
		input_module_impl<device_info, osd_common_t>::input_init(machine);

		bool xinput_detect_failed = false;
		std::vector<DWORD> xinput_deviceids;
		
		HRESULT result = get_xinput_devices(xinput_deviceids);
		if (result != 0)
			xinput_detect_failed = true;
			xinput_deviceids.clear();
			osd_printf_warning("XInput device detection failed. XInput won't be used. Error: 0x%X\n", uint32_t(result));
		}

		// Enumerate all the DirectInput joysticks and add them if they aren't XInput compatible
		result = m_dinput_helper->enum_attached_devices(
				DI8DEVCLASS_GAMECTRL,
				[this, &xinput_deviceids] (LPCDIDEVICEINSTANCE instance)
				{
					// First check if this device is XInput compatible.
					// If so, don't add it here as it'll be picked up by Xinput.
					auto const found = std::find(
							xinput_deviceids.begin(),
							xinput_deviceids.end(),
							instance->guidProduct.Data1);
					if (xinput_deviceids.end() != found)
					{
						osd_printf_verbose("Skipping DirectInput for XInput compatible joystick %S.\n", instance->tszInstanceName);
						return DIENUM_CONTINUE;
					}

					// allocate and link in a new device
					auto devinfo = m_dinput_helper->create_device<dinput_joystick_device>(
							*this,
							instance,
							&c_dfDIJoystick,
							nullptr,
							background_input() ? dinput_cooperative_level::BACKGROUND : dinput_cooperative_level::FOREGROUND,
							[] (auto const &device, auto const &format) -> bool
							{
								// set absolute mode
								HRESULT const result = dinput_api_helper::set_dword_property(
										device,
										DIPROP_AXISMODE,
										0,
										DIPH_DEVICE,
										DIPROPAXISMODE_ABS);
								if ((result != DI_OK) && (result != DI_PROPNOEFFECT))
								{
									osd_printf_error("DirectInput: Unable to set absolute mode for joystick.\n");
									return false;
								}
								return true;
							});
					if (devinfo)
						add_device(DEVICE_CLASS_JOYSTICK, std::move(devinfo));

					return DIENUM_CONTINUE;
				});
		if (result != DI_OK)
			fatalerror("DirectInput: Unable to enumerate game controllers (result=%08X).\n", uint32_t(result));

		// now add all xinput devices
		if (!xinput_detect_failed)
		{
			// Loop through each gamepad to determine if they are connected
			for (UINT i = 0; i < XUSER_MAX_COUNT; i++)
			{
				// allocate and link in a new device
				auto devinfo = m_xinput_helper->create_xinput_device(i, *this);
				if (devinfo)
					add_device(DEVICE_CLASS_JOYSTICK, std::move(devinfo));
			}
		}
	}

	virtual void exit() override
	{
		input_module_impl<device_info, osd_common_t>::exit();

		m_xinput_helper.reset();
		m_dinput_helper.reset();
	}

private:
	int init_helpers()
	{
		if (!m_xinput_helper)
		{
			m_xinput_helper = std::make_unique<xinput_api_helper>();
			int const status = m_xinput_helper->initialize();
			if (status != 0)
			{
				osd_printf_verbose("Failed to initialize XInput API! Error: %u\n", static_cast<unsigned int>(status));
				return -1;
			}
		}

		if (!m_dinput_helper)
		{
			m_dinput_helper = std::make_unique<dinput_api_helper>();
			int const status = m_dinput_helper->initialize();
			if (status != DI_OK)
			{
				osd_printf_verbose("Failed to initialize DirectInput API! Error: %u\n", static_cast<unsigned int>(status));
				return -1;
			}
		}

		return 0;
	}

	BOOL get_4hexd_id(PCWSTR const strIn, PCWSTR const prefix, PCWSTR const fmt, DWORD* pval)
	{
		PCWSTR const strFound = wcsstr(strIn, prefix);
		return strFound && swscanf(strFound, fmt, pval) == 1;
	}

	//-----------------------------------------------------------------------------
	// Enum each present PNP device in device information set using setupapi 
	// and check each device ID  to see if it contains
	// "IG_" (ex. "VID_045E&PID_028E&IG_00").  If it does, then it's an XInput device
	// Unfortunately this information can not be found by just using DirectInput.
	// Checking against a VID/PID of 0x028E/0x045E won't find 3rd party or future
	// XInput devices.
	//-----------------------------------------------------------------------------
	HRESULT get_xinput_devices(std::vector<DWORD> &xinput_deviceids)
	{
		HDEVINFO devInfoSet = SetupDiGetClassDevsW(nullptr, nullptr, nullptr, DIGCF_ALLCLASSES | DIGCF_PRESENT);
		if (devInfoSet == INVALID_HANDLE_VALUE) {
			osd_printf_error("SetupDiGetClassDevs failed.\n");
			return HRESULT_FROM_WIN32(GetLastError());
		}
		SP_DEVINFO_DATA devInfoData = {sizeof(SP_DEVINFO_DATA), 0, 0};
		DWORD devIndex = 0;
		while (SetupDiEnumDeviceInfo(devInfoSet, devIndex, &devInfoData)) 
		{
			devIndex++;
			WCHAR strDeviceID[MAX_DEVICE_ID_LEN];
			if (SetupDiGetDeviceInstanceIdW(
				devInfoSet, &devInfoData, strDeviceID, MAX_DEVICE_ID_LEN, nullptr)) 
			{
				// Check if the device ID contains "IG_".  If it does, then it's an XInput device
				// Unfortunately this information can not be found by just using DirectInput
				// If it does, then get the VID/PID as DWORD numbers
				DWORD dwVid = 0;
				DWORD dwPid = 0;
				if (wcsstr(strDeviceID, L"IG_") &&
					get_4hexd_id(strDeviceID, L"VID_", L"VID_%4X", &dwVid) &&
					get_4hexd_id(strDeviceID, L"PID_", L"PID_%4X", &dwPid))
				{
					// Add the VID/PID to a list
					xinput_deviceids.push_back(MAKELONG(dwVid, dwPid));
				}
			}
		}
		SetupDiDestroyDeviceInfoList(devInfoSet);
		return HRESULT_FROM_WIN32(ERROR_SUCCESS);
	}

};

} // anonymous namespace

} // namespace osd

#else // defined(OSD_WINDOWS) || defined(SDLMAME_WIN32)

#include "input_module.h"

namespace osd { namespace { MODULE_NOT_SUPPORTED(winhybrid_joystick_module, OSD_JOYSTICKINPUT_PROVIDER, "winhybrid") } }

#endif // defined(OSD_WINDOWS) || defined(SDLMAME_WIN32)

MODULE_DEFINITION(JOYSTICKINPUT_WINHYBRID, osd::winhybrid_joystick_module)
