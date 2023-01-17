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
LSB             Button          Button^         Button^         Button^         Button^                         Fret modifier^  Button^
RSB             Button          Button^         Button^         Button^         Button^                         Button^         Button^

^ optional


At least the vast majority of controllers report 8-bit trigger
resolution and 10-bit stick resolution, even when the physical controls
use digital switches.  Resolution can't be used to reliably detect
nominal analog axes controlled by switches.

Some arcade sticks report unknown or gamepad subtype, but have a single
digital joystick with a switch to select between controlling the D-pad,
left stick and right stick.  You can't assume that all three can be
controlled at the same time.

Many controllers don't correctly report the absence of analog sticks.


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

Hori Real Arcade Pro.V Kai, Mad Catz EGO Arcade Stick, Mayflash F300, Mayflash F500
 X  Y  RB LB
A  B  RT LT

Mad Catz WWE All Stars Brawl Stick
 X  Y  LB LT
A  B  RB RT

Arcade pads typically have six face buttons, and come with different
layouts corresponding to the latter two arcade stick layouts, with the
rightmost column on the shoulder buttons.  Examples of face button
layouts:

Hori Fighting Commander OCTA, Mad Catz Street Fighter IV FightPad, PowerA FUSION Wired FightPad
X  Y  RB
A  B  RT

Hori Pad EX Turbo 2, Mad Catz WWE All Stars BrawlPad, Mortal Kombat X Fight Pad, PDP Versus Fighting Pad
X  Y  LB
A  B  RB


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


Drum kits also have multiple layouts.

Rock Band:
B  Y  X  A
    LB

Guitar Hero:
 Y  RB
A  X  B
   LB


Rock band keyboards use axes as bit fields:

LT   7  C
LT   6  C#
LT   5  D
LT   4  D#
LT   3  E
LT   2  F
LT   1  F#
LT   0  G
RT   7  G#
RT   6  A
RT   5  A#
RT   4  B
RT   3  C
RT   2  C#
RT   1  D
RT   0  D#
LSX  7  E
LSX  6  F
LSX  5  F#
LSX  4  G
LSX  3  G#
LSX  2  A
LSX  1  A#
LSX  0  B
LSX 15  C

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

char const *const AXIS_NAMES_GUITAR[]{
		"LSX",
		"LSY",
		"Whammy Bar",
		"Orientation",
		"Pickup Selector",
		"RT" };

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

char const *const HAT_NAMES_GAMEPAD[]{
		"D-pad Up",
		"D-pad Down",
		"D-pad Left",
		"D-pad Right",
		"POV Hat Up",
		"POV Hat Down",
		"POV Hat Left",
		"POV Hat Right" };

char const *const HAT_NAMES_ARCADE_STICK[]{
		"Joystick Up",
		"Joystick Down",
		"Joystick Left",
		"Joystick Right",
		nullptr,
		nullptr,
		nullptr,
		nullptr };

char const *const HAT_NAMES_GUITAR[]{
		"Strum/D-pad Up",
		"Strum/D-pad Down",
		"D-pad Left",
		"D-pad Right" };

char const *const BUTTON_NAMES_GAMEPAD[]{
		"A",
		"B",
		"X",
		"Y",
		"LT",
		"RT",
		"LB",
		"RB",
		"LSB",
		"RSB" };

char const *const BUTTON_NAMES_DRUMKIT[]{
		"Green",        // floor tom
		"Red",          // snare
		"Blue",         // low tom
		"Yellow",       // Rock Band high tom, Guitar Hero hi-hat
		nullptr,
		nullptr,
		"Orange",       // Guitar Hero crash cymbal
		"Bass Drum",
		"LSB",
		"RSB" };

char const *const BUTTON_NAMES_GUITAR[]{
		"Fret 1",
		"Fret 2",
		"Fret 3",
		"Fret 4",
		"Fret 5",
		"Fret Modifier",
		"RB",
		"RSB" };



//============================================================
//  base class for XInput controller handlers
//============================================================

class xinput_device_base : public device_info
{
public:
	virtual void configure() = 0;

protected:
	xinput_device_base(
			running_machine &machine,
			std::string &&name,
			std::string &&id,
			input_module &module,
			uint32_t player,
			XINPUT_CAPABILITIES const &caps,
			std::shared_ptr<xinput_api_helper> const &helper);

