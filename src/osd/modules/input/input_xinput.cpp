// license:BSD-3-Clause
// copyright-holders:Brad Hughes
//============================================================
//
//  input_xinput.cpp - XInput API input support for Windows
//
//============================================================
/*

XInput is infamously inflexible.  It currently only supports a single
controller type (game controller) with a fixed report format.  The
report format includes:
* 16-bit field for single-bit buttons and D-pad directions (two bits
  undefined)
* Two 8-bit axis values, used for triggers, pedals, buttons or throttle
  and rudder controls, depending on the controller
* Four 16-bit axis values, used for joysticks, steering wheels or hat
  switches, depending on the controller

Some button names have changed at various times:
* Face buttons White and Black became shoulder buttons LB and RB,
  respectively.
* Navigation buttons Start and Back became Menu and View, respectively.

Subtype         Gamepad         Wheel           Arcade stick    Arcade pad      Flight stick    Dance pad       Guitar          Drum kit

D-pad           D-pad           D-pad           Joystick        D-pad                           Foot switch     D-Pad/Strum     D-pad
L trigger       Trigger         Brake           Button          Button          Rudder                          Pickup select^
R trigger       Trigger         Accelerator     Button          Button          Throttle                        ^
L stick X       Stick           Wheel                                           Roll
L stick Y       Stick                                                           Pitch
R stick X       Stick                                                           Hat left/right                  Whammy bar
R stick Y       Stick                                                           Hat up/down                     Orientation

A               Button          Button          Button          Button          Primary fire    Foot switch     Fret 1          Floor tom
B               Button          Button          Button          Button          Secondary fire  Foot switch     Fret 2          Snare
X               Button          Button          Button          Button          Button          Foot switch     Fret 4          Low tom
Y               Button          Button          Button          Button          Button          Foot switch     Fret 3          High tom
LB              Button          Button          Button^         Button          Button^                         Fret 5          Bass drum
RB              Button          Button          Button^         Button          Button^                         Button^         Button^
LSB             Button          Button^         Button^         Button^         Button^                         Fret mod^       Button^
RSB             Button          Button^         Button^         Button^         Button^                         Button^         Button^

^ optional


There are multiple physical button layouts for arcade sticks, for
example:

Gamester Xbox Arcade Stick, Hori Real Arcade Pro EX
LT X  Y  LB
RT A  B  RB

PXN 0082 Arcade Stick
LB X  Y  RB
LT A  B  RT

Mortal Kombat Tournament Edition Arcade Stick
   X    Y
     RT
LB A    B

Hori Fighting Stick EX2
 B  X  Y
A  LT RT

Hori Real Arcade Pro VX-SA Kai, Razer Atrox
 B  X  Y  LB
A  LT RT RB

Hori Real Arcade Pro.V Kai, Mayflash F300, Mayflash F500
 X  Y  RB LB
A  B  RT LT

Mad Catz Brawl Stick
 X  Y  LB LT
A  B  RB RT

Arcade pads typically have six face buttons, and come with different
layouts corresponding to the latter two arcade stick layouts, with the
rightmost column on the shoulder buttons.


Dance mats usually have this layout:
BK    ST
B  U  A
L     R
Y  D  X

This layout seems somewhat unusual:
BK    ST
A  U  B
L     R
X  D  Y

This layout is also available but rare:
BK    ST
A  U  B
L     R
Y  D  X

*/

#include "modules/osdmodule.h"

#if defined(OSD_WINDOWS)

#include "emu.h"

#include "input_xinput.h"

// standard windows headers
#include <windows.h>

#include <algorithm>
#include <cstdint>
#include <string>
#include <tuple>
#include <utility>


#define XINPUT_LIBRARIES { "xinput1_4.dll", "xinput9_1_0.dll" }

#define XINPUT_AXIS_MINVALUE (-32'767)
#define XINPUT_AXIS_MAXVALUE (32'767)

