// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

    uiinput.cpp

    Internal MAME user interface input state.

***************************************************************************/

#include "emu.h"
#include "uiinput.h"
#include "render.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

enum
{
	SEQ_PRESSED_FALSE = 0,      /* not pressed */
	SEQ_PRESSED_TRUE,           /* pressed */
	SEQ_PRESSED_RESET           /* reset -- converted to false once detected as not pressed */
};


//**************************************************************************
//  UI INPUT MANAGER
//**************************************************************************

//-------------------------------------------------
//  ui_input_manager - constructor
//-------------------------------------------------

ui_input_manager::ui_input_manager(running_machine &machine)
	: m_machine(machine)
	, m_presses_enabled(true)
	, m_events_start(0)
	, m_events_end(0)
{
	std::fill(std::begin(m_next_repeat), std::end(m_next_repeat), 0);
	std::fill(std::begin(m_seqpressed), std::end(m_seqpressed), 0);
}



/***************************************************************************
    EVENT HANDLING
***************************************************************************/

/*-------------------------------------------------
    check_ui_inputs - looks through pressed input
    as per events pushed our way and posts
    corresponding IPT_UI_* events
-------------------------------------------------*/

void ui_input_manager::check_ui_inputs()
{
	// update the state of all the UI keys
	for (ioport_type code = ioport_type(IPT_UI_FIRST + 1); code < IPT_UI_LAST; ++code)
	{
		if (m_presses_enabled)
		{
			bool pressed = machine().ioport().type_pressed(code);
			if (!pressed || m_seqpressed[code] != SEQ_PRESSED_RESET)
				m_seqpressed[code] = pressed;
		}
		else
		{
			// UI key presses are disabled
			m_seqpressed[code] = false;
		}
	}
}


/*-------------------------------------------------
    push_event - pushes a single event
    onto the queue
-------------------------------------------------*/

bool ui_input_manager::push_event(ui_event evt)
{
	// is the queue filled up?
	if ((m_events_end + 1) % std::size(m_events) == m_events_start)
		return false;

	m_events[m_events_end++] = evt;
	m_events_end %= std::size(m_events);
	return true;
}


/*-------------------------------------------------
    pop_event - pops an event off of the queue
-------------------------------------------------*/

bool ui_input_manager::pop_event(ui_event *evt)
{
	if (m_events_start != m_events_end)
	{
		*evt = m_events[m_events_start++];
		m_events_start %= std::size(m_events);
		return true;
	}
	else
	{
		memset(evt, 0, sizeof(*evt));
		return false;
	}
}


/*-------------------------------------------------
    reset - clears all outstanding events
    and resets the sequence states
-------------------------------------------------*/

void ui_input_manager::reset()
{
	int code;

	m_events_start = 0;
	m_events_end = 0;
	for (code = IPT_UI_FIRST + 1; code < IPT_UI_LAST; code++)
	{
		m_seqpressed[code] = SEQ_PRESSED_RESET;
		m_next_repeat[code] = 0;
	}
}



/***************************************************************************
    USER INTERFACE SEQUENCE READING
***************************************************************************/

/*-------------------------------------------------
    pressed_repeat - return true if a key
    down for the given user interface sequence is
    detected, or if autorepeat at the given speed
    is triggered
-------------------------------------------------*/

bool ui_input_manager::pressed_repeat(int code, int speed)
{
	bool pressed;

	auto profile = g_profiler.start(PROFILER_INPUT);

	/* get the status of this key (assumed to be only in the defaults) */
	assert(code > IPT_UI_FIRST && code < IPT_UI_LAST);
	pressed = (m_seqpressed[code] == SEQ_PRESSED_TRUE);

	/* if down, handle it specially */
	if (pressed)
	{
		osd_ticks_t tps = osd_ticks_per_second();

		/* if this is the first press, set a 3x delay and leave pressed = 1 */
		if (m_next_repeat[code] == 0)
			m_next_repeat[code] = osd_ticks() + 3 * speed * tps / 60;

		/* if this is an autorepeat case, set a 1x delay and leave pressed = 1 */
		else if (speed > 0 && (osd_ticks() + tps - m_next_repeat[code]) >= tps)
		{
			// In the autorepeat case, we need to double-check the key is still pressed
			// as there can be a delay between the key polling and our processing of the event
			m_seqpressed[code] = machine().ioport().type_pressed(ioport_type(code));
			pressed = (m_seqpressed[code] == SEQ_PRESSED_TRUE);
			if (pressed)
				m_next_repeat[code] += 1 * speed * tps / 60;
		}

		/* otherwise, reset pressed = 0 */
		else
			pressed = false;
	}

	/* if we're not pressed, reset the memory field */
	else
		m_next_repeat[code] = 0;

	return pressed;
}

