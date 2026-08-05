// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

    uiinput.h

    Internal MAME user interface input state.

***************************************************************************/

#ifndef MAME_EMU_UIINPUT_H
#define MAME_EMU_UIINPUT_H

#pragma once

#include "interface/uievents.h"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct ui_event
{
	enum class type
	{
		NONE,
		WINDOW_FOCUS,
		WINDOW_DEFOCUS,
		MOUSE_WHEEL,
		POINTER_UPDATE,
		POINTER_LEAVE,
		POINTER_ABORT,
		IME_CHAR
	};

	using pointer = osd::ui_event_handler::pointer;

	type                event_type;
	render_target *     target;
	s32                 mouse_x;
	s32                 mouse_y;
	input_item_id       key;
	char32_t            ch;
	short               zdelta;
	int                 num_lines;

	pointer             pointer_type;       // type of input controlling this pointer
	u16                 pointer_id;         // pointer ID - will be recycled aggressively
	u16                 pointer_device;     // for grouping pointers for multi-touch gesture recognition
	s32                 pointer_x;          // pointer X coordinate
	s32                 pointer_y;          // pointer Y coordinate
	u32                 pointer_buttons;    // currently depressed buttons
	u32                 pointer_pressed;    // buttons pressed since last update (primary action in LSB)
	u32                 pointer_released;   // buttons released since last update (primary action in LSB)
	s16                 pointer_clicks;     // positive for multi-click, negative on release if turned into hold or drag
};


class ui_input_manager
{
public:
	~ui_input_manager();

	void check_ui_inputs();

	// pops an event off of the queue
	bool pop_event(ui_event *event);

	// check the next event type without removing it
	ui_event::type peek_event_type() const { return (m_events_start != m_events_end) ? m_events[m_events_start].event_type : ui_event::type::NONE; }

	// clears all outstanding events
	void reset();

	// enable/disable UI key presses
	bool presses_enabled() const { return m_presses_enabled; }
	void set_presses_enabled(bool enabled) { m_presses_enabled = enabled; }

	// return true if a key down for the given user interface sequence is detected
	bool pressed(int code) { return pressed_repeat(code, 0); }

	// return true if a key down for the given user interface sequence is detected, or if autorepeat at the given speed is triggered
	bool pressed_repeat(int code, int speed);

	// get current or most recently focused render target
	render_target *focused_target() const { return m_focused_target; }
	render_target *last_focused_target() const { return m_last_focused_target; }

	void mark_all_as_pressed();

protected:
	// construction/destruction
	ui_input_manager(running_machine &machine);

	bool push_event(ui_event event);
	void target_focused(render_target &target);
	void target_defocused(render_target &target);

private:
	// constants
	static constexpr unsigned EVENT_QUEUE_SIZE = 256;

	// getters
	running_machine &machine() const { return m_machine; }

	// internal state
	running_machine &   m_machine;

	// track focused render target;
	render_target *     m_focused_target;
	render_target *     m_last_focused_target;

	// pressed states; retrieved with pressed() or pressed_repeat()
	bool                m_presses_enabled;
	osd_ticks_t         m_next_repeat[IPT_COUNT];
	u8                  m_seqpressed[IPT_COUNT];

	// ring buffer of ui_events
	ui_event            m_events[EVENT_QUEUE_SIZE];
	int                 m_events_start;
	int                 m_events_end;
};


class ui_event_sink : protected ui_input_manager
{
public:
	using pointer = osd::ui_event_handler::pointer;

	~ui_event_sink();

	// queueing events
	void push_window_focus_event(render_target &target);
	void push_window_defocus_event(render_target &target);
	void push_mouse_wheel_event(render_target &target, s32 x, s32 y, short delta, int lines);
	void push_pointer_update(render_target &target, pointer type, u16 ptrid, u16 device, s32 x, s32 y, u32 buttons, u32 pressed, u32 released, s16 clicks);
	void push_pointer_leave(render_target &target, pointer type, u16 ptrid, u16 device, s32 x, s32 y, u32 released, s16 clicks);
	void push_pointer_abort(render_target &target, pointer type, u16 ptrid, u16 device, s32 x, s32 y, u32 released, s16 clicks);
	void push_char_event(render_target &target, char32_t ch);

protected:
	// construction/destruction
	ui_event_sink(running_machine &machine);
};


class ui_input_manager_impl : protected ui_event_sink
{
public:
	ui_input_manager_impl(running_machine &machine);
	~ui_input_manager_impl();

	auto &input_manager() { return *static_cast<ui_input_manager *>(this); }
	auto &event_sink() { return *static_cast<ui_event_sink*>(this); }
};

#endif // MAME_EMU_UIINPUT_H
