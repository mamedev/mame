/***************************************************************************

    stackable.c

    Stackable UI primitives

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#include "emu.h"
#include "stackable.h"
#include "uiinput.h"


/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

ui_stackable *ui_stackable::menu_stack;
ui_stackable *ui_stackable::menu_free;


/***************************************************************************
    MENU STACK MANAGEMENT
***************************************************************************/

ui_stackable::ui_stackable(running_machine &machine, render_container *_container)
	: m_machine(machine)
{
	container = _container;
	special_main_menu = false;
}


/*-------------------------------------------------
    stack_reset - reset the menu stack
-------------------------------------------------*/

void ui_stackable::stack_reset(running_machine &machine)
{
	while (menu_stack != NULL)
		ui_menu::stack_pop(machine);
}


/*-------------------------------------------------
    stack_push - push a new menu onto the
    stack
-------------------------------------------------*/

void ui_stackable::stack_push(ui_stackable *menu)
{
	menu->parent = menu_stack;
	menu_stack = menu;
	menu->reset();
	ui_input_reset(menu->machine());
}


/*-------------------------------------------------
    stack_pop - pop a menu from the stack
-------------------------------------------------*/

void ui_stackable::stack_pop(running_machine &machine)
{
	if (menu_stack != NULL)
	{
		ui_stackable *menu = menu_stack;
		menu_stack = menu->parent;
		menu->parent = menu_free;
		menu_free = menu;
		ui_input_reset(machine);
	}
}


/*-------------------------------------------------
    ui_stackable::stack_has_special_main_menu -
    check in the special main menu is in the stack
-------------------------------------------------*/

bool ui_stackable::stack_has_special_main_menu()
{
	ui_stackable *menu;

	for (menu = menu_stack; menu != NULL; menu = menu->parent)
		if (menu->is_special_main_menu())
			return true;

	return false;
}


/*-------------------------------------------------
    is_special_main_menu - returns whether the
    menu has special needs
-------------------------------------------------*/
bool ui_stackable::is_special_main_menu() const
{
	return special_main_menu;
}


/*-------------------------------------------------
    set_special_main_menu - set whether the
    menu has special needs
-------------------------------------------------*/
void ui_stackable::set_special_main_menu(bool special)
{
	special_main_menu = special;
}


/*-------------------------------------------------
    clear_free_list - clear out anything
    accumulated in the free list
-------------------------------------------------*/

void ui_stackable::clear_free_list(running_machine &machine)
{
	while (menu_free != NULL)
	{
		ui_stackable *menu = menu_free;
		menu_free = menu->parent;
		auto_free(machine, menu);
	}
}

