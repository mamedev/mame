// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Brad Hughes
//============================================================
//
//  input_dinput.cpp - Windows DirectInput support
//
//============================================================

#include "input_module.h"
#include "modules/osdmodule.h"

#if defined(OSD_WINDOWS)

// standard windows headers
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <initguid.h>
#include <tchar.h>
#include <wrl/client.h>

// undef WINNT for dinput.h to prevent duplicate definition
#undef WINNT
#include <dinput.h>
#undef interface

#include <mutex>
#include <thread>

// MAME headers
#include "emu.h"
#include "osdepend.h"
#include "strconv.h"

// MAMEOS headers
#include "window.h"
#include "winutil.h"

#include "input_common.h"
#include "input_windows.h"
#include "input_dinput.h"

using namespace Microsoft::WRL;

static INT32 dinput_joystick_pov_get_state(void *device_internal, void *item_internal);

//============================================================
//  dinput_set_dword_property
//============================================================

static HRESULT dinput_set_dword_property(ComPtr<IDirectInputDevice> device, REFGUID property_guid, DWORD object, DWORD how, DWORD value)
{
	DIPROPDWORD dipdw;

	dipdw.diph.dwSize = sizeof(dipdw);
	dipdw.diph.dwHeaderSize = sizeof(dipdw.diph);
	dipdw.diph.dwObj = object;
	dipdw.diph.dwHow = how;
	dipdw.dwData = value;

	return device->SetProperty(property_guid, &dipdw.diph);
}

//============================================================
//  dinput_device - base directinput device
//============================================================

dinput_device::dinput_device(running_machine &machine, const char *name, input_device_class deviceclass, input_module &module)
	: device_info(machine, name, deviceclass, module),
		dinput({nullptr})
{
}

dinput_device::~dinput_device()
{
	if (dinput.device != nullptr)
		dinput.device.Reset();
}

HRESULT dinput_device::poll_dinput(LPVOID pState) const
{
	HRESULT result;

	// first poll the device, then get the state
	if (dinput.device2 != nullptr)
		dinput.device2->Poll();

	// GetDeviceState returns the immediate state
	result = dinput.device->GetDeviceState(dinput.format->dwDataSize, pState);

	// handle lost inputs here
	if (result == DIERR_INPUTLOST || result == DIERR_NOTACQUIRED)
	{
		result = dinput.device->Acquire();
		if (result == DI_OK)
			result = dinput.device->GetDeviceState(dinput.format->dwDataSize, pState);
	}

	return result;
}

//============================================================
//  dinput_keyboard_device - directinput keyboard device
//============================================================

dinput_keyboard_device::dinput_keyboard_device(running_machine &machine, const char *name, input_module &module)
	: dinput_device(machine, name, DEVICE_CLASS_KEYBOARD, module),
		keyboard({{0}})
{
}

// Polls the direct input immediate state
void dinput_keyboard_device::poll()
{
	std::lock_guard<std::mutex> scope_lock(m_device_lock);

	// Poll the state
	dinput_device::poll_dinput(&keyboard.state);
}

void dinput_keyboard_device::reset()
{
	memset(&keyboard.state, 0, sizeof(keyboard.state));
}

//============================================================
//  dinput_api_helper - DirectInput API helper
//============================================================

dinput_api_helper::dinput_api_helper(int version)
	: m_dinput(nullptr),
	  m_dinput_version(version),
	  m_pfn_DirectInputCreate("DirectInputCreateW", L"dinput.dll")
{
}

dinput_api_helper::~dinput_api_helper()
{
	m_dinput.Reset();
}

int dinput_api_helper::initialize()
{
	HRESULT result;

	if (m_dinput_version >= 0x0800)
	{
		result = DirectInput8Create(GetModuleHandleUni(), m_dinput_version, IID_IDirectInput8, reinterpret_cast<void **>(m_dinput.GetAddressOf()), nullptr);
		if (result != DI_OK)
		{
			m_dinput_version = 0;
			return result;
		}
	}
	else
	{
		result = m_pfn_DirectInputCreate.initialize();
		if (result != DI_OK)
			return result;

		// first attempt to initialize DirectInput at v7
		m_dinput_version = 0x0700;
		result = m_pfn_DirectInputCreate(GetModuleHandleUni(), m_dinput_version, m_dinput.GetAddressOf(), nullptr);
		if (result != DI_OK)
		{
			// if that fails, try version 5
			m_dinput_version = 0x0500;
			result = m_pfn_DirectInputCreate(GetModuleHandleUni(), m_dinput_version, m_dinput.GetAddressOf(), nullptr);
			if (result != DI_OK)
			{
				m_dinput_version = 0;
				return result;
			}
		}
	}

	osd_printf_verbose("DirectInput: Using DirectInput %d\n", m_dinput_version >> 8);
	return 0;
}


