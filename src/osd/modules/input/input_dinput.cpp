// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Brad Hughes, Vas Crabb
//============================================================
//
//  input_dinput.cpp - Windows DirectInput support
//
//============================================================
/*

DirectInput joystick input is a bit of a mess.  It gives eight axes
called X, Y, Z, Rx, Ry, Rz, slider 0 and slider 1.  The driver can
assign arbitrary physical axes to these axes.  Up to four hat switches
are supported, giving a direction in hundredths of degrees.  In theory,
this supports dial-like controls with an arbitrary number of stops.  In
practice, it just makes dealing with 8-way hat switches more complicated
and prevents contradictory inputs from being reported altogether.

You may get a vague indication of the type of controller, and you can
obtain usage information for HID controllers.

The Windows HID driver supposedly uses the following mappings:

0x01 Generic Desktop    0x30 X              X
0x01 Generic Desktop    0x31 Y              Y
0x01 Generic Desktop    0x32 Z              Z
0x01 Generic Desktop    0x33 Rx             Rx
0x01 Generic Desktop    0x34 Ry             Ry
0x01 Generic Desktop    0x35 Rz             Rz
0x01 Generic Desktop    0x36 Slider         Slider
0x01 Generic Desktop    0x37 Dial           Slider
0x01 Generic Desktop    0x39 Hat Switch     POV Hat
0x02 Simulation         0xBA Rudder         Rz
0x02 Simulation         0xBB Throttle       Slider
0x02 Simulation         0xC4 Accelerator    Y
0x02 Simulation         0xC5 Brake          Rz
0x02 Simulation         0xC8 Steering       X

Anything without an explicit mapping is treated as a button.

The WinMM driver supposedly uses the following axis mappings:

X   X
Y   Y
Z   Slider
R   Rz
U   Slider
V   Slider

The actual mapping used by various controllers doesn't match what you
might expect from the HID mapping.

Gamepads:

Axis        Logitech        DS4             Xinput          Switch
X           Left X          Left X          Left X          Left X
Y           Left Y          Left Y          Left Y          Left Y
Z           Right X         Right X         Triggers
Rx                          Left trigger    Right X         Right X
Ry                          Right trigger   Right Y         Right Y
Rz          Right Y         Right Y

Thrustmaster controllers:

Axis        HOTAS           Side stick      Throttle/Pedals     Dual Throttles      Triple Throttles    Driving
X           Aileron         Aileron         Mini Stick X        Left Throttle       Right Brake         Steering
Y           Elevator        Elevator        Mini stick Y        Right Throttle      Left Brake          Brake
Z           Throttle                        Throttle            Flaps               Rudder
Rx          Right Brake                     Right Brake         Right Brake         Left Throttle
Ry          Left Brake                      Left Brake          Left Brake          Centre Throttle
Rz          Twist           Rudder          Rocker              Air Brake           Right Throttle      Accelerator
Slider 0    Rocker          Throttle        Antenna             Rudder                                  Clutch
Slider 1    Rudder                          Rudder

Logitech controllers:

Axis        Pro Wheels
X           Steering
Y
Z
Rx          Accelerator
Ry          Brake
Rz          Clutch
Slider 0
Slider 1

MFG Crosswind pedals:

X           Left Brake
Y           Right Brake
Rz          Rudder

*/

#include "modules/osdmodule.h"

#if defined(OSD_WINDOWS) || defined(SDLMAME_WIN32)

#include "input_dinput.h"

#include "interface/inputseq.h"
#include "windows/winutil.h"

// emu
#include "inpttype.h"

// lib/util
#include "util/corestr.h"

#ifdef SDLMAME_WIN32
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#endif

#include <algorithm>
#include <cmath>
#include <iterator>
#include <memory>

// standard windows headers
#include <initguid.h>
#include <tchar.h>


namespace osd {

namespace {

std::string guid_to_string(GUID const &guid)
{
	// size of a GUID string with dashes plus NUL terminator
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



//============================================================
//  dinput_keyboard_device - DirectInput keyboard device
//============================================================

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

	virtual void poll(bool relative_reset) override;
	virtual void reset() override;
	virtual void configure(input_device &device) override;

private:
	std::mutex      m_device_lock;
	keyboard_state  m_keyboard;
};

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

void dinput_keyboard_device::poll(bool relative_reset)
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
	for (unsigned keynum = 0; keynum < MAX_KEYS; keynum++)
	{
		input_item_id itemid = keyboard_trans_table::instance().map_di_scancode_to_itemid(keynum);

		// generate/fetch the name
		snprintf(defname, std::size(defname), "Scan%03d", keynum);

		// add the item to the device
		device.add_item(
				item_name(keynum, defname, nullptr),
				strmakeupper(defname),
				itemid,
				generic_button_get_state<uint8_t>,
				&m_keyboard.state[keynum]);
	}
}


//============================================================
//  dinput_mouse_device - DirectInput mouse device
//============================================================

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