/*-------------------------------------------------
    push_window_focus_event - pushes a focus
    event to the specified render_target
-------------------------------------------------*/

void ui_input_manager::push_window_focus_event(render_target *target)
{
	ui_event event = { ui_event::type::NONE };
	event.event_type = ui_event::type::WINDOW_FOCUS;
	event.target = target;
	push_event(event);
}

/*-------------------------------------------------
    push_window_defocus_event - pushes a defocus
    event to the specified render_target
-------------------------------------------------*/

void ui_input_manager::push_window_defocus_event(render_target *target)
{
	ui_event event = { ui_event::type::NONE };
	event.event_type = ui_event::type::WINDOW_DEFOCUS;
	event.target = target;
	push_event(event);
}

/*-------------------------------------------------
    push_pointer_update - pushes a pointer update
    event to the specified render_target
-------------------------------------------------*/

void ui_input_manager::push_pointer_update(
		render_target *target,
		pointer type,
		u16 ptrid,
		u16 device,
		s32 x,
		s32 y,
		u32 buttons,
		u32 pressed,
		u32 released,
		s16 clicks)
{
	ui_event event = { ui_event::type::NONE };
	event.event_type = ui_event::type::POINTER_UPDATE;
	event.target = target;
	event.pointer_type = type;
	event.pointer_id = ptrid;
	event.pointer_device = device;
	event.pointer_x = x;
	event.pointer_y = y;
	event.pointer_buttons = buttons;
	event.pointer_pressed = pressed;
	event.pointer_released = released;
	event.pointer_clicks = clicks;
	push_event(event);
}

void ui_input_manager::push_pointer_leave(
		render_target *target,
		pointer type,
		u16 ptrid,
		u16 device,
		s32 x,
		s32 y,
		u32 released,
		s16 clicks)
{
	ui_event event = { ui_event::type::NONE };
	event.event_type = ui_event::type::POINTER_LEAVE;
	event.target = target;
	event.pointer_type = type;
	event.pointer_id = ptrid;
	event.pointer_device = device;
	event.pointer_x = x;
	event.pointer_y = y;
	event.pointer_buttons = 0U;
	event.pointer_pressed = 0U;
	event.pointer_released = released;
	event.pointer_clicks = clicks;
	push_event(event);
}

void ui_input_manager::push_pointer_abort(
		render_target *target,
		pointer type,
		u16 ptrid,
		u16 device,
		s32 x,
		s32 y,
		u32 released,
		s16 clicks)
{
	ui_event event = { ui_event::type::NONE };
	event.event_type = ui_event::type::POINTER_ABORT;
	event.target = target;
	event.pointer_type = type;
	event.pointer_id = ptrid;
	event.pointer_device = device;
	event.pointer_x = x;
	event.pointer_y = y;
	event.pointer_buttons = 0U;
	event.pointer_pressed = 0U;
	event.pointer_released = released;
	event.pointer_clicks = clicks;
	push_event(event);
}

/*-------------------------------------------------
    push_char_event - pushes a char event
    to the specified render_target
-------------------------------------------------*/
void ui_input_manager::push_char_event(render_target *target, char32_t ch)
{
	ui_event event = { ui_event::type::NONE };
	event.event_type = ui_event::type::IME_CHAR;
	event.target = target;
	event.ch = ch;
	push_event(event);
}

/*-------------------------------------------------
    push_mouse_wheel_event - pushes a mouse
    wheel event to the specified render_target
-------------------------------------------------*/

void ui_input_manager::push_mouse_wheel_event(render_target *target, s32 x, s32 y, short delta, int lines)
{
	ui_event event = { ui_event::type::NONE };
	event.event_type = ui_event::type::MOUSE_WHEEL;
	event.target = target;
	event.mouse_x = x;
	event.mouse_y = y;
	event.zdelta = delta;
	event.num_lines = lines;
	push_event(event);
}

/*-------------------------------------------------
    mark_all_as_pressed - marks all buttons
    as if they were already pressed once
-------------------------------------------------*/
void ui_input_manager::mark_all_as_pressed()
{
	for (int code = IPT_UI_FIRST + 1; code < IPT_UI_LAST; code++)
		m_next_repeat[code] = osd_ticks();
}
