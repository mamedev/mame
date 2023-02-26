// license:BSD-3-Clause
// copyright-holders:Brad Hughes, Vas Crabb
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


Some controllers use capabilities to indicate extended controller type
information:

                                Type    Sub     LSX     LSY     RSX
Band Hero Wireless Guitar       0x01    0x07    0x1430  0x0705  0x0001
Band Hero Wireless Drum Kit     0x01    0x08    0x1430  0x0805  0x0001
DJ Hero 2 Turntable             0x01    0x17    0x1430  0x1705  0x0001
Rock Band 3 Wireless Keyboard   0x01    0x0f    0x1bad  0x1330  0x0004


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


Rock Band keyboards use axes as bit fields:

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

#if defined(OSD_WINDOWS) || defined(SDLMAME_WIN32)

#include "input_xinput.h"

#include "assignmenthelper.h"

#include "interface/inputseq.h"
#include "modules/lib/osdobj_common.h"

// emu
#include "inpttype.h"

// lib/util
#include "util/coretmpl.h"

#include "eminline.h"

#include <algorithm>
#include <cstdint>
#include <iterator>
#include <string>
#include <tuple>
#include <utility>


#define XINPUT_LIBRARIES { "xinput1_4.dll", "xinput9_1_0.dll" }

#define XINPUT_AXIS_MINVALUE (-32'767)
#define XINPUT_AXIS_MAXVALUE (32'767)

namespace osd {

namespace {

using util::BIT;


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

char const *const BUTTON_NAMES_FLIGHT_STICK[]{
		"A",
		"B",
		"X",
		"Y",
		"LB",
		"RB",
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

char const *const BUTTON_NAMES_DRUMKIT[]{
		"Green",        // floor tom
		"Red",          // snare
		"Blue",         // low tom
		"Yellow",       // Rock Band high tom, Guitar Hero hi-hat
		"Orange",       // Guitar Hero crash cymbal
		"Bass Drum",
		"LSB",
		"RSB" };

char const *const BUTTON_NAMES_KEYBOARD[]{
		"A",
		"B",
		"X",
		"Y",
		"LB",
		"RB",
		"LSB",
		"RSB" };



//============================================================
//  base class for XInput controller handlers
//============================================================

class xinput_device_base : public device_info, protected joystick_assignment_helper
{
protected:
	xinput_device_base(
			std::string &&name,
			std::string &&id,
			input_module &module,
			u32 player,
			XINPUT_CAPABILITIES const &caps,
			xinput_api_helper const &helper);

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

	bool read_state();
	bool is_reset() const { return m_reset; }
	void set_reset() { m_reset = true; }

protected:
	template <unsigned M, unsigned N>
	static bool assign_ui_button(
			input_device::assignment_vector &assignments,
			ioport_type type,
			unsigned preferred,
			input_item_id (&switch_ids)[M],
			unsigned const (&numbered_buttons)[N],
			unsigned button_count);

	template <unsigned M, unsigned N>
	static void assign_ui_actions(
			input_device::assignment_vector &assignments,
			unsigned preferred_back,
			unsigned preferred_clear,
			unsigned preferred_help,
			unsigned start,
			unsigned back,
			input_item_id (&switch_ids)[M],
			unsigned const (&numbered_buttons)[N],
			unsigned button_count);

private:
	bool probe_extended_type();

	u32 const           m_player_index;
	XINPUT_CAPABILITIES m_capabilities;
	XINPUT_STATE        m_xinput_state;
	bool                m_reset;

	xinput_api_helper const &m_xinput_helper;
};


xinput_device_base::xinput_device_base(
		std::string &&name,
		std::string &&id,
		input_module &module,
		u32 player,
		XINPUT_CAPABILITIES const &caps,
		xinput_api_helper const &helper) :
	device_info(std::move(name), std::move(id), module),
	m_player_index(player),
	m_capabilities(caps),
	m_xinput_state{ 0 },
	m_reset(true),
	m_xinput_helper(helper)
{
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
			count_leading_ones_32(u32(m_capabilities.Gamepad.bLeftTrigger) << 24),
			ltcap_bad ? ", invalid" : "",
			m_capabilities.Gamepad.bRightTrigger,
			count_leading_ones_32(u32(m_capabilities.Gamepad.bRightTrigger) << 24),
			rtcap_bad ? ", invalid" : "",
			m_capabilities.Gamepad.sThumbLX,
			count_leading_ones_32(u32(u16(m_capabilities.Gamepad.sThumbLX)) << 16),
			lsxcap_bad ? ", invalid" : "",
			m_capabilities.Gamepad.sThumbLY,
			count_leading_ones_32(u32(u16(m_capabilities.Gamepad.sThumbLY)) << 16),
			lsycap_bad ? ", invalid" : "",
			m_capabilities.Gamepad.sThumbRX,
			count_leading_ones_32(u32(u16(m_capabilities.Gamepad.sThumbRX)) << 16),
			rsxcap_bad ? ", invalid" : "",
			m_capabilities.Gamepad.sThumbRY,
			count_leading_ones_32(u32(u16(m_capabilities.Gamepad.sThumbRY)) << 16),
			rsycap_bad ? ", invalid" : "");

	// ignore capabilities if invalid
	if (!probe_extended_type())
	{
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
}


bool xinput_device_base::read_state()
{
	// save previous packet number and try to read peripheral state
	DWORD const prevpacket = m_xinput_state.dwPacketNumber;
	HRESULT const result = m_xinput_helper.xinput_get_state(m_player_index, &m_xinput_state);

	// only update if it succeeded and the packed number changed
	if (FAILED(result))
	{
		return false;
	}
	else if (m_reset)
	{
		m_reset = false;
		return true;
	}
	else
	{
		return prevpacket != m_xinput_state.dwPacketNumber;
	}
}


template <unsigned M, unsigned N>
bool xinput_device_base::assign_ui_button(
		input_device::assignment_vector &assignments,
		ioport_type type,
		unsigned preferred,
		input_item_id (&switch_ids)[M],
		unsigned const (&numbered_buttons)[N],
		unsigned button_count)
{
	assert(N >= button_count);

	// use preferred button if available
	if (add_button_assignment(assignments, type, { switch_ids[preferred] }))
	{
		switch_ids[preferred] = ITEM_ID_INVALID;
		return true;
	}

	// otherwise find next available button
	for (unsigned i = 0; button_count > i; ++i)
	{
		if (add_button_assignment(assignments, type, { switch_ids[numbered_buttons[i]] }))
		{
			switch_ids[numbered_buttons[i]] = ITEM_ID_INVALID;
			return true;
		}
	}

	// didn't find a suitable button
	return false;
}


template <unsigned M, unsigned N>
void xinput_device_base::assign_ui_actions(
		input_device::assignment_vector &assignments,
		unsigned preferred_back,
		unsigned preferred_clear,
		unsigned preferred_help,
		unsigned start,
		unsigned back,
		input_item_id (&switch_ids)[M],
		unsigned const (&numbered_buttons)[N],
		unsigned button_count)
{
	// the first button is always UI select if present, or we can fall back to start
	if (1U <= button_count)
	{
		add_button_assignment(assignments, IPT_UI_SELECT, { switch_ids[numbered_buttons[0]] });
		switch_ids[numbered_buttons[0]] = ITEM_ID_INVALID;
	}
	else if (add_button_assignment(assignments, IPT_UI_SELECT, { switch_ids[start] }))
	{
		switch_ids[start] = ITEM_ID_INVALID;
	}

	// UI clear is usually X
	assign_ui_button(
			assignments,
			IPT_UI_CLEAR,
			preferred_clear,
			switch_ids,
			numbered_buttons,
			button_count);

	// UI back can fall back from B to the back button
	bool const assigned_back = assign_ui_button(
			assignments,
			IPT_UI_BACK,
			preferred_back,
			switch_ids,
			numbered_buttons,
			button_count);
	if (!assigned_back)
	{
		if (add_button_assignment(assignments, IPT_UI_BACK, { switch_ids[back] }))
			switch_ids[back] = ITEM_ID_INVALID;
	}

	// help takes Y if present
	assign_ui_button(
			assignments,
			IPT_UI_HELP,
			preferred_help,
			switch_ids,
			numbered_buttons,
			button_count);
}


bool xinput_device_base::probe_extended_type()
{
	switch (m_capabilities.Gamepad.sThumbLX)
	{
	case 0x1430:
		switch (m_capabilities.Gamepad.sThumbLY)
		{
		case 0x0705:
			osd_printf_verbose("XInput: Detected Band Hero guitar controller.\n");
			m_capabilities.Gamepad.sThumbLX = s16(u16(0xffc0)); // neck slider
			m_capabilities.Gamepad.sThumbLY = 0;
			m_capabilities.Gamepad.sThumbRX = s16(u16(0xffc0)); // whammy bar
			return true;
		case 0x0805:
			osd_printf_verbose("XInput: Detected Band Hero drum kit controller.\n");
			m_capabilities.Gamepad.sThumbLX = 0;
			m_capabilities.Gamepad.sThumbLY = s16(u16(0xffc0)); // green/red velocity
			m_capabilities.Gamepad.sThumbRX = s16(u16(0xffc0)); // blue/yellow velocity
			return true;
		case 0x1705:
			osd_printf_verbose("XInput: Detected DJ Hero turntable controller.\n");
			m_capabilities.Gamepad.sThumbLX = 0;
			m_capabilities.Gamepad.sThumbLY = s16(u16(0xffc0)); // turntable
			m_capabilities.Gamepad.sThumbRX = s16(u16(0xffc0)); // effects dial
			return true;
		}
		break;
	case 0x1bad:
		switch (m_capabilities.Gamepad.sThumbLY)
		{
		case 0x1330:
			osd_printf_verbose("XInput: Detected Rock Band keyboard controller.\n");
			m_capabilities.Gamepad.sThumbLX = 0; // keys, velocity
			m_capabilities.Gamepad.sThumbLY = 0; // not present?
			m_capabilities.Gamepad.sThumbRX = 0; // not present?
			return true;
		}
		break;
	}
	return false;
}



//============================================================
//  general XInput controller handler
//============================================================

class xinput_joystick_device : public xinput_device_base
{
public:
	xinput_joystick_device(
			std::string &&name,
			std::string &&id,
			input_module &module,
			u32 player,
			XINPUT_CAPABILITIES const &caps,
			xinput_api_helper const &helper);

	virtual void poll(bool relative_reset) override;
	virtual void reset() override;
	virtual void configure(input_device &device) override;

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

		AXIS_TOTAL
	};

	static bool assign_pedal(
			input_device::assignment_vector &assignments,
			bool fallback_shoulder,
			ioport_type type,
			input_item_id preferred_axis,
			input_item_id fallback_axis1,
			input_item_id fallback_axis2,
			input_item_modifier fallback_axis_modifier,
			input_item_id trigger_button,
			input_item_id shoulder_button,
			input_item_id numbered_button);

	u8  m_switches[SWITCH_TOTAL];
	s32 m_axes[AXIS_TOTAL];
};


xinput_joystick_device::xinput_joystick_device(
		std::string &&name,
		std::string &&id,
		input_module &module,
		u32 player,
		XINPUT_CAPABILITIES const &caps,
		xinput_api_helper const &helper) :
	xinput_device_base(std::move(name), std::move(id), module, player, caps, helper)
{
	std::fill(std::begin(m_switches), std::end(m_switches), 0);
	std::fill(std::begin(m_axes), std::end(m_axes), 0);
}


void xinput_joystick_device::poll(bool relative_reset)
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
}


void xinput_joystick_device::reset()
{
	set_reset();
	std::fill(std::begin(m_switches), std::end(m_switches), 0);
	std::fill(std::begin(m_axes), std::end(m_axes), 0);
}


void xinput_joystick_device::configure(input_device &device)
{
	// TODO: proper support for dance mat controllers

	// default characteristics for a gamepad
	bool button_diamond = true;
	bool pedal_fallback_shoulder = true;
	bool lt_rt_button = false;
	char const *const *axis_names = AXIS_NAMES_GAMEPAD;
	input_item_id const *preferred_axis_ids = AXIS_IDS_GAMEPAD;
	char const *const *hat_names = HAT_NAMES_GAMEPAD;
	char const *const *button_names = BUTTON_NAMES_GAMEPAD;

	// consider the device type to decide how to map controls
	switch (device_type())
	{
	case XINPUT_DEVTYPE_GAMEPAD:
		switch (device_subtype())
		{
		case XINPUT_DEVSUBTYPE_WHEEL:
			pedal_fallback_shoulder = false;
			axis_names = AXIS_NAMES_WHEEL;
			break;
		case XINPUT_DEVSUBTYPE_ARCADE_STICK:
			button_diamond = false;
			lt_rt_button = true;
			hat_names = HAT_NAMES_ARCADE_STICK;
			break;
		case XINPUT_DEVSUBTYPE_DANCE_PAD:
			// TODO: proper support
			button_diamond = false;
			break;
		case XINPUT_DEVSUBTYPE_ARCADE_PAD:
			button_diamond = false;
			lt_rt_button = true;
			break;
		}
		break;
	}

	// track item IDs for setting up default assignments
	input_device::assignment_vector assignments;
	input_item_id axis_ids[AXIS_TOTAL];
	input_item_id switch_ids[SWITCH_TOTAL];
	std::fill(std::begin(switch_ids), std::end(switch_ids), ITEM_ID_INVALID);

	// add bidirectional axes
	bool const axis_caps[]{
			has_thumb_left_x(),
			has_thumb_left_y(),
			has_thumb_right_x(),
			has_thumb_right_y() };
	for (unsigned i = 0; std::size(axis_caps) > i; ++i)
	{
		if (axis_caps[i])
		{
			axis_ids[AXIS_LSX + i] = device.add_item(
					axis_names[i],
					std::string_view(),
					preferred_axis_ids[i],
					generic_axis_get_state<s32>,
					&m_axes[AXIS_LSX + i]);
		}
		else
		{
			axis_ids[AXIS_LSX + i] = ITEM_ID_INVALID;
		}
	}

	// add hats
	bool const hat_caps[]{
			has_button(XINPUT_GAMEPAD_DPAD_UP),
			has_button(XINPUT_GAMEPAD_DPAD_DOWN),
			has_button(XINPUT_GAMEPAD_DPAD_LEFT),
			has_button(XINPUT_GAMEPAD_DPAD_RIGHT) };
	for (unsigned i = 0; (SWITCH_DPAD_RIGHT - SWITCH_DPAD_UP) >= i; ++i)
	{
		if (hat_caps[i])
		{
			switch_ids[SWITCH_DPAD_UP + i] = device.add_item(
					hat_names[i],
					std::string_view(),
					input_item_id(ITEM_ID_HAT1UP + i), // matches up/down/left/right order
					generic_button_get_state<u8>,
					&m_switches[SWITCH_DPAD_UP + i]);
		}
	}

	// add buttons
	std::pair<unsigned, bool> const button_caps[]{
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
	input_item_id button_id = ITEM_ID_BUTTON1;
	unsigned button_count = 0;
	unsigned numbered_buttons[SWITCH_RSB - SWITCH_A + 1];
	for (unsigned i = 0; std::size(button_caps) > i; ++i)
	{
		auto const [offset, supported] = button_caps[i];
		if (supported)
		{
			switch_ids[offset] = device.add_item(
					button_names[i],
					std::string_view(),
					button_id++,
					generic_button_get_state<u8>,
					&m_switches[offset]);
			numbered_buttons[button_count] = offset;

			// use these for automatically numbered buttons
			assignments.emplace_back(
					ioport_type(IPT_BUTTON1 + button_count++),
					SEQ_TYPE_STANDARD,
					input_seq(make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, switch_ids[offset])));
		}
	}

	// add start/back
	if (has_button(XINPUT_GAMEPAD_START))
	{
		switch_ids[SWITCH_START] = device.add_item(
				"Start",
				std::string_view(),
				ITEM_ID_START,
				generic_button_get_state<u8>,
				&m_switches[SWITCH_START]);
		add_button_assignment(assignments, IPT_START, { switch_ids[SWITCH_START] });
	}
	if (has_button(XINPUT_GAMEPAD_BACK))
	{
		switch_ids[SWITCH_BACK] = device.add_item(
				"Back",
				std::string_view(),
				ITEM_ID_SELECT,
				generic_button_get_state<u8>,
				&m_switches[SWITCH_BACK]);
		add_button_assignment(assignments, IPT_SELECT, { switch_ids[SWITCH_BACK] });
	}

	// add triggers/pedals
	if (!lt_rt_button)
	{
		for (unsigned i = 0; (AXIS_RT - AXIS_LT) >= i; ++i)
		{
			if (i ? has_trigger_right() : has_trigger_left())
			{
				axis_ids[AXIS_LT + i] = device.add_item(
						axis_names[std::size(axis_caps) + i],
						std::string_view(),
						preferred_axis_ids[std::size(axis_caps) + i],
						generic_axis_get_state<s32>,
						&m_axes[AXIS_LT + i]);
			}
			else
			{
				axis_ids[AXIS_LT + i] = ITEM_ID_INVALID;
			}
		}
	}

	// try to get a "complete" joystick for primary movement controls
	input_item_id directional_axes[2][2];
	choose_primary_stick(
			directional_axes,
			axis_ids[AXIS_LSX],
			axis_ids[AXIS_LSY],
			axis_ids[AXIS_RSX],
			axis_ids[AXIS_RSY]);

	// now set up controls using the primary joystick
	add_directional_assignments(
			assignments,
			directional_axes[0][0],
			directional_axes[0][1],
			switch_ids[SWITCH_DPAD_LEFT],
			switch_ids[SWITCH_DPAD_RIGHT],
			switch_ids[SWITCH_DPAD_UP],
			switch_ids[SWITCH_DPAD_DOWN]);

	// assign a secondary stick axis to joystick Z if available
	bool const stick_z = add_assignment(
			assignments,
			IPT_AD_STICK_Z,
			SEQ_TYPE_STANDARD,
			ITEM_CLASS_ABSOLUTE,
			ITEM_MODIFIER_NONE,
			{ directional_axes[1][1], directional_axes[1][0] });
	if (!stick_z)
	{
		// if both triggers are present, combine them, or failing that, fall back to a pair of buttons
		if ((ITEM_ID_INVALID != axis_ids[AXIS_LT]) && (ITEM_ID_INVALID != axis_ids[AXIS_RT]))
		{
			assignments.emplace_back(
					IPT_AD_STICK_Z,
					SEQ_TYPE_STANDARD,
					input_seq(
							make_code(ITEM_CLASS_ABSOLUTE, ITEM_MODIFIER_NONE, axis_ids[AXIS_LT]),
							make_code(ITEM_CLASS_ABSOLUTE, ITEM_MODIFIER_REVERSE, axis_ids[AXIS_RT])));
		}
		else if (add_axis_inc_dec_assignment(assignments, IPT_AD_STICK_Z, switch_ids[SWITCH_LB], switch_ids[SWITCH_RB]))
		{
			// took shoulder buttons
		}
		else if (add_axis_inc_dec_assignment(assignments, IPT_AD_STICK_Z, switch_ids[SWITCH_LT], switch_ids[SWITCH_RT]))
		{
			// took trigger buttons
		}
	}

	// prefer trigger axes for pedals, otherwise take half axes and buttons
	unsigned pedal_button = 0;
	bool const pedal1_numbered_button = assign_pedal(
			assignments,
			pedal_fallback_shoulder,
			IPT_PEDAL,
			axis_ids[AXIS_RT],
			directional_axes[1][1],
			directional_axes[0][1],
			ITEM_MODIFIER_NEG,
			switch_ids[SWITCH_RT],
			switch_ids[SWITCH_RB],
			(pedal_button < button_count)
				? switch_ids[numbered_buttons[pedal_button]]
				: ITEM_ID_INVALID);
	if (pedal1_numbered_button)
		++pedal_button;
	bool const pedal2_numbered_button = assign_pedal(
			assignments,
			pedal_fallback_shoulder,
			IPT_PEDAL2,
			axis_ids[AXIS_LT],
			directional_axes[1][1],
			directional_axes[0][1],
			ITEM_MODIFIER_POS,
			switch_ids[SWITCH_LT],
			switch_ids[SWITCH_LB],
			(pedal_button < button_count)
				? switch_ids[numbered_buttons[pedal_button]]
				: ITEM_ID_INVALID);
	if (pedal2_numbered_button)
		++pedal_button;
	if (pedal_button < button_count)
	{
		input_item_id const pedal_button_id = switch_ids[numbered_buttons[pedal_button]];
		assignments.emplace_back(
				IPT_PEDAL3,
				SEQ_TYPE_INCREMENT,
				input_seq(make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, pedal_button_id)));
	}

