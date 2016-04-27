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
    CONSTANTS
***************************************************************************/

// special menu item for separators
#define MENU_SEPARATOR_ITEM         "---"


#define SLIDER_NOCHANGE     0x12345678


typedef INT32(*slider_update)(running_machine &machine, void *arg, int id, std::string *str, INT32 newval);

struct slider_state
{
	slider_state *  next;               /* pointer to next slider */
	slider_update   update;             /* callback */
	void *          arg;                /* argument */
	INT32           minval;             /* minimum value */
	INT32           defval;             /* default value */
	INT32           maxval;             /* maximum value */
	INT32           incval;             /* increment value */
	int             id;
	char            description[1];     /* textual description */
};

// types of menu items (TODO: please expand)
enum class ui_menu_item_type
{
	UNKNOWN,
	SLIDER,
	SEPARATOR
};



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

class ui_menu_item
{
public:
	const char          *text;
	const char          *subtext;
	UINT32              flags;
	void                *ref;
	ui_menu_item_type   type;   // item type (eventually will go away when itemref is proper ui_menu_item class rather than void*)

	inline bool is_selectable() const;
};

class ui_manager
{
public:
	// construction/destruction
	ui_manager(running_machine &machine) : m_machine(machine),m_use_natural_keyboard(false),m_show_timecode_counter(false),m_show_timecode_total(false) { }

	virtual ~ui_manager() { }

	virtual void set_startup_text(const char *text, bool force) { }

	virtual bool is_menu_active() { return false; }

	bool use_natural_keyboard() const { return m_use_natural_keyboard; }

	void set_show_timecode_counter(bool value) { m_show_timecode_counter = value; m_show_timecode_total = true; }

	bool show_timecode_counter() const { return m_show_timecode_counter; }
	bool show_timecode_total() const { return m_show_timecode_total; }

	virtual void popup_time_string(int seconds, std::string message) { }

	virtual void image_display(const device_type &type, device_image_interface *image) { }

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
