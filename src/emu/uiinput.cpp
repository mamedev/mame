// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

    uiinput.c

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
	SEQ_PRESSED_RESET           /* reset -- converted to FALSE once detected as not pressed */
};


//**************************************************************************
//  NETWORK MANAGER
//**************************************************************************

//-------------------------------------------------
//  network_manager - constructor
//-------------------------------------------------

ui_input_manager::ui_input_manager(running_machine &machine)
	: m_machine(machine),
		m_current_mouse_target(nullptr),
		m_current_mouse_down(false),
		m_events_start(0),
		m_events_end(0)
{
	/* create the private data */
	m_current_mouse_x = -1;
	m_current_mouse_y = -1;

	/* add a frame callback to poll inputs */
	machine.add_notifier(MACHINE_NOTIFY_FRAME, machine_notify_delegate(FUNC(ui_input_manager::frame_update), this));
}



/***************************************************************************
    EVENT HANDLING
***************************************************************************/

/*-------------------------------------------------
    frame_update - looks through pressed
    input as per events pushed our way and posts
    corresponding IPT_UI_* events
-------------------------------------------------*/

void ui_input_manager::frame_update()
{
	/* update the state of all the UI keys */
	for (ioport_type code = ioport_type(IPT_UI_FIRST + 1); code < IPT_UI_LAST; ++code)
	{
		bool pressed = machine().ioport().type_pressed(code);
		if (!pressed || m_seqpressed[code] != SEQ_PRESSED_RESET)
			m_seqpressed[code] = pressed;
	}
}


/*-------------------------------------------------
    push_event - pushes a single event
    onto the queue
-------------------------------------------------*/

bool ui_input_manager::push_event(ui_event evt)
{
	/* some pre-processing (this is an icky place to do this stuff!) */
	switch (evt.event_type)
	{
		case UI_EVENT_MOUSE_MOVE:
			m_current_mouse_target = evt.target;
			m_current_mouse_x = evt.mouse_x;
			m_current_mouse_y = evt.mouse_y;
			break;

		case UI_EVENT_MOUSE_LEAVE:
			if (m_current_mouse_target == evt.target)
			{
				m_current_mouse_target = nullptr;
				m_current_mouse_x = -1;
				m_current_mouse_y = -1;
			}
			break;

		case UI_EVENT_MOUSE_DOWN:
			m_current_mouse_down = true;
			break;

		case UI_EVENT_MOUSE_UP:
			m_current_mouse_down = false;
			break;

		default:
			/* do nothing */
			break;
	}

	/* is the queue filled up? */
	if ((m_events_end + 1) % ARRAY_LENGTH(m_events) == m_events_start)
		return false;

	m_events[m_events_end++] = evt;
	m_events_end %= ARRAY_LENGTH(m_events);
	return true;
}


/*-------------------------------------------------
    pop_event - pops an event off of the queue
-------------------------------------------------*/

bool ui_input_manager::pop_event(ui_event *evt)
{
	bool result;

	if (m_events_start != m_events_end)
	{
		*evt = m_events[m_events_start++];
		m_events_start %= ARRAY_LENGTH(m_events);
		result = true;
	}
	else
	{
		memset(evt, 0, sizeof(*evt));
		result = false;
	}
	return result;
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


/*-------------------------------------------------
    find_mouse - retrieves the current
    location of the mouse
-------------------------------------------------*/

render_target *ui_input_manager::find_mouse(INT32 *x, INT32 *y, bool *button)
{
	if (x != nullptr)
		*x = m_current_mouse_x;
	if (y != nullptr)
		*y = m_current_mouse_y;
	if (button != nullptr)
		*button = m_current_mouse_down;
	return m_current_mouse_target;
}



/***************************************************************************
    USER INTERFACE SEQUENCE READING
***************************************************************************/

/*-------------------------------------------------
    pressed - return TRUE if a key down
    for the given user interface sequence is
    detected
-------------------------------------------------*/

bool ui_input_manager::pressed(int code)
{
	return pressed_repeat(code, 0);
}


/*-------------------------------------------------
    pressed_repeat - return TRUE if a key
    down for the given user interface sequence is
    detected, or if autorepeat at the given speed
    is triggered
-------------------------------------------------*/

bool ui_input_manager::pressed_repeat(int code, int speed)
{
	int pressed;

g_profiler.start(PROFILER_INPUT);

	/* get the status of this key (assumed to be only in the defaults) */
	assert(code >= IPT_UI_CONFIGURE && code <= IPT_OSD_16);
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
			m_next_repeat[code] += 1 * speed * tps / 60;

		/* otherwise, reset pressed = 0 */
		else
			pressed = false;
	}

	/* if we're not pressed, reset the memory field */
	else
		m_next_repeat[code] = 0;

g_profiler.stop();

	return pressed;
}

