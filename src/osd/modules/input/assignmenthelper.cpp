// license:BSD-3-Clause
// copyright-holders:Vas Crabb
//============================================================
//
//  assignmenthelper.cpp - input assignment setup helper
//
//============================================================

#include "assignmenthelper.h"

#include "interface/inputseq.h"

#include "inpttype.h"


namespace osd {

bool joystick_assignment_helper::add_assignment(
		input_device::assignment_vector &assignments,
		ioport_type fieldtype,
		input_seq_type seqtype,
		input_item_class itemclass,
		input_item_modifier modifier,
		std::initializer_list<input_item_id> items)
{
	for (input_item_id item : items)
	{
		if (ITEM_ID_INVALID != item)
		{
			assignments.emplace_back(
					fieldtype,
					seqtype,
					input_seq(input_code(DEVICE_CLASS_JOYSTICK, 0, itemclass, modifier, item)));
			return true;
		}
	}
	return false;
}


bool joystick_assignment_helper::add_button_assignment(
		input_device::assignment_vector &assignments,
		ioport_type field_type,
		std::initializer_list<input_item_id> items)
{
	return add_assignment(
			assignments,
			field_type,
			SEQ_TYPE_STANDARD,
			ITEM_CLASS_SWITCH,
			ITEM_MODIFIER_NONE,
			items);
}


bool joystick_assignment_helper::add_button_pair_assignment(
		input_device::assignment_vector &assignments,
		ioport_type field1,
		ioport_type field2,
		input_item_id button1,
		input_item_id button2)
{
	if ((ITEM_ID_INVALID == button1) || (ITEM_ID_INVALID == button2))
		return false;

	assignments.emplace_back(
			field1,
			SEQ_TYPE_STANDARD,
			make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, button1));
	assignments.emplace_back(
			field2,
			SEQ_TYPE_STANDARD,
			make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, button2));
	return true;
}


bool joystick_assignment_helper::add_axis_inc_dec_assignment(
		input_device::assignment_vector &assignments,
		ioport_type field_type,
		input_item_id button_dec,
		input_item_id button_inc)
{
	if ((ITEM_ID_INVALID == button_dec) || (ITEM_ID_INVALID == button_inc))
		return false;

	assignments.emplace_back(
			field_type,
			SEQ_TYPE_DECREMENT,
			make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, button_dec));
	assignments.emplace_back(
			field_type,
			SEQ_TYPE_INCREMENT,
			make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, button_inc));
	return true;
}


bool joystick_assignment_helper::add_axis_pair_assignment(
		input_device::assignment_vector &assignments,
		ioport_type field1,
		ioport_type field2,
		input_item_id axis)
{
	if (ITEM_ID_INVALID == axis)
		return false;

	assignments.emplace_back(
			field1,
			SEQ_TYPE_STANDARD,
			make_code(
				ITEM_CLASS_SWITCH,
				(ITEM_ID_XAXIS == axis) ? ITEM_MODIFIER_LEFT : (ITEM_ID_YAXIS == axis) ? ITEM_MODIFIER_UP : ITEM_MODIFIER_NEG,
				axis));
	assignments.emplace_back(
			field2,
			SEQ_TYPE_STANDARD,
			make_code(
				ITEM_CLASS_SWITCH,
				(ITEM_ID_XAXIS == axis) ? ITEM_MODIFIER_RIGHT : (ITEM_ID_YAXIS == axis) ? ITEM_MODIFIER_DOWN : ITEM_MODIFIER_POS,
				axis));
	return true;
}


bool joystick_assignment_helper::consume_button_pair(
		input_device::assignment_vector &assignments,
		ioport_type field1,
		ioport_type field2,
		input_item_id &button1,
		input_item_id &button2)
{
	if (!add_button_pair_assignment(assignments, field1, field2, button1, button2))
		return false;

	button1 = ITEM_ID_INVALID;
	button2 = ITEM_ID_INVALID;
	return true;
}