	// capabilities
	BYTE device_type() const { return m_capabilities.Type; }
	BYTE device_subtype() const { return m_capabilities.SubType; }
	bool has_button(WORD mask) const { return (m_capabilities.Gamepad.wButtons & mask) != 0; }
	bool has_trigger_left() const { return m_capabilities.Gamepad.bLeftTrigger != 0; }
	bool has_trigger_right() const { return m_capabilities.Gamepad.bRightTrigger != 0; }
	bool has_thumb_left_x() const { return m_capabilities.Gamepad.sThumbLX != 0; }
	bool has_thumb_left_y() const { return m_capabilities.Gamepad.sThumbLY != 0; }
	bool has_thumb_right_x() const { return m_capabilities.Gamepad.sThumbRX != 0; }
	bool has_thumb_right_y() const { return m_capabilities.Gamepad.sThumbRY != 0; }

	// device state
	WORD buttons() const { return m_xinput_state.Gamepad.wButtons; }
	BYTE trigger_left() const { return m_xinput_state.Gamepad.bLeftTrigger; }
	BYTE trigger_right() const { return m_xinput_state.Gamepad.bRightTrigger; }
	SHORT thumb_left_x() const { return m_xinput_state.Gamepad.sThumbLX; }
	SHORT thumb_left_y() const { return m_xinput_state.Gamepad.sThumbLY; }
	SHORT thumb_right_x() const { return m_xinput_state.Gamepad.sThumbRX; }
	SHORT thumb_right_y() const { return m_xinput_state.Gamepad.sThumbRY; }

	XINPUT_CAPABILITIES const &check_capabilities();
	bool read_state();

private:
	uint32_t const      m_player_index;
	XINPUT_CAPABILITIES m_capabilities;
	XINPUT_STATE        m_xinput_state;

	std::shared_ptr<xinput_api_helper> m_xinput_helper;
};


