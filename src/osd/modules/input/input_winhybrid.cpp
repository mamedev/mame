// license:BSD-3-Clause
// copyright-holders:Brad Hughes
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
#include <wbemcli.h>


namespace osd {

namespace {

template <class TCom>
class ComArray
{
private:
	std::vector<TCom *> m_entries;

public:
	ComArray(size_t capacity) : m_entries(capacity, nullptr)
	{
	}

	~ComArray()
	{
		Release();
	}

	TCom **ReleaseAndGetAddressOf()
	{
		Release();

		// This works b/c vector elements are guaranteed to be contiguous.
		return &m_entries[0];
	}

	TCom *operator[](int i)
	{
		return m_entries[i];
	}

	size_t Size()
	{
		return m_entries.size();
	}

	void Release()
	{
		for (auto &entry : m_entries)
		{
			if (entry)
			{
				entry->Release();
				entry = nullptr;
			}
		}
	}
};

struct bstr_deleter
{
	void operator () (BSTR bstr) const
	{
		if (bstr)
			SysFreeString(bstr);
	}
};

class variant_wrapper
{
private:
	DISABLE_COPYING(variant_wrapper);
	VARIANT m_variant;

public:
	variant_wrapper()
	{
		VariantInit(&m_variant);
	}

	~variant_wrapper()
	{
		Clear();
	}

	const VARIANT & Get() const
	{
		return m_variant;
	}

	VARIANT* ClearAndGetAddressOf()
	{
		Clear();
		return &m_variant;
	}

	void Clear()
	{
		if (m_variant.vt != VT_EMPTY)
			VariantClear(&m_variant);
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
		{
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

	//-----------------------------------------------------------------------------
	// Enum each PNP device using WMI and check each device ID to see if it contains
	// "IG_" (ex. "VID_045E&PID_028E&IG_00").  If it does, then it's an XInput device
	// Unfortunately this information can not be found by just using DirectInput.
	// Checking against a VID/PID of 0x028E/0x045E won't find 3rd party or future
	// XInput devices.
	//-----------------------------------------------------------------------------
	HRESULT get_xinput_devices(std::vector<DWORD> &xinput_deviceids)
	{
		xinput_deviceids.clear();

		HRESULT hr;

		// Create WMI
		Microsoft::WRL::ComPtr<IWbemLocator> pIWbemLocator;
		hr = CoCreateInstance(
				__uuidof(WbemLocator),
				nullptr,
				CLSCTX_INPROC_SERVER,
				IID_PPV_ARGS(pIWbemLocator.GetAddressOf()));
		if (FAILED(hr) || !pIWbemLocator)
		{
			osd_printf_error("Creating WbemLocator failed. Error: 0x%X\n", static_cast<unsigned int>(hr));
			return hr;
		}

		// Create BSTRs for WMI
		bstr_ptr bstrNamespace = bstr_ptr(SysAllocString(L"\\\\.\\root\\cimv2"));
		bstr_ptr bstrDeviceID = bstr_ptr(SysAllocString(L"DeviceID"));
		bstr_ptr bstrClassName = bstr_ptr(SysAllocString(L"Win32_PNPEntity"));

		// Connect to WMI
		Microsoft::WRL::ComPtr<IWbemServices> pIWbemServices;
		hr = pIWbemLocator->ConnectServer(
				bstrNamespace.get(),
				nullptr,
				nullptr,
				nullptr,
				0L,
				nullptr,
				nullptr,
				pIWbemServices.GetAddressOf());
		if (FAILED(hr) || !pIWbemServices)
		{
			osd_printf_error("Connecting to WMI Server failed. Error: 0x%X\n", static_cast<unsigned int>(hr));
			return hr;
		}

		// Switch security level to IMPERSONATE
		(void)CoSetProxyBlanket(
				pIWbemServices.Get(),
				RPC_C_AUTHN_WINNT,
				RPC_C_AUTHZ_NONE,
				nullptr,
				RPC_C_AUTHN_LEVEL_CALL,
				RPC_C_IMP_LEVEL_IMPERSONATE,
				nullptr,
				0);

		// Get list of Win32_PNPEntity devices
		Microsoft::WRL::ComPtr<IEnumWbemClassObject> pEnumDevices;
		hr = pIWbemServices->CreateInstanceEnum(bstrClassName.get(), 0, nullptr, pEnumDevices.GetAddressOf());
		if (FAILED(hr) || !pEnumDevices)
		{
			osd_printf_error("Getting list of Win32_PNPEntity devices failed. Error: 0x%X\n", static_cast<unsigned int>(hr));
			return hr;
		}

		// Loop over all devices
		ComArray<IWbemClassObject> pDevices(20);
		variant_wrapper var;
		for ( ; ; )
		{
			// Get a few at a time
			DWORD uReturned = 0;
			hr = pEnumDevices->Next(10000, pDevices.Size(), pDevices.ReleaseAndGetAddressOf(), &uReturned);
			if (FAILED(hr))
			{
				osd_printf_error("Enumerating WMI classes failed. Error: 0x%X\n", static_cast<unsigned int>(hr));
				return hr;
			}
			if (uReturned == 0)
				break;

			for (UINT iDevice = 0; iDevice < uReturned; iDevice++)
			{
				if (!pDevices[iDevice])
					continue;

				// For each device, get its device ID
				hr = pDevices[iDevice]->Get(bstrDeviceID.get(), 0L, var.ClearAndGetAddressOf(), nullptr, nullptr);
				if (SUCCEEDED(hr) && var.Get().vt == VT_BSTR && var.Get().bstrVal != nullptr)
				{
					// Check if the device ID contains "IG_".  If it does, then it's an XInput device
					// Unfortunately this information can not be found by just using DirectInput
					if (wcsstr(var.Get().bstrVal, L"IG_"))
					{
						// If it does, then get the VID/PID from var.bstrVal
						DWORD dwVid = 0;
						WCHAR const *const strVid = wcsstr(var.Get().bstrVal, L"VID_");
						if (strVid && swscanf(strVid, L"VID_%4X", &dwVid) != 1)
							dwVid = 0;

						DWORD dwPid = 0;
						WCHAR const *const strPid = wcsstr(var.Get().bstrVal, L"PID_");
						if (strPid && swscanf(strPid, L"PID_%4X", &dwPid) != 1)
							dwPid = 0;

						// Add the VID/PID to a list
						xinput_deviceids.push_back(MAKELONG(dwVid, dwPid));
					}
				}
			}
		}

		if (SUCCEEDED(hr))
			hr = S_OK;

		return hr;
	}
};

} // anonymous namespace

} // namespace osd

#else // defined(OSD_WINDOWS) || defined(SDLMAME_WIN32)

#include "input_module.h"

namespace osd { namespace { MODULE_NOT_SUPPORTED(winhybrid_joystick_module, OSD_JOYSTICKINPUT_PROVIDER, "winhybrid") } }

#endif // defined(OSD_WINDOWS) || defined(SDLMAME_WIN32)

MODULE_DEFINITION(JOYSTICKINPUT_WINHYBRID, osd::winhybrid_joystick_module)