	// potentially use thumb sticks and/or D-pad and A/B/X/Y diamond for twin sticks
	add_twin_stick_assignments(
			assignments,
			axis_ids[AXIS_LSX],
			axis_ids[AXIS_LSY],
			axis_ids[AXIS_RSX],
			axis_ids[AXIS_RSY],
			button_diamond ? switch_ids[SWITCH_DPAD_LEFT]  : ITEM_ID_INVALID,
			button_diamond ? switch_ids[SWITCH_DPAD_RIGHT] : ITEM_ID_INVALID,
			button_diamond ? switch_ids[SWITCH_DPAD_UP]    : ITEM_ID_INVALID,
			button_diamond ? switch_ids[SWITCH_DPAD_DOWN]  : ITEM_ID_INVALID,
			button_diamond ? switch_ids[SWITCH_X]          : ITEM_ID_INVALID,
			button_diamond ? switch_ids[SWITCH_B]          : ITEM_ID_INVALID,
			button_diamond ? switch_ids[SWITCH_Y]          : ITEM_ID_INVALID,
			button_diamond ? switch_ids[SWITCH_A]          : ITEM_ID_INVALID);

	// assign UI select/back/clear/help
	assign_ui_actions(
			assignments,
			SWITCH_B,
			SWITCH_X,
			SWITCH_Y,
			SWITCH_START,
			SWITCH_BACK,
			switch_ids,
			numbered_buttons,
			button_count);

	// try to get a matching pair of buttons for previous/next group
	if (consume_button_pair(assignments, IPT_UI_PREV_GROUP, IPT_UI_NEXT_GROUP, switch_ids[SWITCH_LT], switch_ids[SWITCH_RT]))
	{
		// took digital triggers
	}
	else if (consume_trigger_pair(assignments, IPT_UI_PREV_GROUP, IPT_UI_NEXT_GROUP, axis_ids[AXIS_LT], axis_ids[AXIS_RT]))
	{
		// took analog triggers
	}
	else if (consume_axis_pair(assignments, IPT_UI_PREV_GROUP, IPT_UI_NEXT_GROUP, directional_axes[1][1]))
	{
		// took secondary Y
	}
	else if (consume_axis_pair(assignments, IPT_UI_PREV_GROUP, IPT_UI_NEXT_GROUP, directional_axes[1][0]))
	{
		// took secondary X
	}

	// try to assign secondary stick to page up/down
	consume_axis_pair(assignments, IPT_UI_PAGE_UP, IPT_UI_PAGE_DOWN, directional_axes[1][1]);

	// put focus previous/next on the shoulder buttons if available - this can be overloaded with zoom
	if (add_button_pair_assignment(assignments, IPT_UI_FOCUS_PREV, IPT_UI_FOCUS_NEXT, switch_ids[SWITCH_LB], switch_ids[SWITCH_RB]))
	{
		// took shoulder buttons
	}
	else if (add_axis_pair_assignment(assignments, IPT_UI_FOCUS_PREV, IPT_UI_FOCUS_NEXT, directional_axes[1][0]))
	{
		// took secondary X
	}

	// put zoom on the secondary stick if available, or fall back to shoulder buttons
	if (add_axis_pair_assignment(assignments, IPT_UI_ZOOM_OUT, IPT_UI_ZOOM_IN, directional_axes[1][0]))
	{
		// took secondary X
		if (axis_ids[AXIS_LSX] == directional_axes[1][0])
			add_button_assignment(assignments, IPT_UI_ZOOM_DEFAULT, { switch_ids[SWITCH_LSB] });
		else if (axis_ids[AXIS_RSX] == directional_axes[1][0])
			add_button_assignment(assignments, IPT_UI_ZOOM_DEFAULT, { switch_ids[SWITCH_RSB] });
		directional_axes[1][0] = ITEM_ID_INVALID;
	}
	else if (consume_button_pair(assignments, IPT_UI_ZOOM_OUT, IPT_UI_ZOOM_IN, switch_ids[SWITCH_LB], switch_ids[SWITCH_RB]))
	{
		// took shoulder buttons
	}

