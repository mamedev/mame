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
#include "ui.h"
#include "uiinput.h"


//**************************************************************************
//  STACKABLES
//**************************************************************************

class ui_stackable
{
public:
	ui_stackable(running_machine &machine, render_container *container);
	virtual ~ui_stackable();

	running_machine &machine() const { return m_machine; }

	render_container *	container;          /* render_container we render to */
	ui_stackable *		parent;             /* pointer to parent menu */

	// global initialization
	static void init(running_machine &machine);

	// reset the menus, clearing everything
	static void stack_reset(running_machine &machine);

	// push a new menu onto the stack
	static void stack_push(ui_stackable *menu);

	// pop a menu from the stack
	static void stack_pop(running_machine &machine);

	// test if one of the menus in the stack requires hide disable
	static bool stack_has_special_main_menu();

	// request the specific handling of the game selection main menu
	bool is_special_main_menu() const;
	void set_special_main_menu(bool disable);

	virtual void reset() = 0;
	virtual void do_handle() = 0;

protected:
	// static variables
	static ui_stackable *menu_stack;
	static render_texture *arrow_texture;

	// static methods
	static void clear_free_list(running_machine &machine);

	// rendering
	float get_line_height();
	float get_char_width(unicode_char ch);
	float get_string_width(const char *s);
	void draw_outlined_box(float x0, float y0, float x1, float y1, rgb_t backcolor = UI_TEXT_COLOR);
	void draw_text(const char *origs, float x, float y, rgb_t fgcolor = UI_TEXT_COLOR, rgb_t bgcolor = UI_TEXT_BG_COLOR);
	void draw_text(const char *origs, float x, float y, float origwrapwidth, int justify = JUSTIFY_LEFT, int wrap = WRAP_WORD, int draw = DRAW_NORMAL, rgb_t fgcolor = UI_TEXT_COLOR, rgb_t bgcolor = UI_TEXT_BG_COLOR, float *totalwidth = NULL, float *totalheight = NULL);
	void draw_text_box(const char *text, int justify, float xpos, float ypos, rgb_t backcolor);
	void highlight(float x0, float y0, float x1, float y1, rgb_t bgcolor);

	// input
	bool input_pop_event(ui_event &event);
	bool input_pressed(int key, int repeat = 0);
	bool find_mouse(float &mouse_x, float &mouse_y);
	bool find_mouse(float &mouse_x, float &mouse_y, bool &mouse_button);

private:
	// static variables
	static ui_stackable *menu_free;
	static bitmap_rgb32 *hilight_bitmap;
	static render_texture *hilight_texture;

	// instance variables
	running_machine &	m_machine;          // machine we are attached to
	bool				special_main_menu;

	// static methods
	static void exit(running_machine &machine);
	static void render_triangle(bitmap_argb32 &dest, bitmap_argb32 &source, const rectangle &sbounds, void *param);
};

#endif // __UI_STACKABLE_H__
