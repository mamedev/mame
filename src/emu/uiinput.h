// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

    uiinput.h

    Internal MAME user interface input state.
***************************************************************************/

#pragma once

#ifndef __UIINPUT_H__
#define __UIINPUT_H__

#include "render.h"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

enum ui_event_type
{
	UI_EVENT_NONE,
	UI_EVENT_MOUSE_MOVE,
	UI_EVENT_MOUSE_LEAVE,
	UI_EVENT_MOUSE_DOWN,
	UI_EVENT_MOUSE_UP,
	UI_EVENT_MOUSE_DOUBLE_CLICK,
	UI_EVENT_CHAR
};


struct ui_event
{
	ui_event_type       event_type;
	render_target *     target;
	INT32               mouse_x;
	INT32               mouse_y;
	input_item_id       key;
	unicode_char        ch;
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/


/* ----- core system management ----- */

/* initialization */
void ui_input_init(running_machine &machine);



/* ----- event handling ----- */

void ui_input_frame_update(running_machine &machine);

/* pushes a single event onto the queue */
bool ui_input_push_event(running_machine &machine, ui_event event);

/* pops an event off of the queue */
bool ui_input_pop_event(running_machine &machine, ui_event *event);

/* clears all outstanding events */
void ui_input_reset(running_machine &machine);

/* retrieves the current location of the mouse */
render_target *ui_input_find_mouse(running_machine &machine, INT32 *x, INT32 *y, bool *button);



/* ----- user interface sequence reading ----- */

/* return TRUE if a key down for the given user interface sequence is detected */
bool ui_input_pressed(running_machine &machine, int code);

/* return TRUE if a key down for the given user interface sequence is detected, or if
   autorepeat at the given speed is triggered */
bool ui_input_pressed_repeat(running_machine &machine, int code, int speed);



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    ui_input_push_mouse_move_event - pushes a mouse
    move event to the specified render_target
-------------------------------------------------*/

INLINE void ui_input_push_mouse_move_event(running_machine &machine, render_target *target, INT32 x, INT32 y)
{
	ui_event event = { UI_EVENT_NONE };
	event.event_type = UI_EVENT_MOUSE_MOVE;
	event.target = target;
	event.mouse_x = x;
	event.mouse_y = y;
	ui_input_push_event(machine, event);
}


/*-------------------------------------------------
    ui_input_push_mouse_leave_event - pushes a
    mouse leave event to the specified render_target
-------------------------------------------------*/

INLINE void ui_input_push_mouse_leave_event(running_machine &machine, render_target *target)
{
	ui_event event = { UI_EVENT_NONE };
	event.event_type = UI_EVENT_MOUSE_LEAVE;
	event.target = target;
	ui_input_push_event(machine, event);
}


/*-------------------------------------------------
    ui_input_push_mouse_down_event - pushes a mouse
    down event to the specified render_target
-------------------------------------------------*/

INLINE void ui_input_push_mouse_down_event(running_machine &machine, render_target *target, INT32 x, INT32 y)
{
	ui_event event = { UI_EVENT_NONE };
	event.event_type = UI_EVENT_MOUSE_DOWN;
	event.target = target;
	event.mouse_x = x;
	event.mouse_y = y;
	ui_input_push_event(machine, event);
}


/*-------------------------------------------------
    ui_input_push_mouse_down_event - pushes a mouse
    down event to the specified render_target
-------------------------------------------------*/

INLINE void ui_input_push_mouse_up_event(running_machine &machine, render_target *target, INT32 x, INT32 y)
{
	ui_event event = { UI_EVENT_NONE };
	event.event_type = UI_EVENT_MOUSE_UP;
	event.target = target;
	event.mouse_x = x;
	event.mouse_y = y;
	ui_input_push_event(machine, event);
}


/*-------------------------------------------------
    ui_input_push_mouse_double_click_event - pushes
    a mouse double-click event to the specified
    render_target
-------------------------------------------------*/

INLINE void ui_input_push_mouse_double_click_event(running_machine &machine, render_target *target, INT32 x, INT32 y)
{
	ui_event event = { UI_EVENT_NONE };
	event.event_type = UI_EVENT_MOUSE_DOUBLE_CLICK;
	event.target = target;
	event.mouse_x = x;
	event.mouse_y = y;
	ui_input_push_event(machine, event);
}


/*-------------------------------------------------
    ui_input_push_char_event - pushes a char event
    to the specified render_target
-------------------------------------------------*/

INLINE void ui_input_push_char_event(running_machine &machine, render_target *target, unicode_char ch)
{
	ui_event event = { UI_EVENT_NONE };
	event.event_type = UI_EVENT_CHAR;
	event.target = target;
	event.ch = ch;
	ui_input_push_event(machine, event);
}


#endif  /* __UIINPUT_H__ */
