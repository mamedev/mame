// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    inputman.h

    OSD interface to the input manager

***************************************************************************/
#ifndef MAME_OSD_INTERFACE_INPUTMAN_H
#define MAME_OSD_INTERFACE_INPUTMAN_H

#pragma once

#include "inputcode.h"
#include "inputfwd.h"

#include <string_view>


namespace osd {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// interface to application input manager

class input_manager
{
protected:
	virtual ~input_manager() = default;

public:
	virtual bool class_enabled(input_device_class devclass) const = 0;

	virtual input_device &add_device(
			input_device_class devclass,
			std::string_view name,
			std::string_view id,
			void *internal = nullptr) = 0;
};

} // namespace osd

#endif // MAME_OSD_INTERFACE_INPUTMAN_H
