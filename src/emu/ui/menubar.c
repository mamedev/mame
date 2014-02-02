/***************************************************************************

    menubar.h

    Internal MAME menu bar for the user interface.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#include "emu.h"
#include "ui/menubar.h"

//**************************************************************************
//  CONSTANTS
//**************************************************************************

#define	UI_MENU_COLOR			UI_RED_COLOR
#define UI_MENU_COLOR_SELECTED	UI_GREEN_COLOR



//**************************************************************************
//  MENU ITEMS
//**************************************************************************

//-------------------------------------------------
//  menu_item::ctor
//-------------------------------------------------

ui_menubar::menu_item::menu_item(const char *name, ui_menubar::menu_item *parent)
{
	if (name != NULL)
		m_name.cpy(name);
	m_parent = parent;
	m_first_child = NULL;
	m_last_child = NULL;
	m_previous = NULL;
	m_next = NULL;
	clear_area();
}


//-------------------------------------------------
//  menu_item::dtor
//-------------------------------------------------

ui_menubar::menu_item::~menu_item()
{
	menu_item *mi = m_first_child;
	while (mi)
	{
		menu_item *next = mi->m_next;
		delete mi;
		mi = next;
	}
}


//-------------------------------------------------
//  menu_item::set_area
//-------------------------------------------------

void ui_menubar::menu_item::set_area(float x0, float y0, float x1, float y1)
{
	m_x0 = x0;
	m_y0 = y0;
	m_x1 = x1;
	m_y1 = y1;
}


//-------------------------------------------------
//  menu_item::clear_area_recursive
//-------------------------------------------------

void ui_menubar::menu_item::clear_area_recursive()
{
	clear_area();
	if (m_first_child)
		m_first_child->clear_area_recursive();
	if (m_next)
		m_next->clear_area_recursive();
}


//-------------------------------------------------
//  menu_item::append
//-------------------------------------------------

ui_menubar::menu_item &ui_menubar::menu_item::append(const char *name)
{
	menu_item *child = new menu_item(name, this);
	
	child->m_previous = m_last_child;
	
	if (m_last_child)
		m_last_child->m_next = child;
	else
		m_first_child = child;
	m_last_child = child;

	return *child;
}


//-------------------------------------------------
//  menu_item::find_point
//-------------------------------------------------

ui_menubar::menu_item *ui_menubar::menu_item::find_point(float x, float y)
{
	menu_item *result = NULL;

	if ((x >= m_x0) && (y >= m_y0) && (x <= m_x1) && (y <= m_y1))
		result = this;

	if (!result && m_first_child)
		result = m_first_child->find_point(x, y);
	if (!result && m_next)
		result = m_next->find_point(x, y);
	return result;
}


//-------------------------------------------------
//  menu_item::is_child_of
//-------------------------------------------------

bool ui_menubar::menu_item::is_child_of(ui_menubar::menu_item *that) const
{
	for(menu_item *mi = m_parent; mi != NULL; mi = mi->m_parent)
	{
		if (mi == that)
			return true;
	}
	return false;
}


//**************************************************************************
//  MENUBAR IMPLEMENTATION
//**************************************************************************

//-------------------------------------------------
//  ctor
//-------------------------------------------------

ui_menubar::ui_menubar(running_machine &machine, render_container *container)
	: ui_stackable(machine, container)
{
	m_selected = NULL;
}


//-------------------------------------------------
//  dtor
//-------------------------------------------------

ui_menubar::~ui_menubar()
{
}


//-------------------------------------------------
//	reset
//-------------------------------------------------

void ui_menubar::reset()
{
}


//-------------------------------------------------
//  do_handle
//-------------------------------------------------

void ui_menubar::do_handle()
{
	if (m_menus.is_empty())
		build_menus();

	m_menus.clear_area_recursive();
	find_mouse(m_mouse_x, m_mouse_y);

	float text_height = get_line_height();
	float spacing = text_height / 10;
	float x = spacing;
	float y = spacing;

	for(menu_item *mi = m_menus.child(); mi != NULL; mi = mi->next())
	{
		float width = get_string_width(mi->name());

		draw_outlined_box(
			x,
			y,
			x + width + (spacing * 2),
			y + text_height + (spacing * 2),
			UI_BACKGROUND_COLOR);

		draw_menu_item_text(
			mi,
			x + spacing,
			y + spacing,
			x + spacing + width,
			y + spacing + text_height);

		// child menu open?
		if (m_selected && (m_selected->is_child_of(mi) || (m_selected == mi)))
			draw_child_menu(mi, x, y + text_height + (spacing * 3));

		x += width + spacing * 4;
	}

	// loop while we have interesting events
	bool stop = false;
	ui_event local_menu_event;
	while (input_pop_event(local_menu_event) && !stop)
	{
		switch (local_menu_event.event_type)
		{
			case UI_EVENT_CHAR:
				stop = true;
				break;

			case UI_EVENT_MOUSE_DOWN:
				{
					menu_item *mi = m_menus.find_point(m_mouse_x, m_mouse_y);
					if (mi)
					{
						m_selected = mi;
						return;
					}
				}
				break;

			default:
				break;
		}
	}

	int code_previous_menu;
	int code_next_menu;
	int code_child_menu = IPT_INVALID;
	int code_parent_menu = IPT_INVALID;
	int code_previous_peer = IPT_INVALID;
	int code_next_peer = IPT_INVALID;
	if (!m_selected || !m_selected->is_sub_menu())
	{
		// no pull down menu selected
		code_previous_menu = IPT_UI_LEFT;
		code_next_menu = IPT_UI_RIGHT;
		code_child_menu = IPT_UI_DOWN;
	}
	else
	{
		// pull down menu selected
		code_previous_menu = IPT_UI_UP;
		code_next_menu = IPT_UI_DOWN;
		if (m_selected->child())
			code_child_menu = IPT_UI_SELECT;
		code_previous_peer = IPT_UI_LEFT;
		code_next_peer = IPT_UI_RIGHT;
		if (m_selected->parent()->is_sub_menu())
			code_parent_menu = IPT_UI_LEFT;
	}

	if (input_pressed(code_previous_menu))
		walk_selection_previous();
	else if (input_pressed(code_next_menu))
		walk_selection_next();
	else if ((code_child_menu != IPT_INVALID) && input_pressed(code_child_menu))
		walk_selection_child();
	else if (input_pressed(IPT_UI_CANCEL) || (code_parent_menu != IPT_INVALID && input_pressed(code_parent_menu)))
		walk_selection_parent();
	else if (code_previous_peer != IPT_INVALID && input_pressed(code_previous_peer))
		walk_selection_previous_peer();
	else if (code_next_peer != IPT_INVALID && input_pressed(code_next_peer))
		walk_selection_next_peer();
}


//-------------------------------------------------
//  walk_selection_previous
//-------------------------------------------------

bool ui_menubar::walk_selection_previous()
{
	if (m_selected)
	{
		m_selected = m_selected->previous()
			? m_selected->previous()
			: m_selected->parent()->last_child();
	}
	else
	{
		m_selected = m_menus.last_child();
	}
	return true;
}


//-------------------------------------------------
//  walk_selection_next
//-------------------------------------------------

bool ui_menubar::walk_selection_next()
{
	if (m_selected)
	{
		m_selected = m_selected->next()
			? m_selected->next()
			: m_selected->parent()->child();
	}
	else
	{
		m_selected = m_menus.child();
	}
	return true;
}


//-------------------------------------------------
//  walk_selection_child
//-------------------------------------------------

bool ui_menubar::walk_selection_child()
{
	bool result = false;
	if (m_selected && m_selected->child())
	{
		m_selected = m_selected->child();
		result = true;
	}
	return result;
}


//-------------------------------------------------
//  walk_selection_parent
//-------------------------------------------------

bool ui_menubar::walk_selection_parent()
{
	bool result = false;
	if (m_selected && m_selected->parent() && m_selected->parent() != &m_menus)
	{
		m_selected = m_selected->parent();
		result = true;
	}
	return result;
}


//-------------------------------------------------
//  walk_selection_previous_peer
//-------------------------------------------------

bool ui_menubar::walk_selection_previous_peer()
{
	bool result = walk_selection_parent() && walk_selection_previous();
	if (result)
		walk_selection_child();
	return result;
}


//-------------------------------------------------
//  walk_selection_next_peer
//-------------------------------------------------

bool ui_menubar::walk_selection_next_peer()
{
	bool result = walk_selection_parent() && walk_selection_next();
	if (result)
		walk_selection_child();
	return result;
}


//-------------------------------------------------
//  draw_child_menu
//-------------------------------------------------

void ui_menubar::draw_child_menu(menu_item *menu, float x, float y)
{
	float text_height = get_line_height();
	float spacing = text_height / 10;

	// calculate the maximum width and menu item count
	float max_width = 0;
	int menu_item_count = 0;
	for(menu_item *mi = menu->child(); mi != NULL; mi = mi->next())
	{
		float width = get_string_width(mi->name());
		max_width = MAX(max_width, width);
		menu_item_count++;
	}

	// draw the menu outline
	draw_outlined_box(
		x,
		y,
		x + max_width + (spacing * 2),
		y + (text_height * menu_item_count) + (spacing * 2),
		UI_BACKGROUND_COLOR);

	// draw the individual items
	float mx = x;
	float my = y;
	for(menu_item *mi = menu->child(); mi != NULL; mi = mi->next())
	{
		draw_menu_item_text(
			mi,
			mx + spacing,
			my + spacing,
			mx + spacing + max_width,
			my + spacing + text_height);

		// child menu open?
		if (m_selected && m_selected->is_child_of(mi))
		{
			draw_child_menu(
				mi,
				x + max_width + (spacing * 2),
				y);
		}

		my += text_height;
	}
}


//-------------------------------------------------
//  draw_menu_item_text
//-------------------------------------------------

void ui_menubar::draw_menu_item_text(menu_item *mi, float x0, float y0, float x1, float y1)
{
	// set our area
	mi->set_area(x0, y0, x1, y1);

	// choose the color
	rgb_t fgcolor, bgcolor;
	if (mi == m_selected)
	{
		// selected
		fgcolor = UI_SELECTED_COLOR;
		bgcolor = UI_SELECTED_BG_COLOR;
	}
	else if ((m_mouse_x >= x0) && (m_mouse_x < x1) && (m_mouse_y >= y0) && (m_mouse_y < y1))
	{
		// hover
		fgcolor = UI_MOUSEOVER_COLOR;
		bgcolor = UI_MOUSEOVER_BG_COLOR;
	}
	else
	{
		// normal
		fgcolor = UI_TEXT_COLOR;
		bgcolor = UI_BACKGROUND_COLOR;
	}

	draw_text(mi->name(), x0, y0, fgcolor, bgcolor);
}
