// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb
/***************************************************************************

    uievents.h

    OSD UI event interfaces

***************************************************************************/
#ifndef MAME_OSD_INTERFACE_UIEVENTS_H
#define MAME_OSD_INTERFACE_UIEVENTS_H

#pragma once

#include <cstdint>


namespace osd {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// UI event handler interface

class ui_event_handler
{
protected:
	~ui_event_handler() = default;

public:
	enum class pointer
	{
		UNKNOWN,
		MOUSE,
		PEN,
		TOUCH
	};

	// window events
	virtual void push_window_focus_event() = 0;
	virtual void push_window_defocus_event() = 0;

	// legacy mouse events
	virtual void push_mouse_wheel_event(std::int32_t x, std::int32_t y, short delta, int lines) = 0;

	// pointer events
	virtual void push_pointer_update(
			pointer type, std::uint16_t ptrid, std::uint16_t device,
			std::int32_t x, std::int32_t y, std::uint32_t buttons,
			std::uint32_t pressed, std::uint32_t released, std::int16_t clicks) = 0;
	virtual void push_pointer_leave(
			pointer type, std::uint16_t ptrid, std::uint16_t device,
			std::int32_t x, std::int32_t y,
			std::uint32_t released, std::int16_t clicks) = 0;
	virtual void push_pointer_abort(
			pointer type, std::uint16_t ptrid, std::uint16_t device,
			std::int32_t x, std::int32_t y,
			std::uint32_t released, std::int16_t clicks) = 0;

	// text input events
	virtual void push_char_event(char32_t ch) = 0;
};

} // namespace osd

#endif // MAME_OSD_INTERFACE_UIEVENTS_H