	void poll(bool relative_reset) override;
	void reset() override;
	virtual void configure(input_device &device) override;

private:
	DIMOUSESTATE2 m_mouse;
};

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

void dinput_mouse_device::poll(bool relative_reset)
{
	// poll
	if (relative_reset && (poll_dinput(&m_mouse) == DI_OK))
	{
		// scale the axis data
		m_mouse.lX *= input_device::RELATIVE_PER_PIXEL;
		m_mouse.lY *= input_device::RELATIVE_PER_PIXEL;
		m_mouse.lZ *= input_device::RELATIVE_PER_PIXEL;
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
				std::string_view(),
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
				std::string_view(),
				input_item_id(ITEM_ID_BUTTON1 + butnum),
				generic_button_get_state<BYTE>,
				&m_mouse.rgbButtons[butnum]);
	}
}


//============================================================
//  dinput_module - base DirectInput module
//============================================================

class dinput_module : public input_module_impl<dinput_device, osd_common_t>
{
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

		HRESULT const result = m_dinput_helper->enum_attached_devices(
				dinput_devclass(),
				[this] (LPCDIDEVICEINSTANCE instance) { return device_enum_callback(instance); });
		if (result != DI_OK)
			fatalerror("DirectInput: Unable to enumerate devices (result=%08X)\n", uint32_t(result));
	}

protected:
	virtual int dinput_devclass() = 0;
	virtual BOOL device_enum_callback(LPCDIDEVICEINSTANCE instance) = 0;

	std::unique_ptr<dinput_api_helper> m_dinput_helper;
};


class keyboard_input_dinput : public dinput_module
{
public:
	keyboard_input_dinput() :
		dinput_module(OSD_KEYBOARDINPUT_PROVIDER, "dinput")
	{
	}

protected:
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

protected:
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

protected:
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
//  dinput_device - base DirectInput device
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

void dinput_joystick_device::poll(bool relative_reset)
{
	// poll the device first
	if (dinput_device::poll_dinput(&m_joystick.state) == DI_OK)
	{
		// normalize axis values
		for (int axisnum = 0; axisnum < 8; axisnum++)
		{
			auto const range = m_joystick.rangemax[axisnum] - m_joystick.rangemin[axisnum];
			if (range)
			{
				// assumes output range is symmetrical
				LONG *const axis = &m_joystick.state.lX + axisnum;
				double const offset = *axis - m_joystick.rangemin[axisnum];
				double const scaled = offset * double(input_device::ABSOLUTE_MAX - input_device::ABSOLUTE_MIN) / double(range);
				*axis = lround(std::clamp<double>(scaled + input_device::ABSOLUTE_MIN, input_device::ABSOLUTE_MIN, input_device::ABSOLUTE_MAX));
			}
		}
	}
}

void dinput_joystick_device::configure(input_device &device)
{
	input_device::assignment_vector assignments;
	HRESULT result;

	// get device information - it gives clues about axis usage
	DIDEVICEINSTANCE info;
	info.dwSize = sizeof(info);
	result = m_device->GetDeviceInfo(&info);
	bool hid = false;
	uint8_t type = DI8DEVTYPE_DEVICE;
	uint8_t subtype = 0;
	if (result == DI_OK)
	{
		hid = (info.dwDevType & DIDEVTYPE_HID) != 0;
		type = info.dwDevType & 0x00ff;
		subtype = (info.dwDevType >> 8) & 0x00ff;
		osd_printf_verbose(
				"DirectInput: Device type=0x%02X subtype=0x%02X HID=%u\n",
				type,
				subtype,
				hid ? "yes" : "no");
	}

	// turn off deadzone; we do our own calculations
	result = dinput_api_helper::set_dword_property(m_device, DIPROP_DEADZONE, 0, DIPH_DEVICE, 0);
	if ((result != DI_OK) && (result != DI_PROPNOEFFECT))
		osd_printf_warning("DirectInput: Unable to reset deadzone for joystick %s.\n", name());

	// turn off saturation; we do our own calculations
	result = dinput_api_helper::set_dword_property(m_device, DIPROP_SATURATION, 0, DIPH_DEVICE, 10000);
	if ((result != DI_OK) && (result != DI_PROPNOEFFECT))
		osd_printf_warning("DirectInput: Unable to reset saturation for joystick %s.\n", name());

	// populate the axes
	input_item_id axisitems[8];
	std::fill(std::begin(axisitems), std::end(axisitems), ITEM_ID_INVALID);
	for (uint32_t axisnum = 0, axiscount = 0; (axiscount < m_caps.dwAxes) && (axisnum < 8); axisnum++)
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
			// this is normal when axes are skipped, e.g. X/Y/Z present, rX/rY absent, rZ present
			osd_printf_verbose("DirectInput: Unable to get properties for joystick %s axis %u.\n", name(), axisnum);
			continue;
		}

