// license:BSD-3-Clause
// copyright-holders:Brad Hughes
//============================================================
//
//  input_xinput.cpp - XInput API input support for Windows
//
//============================================================

#include "input_module.h"
#include "modules/osdmodule.h"

#if defined(OSD_WINDOWS)

// standard windows headers
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// XInput header
#include <Xinput.h>

#undef interface

// MAME headers
#include "emu.h"
#include "osdepend.h"
#include "ui/ui.h"

// MAMEOS headers
#include "winmain.h"
#include "window.h"

#include "input_common.h"
#include "input_windows.h"

#define XINPUT_MAX_POV 4
#define XINPUT_MAX_BUTTONS 10
#define XINPUT_MAX_AXIS 4

#define XINPUT_AXIS_MINVALUE -32767
#define XINPUT_AXIS_MAXVALUE 32767

// default axis names
static const char *const xinput_axis_name[] =
{
	"LSX",
	"LSY",
	"RSX",
	"RSY"
};

static const input_item_id xinput_axis_ids[] =
{
	ITEM_ID_XAXIS,
	ITEM_ID_YAXIS,
	ITEM_ID_RXAXIS,
	ITEM_ID_RYAXIS
};

static const USHORT xinput_pov_dir[] = {
	XINPUT_GAMEPAD_DPAD_UP,
	XINPUT_GAMEPAD_DPAD_DOWN,
	XINPUT_GAMEPAD_DPAD_LEFT,
	XINPUT_GAMEPAD_DPAD_RIGHT
};

static const char *const xinput_pov_names[] = {
	"DPAD Up",
	"DPAD Down",
	"DPAD Left",
	"DPAD Right"
};

static const USHORT xinput_buttons[] = {
	XINPUT_GAMEPAD_A,
	XINPUT_GAMEPAD_B,
	XINPUT_GAMEPAD_X,
	XINPUT_GAMEPAD_Y,
	XINPUT_GAMEPAD_LEFT_SHOULDER,
	XINPUT_GAMEPAD_RIGHT_SHOULDER,
	XINPUT_GAMEPAD_START,
	XINPUT_GAMEPAD_BACK,
	XINPUT_GAMEPAD_LEFT_THUMB,
	XINPUT_GAMEPAD_RIGHT_THUMB,
};

static const char *const xinput_button_names[] = {
	"A",
	"B",
	"X",
	"Y",
	"Left Shoulder",
	"Right Shoulder",
	"Start",
	"Back",
	"Left Thumb",
	"Right Thumb"
};

struct gamepad_state
{
	BYTE    rgbButtons[XINPUT_MAX_BUTTONS];
	BYTE    rgbPov[XINPUT_MAX_POV];
	BYTE    bLeftTrigger;
	BYTE    bRightTrigger;
	LONG    sThumbLX;
	LONG    sThumbLY;
	LONG    sThumbRX;
	LONG    sThumbRY;
};

// state information for a gamepad; state must be first element
struct xinput_api_state
{
	UINT32					playerIndex;
	XINPUT_STATE            xstate;
	XINPUT_CAPABILITIES     caps;
};

//============================================================
//  xinput_joystick_device
//============================================================

class xinput_joystick_device : public device_info
{
public:
	gamepad_state      gamepad;
	xinput_api_state   xinput_state;

	xinput_joystick_device(running_machine &machine, const char *name, input_module &module)
		: device_info(machine, name, DEVICE_CLASS_JOYSTICK, module),
			gamepad({0}),
			xinput_state({0})
	{
	}

	void poll() override
	{
		// poll the device first
		HRESULT result = XInputGetState(xinput_state.playerIndex, &xinput_state.xstate);

		// If we can't poll the device, skip
		if (FAILED(result))
			return;

		// Copy the XState into State
		// Start with the POV (DPAD)
		for (int povindex = 0; povindex < XINPUT_MAX_POV; povindex++)
		{
			int currentPov = xinput_pov_dir[povindex];
			gamepad.rgbPov[currentPov] = (xinput_state.xstate.Gamepad.wButtons & currentPov) ? 0xFF : 0;
		}

		// Now do the buttons
		for (int buttonindex = 0; buttonindex < XINPUT_MAX_BUTTONS; buttonindex++)
		{
			int currentButton = xinput_buttons[buttonindex];
			gamepad.rgbButtons[buttonindex] = (xinput_state.xstate.Gamepad.wButtons & currentButton) ? 0xFF : 0;
		}
		
		// Now grab the axis values
		// Each of the thumbstick axis members is a signed value between -32768 and 32767 describing the position of the thumbstick
		// However, the Y axis values are inverted from what MAME expects, so multiply by -1 first
		gamepad.sThumbLX = normalize_absolute_axis(xinput_state.xstate.Gamepad.sThumbLX, XINPUT_AXIS_MINVALUE, XINPUT_AXIS_MAXVALUE);
		gamepad.sThumbLY = normalize_absolute_axis(xinput_state.xstate.Gamepad.sThumbLY * -1, XINPUT_AXIS_MINVALUE, XINPUT_AXIS_MAXVALUE);
		gamepad.sThumbRX = normalize_absolute_axis(xinput_state.xstate.Gamepad.sThumbRX, XINPUT_AXIS_MINVALUE, XINPUT_AXIS_MAXVALUE);
		gamepad.sThumbRY = normalize_absolute_axis(xinput_state.xstate.Gamepad.sThumbRY * -1, XINPUT_AXIS_MINVALUE, XINPUT_AXIS_MAXVALUE);

		// Now the triggers
		gamepad.bLeftTrigger = xinput_state.xstate.Gamepad.bLeftTrigger;
		gamepad.bRightTrigger = xinput_state.xstate.Gamepad.bRightTrigger;
	}

