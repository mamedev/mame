// license:BSD-3-Clause
// copyright-holders:Vas Crabb
#include "input_sdlcommon.h"

#include "input_common.h"

#include "interface/inputseq.h"

// emu
#include "inpttype.h"

#include <cstring>
#include <string_view>
#include <utility>


namespace osd {

sdl_joystick_device_common::sdl_joystick_device_common() :
	m_joystick({ { 0 } }),
	m_ball{ 0 }
{
}


void sdl_joystick_device_common::configure_common(
		input_device &device,
		int axiscount,
		int buttoncount,
		int hatcount,
		int ballcount)
{
	input_device::assignment_vector assignments;
	char tempname[32];

	// loop over all axes
	input_item_id axisactual[MAX_AXES];
	for (int axis = 0; (axis < MAX_AXES) && (axis < axiscount); axis++)
	{
		input_item_id itemid;

		if (axis < INPUT_MAX_AXIS)
			itemid = input_item_id(ITEM_ID_XAXIS + axis);
		else if (axis < (INPUT_MAX_AXIS + INPUT_MAX_ADD_ABSOLUTE))
			itemid = input_item_id(ITEM_ID_ADD_ABSOLUTE1 + axis - INPUT_MAX_AXIS);
		else
			itemid = ITEM_ID_OTHER_AXIS_ABSOLUTE;

		snprintf(tempname, sizeof(tempname), "A%d", axis + 1);
		axisactual[axis] = device.add_item(
				tempname,
				std::string_view(),
				itemid,
				generic_axis_get_state<std::int32_t>,
				&m_joystick.axes[axis]);
	}

	// loop over all buttons
	for (int button = 0; (button < MAX_BUTTONS) && (button < buttoncount); button++)
	{
		input_item_id itemid;

		m_joystick.buttons[button] = 0;

		if (button < INPUT_MAX_BUTTONS)
			itemid = input_item_id(ITEM_ID_BUTTON1 + button);
		else if (button < INPUT_MAX_BUTTONS + INPUT_MAX_ADD_SWITCH)
			itemid = input_item_id(ITEM_ID_ADD_SWITCH1 + button - INPUT_MAX_BUTTONS);
		else
			itemid = ITEM_ID_OTHER_SWITCH;

		input_item_id const actual = device.add_item(
				default_button_name(button),
				std::string_view(),
				itemid,
				generic_button_get_state<std::uint8_t>,
				&m_joystick.buttons[button]);

		// there are sixteen action button types
		if (button < 16)
		{
			input_seq const seq(make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, actual));
			assignments.emplace_back(ioport_type(IPT_BUTTON1 + button), SEQ_TYPE_STANDARD, seq);

			// assign the first few buttons to UI actions and pedals
			switch (button)
			{
			case 0:
				assignments.emplace_back(IPT_PEDAL, SEQ_TYPE_INCREMENT, seq);
				assignments.emplace_back(IPT_UI_SELECT, SEQ_TYPE_STANDARD, seq);
				break;
			case 1:
				assignments.emplace_back(IPT_PEDAL2, SEQ_TYPE_INCREMENT, seq);
				assignments.emplace_back((3 > buttoncount) ? IPT_UI_CLEAR : IPT_UI_BACK, SEQ_TYPE_STANDARD, seq);
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

	// loop over all hats
	input_item_id hatactual[MAX_HATS][4];
	for (int hat = 0; (hat < MAX_HATS) && (hat < hatcount); hat++)
	{
		input_item_id itemid;

		snprintf(tempname, sizeof(tempname), "Hat %d Up", hat + 1);
		itemid = input_item_id((hat < INPUT_MAX_HATS) ? ITEM_ID_HAT1UP + (4 * hat) : ITEM_ID_OTHER_SWITCH);
		hatactual[hat][0] = device.add_item(
				tempname,
				std::string_view(),
				itemid,
				generic_button_get_state<std::uint8_t>,
				&m_joystick.hatsU[hat]);

		snprintf(tempname, sizeof(tempname), "Hat %d Down", hat + 1);
		itemid = input_item_id((hat < INPUT_MAX_HATS) ? ITEM_ID_HAT1DOWN + (4 * hat) : ITEM_ID_OTHER_SWITCH);
		hatactual[hat][1] = device.add_item(
				tempname,
				std::string_view(),
				itemid,
				generic_button_get_state<std::uint8_t>,
				&m_joystick.hatsD[hat]);

		snprintf(tempname, sizeof(tempname), "Hat %d Left", hat + 1);
		itemid = input_item_id((hat < INPUT_MAX_HATS) ? ITEM_ID_HAT1LEFT + (4 * hat) : ITEM_ID_OTHER_SWITCH);
		hatactual[hat][2] = device.add_item(
				tempname,
				std::string_view(),
				itemid,
				generic_button_get_state<std::uint8_t>,
				&m_joystick.hatsL[hat]);

		snprintf(tempname, sizeof(tempname), "Hat %d Right", hat + 1);
		itemid = input_item_id((hat < INPUT_MAX_HATS) ? ITEM_ID_HAT1RIGHT + (4 * hat) : ITEM_ID_OTHER_SWITCH);
		hatactual[hat][3] = device.add_item(
				tempname,
				std::string_view(),
				itemid,
				generic_button_get_state<std::uint8_t>,
				&m_joystick.hatsR[hat]);
	}

	// loop over all (track)balls
	for (int ball = 0; (ball < (MAX_AXES / 2)) && (ball < ballcount); ball++)
	{
		int itemid;

		if (ball * 2 < INPUT_MAX_ADD_RELATIVE)
			itemid = ITEM_ID_ADD_RELATIVE1 + ball * 2;
		else
			itemid = ITEM_ID_OTHER_AXIS_RELATIVE;

		snprintf(tempname, sizeof(tempname), "R%d X", ball + 1);
		input_item_id const xactual = device.add_item(
				tempname,
				std::string_view(),
				input_item_id(itemid),
				generic_axis_get_state<std::int32_t>,
				&m_joystick.balls[ball * 2]);

		snprintf(tempname, sizeof(tempname), "R%d Y", ball + 1);
		input_item_id const yactual = device.add_item(
				tempname,
				std::string_view(),
				input_item_id(itemid + 1),
				generic_axis_get_state<std::int32_t>,
				&m_joystick.balls[ball * 2 + 1]);

		if (0 == ball)
		{
			// assign the first trackball to dial, trackball, mouse and lightgun inputs
			input_seq const xseq(make_code(ITEM_CLASS_RELATIVE, ITEM_MODIFIER_NONE, xactual));
			input_seq const yseq(make_code(ITEM_CLASS_RELATIVE, ITEM_MODIFIER_NONE, yactual));
			assignments.emplace_back(IPT_DIAL,        SEQ_TYPE_STANDARD, xseq);
			assignments.emplace_back(IPT_DIAL_V,      SEQ_TYPE_STANDARD, yseq);
			assignments.emplace_back(IPT_TRACKBALL_X, SEQ_TYPE_STANDARD, xseq);
			assignments.emplace_back(IPT_TRACKBALL_Y, SEQ_TYPE_STANDARD, yseq);
			assignments.emplace_back(IPT_LIGHTGUN_X,  SEQ_TYPE_STANDARD, xseq);
			assignments.emplace_back(IPT_LIGHTGUN_Y,  SEQ_TYPE_STANDARD, yseq);
			assignments.emplace_back(IPT_MOUSE_X,     SEQ_TYPE_STANDARD, xseq);
			assignments.emplace_back(IPT_MOUSE_Y,     SEQ_TYPE_STANDARD, yseq);
			if (2 > axiscount)
			{
				// use it for joystick inputs if axes are limited
				assignments.emplace_back(IPT_AD_STICK_X, SEQ_TYPE_STANDARD, xseq);
				assignments.emplace_back(IPT_AD_STICK_Y, SEQ_TYPE_STANDARD, yseq);
			}
			else
			{
				// use for non-centring throttle control
				assignments.emplace_back(IPT_AD_STICK_Z, SEQ_TYPE_STANDARD, yseq);
			}
		}
		else if ((1 == ball) && (2 > axiscount))
		{
			// provide a non-centring throttle control
			input_seq const yseq(make_code(ITEM_CLASS_RELATIVE, ITEM_MODIFIER_NONE, yactual));
			assignments.emplace_back(IPT_AD_STICK_Z, SEQ_TYPE_STANDARD, yseq);
		}
	}

	// set up default assignments for axes and hats
	add_directional_assignments(
			assignments,
			(1 <= axiscount) ? axisactual[0] : ITEM_ID_INVALID, // assume first axis is X
			(2 <= axiscount) ? axisactual[1] : ITEM_ID_INVALID, // assume second axis is Y
			(1 <= hatcount) ? hatactual[0][2] : ITEM_ID_INVALID,
			(1 <= hatcount) ? hatactual[0][3] : ITEM_ID_INVALID,
			(1 <= hatcount) ? hatactual[0][0] : ITEM_ID_INVALID,
			(1 <= hatcount) ? hatactual[0][1] : ITEM_ID_INVALID);
	if (2 <= axiscount)
	{
		// put pedals on the last of the second, third or fourth axis
		input_item_id const pedalitem = axisactual[std::min(axiscount, 4) - 1];
		assignments.emplace_back(
				IPT_PEDAL,
				SEQ_TYPE_STANDARD,
				input_seq(make_code(ITEM_CLASS_ABSOLUTE, ITEM_MODIFIER_NEG, pedalitem)));
		assignments.emplace_back(
				IPT_PEDAL2,
				SEQ_TYPE_STANDARD,
				input_seq(make_code(ITEM_CLASS_ABSOLUTE, ITEM_MODIFIER_POS, pedalitem)));
	}
	if (3 <= axiscount)
	{
		// assign X/Y to one of the twin sticks
		assignments.emplace_back(
				(4 <= axiscount) ? IPT_JOYSTICKLEFT_LEFT : IPT_JOYSTICKRIGHT_LEFT,
				SEQ_TYPE_STANDARD,
				input_seq(make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_LEFT, axisactual[0])));
		assignments.emplace_back(
				(4 <= axiscount) ? IPT_JOYSTICKLEFT_RIGHT : IPT_JOYSTICKRIGHT_RIGHT,
				SEQ_TYPE_STANDARD,
				input_seq(make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_RIGHT, axisactual[0])));
		assignments.emplace_back(
				(4 <= axiscount) ? IPT_JOYSTICKLEFT_UP : IPT_JOYSTICKRIGHT_UP,
				SEQ_TYPE_STANDARD,
				input_seq(make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_UP, axisactual[1])));
		assignments.emplace_back(
				(4 <= axiscount) ? IPT_JOYSTICKLEFT_DOWN : IPT_JOYSTICKRIGHT_DOWN,
				SEQ_TYPE_STANDARD,
				input_seq(make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_DOWN, axisactual[1])));