HRESULT dinput_api_helper::enum_attached_devices(int devclass, device_enum_interface *enumerate_interface, void *state) const
{
	device_enum_interface::dinput_callback_context ctx;
	ctx.self = enumerate_interface;
	ctx.state = state;

	return m_dinput->EnumDevices(devclass, device_enum_interface::enum_callback, &ctx, DIEDFL_ATTACHEDONLY);
}


//============================================================
//  dinput_module - base directinput module
//============================================================

class dinput_module : public wininput_module, public device_enum_interface
{
protected:
	std::unique_ptr<dinput_api_helper> m_dinput_helper;

public:
	dinput_module(const char* type, const char* name)
		: wininput_module(type, name),
		  m_dinput_helper(nullptr)
	{
	}

	int init_internal() override
	{
		m_dinput_helper = std::make_unique<dinput_api_helper>(DIRECTINPUT_VERSION);
		int result = m_dinput_helper->initialize();
		if (result != 0)
			return result;

		return 0;
	}

	void exit() override
	{
		wininput_module::exit();
		m_dinput_helper.reset();
	}

	void input_init(running_machine &machine) override
	{
		HRESULT result = m_dinput_helper->enum_attached_devices(dinput_devclass(), this, &machine);
		if (result != DI_OK)
			fatalerror("DirectInput: Unable to enumerate keyboards (result=%08X)\n", static_cast<UINT32>(result));
	}

	static std::string device_item_name(dinput_device * devinfo, int offset, const char * defstring, const TCHAR * suffix)
	{
		DIDEVICEOBJECTINSTANCE instance = { 0 };
		HRESULT result;

		// query the key name
		instance.dwSize = sizeof(instance);
		result = devinfo->dinput.device->GetObjectInfo(&instance, offset, DIPH_BYOFFSET);

		// if we got an error and have no default string, just return NULL
		if (result != DI_OK)
		{
			if (defstring == nullptr)
				return nullptr;

			// Return the default value
			return std::string(defstring);
		}

		auto osd_free_deleter = [](char *p) { osd_free(p); };

		// convert the name to utf8
		auto namestring = std::unique_ptr<char, decltype(osd_free_deleter)>(utf8_from_tstring(instance.tszName), osd_free_deleter);

		// if no suffix, return as-is
		if (suffix == nullptr)
		{
			return std::string(namestring.get());
		}

		// otherwise, allocate space to add the suffix
		auto combined = std::make_unique<char[]>(strlen(namestring.get()) + 1 + _tcslen(suffix) + 1);

		// convert the suffix to utf8
		auto suffix_utf8 = std::unique_ptr<char, decltype(osd_free_deleter)>(utf8_from_tstring(suffix), osd_free_deleter);

		// Concat the name and suffix
		strcpy(combined.get(), namestring.get());
		strcat(combined.get(), " ");
		strcat(combined.get(), suffix_utf8.get());

		return std::string(combined.get());
	}

protected:
	virtual int dinput_devclass() = 0;
};

class keyboard_input_dinput : public dinput_module
{
public:
	keyboard_input_dinput() :
		dinput_module(OSD_KEYBOARDINPUT_PROVIDER, "dinput")
	{
	}

	int dinput_devclass() override
	{
#if DIRECTINPUT_VERSION >= 0x800
		return DI8DEVCLASS_KEYBOARD;
#else
		return DIDEVTYPE_KEYBOARD;
#endif
	}