/*-------------------------------------------------
    push_mouse_move_event - pushes a mouse
    move event to the specified render_target
-------------------------------------------------*/

void ui_input_manager::push_mouse_move_event(render_target* target, INT32 x, INT32 y)
{
	ui_event event = { UI_EVENT_NONE };
	event.event_type = UI_EVENT_MOUSE_MOVE;
	event.target = target;
	event.mouse_x = x;
	event.mouse_y = y;
	push_event(event);
}

/*-------------------------------------------------
    push_mouse_leave_event - pushes a
    mouse leave event to the specified render_target
-------------------------------------------------*/

void ui_input_manager::push_mouse_leave_event(render_target* target)
{
	ui_event event = { UI_EVENT_NONE };
	event.event_type = UI_EVENT_MOUSE_LEAVE;
	event.target = target;
	push_event(event);
}

/*-------------------------------------------------
    push_mouse_down_event - pushes a mouse
    down event to the specified render_target
-------------------------------------------------*/

void ui_input_manager::push_mouse_down_event(render_target* target, INT32 x, INT32 y)
{
	ui_event event = { UI_EVENT_NONE };
	event.event_type = UI_EVENT_MOUSE_DOWN;
	event.target = target;
	event.mouse_x = x;
	event.mouse_y = y;
	push_event(event);
}

/*-------------------------------------------------
    push_mouse_down_event - pushes a mouse
    down event to the specified render_target
-------------------------------------------------*/

void ui_input_manager::push_mouse_up_event(render_target* target, INT32 x, INT32 y)
{
	ui_event event = { UI_EVENT_NONE };
	event.event_type = UI_EVENT_MOUSE_UP;
	event.target = target;
	event.mouse_x = x;
	event.mouse_y = y;
	push_event(event);
}

/*-------------------------------------------------
    push_mouse_double_click_event - pushes
    a mouse double-click event to the specified
    render_target
-------------------------------------------------*/
void ui_input_manager::push_mouse_double_click_event(render_target* target, INT32 x, INT32 y)
{
	ui_event event = { UI_EVENT_NONE };
	event.event_type = UI_EVENT_MOUSE_DOUBLE_CLICK;
	event.target = target;
	event.mouse_x = x;
	event.mouse_y = y;
	push_event(event);
}

/*-------------------------------------------------
    push_char_event - pushes a char event
    to the specified render_target
-------------------------------------------------*/
void ui_input_manager::push_char_event(render_target* target, unicode_char ch)
{
	ui_event event = { UI_EVENT_NONE };
	event.event_type = UI_EVENT_CHAR;
	event.target = target;
	event.ch = ch;
	push_event(event);
}

/*-------------------------------------------------
	push_mouse_wheel_event - pushes a mouse 
	wheel event to the specified render_target
-------------------------------------------------*/

void ui_input_manager::push_mouse_wheel_event(render_target *target, INT32 x, INT32 y, short delta, int ucNumLines)
{
	ui_event event = { UI_EVENT_NONE };
	event.event_type = UI_EVENT_MOUSE_WHEEL;
	event.target = target;
	event.mouse_x = x;
	event.mouse_y = y;
	event.zdelta = delta;
	event.num_lines = ucNumLines;
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