		m_joystick.rangemin[axisnum] = dipr.lMin;
		m_joystick.rangemax[axisnum] = dipr.lMax;

		// populate the item description as well
		axisitems[axisnum] = device.add_item(
				item_name(offsetof(DIJOYSTATE2, lX) + axisnum * sizeof(LONG), default_axis_name[axisnum], nullptr),
				std::string_view(),
				input_item_id(ITEM_ID_XAXIS + axisnum),
				generic_axis_get_state<LONG>,
				&m_joystick.state.lX + axisnum);

		axiscount++;
	}

	// take a guess at which axes might be pedals depending on type and remap onto negative half of range
	input_item_id pedalitems[3] = { ITEM_ID_INVALID, ITEM_ID_INVALID, ITEM_ID_INVALID };
	if (DI8DEVTYPE_FLIGHT == type)
	{
		// Rx/Ry are often used for brakes
		bool const rxpedal = (ITEM_ID_INVALID != axisitems[3]) && !m_joystick.rangemin[3] && (0 < m_joystick.rangemax[3]);
		bool const rypedal = (ITEM_ID_INVALID != axisitems[4]) && !m_joystick.rangemin[4] && (0 < m_joystick.rangemax[4]);
		if (rxpedal && rypedal)
		{
			pedalitems[0] = axisitems[3];
			m_joystick.rangemin[3] = m_joystick.rangemax[3];
			m_joystick.rangemax[3] = -m_joystick.rangemax[3];

			pedalitems[1] = axisitems[4];
			m_joystick.rangemin[4] = m_joystick.rangemax[4];
			m_joystick.rangemax[4] = -m_joystick.rangemax[4];
		}
	}
	else if (DI8DEVTYPE_DRIVING == type)
	{
		bool const ypedal = (ITEM_ID_INVALID != axisitems[1]) && !m_joystick.rangemin[1] && (0 < m_joystick.rangemax[1]);
		bool const rxpedal = (ITEM_ID_INVALID != axisitems[3]) && !m_joystick.rangemin[3] && (0 < m_joystick.rangemax[3]);
		bool const rypedal = (ITEM_ID_INVALID != axisitems[4]) && !m_joystick.rangemin[4] && (0 < m_joystick.rangemax[4]);
		bool const rzpedal = (ITEM_ID_INVALID != axisitems[5]) && !m_joystick.rangemin[5] && (0 < m_joystick.rangemax[5]);
		bool const s0pedal = (ITEM_ID_INVALID != axisitems[6]) && !m_joystick.rangemin[6] && (0 < m_joystick.rangemax[6]);
		if (DI8DEVTYPEDRIVING_DUALPEDALS == subtype)
		{
			// dual pedals are usually Y and Rz
			if (ypedal && rzpedal)
			{
				pedalitems[0] = axisitems[1];
				m_joystick.rangemin[1] = m_joystick.rangemax[1];
				m_joystick.rangemax[1] = -m_joystick.rangemax[1];

				pedalitems[1] = axisitems[5];
				m_joystick.rangemin[5] = m_joystick.rangemax[5];
				m_joystick.rangemax[5] = -m_joystick.rangemax[5];
			}
		}
		else if (DI8DEVTYPEDRIVING_THREEPEDALS == subtype)
		{
			// triple pedals may be Y, Rz and slider 0, or Rx, Ry and Rz
			if (ypedal && rzpedal && s0pedal)
			{
				pedalitems[0] = axisitems[1];
				m_joystick.rangemin[1] = m_joystick.rangemax[1];
				m_joystick.rangemax[1] = -m_joystick.rangemax[1];

				pedalitems[1] = axisitems[5];
				m_joystick.rangemin[5] = m_joystick.rangemax[5];
				m_joystick.rangemax[5] = -m_joystick.rangemax[5];

				pedalitems[2] = axisitems[6];
				m_joystick.rangemin[6] = m_joystick.rangemax[6];
				m_joystick.rangemax[6] = -m_joystick.rangemax[6];
			}
			else if (rxpedal && rypedal && rzpedal)
			{
				pedalitems[0] = axisitems[3];
				m_joystick.rangemin[3] = m_joystick.rangemax[3];
				m_joystick.rangemax[3] = -m_joystick.rangemax[3];

				pedalitems[1] = axisitems[4];
				m_joystick.rangemin[4] = m_joystick.rangemax[4];
				m_joystick.rangemax[4] = -m_joystick.rangemax[4];

				pedalitems[2] = axisitems[5];
				m_joystick.rangemin[5] = m_joystick.rangemax[5];
				m_joystick.rangemax[5] = -m_joystick.rangemax[5];
			}
		}
	}

	// populate the POV hats
	input_item_id povitems[4][4];
	for (auto &pov : povitems)
		std::fill(std::begin(pov), std::end(pov), ITEM_ID_INVALID);
	for (uint32_t povnum = 0; povnum < m_caps.dwPOVs; povnum++)
	{
		// left
		povitems[povnum][0] = device.add_item(
				item_name(offsetof(DIJOYSTATE2, rgdwPOV) + povnum * sizeof(DWORD), default_pov_name(povnum), "Left"),
				std::string_view(),
				input_item_id((povnum * 4) + ITEM_ID_HAT1LEFT),
				&dinput_joystick_device::pov_get_state,
				reinterpret_cast<void *>(uintptr_t(povnum * 4 + POVDIR_LEFT)));

		// right
		povitems[povnum][1] = device.add_item(
				item_name(offsetof(DIJOYSTATE2, rgdwPOV) + povnum * sizeof(DWORD), default_pov_name(povnum), "Right"),
				std::string_view(),
				input_item_id((povnum * 4) + ITEM_ID_HAT1RIGHT),
				&dinput_joystick_device::pov_get_state,
				reinterpret_cast<void *>(uintptr_t(povnum * 4 + POVDIR_RIGHT)));

		// up
		povitems[povnum][2] = device.add_item(
				item_name(offsetof(DIJOYSTATE2, rgdwPOV) + povnum * sizeof(DWORD), default_pov_name(povnum), "Up"),
				std::string_view(),
				input_item_id((povnum * 4) + ITEM_ID_HAT1UP),
				&dinput_joystick_device::pov_get_state,
				reinterpret_cast<void *>(uintptr_t(povnum * 4 + POVDIR_UP)));

		// down
		povitems[povnum][3] = device.add_item(
				item_name(offsetof(DIJOYSTATE2, rgdwPOV) + povnum * sizeof(DWORD), default_pov_name(povnum), "Down"),
				std::string_view(),
				input_item_id((povnum * 4) + ITEM_ID_HAT1DOWN),
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

		input_item_id const actual = device.add_item(
				item_name(offset, default_button_name(butnum), nullptr),
				std::string_view(),
				itemid,
				generic_button_get_state<BYTE>,
				&m_joystick.state.rgbButtons[butnum]);

		// there are sixteen action button types
		if (butnum < 16)
		{
			input_seq const seq(make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, actual));
			assignments.emplace_back(ioport_type(IPT_BUTTON1 + butnum), SEQ_TYPE_STANDARD, seq);

			// assign the first few buttons to UI actions and pedals
			// TODO: don't map pedals for driving controls that have them present
			switch (butnum)
			{
			case 0:
				assignments.emplace_back(IPT_PEDAL, SEQ_TYPE_INCREMENT, seq);
				assignments.emplace_back(IPT_UI_SELECT, SEQ_TYPE_STANDARD, seq);
				break;
			case 1:
				assignments.emplace_back(IPT_PEDAL2, SEQ_TYPE_INCREMENT, seq);
				assignments.emplace_back((3 > m_caps.dwButtons) ? IPT_UI_CLEAR : IPT_UI_BACK, SEQ_TYPE_STANDARD, seq);
				break;
			case 2:
				assignments.emplace_back(IPT_PEDAL3, SEQ_TYPE_INCREMENT, seq);
				assignments.emplace_back(IPT_UI_CLEAR, SEQ_TYPE_STANDARD, seq);
				break;
			case 3:
				assignments.emplace_back(IPT_UI_HELP, SEQ_TYPE_STANDARD, seq);
				break;
			}
		}
	}

	// add default assignments depending on type
	if (DI8DEVTYPE_FLIGHT == type)
	{
		if (((ITEM_ID_INVALID == axisitems[0]) || (ITEM_ID_INVALID == axisitems[1])) && (1 <= m_caps.dwPOVs))
		{
			// X or Y missing, fall back to using POV hat for navigation
			add_directional_assignments(
					assignments,
					axisitems[0],
					axisitems[1],
					povitems[0][0],
					povitems[0][1],
					povitems[0][2],
					povitems[0][3]);

			// try using throttle for zoom/focus
			if (ITEM_ID_INVALID != axisitems[2])
			{
				input_seq const negseq(make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_NEG, axisitems[2]));
				input_seq const posseq(make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_POS, axisitems[2]));
				if (ITEM_ID_INVALID != axisitems[5])
				{
					assignments.emplace_back(IPT_UI_ZOOM_IN, SEQ_TYPE_STANDARD, posseq);
					assignments.emplace_back(IPT_UI_ZOOM_OUT, SEQ_TYPE_STANDARD, negseq);
					assignments.emplace_back(IPT_UI_FOCUS_PREV, SEQ_TYPE_STANDARD, negseq);
					assignments.emplace_back(IPT_UI_FOCUS_NEXT, SEQ_TYPE_STANDARD, posseq);
				}
				else
				{
					assignments.emplace_back(IPT_UI_PREV_GROUP, SEQ_TYPE_STANDARD, posseq);
					assignments.emplace_back(IPT_UI_NEXT_GROUP, SEQ_TYPE_STANDARD, negseq);
				}
			}

			// try using twist/rudder for next/previous group
			if (ITEM_ID_INVALID != axisitems[5])
			{
				input_seq const negseq(make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_NEG, axisitems[5]));
				input_seq const posseq(make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_POS, axisitems[5]));
				assignments.emplace_back(IPT_UI_PREV_GROUP, SEQ_TYPE_STANDARD, negseq);
				assignments.emplace_back(IPT_UI_NEXT_GROUP, SEQ_TYPE_STANDARD, posseq);
			}
		}
		else
		{
			// only use stick for primary navigation/movement
			add_directional_assignments(
					assignments,
					axisitems[0],
					axisitems[1],
					ITEM_ID_INVALID,
					ITEM_ID_INVALID,
					ITEM_ID_INVALID,
					ITEM_ID_INVALID);

			// try using hat for secondary navigation functions
			if (1 <= m_caps.dwPOVs)
			{
				input_seq const leftseq(make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, povitems[0][0]));
				input_seq const rightseq(make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, povitems[0][1]));
				input_seq const upseq(make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, povitems[0][2]));
				input_seq const downseq(make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, povitems[0][3]));
				if ((ITEM_ID_INVALID != axisitems[2]) || (ITEM_ID_INVALID != axisitems[5]))
				{
					assignments.emplace_back(IPT_UI_PREV_GROUP, SEQ_TYPE_STANDARD, leftseq);
					assignments.emplace_back(IPT_UI_NEXT_GROUP, SEQ_TYPE_STANDARD, rightseq);
					assignments.emplace_back(IPT_UI_PAGE_UP,    SEQ_TYPE_STANDARD, upseq);
					assignments.emplace_back(IPT_UI_PAGE_DOWN,  SEQ_TYPE_STANDARD, downseq);
				}
				else
				{
					assignments.emplace_back(IPT_UI_FOCUS_PREV, SEQ_TYPE_STANDARD, leftseq);
					assignments.emplace_back(IPT_UI_FOCUS_NEXT, SEQ_TYPE_STANDARD, rightseq);
					assignments.emplace_back(IPT_UI_ZOOM_OUT,   SEQ_TYPE_STANDARD, leftseq);
					assignments.emplace_back(IPT_UI_ZOOM_IN,    SEQ_TYPE_STANDARD, rightseq);
					assignments.emplace_back(IPT_UI_PREV_GROUP, SEQ_TYPE_STANDARD, upseq);
					assignments.emplace_back(IPT_UI_NEXT_GROUP, SEQ_TYPE_STANDARD, downseq);
				}
			}

			// try using throttle for zoom
			if (ITEM_ID_INVALID != axisitems[2])
			{
				input_seq const negseq(make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_NEG, axisitems[2]));
				input_seq const posseq(make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_POS, axisitems[2]));
				if ((1 <= m_caps.dwPOVs) || (ITEM_ID_INVALID != axisitems[5]))
				{
					assignments.emplace_back(IPT_UI_ZOOM_IN, SEQ_TYPE_STANDARD, posseq);
					assignments.emplace_back(IPT_UI_ZOOM_OUT, SEQ_TYPE_STANDARD, negseq);
					if ((1 > m_caps.dwPOVs) || (ITEM_ID_INVALID == axisitems[5]))
					{
						assignments.emplace_back(IPT_UI_FOCUS_PREV, SEQ_TYPE_STANDARD, negseq);
						assignments.emplace_back(IPT_UI_FOCUS_NEXT, SEQ_TYPE_STANDARD, posseq);
					}
				}
				else
				{
					assignments.emplace_back(IPT_UI_PREV_GROUP, SEQ_TYPE_STANDARD, posseq);
					assignments.emplace_back(IPT_UI_NEXT_GROUP, SEQ_TYPE_STANDARD, negseq);
				}
			}

			// try using twist/rudder for focus next/previous
			if (ITEM_ID_INVALID != axisitems[5])
			{
				input_seq const negseq(make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_NEG, axisitems[5]));
				input_seq const posseq(make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_POS, axisitems[5]));
				if (1 <= m_caps.dwPOVs)
				{
					assignments.emplace_back(IPT_UI_FOCUS_PREV, SEQ_TYPE_STANDARD, negseq);
					assignments.emplace_back(IPT_UI_FOCUS_NEXT, SEQ_TYPE_STANDARD, posseq);
					if (ITEM_ID_INVALID == axisitems[2])
					{
						assignments.emplace_back(IPT_UI_ZOOM_IN, SEQ_TYPE_STANDARD, posseq);
						assignments.emplace_back(IPT_UI_ZOOM_OUT, SEQ_TYPE_STANDARD, negseq);
					}
				}
				else
				{
					assignments.emplace_back(IPT_UI_PREV_GROUP, SEQ_TYPE_STANDARD, negseq);
					assignments.emplace_back(IPT_UI_NEXT_GROUP, SEQ_TYPE_STANDARD, posseq);
				}
			}
		}

		// Z or slider 0 is usually the throttle - use one of them for joystick Z
		add_assignment(
				assignments,
				IPT_AD_STICK_Z,
				SEQ_TYPE_STANDARD,
				ITEM_CLASS_ABSOLUTE,
				ITEM_MODIFIER_NONE,
				{ axisitems[2], axisitems[6] });

		// use Z for the first two pedals if present
		if (ITEM_ID_INVALID != axisitems[2])
		{
			// TODO: use Rx/Ry as well if they appear to be brakes
			input_seq const pedal1seq(make_code(ITEM_CLASS_ABSOLUTE, ITEM_MODIFIER_POS, axisitems[2]));
			input_seq const pedal2seq(make_code(ITEM_CLASS_ABSOLUTE, ITEM_MODIFIER_NEG, axisitems[2]));
			assignments.emplace_back(IPT_PEDAL, SEQ_TYPE_STANDARD, pedal1seq);
			assignments.emplace_back(IPT_PEDAL2, SEQ_TYPE_STANDARD, pedal2seq);
		}
	}
	else if (DI8DEVTYPE_DRIVING == type)
	{
		// use the wheel and D-pad for navigation and directional controls
		add_directional_assignments(
				assignments,
				axisitems[0],
				ITEM_ID_INVALID,
				povitems[0][0],
				povitems[0][1],
				povitems[0][2],
				povitems[0][3]);

		// check subtype to determine how pedals should be assigned
		if (DI8DEVTYPEDRIVING_COMBINEDPEDALS == subtype)
		{
			if (ITEM_ID_INVALID != axisitems[1])
			{
				// put first two pedals on opposite sides of Y axis
				assignments.emplace_back(
						IPT_PEDAL,
						SEQ_TYPE_STANDARD,
						input_seq(make_code(ITEM_CLASS_ABSOLUTE, ITEM_MODIFIER_POS, axisitems[1])));
				assignments.emplace_back(
						IPT_PEDAL2,
						SEQ_TYPE_STANDARD,
						input_seq(make_code(ITEM_CLASS_ABSOLUTE, ITEM_MODIFIER_NEG, axisitems[1])));

				// use for previous/next group as well
				assignments.emplace_back(
						IPT_UI_NEXT_GROUP,
						SEQ_TYPE_STANDARD,
						input_seq(make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_POS, axisitems[1])));
				assignments.emplace_back(
						IPT_UI_PREV_GROUP,
						SEQ_TYPE_STANDARD,
						input_seq(make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_NEG, axisitems[1])));
			}
		}
		else
		{
			// see if we have individual pedals
			if (ITEM_ID_INVALID != pedalitems[0])
			{
				assignments.emplace_back(
						IPT_PEDAL,
						SEQ_TYPE_STANDARD,
						input_seq(make_code(ITEM_CLASS_ABSOLUTE, ITEM_MODIFIER_NEG, pedalitems[0])));
				assignments.emplace_back(
						IPT_UI_NEXT_GROUP,
						SEQ_TYPE_STANDARD,
						input_seq(make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_NEG, pedalitems[0])));
			}
			if (ITEM_ID_INVALID != pedalitems[1])
			{
				assignments.emplace_back(
						IPT_PEDAL2,
						SEQ_TYPE_STANDARD,
						input_seq(make_code(ITEM_CLASS_ABSOLUTE, ITEM_MODIFIER_NEG, pedalitems[1])));
				assignments.emplace_back(
						IPT_UI_PREV_GROUP,
						SEQ_TYPE_STANDARD,
						input_seq(make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_NEG, pedalitems[1])));
			}
			if (ITEM_ID_INVALID != pedalitems[2])
			{
				assignments.emplace_back(
						IPT_PEDAL3,
						SEQ_TYPE_STANDARD,
						input_seq(make_code(ITEM_CLASS_ABSOLUTE, ITEM_MODIFIER_NEG, pedalitems[2])));
				assignments.emplace_back(
						IPT_UI_FOCUS_NEXT,
						SEQ_TYPE_STANDARD,
						input_seq(make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_NEG, pedalitems[2])));
			}
		}
	}
	else
	{
		// assume this is a gamepad - see if it looks like it has dual analog sticks
		input_item_id stickaxes[2][2] = {
				{ axisitems[0], axisitems[1] },
				{ ITEM_ID_INVALID, ITEM_ID_INVALID } };
		input_item_id pedalaxis = ITEM_ID_INVALID;
		if ((ITEM_ID_INVALID != axisitems[2]) && (ITEM_ID_INVALID != axisitems[5]))
		{
			// assume Z/Rz are right stick
			stickaxes[1][0] = axisitems[2];
			stickaxes[1][1] = axisitems[5];
			add_twin_stick_assignments(
					assignments,
					stickaxes[0][0],
					stickaxes[0][1],
					stickaxes[1][0],
					stickaxes[1][1],
					ITEM_ID_INVALID,
					ITEM_ID_INVALID,
					ITEM_ID_INVALID,
					ITEM_ID_INVALID,
					ITEM_ID_INVALID,
					ITEM_ID_INVALID,
					ITEM_ID_INVALID,
					ITEM_ID_INVALID);
		}
		else if ((ITEM_ID_INVALID != axisitems[3]) && (ITEM_ID_INVALID != axisitems[4]))
		{
			// assume Rx/Ry are right stick and Z is triggers if present
			stickaxes[1][0] = axisitems[3];
			stickaxes[1][1] = axisitems[4];
			pedalaxis = axisitems[2];
			add_twin_stick_assignments(
					assignments,
					stickaxes[0][0],
					stickaxes[0][1],
					stickaxes[1][0],
					stickaxes[1][1],
					ITEM_ID_INVALID,
					ITEM_ID_INVALID,
					ITEM_ID_INVALID,
					ITEM_ID_INVALID,
					ITEM_ID_INVALID,
					ITEM_ID_INVALID,
					ITEM_ID_INVALID,
					ITEM_ID_INVALID);
		}
		else
		{
			// if Z is present, use it as secondary Y
			stickaxes[1][1] = axisitems[2];
		}

		// try to find a "complete" stick for primary movement controls
		input_item_id diraxis[2][2];
		choose_primary_stick(diraxis, stickaxes[0][0], stickaxes[0][1], stickaxes[1][0], stickaxes[1][1]);
		add_directional_assignments(
				assignments,
				diraxis[0][0],
				diraxis[0][1],
				povitems[0][0],
				povitems[0][1],
				povitems[0][2],
				povitems[0][3]);

		// assign a secondary stick axis to joystick Z
		add_assignment(
				assignments,
				IPT_AD_STICK_Z,
				SEQ_TYPE_STANDARD,
				ITEM_CLASS_ABSOLUTE,
				ITEM_MODIFIER_NONE,
				{ diraxis[1][1], diraxis[1][0] });

		// try to find a suitable axis to use for the first two pedals
		add_assignment(
				assignments,
				IPT_PEDAL,
				SEQ_TYPE_STANDARD,
				ITEM_CLASS_ABSOLUTE,
				ITEM_MODIFIER_NEG,
				{ pedalaxis, diraxis[1][1], diraxis[0][1] });
		add_assignment(
				assignments,
				IPT_PEDAL2,
				SEQ_TYPE_STANDARD,
				ITEM_CLASS_ABSOLUTE,
				ITEM_MODIFIER_POS,
				{ pedalaxis, diraxis[1][1], diraxis[0][1] });

		// try to choose an axis for previous/next group
		if (ITEM_ID_INVALID != pedalaxis)
		{
			// this is reversed because right trigger is negative direction
			assignments.emplace_back(
					IPT_UI_PREV_GROUP,
					SEQ_TYPE_STANDARD,
					input_seq(make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_POS, pedalaxis)));
			assignments.emplace_back(
					IPT_UI_NEXT_GROUP,
					SEQ_TYPE_STANDARD,
					input_seq(make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_NEG, pedalaxis)));
			pedalaxis = ITEM_ID_INVALID;
		}
		else if (consume_axis_pair(assignments, IPT_UI_PREV_GROUP, IPT_UI_NEXT_GROUP, diraxis[1][1]))
		{
			// took secondary Y
		}
		else if (consume_axis_pair(assignments, IPT_UI_PREV_GROUP, IPT_UI_NEXT_GROUP, diraxis[1][0]))
		{
			// took secondary X
		}

		// use secondary Y for page up/down if available
		consume_axis_pair(assignments, IPT_UI_PAGE_UP, IPT_UI_PAGE_DOWN, diraxis[1][1]);

		// put focus previous/next and zoom on secondary X if available
		add_axis_pair_assignment(assignments, IPT_UI_FOCUS_PREV, IPT_UI_FOCUS_NEXT, diraxis[1][0]);
		add_axis_pair_assignment(assignments, IPT_UI_ZOOM_OUT, IPT_UI_ZOOM_IN, diraxis[1][0]);
	}

	// set default assignments
	device.set_default_assignments(std::move(assignments));
}