bool joystick_assignment_helper::consume_trigger_pair(
		input_device::assignment_vector &assignments,
		ioport_type field1,
		ioport_type field2,
		input_item_id &axis1,
		input_item_id &axis2)
{
	if ((ITEM_ID_INVALID == axis1) || (ITEM_ID_INVALID == axis2))
		return false;

	assignments.emplace_back(
			field1,
			SEQ_TYPE_STANDARD,
			make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_NEG, axis1));
	assignments.emplace_back(
			field2,
			SEQ_TYPE_STANDARD,
			make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_NEG, axis2));
	axis1 = ITEM_ID_INVALID;
	axis2 = ITEM_ID_INVALID;
	return true;
}


bool joystick_assignment_helper::consume_axis_pair(
		input_device::assignment_vector &assignments,
		ioport_type field1,
		ioport_type field2,
		input_item_id &axis)
{
	if (!add_axis_pair_assignment(assignments, field1, field2, axis))
		return false;

	axis = ITEM_ID_INVALID;
	return true;
}


void joystick_assignment_helper::add_directional_assignments(
		input_device::assignment_vector &assignments,
		input_item_id xaxis,
		input_item_id yaxis,
		input_item_id leftswitch,
		input_item_id rightswitch,
		input_item_id upswitch,
		input_item_id downswitch)
{
	// see if we have complementary pairs of directional switches
	bool const hswitches = (ITEM_ID_INVALID != leftswitch) && (ITEM_ID_INVALID != rightswitch);
	bool const vswitches = (ITEM_ID_INVALID != upswitch) && (ITEM_ID_INVALID != downswitch);

	// use X axis if present
	if (ITEM_ID_INVALID != xaxis)
	{
		// use this for horizontal axis movement
		input_seq const xseq(make_code(ITEM_CLASS_ABSOLUTE, ITEM_MODIFIER_NONE, xaxis));
		assignments.emplace_back(IPT_PADDLE,       SEQ_TYPE_STANDARD, xseq);
		assignments.emplace_back(IPT_POSITIONAL,   SEQ_TYPE_STANDARD, xseq);
		assignments.emplace_back(IPT_DIAL,         SEQ_TYPE_STANDARD, xseq);
		assignments.emplace_back(IPT_TRACKBALL_X,  SEQ_TYPE_STANDARD, xseq);
		assignments.emplace_back(IPT_AD_STICK_X,   SEQ_TYPE_STANDARD, xseq);
		assignments.emplace_back(IPT_LIGHTGUN_X,   SEQ_TYPE_STANDARD, xseq);

		// use it for the main left/right control, too
		input_seq leftseq(make_code(ITEM_CLASS_SWITCH, (ITEM_ID_XAXIS == xaxis) ? ITEM_MODIFIER_LEFT : ITEM_MODIFIER_NEG, xaxis));
		input_seq rightseq(make_code(ITEM_CLASS_SWITCH, (ITEM_ID_XAXIS == xaxis) ? ITEM_MODIFIER_RIGHT : ITEM_MODIFIER_POS, xaxis));
		if (ITEM_ID_INVALID != leftswitch)
		{
			leftseq += input_seq::or_code;
			leftseq += make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, leftswitch);
		}
		if (ITEM_ID_INVALID != rightswitch)
		{
			rightseq += input_seq::or_code;
			rightseq += make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, rightswitch);
		}
		assignments.emplace_back(IPT_JOYSTICK_LEFT, SEQ_TYPE_STANDARD, leftseq);
		assignments.emplace_back(IPT_JOYSTICK_RIGHT, SEQ_TYPE_STANDARD, rightseq);

		// use for vertical navigation if there's no Y axis, or horizontal otherwise
		if (ITEM_ID_INVALID != yaxis)
		{
			// if left/right are both present but not both up/down, they'll be taken for vertical navigation
			if (hswitches && !vswitches)
			{
				leftseq.backspace();
				if (ITEM_ID_INVALID != upswitch)
					leftseq += make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, upswitch);
				else
					leftseq.backspace();

				rightseq.backspace();
				if (ITEM_ID_INVALID != downswitch)
					rightseq += make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, downswitch);
				else
					rightseq.backspace();
			}
			assignments.emplace_back(IPT_UI_LEFT, SEQ_TYPE_STANDARD, leftseq);
			assignments.emplace_back(IPT_UI_RIGHT, SEQ_TYPE_STANDARD, rightseq);
		}
		else
		{
			// prefer D-pad up/down for vertical navigation if present
			if (!hswitches || vswitches)
			{
				while (leftseq.length() > 1)
					leftseq.backspace();
				if (ITEM_ID_INVALID != upswitch)
				{
					leftseq += input_seq::or_code;
					leftseq += make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, upswitch);
				}

				while (rightseq.length() > 1)
					rightseq.backspace();
				if (ITEM_ID_INVALID != downswitch)
				{
					rightseq += input_seq::or_code;
					rightseq += make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, downswitch);
				}
			}
			assignments.emplace_back(IPT_UI_UP, SEQ_TYPE_STANDARD, leftseq);
			assignments.emplace_back(IPT_UI_DOWN, SEQ_TYPE_STANDARD, rightseq);
		}
	}
	else
	{
		// without a primary analog X axis, we still want D-pad left/right controls if possible
		add_button_assignment(assignments, IPT_JOYSTICK_LEFT, { leftswitch });
		add_button_assignment(assignments, IPT_JOYSTICK_RIGHT, { rightswitch });

		// vertical navigation gets first pick on directional controls
		add_button_assignment(assignments, IPT_UI_LEFT, { (hswitches && !vswitches) ? upswitch : leftswitch });
		add_button_assignment(assignments, IPT_UI_RIGHT, { (hswitches && !vswitches) ? downswitch : rightswitch });
	}

	// use Y axis if present
	if (ITEM_ID_INVALID != yaxis)
	{
		// use this for vertical axis movement
		input_seq const yseq(make_code(ITEM_CLASS_ABSOLUTE, ITEM_MODIFIER_NONE, yaxis));
		assignments.emplace_back(IPT_PADDLE_V,     SEQ_TYPE_STANDARD, yseq);
		assignments.emplace_back(IPT_POSITIONAL_V, SEQ_TYPE_STANDARD, yseq);
		assignments.emplace_back(IPT_DIAL_V,       SEQ_TYPE_STANDARD, yseq);
		assignments.emplace_back(IPT_TRACKBALL_Y,  SEQ_TYPE_STANDARD, yseq);
		assignments.emplace_back(IPT_AD_STICK_Y,   SEQ_TYPE_STANDARD, yseq);
		assignments.emplace_back(IPT_LIGHTGUN_Y,   SEQ_TYPE_STANDARD, yseq);

		// use it for the main up/down control, too
		input_seq upseq(make_code(ITEM_CLASS_SWITCH, (ITEM_ID_YAXIS == yaxis) ? ITEM_MODIFIER_UP : ITEM_MODIFIER_NEG, yaxis));
		input_seq downseq(make_code(ITEM_CLASS_SWITCH, (ITEM_ID_YAXIS == yaxis) ? ITEM_MODIFIER_DOWN : ITEM_MODIFIER_POS, yaxis));
		if (ITEM_ID_INVALID != upswitch)
		{
			upseq += input_seq::or_code;
			upseq += make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, upswitch);
		}
		if (ITEM_ID_INVALID != downswitch)
		{
			downseq += input_seq::or_code;
			downseq += make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, downswitch);
		}
		assignments.emplace_back(IPT_JOYSTICK_UP, SEQ_TYPE_STANDARD, upseq);
		assignments.emplace_back(IPT_JOYSTICK_DOWN, SEQ_TYPE_STANDARD, downseq);

		// if available, this is used for vertical navigation
		if (hswitches && !vswitches)
		{
			if (upseq.length() > 1)
				upseq.backspace();
			else
				upseq += input_seq::or_code;
			upseq += make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, leftswitch);

			if (downseq.length() > 1)
				downseq.backspace();
			else
				downseq += input_seq::or_code;
			downseq += make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, rightswitch);
		}
		assignments.emplace_back(IPT_UI_UP, SEQ_TYPE_STANDARD, upseq);
		assignments.emplace_back(IPT_UI_DOWN, SEQ_TYPE_STANDARD, downseq);
	}
	else
	{
		// without a primary analog Y axis, we still want D-pad up/down controls if possible
		add_button_assignment(assignments, IPT_JOYSTICK_UP, { upswitch });
		add_button_assignment(assignments, IPT_JOYSTICK_DOWN, { downswitch });

		// vertical navigation may be assigned to X axis if Y axis is not present
		bool const dpadflip = (ITEM_ID_INVALID != xaxis) == (!hswitches || vswitches);
		add_button_assignment(
				assignments,
				(ITEM_ID_INVALID != xaxis) ? IPT_UI_LEFT : IPT_UI_UP,
				{ dpadflip ? leftswitch : upswitch });
		add_button_assignment(
				assignments,
				(ITEM_ID_INVALID != xaxis) ? IPT_UI_RIGHT : IPT_UI_DOWN,
				{ dpadflip ? rightswitch : downswitch });
	}

	// if we're missing either primary axis, fall back to D-pad for analog increment/decrement
	if ((ITEM_ID_INVALID == xaxis) || (ITEM_ID_INVALID == yaxis))
	{
		if (ITEM_ID_INVALID != leftswitch)
		{
			input_seq const leftseq(make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, leftswitch));
			assignments.emplace_back(IPT_PADDLE,       SEQ_TYPE_DECREMENT, leftseq);
			assignments.emplace_back(IPT_POSITIONAL,   SEQ_TYPE_DECREMENT, leftseq);
			assignments.emplace_back(IPT_DIAL,         SEQ_TYPE_DECREMENT, leftseq);
			assignments.emplace_back(IPT_TRACKBALL_X,  SEQ_TYPE_DECREMENT, leftseq);
			assignments.emplace_back(IPT_AD_STICK_X,   SEQ_TYPE_DECREMENT, leftseq);
			assignments.emplace_back(IPT_LIGHTGUN_X,   SEQ_TYPE_DECREMENT, leftseq);
		}
		if (ITEM_ID_INVALID != rightswitch)
		{
			input_seq const rightseq(make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, rightswitch));
			assignments.emplace_back(IPT_PADDLE,       SEQ_TYPE_INCREMENT, rightseq);
			assignments.emplace_back(IPT_POSITIONAL,   SEQ_TYPE_INCREMENT, rightseq);
			assignments.emplace_back(IPT_DIAL,         SEQ_TYPE_INCREMENT, rightseq);
			assignments.emplace_back(IPT_TRACKBALL_X,  SEQ_TYPE_INCREMENT, rightseq);
			assignments.emplace_back(IPT_AD_STICK_X,   SEQ_TYPE_INCREMENT, rightseq);
			assignments.emplace_back(IPT_LIGHTGUN_X,   SEQ_TYPE_INCREMENT, rightseq);
		}
		if (ITEM_ID_INVALID != upswitch)
		{
			input_seq const upseq(make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, upswitch));
			assignments.emplace_back(IPT_PADDLE_V,     SEQ_TYPE_DECREMENT, upseq);
			assignments.emplace_back(IPT_POSITIONAL_V, SEQ_TYPE_DECREMENT, upseq);
			assignments.emplace_back(IPT_DIAL_V,       SEQ_TYPE_DECREMENT, upseq);
			assignments.emplace_back(IPT_TRACKBALL_Y,  SEQ_TYPE_DECREMENT, upseq);
			assignments.emplace_back(IPT_AD_STICK_Y,   SEQ_TYPE_DECREMENT, upseq);
			assignments.emplace_back(IPT_LIGHTGUN_Y,   SEQ_TYPE_DECREMENT, upseq);
		}
		if (ITEM_ID_INVALID != downswitch)
		{
			input_seq const downseq(make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, downswitch));
			assignments.emplace_back(IPT_PADDLE_V,     SEQ_TYPE_INCREMENT, downseq);
			assignments.emplace_back(IPT_POSITIONAL_V, SEQ_TYPE_INCREMENT, downseq);
			assignments.emplace_back(IPT_DIAL_V,       SEQ_TYPE_INCREMENT, downseq);
			assignments.emplace_back(IPT_TRACKBALL_Y,  SEQ_TYPE_INCREMENT, downseq);
			assignments.emplace_back(IPT_AD_STICK_Y,   SEQ_TYPE_INCREMENT, downseq);
			assignments.emplace_back(IPT_LIGHTGUN_Y,   SEQ_TYPE_INCREMENT, downseq);
		}
	}
}


