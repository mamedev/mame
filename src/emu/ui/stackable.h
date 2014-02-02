/***************************************************************************

    stackable.h

    Stackable UI primitives

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __UI_STACKABLE_H__
#define __UI_STACKABLE_H__

#include "render.h"


//**************************************************************************
//  STACKABLES
//**************************************************************************

class ui_stackable
{
public:
	ui_stackable(running_machine &machine, render_container *container);

	running_machine &machine() const { return m_machine; }

	render_container *	container;          /* render_container we render to */
	ui_stackable *		parent;             /* pointer to parent menu */

	/* reset the menus, clearing everything */
	static void stack_reset(running_machine &machine);

	/* push a new menu onto the stack */
	static void stack_push(ui_stackable *menu);

	/* pop a menu from the stack */
	static void stack_pop(running_machine &machine);

	/* test if one of the menus in the stack requires hide disable */
	static bool stack_has_special_main_menu();

	/* request the specific handling of the game selection main menu */
	bool is_special_main_menu() const;
	void set_special_main_menu(bool disable);

	virtual void reset() = 0;
	virtual void do_handle() = 0;

protected:
	static ui_stackable *menu_stack;

	static void clear_free_list(running_machine &machine);

private:
	static ui_stackable *menu_free;

	running_machine &	m_machine;          /* machine we are attached to */
	bool				special_main_menu;
};

#endif // __UI_STACKABLE_H__
