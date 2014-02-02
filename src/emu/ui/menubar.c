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
}


//-------------------------------------------------
//  menu_item::ctor
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
//  menu_item::append
//-------------------------------------------------

ui_menubar::menu_item *ui_menubar::menu_item::append(const char *name)
{
	menu_item *child = new menu_item(name, this);
	
	child->m_previous = m_last_child;
	
	if (m_last_child)
		m_last_child->m_next = child;
	else
		m_first_child = child;
	m_last_child = child;

	return child;
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
		if (m_selected && m_selected->is_child_of(mi))
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

			default:
				break;
		}
	}

	int code_previous_menu;
	int code_next_menu;
	int code_child_menu;
	int code_previous_peer;
	menu_item *current_menu;
	if (!m_selected || !m_selected->is_sub_menu())
	{
		// no pull down menu selected
		code_previous_menu = IPT_UI_LEFT;
		code_next_menu = IPT_UI_RIGHT;
		code_child_menu = IPT_UI_DOWN;
		code_previous_peer = IPT_INVALID;
		current_menu = &m_menus;
	}
	else
	{
		// pull down menu selected
		code_previous_menu = IPT_UI_UP;
		code_next_menu = IPT_UI_DOWN;
		code_child_menu = IPT_UI_RIGHT;
		code_previous_peer = IPT_UI_LEFT;
		current_menu = m_selected->parent();
	}

	if (input_pressed(code_previous_menu))
	{
		// select previous menu
		m_selected = !m_selected || !m_selected->previous()
			? current_menu->last_child()
			: m_selected->previous();
	}
	else if (input_pressed(code_next_menu))
	{
		// select next menu
		m_selected = !m_selected || !m_selected->next()
			? current_menu->child()
			: m_selected->next();
	}
	else if (input_pressed(code_child_menu) && m_selected && m_selected->child())
	{
		// enter child menu
		m_selected = m_selected->child();
	}
	else if (code_previous_peer != IPT_INVALID && input_pressed(code_previous_peer))
	{
		// go to the first child of the previous peer menu
		m_selected = m_selected->parent()->previous() ? m_selected->parent()->last_child() : m_selected->parent()->previous();
		if (m_selected->child())
			m_selected = m_selected->child();
	}

	else if (input_pressed(IPT_UI_CANCEL) && m_selected)
	{
		// exit menu
		m_selected = m_selected->parent();
	}
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


//-------------------------------------------------
//  do_handle
//-------------------------------------------------

void ui_menubar::build_menus()
{
	menu_item *file_menu = m_menus.append("File");
	file_menu->append("Pause");
	file_menu->append("Exit");

	menu_item *snapshot_menu = m_menus.append("Snapshot");
	snapshot_menu->append("Load snapshot...");
	snapshot_menu->append("Save snapshot...");

	menu_item *options_menu = m_menus.append("Options");
	menu_item *throttle_menu = options_menu->append("Throttle");
	throttle_menu->append("200%");
	throttle_menu->append("100%");
	throttle_menu->append("50%");
	throttle_menu->append("20%");
	throttle_menu->append("10%");
	throttle_menu->append("Unthrottled");

	menu_item *settings_menu = m_menus.append("Settings");
	settings_menu->append("Foo");

	menu_item *help_menu = m_menus.append("Help");
	help_menu->append("About...");
}