	// set default assignments
	device.set_default_assignments(std::move(assignments));
}


bool xinput_joystick_device::assign_pedal(
		input_device::assignment_vector &assignments,
		bool fallback_shoulder,
		ioport_type type,
		input_item_id preferred_axis,
		input_item_id fallback_axis1,
		input_item_id fallback_axis2,
		input_item_modifier fallback_axis_modifier,
		input_item_id trigger_button,
		input_item_id shoulder_button,
		input_item_id numbered_button)
{
	// first try the preferred trigger/pedal axis
	if (ITEM_ID_INVALID != preferred_axis)
	{
		assignments.emplace_back(
				type,
				SEQ_TYPE_STANDARD,
				input_seq(make_code(ITEM_CLASS_ABSOLUTE, ITEM_MODIFIER_NEG, preferred_axis)));
		return false;
	}

	// try adding half a joystick axis
	add_assignment(
			assignments,
			type,
			SEQ_TYPE_STANDARD,
			ITEM_CLASS_ABSOLUTE,
			fallback_axis_modifier,
			{ fallback_axis1, fallback_axis2 });

	// try a trigger button
	if (ITEM_ID_INVALID != trigger_button)
	{
		assignments.emplace_back(
				type,
				SEQ_TYPE_INCREMENT,
				input_seq(make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, trigger_button)));
		return false;
	}

	// try a shoulder button if appropriate
	if (fallback_shoulder && (ITEM_ID_INVALID != shoulder_button))
	{
		assignments.emplace_back(
				type,
				SEQ_TYPE_INCREMENT,
				input_seq(make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, shoulder_button)));
		return false;
	}

	// if no numbered button, nothing can be done
	if (ITEM_ID_INVALID == numbered_button)
		return false;

	// last resort
	assignments.emplace_back(
			type,
			SEQ_TYPE_INCREMENT,
			input_seq(make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, numbered_button)));
	return true;
}



//============================================================
//  XInput flight stick handler
//============================================================

class xinput_flight_stick_device : public xinput_device_base
{
public:
	xinput_flight_stick_device(
			std::string &&name,
			std::string &&id,
			input_module &module,
			u32 player,
			XINPUT_CAPABILITIES const &caps,
			xinput_api_helper const &helper);

	virtual void poll(bool relative_reset) override;
	virtual void reset() override;
	virtual void configure(input_device &device) override;

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

		SWITCH_HAT_UP,      // for POV hat as right stick
		SWITCH_HAT_DOWN,
		SWITCH_HAT_LEFT,
		SWITCH_HAT_RIGHT,

		SWITCH_TOTAL
	};

	enum
	{
		AXIS_RUDDER,        // LT/RT mapped as bidirectional axes
		AXIS_THROTTLE,

		AXIS_X,             // full-precision axes
		AXIS_Y,

		AXIS_TOTAL
	};

	u8  m_switches[SWITCH_TOTAL];
	s32 m_axes[AXIS_TOTAL];
};


xinput_flight_stick_device::xinput_flight_stick_device(
		std::string &&name,
		std::string &&id,
		input_module &module,
		u32 player,
		XINPUT_CAPABILITIES const &caps,
		xinput_api_helper const &helper) :
	xinput_device_base(std::move(name), std::move(id), module, player, caps, helper)
{
	std::fill(std::begin(m_switches), std::end(m_switches), 0);
	std::fill(std::begin(m_axes), std::end(m_axes), 0);
}


