// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    inputdev.h

    OSD interface to input devices

***************************************************************************/
#ifndef MAME_OSD_INTERFACE_INPUTDEV_H
#define MAME_OSD_INTERFACE_INPUTDEV_H

#pragma once

#include "inputcode.h"
#include "inputfwd.h"

#include <string_view>
#include <tuple>
#include <vector>


namespace osd {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// base for application representation of host input device

class input_device
{
protected:
	virtual ~input_device() = default;

public:
	using assignment_vector = std::vector<std::tuple<ioport_type, input_seq_type, input_seq> >;

	// relative devices return ~512 units per on-screen pixel
	static inline constexpr s32 RELATIVE_PER_PIXEL = 512;

	// absolute devices return values between -65536 and +65536
	static inline constexpr s32 ABSOLUTE_MIN = -65'536;
	static inline constexpr s32 ABSOLUTE_MAX = 65'536;

	// invalid memory value for axis polling
	static inline constexpr s32 INVALID_AXIS_VALUE = 0x7fff'ffff;

	// callback for getting the value of an individual input on a device
	typedef s32 (*item_get_state_func)(void *device_internal, void *item_internal);

	// add a control item to an input device
	virtual input_item_id add_item(
			std::string_view name,
			std::string_view tokenhint,
			input_item_id itemid,
			item_get_state_func getstate,
			void *internal) = 0;

	// set additional default assignments suitable for device
	virtual void set_default_assignments(assignment_vector &&assignments) = 0;
};

} // namespace osd

#endif // MAME_OSD_INTERFACE_INPUTDEV_H
