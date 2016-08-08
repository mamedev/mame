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
	SEQ_PRESSED_RESET           /* reset -- converted to FALSE once detected as not pressed */
};


/***************************************************************************
    LOCAL VARIABLES
***************************************************************************/

// list of natural keyboard keys that are not associated with UI_EVENT_CHARs
const input_item_id ui_input_manager::s_non_char_keys[] =
{
	ITEM_ID_ESC,
	ITEM_ID_F1,
	ITEM_ID_F2,
	ITEM_ID_F3,
	ITEM_ID_F4,
	ITEM_ID_F5,
	ITEM_ID_F6,
	ITEM_ID_F7,
	ITEM_ID_F8,
	ITEM_ID_F9,
	ITEM_ID_F10,
	ITEM_ID_F11,
	ITEM_ID_F12,
	ITEM_ID_NUMLOCK,
	ITEM_ID_0_PAD,
	ITEM_ID_1_PAD,
	ITEM_ID_2_PAD,
	ITEM_ID_3_PAD,
	ITEM_ID_4_PAD,
	ITEM_ID_5_PAD,
	ITEM_ID_6_PAD,
	ITEM_ID_7_PAD,
	ITEM_ID_8_PAD,
	ITEM_ID_9_PAD,
	ITEM_ID_DEL_PAD,
	ITEM_ID_PLUS_PAD,
	ITEM_ID_MINUS_PAD,
	ITEM_ID_INSERT,
	ITEM_ID_DEL,
	ITEM_ID_HOME,
	ITEM_ID_END,
	ITEM_ID_PGUP,
	ITEM_ID_PGDN,
	ITEM_ID_UP,
	ITEM_ID_DOWN,
	ITEM_ID_LEFT,
	ITEM_ID_RIGHT,
	ITEM_ID_PAUSE,
	ITEM_ID_CANCEL
};


//**************************************************************************
//  UI INPUT MANAGER
//**************************************************************************

//-------------------------------------------------
//  ui_input_manager - constructor
//-------------------------------------------------

ui_input_manager::ui_input_manager(running_machine &machine)
	: m_machine(machine),
		m_current_mouse_target(nullptr),
		m_current_mouse_down(false),
		m_current_mouse_field(nullptr),
		m_events_start(0),
		m_events_end(0)
{
	/* create the private data */
	m_current_mouse_x = -1;
	m_current_mouse_y = -1;
	m_non_char_keys_down = std::make_unique<UINT8[]>((ARRAY_LENGTH(s_non_char_keys) + 7) / 8);

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

	// perform mouse hit testing
	ioport_field *mouse_field = m_current_mouse_down ? find_mouse_field() : nullptr;
	if (m_current_mouse_field != mouse_field)
	{
		// clear the old field if there was one
		if (m_current_mouse_field != nullptr)
			m_current_mouse_field->set_value(0);

		// set the new field if it exists and isn't already being pressed
		if (mouse_field != nullptr && !mouse_field->digital_value())
			mouse_field->set_value(1);

		// update internal state
		m_current_mouse_field = mouse_field;
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

render_target *ui_input_manager::find_mouse(INT32 *x, INT32 *y, bool *button) const
{
	if (x != nullptr)
		*x = m_current_mouse_x;
	if (y != nullptr)
		*y = m_current_mouse_y;
	if (button != nullptr)
		*button = m_current_mouse_down;
	return m_current_mouse_target;
}


/*-------------------------------------------------
    find_mouse_field - retrieves the input field
    the mouse is currently pointing at
-------------------------------------------------*/

ioport_field *ui_input_manager::find_mouse_field() const
{
	// map the point and determine what was hit
	if (m_current_mouse_target != nullptr)
	{
		ioport_port *port = nullptr;
		ioport_value mask;
		float x, y;
		if (m_current_mouse_target->map_point_input(m_current_mouse_x, m_current_mouse_y, port, mask, x, y))
		{
			if (port != nullptr)
				return port->field(mask);
		}
	}
	return nullptr;
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
push_mouse_down_event - pushes a mouse
down event to the specified render_target
-------------------------------------------------*/

void ui_input_manager::push_mouse_rdown_event(render_target* target, INT32 x, INT32 y)
{
	ui_event event = { UI_EVENT_NONE };
	event.event_type = UI_EVENT_MOUSE_RDOWN;
	event.target = target;
	event.mouse_x = x;
	event.mouse_y = y;
	push_event(event);
}

/*-------------------------------------------------
push_mouse_down_event - pushes a mouse
down event to the specified render_target
-------------------------------------------------*/

void ui_input_manager::push_mouse_rup_event(render_target* target, INT32 x, INT32 y)
{
	ui_event event = { UI_EVENT_NONE };
	event.event_type = UI_EVENT_MOUSE_RUP;
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


//-------------------------------------------------
//  process_natural_keyboard - processes any
//  natural keyboard input
//-------------------------------------------------

void ui_input_manager::process_natural_keyboard()
{
	// loop while we have interesting events
	ui_event event;
	while (pop_event(&event))
	{
		// if this was a UI_EVENT_CHAR event, post it
		if (event.event_type == UI_EVENT_CHAR)
			machine().ioport().natkeyboard().post(event.ch);
	}

	// process natural keyboard keys that don't get UI_EVENT_CHARs
	for (int i = 0; i < ARRAY_LENGTH(s_non_char_keys); i++)
	{
		// identify this keycode
		input_item_id itemid = s_non_char_keys[i];
		input_code code = machine().input().code_from_itemid(itemid);

		// ...and determine if it is pressed
		bool pressed = machine().input().code_pressed(code);

		// figure out whey we are in the key_down map
		UINT8 *key_down_ptr = &m_non_char_keys_down[i / 8];
		UINT8 key_down_mask = 1 << (i % 8);

		if (pressed && !(*key_down_ptr & key_down_mask))
		{
			// this key is now down
			*key_down_ptr |= key_down_mask;

			// post the key
			machine().ioport().natkeyboard().post(UCHAR_MAMEKEY_BEGIN + code.item_id());
		}
		else if (!pressed && (*key_down_ptr & key_down_mask))
		{
			// this key is now up
			*key_down_ptr &= ~key_down_mask;
		}
	}
}