void xinput_flight_stick_device::poll(bool relative_reset)
{
	// poll the device first, and skip if nothing changed
	if (!read_state())
		return;

	// translate button bits
	for (unsigned i = 0; std::size(SWITCH_BITS) > i; ++i)
		m_switches[SWITCH_A + i] = (buttons() & SWITCH_BITS[i]) ? 0xff : 0x00;

	// translate rudder and throttle
	m_axes[AXIS_RUDDER] = normalize_absolute_axis(trigger_left(), 0, 255);
	m_axes[AXIS_THROTTLE] = normalize_absolute_axis(trigger_right(), 0, 255);

	// translate full-precision axes - Y direction is opposite to what MAME uses
	m_axes[AXIS_X] = normalize_absolute_axis(thumb_left_x(), XINPUT_AXIS_MINVALUE, XINPUT_AXIS_MAXVALUE);
	m_axes[AXIS_Y] = normalize_absolute_axis(-thumb_left_y(), XINPUT_AXIS_MINVALUE, XINPUT_AXIS_MAXVALUE);

	// translate right stick as POV hat
	m_switches[SWITCH_HAT_UP] = (16'384 <= thumb_right_y()) ? 0xff : 0x00;
	m_switches[SWITCH_HAT_DOWN] = (-16'384 >= thumb_right_y()) ? 0xff : 0x00;
	m_switches[SWITCH_HAT_LEFT] = (-16'384 >= thumb_right_x()) ? 0xff : 0x00;
	m_switches[SWITCH_HAT_RIGHT] = (16'384 <= thumb_right_x()) ? 0xff : 0x00;
}


void xinput_flight_stick_device::reset()
{
	set_reset();
	std::fill(std::begin(m_switches), std::end(m_switches), 0);
	std::fill(std::begin(m_axes), std::end(m_axes), 0);
}


void xinput_flight_stick_device::configure(input_device &device)
{
	// track item IDs for setting up default assignments
	input_device::assignment_vector assignments;
	input_item_id switch_ids[SWITCH_TOTAL];
	std::fill(std::begin(switch_ids), std::end(switch_ids), ITEM_ID_INVALID);

	// add bidirectional axes
	std::tuple<input_item_id, char const *, bool> const axis_caps[]{
			{ ITEM_ID_RZAXIS, "Rudder",     has_trigger_left() },
			{ ITEM_ID_ZAXIS,  "Throttle",   has_trigger_right() },
			{ ITEM_ID_XAXIS,  "Joystick X", has_thumb_left_x() },
			{ ITEM_ID_YAXIS,  "Joystick Y", has_thumb_left_y() } };
	input_item_id axis_ids[AXIS_TOTAL];
	for (unsigned i = 0; AXIS_TOTAL > i; ++i)
	{
		auto const [id, name, supported] = axis_caps[i];
		if (supported)
		{
			axis_ids[i] = device.add_item(
					name,
					std::string_view(),
					id,
					generic_axis_get_state<s32>,
					&m_axes[i]);
		}
		else
		{
			axis_ids[i] = ITEM_ID_INVALID;
		}
	}

	// add hats
	bool const hat_caps[]{
			has_button(XINPUT_GAMEPAD_DPAD_UP),
			has_button(XINPUT_GAMEPAD_DPAD_DOWN),
			has_button(XINPUT_GAMEPAD_DPAD_LEFT),
			has_button(XINPUT_GAMEPAD_DPAD_RIGHT),
			has_thumb_right_x(),
			has_thumb_right_x(),
			has_thumb_right_y(),
			has_thumb_right_y() };
	for (unsigned i = 0; (SWITCH_HAT_RIGHT - SWITCH_DPAD_UP) >= i; ++i)
	{
		if (hat_caps[i])
		{
			switch_ids[SWITCH_DPAD_UP + i] = device.add_item(
					HAT_NAMES_GAMEPAD[i],
					std::string_view(),
					input_item_id(ITEM_ID_HAT1UP + i), // matches up/down/left/right order
					generic_button_get_state<u8>,
					&m_switches[SWITCH_DPAD_UP + i]);
		}
	}

	// add buttons
	input_item_id button_id = ITEM_ID_BUTTON1;
	unsigned button_count = 0;
	unsigned numbered_buttons[SWITCH_RSB - SWITCH_A + 1];
	for (unsigned i = 0; (SWITCH_RSB - SWITCH_A) >= i; ++i)
	{
		if (has_button(SWITCH_BITS[i]))
		{
			switch_ids[SWITCH_A + i] = device.add_item(
					BUTTON_NAMES_FLIGHT_STICK[i],
					std::string_view(),
					button_id++,
					generic_button_get_state<u8>,
					&m_switches[SWITCH_A + i]);
			numbered_buttons[button_count] = SWITCH_A + i;

			// use these for automatically numbered buttons and pedals
			input_seq const seq(make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, switch_ids[SWITCH_A + i]));
			assignments.emplace_back(ioport_type(IPT_BUTTON1 + button_count), SEQ_TYPE_STANDARD, seq);
			if (3 > button_count)
				assignments.emplace_back(ioport_type(IPT_PEDAL + button_count), SEQ_TYPE_INCREMENT, seq);
			++button_count;
		}
	}

	// add start/back
	if (has_button(XINPUT_GAMEPAD_START))
	{
		switch_ids[SWITCH_START] = device.add_item(
				"Start",
				std::string_view(),
				ITEM_ID_START,
				generic_button_get_state<u8>,
				&m_switches[SWITCH_START]);
		add_button_assignment(assignments, IPT_START, { switch_ids[SWITCH_START] });
	}
	if (has_button(XINPUT_GAMEPAD_BACK))
	{
		switch_ids[SWITCH_BACK] = device.add_item(
				"Back",
				std::string_view(),
				ITEM_ID_SELECT,
				generic_button_get_state<u8>,
				&m_switches[SWITCH_BACK]);
		add_button_assignment(assignments, IPT_SELECT, { switch_ids[SWITCH_BACK] });
	}

	// use throttle for joystick Z, or rudder if it isn't available
	add_assignment(
			assignments,
			IPT_AD_STICK_Z,
			SEQ_TYPE_STANDARD,
			ITEM_CLASS_ABSOLUTE,
			ITEM_MODIFIER_NONE,
			{ axis_ids[AXIS_THROTTLE], axis_ids[AXIS_RUDDER] });

	// if throttle is available, use it for first two pedals, too
	if (ITEM_ID_INVALID != axis_ids[AXIS_THROTTLE])
	{
		assignments.emplace_back(
				IPT_PEDAL,
				SEQ_TYPE_STANDARD,
				input_seq(make_code(ITEM_CLASS_ABSOLUTE, ITEM_MODIFIER_NEG, axis_ids[AXIS_THROTTLE])));
		assignments.emplace_back(
				IPT_PEDAL2,
				SEQ_TYPE_STANDARD,
				input_seq(make_code(ITEM_CLASS_ABSOLUTE, ITEM_MODIFIER_POS, axis_ids[AXIS_THROTTLE])));
	}

	// find something to use for directional controls and navigation
	bool const axis_missing = (ITEM_ID_INVALID == axis_ids[AXIS_X]) || (ITEM_ID_INVALID == axis_ids[AXIS_Y]);
	bool const hat_complete = has_thumb_right_x() && has_thumb_right_y();
	if (axis_missing && hat_complete)
	{
		// X or Y missing - rely on POV hat
		add_directional_assignments(
				assignments,
				axis_ids[AXIS_X],
				axis_ids[AXIS_Y],
				switch_ids[SWITCH_HAT_LEFT],
				switch_ids[SWITCH_HAT_RIGHT],
				switch_ids[SWITCH_HAT_UP],
				switch_ids[SWITCH_HAT_DOWN]);

		// choose something for previous/next group
		if (consume_button_pair(assignments, IPT_UI_PREV_GROUP, IPT_UI_NEXT_GROUP, switch_ids[SWITCH_DPAD_UP], switch_ids[SWITCH_DPAD_DOWN]))
		{
			// took D-pad up/down
		}
		else if (consume_button_pair(assignments, IPT_UI_PREV_GROUP, IPT_UI_NEXT_GROUP, switch_ids[SWITCH_DPAD_LEFT], switch_ids[SWITCH_DPAD_RIGHT]))
		{
			// took D-pad left/right
		}
		else if (consume_axis_pair(assignments, IPT_UI_PREV_GROUP, IPT_UI_NEXT_GROUP, axis_ids[AXIS_RUDDER]))
		{
			// took rudder
		}
		else if (consume_axis_pair(assignments, IPT_UI_PREV_GROUP, IPT_UI_NEXT_GROUP, axis_ids[AXIS_THROTTLE]))
		{
			// took throttle
		}

		// choose something for zoom and focus previous/next
		if (add_axis_pair_assignment(assignments, IPT_UI_ZOOM_OUT, IPT_UI_ZOOM_IN, axis_ids[AXIS_THROTTLE]))
		{
			if (!add_axis_pair_assignment(assignments, IPT_UI_FOCUS_PREV, IPT_UI_FOCUS_NEXT, axis_ids[AXIS_RUDDER]))
				add_axis_pair_assignment(assignments, IPT_UI_FOCUS_PREV, IPT_UI_FOCUS_NEXT, axis_ids[AXIS_THROTTLE]);
		}
		else if (add_axis_pair_assignment(assignments, IPT_UI_ZOOM_OUT, IPT_UI_ZOOM_IN, axis_ids[AXIS_RUDDER]))
		{
			add_axis_pair_assignment(assignments, IPT_UI_FOCUS_PREV, IPT_UI_FOCUS_NEXT, axis_ids[AXIS_RUDDER]);
		}
		else if (add_button_pair_assignment(assignments, IPT_UI_ZOOM_OUT, IPT_UI_ZOOM_IN, switch_ids[SWITCH_DPAD_LEFT], switch_ids[SWITCH_DPAD_RIGHT]))
		{
			consume_button_pair(assignments, IPT_UI_FOCUS_PREV, IPT_UI_FOCUS_NEXT, switch_ids[SWITCH_DPAD_LEFT], switch_ids[SWITCH_DPAD_RIGHT]);
		}
	}
	else
	{
		// only use stick for the primary directional controls
		add_directional_assignments(
				assignments,
				axis_ids[AXIS_X],
				axis_ids[AXIS_Y],
				ITEM_ID_INVALID,
				ITEM_ID_INVALID,
				ITEM_ID_INVALID,
				ITEM_ID_INVALID);

		// assign the POV hat differently depending on whether rudder and/or throttle are present
		if ((ITEM_ID_INVALID != axis_ids[AXIS_RUDDER]) || (ITEM_ID_INVALID != axis_ids[AXIS_THROTTLE]))
		{
			// previous/next group
			if (consume_button_pair(assignments, IPT_UI_PREV_GROUP, IPT_UI_NEXT_GROUP, switch_ids[SWITCH_HAT_LEFT], switch_ids[SWITCH_HAT_RIGHT]))
			{
				// took hat left/right
			}
			else if (consume_button_pair(assignments, IPT_UI_PREV_GROUP, IPT_UI_NEXT_GROUP, switch_ids[SWITCH_DPAD_LEFT], switch_ids[SWITCH_DPAD_RIGHT]))
			{
				// took D-pad left/right
			}
			else if (consume_button_pair(assignments, IPT_UI_PREV_GROUP, IPT_UI_NEXT_GROUP, switch_ids[SWITCH_HAT_UP], switch_ids[SWITCH_HAT_DOWN]))
			{
				// took hat up/down
			}
			else if (consume_button_pair(assignments, IPT_UI_PREV_GROUP, IPT_UI_NEXT_GROUP, switch_ids[SWITCH_DPAD_UP], switch_ids[SWITCH_DPAD_DOWN]))
			{
				// took D-pad up/down
			}
			else if (consume_axis_pair(assignments, IPT_UI_PREV_GROUP, IPT_UI_NEXT_GROUP, axis_ids[AXIS_RUDDER]))
			{
				// took rudder
			}
			else if (consume_axis_pair(assignments, IPT_UI_PREV_GROUP, IPT_UI_NEXT_GROUP, axis_ids[AXIS_THROTTLE]))
			{
				// took throttle
			}

			// page up/down
			if (consume_button_pair(assignments, IPT_UI_PAGE_UP, IPT_UI_PAGE_DOWN, switch_ids[SWITCH_HAT_UP], switch_ids[SWITCH_HAT_DOWN]))
			{
				// took hat up/down
			}
			else if (consume_button_pair(assignments, IPT_UI_PAGE_UP, IPT_UI_PAGE_DOWN, switch_ids[SWITCH_DPAD_UP], switch_ids[SWITCH_DPAD_DOWN]))
			{
				// took D-pad up/down
			}
			else if (consume_button_pair(assignments, IPT_UI_PAGE_UP, IPT_UI_PAGE_DOWN, switch_ids[SWITCH_DPAD_LEFT], switch_ids[SWITCH_DPAD_RIGHT]))
			{
				// took D-pad left/right
			}

			// home/end
			if (consume_button_pair(assignments, IPT_UI_HOME, IPT_UI_END, switch_ids[SWITCH_DPAD_UP], switch_ids[SWITCH_DPAD_DOWN]))
			{
				// took D-pad up/down
			}
			else if (consume_button_pair(assignments, IPT_UI_HOME, IPT_UI_END, switch_ids[SWITCH_DPAD_LEFT], switch_ids[SWITCH_DPAD_RIGHT]))
			{
				// took D-pad left/right
			}

			// assign something for zoom - this can overlap with focus previous/next
			if (add_axis_pair_assignment(assignments, IPT_UI_ZOOM_OUT, IPT_UI_ZOOM_IN, axis_ids[AXIS_THROTTLE]))
			{
				// took throttle
			}
			else if (add_button_pair_assignment(assignments, IPT_UI_ZOOM_OUT, IPT_UI_ZOOM_IN, switch_ids[SWITCH_DPAD_LEFT], switch_ids[SWITCH_DPAD_RIGHT]))
			{
				// took D-pad left/right
			}
			else if (add_axis_pair_assignment(assignments, IPT_UI_ZOOM_OUT, IPT_UI_ZOOM_IN, axis_ids[AXIS_RUDDER]))
			{
				// took rudder
			}

			// assign something for focus previous/next
			if (add_axis_pair_assignment(assignments, IPT_UI_FOCUS_PREV, IPT_UI_FOCUS_NEXT, axis_ids[AXIS_RUDDER]))
			{
				// took rudder
			}
			else if (add_button_pair_assignment(assignments, IPT_UI_FOCUS_PREV, IPT_UI_FOCUS_NEXT, switch_ids[SWITCH_DPAD_LEFT], switch_ids[SWITCH_DPAD_RIGHT]))
			{
				// took D-pad left/right
			}
			else if (add_axis_pair_assignment(assignments, IPT_UI_FOCUS_PREV, IPT_UI_FOCUS_NEXT, axis_ids[AXIS_THROTTLE]))
			{
				// took throttle
			}
		}
		else
		{
			// previous/next group
			if (consume_button_pair(assignments, IPT_UI_PREV_GROUP, IPT_UI_NEXT_GROUP, switch_ids[SWITCH_HAT_UP], switch_ids[SWITCH_HAT_DOWN]))
			{
				// took hat up/down
			}
			else if (consume_button_pair(assignments, IPT_UI_PREV_GROUP, IPT_UI_NEXT_GROUP, switch_ids[SWITCH_DPAD_UP], switch_ids[SWITCH_DPAD_DOWN]))
			{
				// took D-pad up/down
			}
			else if (consume_button_pair(assignments, IPT_UI_PREV_GROUP, IPT_UI_NEXT_GROUP, switch_ids[SWITCH_HAT_LEFT], switch_ids[SWITCH_HAT_RIGHT]))
			{
				// took hat left/right
			}
			else if (consume_button_pair(assignments, IPT_UI_PREV_GROUP, IPT_UI_NEXT_GROUP, switch_ids[SWITCH_DPAD_LEFT], switch_ids[SWITCH_DPAD_RIGHT]))
			{
				// took D-pad left/right
			}

			// try to choose something for focus previous/next
			if (add_button_pair_assignment(assignments, IPT_UI_FOCUS_PREV, IPT_UI_FOCUS_NEXT, switch_ids[SWITCH_HAT_LEFT], switch_ids[SWITCH_HAT_RIGHT]))
			{
				// took hat left/right - use it for zoom as well
				consume_button_pair(assignments, IPT_UI_ZOOM_OUT, IPT_UI_ZOOM_IN, switch_ids[SWITCH_HAT_LEFT], switch_ids[SWITCH_HAT_RIGHT]);
			}
			else if (add_button_pair_assignment(assignments, IPT_UI_FOCUS_PREV, IPT_UI_FOCUS_NEXT, switch_ids[SWITCH_DPAD_LEFT], switch_ids[SWITCH_DPAD_RIGHT]))
			{
				// took D-pad left/right - use it for zoom as well
				consume_button_pair(assignments, IPT_UI_ZOOM_OUT, IPT_UI_ZOOM_IN, switch_ids[SWITCH_DPAD_LEFT], switch_ids[SWITCH_DPAD_RIGHT]);
			}
			else if (add_button_pair_assignment(assignments, IPT_UI_FOCUS_PREV, IPT_UI_FOCUS_NEXT, switch_ids[SWITCH_DPAD_UP], switch_ids[SWITCH_DPAD_DOWN]))
			{
				// took D-pad left/right - use it for zoom as well
				consume_button_pair(assignments, IPT_UI_ZOOM_OUT, IPT_UI_ZOOM_IN, switch_ids[SWITCH_DPAD_UP], switch_ids[SWITCH_DPAD_DOWN]);
			}

			// use D-pad for page up/down and home/end if it's still available
			if (consume_button_pair(assignments, IPT_UI_PAGE_UP, IPT_UI_PAGE_DOWN, switch_ids[SWITCH_DPAD_UP], switch_ids[SWITCH_DPAD_DOWN]))
			{
				// took D-pad up/down
			}
			else if (consume_button_pair(assignments, IPT_UI_PAGE_UP, IPT_UI_PAGE_DOWN, switch_ids[SWITCH_DPAD_LEFT], switch_ids[SWITCH_DPAD_RIGHT]))
			{
				// took D-pad left/right
			}
			consume_button_pair(assignments, IPT_UI_HOME, IPT_UI_END, switch_ids[SWITCH_DPAD_LEFT], switch_ids[SWITCH_DPAD_RIGHT]);
		}
	}

	// assign UI select/back/clear/help
	assign_ui_actions(
			assignments,
			SWITCH_B,
			SWITCH_X,
			SWITCH_Y,
			SWITCH_START,
			SWITCH_BACK,
			switch_ids,
			numbered_buttons,
			button_count);

	// set default assignments
	device.set_default_assignments(std::move(assignments));
}



//============================================================
//  XInput guitar handler
//============================================================

class xinput_guitar_device : public xinput_device_base
{
public:
	xinput_guitar_device(
			std::string &&name,
			std::string &&id,
			input_module &module,
			u32 player,
			XINPUT_CAPABILITIES const &caps,
			xinput_api_helper const &helper);

	virtual void poll(bool relative_reset) override;
	virtual void reset() override;
	virtual void configure(input_device &device) override;

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

	u8  m_switches[SWITCH_TOTAL];
	s32 m_axes[AXIS_TOTAL];
};


xinput_guitar_device::xinput_guitar_device(
		std::string &&name,
		std::string &&id,
		input_module &module,
		u32 player,
		XINPUT_CAPABILITIES const &caps,
		xinput_api_helper const &helper) :
	xinput_device_base(std::move(name), std::move(id), module, player, caps, helper)
{
	std::fill(std::begin(m_switches), std::end(m_switches), 0);
	std::fill(std::begin(m_axes), std::end(m_axes), 0);
}


void xinput_guitar_device::poll(bool relative_reset)
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
	m_axes[AXIS_WHAMMY] = -normalize_absolute_axis(s32(thumb_right_x()) + 32'768, -65'535, 65'535);

	// translate orientation sensors
	m_axes[AXIS_ORIENT_NECK] = normalize_absolute_axis(-thumb_right_y(), XINPUT_AXIS_MINVALUE, XINPUT_AXIS_MAXVALUE);
	m_axes[AXIS_ORIENT_BRIDGE] = normalize_absolute_axis(trigger_left(), 0, 255);
	m_axes[AXIS_ORIENT_BODY] = normalize_absolute_axis(trigger_right(), 0, 255);

	// translate pickup selector
	m_axes[AXIS_PICKUP] = -normalize_absolute_axis(trigger_left(), -255, 255);
}


void xinput_guitar_device::reset()
{
	set_reset();
	std::fill(std::begin(m_switches), std::end(m_switches), 0);
	std::fill(std::begin(m_axes), std::end(m_axes), 0);
}


void xinput_guitar_device::configure(input_device &device)
{
	// TODO: does subtype 0x06 indicate digital neck orientation sensor or lack of three-axis accelerometer?

	// track item IDs for setting up default assignments
	input_device::assignment_vector assignments;
	input_item_id switch_ids[SWITCH_TOTAL];
	std::fill(std::begin(switch_ids), std::end(switch_ids), ITEM_ID_INVALID);

	// add axes
	std::tuple<input_item_id, char const *, bool> const axis_caps[]{
			{ ITEM_ID_RXAXIS,  "Neck Slider",        has_thumb_left_x() },
			{ ITEM_ID_RYAXIS,  "LSY",                has_thumb_left_y() },
			{ ITEM_ID_SLIDER1, "Whammy Bar",         has_thumb_right_x() },
			{ ITEM_ID_YAXIS,   "Neck Orientation",   has_thumb_right_y() },
			{ ITEM_ID_XAXIS,   "Bridge Orientation", has_trigger_left() && has_trigger_right() },
			{ ITEM_ID_ZAXIS,   "Body Orientation",   has_trigger_right() },
			{ ITEM_ID_SLIDER2, "Pickup Selector",    has_trigger_left() && !has_trigger_right() } };
	input_item_id axis_ids[AXIS_TOTAL];
	for (unsigned i = 0; AXIS_TOTAL > i; ++i)
	{
		auto const [item, name, supported] = axis_caps[i];
		if (supported)
		{
			axis_ids[i] = device.add_item(
					name,
					std::string_view(),
					item,
					generic_axis_get_state<s32>,
					&m_axes[i]);
		}
		else
		{
			axis_ids[i] = ITEM_ID_INVALID;
		}
	}

	// add hats
	for (unsigned i = 0; (SWITCH_DPAD_RIGHT - SWITCH_DPAD_UP) >= i; ++i)
	{
		if (has_button(SWITCH_BITS[SWITCH_DPAD_UP + i]))
		{
			switch_ids[SWITCH_DPAD_UP + i] = device.add_item(
					HAT_NAMES_GUITAR[i],
					std::string_view(),
					input_item_id(ITEM_ID_HAT1UP + i), // matches up/down/left/right order
					generic_button_get_state<u8>,
					&m_switches[SWITCH_DPAD_UP + i]);
		}
	}

	// add buttons
	input_item_id button_id = ITEM_ID_BUTTON1;
	unsigned button_count = 0;
	unsigned numbered_buttons[SWITCH_FRET5 - SWITCH_FRET1 + 1];
	for (unsigned i = 0; (SWITCH_RSB - SWITCH_FRET1) >= i; ++i)
	{
		if (has_button(SWITCH_BITS[i]))
		{
			switch_ids[SWITCH_FRET1 + i] = device.add_item(
					BUTTON_NAMES_GUITAR[i],
					std::string_view(),
					button_id++,
					generic_button_get_state<u8>,
					&m_switches[SWITCH_FRET1 + i]);

			// use fret buttons for automatically numbered buttons and pedals
			if ((SWITCH_FRET5 - SWITCH_FRET1) >= i)
			{
				numbered_buttons[button_count] = SWITCH_FRET1 + i;
				input_seq const seq(make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, switch_ids[SWITCH_FRET1 + i]));
				assignments.emplace_back(ioport_type(IPT_BUTTON1 + button_count), SEQ_TYPE_STANDARD, seq);
				if (((ITEM_ID_INVALID != axis_ids[AXIS_WHAMMY]) ? 2 : 3) > button_count)
				{
					ioport_type const first_pedal = (ITEM_ID_INVALID != axis_ids[AXIS_WHAMMY])
							? IPT_PEDAL2
							: IPT_PEDAL;
					assignments.emplace_back(
							ioport_type(first_pedal + button_count),
							SEQ_TYPE_INCREMENT,
							seq);
				}
				++button_count;
			}
		}
	}

	// add start/back
	if (has_button(XINPUT_GAMEPAD_START))
	{
		switch_ids[SWITCH_START] = device.add_item(
				"Start",
				std::string_view(),
				ITEM_ID_START,
				generic_button_get_state<u8>,
				&m_switches[SWITCH_START]);
		add_button_assignment(assignments, IPT_START, { switch_ids[SWITCH_START] });
	}
	if (has_button(XINPUT_GAMEPAD_BACK))
	{
		switch_ids[SWITCH_BACK] = device.add_item(
				"Back",
				std::string_view(),
				ITEM_ID_SELECT,
				generic_button_get_state<u8>,
				&m_switches[SWITCH_BACK]);
		add_button_assignment(assignments, IPT_SELECT, { switch_ids[SWITCH_BACK] });
	}

	// use the D-pad for directional controls - accelerometers are an annoyance
	add_directional_assignments(
			assignments,
			ITEM_ID_INVALID,
			ITEM_ID_INVALID,
			switch_ids[SWITCH_DPAD_LEFT],
			switch_ids[SWITCH_DPAD_RIGHT],
			switch_ids[SWITCH_DPAD_UP],
			switch_ids[SWITCH_DPAD_DOWN]);

	// use the whammy bar for the first pedal and focus next if present
	if (ITEM_ID_INVALID != axis_ids[AXIS_WHAMMY])
	{
		assignments.emplace_back(
				IPT_PEDAL,
				SEQ_TYPE_STANDARD,
				input_seq(make_code(ITEM_CLASS_ABSOLUTE, ITEM_MODIFIER_NEG, axis_ids[AXIS_WHAMMY])));
		assignments.emplace_back(
				IPT_UI_FOCUS_NEXT,
				SEQ_TYPE_STANDARD,
				input_seq(make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_NEG, axis_ids[AXIS_WHAMMY])));
	}

	// use the neck slider for a couple of things if it's present
	if (ITEM_ID_INVALID != axis_ids[AXIS_SLIDER])
	{
		input_seq const seq(make_code(ITEM_CLASS_ABSOLUTE, ITEM_MODIFIER_NONE, axis_ids[AXIS_SLIDER]));
		assignments.emplace_back(IPT_PADDLE,     SEQ_TYPE_STANDARD, seq);
		assignments.emplace_back(IPT_AD_STICK_X, SEQ_TYPE_STANDARD, seq);
	}

	// assign UI select/back/clear/help
	assign_ui_actions(
			assignments,
			SWITCH_FRET2,
			SWITCH_FRET3,
			SWITCH_FRET4,
			SWITCH_START,
			SWITCH_BACK,
			switch_ids,
			numbered_buttons,
			button_count);

	// set default assignments
	device.set_default_assignments(std::move(assignments));
}



//============================================================
//  XInput drum kit handler
//============================================================

class xinput_drumkit_device : public xinput_device_base
{
public:
	xinput_drumkit_device(
			std::string &&name,
			std::string &&id,
			input_module &module,
			u32 player,
			XINPUT_CAPABILITIES const &caps,
			xinput_api_helper const &helper);

	virtual void poll(bool relative_reset) override;
	virtual void reset() override;
	virtual void configure(input_device &device) override;

private:
	static inline constexpr USHORT SWITCH_BITS[] =
	{
		XINPUT_GAMEPAD_A,
		XINPUT_GAMEPAD_B,
		XINPUT_GAMEPAD_X,
		XINPUT_GAMEPAD_Y,
		XINPUT_GAMEPAD_RIGHT_SHOULDER,
		XINPUT_GAMEPAD_LEFT_SHOULDER,
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
		SWITCH_GREEN,       // button bits
		SWITCH_RED,
		SWITCH_BLUE,
		SWITCH_YELLOW,
		SWITCH_ORANGE,
		SWITCH_BASS_DRUM,
		SWITCH_LSB,
		SWITCH_RSB,
		SWITCH_START,
		SWITCH_BACK,

		SWITCH_DPAD_UP,     // D-pad bits
		SWITCH_DPAD_DOWN,
		SWITCH_DPAD_LEFT,
		SWITCH_DPAD_RIGHT,

		SWITCH_TOTAL
	};

	enum
	{
		AXIS_GREEN,         // LSY low
		AXIS_RED,           // LSY high
		AXIS_BLUE,          // RSX high
		AXIS_YELLOW,        // RSX low
		AXIS_ORANGE,        // RSY low
		AXIS_BASS_DRUM,     // RSY high

		AXIS_TOTAL
	};

	u8  m_switches[SWITCH_TOTAL];
	s32 m_axes[AXIS_TOTAL];
};


xinput_drumkit_device::xinput_drumkit_device(
		std::string &&name,
		std::string &&id,
		input_module &module,
		u32 player,
		XINPUT_CAPABILITIES const &caps,
		xinput_api_helper const &helper) :
	xinput_device_base(std::move(name), std::move(id), module, player, caps, helper)
{
	std::fill(std::begin(m_switches), std::end(m_switches), 0);
	std::fill(std::begin(m_axes), std::end(m_axes), 0);
}


void xinput_drumkit_device::poll(bool relative_reset)
{
	// poll the device first, and skip if nothing changed
	if (!read_state())
		return;

	// translate button bits
	for (unsigned i = 0; std::size(SWITCH_BITS) > i; ++i)
		m_switches[SWITCH_GREEN + i] = (buttons() & SWITCH_BITS[i]) ? 0xff : 0x00;

	// translate axes
	m_axes[AXIS_GREEN] = -normalize_absolute_axis(BIT(u16(thumb_left_y()), 0, 8), -255, 255);
	m_axes[AXIS_RED] = -normalize_absolute_axis(BIT(u16(thumb_left_y()), 8, 8), -255, 255);
	m_axes[AXIS_BLUE] = -normalize_absolute_axis(BIT(u16(thumb_right_x()), 8, 8), -255, 255);
	m_axes[AXIS_YELLOW] = -normalize_absolute_axis(BIT(u16(thumb_right_x()), 0, 8), -255, 255);
	m_axes[AXIS_ORANGE] = -normalize_absolute_axis(BIT(u16(thumb_right_y()), 0, 8), -255, 255);
	m_axes[AXIS_BASS_DRUM] = -normalize_absolute_axis(BIT(u16(thumb_right_y()), 8, 8), -255, 255);
}


void xinput_drumkit_device::reset()
{
	set_reset();
	std::fill(std::begin(m_switches), std::end(m_switches), 0);
	std::fill(std::begin(m_axes), std::end(m_axes), 0);
}


void xinput_drumkit_device::configure(input_device &device)
{
	// track item IDs for setting up default assignments
	input_device::assignment_vector assignments;
	input_item_id switch_ids[SWITCH_TOTAL];
	std::fill(std::begin(switch_ids), std::end(switch_ids), ITEM_ID_INVALID);

	// add axes
	std::tuple<input_item_id, char const *, bool> const axis_caps[]{
			{ ITEM_ID_XAXIS,  "Green Velocity",     has_thumb_left_y() },
			{ ITEM_ID_YAXIS,  "Red Velocity",       has_thumb_left_y() },
			{ ITEM_ID_ZAXIS,  "Blue Velocity",      has_thumb_right_x() },
			{ ITEM_ID_RXAXIS, "Yellow Velocity",    has_thumb_right_x() },
			{ ITEM_ID_RYAXIS, "Orange Velocity",    has_thumb_right_y() },
			{ ITEM_ID_RZAXIS, "Bass Drum Velocity", has_thumb_right_y() } };
	for (unsigned i = 0; (AXIS_BASS_DRUM - AXIS_GREEN) >= i; ++i)
	{
		auto const [item, name, supported] = axis_caps[i];
		if (supported)
		{
			device.add_item(
					name,
					std::string_view(),
					item,
					generic_axis_get_state<s32>,
					&m_axes[AXIS_GREEN + i]);
		}
	}

	// add hats
	for (unsigned i = 0; (SWITCH_DPAD_RIGHT - SWITCH_DPAD_UP) >= i; ++i)
	{
		if (has_button(SWITCH_BITS[SWITCH_DPAD_UP + i]))
		{
			switch_ids[SWITCH_DPAD_UP + i] = device.add_item(
					HAT_NAMES_GAMEPAD[i],
					std::string_view(),
					input_item_id(ITEM_ID_HAT1UP + i), // matches up/down/left/right order
					generic_button_get_state<u8>,
					&m_switches[SWITCH_DPAD_UP + i]);
		}
	}

	// add buttons
	unsigned button_count = 0;
	unsigned numbered_buttons[SWITCH_RSB - SWITCH_GREEN + 1];
	for (unsigned i = 0; (SWITCH_RSB - SWITCH_GREEN) >= i; ++i)
	{
		if (has_button(SWITCH_BITS[i]))
		{
			switch_ids[SWITCH_GREEN + i] = device.add_item(
					BUTTON_NAMES_DRUMKIT[i],
					std::string_view(),
					input_item_id(ITEM_ID_BUTTON1 + button_count),
					generic_button_get_state<u8>,
					&m_switches[SWITCH_GREEN + i]);
			numbered_buttons[button_count] = SWITCH_GREEN + i;

			// use these for automatically numbered buttons and pedals
			input_seq const seq(make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, switch_ids[SWITCH_GREEN + i]));
			assignments.emplace_back(ioport_type(IPT_BUTTON1 + button_count), SEQ_TYPE_STANDARD, seq);
			if (3 > button_count)
				assignments.emplace_back(ioport_type(IPT_PEDAL + button_count), SEQ_TYPE_INCREMENT, seq);
			++button_count;
		}
	}

	// add start/back
	if (has_button(XINPUT_GAMEPAD_START))
	{
		switch_ids[SWITCH_START] = device.add_item(
				"Start",
				std::string_view(),
				ITEM_ID_START,
				generic_button_get_state<u8>,
				&m_switches[SWITCH_START]);
		add_button_assignment(assignments, IPT_START, { switch_ids[SWITCH_START] });
	}
	if (has_button(XINPUT_GAMEPAD_BACK))
	{
		switch_ids[SWITCH_BACK] = device.add_item(
				"Back",
				std::string_view(),
				ITEM_ID_SELECT,
				generic_button_get_state<u8>,
				&m_switches[SWITCH_BACK]);
		add_button_assignment(assignments, IPT_SELECT, { switch_ids[SWITCH_BACK] });
	}

	// use the D-pad for directional controls
	add_directional_assignments(
			assignments,
			ITEM_ID_INVALID,
			ITEM_ID_INVALID,
			switch_ids[SWITCH_DPAD_LEFT],
			switch_ids[SWITCH_DPAD_RIGHT],
			switch_ids[SWITCH_DPAD_UP],
			switch_ids[SWITCH_DPAD_DOWN]);

	// use the D-pad and A/B/X/Y diamond for twin sticks
	add_twin_stick_assignments(
			assignments,
			ITEM_ID_INVALID,
			ITEM_ID_INVALID,
			ITEM_ID_INVALID,
			ITEM_ID_INVALID,
			switch_ids[SWITCH_DPAD_LEFT],
			switch_ids[SWITCH_DPAD_RIGHT],
			switch_ids[SWITCH_DPAD_UP],
			switch_ids[SWITCH_DPAD_DOWN],
			switch_ids[SWITCH_BLUE],
			switch_ids[SWITCH_RED],
			switch_ids[SWITCH_YELLOW],
			switch_ids[SWITCH_GREEN]);

	// assign UI select/back/clear/help
	assign_ui_actions(
			assignments,
			SWITCH_RED,
			SWITCH_BLUE,
			SWITCH_YELLOW,
			SWITCH_START,
			SWITCH_BACK,
			switch_ids,
			numbered_buttons,
			button_count);

	// use bass drum pedal for focus next if available to make the system selection menu usable
	if (add_button_assignment(assignments, IPT_UI_FOCUS_NEXT, { switch_ids[SWITCH_BASS_DRUM] }))
		switch_ids[SWITCH_BASS_DRUM] = ITEM_ID_INVALID;

	// set default assignments
	device.set_default_assignments(std::move(assignments));
}