namespace {

char const *const AXIS_NAMES_GAMEPAD[]{
		"LSX",
		"LSY",
		"RSX",
		"RSY",
		"LT",
		"RT" };

char const *const AXIS_NAMES_WHEEL[]{
		"Wheel",
		"LSY",
		"RSX",
		"RSY",
		"Brake",
		"Accelerator" };

char const *const AXIS_NAMES_FLIGHT_STICK[]{
		"Joystick X",
		"Joystick Y",
		nullptr,
		nullptr,
		"Rudder",
		"Throttle" };

input_item_id const AXIS_IDS_GAMEPAD[]{
		ITEM_ID_XAXIS,
		ITEM_ID_YAXIS,
		ITEM_ID_ZAXIS,
		ITEM_ID_RZAXIS,
		ITEM_ID_SLIDER1,
		ITEM_ID_SLIDER2 };

input_item_id const AXIS_IDS_FLIGHT_STICK[]{
		ITEM_ID_XAXIS,
		ITEM_ID_YAXIS,
		ITEM_ID_INVALID,
		ITEM_ID_INVALID,
		ITEM_ID_RZAXIS,
		ITEM_ID_ZAXIS };

const char *const HAT_NAMES_GAMEPAD[]{
		"D-pad Up",
		"D-pad Down",
		"D-pad Left",
		"D-pad Right",
		"POV Hat Up",
		"POV Hat Down",
		"POV Hat Left",
		"POV Hat Right" };

const char *const HAT_NAMES_ARCADE_STICK[]{
		"Joystick Up",
		"Joystick Down",
		"Joystick Left",
		"Joystick Right",
		nullptr,
		nullptr,
		nullptr,
		nullptr };


//============================================================
//  xinput_joystick_device
//============================================================

class xinput_joystick_device : public device_info
{
public:
	xinput_joystick_device(
			running_machine &machine,
			std::string &&name,
			std::string &&id,
			input_module &module,
			uint32_t player,
			XINPUT_CAPABILITIES const &caps,
			std::shared_ptr<xinput_api_helper> helper);

	void poll() override;
	void reset() override;
	void configure();

private:
	static inline constexpr USHORT SWITCH_BITS[] =
	{
		XINPUT_GAMEPAD_A,
		XINPUT_GAMEPAD_B,
		XINPUT_GAMEPAD_X,
		XINPUT_GAMEPAD_Y,
		XINPUT_GAMEPAD_LEFT_SHOULDER,
		XINPUT_GAMEPAD_RIGHT_SHOULDER,
		XINPUT_GAMEPAD_LEFT_THUMB,
		XINPUT_GAMEPAD_RIGHT_THUMB,
		XINPUT_GAMEPAD_START,
		XINPUT_GAMEPAD_BACK,

		XINPUT_GAMEPAD_DPAD_UP,
		XINPUT_GAMEPAD_DPAD_DOWN,
		XINPUT_GAMEPAD_DPAD_LEFT,
		XINPUT_GAMEPAD_DPAD_RIGHT
	};

	enum
	{
		SWITCH_A,           // button bits
		SWITCH_B,
		SWITCH_X,
		SWITCH_Y,
		SWITCH_LB,
		SWITCH_RB,
		SWITCH_LSB,
		SWITCH_RSB,
		SWITCH_START,
		SWITCH_BACK,

		SWITCH_DPAD_UP,     // D-pad bits
		SWITCH_DPAD_DOWN,
		SWITCH_DPAD_LEFT,
		SWITCH_DPAD_RIGHT,

		SWITCH_HAT_UP,      // for flight stick with POV hat as right stick
		SWITCH_HAT_DOWN,
		SWITCH_HAT_LEFT,
		SWITCH_HAT_RIGHT,

		SWITCH_LT,          // for arcade stick/pad with LT/RT buttons
		SWITCH_RT,

		SWITCH_TOTAL
	};

	enum
	{
		AXIS_LT,            // half-axes for triggers
		AXIS_RT,

		AXIS_LSX,           // full-precision axes
		AXIS_LSY,
		AXIS_RSX,
		AXIS_RSY,

		AXIS_RUDDER,        // LT/RT mapped differently for flight sticks
		AXIS_THROTTLE,

		AXIS_TOTAL
	};

	uint32_t const            m_player_index;
	XINPUT_CAPABILITIES const m_capabilities;
	XINPUT_STATE              m_xinput_state;
	uint8_t                   m_switches[SWITCH_TOTAL];
	int32_t                   m_axes[AXIS_TOTAL];

