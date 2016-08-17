// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Nathan Woods
/***************************************************************************

    ui/menu.h

    Internal MAME menus for the user interface.

***************************************************************************/

#pragma once

#ifndef __BASIC_UI_H__
#define __BASIC_UI_H__

#include "emu.h"

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

class ui_manager
{
public:
	// construction/destruction
	ui_manager(running_machine &machine) : m_machine(machine),m_use_natural_keyboard(false),m_show_timecode_counter(false),m_show_timecode_total(false) { }

	virtual ~ui_manager() { }

	virtual void set_startup_text(const char *text, bool force) { }

	// is a menuing system active?  we want to disable certain keyboard/mouse inputs under such context
	virtual bool is_menu_active() { return false; }

	bool use_natural_keyboard() const { return m_use_natural_keyboard; }

	void set_show_timecode_counter(bool value) { m_show_timecode_counter = value; m_show_timecode_total = true; }

	bool show_timecode_counter() const { return m_show_timecode_counter; }
	bool show_timecode_total() const { return m_show_timecode_total; }

	virtual void popup_time_string(int seconds, std::string message) { }

	virtual void menu_reset() { }

	template <typename Format, typename... Params> void popup_time(int seconds, Format &&fmt, Params &&... args);

protected:
	// instance variables
	running_machine &       m_machine;
	bool                    m_use_natural_keyboard;
	bool                    m_show_timecode_counter;
	bool                    m_show_timecode_total;
};

/***************************************************************************
    MEMBER TEMPLATES
***************************************************************************/

//-------------------------------------------------
//  popup_time - popup a message for a specific
//  amount of time
//-------------------------------------------------

template <typename Format, typename... Params>
inline void ui_manager::popup_time(int seconds, Format &&fmt, Params &&... args)
{
	// extract the text
	popup_time_string(seconds, string_format(std::forward<Format>(fmt), std::forward<Params>(args)...));
}

#endif  // __BASIC_UI_H__