//============================================================
//  XInput turntable handler
//============================================================

class xinput_turntable_device : public xinput_device_base
{
public:
	xinput_turntable_device(
			std::string &&name,
			std::string &&id,
			input_module &module,
			u32 player,
			XINPUT_CAPABILITIES const &caps,
			xinput_api_helper const &helper);

	virtual void poll(bool relative_reset) override;
	virtual void reset() override;
	virtual void configure(input_device &device) override;

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

		SWITCH_GREEN,       // RT bits
		SWITCH_RED,
		SWITCH_BLUE,

		SWITCH_TOTAL
	};

	enum
	{
		AXIS_TURNTABLE,     // LSY
		AXIS_EFFECT,        // RSX
		AXIS_CROSSFADE,     // RSY

		AXIS_TOTAL
	};

	u8  m_switches[SWITCH_TOTAL];
	s32 m_axes[AXIS_TOTAL];
	u16 m_prev_effect;
};


xinput_turntable_device::xinput_turntable_device(
		std::string &&name,
		std::string &&id,
		input_module &module,
		u32 player,
		XINPUT_CAPABILITIES const &caps,
		xinput_api_helper const &helper) :
	xinput_device_base(std::move(name), std::move(id), module, player, caps, helper),
	m_prev_effect(0)
{
	std::fill(std::begin(m_switches), std::end(m_switches), 0);
	std::fill(std::begin(m_axes), std::end(m_axes), 0);
}


