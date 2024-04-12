// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb
/***************************************************************************

    uievents.h

    OSD UI event interfaces

***************************************************************************/
#ifndef MAME_OSD_INTERFACE_UIEVENTS_H
#define MAME_OSD_INTERFACE_UIEVENTS_H


//**************************************************************************
//  FORWARD DECLARATIONS
//**************************************************************************

class render_target;


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
	virtual void push_window_focus_event(render_target *target) = 0;
	virtual void push_window_defocus_event(render_target *target) = 0;

	// legacy mouse events
	virtual void push_mouse_wheel_event(render_target *target, s32 x, s32 y, short delta, int lines) = 0;

	// pointer events
	virtual void push_pointer_update(render_target *target, pointer type, u16 ptrid, u16 device, s32 x, s32 y, u32 buttons, u32 pressed, u32 released, s16 clicks) = 0;
	virtual void push_pointer_leave(render_target *target, pointer type, u16 ptrid, u16 device, s32 x, s32 y, u32 released, s16 clicks) = 0;
	virtual void push_pointer_abort(render_target *target, pointer type, u16 ptrid, u16 device, s32 x, s32 y, u32 released, s16 clicks) = 0;

	// text input events
	virtual void push_char_event(render_target *target, char32_t ch) = 0;
};

} // namespace osd

#endif // MAME_OSD_INTERFACE_UIEVENTS_H