		// use third or fourth axis for Z
		input_seq const seq(make_code(ITEM_CLASS_ABSOLUTE, ITEM_MODIFIER_NONE, axisactual[std::min(axiscount, 4) - 1]));
		assignments.emplace_back(IPT_AD_STICK_Z, SEQ_TYPE_STANDARD, seq);

		// use this for focus next/previous to make system selection menu practical to navigate
		input_seq const upseq(make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_NEG, axisactual[2]));
		input_seq const downseq(make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_POS, axisactual[2]));
		assignments.emplace_back(IPT_UI_FOCUS_PREV, SEQ_TYPE_STANDARD, upseq);
		assignments.emplace_back(IPT_UI_FOCUS_NEXT, SEQ_TYPE_STANDARD, downseq);
		if (4 <= axiscount)
		{
			// use for zoom as well if there's another axis to use for previous/next group
			assignments.emplace_back(IPT_UI_ZOOM_IN, SEQ_TYPE_STANDARD, downseq);
			assignments.emplace_back(IPT_UI_ZOOM_OUT, SEQ_TYPE_STANDARD, upseq);
		}

		// use this for twin sticks, too
		assignments.emplace_back((4 <= axiscount) ? IPT_JOYSTICKRIGHT_LEFT : IPT_JOYSTICKLEFT_UP, SEQ_TYPE_STANDARD, upseq);
		assignments.emplace_back((4 <= axiscount) ? IPT_JOYSTICKRIGHT_RIGHT : IPT_JOYSTICKLEFT_DOWN, SEQ_TYPE_STANDARD, downseq);

		// put previous/next group on the last of the third or fourth axis
		input_item_id const groupitem = axisactual[std::min(axiscount, 4) - 1];
		assignments.emplace_back(
				IPT_UI_PREV_GROUP,
				SEQ_TYPE_STANDARD,
				input_seq(make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_NEG, groupitem)));
		assignments.emplace_back(
				IPT_UI_NEXT_GROUP,
				SEQ_TYPE_STANDARD,
				input_seq(make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_POS, groupitem)));
	}
	if (4 <= axiscount)
	{
		// use this for twin sticks
		input_seq const upseq(make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_NEG, axisactual[3]));
		input_seq const downseq(make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_POS, axisactual[3]));
		assignments.emplace_back(IPT_JOYSTICKRIGHT_UP, SEQ_TYPE_STANDARD, upseq);
		assignments.emplace_back(IPT_JOYSTICKRIGHT_DOWN, SEQ_TYPE_STANDARD, downseq);
	}

	// set default assignments
	device.set_default_assignments(std::move(assignments));
}


void sdl_joystick_device_common::clear_buffer() noexcept
{
	memset(&m_joystick, 0, sizeof(m_joystick));
	memset(&m_ball, 0, sizeof(m_ball));
}

} // anonymous namespace