void xinput_turntable_device::poll(bool relative_reset)
{
	// poll the device first
	bool const was_reset = is_reset();
	if (read_state())
	{
		// translate button bits
		for (unsigned i = 0; std::size(SWITCH_BITS) > i; ++i)
			m_switches[SWITCH_A + i] = (buttons() & SWITCH_BITS[i]) ? 0xff : 0x00;

		// translate RT bits
		for (unsigned i = 0; (SWITCH_BLUE - SWITCH_GREEN) >= i; ++i)
			m_switches[SWITCH_GREEN + i] = BIT(trigger_right(), i) ? 0xff : 0x00;

		// translate axes
		m_axes[AXIS_TURNTABLE] = s32(thumb_left_y()) * input_device::RELATIVE_PER_PIXEL * 2;
		m_axes[AXIS_CROSSFADE] = normalize_absolute_axis(thumb_right_y(), XINPUT_AXIS_MINVALUE, XINPUT_AXIS_MAXVALUE);
	}

	// handle effect dial
	if (was_reset)
	{
		// just grab the current count after regaining focus
		m_prev_effect = u16(thumb_right_x());
	}
	else if (relative_reset)
	{
		// convert value to relative displacement
		s32 effect_delta = s32(u32(u16(thumb_right_x()))) - s32(u32(m_prev_effect));
		m_prev_effect = u16(thumb_right_x());
		if (0x8000 < effect_delta)
			effect_delta -= 0x1'0000;
		else if (-0x8000 > effect_delta)
			effect_delta += 0x1'0000;
		m_axes[AXIS_EFFECT] = effect_delta * input_device::RELATIVE_PER_PIXEL / 128;
	}
}


void xinput_turntable_device::reset()
{
	set_reset();
	std::fill(std::begin(m_switches), std::end(m_switches), 0);
	std::fill(std::begin(m_axes), std::end(m_axes), 0);
}


