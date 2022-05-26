// license:BSD-3-Clause
// copyright-holders:Brad Hughes
//============================================================
//
//  input_winhybrid.cpp - Windows hybrid DirectInput/Xinput
//
//============================================================

#include "modules/osdmodule.h"

#if defined(OSD_WINDOWS)

#include "emu.h"

#include "input_dinput.h"
#include "input_xinput.h"

#include <list>
#include <vector>

#include <oleauto.h>
#include <wbemcli.h>


namespace {

using namespace Microsoft::WRL;

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

	TCom** ReleaseAndGetAddressOf()
	{
		Release();

		// This works b/c vector elements are guaranteed to be contiguous.
		return &m_entries[0];
	}

	TCom* operator [] (int i)
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
			if (entry != nullptr)
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

class winhybrid_joystick_module : public wininput_module, public device_enum_interface
{
private:
	std::shared_ptr<xinput_api_helper> m_xinput_helper;
	std::unique_ptr<dinput_api_helper> m_dinput_helper;
	std::list<DWORD> m_xinput_deviceids;
	bool m_xinput_detect_failed;

public:
	winhybrid_joystick_module() :
		wininput_module(OSD_JOYSTICKINPUT_PROVIDER, "winhybrid"),
		m_xinput_helper(nullptr),
		m_dinput_helper(nullptr),
		m_xinput_detect_failed(false)
	{
	}

	bool probe() override
	{
		int status = init_helpers();
		if (status != 0)
		{
			osd_printf_verbose("Hybrid joystick module isn't supported, falling back.\n");
			return false;
		}

		return true;
	}

