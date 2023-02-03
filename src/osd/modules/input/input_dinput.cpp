// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Brad Hughes
//============================================================
//
//  input_dinput.cpp - Windows DirectInput support
//
//============================================================

#include "modules/osdmodule.h"

#if defined(OSD_WINDOWS) || defined(SDLMAME_WIN32)

// emu
#include "emu.h" // put this here before Windows headers define interface as a macro

#include "input_dinput.h"

#include "windows/winutil.h"

// lib/util
#include "util/corestr.h"

#ifdef SDLMAME_WIN32
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#endif

#include <algorithm>
#include <iterator>
#include <memory>

// standard windows headers
#include <initguid.h>
#include <tchar.h>


namespace osd {

namespace {

BOOL CALLBACK device_enum_interface_callback(LPCDIDEVICEINSTANCE instance, LPVOID ref)
{
	return static_cast<device_enum_interface *>(ref)->device_enum_callback(instance);
}


//============================================================
//  dinput_module - base directinput module
//============================================================

class dinput_module : public input_module_impl<dinput_device, osd_common_t>, public device_enum_interface
{
protected:
	std::unique_ptr<dinput_api_helper> m_dinput_helper;

public:
	dinput_module(const char* type, const char* name) :
		input_module_impl<dinput_device, osd_common_t>(type, name),
		m_dinput_helper(nullptr)
	{
	}

	virtual int init(osd_interface &osd, osd_options const &options) override
	{
		m_dinput_helper = std::make_unique<dinput_api_helper>();
		int const result = m_dinput_helper->initialize();
		if (result)
		{
			m_dinput_helper.reset();
			return result;
		}

		return input_module_impl<dinput_device, osd_common_t>::init(osd, options);
	}

	virtual void exit() override
	{
		input_module_impl<dinput_device, osd_common_t>::exit();

		m_dinput_helper.reset();
	}

	virtual void input_init(running_machine &machine) override
	{
		input_module_impl<dinput_device, osd_common_t>::input_init(machine);

		HRESULT result = m_dinput_helper->enum_attached_devices(dinput_devclass(), *this);
		if (result != DI_OK)
			fatalerror("DirectInput: Unable to enumerate devices (result=%08X)\n", uint32_t(result));
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

	virtual int dinput_devclass() override
	{
		return DI8DEVCLASS_KEYBOARD;
	}

	virtual BOOL device_enum_callback(LPCDIDEVICEINSTANCE instance) override
	{
		// allocate and link in a new device
		auto devinfo = m_dinput_helper->create_device<dinput_keyboard_device>(
				*this,
				instance,
				&c_dfDIKeyboard,
				nullptr,
				background_input() ? dinput_cooperative_level::BACKGROUND : dinput_cooperative_level::FOREGROUND,
				[] (auto...) { return true; });
		if (devinfo)
			add_device(DEVICE_CLASS_KEYBOARD, std::move(devinfo));

		return DIENUM_CONTINUE;
	}
};


class mouse_input_dinput : public dinput_module
{
public:
	mouse_input_dinput() :
		dinput_module(OSD_MOUSEINPUT_PROVIDER, "dinput")
	{
	}

	virtual int dinput_devclass() override
	{
		return DI8DEVCLASS_POINTER;
	}

	virtual BOOL device_enum_callback(LPCDIDEVICEINSTANCE instance) override
	{
		// allocate and link in a new device
		auto devinfo = m_dinput_helper->create_device<dinput_mouse_device>(
				*this,
				instance,
				&c_dfDIMouse2,
				&c_dfDIMouse,
				background_input() ? dinput_cooperative_level::BACKGROUND : dinput_cooperative_level::FOREGROUND,
				[] (auto const &device, auto const &format) -> bool
				{
					// set relative mode
					HRESULT const result = dinput_api_helper::set_dword_property(
							device,
							DIPROP_AXISMODE,
							0,
							DIPH_DEVICE,
							DIPROPAXISMODE_REL);
					if ((result != DI_OK) && (result != DI_PROPNOEFFECT))
					{
						osd_printf_error("DirectInput: Unable to set relative mode for mouse.\n");
						return false;
					}
					return true;
				});
		if (devinfo)
			add_device(DEVICE_CLASS_MOUSE, std::move(devinfo));

		return DIENUM_CONTINUE;
	}
};


class joystick_input_dinput : public dinput_module
{
public:
	joystick_input_dinput() :
		dinput_module(OSD_JOYSTICKINPUT_PROVIDER, "dinput")
	{
	}