	BOOL device_enum_callback(LPCDIDEVICEINSTANCE instance, LPVOID ref) override
	{
		running_machine &machine = *static_cast<running_machine *>(ref);
		dinput_keyboard_device *devinfo;
		int keynum;

		// allocate and link in a new device
		devinfo = m_dinput_helper->create_device<dinput_keyboard_device>(machine, *this, instance, &c_dfDIKeyboard, nullptr, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
		if (devinfo == nullptr)
			goto exit;

		// populate it
		for (keynum = 0; keynum < MAX_KEYS; keynum++)
		{
			input_item_id itemid = keyboard_trans_table::instance().map_di_scancode_to_itemid(keynum);
			char defname[20];
			std::string name;

			// generate/fetch the name
			snprintf(defname, ARRAY_LENGTH(defname), "Scan%03d", keynum);
			name = device_item_name(devinfo, keynum, defname, nullptr);

			// add the item to the device
			devinfo->device()->add_item(name.c_str(), itemid, generic_button_get_state, &devinfo->keyboard.state[keynum]);
		}

	exit:
		return DIENUM_CONTINUE;
	}
};


dinput_mouse_device::dinput_mouse_device(running_machine &machine, const char *name, input_module &module)
	: dinput_device(machine, name, DEVICE_CLASS_MOUSE, module),
		mouse({0})
{
}

void dinput_mouse_device::poll()
{
	// poll
	dinput_device::poll_dinput(&mouse);

	// scale the axis data
	mouse.lX *= INPUT_RELATIVE_PER_PIXEL;
	mouse.lY *= INPUT_RELATIVE_PER_PIXEL;
	mouse.lZ *= INPUT_RELATIVE_PER_PIXEL;
}

void dinput_mouse_device::reset()
{
	memset(&mouse, 0, sizeof(mouse));
}

class mouse_input_dinput : public dinput_module
{
public:
	mouse_input_dinput() :
		dinput_module(OSD_MOUSEINPUT_PROVIDER, "dinput")
	{
	}

	int dinput_devclass() override
	{
#if DIRECTINPUT_VERSION >= 0x800
		return DI8DEVCLASS_POINTER;
#else
		return DIDEVTYPE_MOUSE;
#endif
	}

