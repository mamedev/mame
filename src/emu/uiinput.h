// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

    uiinput.h

    Internal MAME user interface input state.
***************************************************************************/

#pragma once

#ifndef __UIINPUT_H__
#define __UIINPUT_H__


/***************************************************************************
    CONSTANTS
***************************************************************************/

#define EVENT_QUEUE_SIZE        128

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

class render_target;

enum ui_event_type
{
	UI_EVENT_NONE,
	UI_EVENT_MOUSE_MOVE,
	UI_EVENT_MOUSE_LEAVE,
	UI_EVENT_MOUSE_DOWN,
	UI_EVENT_MOUSE_UP,
	UI_EVENT_MOUSE_RDOWN,
	UI_EVENT_MOUSE_RUP,
	UI_EVENT_MOUSE_DOUBLE_CLICK,
	UI_EVENT_MOUSE_WHEEL,
	UI_EVENT_CHAR
};

struct ui_event
{
	ui_event_type       event_type;
	render_target *     target;
	int32_t               mouse_x;
	int32_t               mouse_y;
	input_item_id       key;
	char32_t        ch;
	short               zdelta;
	int                 num_lines;
};

// ======================> ui_input_manager

class ui_input_manager
{
public:
	// construction/destruction
	ui_input_manager(running_machine &machine);

	void frame_update();

	/* pushes a single event onto the queue */
	bool push_event(ui_event event);

	/* pops an event off of the queue */
	bool pop_event(ui_event *event);

	/* clears all outstanding events */
	void reset();

	/* retrieves the current location of the mouse */
	render_target *find_mouse(int32_t *x, int32_t *y, bool *button) const;
	ioport_field *find_mouse_field() const;

	/* return true if a key down for the given user interface sequence is detected */
	bool pressed(int code);

	/* return true if a key down for the given user interface sequence is detected, or if
	autorepeat at the given speed is triggered */
	bool pressed_repeat(int code, int speed);

	// getters
	running_machine &machine() const { return m_machine; }


	void push_mouse_move_event(render_target* target, int32_t x, int32_t y);
	void push_mouse_leave_event(render_target* target);
	void push_mouse_down_event(render_target* target, int32_t x, int32_t y);
	void push_mouse_up_event(render_target* target, int32_t x, int32_t y);
	void push_mouse_rdown_event(render_target* target, int32_t x, int32_t y);
	void push_mouse_rup_event(render_target* target, int32_t x, int32_t y);
	void push_mouse_double_click_event(render_target* target, int32_t x, int32_t y);
	void push_char_event(render_target* target, char32_t ch);
	void push_mouse_wheel_event(render_target *target, int32_t x, int32_t y, short delta, int ucNumLines);

	void mark_all_as_pressed();

private:

	// internal state
	running_machine &   m_machine;                  // reference to our machine

	/* pressed states; retrieved with ui_input_pressed() */
	osd_ticks_t                 m_next_repeat[IPT_COUNT];
	uint8_t                       m_seqpressed[IPT_COUNT];

	/* mouse position/info */
	render_target *             m_current_mouse_target;
	int32_t                       m_current_mouse_x;
	int32_t                       m_current_mouse_y;
	bool                        m_current_mouse_down;
	ioport_field *              m_current_mouse_field;

	/* popped states; ring buffer of ui_events */
	ui_event                    m_events[EVENT_QUEUE_SIZE];
	int                         m_events_start;
	int                         m_events_end;
};

#endif  /* __UIINPUT_H__ */
