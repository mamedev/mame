#ifndef INPUT_XINPUT_H_
#define INPUT_XINPUT_H_

#include <mutex>

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
	BYTE    rgbButtons[XINPUT_MAX_BUTTONS];
	BYTE    rgbPov[XINPUT_MAX_POV];
	LONG    bLeftTrigger;
	LONG    bRightTrigger;
	LONG    sThumbLX;
	LONG    sThumbLY;
	LONG    sThumbRX;
	LONG    sThumbRY;
};

// state information for a gamepad; state must be first element
struct xinput_api_state
{
	UINT32                  playerIndex;
	XINPUT_STATE            xstate;
	XINPUT_CAPABILITIES     caps;
};

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
// Typedef for pointers to XInput Functions
typedef lazy_loaded_function_p2<DWORD, DWORD, XINPUT_STATE*> xinput_get_state_fn;
typedef lazy_loaded_function_p3<DWORD, DWORD, DWORD, XINPUT_CAPABILITIES*> xinput_get_caps_fn;
#endif

class xinput_api_helper : public std::enable_shared_from_this<xinput_api_helper>
{
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
private:
	const wchar_t* xinput_dll_names[2] = { L"xinput1_4.dll", L"xinput9_1_0.dll" };

public:
	xinput_get_state_fn   XInputGetState;
	xinput_get_caps_fn    XInputGetCapabilities;
#endif

public:
	xinput_api_helper();

	int initialize();
	xinput_joystick_device * create_xinput_device(running_machine &machine, UINT index, wininput_module &module);

#if !WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
	// Pass-through functions for Universal Windows
	inline DWORD XInputGetState(DWORD dwUserindex, XINPUT_STATE *pState);
	inline DWORD XInputGetCapabilities(DWORD dwUserindex, DWORD dwFlags, XINPUT_CAPABILITIES* pCapabilities);
#endif
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