	BOOL device_enum_callback(LPCDIDEVICEINSTANCE instance, LPVOID ref) override
	{
		dinput_mouse_device *devinfo = nullptr;
		running_machine &machine = *static_cast<running_machine *>(ref);
		int axisnum, butnum;
		HRESULT result;

		// allocate and link in a new device
		devinfo = m_dinput_helper->create_device<dinput_mouse_device>(machine, *this, instance, &c_dfDIMouse2, &c_dfDIMouse, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
		if (devinfo == nullptr)
			goto exit;

		// set relative mode on the mouse device
		result = dinput_set_dword_property(devinfo->dinput.device, DIPROP_AXISMODE, 0, DIPH_DEVICE, DIPROPAXISMODE_REL);
		if (result != DI_OK && result != DI_PROPNOEFFECT)
		{
			osd_printf_error("DirectInput: Unable to set relative mode for mouse %d (%s)\n", devicelist()->size(), devinfo->name());
			goto error;
		}

		// cap the number of axes and buttons based on the format
		devinfo->dinput.caps.dwAxes = MIN(devinfo->dinput.caps.dwAxes, 3);
		devinfo->dinput.caps.dwButtons = MIN(devinfo->dinput.caps.dwButtons, (devinfo->dinput.format == &c_dfDIMouse) ? 4 : 8);

		// populate the axes
		for (axisnum = 0; axisnum < devinfo->dinput.caps.dwAxes; axisnum++)
		{
			// add to the mouse device and optionally to the gun device as well
			std::string name = device_item_name(devinfo, offsetof(DIMOUSESTATE, lX) + axisnum * sizeof(LONG), default_axis_name[axisnum], nullptr);
			devinfo->device()->add_item(name.c_str(), static_cast<input_item_id>(ITEM_ID_XAXIS + axisnum), generic_axis_get_state, &devinfo->mouse.lX + axisnum);
		}

		// populate the buttons
		for (butnum = 0; butnum < devinfo->dinput.caps.dwButtons; butnum++)
		{
			FPTR offset = reinterpret_cast<FPTR>(&static_cast<DIMOUSESTATE *>(nullptr)->rgbButtons[butnum]);

			// add to the mouse device
			std::string name = device_item_name(devinfo, offset, default_button_name(butnum), nullptr);
			devinfo->device()->add_item(name.c_str(), static_cast<input_item_id>(ITEM_ID_BUTTON1 + butnum), generic_button_get_state, &devinfo->mouse.rgbButtons[butnum]);
		}

	exit:
		return DIENUM_CONTINUE;

	error:
		if (devinfo != nullptr)
			devicelist()->free_device(devinfo);
		goto exit;
	}
};

dinput_joystick_device::dinput_joystick_device(running_machine &machine, const char *name, input_module &module)
	: dinput_device(machine, name, DEVICE_CLASS_JOYSTICK, module),
		joystick({{0}})
{
}

void dinput_joystick_device::reset()
{
	memset(&joystick.state, 0, sizeof(joystick.state));
}

void dinput_joystick_device::poll()
{
	int axisnum;

	// poll the device first
	if (dinput_device::poll_dinput(&joystick.state) != ERROR_SUCCESS)
		return;

	// normalize axis values
	for (axisnum = 0; axisnum < 8; axisnum++)
	{
		LONG *axis = (&joystick.state.lX) + axisnum;
		*axis = normalize_absolute_axis(*axis, joystick.rangemin[axisnum], joystick.rangemax[axisnum]);
	}
}

int dinput_joystick_device::configure()
{
	HRESULT result;
	UINT32 axisnum, axiscount;

	auto devicelist = static_cast<input_module_base&>(module()).devicelist();

	// temporary approximation of index
	int devindex = devicelist->size();

	// set absolute mode
	result = dinput_set_dword_property(dinput.device, DIPROP_AXISMODE, 0, DIPH_DEVICE, DIPROPAXISMODE_ABS);
	if (result != DI_OK && result != DI_PROPNOEFFECT)
		osd_printf_warning("DirectInput: Unable to set absolute mode for joystick %d (%s)\n", devindex, name());

	// turn off deadzone; we do our own calculations
	result = dinput_set_dword_property(dinput.device, DIPROP_DEADZONE, 0, DIPH_DEVICE, 0);
	if (result != DI_OK && result != DI_PROPNOEFFECT)
		osd_printf_warning("DirectInput: Unable to reset deadzone for joystick %d (%s)\n", devindex, name());

	// turn off saturation; we do our own calculations
	result = dinput_set_dword_property(dinput.device, DIPROP_SATURATION, 0, DIPH_DEVICE, 10000);
	if (result != DI_OK && result != DI_PROPNOEFFECT)
		osd_printf_warning("DirectInput: Unable to reset saturation for joystick %d (%s)\n", devindex, name());

	// cap the number of axes, POVs, and buttons based on the format
	dinput.caps.dwAxes = MIN(dinput.caps.dwAxes, 8);
	dinput.caps.dwPOVs = MIN(dinput.caps.dwPOVs, 4);
	dinput.caps.dwButtons = MIN(dinput.caps.dwButtons, 128);

	// populate the axes
	for (axisnum = axiscount = 0; axiscount < dinput.caps.dwAxes && axisnum < 8; axisnum++)
	{
		DIPROPRANGE dipr;
		std::string name;

		// fetch the range of this axis
		dipr.diph.dwSize = sizeof(dipr);
		dipr.diph.dwHeaderSize = sizeof(dipr.diph);
		dipr.diph.dwObj = offsetof(DIJOYSTATE2, lX) + axisnum * sizeof(LONG);
		dipr.diph.dwHow = DIPH_BYOFFSET;
		result = dinput.device->GetProperty(DIPROP_RANGE, &dipr.diph);
		if (result != DI_OK)
			continue;

		joystick.rangemin[axisnum] = dipr.lMin;
		joystick.rangemax[axisnum] = dipr.lMax;

		// populate the item description as well
		name = dinput_module::device_item_name(this, offsetof(DIJOYSTATE2, lX) + axisnum * sizeof(LONG), default_axis_name[axisnum], nullptr);
		device()->add_item(name.c_str(), static_cast<input_item_id>(ITEM_ID_XAXIS + axisnum), generic_axis_get_state, &joystick.state.lX + axisnum);

		axiscount++;
	}

	// populate the POVs
	for (UINT32 povnum = 0; povnum < dinput.caps.dwPOVs; povnum++)
	{
		std::string name;

		// left
		name = dinput_module::device_item_name(this, offsetof(DIJOYSTATE2, rgdwPOV) + povnum * sizeof(DWORD), default_pov_name(povnum), TEXT("L"));
		device()->add_item(name.c_str(), ITEM_ID_OTHER_SWITCH, dinput_joystick_pov_get_state, reinterpret_cast<void *>(static_cast<FPTR>(povnum * 4 + POVDIR_LEFT)));

		// right
		name = dinput_module::device_item_name(this, offsetof(DIJOYSTATE2, rgdwPOV) + povnum * sizeof(DWORD), default_pov_name(povnum), TEXT("R"));
		device()->add_item(name.c_str(), ITEM_ID_OTHER_SWITCH, dinput_joystick_pov_get_state, reinterpret_cast<void *>(static_cast<FPTR>(povnum * 4 + POVDIR_RIGHT)));

		// up
		name = dinput_module::device_item_name(this, offsetof(DIJOYSTATE2, rgdwPOV) + povnum * sizeof(DWORD), default_pov_name(povnum), TEXT("U"));
		device()->add_item(name.c_str(), ITEM_ID_OTHER_SWITCH, dinput_joystick_pov_get_state, reinterpret_cast<void *>(static_cast<FPTR>(povnum * 4 + POVDIR_UP)));

		// down
		name = dinput_module::device_item_name(this, offsetof(DIJOYSTATE2, rgdwPOV) + povnum * sizeof(DWORD), default_pov_name(povnum), TEXT("D"));
		device()->add_item(name.c_str(), ITEM_ID_OTHER_SWITCH, dinput_joystick_pov_get_state, reinterpret_cast<void *>(static_cast<FPTR>(povnum * 4 + POVDIR_DOWN)));
	}

	// populate the buttons
	for (UINT32 butnum = 0; butnum < dinput.caps.dwButtons; butnum++)
	{
		FPTR offset = reinterpret_cast<FPTR>(&static_cast<DIJOYSTATE2 *>(nullptr)->rgbButtons[butnum]);
		std::string name = dinput_module::device_item_name(this, offset, default_button_name(butnum), nullptr);

		input_item_id itemid;

		if (butnum < INPUT_MAX_BUTTONS)
			itemid = static_cast<input_item_id>(ITEM_ID_BUTTON1 + butnum);
		else if (butnum < INPUT_MAX_BUTTONS + INPUT_MAX_ADD_SWITCH)
			itemid = static_cast<input_item_id>(ITEM_ID_ADD_SWITCH1 - INPUT_MAX_BUTTONS + butnum);
		else
			itemid = ITEM_ID_OTHER_SWITCH;

		device()->add_item(name.c_str(), itemid, generic_button_get_state, &joystick.state.rgbButtons[butnum]);
	}

	return 0;
}

class joystick_input_dinput : public dinput_module
{
public:
	joystick_input_dinput() :
		dinput_module(OSD_JOYSTICKINPUT_PROVIDER, "dinput")
	{
	}