void xinput_turntable_device::configure(input_device &device)
{
	// track item IDs for setting up default assignments
	input_device::assignment_vector assignments;
	input_item_id switch_ids[SWITCH_TOTAL];
	std::fill(std::begin(switch_ids), std::end(switch_ids), ITEM_ID_INVALID);

	// add axes
	input_item_id const turntable_id = device.add_item(
			"Turntable",
			std::string_view(),
			ITEM_ID_ADD_RELATIVE1,
			generic_axis_get_state<s32>,
			&m_axes[AXIS_TURNTABLE]);
	input_item_id const effect_id = device.add_item(
			"Effect",
			std::string_view(),
			ITEM_ID_ADD_RELATIVE2,
			generic_axis_get_state<s32>,
			&m_axes[AXIS_EFFECT]);
	input_item_id const crossfade_id = device.add_item(
			"Crossfade",
			std::string_view(),
			ITEM_ID_XAXIS,
			generic_axis_get_state<s32>,
			&m_axes[AXIS_CROSSFADE]);

	// add hats
	for (unsigned i = 0; (SWITCH_DPAD_RIGHT - SWITCH_DPAD_UP) >= i; ++i)
	{
		if (has_button(SWITCH_BITS[SWITCH_DPAD_UP + i]))
		{
			switch_ids[SWITCH_DPAD_UP + i] = device.add_item(
					HAT_NAMES_GAMEPAD[i],
					std::string_view(),
					input_item_id(ITEM_ID_HAT1UP + i), // matches up/down/left/right order
					generic_button_get_state<u8>,
					&m_switches[SWITCH_DPAD_UP + i]);
		}
	}

	// add buttons
	input_item_id button_id = ITEM_ID_BUTTON1;
	unsigned button_count = 0;
	unsigned numbered_buttons[SWITCH_RSB - SWITCH_A + 1];
	for (unsigned i = 0; (SWITCH_RSB - SWITCH_A) >= i; ++i)
	{
		if (has_button(SWITCH_BITS[i]))
		{
			switch_ids[SWITCH_A + i] = device.add_item(
					BUTTON_NAMES_KEYBOARD[i],
					std::string_view(),
					button_id++,
					generic_button_get_state<u8>,
					&m_switches[SWITCH_A + i]);
			numbered_buttons[button_count] = SWITCH_A + i;

			// use these for automatically numbered buttons and pedals
			input_seq const seq(make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, switch_ids[SWITCH_A + i]));
			assignments.emplace_back(ioport_type(IPT_BUTTON1 + button_count), SEQ_TYPE_STANDARD, seq);
			if (3 > button_count)
				assignments.emplace_back(ioport_type(IPT_PEDAL + button_count), SEQ_TYPE_INCREMENT, seq);
			++button_count;
		}
	}

	// turntable buttons activate these as well as A/B/X
	switch_ids[SWITCH_GREEN] = device.add_item(
			"Green",
			std::string_view(),
			button_id++,
			generic_button_get_state<u8>,
			&m_switches[SWITCH_GREEN]);
	switch_ids[SWITCH_RED] = device.add_item(
			"Red",
			std::string_view(),
			button_id++,
			generic_button_get_state<u8>,
			&m_switches[SWITCH_RED]);
	switch_ids[SWITCH_BLUE] = device.add_item(
			"Blue",
			std::string_view(),
			button_id++,
			generic_button_get_state<u8>,
			&m_switches[SWITCH_BLUE]);

	// add start/back
	if (has_button(XINPUT_GAMEPAD_START))
	{
		switch_ids[SWITCH_START] = device.add_item(
				"Start",
				std::string_view(),
				ITEM_ID_START,
				generic_button_get_state<u8>,
				&m_switches[SWITCH_START]);
		add_button_assignment(assignments, IPT_START, { switch_ids[SWITCH_START] });
	}
	if (has_button(XINPUT_GAMEPAD_BACK))
	{
		switch_ids[SWITCH_BACK] = device.add_item(
				"Back",
				std::string_view(),
				ITEM_ID_SELECT,
				generic_button_get_state<u8>,
				&m_switches[SWITCH_BACK]);
		add_button_assignment(assignments, IPT_SELECT, { switch_ids[SWITCH_BACK] });
	}

	// use D-pad and A/B/X/Y diamond for twin sticks
	add_twin_stick_assignments(
			assignments,
			ITEM_ID_INVALID,
			ITEM_ID_INVALID,
			ITEM_ID_INVALID,
			ITEM_ID_INVALID,
			switch_ids[SWITCH_DPAD_LEFT],
			switch_ids[SWITCH_DPAD_RIGHT],
			switch_ids[SWITCH_DPAD_UP],
			switch_ids[SWITCH_DPAD_DOWN],
			switch_ids[SWITCH_X],
			switch_ids[SWITCH_B],
			switch_ids[SWITCH_Y],
			switch_ids[SWITCH_A]);

	// for most analog player controls, use turntable for X and effect for Y
	input_seq const turntable_seq(make_code(ITEM_CLASS_RELATIVE, ITEM_MODIFIER_NONE, turntable_id));
	input_seq const effect_seq(make_code(ITEM_CLASS_RELATIVE, ITEM_MODIFIER_NONE, effect_id));
	input_seq const crossfade_seq(make_code(ITEM_CLASS_ABSOLUTE, ITEM_MODIFIER_NONE, crossfade_id));
	input_seq joystick_left_seq(make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_NEG, turntable_id));
	input_seq joystick_right_seq(make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_POS, turntable_id));
	input_seq joystick_up_seq(make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_NEG, effect_id));
	input_seq joystick_down_seq(make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_POS, effect_id));
	assignments.emplace_back(IPT_PADDLE,       SEQ_TYPE_STANDARD, turntable_seq);
	assignments.emplace_back(IPT_PADDLE_V,     SEQ_TYPE_STANDARD, effect_seq);
	assignments.emplace_back(IPT_POSITIONAL,   SEQ_TYPE_STANDARD, crossfade_seq);
	assignments.emplace_back(IPT_POSITIONAL_V, SEQ_TYPE_STANDARD, effect_seq);
	assignments.emplace_back(IPT_DIAL,         SEQ_TYPE_STANDARD, turntable_seq);
	assignments.emplace_back(IPT_DIAL_V,       SEQ_TYPE_STANDARD, effect_seq);
	assignments.emplace_back(IPT_TRACKBALL_X,  SEQ_TYPE_STANDARD, turntable_seq);
	assignments.emplace_back(IPT_TRACKBALL_Y,  SEQ_TYPE_STANDARD, effect_seq);
	assignments.emplace_back(IPT_AD_STICK_X,   SEQ_TYPE_STANDARD, turntable_seq);
	assignments.emplace_back(IPT_AD_STICK_Y,   SEQ_TYPE_STANDARD, effect_seq);
	assignments.emplace_back(IPT_AD_STICK_Z,   SEQ_TYPE_STANDARD, crossfade_seq);
	assignments.emplace_back(IPT_LIGHTGUN_X,   SEQ_TYPE_STANDARD, turntable_seq);
	assignments.emplace_back(IPT_LIGHTGUN_Y,   SEQ_TYPE_STANDARD, effect_seq);
	assignments.emplace_back(IPT_MOUSE_X,      SEQ_TYPE_STANDARD, turntable_seq);
	assignments.emplace_back(IPT_MOUSE_Y,      SEQ_TYPE_STANDARD, effect_seq);

	// use D-pad for analog controls as well if present
	bool const have_dpad_left = ITEM_ID_INVALID != switch_ids[SWITCH_DPAD_LEFT];
	bool const have_dpad_right = ITEM_ID_INVALID != switch_ids[SWITCH_DPAD_RIGHT];
	bool const have_dpad_up = ITEM_ID_INVALID != switch_ids[SWITCH_DPAD_UP];
	bool const have_dpad_down = ITEM_ID_INVALID != switch_ids[SWITCH_DPAD_DOWN];
	if (have_dpad_left)
	{
		input_code const code(make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, switch_ids[SWITCH_DPAD_LEFT]));
		input_seq const left_seq(code);
		joystick_left_seq += input_seq::or_code;
		joystick_left_seq += code;
		assignments.emplace_back(IPT_PADDLE,      SEQ_TYPE_DECREMENT, left_seq);
		assignments.emplace_back(IPT_POSITIONAL,  SEQ_TYPE_DECREMENT, left_seq);
		assignments.emplace_back(IPT_DIAL,        SEQ_TYPE_DECREMENT, left_seq);
		assignments.emplace_back(IPT_TRACKBALL_X, SEQ_TYPE_DECREMENT, left_seq);
		assignments.emplace_back(IPT_AD_STICK_X,  SEQ_TYPE_DECREMENT, left_seq);
		assignments.emplace_back(IPT_LIGHTGUN_X,  SEQ_TYPE_DECREMENT, left_seq);
	}
	if (have_dpad_right)
	{
		input_code const code(make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, switch_ids[SWITCH_DPAD_RIGHT]));
		input_seq const right_seq(code);
		joystick_right_seq += input_seq::or_code;
		joystick_right_seq += code;
		assignments.emplace_back(IPT_PADDLE,      SEQ_TYPE_INCREMENT, right_seq);
		assignments.emplace_back(IPT_POSITIONAL,  SEQ_TYPE_INCREMENT, right_seq);
		assignments.emplace_back(IPT_DIAL,        SEQ_TYPE_INCREMENT, right_seq);
		assignments.emplace_back(IPT_TRACKBALL_X, SEQ_TYPE_INCREMENT, right_seq);
		assignments.emplace_back(IPT_AD_STICK_X,  SEQ_TYPE_INCREMENT, right_seq);
		assignments.emplace_back(IPT_LIGHTGUN_X,  SEQ_TYPE_INCREMENT, right_seq);
	}
	if (have_dpad_up)
	{
		input_code const code(make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, switch_ids[SWITCH_DPAD_UP]));
		input_seq const up_seq(code);
		joystick_up_seq += input_seq::or_code;
		joystick_up_seq += code;
		assignments.emplace_back(IPT_PADDLE_V,     SEQ_TYPE_DECREMENT, up_seq);
		assignments.emplace_back(IPT_POSITIONAL_V, SEQ_TYPE_DECREMENT, up_seq);
		assignments.emplace_back(IPT_DIAL_V,       SEQ_TYPE_DECREMENT, up_seq);
		assignments.emplace_back(IPT_TRACKBALL_Y,  SEQ_TYPE_DECREMENT, up_seq);
		assignments.emplace_back(IPT_AD_STICK_Y,   SEQ_TYPE_DECREMENT, up_seq);
		assignments.emplace_back(IPT_LIGHTGUN_Y,   SEQ_TYPE_DECREMENT, up_seq);
	}
	if (have_dpad_down)
	{
		input_code const code(make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, switch_ids[SWITCH_DPAD_DOWN]));
		input_seq const down_seq(code);
		joystick_down_seq += input_seq::or_code;
		joystick_down_seq += code;
		assignments.emplace_back(IPT_PADDLE_V,     SEQ_TYPE_INCREMENT, down_seq);
		assignments.emplace_back(IPT_POSITIONAL_V, SEQ_TYPE_INCREMENT, down_seq);
		assignments.emplace_back(IPT_DIAL_V,       SEQ_TYPE_INCREMENT, down_seq);
		assignments.emplace_back(IPT_TRACKBALL_Y,  SEQ_TYPE_INCREMENT, down_seq);
		assignments.emplace_back(IPT_AD_STICK_Y,   SEQ_TYPE_INCREMENT, down_seq);
		assignments.emplace_back(IPT_LIGHTGUN_Y,   SEQ_TYPE_INCREMENT, down_seq);
	}
	assignments.emplace_back(IPT_JOYSTICK_LEFT,  SEQ_TYPE_STANDARD, joystick_left_seq);
	assignments.emplace_back(IPT_JOYSTICK_RIGHT, SEQ_TYPE_STANDARD, joystick_right_seq);
	assignments.emplace_back(IPT_JOYSTICK_UP,    SEQ_TYPE_STANDARD, joystick_up_seq);
	assignments.emplace_back(IPT_JOYSTICK_DOWN,  SEQ_TYPE_STANDARD, joystick_down_seq);

	// choose navigation controls
	input_seq ui_up_seq(make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_NEG, turntable_id));
	input_seq ui_down_seq(make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_POS, turntable_id));
	input_seq ui_left_seq(make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_NEG, effect_id));
	input_seq ui_right_seq(make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_POS, effect_id));
	if ((have_dpad_up && have_dpad_down) || !have_dpad_left || !have_dpad_right)
	{
		if (have_dpad_up)
		{
			ui_up_seq += input_seq::or_code;
			ui_up_seq += make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, switch_ids[SWITCH_DPAD_UP]);
		}
		if (have_dpad_down)
		{
			ui_down_seq += input_seq::or_code;
			ui_down_seq += make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, switch_ids[SWITCH_DPAD_DOWN]);
		}
		if (have_dpad_left)
		{
			ui_left_seq += input_seq::or_code;
			ui_left_seq += make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, switch_ids[SWITCH_DPAD_LEFT]);
		}
		if (have_dpad_right)
		{
			ui_right_seq += input_seq::or_code;
			ui_right_seq += make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, switch_ids[SWITCH_DPAD_RIGHT]);
		}
	}
	else
	{
		if (have_dpad_left)
		{
			ui_up_seq += input_seq::or_code;
			ui_up_seq += make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, switch_ids[SWITCH_DPAD_LEFT]);
		}
		if (have_dpad_right)
		{
			ui_down_seq += input_seq::or_code;
			ui_down_seq += make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, switch_ids[SWITCH_DPAD_RIGHT]);
		}
		if (have_dpad_up)
		{
			ui_left_seq += input_seq::or_code;
			ui_left_seq += make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, switch_ids[SWITCH_DPAD_UP]);
		}
		if (have_dpad_down)
		{
			ui_right_seq += input_seq::or_code;
			ui_right_seq += make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, switch_ids[SWITCH_DPAD_DOWN]);
		}
	}
	assignments.emplace_back(IPT_UI_UP,    SEQ_TYPE_STANDARD, ui_up_seq);
	assignments.emplace_back(IPT_UI_DOWN,  SEQ_TYPE_STANDARD, ui_down_seq);
	assignments.emplace_back(IPT_UI_LEFT,  SEQ_TYPE_STANDARD, ui_left_seq);
	assignments.emplace_back(IPT_UI_RIGHT, SEQ_TYPE_STANDARD, ui_right_seq);

	// assign UI select/back/clear/help
	assign_ui_actions(
			assignments,
			SWITCH_B,
			SWITCH_X,
			SWITCH_Y,
			SWITCH_START,
			SWITCH_BACK,
			switch_ids,
			numbered_buttons,
			std::min<unsigned>(button_count, 4));

	// set default assignments
	device.set_default_assignments(std::move(assignments));
}



//============================================================
//  XInput keyboard handler
//============================================================

class xinput_keyboard_device : public xinput_device_base
{
public:
	xinput_keyboard_device(
			std::string &&name,
			std::string &&id,
			input_module &module,
			u32 player,
			XINPUT_CAPABILITIES const &caps,
			xinput_api_helper const &helper);

	virtual void poll(bool relative_reset) override;
	virtual void reset() override;
	virtual void configure(input_device &device) override;

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

		SWITCH_C1,
		SWITCH_C3 = SWITCH_C1 + 24,

		SWITCH_TOTAL
	};

	enum
	{
		AXIS_VELOCITY,      // field in LSX
		AXIS_PEDAL,         // RSY, most positive value neutral

		AXIS_TOTAL
	};

	u8  m_switches[SWITCH_TOTAL];
	s32 m_axes[AXIS_TOTAL];
};


xinput_keyboard_device::xinput_keyboard_device(
		std::string &&name,
		std::string &&id,
		input_module &module,
		u32 player,
		XINPUT_CAPABILITIES const &caps,
		xinput_api_helper const &helper) :
	xinput_device_base(std::move(name), std::move(id), module, player, caps, helper)
{
	std::fill(std::begin(m_switches), std::end(m_switches), 0);
	std::fill(std::begin(m_axes), std::end(m_axes), 0);
}


void xinput_keyboard_device::poll(bool relative_reset)
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

	// translate axes
	m_axes[AXIS_VELOCITY] = -normalize_absolute_axis(BIT(thumb_left_x(), 8, 7), -127, 127);
	m_axes[AXIS_PEDAL] = -normalize_absolute_axis(32'767 - thumb_right_y(), -32'767, 32'767);
}


void xinput_keyboard_device::reset()
{
	set_reset();
	std::fill(std::begin(m_switches), std::end(m_switches), 0);
	std::fill(std::begin(m_axes), std::end(m_axes), 0);
}