int32_t dinput_joystick_device::pov_get_state(void *device_internal, void *item_internal)
{
	auto *const devinfo = static_cast<dinput_joystick_device *>(device_internal);
	int const povnum = uintptr_t(item_internal) / 4;
	int const povdir = uintptr_t(item_internal) % 4;

	// get the current state
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
	HRESULT result = DirectInput8Create(GetModuleHandleUni(), DIRECTINPUT_VERSION, IID_IDirectInput8, &m_dinput, nullptr);
	if (result != DI_OK)
	{
		return result;
	}

	osd_printf_verbose("DirectInput: Using DirectInput %d\n", DIRECTINPUT_VERSION >> 8);
	return 0;
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
	result = m_dinput->CreateDevice(instance->guidInstance, &device, nullptr);
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
	// For now, we always use the desktop window due to multiple issues:
	// * MAME recreates windows on toggling fullscreen.  DirectInput really doesn't like this.
	// * DirectInput doesn't like the window used for D3D fullscreen exclusive mode.
	// * With multiple windows, the first window needs to have focus when using foreground mode.
	// This makes it impossible to use force feedback as that requires foreground exclusive mode.
	// The only way to get around this would be to reopen devices on focus changes.
	[[maybe_unused]] HWND window_handle;
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
			//di_cooperative_level = DISCL_FOREGROUND | DISCL_NONEXCLUSIVE;
			di_cooperative_level = DISCL_BACKGROUND | DISCL_NONEXCLUSIVE;
			break;
		default:
			throw false;
		}
	}

	// set the cooperative level
	result = device->SetCooperativeLevel(GetDesktopWindow(), di_cooperative_level);
	if (result != DI_OK)
	{
		osd_printf_error("DirectInput: Unable to set cooperative level.\n");
		return std::make_pair(nullptr, nullptr);
	}

	// return new device
	return std::make_pair(std::move(device), format);
}


std::string dinput_api_helper::make_id(LPCDIDEVICEINSTANCE instance)
{
	// use name, product GUID and instance GUID as identifier
	return
			text::from_tstring(instance->tszInstanceName) +
			" product_" +
			guid_to_string(instance->guidProduct) +
			" instance_" +
			guid_to_string(instance->guidInstance);
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
