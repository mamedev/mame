#ifndef INPUT_XINPUT_H_
#define INPUT_XINPUT_H_

#include <mutex>

#include "modules/lib/osdlib.h"

#define XINPUT_MAX_POV 4
#define XINPUT_MAX_BUTTONS 10
#define XINPUT_MAX_AXIS 4

#define XINPUT_AXIS_MINVALUE -32767
#define XINPUT_AXIS_MAXVALUE 32767

class xinput_joystick_device;
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
	BYTE    buttons[XINPUT_MAX_BUTTONS];
	BYTE    povs[XINPUT_MAX_POV];
	LONG    left_trigger;
	LONG    right_trigger;
	LONG    left_thumb_x;
	LONG    left_thumb_y;
	LONG    right_thumb_x;
	LONG    right_thumb_y;
};

// state information for a gamepad; state must be first element
struct xinput_api_state
{
	UINT32                  player_index;
	XINPUT_STATE            xstate;
	XINPUT_CAPABILITIES     caps;
};

// Typedefs for dynamically loaded functions
typedef DWORD (WINAPI *xinput_get_state_fn)(DWORD, XINPUT_STATE *);
typedef DWORD (WINAPI *xinput_get_caps_fn)(DWORD, DWORD, XINPUT_CAPABILITIES *);

class xinput_api_helper : public std::enable_shared_from_this<xinput_api_helper>
{
public:
	int initialize();
	xinput_joystick_device * create_xinput_device(running_machine &machine, UINT index, wininput_module &module);

	inline DWORD xinput_get_state(DWORD dwUserindex, XINPUT_STATE *pState)
	{
		return (*XInputGetState)(dwUserindex, pState);
	}

	inline DWORD xinput_get_capabilities(DWORD dwUserindex, DWORD dwFlags, XINPUT_CAPABILITIES* pCapabilities)
	{
		return (*XInputGetCapabilities)(dwUserindex, dwFlags, pCapabilities);
	}

private:
	osd::dynamic_module::ptr m_xinput_dll;
	xinput_get_state_fn      XInputGetState;
	xinput_get_caps_fn       XInputGetCapabilities;
};

class xinput_joystick_device : public device_info
{
public:
	gamepad_state      gamepad;
	xinput_api_state   xinput_state;

private:
	std::shared_ptr<xinput_api_helper> m_xinput_helper;
	std::mutex                         m_device_lock;
	bool                               m_configured;

public:
	xinput_joystick_device(running_machine &machine, const char *name, input_module &module, std::shared_ptr<xinput_api_helper> helper);

	void poll() override;
	void reset() override;
	void configure();
};

#endif