xinput_device_base::xinput_device_base(
		running_machine &machine,
		std::string &&name,
		std::string &&id,
		input_module &module,
		uint32_t player,
		XINPUT_CAPABILITIES const &caps,
		std::shared_ptr<xinput_api_helper> const &helper) :
	device_info(machine, std::move(name), std::move(id), DEVICE_CLASS_JOYSTICK, module),
	m_player_index(player),
	m_capabilities(caps),
	m_xinput_state{ 0 },
	m_xinput_helper(helper)
{
	// TODO: some controller types may have different capabilities format (e.g. DJ Hero turntable)

	// get friendly names for controller type and subtype
	char const *type_name = "unsupported";
	char const *subtype_name = "unsupported";
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
			break;
		case XINPUT_DEVSUBTYPE_ARCADE_STICK:
			subtype_name = "arcade stick";
			break;
		case 0x04: // XINPUT_DEVSUBTYPE_FLIGHT_STICK: work around MinGW header issues
			subtype_name = "flight stick";
			break;
		case XINPUT_DEVSUBTYPE_DANCE_PAD:
			subtype_name = "dance pad";
			break;
		case XINPUT_DEVSUBTYPE_GUITAR:
			subtype_name = "guitar";
			break;
		case XINPUT_DEVSUBTYPE_GUITAR_ALTERNATE:
			subtype_name = "alternate guitar";
			break;
		case XINPUT_DEVSUBTYPE_DRUM_KIT:
			subtype_name = "drum kit";
			break;
		case XINPUT_DEVSUBTYPE_GUITAR_BASS:
			subtype_name = "bass guitar";
			break;
		case XINPUT_DEVSUBTYPE_ARCADE_PAD:
			subtype_name = "arcade pad";
			break;
		}
		break;
	}

	// detect invalid axis resolutions
	bool const ltcap_bad = m_capabilities.Gamepad.bLeftTrigger && count_leading_zeros_32(m_capabilities.Gamepad.bLeftTrigger << 24);
	bool const rtcap_bad = m_capabilities.Gamepad.bRightTrigger && count_leading_zeros_32(m_capabilities.Gamepad.bRightTrigger << 24);
	bool const lsxcap_bad = m_capabilities.Gamepad.sThumbLX && count_leading_zeros_32(m_capabilities.Gamepad.sThumbLX << 16);
	bool const lsycap_bad = m_capabilities.Gamepad.sThumbLY && count_leading_zeros_32(m_capabilities.Gamepad.sThumbLY << 16);
	bool const rsxcap_bad = m_capabilities.Gamepad.sThumbRX && count_leading_zeros_32(m_capabilities.Gamepad.sThumbRX << 16);
	bool const rsycap_bad = m_capabilities.Gamepad.sThumbRY && count_leading_zeros_32(m_capabilities.Gamepad.sThumbRY << 16);

	// log some diagnostic information
	osd_printf_verbose(
			"XInput: Configuring player %d type 0x%02X (%s) sub type 0x%02X (%s).\n",
			m_player_index + 1,
			m_capabilities.Type,
			type_name,
			m_capabilities.SubType,
			subtype_name);
	osd_printf_verbose(
			"XInput: Switch capabilities A=%d B=%d X=%d Y=%d LB=%d RB=%d LSB=%d RSB=%d Start=%d Back=%d Up=%d Down=%d Left=%d Right=%d.\n",
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
			"XInput: Axis capabilities LT=0x%02X (%d-bit%s) RT=0x%02X (%d-bit%s) LSX=0x%04X (%d-bit%s) LSY=0x%04X (%d-bit%s) RSX=0x%04X (%d-bit%s) RSY=0x%04X (%d-bit%s).\n",
			m_capabilities.Gamepad.bLeftTrigger,
			count_leading_ones_32(uint32_t(m_capabilities.Gamepad.bLeftTrigger) << 24),
			ltcap_bad ? ", invalid" : "",
			m_capabilities.Gamepad.bRightTrigger,
			count_leading_ones_32(uint32_t(m_capabilities.Gamepad.bRightTrigger) << 24),
			rtcap_bad ? ", invalid" : "",
			m_capabilities.Gamepad.sThumbLX,
			count_leading_ones_32(uint32_t(uint16_t(m_capabilities.Gamepad.sThumbLX)) << 16),
			lsxcap_bad ? ", invalid" : "",
			m_capabilities.Gamepad.sThumbLY,
			count_leading_ones_32(uint32_t(uint16_t(m_capabilities.Gamepad.sThumbLY)) << 16),
			lsycap_bad ? ", invalid" : "",
			m_capabilities.Gamepad.sThumbRX,
			count_leading_ones_32(uint32_t(uint16_t(m_capabilities.Gamepad.sThumbRX)) << 16),
			rsxcap_bad ? ", invalid" : "",
			m_capabilities.Gamepad.sThumbRY,
			count_leading_ones_32(uint32_t(uint16_t(m_capabilities.Gamepad.sThumbRY)) << 16),
			rsycap_bad ? ", invalid" : "");

	// ignore capabilities if invalid
	bool ignore_caps = false;
	if (ltcap_bad || rtcap_bad || lsxcap_bad || lsycap_bad || rsxcap_bad || rsycap_bad)
	{
		// Retro-Bit Sega Saturn Control Pad reports garbage for axis resolutions and absence of several buttons
		osd_printf_verbose("XInput: Ignoring invalid capabilities (invalid axis resolution).\n");
		ignore_caps = true;
	}
	else if (!m_capabilities.Gamepad.wButtons && !m_capabilities.Gamepad.bLeftTrigger && !m_capabilities.Gamepad.bRightTrigger && !m_capabilities.Gamepad.sThumbLX && !m_capabilities.Gamepad.sThumbLY && !m_capabilities.Gamepad.sThumbRX && !m_capabilities.Gamepad.sThumbRY)
	{
		// 8BitDo SN30 Pro V1 reports no controls at all, which would be completely useless
		osd_printf_verbose("XInput: Ignoring invalid capabilities (no controls reported).\n");
		ignore_caps = true;
	}
	if (ignore_caps)
	{
		m_capabilities.Gamepad.wButtons = 0xf3ff;
		m_capabilities.Gamepad.bLeftTrigger = 0xff;
		m_capabilities.Gamepad.bRightTrigger = 0xff;
		m_capabilities.Gamepad.sThumbLX = s16(u16(0xffc0));
		m_capabilities.Gamepad.sThumbLY = s16(u16(0xffc0));
		m_capabilities.Gamepad.sThumbRX = s16(u16(0xffc0));
		m_capabilities.Gamepad.sThumbRY = s16(u16(0xffc0));
	}
}


bool xinput_device_base::read_state()
{
	// when losing focus, the packet number changes and everything is zeroed
	// when regaining focus, the packet number doesn't change when control states are returned

	// save previous packet number and try to read peripheral state
	//DWORD const prevpacket = m_xinput_state.dwPacketNumber;
	HRESULT const result = m_xinput_helper->xinput_get_state(m_player_index, &m_xinput_state);

	// only update if it succeeded and the packed number changed
	return !FAILED(result) /*&& (prevpacket != m_xinput_state.dwPacketNumber)*/;
}



//============================================================
//  general XInput controller handler
//============================================================

class xinput_joystick_device : public xinput_device_base
{
public:
	xinput_joystick_device(
			running_machine &machine,
			std::string &&name,
			std::string &&id,
			input_module &module,
			uint32_t player,
			XINPUT_CAPABILITIES const &caps,
			std::shared_ptr<xinput_api_helper> const &helper);

	virtual void poll() override;
	virtual void reset() override;

	virtual void configure() override;

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

	uint8_t m_switches[SWITCH_TOTAL];
	int32_t m_axes[AXIS_TOTAL];
};