	std::shared_ptr<xinput_api_helper> m_xinput_helper;
};

xinput_joystick_device::xinput_joystick_device(
		running_machine &machine,
		std::string &&name,
		std::string &&id,
		input_module &module,
		uint32_t player,
		XINPUT_CAPABILITIES const &caps,
		std::shared_ptr<xinput_api_helper> helper) :
	device_info(machine, std::move(name), std::move(id), DEVICE_CLASS_JOYSTICK, module),
	m_player_index(player),
	m_capabilities(caps),
	m_xinput_state{ 0 },
	m_xinput_helper(helper)
{
	std::fill(std::begin(m_switches), std::end(m_switches), 0);
	std::fill(std::begin(m_axes), std::end(m_axes), 0);
}

void xinput_joystick_device::poll()
{
	// poll the device first
	DWORD const prevpacket = m_xinput_state.dwPacketNumber;
	HRESULT result = m_xinput_helper->xinput_get_state(m_player_index, &m_xinput_state);

	// if we can't poll the device or nothing changed, skip
	if (FAILED(result) || (prevpacket == m_xinput_state.dwPacketNumber))
		return;

	// translate button bits
	for (unsigned i = 0; std::size(SWITCH_BITS) > i; ++i)
		m_switches[SWITCH_A + i] = (m_xinput_state.Gamepad.wButtons & SWITCH_BITS[i]) ? 0xff : 0x00;

	// translate the triggers onto the negative side of the axes
	m_axes[AXIS_LT] = -normalize_absolute_axis(m_xinput_state.Gamepad.bLeftTrigger, -255, 255);
	m_axes[AXIS_RT] = -normalize_absolute_axis(m_xinput_state.Gamepad.bRightTrigger, -255, 255);

	// translate full-precision axes - Y direction is opposite to what MAME uses
	m_axes[AXIS_LSX] = normalize_absolute_axis(m_xinput_state.Gamepad.sThumbLX, XINPUT_AXIS_MINVALUE, XINPUT_AXIS_MAXVALUE);
	m_axes[AXIS_LSY] = normalize_absolute_axis(-m_xinput_state.Gamepad.sThumbLY, XINPUT_AXIS_MINVALUE, XINPUT_AXIS_MAXVALUE);
	m_axes[AXIS_RSX] = normalize_absolute_axis(m_xinput_state.Gamepad.sThumbRX, XINPUT_AXIS_MINVALUE, XINPUT_AXIS_MAXVALUE);
	m_axes[AXIS_RSY] = normalize_absolute_axis(-m_xinput_state.Gamepad.sThumbRY, XINPUT_AXIS_MINVALUE, XINPUT_AXIS_MAXVALUE);

	// translate LT/RT switches for arcade sticks/pads
	m_switches[SWITCH_LT] = (0x80 <= m_xinput_state.Gamepad.bLeftTrigger) ? 0xff : 0x00;
	m_switches[SWITCH_RT] = (0x80 <= m_xinput_state.Gamepad.bRightTrigger) ? 0xff : 0x00;

	// translate POV hat for flight sticks
	m_switches[SWITCH_HAT_UP] = (16'384 <= m_xinput_state.Gamepad.sThumbRY) ? 0xff : 0x00;
	m_switches[SWITCH_HAT_DOWN] = (-16'384 >= m_xinput_state.Gamepad.sThumbRY) ? 0xff : 0x00;
	m_switches[SWITCH_HAT_LEFT] = (-16'384 >= m_xinput_state.Gamepad.sThumbRX) ? 0xff : 0x00;
	m_switches[SWITCH_HAT_RIGHT] = (16'384 <= m_xinput_state.Gamepad.sThumbRX) ? 0xff : 0x00;

	// translate rudder and throttle for flight sticks
	m_axes[AXIS_RUDDER] = normalize_absolute_axis(m_xinput_state.Gamepad.bLeftTrigger, 0, 255);
	m_axes[AXIS_THROTTLE] = normalize_absolute_axis(m_xinput_state.Gamepad.bRightTrigger, 0, 255);
}

void xinput_joystick_device::reset()
{
	std::fill(std::begin(m_switches), std::end(m_switches), 0);
	std::fill(std::begin(m_axes), std::end(m_axes), 0);
}

void xinput_joystick_device::configure()
{
	// TODO: don't add axes not present in capabilities
	// TODO: proper support for dance mat, guitar and drum kit controllers

	// default characteristics for a gamepad
	char const *type_name = "unsupported";
	char const *subtype_name = "unsupported";
	bool lt_rt_button = false;
	bool lt_rt_fullaxis = false;
	bool rstick_hat = false;
	char const *const *axis_names = AXIS_NAMES_GAMEPAD;
	input_item_id const *axis_ids = AXIS_IDS_GAMEPAD;
	char const *const *hat_names = HAT_NAMES_GAMEPAD;

	// consider the device type to decide how to map controls
	switch (m_capabilities.Type)
	{
	case XINPUT_DEVTYPE_GAMEPAD:
		type_name = "game controller";
		switch (m_capabilities.SubType)
		{
		case 0x00: // XINPUT_DEVSUBTYPE_UNKNOWN: work around MinGW header issues
			subtype_name = "unknown";
			break;
		case XINPUT_DEVSUBTYPE_GAMEPAD:
			subtype_name = "gamepad";
			break;
		case XINPUT_DEVSUBTYPE_WHEEL:
			subtype_name = "wheel";
			axis_names = AXIS_NAMES_WHEEL;
			break;
		case XINPUT_DEVSUBTYPE_ARCADE_STICK:
			subtype_name = "arcade stick";
			lt_rt_button = true;
			hat_names = HAT_NAMES_ARCADE_STICK;
			break;
		case 0x04: // XINPUT_DEVSUBTYPE_FLIGHT_STICK: work around MinGW header issues
			subtype_name = "flight stick";
			lt_rt_fullaxis = true;
			rstick_hat = true;
			axis_names = AXIS_NAMES_FLIGHT_STICK;
			axis_ids = AXIS_IDS_FLIGHT_STICK;
			break;
		case XINPUT_DEVSUBTYPE_DANCE_PAD:
			subtype_name = "dance pad";
			break;
		case XINPUT_DEVSUBTYPE_GUITAR:
		case XINPUT_DEVSUBTYPE_GUITAR_ALTERNATE:
		case XINPUT_DEVSUBTYPE_GUITAR_BASS:
			subtype_name = "guitar";
			break;
		case XINPUT_DEVSUBTYPE_DRUM_KIT:
			subtype_name = "drum kit";
			break;
		case XINPUT_DEVSUBTYPE_ARCADE_PAD:
			subtype_name = "arcade pad";
			lt_rt_button = true;
			break;
		}
		break;
	}

	// log some diagnostic information
	osd_printf_verbose(
			"Configuring XInput player %d type 0x%02X (%s) sub type 0x%02X (%s)\n",
			m_player_index + 1,
			m_capabilities.Type,
			type_name,
			m_capabilities.SubType,
			subtype_name);
	osd_printf_verbose(
			"XInput switch capabilities A=%d B=%d X=%d Y=%d LB=%d RB=%d LSB=%d RSB=%d Start=%d Back=%d Up=%d Down=%d Left=%d Right=%d\n",
			(m_capabilities.Gamepad.wButtons & XINPUT_GAMEPAD_A) ? 1 : 0,
			(m_capabilities.Gamepad.wButtons & XINPUT_GAMEPAD_B) ? 1 : 0,
			(m_capabilities.Gamepad.wButtons & XINPUT_GAMEPAD_X) ? 1 : 0,
			(m_capabilities.Gamepad.wButtons & XINPUT_GAMEPAD_Y) ? 1 : 0,
			(m_capabilities.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) ? 1 : 0,
			(m_capabilities.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) ? 1 : 0,
			(m_capabilities.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB) ? 1 : 0,
			(m_capabilities.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB) ? 1 : 0,
			(m_capabilities.Gamepad.wButtons & XINPUT_GAMEPAD_START) ? 1 : 0,
			(m_capabilities.Gamepad.wButtons & XINPUT_GAMEPAD_BACK) ? 1 : 0,
			(m_capabilities.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP) ? 1 : 0,
			(m_capabilities.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) ? 1 : 0,
			(m_capabilities.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) ? 1 : 0,
			(m_capabilities.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) ? 1 : 0);
	osd_printf_verbose(
			"XInput axis capabilities LT=0x%02X (%d-bit) RT=0x%02X (%d-bit) LSX=0x%04X (%d-bit) LSY=0x%04X (%d-bit) RSX=0x%04X (%d-bit) RSY=0x%04X (%d-bit)\n",
			m_capabilities.Gamepad.bLeftTrigger,
			count_leading_ones_32(uint32_t(m_capabilities.Gamepad.bLeftTrigger) << 24),
			m_capabilities.Gamepad.bRightTrigger,
			count_leading_ones_32(uint32_t(m_capabilities.Gamepad.bRightTrigger) << 24),
			m_capabilities.Gamepad.sThumbLX,
			count_leading_ones_32(uint32_t(uint16_t(m_capabilities.Gamepad.sThumbLX)) << 16),
			m_capabilities.Gamepad.sThumbLY,
			count_leading_ones_32(uint32_t(uint16_t(m_capabilities.Gamepad.sThumbLY)) << 16),
			m_capabilities.Gamepad.sThumbRX,
			count_leading_ones_32(uint32_t(uint16_t(m_capabilities.Gamepad.sThumbRX)) << 16),
			m_capabilities.Gamepad.sThumbRY,
			count_leading_ones_32(uint32_t(uint16_t(m_capabilities.Gamepad.sThumbRY)) << 16));

	// add full-precision axes
	for (unsigned i = 0; ((rstick_hat ? AXIS_LSY : AXIS_RSY) - AXIS_LSX) >= i; ++i)
	{
		device()->add_item(
				axis_names[i],
				axis_ids[i],
				generic_axis_get_state<int32_t>,
				&m_axes[AXIS_LSX + i]);
	}

	// add extra axes for flight sticks
	if (lt_rt_fullaxis)
	{
		for (unsigned i = 0; (AXIS_THROTTLE - AXIS_RUDDER) >= i; ++i)
		{
			device()->add_item(
					axis_names[4 + i],
					axis_ids[4 + i],
					generic_axis_get_state<int32_t>,
					&m_axes[AXIS_RUDDER + i]);
		}
	}

	// add hats
	bool const hat_caps[]{
			(m_capabilities.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP) != 0,
			(m_capabilities.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) != 0,
			(m_capabilities.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) != 0,
			(m_capabilities.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) != 0,
			rstick_hat && m_capabilities.Gamepad.sThumbRY,
			rstick_hat && m_capabilities.Gamepad.sThumbRY,
			rstick_hat && m_capabilities.Gamepad.sThumbRX,
			rstick_hat && m_capabilities.Gamepad.sThumbRX };
	for (unsigned i = 0; (SWITCH_HAT_RIGHT - SWITCH_DPAD_UP) >= i; ++i)
	{
		if (hat_caps[i])
		{
			device()->add_item(
					hat_names[i],
					input_item_id(ITEM_ID_HAT1UP + i), // matches up/down/left/right order
					generic_button_get_state<uint8_t>,
					&m_switches[SWITCH_DPAD_UP + i]);
		}
	}

	// add buttons
	std::tuple<unsigned, char const *, bool> const button_caps[]{
			{ SWITCH_A,   "A",   (m_capabilities.Gamepad.wButtons & XINPUT_GAMEPAD_A) != 0 },
			{ SWITCH_B,   "B",   (m_capabilities.Gamepad.wButtons & XINPUT_GAMEPAD_B) != 0 },
			{ SWITCH_X,   "X",   (m_capabilities.Gamepad.wButtons & XINPUT_GAMEPAD_X) != 0 },
			{ SWITCH_Y,   "Y",   (m_capabilities.Gamepad.wButtons & XINPUT_GAMEPAD_Y) != 0 },
			{ SWITCH_LT,  "LT",  lt_rt_button && m_capabilities.Gamepad.bLeftTrigger },
			{ SWITCH_RT,  "RT",  lt_rt_button && m_capabilities.Gamepad.bRightTrigger },
			{ SWITCH_LB,  "LB",  (m_capabilities.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) != 0 },
			{ SWITCH_RB,  "RB",  (m_capabilities.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) != 0 },
			{ SWITCH_LSB, "LSB", (m_capabilities.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB) != 0 },
			{ SWITCH_RSB, "RSB", (m_capabilities.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB) != 0 } };
	input_item_id button_id = ITEM_ID_BUTTON1;
	for (auto [i, name, supported] : button_caps)
	{
		if (supported)
		{
			device()->add_item(
					name,
					button_id++,
					generic_button_get_state<uint8_t>,
					&m_switches[i]);
		}
	}

	// add start/back
	if (m_capabilities.Gamepad.wButtons & XINPUT_GAMEPAD_START)
	{
		device()->add_item(
				"Start",
				ITEM_ID_START,
				generic_button_get_state<uint8_t>,
				&m_switches[SWITCH_START]);
	}
	if (m_capabilities.Gamepad.wButtons & XINPUT_GAMEPAD_BACK)
	{
		device()->add_item(
				"Back",
				ITEM_ID_SELECT,
				generic_button_get_state<uint8_t>,
				&m_switches[SWITCH_BACK]);
	}

	// add triggers/pedals
	if (!lt_rt_button && !lt_rt_fullaxis)
	{
		for (unsigned i = 0; (AXIS_RT - AXIS_LT) >= i; ++i)
		{
			device()->add_item(
					axis_names[4 + i],
					axis_ids[4 + i],
					generic_axis_get_state<int32_t>,
					&m_axes[AXIS_LT + i]);
		}
	}
}


//============================================================
//  xinput_joystick_module
//============================================================

class xinput_joystick_module : public wininput_module
{
public:
	xinput_joystick_module() : wininput_module(OSD_JOYSTICKINPUT_PROVIDER, "xinput")
	{
	}

	int init(const osd_options &options) override
	{
		// Call the base
		int status = wininput_module::init(options);
		if (status != 0)
			return status;

		// Create and initialize our helper
		m_xinput_helper = std::make_shared<xinput_api_helper>();
		status = m_xinput_helper->initialize();
		if (status != 0)
		{
			osd_printf_error("xinput_joystick_module failed to get XInput interface! Error: %u\n", static_cast<unsigned int>(status));
			return -1;
		}

		return 0;
	}

protected:
	virtual void input_init(running_machine &machine) override
	{
		// Loop through each gamepad to determine if they are connected
		for (UINT i = 0; i < XUSER_MAX_COUNT; i++)
		{
			XINPUT_STATE state = {0};

			// allocate and link in a new device
			if (m_xinput_helper->xinput_get_state(i, &state) == ERROR_SUCCESS)
				m_xinput_helper->create_xinput_device(machine, i, *this);
		}
	}

private:
	std::shared_ptr<xinput_api_helper> m_xinput_helper;
};

} // anonymous namespace


int xinput_api_helper::initialize()
{
	m_xinput_dll = osd::dynamic_module::open(XINPUT_LIBRARIES);

	XInputGetState = m_xinput_dll->bind<xinput_get_state_fn>("XInputGetState");
	XInputGetCapabilities = m_xinput_dll->bind<xinput_get_caps_fn>("XInputGetCapabilities");

	if (!XInputGetState || !XInputGetCapabilities)
	{
		osd_printf_verbose("Could not find XInput. Please try to reinstall DirectX runtime package.\n");
		return -1;
	}

	return 0;
}

//============================================================
//  create_xinput_device
//============================================================

device_info *xinput_api_helper::create_xinput_device(running_machine &machine, UINT index, wininput_module &module)
{
	// If we can't get the capabilities skip this device
	XINPUT_CAPABILITIES caps = { 0 };
	if (FAILED(xinput_get_capabilities(index, 0, &caps)))
		return nullptr;

	char device_name[16];
	snprintf(device_name, sizeof(device_name), "XInput Player %u", index + 1);

	// allocate the device object
	auto &devinfo = module.devicelist().create_device<xinput_joystick_device>(
			machine,
			device_name,
			device_name,
			module,
			index,
			caps,
			shared_from_this());

	// configure each controller to add buttons, axes, etc.
	devinfo.configure();

	return &devinfo;
}

#else // defined(OSD_WINDOWS)

#include "input_module.h"

MODULE_NOT_SUPPORTED(xinput_joystick_module, OSD_JOYSTICKINPUT_PROVIDER, "xinput")

#endif // defined(OSD_WINDOWS)

MODULE_DEFINITION(JOYSTICKINPUT_XINPUT, xinput_joystick_module)
