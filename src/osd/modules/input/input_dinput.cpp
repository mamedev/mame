// license:BSD-3-Clause
// copyright-holders:Brad Hughes
/*
* input_dinput.cpp
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

// undef WINNT for dinput.h to prevent duplicate definition
#undef WINNT
#include <dinput.h>
#undef interface

#include <mutex>
#include <thread>

// MAME headers
#include "emu.h"
#include "osdepend.h"
#include "ui/ui.h"

// MAMEOS headers
#include "winmain.h"
#include "window.h"
#include "winutil.h"
#include "../../windows/input.h"

#include "input_windows.h"

#define STRUCTSIZE(x) (m_dinput_version == 0x0300) ? sizeof(x##_DX3) : sizeof(x)

static INT32 dinput_joystick_pov_get_state(void *device_internal, void *item_internal);

//============================================================
//  dinput_set_dword_property
//============================================================

static HRESULT dinput_set_dword_property(LPDIRECTINPUTDEVICE device, REFGUID property_guid, DWORD object, DWORD how, DWORD value)
{
	DIPROPDWORD dipdw;

	dipdw.diph.dwSize = sizeof(dipdw);
	dipdw.diph.dwHeaderSize = sizeof(dipdw.diph);
	dipdw.diph.dwObj = object;
	dipdw.diph.dwHow = how;
	dipdw.dwData = value;

	return IDirectInputDevice_SetProperty(device, property_guid, &dipdw.diph);
}

//============================================================
//  default_pov_name
//============================================================

static const TCHAR *default_pov_name(int which)
{
	static TCHAR buffer[20];
	_sntprintf(buffer, ARRAY_LENGTH(buffer), TEXT("POV%d"), which);
	return buffer;
}

//============================================================
//  dinput_device - base directinput device
//============================================================

// DirectInput-specific information about a device
struct dinput_api_state
{
	LPDIRECTINPUTDEVICE     device;
	LPDIRECTINPUTDEVICE2    device2;
	DIDEVCAPS               caps;
	LPCDIDATAFORMAT         format;
};

class dinput_device
{
public:
	dinput_api_state dinput;

	dinput_device()
		: dinput({0})
	{
	}

protected:
	HRESULT poll(LPVOID pState)
	{
		HRESULT result;

		// first poll the device, then get the state
		if (dinput.device2 != NULL)
			IDirectInputDevice2_Poll(dinput.device2);

		// GetDeviceState returns the immediate state
		result = IDirectInputDevice_GetDeviceState(dinput.device, dinput.format->dwDataSize, pState);

		// handle lost inputs here
		if (result == DIERR_INPUTLOST || result == DIERR_NOTACQUIRED)
		{
			result = IDirectInputDevice_Acquire(dinput.device);
			if (result == DI_OK)
				result = IDirectInputDevice_GetDeviceState(dinput.device, dinput.format->dwDataSize, pState);
		}

		return result;
	}
};

//============================================================
//  dinput_keyboard_device - directinput keyboard device
//============================================================

class dinput_keyboard_device : public keyboard_device, public dinput_device
{
private:
	HANDLE m_dataEvent;
	HANDLE m_exitEvent;
	std::mutex m_device_lock;

public:
	dinput_keyboard_device(running_machine &machine, const TCHAR *name, input_module &module)
		: keyboard_device(machine, name, module),
		  dinput_device()
	{
	}

	// Polls the direct input immediate state
	void poll() override
	{
		std::lock_guard<std::mutex> scope_lock(m_device_lock);

		// Poll the state
		dinput_device::poll(&keyboard.state);
	}
};

//============================================================
//  dinput_module - base directinput module
//============================================================

class dinput_module : public wininput_module
{
private:
	LPDIRECTINPUT        m_dinput;
	int                  m_dinput_version;
	int                  didevtype_keyboard;
	int                  didevtype_mouse;

public:
	dinput_module(const char* type, const char* name)
		: wininput_module(type, name)
	{
	}

	int init_internal() override
	{
		HRESULT result = S_OK;

#if DIRECTINPUT_VERSION >= 0x800
		m_dinput_version = DIRECTINPUT_VERSION;
		result = DirectInput8Create(GetModuleHandleUni(), m_dinput_version, IID_IDirectInput8, (void **)&m_dinput, NULL);
		if (result != DI_OK)
		{
			m_dinput_version = 0;
			return result;
		}
#else
		// first attempt to initialize DirectInput at the current version
		m_dinput_version = DIRECTINPUT_VERSION;
		result = DirectInputCreate(GetModuleHandleUni(), m_dinput_version, &m_dinput, NULL);
		if (result != DI_OK)
		{
			// if that fails, try version 5
			m_dinput_version = 0x0500;
			result = DirectInputCreate(GetModuleHandleUni(), m_dinput_version, &m_dinput, NULL);
			if (result != DI_OK)
			{
				// if that fails, try version 3
				m_dinput_version = 0x0300;
				result = DirectInputCreate(GetModuleHandleUni(), m_dinput_version, &m_dinput, NULL);
				if (result != DI_OK)
				{
					m_dinput_version = 0;
					return result;
				}
			}
		}
#endif

		osd_printf_verbose("DirectInput: Using DirectInput %d\n", m_dinput_version >> 8);
		return 0;
	}

	void exit() override
	{
		wininput_module::exit();
	}

	virtual BOOL device_enum_callback(LPCDIDEVICEINSTANCE instance, LPVOID ref) = 0;

	void input_init(running_machine &machine) override
	{
		struct dinput_callback_context
		{
			dinput_module *    self;
			running_machine *           machine;
		} context = { this, &machine };

		// enumerate the ones we have
		auto enum_callback = [](LPCDIDEVICEINSTANCE instance, LPVOID ref) {
			return ((dinput_callback_context*)ref)->self->device_enum_callback(instance, ((dinput_callback_context*)ref)->machine);
		};

		HRESULT result = IDirectInput_EnumDevices(m_dinput, dinput_devclass(), enum_callback, &context, DIEDFL_ATTACHEDONLY);
		if (result != DI_OK)
			fatalerror("DirectInput: Unable to enumerate keyboards (result=%08X)\n", (UINT32)result);
	}

protected:
	virtual int dinput_devclass() = 0;

	template <class TDevice>
	TDevice* create_dinput_device(
		running_machine &machine,
		LPCDIDEVICEINSTANCE instance,
		LPCDIDATAFORMAT format1,
		LPCDIDATAFORMAT format2,
		DWORD cooperative_level)
	{
		HRESULT result;

		// allocate memory for the device object
		TDevice* devinfo = devicelist()->create_device<TDevice>(machine, instance->tszInstanceName, *this);

		// attempt to create a device
		result = IDirectInput_CreateDevice(m_dinput, WRAP_REFIID(instance->guidInstance), &devinfo->dinput.device, NULL);
		if (result != DI_OK)
			goto error;

		// try to get a version 2 device for it
		result = IDirectInputDevice_QueryInterface(devinfo->dinput.device, WRAP_REFIID(IID_IDirectInputDevice2), (void **)&devinfo->dinput.device2);
		if (result != DI_OK)
			devinfo->dinput.device2 = NULL;

		// get the caps
		devinfo->dinput.caps.dwSize = STRUCTSIZE(DIDEVCAPS);
		result = IDirectInputDevice_GetCapabilities(devinfo->dinput.device, &devinfo->dinput.caps);
		if (result != DI_OK)
			goto error;

		// attempt to set the data format
		devinfo->dinput.format = format1;
		result = IDirectInputDevice_SetDataFormat(devinfo->dinput.device, devinfo->dinput.format);
		if (result != DI_OK)
		{
			// use the secondary format if available
			if (format2 != NULL)
			{
				devinfo->dinput.format = format2;
				result = IDirectInputDevice_SetDataFormat(devinfo->dinput.device, devinfo->dinput.format);
			}
			if (result != DI_OK)
				goto error;
		}

		// set the cooperative level
		result = IDirectInputDevice_SetCooperativeLevel(devinfo->dinput.device, win_window_list->m_hwnd, cooperative_level);
		if (result != DI_OK)
			goto error;
		return devinfo;

	error:
		devicelist()->free_device(devinfo);
		return NULL;
	}

	char * device_item_name(dinput_device * devinfo, int offset, const TCHAR * defstring, const TCHAR * suffix)
	{
		DIDEVICEOBJECTINSTANCE instance = { 0 };
		const TCHAR *namestring = instance.tszName;
		TCHAR *combined;
		HRESULT result;
		char *utf8;

		// query the key name
		instance.dwSize = STRUCTSIZE(DIDEVICEOBJECTINSTANCE);
		result = IDirectInputDevice_GetObjectInfo(devinfo->dinput.device, &instance, offset, DIPH_BYOFFSET);

		// if we got an error and have no default string, just return NULL
		if (result != DI_OK)
		{
			if (defstring == NULL)
				return NULL;
			namestring = defstring;
		}

		// if no suffix, return as-is
		if (suffix == NULL)
			return utf8_from_tstring(namestring);

		// otherwise, allocate space to add the suffix
		combined = global_alloc_array(TCHAR, _tcslen(namestring) + 1 + _tcslen(suffix) + 1);
		_tcscpy(combined, namestring);
		_tcscat(combined, TEXT(" "));
		_tcscat(combined, suffix);

		// convert to UTF8, free the temporary string, and return
		utf8 = utf8_from_tstring(combined);
		global_free_array(combined);
		return utf8;
	}
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
		running_machine &machine = *(running_machine *)ref;
		dinput_keyboard_device *devinfo;
		int keynum;

		// allocate and link in a new device
		devinfo = create_dinput_device<dinput_keyboard_device>(machine, instance, &c_dfDIKeyboard, NULL, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
		if (devinfo == NULL)
			goto exit;

		// populate it
		for (keynum = 0; keynum < MAX_KEYS; keynum++)
		{
			input_item_id itemid = keyboard_trans_table::instance().map_scancode_to_itemid(keynum);
			TCHAR defname[20];
			char *name;

			// generate/fetch the name
			_sntprintf(defname, ARRAY_LENGTH(defname), TEXT("Scan%03d"), keynum);
			name = device_item_name(devinfo, keynum, defname, NULL);

			// add the item to the device
			devinfo->device()->add_item(name, itemid, generic_button_get_state, &devinfo->keyboard.state[keynum]);
			osd_free(name);
		}

	exit:
		return DIENUM_CONTINUE;
	}
};

class dinput_mouse_device : public mouse_device, public dinput_device
{
public:
	dinput_mouse_device(running_machine &machine, const TCHAR *name, input_module &module)
		: mouse_device(machine, name, module),
		dinput_device()
	{
	}

	void poll() override
	{
		// poll
		dinput_device::poll(&mouse);

		// scale the axis data
		mouse.lX *= INPUT_RELATIVE_PER_PIXEL;
		mouse.lY *= INPUT_RELATIVE_PER_PIXEL;
		mouse.lZ *= INPUT_RELATIVE_PER_PIXEL;
	}
};

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
		return DI8DEVTYPE_MOUSE;
#endif
	}

	BOOL device_enum_callback(LPCDIDEVICEINSTANCE instance, LPVOID ref) override
	{
		dinput_mouse_device *devinfo = NULL;
		running_machine &machine = *(running_machine *)ref;
		int axisnum, butnum;
		HRESULT result;

		// allocate and link in a new device
		devinfo = create_dinput_device<dinput_mouse_device>(machine, instance, &c_dfDIMouse2, &c_dfDIMouse, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
		if (devinfo == NULL)
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
			char *name = device_item_name(devinfo, offsetof(DIMOUSESTATE, lX) + axisnum * sizeof(LONG), default_axis_name[axisnum], NULL);
			devinfo->device()->add_item(name, (input_item_id)(ITEM_ID_XAXIS + axisnum), generic_axis_get_state, &devinfo->mouse.lX + axisnum);
			osd_free(name);
		}

		// populate the buttons
		for (butnum = 0; butnum < devinfo->dinput.caps.dwButtons; butnum++)
		{
			FPTR offset = (FPTR)(&((DIMOUSESTATE *)NULL)->rgbButtons[butnum]);

			// add to the mouse device
			char *name = device_item_name(devinfo, offset, default_button_name(butnum), NULL);
			devinfo->device()->add_item(name, (input_item_id)(ITEM_ID_BUTTON1 + butnum), generic_button_get_state, &devinfo->mouse.rgbButtons[butnum]);
			osd_free(name);
		}

	exit:
		return DIENUM_CONTINUE;

	error:
		if (devinfo != NULL)
			devicelist()->free_device(devinfo);
		goto exit;
	}
};

// state information for a joystick; DirectInput state must be first element
struct dinput_joystick_state
{
	DIJOYSTATE              state;
	LONG                    rangemin[8];
	LONG                    rangemax[8];
};

class dinput_joystick_device : public joystick_device, public dinput_device
{
public:
	dinput_joystick_state   joystick;

	dinput_joystick_device(running_machine &machine, const TCHAR *name, input_module &module)
		: joystick_device(machine, name, module),
		  dinput_device(),
		  joystick({0})
	{
	}

	void reset() override
	{
		memset(&joystick, 0, sizeof(joystick));
	}

	void poll() override
	{
		int axisnum;

		// poll the device first
		dinput_device::poll(&joystick.state);

		// normalize axis values
		for (axisnum = 0; axisnum < 8; axisnum++)
		{
			LONG *axis = (&joystick.state.lX) + axisnum;
			*axis = normalize_absolute_axis(*axis, joystick.rangemin[axisnum], joystick.rangemax[axisnum]);
		}
	}
};

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
		return DI8DEVTYPE_JOYSTICK;
#endif
	}

	BOOL device_enum_callback(LPCDIDEVICEINSTANCE instance, LPVOID ref) override
	{
		DWORD cooperative_level = DISCL_FOREGROUND | DISCL_NONEXCLUSIVE;
		int axisnum, axiscount, povnum, butnum;
		running_machine &machine = *(running_machine *)ref;
		dinput_joystick_device *devinfo;
		HRESULT result;

		if (win_window_list != NULL && win_window_list->win_has_menu()) {
			cooperative_level = DISCL_BACKGROUND | DISCL_NONEXCLUSIVE;
		}
		// allocate and link in a new device
		devinfo = create_dinput_device<dinput_joystick_device>(machine, instance, &c_dfDIJoystick, NULL, cooperative_level);
		if (devinfo == NULL)
			goto exit;

		// set absolute mode
		result = dinput_set_dword_property(devinfo->dinput.device, DIPROP_AXISMODE, 0, DIPH_DEVICE, DIPROPAXISMODE_ABS);
		if (result != DI_OK && result != DI_PROPNOEFFECT)
			osd_printf_warning("DirectInput: Unable to set absolute mode for joystick %d (%s)\n", devicelist()->size(), devinfo->name());

		// turn off deadzone; we do our own calculations
		result = dinput_set_dword_property(devinfo->dinput.device, DIPROP_DEADZONE, 0, DIPH_DEVICE, 0);
		if (result != DI_OK && result != DI_PROPNOEFFECT)
			osd_printf_warning("DirectInput: Unable to reset deadzone for joystick %d (%s)\n", devicelist()->size(), devinfo->name());

		// turn off saturation; we do our own calculations
		result = dinput_set_dword_property(devinfo->dinput.device, DIPROP_SATURATION, 0, DIPH_DEVICE, 10000);
		if (result != DI_OK && result != DI_PROPNOEFFECT)
			osd_printf_warning("DirectInput: Unable to reset saturation for joystick %d (%s)\n", devicelist()->size(), devinfo->name());

		// cap the number of axes, POVs, and buttons based on the format
		devinfo->dinput.caps.dwAxes = MIN(devinfo->dinput.caps.dwAxes, 8);
		devinfo->dinput.caps.dwPOVs = MIN(devinfo->dinput.caps.dwPOVs, 4);
		devinfo->dinput.caps.dwButtons = MIN(devinfo->dinput.caps.dwButtons, 128);

		// populate the axes
		for (axisnum = axiscount = 0; axiscount < devinfo->dinput.caps.dwAxes && axisnum < 8; axisnum++)
		{
			DIPROPRANGE dipr;
			char *name;

			// fetch the range of this axis
			dipr.diph.dwSize = sizeof(dipr);
			dipr.diph.dwHeaderSize = sizeof(dipr.diph);
			dipr.diph.dwObj = offsetof(DIJOYSTATE2, lX) + axisnum * sizeof(LONG);
			dipr.diph.dwHow = DIPH_BYOFFSET;
			result = IDirectInputDevice_GetProperty(devinfo->dinput.device, DIPROP_RANGE, &dipr.diph);
			if (result != DI_OK)
				continue;

			devinfo->joystick.rangemin[axisnum] = dipr.lMin;
			devinfo->joystick.rangemax[axisnum] = dipr.lMax;

			// populate the item description as well
			name = device_item_name(devinfo, offsetof(DIJOYSTATE2, lX) + axisnum * sizeof(LONG), default_axis_name[axisnum], NULL);
			devinfo->device()->add_item(name, (input_item_id)(ITEM_ID_XAXIS + axisnum), generic_axis_get_state, &devinfo->joystick.state.lX + axisnum);
			osd_free(name);

			axiscount++;
		}

		// populate the POVs
		for (povnum = 0; povnum < devinfo->dinput.caps.dwPOVs; povnum++)
		{
			char *name;

			// left
			name = device_item_name(devinfo, offsetof(DIJOYSTATE2, rgdwPOV) + povnum * sizeof(DWORD), default_pov_name(povnum), TEXT("L"));
			devinfo->device()->add_item(name, ITEM_ID_OTHER_SWITCH, dinput_joystick_pov_get_state, (void *)(FPTR)(povnum * 4 + POVDIR_LEFT));
			osd_free(name);

			// right
			name = device_item_name(devinfo, offsetof(DIJOYSTATE2, rgdwPOV) + povnum * sizeof(DWORD), default_pov_name(povnum), TEXT("R"));
			devinfo->device()->add_item(name, ITEM_ID_OTHER_SWITCH, dinput_joystick_pov_get_state, (void *)(FPTR)(povnum * 4 + POVDIR_RIGHT));
			osd_free(name);

			// up
			name = device_item_name(devinfo, offsetof(DIJOYSTATE2, rgdwPOV) + povnum * sizeof(DWORD), default_pov_name(povnum), TEXT("U"));
			devinfo->device()->add_item(name, ITEM_ID_OTHER_SWITCH, dinput_joystick_pov_get_state, (void *)(FPTR)(povnum * 4 + POVDIR_UP));
			osd_free(name);

			// down
			name = device_item_name(devinfo, offsetof(DIJOYSTATE2, rgdwPOV) + povnum * sizeof(DWORD), default_pov_name(povnum), TEXT("D"));
			devinfo->device()->add_item(name, ITEM_ID_OTHER_SWITCH, dinput_joystick_pov_get_state, (void *)(FPTR)(povnum * 4 + POVDIR_DOWN));
			osd_free(name);
		}

		// populate the buttons
		for (butnum = 0; butnum < devinfo->dinput.caps.dwButtons; butnum++)
		{
			FPTR offset = (FPTR)(&((DIJOYSTATE2 *)NULL)->rgbButtons[butnum]);
			char *name = device_item_name(devinfo, offset, default_button_name(butnum), NULL);

			input_item_id itemid;

			if (butnum < INPUT_MAX_BUTTONS)
				itemid = (input_item_id)(ITEM_ID_BUTTON1 + butnum);
			else if (butnum < INPUT_MAX_BUTTONS + INPUT_MAX_ADD_SWITCH)
				itemid = (input_item_id)(ITEM_ID_ADD_SWITCH1 - INPUT_MAX_BUTTONS + butnum);
			else
				itemid = ITEM_ID_OTHER_SWITCH;

			devinfo->device()->add_item(name, itemid, generic_button_get_state, &devinfo->joystick.state.rgbButtons[butnum]);
			osd_free(name);
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
	dinput_joystick_device *devinfo = (dinput_joystick_device *)device_internal;
	int povnum = (FPTR)item_internal / 4;
	int povdir = (FPTR)item_internal % 4;
	INT32 result = 0;
	DWORD pov;

	// get the current state
	downcast<wininput_module&>(devinfo->module()).poll_if_necessary(devinfo->machine());
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