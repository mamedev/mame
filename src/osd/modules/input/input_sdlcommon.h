// license:BSD-3-Clause
// copyright-holders:Vas Crabb
#ifndef MAME_OSD_INPUT_INPUT_SDLCOMMON_H
#define MAME_OSD_INPUT_INPUT_SDLCOMMON_H

#pragma once

#include "assignmenthelper.h"

#include "interface/inputdev.h"

#include <cstdint>


namespace osd {

class sdl_joystick_device_common : protected joystick_assignment_helper
{
public:
	static constexpr unsigned MAX_AXES = 32;
	static constexpr unsigned MAX_BUTTONS = 128;
	static constexpr unsigned MAX_HATS = 8;

protected:
	// state information for a joystick
	struct sdl_joystick_state
	{
		std::int32_t axes[MAX_AXES];
		std::int32_t balls[MAX_AXES];
		std::uint8_t buttons[MAX_BUTTONS];
		std::uint8_t hatsU[MAX_HATS], hatsD[MAX_HATS], hatsL[MAX_HATS], hatsR[MAX_HATS];
	};

	sdl_joystick_device_common();

	void configure_common(input_device &device, int axiscount, int buttoncount, int hatcount, int ballcount);

	void clear_buffer() noexcept;

	sdl_joystick_state m_joystick;
	std::int32_t m_ball[MAX_AXES];
};

} // namespace osd

#endif // MAME_OSD_INPUT_INPUT_SDLCOMMON_H