void xinput_keyboard_device::configure(input_device &device)
{
	// track item IDs for setting up default assignments
	input_device::assignment_vector assignments;
	input_item_id switch_ids[SWITCH_TOTAL];
	std::fill(std::begin(switch_ids), std::end(switch_ids), ITEM_ID_INVALID);

	// add axes
	device.add_item(
			"Velocity",
			std::string_view(),
			ITEM_ID_SLIDER1,
			generic_axis_get_state<s32>,
			&m_axes[AXIS_VELOCITY]);
	input_item_id const pedal_id = device.add_item(
			"Pedal",
			std::string_view(),
			ITEM_ID_SLIDER2,
			generic_axis_get_state<s32>,
			&m_axes[AXIS_PEDAL]);
	assignments.emplace_back(
			IPT_PEDAL,
			SEQ_TYPE_STANDARD,
			input_seq(make_code(ITEM_CLASS_ABSOLUTE, ITEM_MODIFIER_NEG, pedal_id)));

	// add hats
	for (unsigned i = 0; (SWITCH_DPAD_RIGHT - SWITCH_DPAD_UP) >= i; ++i)
	{
		if (has_button(SWITCH_BITS[SWITCH_DPAD_UP + i]))
		{
			switch_ids[SWITCH_DPAD_UP + i] = device.add_item(
					HAT_NAMES_GAMEPAD[i],
					std::string_view(),
					input_item_id(ITEM_ID_HAT1UP + i), // matches up/down/left/right order
					generic_button_get_state<u8>,
					&m_switches[SWITCH_DPAD_UP + i]);
		}
	}

	// add buttons
	input_item_id button_id = ITEM_ID_BUTTON1;
	unsigned button_count = 0;
	unsigned numbered_buttons[SWITCH_RSB - SWITCH_A + 1];
	for (unsigned i = 0; (SWITCH_RSB - SWITCH_A) >= i; ++i)
	{
		if (has_button(SWITCH_BITS[i]))
		{
			switch_ids[SWITCH_A + i] = device.add_item(
					BUTTON_NAMES_KEYBOARD[i],
					std::string_view(),
					button_id++,
					generic_button_get_state<u8>,
					&m_switches[SWITCH_A + i]);
			numbered_buttons[button_count] = SWITCH_A + i;

			// use these for automatically numbered buttons and pedals
			input_seq const seq(make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, switch_ids[SWITCH_A + i]));
			assignments.emplace_back(ioport_type(IPT_BUTTON1 + button_count), SEQ_TYPE_STANDARD, seq);
			if (3 > button_count)
				assignments.emplace_back(ioport_type(IPT_PEDAL + button_count), SEQ_TYPE_INCREMENT, seq);
			++button_count;
		}
	}

	// add keys
	char const *const key_formats[]{
			"C %d", "C# %d", "D %d", "D# %d", "E %d", "F %d", "F# %d", "G %d", "G# %d", "A %d", "A# %d", "B %d" };
	std::pair<ioport_type, ioport_type> const key_ids[]{
			{ IPT_MAHJONG_A,           IPT_HANAFUDA_A   },   // C
			{ IPT_MAHJONG_SCORE,       IPT_INVALID      },   // C#
			{ IPT_MAHJONG_B,           IPT_HANAFUDA_B   },   // D
			{ IPT_MAHJONG_DOUBLE_UP,   IPT_INVALID      },   // D#
			{ IPT_MAHJONG_C,           IPT_HANAFUDA_C   },   // E
			{ IPT_MAHJONG_D,           IPT_HANAFUDA_D   },   // F
			{ IPT_MAHJONG_BIG,         IPT_INVALID      },   // F#
			{ IPT_MAHJONG_E,           IPT_HANAFUDA_E   },   // G
			{ IPT_MAHJONG_SMALL,       IPT_INVALID      },   // G#
			{ IPT_MAHJONG_F,           IPT_HANAFUDA_F   },   // A
			{ IPT_MAHJONG_LAST_CHANCE, IPT_INVALID      },   // A#
			{ IPT_MAHJONG_G,           IPT_HANAFUDA_G   },   // B
			{ IPT_MAHJONG_H,           IPT_HANAFUDA_H   },   // C
			{ IPT_MAHJONG_KAN,         IPT_INVALID      },   // C#
			{ IPT_MAHJONG_I,           IPT_INVALID      },   // D
			{ IPT_MAHJONG_PON,         IPT_INVALID      },   // D#
			{ IPT_MAHJONG_J,           IPT_INVALID      },   // E
			{ IPT_MAHJONG_K,           IPT_INVALID      },   // F
			{ IPT_MAHJONG_CHI,         IPT_INVALID      },   // F#
			{ IPT_MAHJONG_L,           IPT_INVALID      },   // G
			{ IPT_MAHJONG_REACH,       IPT_INVALID      },   // G#
			{ IPT_MAHJONG_M,           IPT_HANAFUDA_YES },   // A
			{ IPT_MAHJONG_RON,         IPT_INVALID      },   // A#
			{ IPT_MAHJONG_N,           IPT_HANAFUDA_NO  },   // B
			{ IPT_MAHJONG_O,           IPT_INVALID      } }; // C
	for (unsigned i = 0; (SWITCH_C3 - SWITCH_C1) >= i; ++i)
	{
		switch_ids[SWITCH_C1 + i] = device.add_item(
				util::string_format(key_formats[i % 12], (i / 12) + 1),
				std::string_view(),
				(ITEM_ID_BUTTON32 >= button_id) ? button_id++ : ITEM_ID_OTHER_SWITCH,
				generic_button_get_state<u8>,
				&m_switches[SWITCH_C1 + i]);

		// add mahjong/hanafuda control assignments
		input_seq const seq(make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, switch_ids[SWITCH_C1 + i]));
		if (IPT_INVALID != key_ids[i].first)
			assignments.emplace_back(key_ids[i].first, SEQ_TYPE_STANDARD, seq);
		if (IPT_INVALID != key_ids[i].second)
			assignments.emplace_back(key_ids[i].second, SEQ_TYPE_STANDARD, seq);
	}

	// add start/back
	if (has_button(XINPUT_GAMEPAD_START))
	{
		switch_ids[SWITCH_START] = device.add_item(
				"Start",
				std::string_view(),
				ITEM_ID_START,
				generic_button_get_state<u8>,
				&m_switches[SWITCH_START]);
		add_button_assignment(assignments, IPT_START, { switch_ids[SWITCH_START] });
	}
	if (has_button(XINPUT_GAMEPAD_BACK))
	{
		switch_ids[SWITCH_BACK] = device.add_item(
				"Back",
				std::string_view(),
				ITEM_ID_SELECT,
				generic_button_get_state<u8>,
				&m_switches[SWITCH_BACK]);
		add_button_assignment(assignments, IPT_SELECT, { switch_ids[SWITCH_BACK] });
	}

	// use the D-pad for directional controls
	add_directional_assignments(
			assignments,
			ITEM_ID_INVALID,
			ITEM_ID_INVALID,
			switch_ids[SWITCH_DPAD_LEFT],
			switch_ids[SWITCH_DPAD_RIGHT],
			switch_ids[SWITCH_DPAD_UP],
			switch_ids[SWITCH_DPAD_DOWN]);

	// use the D-pad and A/B/X/Y diamond for twin sticks
	add_twin_stick_assignments(
			assignments,
			ITEM_ID_INVALID,
			ITEM_ID_INVALID,
			ITEM_ID_INVALID,
			ITEM_ID_INVALID,
			switch_ids[SWITCH_DPAD_LEFT],
			switch_ids[SWITCH_DPAD_RIGHT],
			switch_ids[SWITCH_DPAD_UP],
			switch_ids[SWITCH_DPAD_DOWN],
			switch_ids[SWITCH_X],
			switch_ids[SWITCH_B],
			switch_ids[SWITCH_Y],
			switch_ids[SWITCH_A]);

	// assign UI select/back/clear/help
	assign_ui_actions(
			assignments,
			SWITCH_B,
			SWITCH_X,
			SWITCH_Y,
			SWITCH_START,
			SWITCH_BACK,
			switch_ids,
			numbered_buttons,
			button_count);

	// set default assignments
	device.set_default_assignments(std::move(assignments));
}



//============================================================
//  XInput joystick module
//============================================================

class xinput_joystick_module : public input_module_impl<device_info, osd_common_t>
{
public:
	xinput_joystick_module() : input_module_impl<device_info, osd_common_t>(OSD_JOYSTICKINPUT_PROVIDER, "xinput")
	{
	}

	virtual int init(osd_interface &osd, const osd_options &options) override
	{
		int status;

		// Call the base
		status = input_module_impl<device_info, osd_common_t>::init(osd, options);
		if (status != 0)
			return status;

		// Create and initialize our helper
		m_xinput_helper = std::make_unique<xinput_api_helper>();
		status = m_xinput_helper->initialize();
		if (status != 0)
		{
			osd_printf_error("xinput_joystick_module failed to get XInput interface! Error: %u\n", static_cast<unsigned int>(status));
			return -1;
		}

		return 0;
	}

	virtual void input_init(running_machine &machine) override
	{
		input_module_impl<device_info, osd_common_t>::input_init(machine);

		// Loop through each gamepad to determine if they are connected
		for (UINT i = 0; i < XUSER_MAX_COUNT; i++)
		{
			// allocate and link in a new device
			auto devinfo = m_xinput_helper->create_xinput_device(i, *this);
			if (devinfo)
				add_device(DEVICE_CLASS_JOYSTICK, std::move(devinfo));
		}
	}

	virtual void exit() override
	{
		input_module_impl<device_info, osd_common_t>::exit();

		m_xinput_helper.reset();
	}

private:
	std::unique_ptr<xinput_api_helper> m_xinput_helper;
};

} // anonymous namespace



int xinput_api_helper::initialize()
{
	m_xinput_dll = dynamic_module::open(XINPUT_LIBRARIES);

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

std::unique_ptr<device_info> xinput_api_helper::create_xinput_device(
		UINT index,
		input_module_base &module)
{
	// If we can't get the capabilities skip this device
	XINPUT_STATE state{ 0 };
	if (xinput_get_state(index, &state) != ERROR_SUCCESS)
		return nullptr;
	XINPUT_CAPABILITIES caps{ 0 };
	if (FAILED(xinput_get_capabilities(index, 0, &caps)))
		return nullptr;

	char device_name[16];
	snprintf(device_name, sizeof(device_name), "XInput Player %u", index + 1);

	// allocate specialised device objects
	switch (caps.Type)
	{
	case XINPUT_DEVTYPE_GAMEPAD:
		switch (caps.SubType)
		{
		case 0x04: // XINPUT_DEVSUBTYPE_FLIGHT_STICK: work around MinGW header issues
			return std::make_unique<xinput_flight_stick_device>(
					device_name,
					device_name,
					module,
					index,
					caps,
					*this);
		case XINPUT_DEVSUBTYPE_GUITAR:
		case XINPUT_DEVSUBTYPE_GUITAR_ALTERNATE:
		case XINPUT_DEVSUBTYPE_GUITAR_BASS:
			return std::make_unique<xinput_guitar_device>(
					device_name,
					device_name,
					module,
					index,
					caps,
					*this);
		case XINPUT_DEVSUBTYPE_DRUM_KIT:
			return std::make_unique<xinput_drumkit_device>(
					device_name,
					device_name,
					module,
					index,
					caps,
					*this);
		case 0x0f:
			return std::make_unique<xinput_keyboard_device>(
					device_name,
					device_name,
					module,
					index,
					caps,
					*this);
		case 0x17:
			return std::make_unique<xinput_turntable_device>(
					device_name,
					device_name,
					module,
					index,
					caps,
					*this);
		}
	}

	// create default general-purpose device
	return std::make_unique<xinput_joystick_device>(
			device_name,
			device_name,
			module,
			index,
			caps,
			*this);
}

} // namespace osd

#else // defined(OSD_WINDOWS) || defined(SDLMAME_WIN32)

#include "input_module.h"

namespace osd { namespace { MODULE_NOT_SUPPORTED(xinput_joystick_module, OSD_JOYSTICKINPUT_PROVIDER, "xinput") } }

#endif // defined(OSD_WINDOWS) || defined(SDLMAME_WIN32)

MODULE_DEFINITION(JOYSTICKINPUT_XINPUT, osd::xinput_joystick_module)
