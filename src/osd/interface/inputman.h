// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    inputman.h

    OSD interface to the input manager.

***************************************************************************/
#ifndef MAME_OSD_INTERFACE_INPUTMAN_H
#define MAME_OSD_INTERFACE_INPUTMAN_H

#pragma once

#include "inputcode.h"

#include <string_view>


namespace osd {

//**************************************************************************
//  CONSTANTS
//**************************************************************************

// relative devices return ~512 units per on-screen pixel
constexpr s32 INPUT_RELATIVE_PER_PIXEL = 512;

// absolute devices return values between -65536 and +65536
constexpr s32 INPUT_ABSOLUTE_MIN = -65'536;
constexpr s32 INPUT_ABSOLUTE_MAX = 65'536;

// invalid memory value for axis polling
constexpr s32 INVALID_AXIS_VALUE = 0x7fff'ffff;



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// base for application representation of host input device

class input_device
{
public:
	// callback for getting the value of an individual input on a device
	typedef s32 (*item_get_state_func)(void *device_internal, void *item_internal);

	virtual ~input_device() = default;

	virtual input_item_id add_item(
			std::string_view name,
			input_item_id itemid,
			item_get_state_func getstate,
			void *internal = nullptr) = 0;
};


// base for application input manager

class input_manager
{
public:
	virtual ~input_manager() = default;

	virtual input_device &add_device(
			input_device_class devclass,
			std::string_view name,
			std::string_view id,
			void *internal = nullptr) = 0;
};


} // namespace osd

#endif // MAME_OSD_INTERFACE_INPUTMAN_H