void joystick_assignment_helper::add_twin_stick_assignments(
		input_device::assignment_vector &assignments,
		input_item_id leftx,
		input_item_id lefty,
		input_item_id rightx,
		input_item_id righty,
		input_item_id leftleft,
		input_item_id leftright,
		input_item_id leftup,
		input_item_id leftdown,
		input_item_id rightleft,
		input_item_id rightright,
		input_item_id rightup,
		input_item_id rightdown)
{
	// we'll add these at the end if they aren't empty
	input_seq leftleftseq, leftrightseq, leftupseq, leftdownseq;
	input_seq rightleftseq, rightrightseq, rightupseq, rightdownseq;

	// only use axes if there are at least two axes in the same orientation
	bool const useaxes =
			((ITEM_ID_INVALID != leftx) && (ITEM_ID_INVALID != rightx)) ||
			((ITEM_ID_INVALID != lefty) && (ITEM_ID_INVALID != righty));
	if (useaxes)
	{
		// left stick
		if (ITEM_ID_INVALID != leftx)
		{
			leftleftseq += make_code(
					ITEM_CLASS_SWITCH,
					(ITEM_ID_XAXIS == leftx) ? ITEM_MODIFIER_LEFT : ITEM_MODIFIER_NEG,
					leftx);
			leftrightseq += make_code(
					ITEM_CLASS_SWITCH,
					(ITEM_ID_XAXIS == leftx) ? ITEM_MODIFIER_RIGHT : ITEM_MODIFIER_POS,
					leftx);
		}
		if (ITEM_ID_INVALID != lefty)
		{
			leftupseq += make_code(
					ITEM_CLASS_SWITCH,
					(ITEM_ID_YAXIS == lefty) ? ITEM_MODIFIER_UP : ITEM_MODIFIER_NEG,
					lefty);
			leftdownseq += make_code(
					ITEM_CLASS_SWITCH,
					(ITEM_ID_YAXIS == lefty) ? ITEM_MODIFIER_DOWN : ITEM_MODIFIER_POS,
					lefty);
		}

		// right stick
		if (ITEM_ID_INVALID != rightx)
		{
			rightleftseq += make_code(
					ITEM_CLASS_SWITCH,
					(ITEM_ID_XAXIS == rightx) ? ITEM_MODIFIER_LEFT : ITEM_MODIFIER_NEG,
					rightx);
			rightrightseq += make_code(
					ITEM_CLASS_SWITCH,
					(ITEM_ID_XAXIS == rightx) ? ITEM_MODIFIER_RIGHT : ITEM_MODIFIER_POS,
					rightx);
		}
		if (ITEM_ID_INVALID != righty)
		{
			rightupseq += make_code(
					ITEM_CLASS_SWITCH,
					(ITEM_ID_YAXIS == righty) ? ITEM_MODIFIER_UP : ITEM_MODIFIER_NEG,
					righty);
			rightdownseq += make_code(
					ITEM_CLASS_SWITCH,
					(ITEM_ID_YAXIS == righty) ? ITEM_MODIFIER_DOWN : ITEM_MODIFIER_POS,
					righty);
		}
	}

	// only use switches if we have at least one pair of matching opposing directions
	bool const lefth = (ITEM_ID_INVALID != leftleft) && (ITEM_ID_INVALID != leftright);
	bool const leftv = (ITEM_ID_INVALID != leftup) && (ITEM_ID_INVALID != leftdown);
	bool const righth = (ITEM_ID_INVALID != rightleft) && (ITEM_ID_INVALID != rightright);
	bool const rightv = (ITEM_ID_INVALID != rightup) && (ITEM_ID_INVALID != rightdown);
	if ((lefth && righth) || (leftv && rightv))
	{
		// left stick
		if (ITEM_ID_INVALID != leftleft)
		{
			if (!leftleftseq.empty())
				leftleftseq += input_seq::or_code;
			leftleftseq += make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, leftleft);
		}
		if (ITEM_ID_INVALID != leftright)
		{
			if (!leftrightseq.empty())
				leftrightseq += input_seq::or_code;
			leftrightseq += make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, leftright);
		}
		if (ITEM_ID_INVALID != leftup)
		{
			if (!leftupseq.empty())
				leftupseq += input_seq::or_code;
			leftupseq += make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, leftup);
		}
		if (ITEM_ID_INVALID != leftdown)
		{
			if (!leftdownseq.empty())
				leftdownseq += input_seq::or_code;
			leftdownseq += make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, leftdown);
		}

		// right stick
		if (ITEM_ID_INVALID != rightleft)
		{
			if (!rightleftseq.empty())
				rightleftseq += input_seq::or_code;
			rightleftseq += make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, rightleft);
		}
		if (ITEM_ID_INVALID != rightright)
		{
			if (!rightrightseq.empty())
				rightrightseq += input_seq::or_code;
			rightrightseq += make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, rightright);
		}
		if (ITEM_ID_INVALID != rightup)
		{
			if (!rightupseq.empty())
				rightupseq += input_seq::or_code;
			rightupseq += make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, rightup);
		}
		if (ITEM_ID_INVALID != rightdown)
		{
			if (!rightdownseq.empty())
				rightdownseq += input_seq::or_code;
			rightdownseq += make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, rightdown);
		}
	}

	// now add collected assignments
	if (!leftleftseq.empty())
		assignments.emplace_back(IPT_JOYSTICKLEFT_LEFT, SEQ_TYPE_STANDARD, leftleftseq);
	if (!leftrightseq.empty())
		assignments.emplace_back(IPT_JOYSTICKLEFT_RIGHT, SEQ_TYPE_STANDARD, leftrightseq);
	if (!leftupseq.empty())
		assignments.emplace_back(IPT_JOYSTICKLEFT_UP, SEQ_TYPE_STANDARD, leftupseq);
	if (!leftdownseq.empty())
		assignments.emplace_back(IPT_JOYSTICKLEFT_DOWN, SEQ_TYPE_STANDARD, leftdownseq);
	if (!rightleftseq.empty())
		assignments.emplace_back(IPT_JOYSTICKRIGHT_LEFT, SEQ_TYPE_STANDARD, rightleftseq);
	if (!rightrightseq.empty())
		assignments.emplace_back(IPT_JOYSTICKRIGHT_RIGHT, SEQ_TYPE_STANDARD, rightrightseq);
	if (!rightupseq.empty())
		assignments.emplace_back(IPT_JOYSTICKRIGHT_UP, SEQ_TYPE_STANDARD, rightupseq);
	if (!rightdownseq.empty())
		assignments.emplace_back(IPT_JOYSTICKRIGHT_DOWN, SEQ_TYPE_STANDARD, rightdownseq);
}