xinput_joystick_device::xinput_joystick_device(
		running_machine &machine,
		std::string &&name,
		std::string &&id,
		input_module &module,
		uint32_t player,
		XINPUT_CAPABILITIES const &caps,
		std::shared_ptr<xinput_api_helper> const &helper) :
	xinput_device_base(machine, std::move(name), std::move(id), module, player, caps, helper)
{
	std::fill(std::begin(m_switches), std::end(m_switches), 0);
	std::fill(std::begin(m_axes), std::end(m_axes), 0);
}


void xinput_joystick_device::poll()

{
	// poll the device first, and skip if nothing changed
	if (!read_state())
		return;

	// translate button bits
	for (unsigned i = 0; std::size(SWITCH_BITS) > i; ++i)
		m_switches[SWITCH_A + i] = (buttons() & SWITCH_BITS[i]) ? 0xff : 0x00;

	// translate the triggers onto the negative side of the axes
	m_axes[AXIS_LT] = -normalize_absolute_axis(trigger_left(), -255, 255);
	m_axes[AXIS_RT] = -normalize_absolute_axis(trigger_right(), -255, 255);

	// translate full-precision axes - Y direction is opposite to what MAME uses
	m_axes[AXIS_LSX] = normalize_absolute_axis(thumb_left_x(), XINPUT_AXIS_MINVALUE, XINPUT_AXIS_MAXVALUE);
	m_axes[AXIS_LSY] = normalize_absolute_axis(-thumb_left_y(), XINPUT_AXIS_MINVALUE, XINPUT_AXIS_MAXVALUE);
	m_axes[AXIS_RSX] = normalize_absolute_axis(thumb_right_x(), XINPUT_AXIS_MINVALUE, XINPUT_AXIS_MAXVALUE);
	m_axes[AXIS_RSY] = normalize_absolute_axis(-thumb_right_y(), XINPUT_AXIS_MINVALUE, XINPUT_AXIS_MAXVALUE);

	// translate LT/RT switches for arcade sticks/pads
	m_switches[SWITCH_LT] = (0x80 <= trigger_left()) ? 0xff : 0x00;
	m_switches[SWITCH_RT] = (0x80 <= trigger_right()) ? 0xff : 0x00;

	// translate POV hat for flight sticks
	m_switches[SWITCH_HAT_UP] = (16'384 <= thumb_right_y()) ? 0xff : 0x00;
	m_switches[SWITCH_HAT_DOWN] = (-16'384 >= thumb_right_y()) ? 0xff : 0x00;
	m_switches[SWITCH_HAT_LEFT] = (-16'384 >= thumb_right_x()) ? 0xff : 0x00;
	m_switches[SWITCH_HAT_RIGHT] = (16'384 <= thumb_right_x()) ? 0xff : 0x00;

	// translate rudder and throttle for flight sticks
	m_axes[AXIS_RUDDER] = normalize_absolute_axis(trigger_left(), 0, 255);
	m_axes[AXIS_THROTTLE] = normalize_absolute_axis(trigger_right(), 0, 255);
}


void xinput_joystick_device::reset()
{
	std::fill(std::begin(m_switches), std::end(m_switches), 0);
	std::fill(std::begin(m_axes), std::end(m_axes), 0);
}


void xinput_joystick_device::configure()
{
	// TODO: proper support for dance mat and drum kit controllers

	// default characteristics for a gamepad
	bool lt_rt_button = false;
	bool lt_rt_fullaxis = false;
	bool rstick_hat = false;
	bool button_order_drumkit = false;
	char const *const *axis_names = AXIS_NAMES_GAMEPAD;
	input_item_id const *axis_ids = AXIS_IDS_GAMEPAD;
	char const *const *hat_names = HAT_NAMES_GAMEPAD;
	char const *const *button_names = BUTTON_NAMES_GAMEPAD;

	// consider the device type to decide how to map controls
	switch (device_type())
	{
	case XINPUT_DEVTYPE_GAMEPAD:
		switch (device_subtype())
		{
		case XINPUT_DEVSUBTYPE_WHEEL:
			axis_names = AXIS_NAMES_WHEEL;
			break;
		case XINPUT_DEVSUBTYPE_ARCADE_STICK:
			lt_rt_button = true;
			hat_names = HAT_NAMES_ARCADE_STICK;
			break;
		case 0x04: // XINPUT_DEVSUBTYPE_FLIGHT_STICK: work around MinGW header issues
			lt_rt_fullaxis = true;
			rstick_hat = true;
			axis_names = AXIS_NAMES_FLIGHT_STICK;
			axis_ids = AXIS_IDS_FLIGHT_STICK;
			break;
		case XINPUT_DEVSUBTYPE_DANCE_PAD:
			// TODO: proper support
			break;
		case XINPUT_DEVSUBTYPE_DRUM_KIT:
			button_order_drumkit = true;
			button_names = BUTTON_NAMES_DRUMKIT;
			break;
		case XINPUT_DEVSUBTYPE_ARCADE_PAD:
			lt_rt_button = true;
			break;
		}
		break;
	}

	// add bidirectional axes
	bool const axis_caps[]{
			has_thumb_left_x(),
			has_thumb_left_y(),
			!rstick_hat && has_thumb_right_x(),
			!rstick_hat && has_thumb_right_y(),
			lt_rt_fullaxis && has_trigger_left(),
			lt_rt_fullaxis && has_trigger_right() };
	for (unsigned i = 0; std::size(axis_caps) > i; ++i)
	{
		if (axis_caps[i])
		{
			device()->add_item(
					axis_names[i],
					axis_ids[i],
					generic_axis_get_state<int32_t>,
					&m_axes[AXIS_LSX + i]);
		}
	}

	// add hats
	bool const hat_caps[]{
			has_button(XINPUT_GAMEPAD_DPAD_UP),
			has_button(XINPUT_GAMEPAD_DPAD_DOWN),
			has_button(XINPUT_GAMEPAD_DPAD_LEFT),
			has_button(XINPUT_GAMEPAD_DPAD_RIGHT),
			rstick_hat && has_thumb_right_x(),
			rstick_hat && has_thumb_right_x(),
			rstick_hat && has_thumb_right_y(),
			rstick_hat && has_thumb_right_y() };
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
	std::pair<unsigned, bool> button_caps[]{
			{ SWITCH_A,   has_button(XINPUT_GAMEPAD_A) },
			{ SWITCH_B,   has_button(XINPUT_GAMEPAD_B) },
			{ SWITCH_X,   has_button(XINPUT_GAMEPAD_X) },
			{ SWITCH_Y,   has_button(XINPUT_GAMEPAD_Y) },
			{ SWITCH_LT,  lt_rt_button && has_trigger_left() },
			{ SWITCH_RT,  lt_rt_button && has_trigger_right() },
			{ SWITCH_LB,  has_button(XINPUT_GAMEPAD_LEFT_SHOULDER) },
			{ SWITCH_RB,  has_button(XINPUT_GAMEPAD_RIGHT_SHOULDER) },
			{ SWITCH_LSB, has_button(XINPUT_GAMEPAD_LEFT_THUMB) },
			{ SWITCH_RSB, has_button(XINPUT_GAMEPAD_RIGHT_THUMB) } };
	if (button_order_drumkit)
	{
		using std::swap;
		swap(button_caps[6], button_caps[7]);
	}
	input_item_id button_id = ITEM_ID_BUTTON1;
	for (unsigned i = 0; std::size(button_caps) > i; ++i)
	{
		auto const [offset, supported] = button_caps[i];
		if (supported)
		{
			device()->add_item(
					button_names[i],
					button_id++,
					generic_button_get_state<uint8_t>,
					&m_switches[offset]);
		}
	}

	// add start/back
	if (has_button(XINPUT_GAMEPAD_START))
	{
		device()->add_item(
				"Start",
				ITEM_ID_START,
				generic_button_get_state<uint8_t>,
				&m_switches[SWITCH_START]);
	}
	if (has_button(XINPUT_GAMEPAD_BACK))
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
			if (i ? has_trigger_right() : has_trigger_left())
			{
				device()->add_item(
						axis_names[4 + i],
						axis_ids[4 + i],
						generic_axis_get_state<int32_t>,
						&m_axes[AXIS_LT + i]);
			}
		}
	}
}



//============================================================
//  XInput guitar handler
//============================================================

class xinput_guitar_device : public xinput_device_base
{
public:
	xinput_guitar_device(
			running_machine &machine,
			std::string &&name,
			std::string &&id,
			input_module &module,
			uint32_t player,
			XINPUT_CAPABILITIES const &caps,
			std::shared_ptr<xinput_api_helper> const &helper);

	virtual void poll() override;
	virtual void reset() override;

	virtual void configure() override;

private:
	static inline constexpr USHORT SWITCH_BITS[] =
	{
		XINPUT_GAMEPAD_A,
		XINPUT_GAMEPAD_B,
		XINPUT_GAMEPAD_Y,
		XINPUT_GAMEPAD_X,
		XINPUT_GAMEPAD_LEFT_SHOULDER,
		XINPUT_GAMEPAD_LEFT_THUMB,
		XINPUT_GAMEPAD_RIGHT_SHOULDER,
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
		SWITCH_FRET1,
		SWITCH_FRET2,
		SWITCH_FRET3,
		SWITCH_FRET4,
		SWITCH_FRET5,
		SWITCH_FRET_MOD,    // indicates a "solo fret" is pressed, not a separate button
		SWITCH_RB,          // often reported present but no physical button
		SWITCH_RSB,         // usually reported absent
		SWITCH_START,
		SWITCH_BACK,        // also used for "Star Power" in some games

		SWITCH_DPAD_UP,     // D-pad bits
		SWITCH_DPAD_DOWN,
		SWITCH_DPAD_LEFT,
		SWITCH_DPAD_RIGHT,

		SWITCH_TOTAL
	};

	enum
	{
		AXIS_SLIDER,        // LSX, positive toward bridge
		AXIS_LSY,           // not used on guitar controllers?
		AXIS_WHAMMY,        // RSX, single-ended, neutral at negative extreme

		AXIS_ORIENT_NECK,   // RSY
		AXIS_ORIENT_BRIDGE, // LT, toward zero with frets up
		AXIS_ORIENT_BODY,   // RT, toward zero in right-handed orientation

		AXIS_PICKUP,        // LT, positive extreme at one end

		AXIS_TOTAL
	};

	uint8_t m_switches[SWITCH_TOTAL];
	int32_t m_axes[AXIS_TOTAL];
};


xinput_guitar_device::xinput_guitar_device(
		running_machine &machine,
		std::string &&name,
		std::string &&id,
		input_module &module,
		uint32_t player,
		XINPUT_CAPABILITIES const &caps,
		std::shared_ptr<xinput_api_helper> const &helper) :
	xinput_device_base(machine, std::move(name), std::move(id), module, player, caps, helper)
{
	std::fill(std::begin(m_switches), std::end(m_switches), 0);
	std::fill(std::begin(m_axes), std::end(m_axes), 0);
}


void xinput_guitar_device::poll()

{
	// poll the device first, and skip if nothing changed
	if (!read_state())
		return;

	// translate button bits
	for (unsigned i = 0; std::size(SWITCH_BITS) > i; ++i)
		m_switches[SWITCH_FRET1 + i] = (buttons() & SWITCH_BITS[i]) ? 0xff : 0x00;

	// translate miscellaneous axes
	m_axes[AXIS_SLIDER] = normalize_absolute_axis(thumb_left_x(), XINPUT_AXIS_MINVALUE, XINPUT_AXIS_MAXVALUE);
	m_axes[AXIS_LSY] = normalize_absolute_axis(-thumb_left_y(), XINPUT_AXIS_MINVALUE, XINPUT_AXIS_MAXVALUE);
	m_axes[AXIS_WHAMMY] = -normalize_absolute_axis(int32_t(thumb_right_x()) + 32'768, -65'535, 65'535);

	// translate orientation sensors
	m_axes[AXIS_ORIENT_NECK] = normalize_absolute_axis(-thumb_right_y(), XINPUT_AXIS_MINVALUE, XINPUT_AXIS_MAXVALUE);
	m_axes[AXIS_ORIENT_BRIDGE] = normalize_absolute_axis(trigger_left(), 0, 255);
	m_axes[AXIS_ORIENT_BODY] = normalize_absolute_axis(trigger_right(), 0, 255);

	// translate pickup selector
	m_axes[AXIS_PICKUP] = -normalize_absolute_axis(255 - trigger_right(), -255, 255);
}


void xinput_guitar_device::reset()
{
	std::fill(std::begin(m_switches), std::end(m_switches), 0);
	std::fill(std::begin(m_axes), std::end(m_axes), 0);
}


void xinput_guitar_device::configure()
{
	// add axes
	std::tuple<input_item_id, char const *, bool> const axis_caps[]{
			{ ITEM_ID_RXAXIS,  "Neck Slider",        has_thumb_left_x() },
			{ ITEM_ID_RYAXIS,  "LSY",                has_thumb_left_y() },
			{ ITEM_ID_SLIDER1, "Whammy Bar",         has_thumb_right_x() },
			{ ITEM_ID_YAXIS,   "Neck Orientation",   has_thumb_right_y() },
			{ ITEM_ID_XAXIS,   "Bridge Orientation", has_trigger_left() && has_trigger_right() },
			{ ITEM_ID_ZAXIS,   "Body Orientation",   has_trigger_right() },
			{ ITEM_ID_SLIDER2, "Pickup Selector",    has_trigger_left() && !has_trigger_right() } };
	for (unsigned i = 0; (AXIS_PICKUP - AXIS_SLIDER) >= i; ++i)
	{
		auto const [item, name, supported] = axis_caps[i];
		if (supported)
		{
			device()->add_item(
					name,
					item,
					generic_axis_get_state<uint32_t>,
					&m_axes[AXIS_SLIDER + i]);
		}
	}

	// add hats
	for (unsigned i = 0; (SWITCH_DPAD_RIGHT - SWITCH_DPAD_UP) >= i; ++i)
	{
		if (has_button(SWITCH_BITS[SWITCH_DPAD_UP + i]))
		{
			device()->add_item(
					HAT_NAMES_GUITAR[i],
					input_item_id(ITEM_ID_HAT1UP + i), // matches up/down/left/right order
					generic_button_get_state<uint8_t>,
					&m_switches[SWITCH_DPAD_UP + i]);
		}
	}

	// add buttons
	input_item_id button_id = ITEM_ID_BUTTON1;
	for (unsigned i = 0; (SWITCH_RSB - SWITCH_FRET1) >= i; ++i)
	{
		if (has_button(SWITCH_BITS[i]))
		{
			device()->add_item(
					BUTTON_NAMES_GUITAR[i],
					button_id++,
					generic_button_get_state<uint8_t>,
					&m_switches[SWITCH_FRET1 + i]);
		}
	}

	// add start/back
	if (has_button(XINPUT_GAMEPAD_START))
	{
		device()->add_item(
				"Start",
				ITEM_ID_START,
				generic_button_get_state<uint8_t>,
				&m_switches[SWITCH_START]);
	}
	if (has_button(XINPUT_GAMEPAD_BACK))
	{
		device()->add_item(
				"Back",
				ITEM_ID_SELECT,
				generic_button_get_state<uint8_t>,
				&m_switches[SWITCH_BACK]);
	}
}



//============================================================
//  XInput keyboard handler
//============================================================

class xinput_keyboard_device : public xinput_device_base
{
public:
	xinput_keyboard_device(
			running_machine &machine,
			std::string &&name,
			std::string &&id,
			input_module &module,
			uint32_t player,
			XINPUT_CAPABILITIES const &caps,
			std::shared_ptr<xinput_api_helper> const &helper);

	virtual void poll() override;
	virtual void reset() override;

	virtual void configure() override;

private:
	static inline constexpr USHORT SWITCH_BITS[] =
	{
		XINPUT_GAMEPAD_A,
		XINPUT_GAMEPAD_B,
		XINPUT_GAMEPAD_X,
		XINPUT_GAMEPAD_Y,
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
		SWITCH_START,
		SWITCH_BACK,

		SWITCH_DPAD_UP,     // D-pad bits
		SWITCH_DPAD_DOWN,
		SWITCH_DPAD_LEFT,
		SWITCH_DPAD_RIGHT,

		SWITCH_C1,
		SWITCH_C3 = SWITCH_C1 + 24,

		SWITCH_TOTAL
	};

	enum
	{
		AXIS_VELOCITY,

		AXIS_TOTAL
	};

	uint8_t m_switches[SWITCH_TOTAL];
	int32_t m_axes[AXIS_TOTAL];
};


xinput_keyboard_device::xinput_keyboard_device(
		running_machine &machine,
		std::string &&name,
		std::string &&id,
		input_module &module,
		uint32_t player,
		XINPUT_CAPABILITIES const &caps,
		std::shared_ptr<xinput_api_helper> const &helper) :
	xinput_device_base(machine, std::move(name), std::move(id), module, player, caps, helper)
{
	std::fill(std::begin(m_switches), std::end(m_switches), 0);
	std::fill(std::begin(m_axes), std::end(m_axes), 0);
}


void xinput_keyboard_device::poll()
{
	// TODO: how many bits are really velocity?
	// TODO: how are touch strip and overdrive read?

	// poll the device first, and skip if nothing changed
	if (!read_state())
		return;

	// translate button bits
	for (unsigned i = 0; std::size(SWITCH_BITS) > i; ++i)
		m_switches[SWITCH_A + i] = (buttons() & SWITCH_BITS[i]) ? 0xff : 0x00;

	// translate keys
	for (unsigned i = 0; 8 > i; ++i)
	{
		m_switches[SWITCH_C1 + i] = BIT(trigger_left(), 7 - i) ? 0xff : 0x00;
		m_switches[SWITCH_C1 + 8 + i] = BIT(trigger_right(), 7 - i) ? 0xff : 0x00;
		m_switches[SWITCH_C1 + 16 + i] = BIT(thumb_left_x(), 7 - i) ? 0xff : 0x00;
	}
	m_switches[SWITCH_C3] = BIT(thumb_left_x(), 15) ? 0xff : 0x00;

	// translate velocity
	m_axes[AXIS_VELOCITY] = -normalize_absolute_axis(BIT(thumb_left_x(), 8, 7), -127, 127);
}


void xinput_keyboard_device::reset()
{
	std::fill(std::begin(m_switches), std::end(m_switches), 0);
	std::fill(std::begin(m_axes), std::end(m_axes), 0);
}


void xinput_keyboard_device::configure()
{
	// TODO: button capabilities actually seem sane on this controller but we nuke them due to bad axis capabilities

	// add velocity axis
	device()->add_item(
			"Velocity",
			ITEM_ID_SLIDER1,
			generic_axis_get_state<int32_t>,
			&m_axes[AXIS_VELOCITY]);

	// add hats
	for (unsigned i = 0; (SWITCH_DPAD_RIGHT - SWITCH_DPAD_UP) >= i; ++i)
	{
		if (has_button(SWITCH_BITS[SWITCH_DPAD_UP + i]))
		{
			device()->add_item(
					HAT_NAMES_GAMEPAD[i],
					input_item_id(ITEM_ID_HAT1UP + i), // matches up/down/left/right order
					generic_button_get_state<uint8_t>,
					&m_switches[SWITCH_DPAD_UP + i]);
		}
	}

	// add buttons
	input_item_id button_id = ITEM_ID_BUTTON1;
	for (unsigned i = 0; (SWITCH_Y - SWITCH_A) >= i; ++i)
	{
		if (has_button(SWITCH_BITS[i]))
		{
			device()->add_item(
					BUTTON_NAMES_GAMEPAD[i],
					button_id++,
					generic_button_get_state<uint8_t>,
					&m_switches[SWITCH_A + i]);
		}
	}

	// add keys
	char const *const key_formats[]{
			"C %d", "C# %d", "D %d", "D# %d", "E %d", "F %d", "F# %d", "G %d", "G# %d", "A %d", "A# %d", "B %d" };
	for (unsigned i = 0; (SWITCH_C3 - SWITCH_C1) >= i; ++i)
	{
		device()->add_item(
				util::string_format(key_formats[i % 12], (i / 12) + 1),
				button_id++,
				generic_button_get_state<uint8_t>,
				&m_switches[SWITCH_C1 + i]);
	}

	// add start/back
	if (has_button(XINPUT_GAMEPAD_START))
	{
		device()->add_item(
				"Start",
				ITEM_ID_START,
				generic_button_get_state<uint8_t>,
				&m_switches[SWITCH_START]);
	}
	if (has_button(XINPUT_GAMEPAD_BACK))
	{
		device()->add_item(
				"Back",
				ITEM_ID_SELECT,
				generic_button_get_state<uint8_t>,
				&m_switches[SWITCH_BACK]);
	}
}



//============================================================
//  XInput joystick module
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
			XINPUT_STATE state{ 0 };

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
		osd_printf_error("XInput: Could not find API functions.\n");
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
	XINPUT_CAPABILITIES caps{ 0 };
	if (FAILED(xinput_get_capabilities(index, 0, &caps)))
		return nullptr;

	char device_name[16];
	snprintf(device_name, sizeof(device_name), "XInput Player %u", index + 1);

	// allocate the device object
	xinput_device_base *devinfo;
	switch (caps.Type)
	{
	case XINPUT_DEVTYPE_GAMEPAD:
		switch (caps.SubType)
		{
		case XINPUT_DEVSUBTYPE_GUITAR:
		case XINPUT_DEVSUBTYPE_GUITAR_ALTERNATE:
		case XINPUT_DEVSUBTYPE_GUITAR_BASS:
			devinfo = &module.devicelist().create_device<xinput_guitar_device>(
					machine,
					device_name,
					device_name,
					module,
					index,
					caps,
					shared_from_this());
			break;
		case 0x0f:
			devinfo = &module.devicelist().create_device<xinput_keyboard_device>(
					machine,
					device_name,
					device_name,
					module,
					index,
					caps,
					shared_from_this());
			break;
		default:
			devinfo = &module.devicelist().create_device<xinput_joystick_device>(
					machine,
					device_name,
					device_name,
					module,
					index,
					caps,
					shared_from_this());
		}
		break;
	default:
		devinfo = &module.devicelist().create_device<xinput_joystick_device>(
				machine,
				device_name,
				device_name,
				module,
				index,
				caps,
				shared_from_this());
	}

	// configure each controller to add buttons, axes, etc.
	devinfo->configure();

	return devinfo;
}

#else // defined(OSD_WINDOWS)

#include "input_module.h"

MODULE_NOT_SUPPORTED(xinput_joystick_module, OSD_JOYSTICKINPUT_PROVIDER, "xinput")

#endif // defined(OSD_WINDOWS)

MODULE_DEFINITION(JOYSTICKINPUT_XINPUT, xinput_joystick_module)