	void reset() override
	{
		memset(&gamepad, 0, sizeof(gamepad));
	}
};

//============================================================
//  xinput_joystick_module
//============================================================

class xinput_joystick_module : public wininput_module
{
public:
	xinput_joystick_module()
		: wininput_module(OSD_JOYSTICKINPUT_PROVIDER, "xinput")
	{
	}

protected:
	virtual void input_init(running_machine &machine) override
	{
		xinput_joystick_device *devinfo;

		// Loop through each gamepad to determine if they are connected
		for (UINT i = 0; i < XUSER_MAX_COUNT; i++)
		{
			XINPUT_STATE state = {0};

			if (XInputGetState(i, &state) == ERROR_SUCCESS)
			{
				// allocate and link in a new device
				devinfo = create_xinput_device(machine, i);
				if (devinfo == nullptr)
					continue;

				// Add the axes
				for (int i = 0; i < XINPUT_MAX_AXIS; i++)
				{
					devinfo->device()->add_item(
						xinput_axis_name[i],
						xinput_axis_ids[i],
						generic_axis_get_state,
						&devinfo->gamepad.sThumbLX + i);
				}

				// Populate the POVs
				// For XBOX, we treat the DPAD as a hat switch
				for (int povnum = 0; povnum < XINPUT_MAX_POV; povnum++)
				{
					devinfo->device()->add_item(
						xinput_pov_names[povnum],
						ITEM_ID_OTHER_SWITCH,
						generic_button_get_state,
						&devinfo->gamepad.rgbPov[povnum]);
				}

				// populate the buttons
				for (int butnum = 0; butnum < XINPUT_MAX_BUTTONS; butnum++)
				{
					devinfo->device()->add_item(
						xinput_button_names[butnum],
						(input_item_id)(ITEM_ID_BUTTON1 + butnum),
						generic_button_get_state,
						&devinfo->gamepad.rgbButtons[butnum]);
				}

				devinfo->device()->add_item(
					"Left Trigger",
					(input_item_id)(ITEM_ID_BUTTON1 + XINPUT_MAX_BUTTONS),
					generic_button_get_state,
					&devinfo->gamepad.bLeftTrigger);

				devinfo->device()->add_item(
					"Right Trigger",
					(input_item_id)(ITEM_ID_BUTTON1 + XINPUT_MAX_BUTTONS + 1),
					generic_button_get_state,
					&devinfo->gamepad.bRightTrigger);
			}
		}
	}

	private:
		//============================================================
		//  create_xinput_device
		//============================================================

		xinput_joystick_device * create_xinput_device(running_machine &machine, UINT index)
		{
			xinput_joystick_device *devinfo;

			XINPUT_CAPABILITIES caps = {0};
			if (FAILED(XInputGetCapabilities(index, 0, &caps)))
			{
				// If we can't get the capabilities skip this device
				return nullptr;
			}

			char device_name[16];
			snprintf(device_name, sizeof(device_name), "XInput Player %u", index + 1);

			// allocate the device object
			devinfo = devicelist()->create_device<xinput_joystick_device>(machine, device_name, *this);

			// Set the player ID
			devinfo->xinput_state.playerIndex = index;

			// Assign the caps we captured earlier
			devinfo->xinput_state.caps = caps;

			return devinfo;
		}
};

#else
MODULE_NOT_SUPPORTED(xinput_joystick_module, OSD_KEYBOARDINPUT_PROVIDER, "xinput")
#endif

MODULE_DEFINITION(JOYSTICKINPUT_XINPUT, xinput_joystick_module)