void joystick_assignment_helper::choose_primary_stick(
		input_item_id (&stickaxes)[2][2],
		input_item_id leftx,
		input_item_id lefty,
		input_item_id rightx,
		input_item_id righty)
{
	if ((ITEM_ID_INVALID != leftx) && (ITEM_ID_INVALID != lefty))
	{
		// left stick has both axes, make it primary
		stickaxes[0][0] = leftx;
		stickaxes[0][1] = lefty;
		stickaxes[1][0] = rightx;
		stickaxes[1][1] = righty;
	}
	else if ((ITEM_ID_INVALID != rightx) && (ITEM_ID_INVALID != righty))
	{
		// right stick has both axes, make it primary
		stickaxes[0][0] = rightx;
		stickaxes[0][1] = righty;
		stickaxes[1][0] = leftx;
		stickaxes[1][1] = lefty;
	}
	else if (ITEM_ID_INVALID != leftx)
	{
		// degenerate case - left X and possibly right X or Y
		stickaxes[0][0] = leftx;
		stickaxes[0][1] = righty;
		stickaxes[1][0] = rightx;
		stickaxes[1][1] = ITEM_ID_INVALID;
	}
	else if ((ITEM_ID_INVALID != rightx) || (ITEM_ID_INVALID != lefty))
	{
		// degenerate case - right X and possibly left Y, or one or two Y axes
		stickaxes[0][0] = rightx;
		stickaxes[0][1] = lefty;
		stickaxes[1][0] = ITEM_ID_INVALID;
		stickaxes[1][1] = righty;
	}
	else
	{
		// degenerate case - one Y axis at most
		stickaxes[0][0] = ITEM_ID_INVALID;
		stickaxes[0][1] = righty;
		stickaxes[1][0] = ITEM_ID_INVALID;
		stickaxes[1][1] = ITEM_ID_INVALID;
	}
}

} // namespace osd
