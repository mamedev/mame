// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Nathan Woods

/***************************************************************************

    ui/menuitem.h

    Internal data representation for a UI menu item.

***************************************************************************/

#pragma once

#ifndef __UI_MENUITEM__
#define __UI_MENUITEM__

#include "emu.h"

// special menu item for separators
#define MENU_SEPARATOR_ITEM         "---"

// types of menu items (TODO: please expand)
enum class ui_menu_item_type
{
	UNKNOWN,
	SLIDER,
	SEPARATOR
};

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

#endif // __UI_MENUITEM__
