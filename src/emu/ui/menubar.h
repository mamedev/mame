/***************************************************************************

    menubar.h

    Internal MAME menu bar for the user interface.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __UI_MENUBAR_H__
#define __UI_MENUBAR_H__

#include "render.h"
#include "ui/stackable.h"


//**************************************************************************
//  MENU BAR
//**************************************************************************

class ui_menubar : public ui_stackable
{
public:
	ui_menubar(running_machine &machine, render_container *container);
	~ui_menubar();

	virtual void reset();
	virtual void do_handle();

protected:
	class menu_item
	{
	public:
		menu_item(const char *name = NULL, menu_item *parent = NULL);
		~menu_item();

		// methods
		menu_item &append(const char *name);
		bool is_child_of(menu_item *that) const;

		// accessors
		bool is_empty() const { return !m_first_child; }
		const astring &name() const { return m_name; }
		menu_item *parent() { return m_parent; }
		menu_item *child() { return m_first_child; }
		menu_item *last_child() { return m_last_child; }
		menu_item *previous() { return m_previous; }
		menu_item *next() { return m_next; }
		bool is_sub_menu() const { return m_parent && m_parent->m_parent; }
		void set_area(float x0, float y0, float x1, float y1);
		void clear_area() { set_area(-1, -1, -1, -1); }
		void clear_area_recursive();
		menu_item *find_point(float x, float y);

	private:
		astring			m_name;
		menu_item *		m_parent;
		menu_item *		m_first_child;
		menu_item *		m_last_child;
		menu_item *		m_previous;
		menu_item *		m_next;
		float			m_x0;
		float			m_y0;
		float			m_x1;
		float			m_y1;
	};

	// implemented by child classes
	virtual void build_menus() = 0;

	// accessors
	menu_item &root_menu() { return m_menus; }

private:
	// instance variables
	menu_item		m_menus;
	menu_item *		m_selected;
	float			m_mouse_x, m_mouse_y;

	// selection walking
	bool walk_selection_previous();
	bool walk_selection_next();
	bool walk_selection_child();
	bool walk_selection_parent();
	bool walk_selection_previous_peer();
	bool walk_selection_next_peer();

	// miscellaneous
	void draw_child_menu(menu_item *menu, float x, float y);
	void draw_menu_item_text(menu_item *mi, float x0, float y0, float x1, float y1);
};


#endif /* __UI_MENUBAR_H__ */