	int dinput_devclass() override
	{
#if DIRECTINPUT_VERSION >= 0x800
		return DI8DEVCLASS_GAMECTRL;
#else
		return DIDEVTYPE_JOYSTICK;
#endif
	}

	BOOL device_enum_callback(LPCDIDEVICEINSTANCE instance, LPVOID ref) override
	{
		DWORD cooperative_level = DISCL_FOREGROUND | DISCL_NONEXCLUSIVE;
		running_machine &machine = *static_cast<running_machine *>(ref);
		dinput_joystick_device *devinfo;
		int result = 0;

		if (win_window_list != nullptr && win_window_list->win_has_menu())
			cooperative_level = DISCL_BACKGROUND | DISCL_NONEXCLUSIVE;

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
};

//============================================================
//  dinput_joystick_pov_get_state
//============================================================

static INT32 dinput_joystick_pov_get_state(void *device_internal, void *item_internal)
{
	dinput_joystick_device *devinfo = static_cast<dinput_joystick_device *>(device_internal);
	int povnum = reinterpret_cast<FPTR>(item_internal) / 4;
	int povdir = reinterpret_cast<FPTR>(item_internal) % 4;
	INT32 result = 0;
	DWORD pov;

	// get the current state
	devinfo->module().poll_if_necessary(devinfo->machine());
	pov = devinfo->joystick.state.rgdwPOV[povnum];

	// if invalid, return 0
	if ((pov & 0xffff) == 0xffff)
		return result;

	// return the current state
	switch (povdir)
	{
	case POVDIR_LEFT:   result = (pov >= 22500 && pov <= 31500);    break;
	case POVDIR_RIGHT:  result = (pov >= 4500 && pov <= 13500);     break;
	case POVDIR_UP:     result = (pov >= 31500 || pov <= 4500);     break;
	case POVDIR_DOWN:   result = (pov >= 13500 && pov <= 22500);    break;
	}
	return result;
}

#else
MODULE_NOT_SUPPORTED(keyboard_input_dinput, OSD_KEYBOARDINPUT_PROVIDER, "dinput")
MODULE_NOT_SUPPORTED(mouse_input_dinput, OSD_MOUSEINPUT_PROVIDER, "dinput")
MODULE_NOT_SUPPORTED(joystick_input_dinput, OSD_JOYSTICKINPUT_PROVIDER, "dinput")
#endif

MODULE_DEFINITION(KEYBOARDINPUT_DINPUT, keyboard_input_dinput)
MODULE_DEFINITION(MOUSEINPUT_DINPUT, mouse_input_dinput)
MODULE_DEFINITION(JOYSTICKINPUT_DINPUT, joystick_input_dinput)