	int init(const osd_options &options) override
	{
		// Call the base
		int status = wininput_module::init(options);
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

	BOOL device_enum_callback(LPCDIDEVICEINSTANCE instance, LPVOID ref) override
	{
		dinput_cooperative_level cooperative_level = dinput_cooperative_level::FOREGROUND;
		running_machine &machine = *static_cast<running_machine *>(ref);
		dinput_joystick_device *devinfo;
		int result = 0;

		// First check if this device is XInput Compatible. If so, don't add it here
		// as it'll be picked up by Xinput
		if (!m_xinput_detect_failed && is_xinput_device(&instance->guidProduct))
		{
			osd_printf_verbose("Skipping DirectInput for XInput compatible joystick %S.\n", instance->tszInstanceName);
			goto exit;
		}

		if (!osd_common_t::s_window_list.empty() && osd_common_t::s_window_list.front()->win_has_menu())
			cooperative_level = dinput_cooperative_level::BACKGROUND;

		// allocate and link in a new device
		devinfo = m_dinput_helper->create_device<dinput_joystick_device>(machine, *this, instance, &c_dfDIJoystick, nullptr, cooperative_level);
		if (devinfo == nullptr)
			goto exit;

		result = devinfo->configure();
		if (result != 0)
		{
			osd_printf_error("Failed to configure DI Joystick device. Error 0x%x\n", static_cast<unsigned int>(result));
		}

	exit:
		return DIENUM_CONTINUE;
	}

	void exit() override
	{
		m_xinput_helper.reset();
		m_dinput_helper.reset();

		wininput_module::exit();
	}

protected:
	virtual void input_init(running_machine &machine) override
	{
		HRESULT result = get_xinput_devices(m_xinput_deviceids);
		if (result != 0)
		{
			m_xinput_detect_failed = true;
			osd_printf_warning("XInput device detection failed. XInput won't be used. Error: 0x%X\n", uint32_t(result));
		}

		// Enumerate all the directinput joysticks and add them if they aren't xinput compatible
		result = m_dinput_helper->enum_attached_devices(DI8DEVCLASS_GAMECTRL, this, &machine);
		if (result != DI_OK)
			fatalerror("DirectInput: Unable to enumerate game controllers (result=%08X)\n", uint32_t(result));

		xinput_joystick_device *devinfo;

		// now add all xinput devices
		if (!m_xinput_detect_failed)
		{
			// Loop through each gamepad to determine if they are connected
			for (UINT i = 0; i < XUSER_MAX_COUNT; i++)
			{
				XINPUT_STATE state = { 0 };

				if (m_xinput_helper->xinput_get_state(i, &state) == ERROR_SUCCESS)
				{
					// allocate and link in a new device
					devinfo = m_xinput_helper->create_xinput_device(machine, i, *this);
					if (!devinfo)
						continue;

					// Configure each gamepad to add buttons and Axes, etc.
					devinfo->configure();
				}
			}
		}
	}

private:
	int init_helpers()
	{
		int status = 0;

		if (!m_xinput_helper)
		{
			m_xinput_helper = std::make_shared<xinput_api_helper>();
			status = m_xinput_helper->initialize();
			if (status != 0)
			{
				osd_printf_verbose("xinput_api_helper failed to initialize! Error: %u\n", static_cast<unsigned int>(status));
				return -1;
			}
		}

		if (!m_dinput_helper)
		{
			m_dinput_helper = std::make_unique<dinput_api_helper>();
			status = m_dinput_helper->initialize();
			if (status != DI_OK)
			{
				osd_printf_verbose("dinput_api_helper failed to initialize! Error: %u\n", static_cast<unsigned int>(status));
				return -1;
			}
		}

		return status;
	}

	//-----------------------------------------------------------------------------
	// Returns true if the DirectInput device is also an XInput device.
	//-----------------------------------------------------------------------------
	bool is_xinput_device(const GUID* pGuidProductFromDirectInput)
	{
		// Check each xinput device to see if this device's vid/pid matches
		for (auto devid = m_xinput_deviceids.begin(); devid != m_xinput_deviceids.end(); ++devid)
		{
			if (*devid == pGuidProductFromDirectInput->Data1)
				return true;
		}

		return false;
	}

	//-----------------------------------------------------------------------------
	// Enum each PNP device using WMI and check each device ID to see if it contains
	// "IG_" (ex. "VID_045E&PID_028E&IG_00").  If it does, then it's an XInput device
	// Unfortunately this information can not be found by just using DirectInput.
	// Checking against a VID/PID of 0x028E/0x045E won't find 3rd party or future
	// XInput devices.
	//-----------------------------------------------------------------------------
	HRESULT get_xinput_devices(std::list<DWORD> &xinput_id_list) const
	{
		ComPtr<IWbemServices> pIWbemServices;
		ComPtr<IEnumWbemClassObject> pEnumDevices;
		ComPtr<IWbemLocator> pIWbemLocator;
		ComArray<IWbemClassObject> pDevices(20);
		bstr_ptr bstrDeviceID;
		bstr_ptr bstrClassName;
		bstr_ptr bstrNamespace;
		DWORD uReturned = 0;
		UINT iDevice;
		variant_wrapper var;
		HRESULT hr;

		// CoInit if needed
		CoInitialize(nullptr);

		// Create WMI
		hr = CoCreateInstance(
			__uuidof(WbemLocator),
			nullptr,
			CLSCTX_INPROC_SERVER,
			__uuidof(IWbemLocator),
			reinterpret_cast<void**>(pIWbemLocator.GetAddressOf()));

		if (FAILED(hr) || pIWbemLocator == nullptr)
		{
			osd_printf_error("Creating WbemLocator failed. Error: 0x%X\n", static_cast<unsigned int>(hr));
			return hr;
		}

		// Create BSTRs for WMI
		bstrNamespace = bstr_ptr(SysAllocString(L"\\\\.\\root\\cimv2"));
		bstrDeviceID = bstr_ptr(SysAllocString(L"DeviceID"));
		bstrClassName = bstr_ptr(SysAllocString(L"Win32_PNPEntity"));

		// Connect to WMI
		hr = pIWbemLocator->ConnectServer(
			bstrNamespace.get(),
			nullptr,
			nullptr,
			nullptr,
			0L,
			nullptr,
			nullptr,
			pIWbemServices.GetAddressOf());

		if (FAILED(hr) || pIWbemServices == nullptr)
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
		hr = pIWbemServices->CreateInstanceEnum(bstrClassName.get(), 0, nullptr, pEnumDevices.GetAddressOf());
		if (FAILED(hr) || pEnumDevices == nullptr)
		{
			osd_printf_error("Getting list of Win32_PNPEntity devices failed. Error: 0x%X\n", static_cast<unsigned int>(hr));
			return hr;
		}

		// Loop over all devices
		for (; ; )
		{
			// Get a few at a time
			hr = pEnumDevices->Next(10000, pDevices.Size(), pDevices.ReleaseAndGetAddressOf(), &uReturned);
			if (FAILED(hr))
			{
				osd_printf_error("Enumerating WMI classes failed. Error: 0x%X\n", static_cast<unsigned int>(hr));
				return hr;
			}

			if (uReturned == 0)
				break;

			for (iDevice = 0; iDevice < uReturned; iDevice++)
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
						DWORD dwPid = 0, dwVid = 0;
						WCHAR* strVid = wcsstr(var.Get().bstrVal, L"VID_");
						if (strVid && swscanf(strVid, L"VID_%4X", &dwVid) != 1)
							dwVid = 0;

						WCHAR* strPid = wcsstr(var.Get().bstrVal, L"PID_");
						if (strPid && swscanf(strPid, L"PID_%4X", &dwPid) != 1)
							dwPid = 0;

						DWORD dwVidPid = MAKELONG(dwVid, dwPid);

						// Add the VID/PID to a linked list
						xinput_id_list.push_back(dwVidPid);
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

#else // defined(OSD_WINDOWS)

#include "input_module.h"

MODULE_NOT_SUPPORTED(winhybrid_joystick_module, OSD_JOYSTICKINPUT_PROVIDER, "winhybrid")

#endif // defined(OSD_WINDOWS)

MODULE_DEFINITION(JOYSTICKINPUT_WINHYBRID, winhybrid_joystick_module)
