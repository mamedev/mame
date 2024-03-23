// license:BSD-3-Clause
// copyright-holders:Vas Crabb
//============================================================
//
//  assignmenthelper.h - input assignment setup helper
//
//============================================================
#ifndef MAME_OSD_INPUT_ASSIGNMENTHELPER_H
#define MAME_OSD_INPUT_ASSIGNMENTHELPER_H

#pragma once

#include "interface/inputcode.h"
#include "interface/inputdev.h"

#include <initializer_list>


namespace osd {

class joystick_assignment_helper
{
protected:
	static constexpr input_code make_code(
			input_item_class itemclass,
			input_item_modifier modifier,
			input_item_id item)
	{
		return input_code(DEVICE_CLASS_JOYSTICK, 0, itemclass, modifier, item);
	}

	static bool add_assignment(
			input_device::assignment_vector &assignments,
			ioport_type fieldtype,
			input_seq_type seqtype,
			input_item_class itemclass,
			input_item_modifier modifier,
			std::initializer_list<input_item_id> items);

	static bool add_button_assignment(
			input_device::assignment_vector &assignments,
			ioport_type field_type,
			std::initializer_list<input_item_id> items);

	static bool add_button_pair_assignment(
			input_device::assignment_vector &assignments,
			ioport_type field1,
			ioport_type field2,
			input_item_id button1,
			input_item_id button2);

	static bool add_axis_inc_dec_assignment(
			input_device::assignment_vector &assignments,
			ioport_type field_type,
			input_item_id button_dec,
			input_item_id button_inc);

	static bool add_axis_pair_assignment(
			input_device::assignment_vector &assignments,
			ioport_type field1,
			ioport_type field2,
			input_item_id axis);

	static bool consume_button_pair(
			input_device::assignment_vector &assignments,
			ioport_type field1,
			ioport_type field2,
			input_item_id &button1,
			input_item_id &button2);

	static bool consume_trigger_pair(
			input_device::assignment_vector &assignments,
			ioport_type field1,
			ioport_type field2,
			input_item_id &axis1,
			input_item_id &axis2);

	static bool consume_axis_pair(
			input_device::assignment_vector &assignments,
			ioport_type field1,
			ioport_type field2,
			input_item_id &axis);

	static void add_directional_assignments(
			input_device::assignment_vector &assignments,
			input_item_id xaxis,
			input_item_id yaxis,
			input_item_id leftswitch,
			input_item_id rightswitch,
			input_item_id upswitch,
			input_item_id downswitch);

	static void add_twin_stick_assignments(
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
			input_item_id rightdown);

	static void choose_primary_stick(
			input_item_id (&stickaxes)[2][2],
			input_item_id leftx,
			input_item_id lefty,
			input_item_id rightx,
			input_item_id righty);
};

} // namespace osd

#endif // MAME_OSD_INPUT_ASSIGNMENTHELPER_H