	virtual int dinput_devclass() override
	{
		return DI8DEVCLASS_GAMECTRL;
	}

	virtual BOOL device_enum_callback(LPCDIDEVICEINSTANCE instance) override
	{
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
	}
};

} // anonymous namespace


//============================================================
//  dinput_device - base directinput device
//============================================================

dinput_device::dinput_device(
		std::string &&name,
		std::string &&id,
		input_module &module,
		Microsoft::WRL::ComPtr<IDirectInputDevice8> &&device,
		DIDEVCAPS const &caps,
		LPCDIDATAFORMAT format) :
	device_info(std::move(name), std::move(id), module),
	m_device(std::move(device)),
	m_caps(caps),
	m_format(format)
{
}

HRESULT dinput_device::poll_dinput(LPVOID pState) const
{
	HRESULT result;

	// first poll the device, then get the state
	result = m_device->Poll();

	// handle lost inputs here
	if ((result == DIERR_INPUTLOST) || (result == DIERR_NOTACQUIRED))
	{
		result = m_device->Acquire();
		if ((result == DI_OK) || (result == S_FALSE))
			result = m_device->Poll();
	}

	// GetDeviceState returns the immediate state
	if ((result == DI_OK) || (result == DI_NOEFFECT))
		result = m_device->GetDeviceState(m_format->dwDataSize, pState);

	return result;
}

std::string dinput_device::item_name(int offset, std::string_view defstring, const char *suffix) const
{
	// query the key name
	DIDEVICEOBJECTINSTANCE instance = { 0 };
	instance.dwSize = sizeof(instance);
	HRESULT const result = m_device->GetObjectInfo(&instance, offset, DIPH_BYOFFSET);

	// use the default value if it failed
	std::string name;
	if (result != DI_OK)
		name = defstring;
	else
		name = text::from_tstring(instance.tszName);

	// if no suffix, return as-is
	if (suffix)
		name.append(" ").append(suffix);

	return name;
}


//============================================================
//  dinput_keyboard_device - directinput keyboard device
//============================================================

dinput_keyboard_device::dinput_keyboard_device(
		std::string &&name,
		std::string &&id,
		input_module &module,
		Microsoft::WRL::ComPtr<IDirectInputDevice8> &&device,
		DIDEVCAPS const &caps,
		LPCDIDATAFORMAT format) :
	dinput_device(std::move(name), std::move(id), module, std::move(device), caps, format),
	m_keyboard({ { 0 } })
{
}

void dinput_keyboard_device::poll()
{
	// poll the DirectInput immediate state
	std::lock_guard<std::mutex> scope_lock(m_device_lock);
	poll_dinput(&m_keyboard.state);
}

void dinput_keyboard_device::reset()
{
	memset(&m_keyboard.state, 0, sizeof(m_keyboard.state));
}

void dinput_keyboard_device::configure(input_device &device)
{
	// populate it
	char defname[20];
	for (int keynum = 0; keynum < MAX_KEYS; keynum++)
	{
		input_item_id itemid = keyboard_trans_table::instance().map_di_scancode_to_itemid(keynum);

		// generate/fetch the name
		snprintf(defname, std::size(defname), "Scan%03d", keynum);

		// add the item to the device
		device.add_item(
				item_name(keynum, defname, nullptr),
				itemid,
				generic_button_get_state<std::uint8_t>,
				&m_keyboard.state[keynum]);
	}
}


//============================================================
//  dinput_mouse_device - directinput mouse device
//============================================================

dinput_mouse_device::dinput_mouse_device(
		std::string &&name,
		std::string &&id,
		input_module &module,
		Microsoft::WRL::ComPtr<IDirectInputDevice8> &&device,
		DIDEVCAPS const &caps,
		LPCDIDATAFORMAT format) :
	dinput_device(std::move(name), std::move(id), module, std::move(device), caps, format),
	m_mouse({0})
{
	// cap the number of axes and buttons based on the format
	m_caps.dwAxes = std::min(m_caps.dwAxes, DWORD(3));
	m_caps.dwButtons = std::min(m_caps.dwButtons, DWORD((m_format == &c_dfDIMouse) ? 4 : 8));
}

void dinput_mouse_device::poll()
{
	// poll
	if (poll_dinput(&m_mouse) == DI_OK)
	{
		// scale the axis data
		m_mouse.lX *= INPUT_RELATIVE_PER_PIXEL;
		m_mouse.lY *= INPUT_RELATIVE_PER_PIXEL;
		m_mouse.lZ *= INPUT_RELATIVE_PER_PIXEL;
	}
}

void dinput_mouse_device::reset()
{
	memset(&m_mouse, 0, sizeof(m_mouse));
}

void dinput_mouse_device::configure(input_device &device)
{
	// populate the axes
	for (int axisnum = 0; axisnum < m_caps.dwAxes; axisnum++)
	{
		// add to the mouse device and optionally to the gun device as well
		device.add_item(
				item_name(offsetof(DIMOUSESTATE, lX) + axisnum * sizeof(LONG), default_axis_name[axisnum], nullptr),
				input_item_id(ITEM_ID_XAXIS + axisnum),
				generic_axis_get_state<LONG>,
				&m_mouse.lX + axisnum);
	}

	// populate the buttons
	for (int butnum = 0; butnum < m_caps.dwButtons; butnum++)
	{
		auto offset = reinterpret_cast<uintptr_t>(&static_cast<DIMOUSESTATE *>(nullptr)->rgbButtons[butnum]);

		// add to the mouse device
		device.add_item(
				item_name(offset, default_button_name(butnum), nullptr),
				input_item_id(ITEM_ID_BUTTON1 + butnum),
				generic_button_get_state<BYTE>,
				&m_mouse.rgbButtons[butnum]);
	}
}


//============================================================
//  dinput_joystick_device - directinput joystick device
//============================================================

dinput_joystick_device::dinput_joystick_device(
		std::string &&name,
		std::string &&id,
		input_module &module,
		Microsoft::WRL::ComPtr<IDirectInputDevice8> &&device,
		DIDEVCAPS const &caps,
		LPCDIDATAFORMAT format) :
	dinput_device(std::move(name), std::move(id), module, std::move(device), caps, format),
	m_joystick({ { 0 } })
{
	// cap the number of axes, POVs, and buttons based on the format
	m_caps.dwAxes = std::min(m_caps.dwAxes, DWORD(8));
	m_caps.dwPOVs = std::min(m_caps.dwPOVs, DWORD(4));
	m_caps.dwButtons = std::min(m_caps.dwButtons, DWORD(128));
}

void dinput_joystick_device::reset()
{
	memset(&m_joystick.state, 0, sizeof(m_joystick.state));
	std::fill(std::begin(m_joystick.state.rgdwPOV), std::end(m_joystick.state.rgdwPOV), 0xffff);
}

void dinput_joystick_device::poll()
{
	// poll the device first
	if (dinput_device::poll_dinput(&m_joystick.state) == DI_OK)
	{
		// normalize axis values
		for (int axisnum = 0; axisnum < 8; axisnum++)
		{
			LONG *const axis = &m_joystick.state.lX + axisnum;
			*axis = normalize_absolute_axis(*axis, m_joystick.rangemin[axisnum], m_joystick.rangemax[axisnum]);
		}
	}
}

void dinput_joystick_device::configure(input_device &device)
{
	HRESULT result;

	// turn off deadzone; we do our own calculations
	result = dinput_api_helper::set_dword_property(m_device, DIPROP_DEADZONE, 0, DIPH_DEVICE, 0);
	if (result != DI_OK && result != DI_PROPNOEFFECT)
		osd_printf_warning("DirectInput: Unable to reset deadzone for joystick %s.\n", name());

	// turn off saturation; we do our own calculations
	result = dinput_api_helper::set_dword_property(m_device, DIPROP_SATURATION, 0, DIPH_DEVICE, 10000);
	if (result != DI_OK && result != DI_PROPNOEFFECT)
		osd_printf_warning("DirectInput: Unable to reset saturation for joystick %s.\n", name());

	// populate the axes
	for (uint32_t axisnum = 0, axiscount = 0; axiscount < m_caps.dwAxes && axisnum < 8; axisnum++)
	{
		// fetch the range of this axis
		DIPROPRANGE dipr;
		dipr.diph.dwSize = sizeof(dipr);
		dipr.diph.dwHeaderSize = sizeof(dipr.diph);
		dipr.diph.dwObj = offsetof(DIJOYSTATE2, lX) + axisnum * sizeof(LONG);
		dipr.diph.dwHow = DIPH_BYOFFSET;
		result = m_device->GetProperty(DIPROP_RANGE, &dipr.diph);
		if (result != DI_OK)
		{
			osd_printf_verbose("DirectInput: Unable to get properties for joystick %s axis %u.\n", name(), axisnum);
			continue;
		}

		m_joystick.rangemin[axisnum] = dipr.lMin;
		m_joystick.rangemax[axisnum] = dipr.lMax;

		// populate the item description as well
		device.add_item(
				item_name(offsetof(DIJOYSTATE2, lX) + axisnum * sizeof(LONG), default_axis_name[axisnum], nullptr),
				input_item_id(ITEM_ID_XAXIS + axisnum),
				generic_axis_get_state<LONG>,
				&m_joystick.state.lX + axisnum);

		axiscount++;
	}

	// populate the POVs
	for (uint32_t povnum = 0; povnum < m_caps.dwPOVs; povnum++)
	{
		// left
		device.add_item(
				item_name(offsetof(DIJOYSTATE2, rgdwPOV) + povnum * sizeof(DWORD), default_pov_name(povnum), "Left"),
				input_item_id(povnum * 4 + ITEM_ID_HAT1LEFT),
				&dinput_joystick_device::pov_get_state,
				reinterpret_cast<void *>(uintptr_t(povnum * 4 + POVDIR_LEFT)));

		// right
		device.add_item(
				item_name(offsetof(DIJOYSTATE2, rgdwPOV) + povnum * sizeof(DWORD), default_pov_name(povnum), "Right"),
				input_item_id(povnum * 4 + ITEM_ID_HAT1RIGHT),
				&dinput_joystick_device::pov_get_state,
				reinterpret_cast<void *>(uintptr_t(povnum * 4 + POVDIR_RIGHT)));

		// up
		device.add_item(
				item_name(offsetof(DIJOYSTATE2, rgdwPOV) + povnum * sizeof(DWORD), default_pov_name(povnum), "Up"),
				input_item_id(povnum * 4 + ITEM_ID_HAT1UP),
				&dinput_joystick_device::pov_get_state,
				reinterpret_cast<void *>(uintptr_t(povnum * 4 + POVDIR_UP)));

		// down
		device.add_item(
				item_name(offsetof(DIJOYSTATE2, rgdwPOV) + povnum * sizeof(DWORD), default_pov_name(povnum), "Down"),
				input_item_id(povnum * 4 + ITEM_ID_HAT1DOWN),
				&dinput_joystick_device::pov_get_state,
				reinterpret_cast<void *>(uintptr_t(povnum * 4 + POVDIR_DOWN)));
	}

	// populate the buttons
	for (uint32_t butnum = 0; butnum < m_caps.dwButtons; butnum++)
	{
		auto offset = reinterpret_cast<uintptr_t>(&static_cast<DIJOYSTATE2 *>(nullptr)->rgbButtons[butnum]);

		input_item_id itemid;
		if (butnum < INPUT_MAX_BUTTONS)
			itemid = input_item_id(ITEM_ID_BUTTON1 + butnum);
		else if (butnum < INPUT_MAX_BUTTONS + INPUT_MAX_ADD_SWITCH)
			itemid = input_item_id(ITEM_ID_ADD_SWITCH1 - INPUT_MAX_BUTTONS + butnum);
		else
			itemid = ITEM_ID_OTHER_SWITCH;

		device.add_item(
				item_name(offset, default_button_name(butnum), nullptr),
				itemid,
				generic_button_get_state<BYTE>,
				&m_joystick.state.rgbButtons[butnum]);
	}
}

int32_t dinput_joystick_device::pov_get_state(void *device_internal, void *item_internal)
{
	auto *const devinfo = static_cast<dinput_joystick_device *>(device_internal);
	int const povnum = uintptr_t(item_internal) / 4;
	int const povdir = uintptr_t(item_internal) % 4;

	// get the current state
	devinfo->module().poll_if_necessary();
	DWORD const pov = devinfo->m_joystick.state.rgdwPOV[povnum];

	// if invalid, return 0
	if ((pov & 0xffff) == 0xffff)
		return 0;

	// return the current state
	switch (povdir)
	{
	case POVDIR_LEFT:   return (pov >= 22500) && (pov <= 31500);
	case POVDIR_RIGHT:  return (pov >=  4500) && (pov <= 13500);
	case POVDIR_UP:     return (pov >= 31500) || (pov <=  4500);
	case POVDIR_DOWN:   return (pov >= 13500) && (pov <= 22500);
	}

	return 0;
}


//============================================================
//  dinput_api_helper - DirectInput API helper
//============================================================

dinput_api_helper::dinput_api_helper()
{
}

dinput_api_helper::~dinput_api_helper()
{
}

int dinput_api_helper::initialize()
{
	HRESULT result = DirectInput8Create(GetModuleHandleUni(), DIRECTINPUT_VERSION, IID_IDirectInput8, reinterpret_cast<void **>(m_dinput.GetAddressOf()), nullptr);
	if (result != DI_OK)
	{
		return result;
	}

	osd_printf_verbose("DirectInput: Using DirectInput %d\n", DIRECTINPUT_VERSION >> 8);
	return 0;
}


HRESULT dinput_api_helper::enum_attached_devices(int devclass, device_enum_interface &enumerate_interface) const
{
	return m_dinput->EnumDevices(devclass, device_enum_interface_callback, &enumerate_interface, DIEDFL_ATTACHEDONLY);
}


std::pair<Microsoft::WRL::ComPtr<IDirectInputDevice8>, LPCDIDATAFORMAT> dinput_api_helper::open_device(
		LPCDIDEVICEINSTANCE instance,
		LPCDIDATAFORMAT format1,
		LPCDIDATAFORMAT format2,
		dinput_cooperative_level cooperative_level)
{
	HRESULT result;

	// attempt to create a device
	Microsoft::WRL::ComPtr<IDirectInputDevice8> device;
	result = m_dinput->CreateDevice(instance->guidInstance, device.GetAddressOf(), nullptr);
	if (result != DI_OK)
	{
		osd_printf_error("DirectInput: Unable to create device.\n");
		return std::make_pair(nullptr, nullptr);
	}

	// attempt to set the data format
	LPCDIDATAFORMAT format = format1;
	result = device->SetDataFormat(format);
	if ((result != DI_OK) && format2)
	{
		// use the secondary format if available
		osd_printf_verbose("DirectInput: Error setting primary data format, trying secondary format.\n");
		format = format2;
		result = device->SetDataFormat(format);
	}
	if (result != DI_OK)
	{
		osd_printf_error("DirectInput: Unable to set data format.\n");
		return std::make_pair(nullptr, nullptr);
	}

	// default window to the first window in the list
	HWND window_handle;
	DWORD di_cooperative_level;
#if defined(OSD_WINDOWS)
	auto const &window = dynamic_cast<win_window_info &>(*osd_common_t::window_list().front());
	bool const standalone_window = !window.attached_mode();
#elif defined(SDLMAME_WIN32)
	auto const &window = dynamic_cast<sdl_window_info &>(*osd_common_t::window_list().front());
	bool const standalone_window = true;
#endif
	if (!standalone_window)
	{
		// in attached mode we have to ignore the caller and hook up to the desktop window
		window_handle = GetDesktopWindow();
		di_cooperative_level = DISCL_BACKGROUND | DISCL_NONEXCLUSIVE;
	}
	else
	{
#if defined(OSD_WINDOWS)
		window_handle = window.platform_window();
#elif defined(SDLMAME_WIN32)
		auto const sdlwindow = window.platform_window();
		SDL_SysWMinfo info;
		SDL_VERSION(&info.version);
		if (!SDL_GetWindowWMInfo(sdlwindow, &info))
			return std::make_pair(nullptr, nullptr);
		window_handle = info.info.win.window;
#endif
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
	result = device->SetCooperativeLevel(window_handle, di_cooperative_level);
	if (result != DI_OK)
	{
		osd_printf_error("DirectInput: Unable to set cooperative level.\n");
		return std::make_pair(nullptr, nullptr);
	}

	// return new device
	return std::make_pair(std::move(device), format);
}

} // namespace osd


#else // defined(OSD_WINDOWS) || defined(SDLMAME_WIN32)

#include "input_module.h"

namespace osd {

namespace {

MODULE_NOT_SUPPORTED(keyboard_input_dinput, OSD_KEYBOARDINPUT_PROVIDER, "dinput")
MODULE_NOT_SUPPORTED(mouse_input_dinput, OSD_MOUSEINPUT_PROVIDER, "dinput")
MODULE_NOT_SUPPORTED(joystick_input_dinput, OSD_JOYSTICKINPUT_PROVIDER, "dinput")

} // anonymous namespace

} // namespace osd

#endif // defined(OSD_WINDOWS) || defined(SDLMAME_WIN32)

MODULE_DEFINITION(KEYBOARDINPUT_DINPUT, osd::keyboard_input_dinput)
MODULE_DEFINITION(MOUSEINPUT_DINPUT, osd::mouse_input_dinput)
MODULE_DEFINITION(JOYSTICKINPUT_DINPUT, osd::joystick_input_dinput)
