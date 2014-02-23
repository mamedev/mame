/***************************************************************************

    menubar.c

    Internal MAME menu bar for the user interface.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#include "emu.h"
#include "ui/menubar.h"
#include "ui/ui.h"
#include "uiinput.h"


//**************************************************************************
//  CONSTANTS
//**************************************************************************

#define CHECKMARK				"x "
#define SEPARATOR_HEIGHT		0.25


//**************************************************************************
//  CLASSES
//**************************************************************************

class tabbed_text_iterator
{
public:
	tabbed_text_iterator(const char *text);
	bool next();
	int index() const { return m_index; }
	const char *current() { return m_current; }

private:
	astring			m_buffer;
	const char *	m_text;
	const char *	m_current;
	int				m_position;
	int				m_index;
};


//**************************************************************************
//  MENUBAR IMPLEMENTATION
//**************************************************************************

//-------------------------------------------------
//  ctor
//-------------------------------------------------

ui_menubar::ui_menubar(running_machine &machine)
	: m_machine(machine), m_menus(*this)
{
	m_container = NULL;
	m_shortcuted_menu_items = NULL;
	m_selected_item = NULL;
	m_active_item = NULL;
	m_dragged = false;
	m_checkmark_width = -1;
	m_mouse_x = -1;
	m_mouse_y = -1;
	m_mouse_button = false;
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
//  input_pressed_safe
//-------------------------------------------------

bool ui_menubar::input_pressed_safe(int key)
{
	return (key != IPT_INVALID) && ui_input_pressed(machine(), key);
}


//-------------------------------------------------
//  handle
//-------------------------------------------------

void ui_menubar::handle(render_container *current_container)
{
	m_container = current_container;

	// do we need to initialize the menus?
	if (m_menus.is_empty())
		menubar_build_menus();

	// measure standard string widths (if necessary)
	if (m_checkmark_width <= 0)
		m_checkmark_width = machine().ui().get_string_width(CHECKMARK);

	// reset screen locations of all menu items
	m_menus.clear_area_recursive();

	// calculate visibility of the menubar
	m_menubar_visibility = get_menubar_visibility();

	// draw conventional UI elements (e.g. - frameskip)
	menubar_draw_ui_elements();

	float text_height = machine().ui().get_line_height();
	float spacing = text_height / 10;
	float x = spacing;
	float y = spacing;

	for(menu_item *mi = m_menus.child(); mi != NULL; mi = mi->next())
	{
		float width = machine().ui().get_string_width(mi->text());

		machine().ui().draw_outlined_box(
			container(),
			x,
			y,
			x + width + (spacing * 2),
			y + text_height + (spacing * 2),
			adjust_color(UI_BORDER_COLOR),
			adjust_color(UI_BACKGROUND_COLOR));

		draw_menu_item_text(
			mi,
			x + spacing,
			y + spacing,
			x + spacing + width,
			y + spacing + text_height,
			false);

		// child menu open?
		if (is_child_menu_visible(mi))
			draw_child_menu(mi, x, y + text_height + (spacing * 3));

		x += width + spacing * 4;
	}

	// loop while we have interesting events
	while(!event_loop())
	{
	}
}


//-------------------------------------------------
//  event_loop
//-------------------------------------------------

bool ui_menubar::event_loop()
{
	bool done = false;

	ui_event local_menu_event;
	if (ui_input_pop_event(machine(), &local_menu_event))
	{
		// find the menu item we're pointing at
		find_mouse(m_mouse_x, m_mouse_y, m_mouse_button);
		menu_item *mi = m_menus.find_point(m_mouse_x, m_mouse_y);

		switch (local_menu_event.event_type)
		{
			case UI_EVENT_MOUSE_DOWN:
				if (mi != NULL)
				{
					m_selected_item = mi;
					m_active_item = mi;
					m_dragged = false;
				}
				break;

			case UI_EVENT_MOUSE_MOVE:
				// record the move
				m_last_mouse_move = osd_ticks();

				// moving is only interesting if we have an active menu selection
				if (m_mouse_button && m_active_item != NULL)
				{
					// are we changing the active menu item?
					if (m_active_item != mi)
					{
						if (mi != NULL)
							m_active_item = mi->has_children() ? mi->child() : mi;
						m_dragged = true;
						done = true;
					}

					// are we changing the selection?
					if (m_selected_item != mi)
					{
						m_selected_item = mi;
						done = true;
					}
				}
				break;

			case UI_EVENT_MOUSE_UP:
				m_active_item = NULL;
				if (m_selected_item && m_selected_item == mi)
				{
					// looks like we did a mouse up on the current selection; we
					// should invoke or expand the current selection (whichever
					// may be appropriate)
					if (m_selected_item->is_invokable())
						invoke(m_selected_item);
					else if (m_selected_item->has_children())
						m_active_item = m_selected_item->child();
				}
				else if (m_dragged)
				{
					m_selected_item = NULL;					
				}
				done = true;
				break;

			default:
				break;
		}
	}

	if (!done)
	{
		bool navigation_input_pressed = poll_navigation_keys();
		poll_shortcut_keys(navigation_input_pressed);
		done = true;
	}
	return done;
}


//-------------------------------------------------
//  poll_navigation_keys
//-------------------------------------------------

bool ui_menubar::poll_navigation_keys()
{
	int code_previous_menu = IPT_INVALID;
	int code_next_menu = IPT_INVALID;
	int code_child_menu = IPT_INVALID;
	int code_parent_menu = IPT_INVALID;
	int code_previous_peer = IPT_INVALID;
	int code_next_peer = IPT_INVALID;
	int code_selected = (m_selected_item && m_selected_item->is_invokable())
		? IPT_UI_SELECT
		: IPT_INVALID;

	// are we navigating the menu?
	if (m_selected_item != NULL)
	{
		// if so, are we in a pull down menu?
		if (!m_selected_item->is_sub_menu())
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
			if (m_selected_item->child())
				code_child_menu = IPT_UI_SELECT;
			code_previous_peer = IPT_UI_LEFT;
			code_next_peer = IPT_UI_RIGHT;
			if (m_selected_item->parent()->is_sub_menu())
				code_parent_menu = IPT_UI_LEFT;
		}
	}

	bool result = true;
	if (input_pressed_safe(code_previous_menu))
		result = walk_selection_previous();
	else if (input_pressed_safe(code_next_menu))
		result = walk_selection_next();
	else if (input_pressed_safe(code_child_menu))
		result = walk_selection_child();
	else if (input_pressed_safe(IPT_UI_CANCEL))
		result = walk_selection_escape();
	else if (input_pressed_safe(code_parent_menu))
		result = walk_selection_parent();
	else if (input_pressed_safe(code_previous_peer))
		result = walk_selection_previous_peer();
	else if (input_pressed_safe(code_next_peer))
		result = walk_selection_next_peer();
	else if (input_pressed_safe(IPT_UI_CONFIGURE))
		toggle_selection();
	else if (input_pressed_safe(code_selected))
		invoke(m_selected_item);
	else
		result = false;	// didn't do anything

	// if we changed something, set the active item accordingly
	if (result)
		m_active_item = m_selected_item;

	return result;
}


//-------------------------------------------------
//  poll_shortcut_keys
//-------------------------------------------------

bool ui_menubar::poll_shortcut_keys(bool swallow)
{
	// loop through all shortcut items
	for (menu_item *item = m_shortcuted_menu_items; item != NULL; item = item->next_with_shortcut())
	{
		assert(item->is_invokable());
		
		// did we press this shortcut?
		if (input_pressed_safe(item->shortcut()) && !swallow)
		{
			// this shortcut was pressed and we're not swallowing them; invoke it
			invoke(item);
			return true;
		}
	}
	return false;
}


//-------------------------------------------------
//  toggle_selection
//-------------------------------------------------

void ui_menubar::toggle_selection()
{
	m_selected_item = m_selected_item != NULL
		? NULL
		: m_menus.child();
}


//-------------------------------------------------
//  invoke
//-------------------------------------------------

void ui_menubar::invoke(menu_item *menu)
{
	// first, we're ending the menu; pop us off first
	machine().ui().set_handler(NULL, 0);

	// and invoke the selection
	menu->invoke();
}


//-------------------------------------------------
//  walk_selection_previous
//-------------------------------------------------

bool ui_menubar::walk_selection_previous()
{
	if (m_selected_item)
	{
		do
		{
			m_selected_item = m_selected_item->previous()
				? m_selected_item->previous()
				: m_selected_item->parent()->last_child();
		}
		while(!m_selected_item->is_enabled());
	}
	else
	{
		m_selected_item = m_menus.last_child();
	}
	return true;
}


//-------------------------------------------------
//  walk_selection_next
//-------------------------------------------------

bool ui_menubar::walk_selection_next()
{
	if (m_selected_item)
	{
		do
		{
			m_selected_item = m_selected_item->next()
				? m_selected_item->next()
				: m_selected_item->parent()->child();
		}
		while(!m_selected_item->is_enabled());
	}
	else
	{
		m_selected_item = m_menus.child();
	}
	return true;
}


//-------------------------------------------------
//  walk_selection_child
//-------------------------------------------------

bool ui_menubar::walk_selection_child()
{
	bool result = false;
	if (m_selected_item && m_selected_item->child())
	{
		m_selected_item = m_selected_item->child();
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
	if (m_selected_item && m_selected_item->parent() && m_selected_item->parent() != &m_menus)
	{
		m_selected_item = m_selected_item->parent();
		result = true;
	}
	return result;
}


//-------------------------------------------------
//  walk_selection_escape
//-------------------------------------------------

bool ui_menubar::walk_selection_escape()
{
	bool result = walk_selection_parent();

	if (!result && m_selected_item != NULL)
	{
		m_selected_item = NULL;
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
	float text_height = machine().ui().get_line_height();
	float separator_height = text_height * SEPARATOR_HEIGHT;
	float spacing = text_height / 10;

	// calculate the maximum width and menu item count
	float max_widths[4] = {0, };
	float total_height = (spacing * 2);
	float max_shortcuts_width = 0;
	for(menu_item *mi = menu->child(); mi != NULL; mi = mi->next())
	{
		// aggregate the maximum width for each column
		tabbed_text_iterator iter(mi->text());
		while(iter.next())
		{
			float width = machine().ui().get_string_width(iter.current());
			assert(iter.index() < ARRAY_LENGTH(max_widths));
			max_widths[iter.index()] = MAX(max_widths[iter.index()], width);
		}

		// measure the shortcut
		float shortcut_width = mi->shortcut_text_width();
		if (shortcut_width > 0)
			max_shortcuts_width = MAX(max_shortcuts_width, shortcut_width + spacing);

		// increase the height
		total_height += (mi->is_separator() ? separator_height : text_height);
	}

	// get the aggregate maximum widths across all columns
	float max_width = m_checkmark_width * 2 + max_shortcuts_width;
	for (int i = 0; i < ARRAY_LENGTH(max_widths); i++)
		max_width += max_widths[i];

	// draw the menu outline
	machine().ui().draw_outlined_box(
		container(),
		x,
		y,
		x + max_width + (spacing * 2),
		y + total_height,
		adjust_color(UI_BACKGROUND_COLOR));

	// draw the individual items
	float mx = x;
	float my = y;
	for(menu_item *mi = menu->child(); mi != NULL; mi = mi->next())
	{
		if (mi->is_separator())
		{
			// draw separator
			container()->add_line(
				mx,
				my + spacing + separator_height / 2,
				mx + max_width + (spacing * 2),
				my + spacing + separator_height / 2,
				separator_height / 8,
				adjust_color(UI_BORDER_COLOR),
				0);
		}
		else
		{
			// draw normal text
			draw_menu_item_text(
				mi,
				mx + spacing,
				my + spacing,
				mx + spacing + max_width,
				my + spacing + text_height,
				true,
				max_widths);

			// child menu open?
			if (is_child_menu_visible(mi))
			{
				draw_child_menu(
					mi,
					x + max_width + (spacing * 2),
					my);
			}
		}

		// move down...
		my += (mi->is_separator() ? separator_height : text_height);
	}
}


//-------------------------------------------------
//  is_child_menu_visible
//-------------------------------------------------

bool ui_menubar::is_child_menu_visible(menu_item *menu) const
{
	menu_item *current_menu = m_active_item ? m_active_item : m_selected_item;
	return current_menu && current_menu->is_child_of(menu);
}


//-------------------------------------------------
//  draw_menu_item_text
//-------------------------------------------------

void ui_menubar::draw_menu_item_text(menu_item *mi, float x0, float y0, float x1, float y1, bool decorations, const float *column_widths)
{
	// set our area
	mi->set_area(x0, y0, x1, y1);

	// choose the color
	rgb_t fgcolor, bgcolor;
	if (!mi->is_enabled())
	{
		// disabled
		fgcolor = UI_UNAVAILABLE_COLOR;
		bgcolor = UI_TEXT_BG_COLOR;
	}
	else if (mi == m_selected_item)
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
		bgcolor = UI_TEXT_BG_COLOR;
	}

	// highlight?
	if (bgcolor != UI_TEXT_BG_COLOR)
		ui_menu::highlight(container(), x0, y0, x1, y1, adjust_color(bgcolor));

	// do we have to draw additional decorations?
	if (decorations)
	{
		// account for the checkbox
		if (mi->is_checked())
			machine().ui().draw_text_full(container(), CHECKMARK, x0, y0, 1.0f - x0, JUSTIFY_LEFT, WRAP_WORD, DRAW_NORMAL, adjust_color(fgcolor), adjust_color(bgcolor));
		x0 += m_checkmark_width;

		// expanders?
		if (mi->child())
		{
			float lr_arrow_width = 0.4f * (y1 - y0) * machine().render().ui_aspect();
			ui_menu::draw_arrow(
				container(),
				x1 - lr_arrow_width,
				y0 + (0.1f * (y1 - y0)),
				x1,
				y0 + (0.9f * (y1 - y0)),
				adjust_color(fgcolor),
				ROT90);
		}

		// shortcut?
		machine().ui().draw_text_full(
			container(),
			mi->shortcut_text(),
			x0,
			y0,
			x1 - x0,
			JUSTIFY_RIGHT,
			WRAP_WORD,
			DRAW_NORMAL,
			adjust_color(fgcolor),
			adjust_color(bgcolor));
	}

	tabbed_text_iterator iter(mi->text());
	while(iter.next())
	{
		machine().ui().draw_text_full(container(), iter.current(), x0, y0, 1.0f - x0, JUSTIFY_LEFT, WRAP_WORD, DRAW_NORMAL, adjust_color(fgcolor), adjust_color(bgcolor));
		if (column_widths != NULL)
			x0 += column_widths[iter.index()];
	}
}


//-------------------------------------------------
//  find_mouse
//-------------------------------------------------

bool ui_menubar::find_mouse(float &mouse_x, float &mouse_y, bool &mouse_button)
{
	bool result = false;
	mouse_x = -1;
	mouse_y = -1;

	INT32 mouse_target_x, mouse_target_y;
	render_target *mouse_target = ui_input_find_mouse(machine(), &mouse_target_x, &mouse_target_y, &mouse_button);
	if (mouse_target != NULL)
	{
		if (mouse_target->map_point_container(mouse_target_x, mouse_target_y, *container(), mouse_x, mouse_y))
			result = true;
	}

	return result;
}


//-------------------------------------------------
//  get_menubar_visibility
//-------------------------------------------------

ui_menubar::menubar_visibility_t ui_menubar::get_menubar_visibility()
{
	menubar_visibility_t result;

	// is the mouse in the menu bar?
	bool in_menu_bar = m_mouse_y <= machine().ui().get_line_height();

	// did we recently move the mouse?
	bool recently_moved = (osd_ticks() - m_last_mouse_move) * 5 / osd_ticks_per_second() < 1;

	// make the choice
	if ((m_selected_item != NULL) || (m_active_item != NULL))
		result = MENUBAR_VISIBILITY_VISIBLE;
	else if (in_menu_bar || recently_moved)
		result = MENUBAR_VISIBILITY_TRANSLUCENT;
	else
		result = MENUBAR_VISIBILITY_INVISIBLE;

	return result;
}


//-------------------------------------------------
//  adjust_color
//-------------------------------------------------

rgb_t ui_menubar::adjust_color(rgb_t color)
{
	switch(m_menubar_visibility)
	{
		case MENUBAR_VISIBILITY_INVISIBLE:
			color = rgb_t(0, 0, 0, 0);
			break;

		case MENUBAR_VISIBILITY_TRANSLUCENT:
			color = rgb_t(
				color.a() / 4,
				color.r(),
				color.g(),
				color.b());
			break;
		
		case MENUBAR_VISIBILITY_VISIBLE:
		default:
			// do nothing
			break;
	}
	return color;
}


//**************************************************************************
//  MENU ITEMS
//**************************************************************************

//-------------------------------------------------
//  menu_item::ctor
//-------------------------------------------------

ui_menubar::menu_item::menu_item(ui_menubar &menubar, const char *text, ui_menubar::menu_item *parent, bool is_invokable, int shortcut)
	: m_menubar(menubar)
{
	if (text != NULL)
		m_text.cpy(text);
	m_is_invokable = is_invokable;
	m_parent = parent;
	m_first_child = NULL;
	m_last_child = NULL;
	m_previous = NULL;
	m_next = NULL;
	m_is_checked = false;
	m_is_enabled = true;
	m_is_separator = false;
	m_shortcut = shortcut;
	m_shortcut_text_width = -1;
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
//  menu_item::initialize
//-------------------------------------------------

void ui_menubar::menu_item::initialize(ui_menubar::menu_item &child)
{
	// link this back to the previous item
	child.m_previous = m_last_child;

	// link the end of the chain to this new item
	if (m_last_child)
		m_last_child->m_next = &child;
	else
		m_first_child = &child;

	// this new child is now last in the chain
	m_last_child = &child;

	// link this up into the shortcut list, if appropriate
	if (child.shortcut() != 0)
	{
		child.set_next_with_shortcut(m_menubar.m_shortcuted_menu_items);
		m_menubar.m_shortcuted_menu_items = &child;
	}
}


//-------------------------------------------------
//  menu_item::append
//-------------------------------------------------

ui_menubar::menu_item &ui_menubar::menu_item::append(const char *text)
{
	menu_item *child = new menu_item(m_menubar, text, this);
	initialize(*child);
	return *child;
}


//-------------------------------------------------
//  menu_item::append_separator
//-------------------------------------------------

void ui_menubar::menu_item::append_separator()
{
	menu_item &separator = append("-");
	separator.set_enabled(false);
	separator.m_is_separator = true;
}


//-------------------------------------------------
//  menu_item::find_point
//-------------------------------------------------

ui_menubar::menu_item *ui_menubar::menu_item::find_point(float x, float y)
{
	menu_item *result = NULL;

	if (m_is_enabled && (x >= m_x0) && (y >= m_y0) && (x <= m_x1) && (y <= m_y1))
		result = this;

	if (!result && m_first_child)
		result = m_first_child->find_point(x, y);
	if (!result && m_next)
		result = m_next->find_point(x, y);
	return result;
}


//-------------------------------------------------
//  menu_item::find_child
//-------------------------------------------------

ui_menubar::menu_item &ui_menubar::menu_item::find_child(const char *target)
{
	menu_item *item = find_child_internal(target);
	assert(item != NULL);
	return *item;
}


//-------------------------------------------------
//  menu_item::find_child_internal
//-------------------------------------------------

ui_menubar::menu_item *ui_menubar::menu_item::find_child_internal(const char *target)
{
	if (!strcmp(target, text()))
		return this;

	for(menu_item *item = child(); item != NULL; item = item->next())
	{
		menu_item *result = item->find_child_internal(target);
		if (result != NULL)
			return result;
	}
	return NULL;
}


//-------------------------------------------------
//  menu_item::shortcut_text
//-------------------------------------------------

const char *ui_menubar::menu_item::shortcut_text()
{	
	// do we have to calculate this stuff?
	if (m_shortcut_text_width < 0)
	{
		// clear the text
		m_shortcut_text.cpy("");

		// first, do we even have a shortcut?
		if (shortcut() != 0)
		{
			// iterate over the input ports and add menu items
			for (input_type_entry *entry = m_menubar.machine().ioport().first_type(); entry != NULL; entry = entry->next())
			{
				// add if we match the group and we have a valid name */
				if (entry->group() == IPG_UI_SHORTCUT && entry->name() != NULL && entry->name()[0] != 0 && entry->type() == shortcut())
				{
					const input_seq &seq = m_menubar.machine().ioport().type_seq(entry->type(), entry->player(), SEQ_TYPE_STANDARD);
					sensible_seq_name(m_shortcut_text, seq);
					break;
				}
			}
		}

		// finally calculate the text width
		m_shortcut_text_width = m_menubar.machine().ui().get_string_width(m_shortcut_text);
	}

	// return the text
	return m_shortcut_text;
}


//-------------------------------------------------
//  menu_item::shortcut_text_width
//-------------------------------------------------

float ui_menubar::menu_item::shortcut_text_width()
{
	// force a calculation, if we didn't have one
	shortcut_text();

	// and return the text width
	return m_shortcut_text_width;
}


//-------------------------------------------------
//  menu_item::sensible_seq_name
//-------------------------------------------------

void ui_menubar::menu_item::sensible_seq_name(astring &text, const input_seq &seq)
{
	// special case; we don't want 'None'
	if (seq[0] == input_seq::end_code)
		m_shortcut_text.cpy("");
	else
		m_menubar.machine().input().seq_name(m_shortcut_text, seq);
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


//-------------------------------------------------
//  menu_item::invoke
//-------------------------------------------------

void ui_menubar::menu_item::invoke()
{
	// do nothing
}


//**************************************************************************
//  TABBED TEXT ITERATOR
//**************************************************************************

//-------------------------------------------------
//  tabbed_text_iterator::ctor
//-------------------------------------------------

tabbed_text_iterator::tabbed_text_iterator(const char *text)
{
	m_text = text;
	m_current = NULL;
	m_position = 0;
	m_index = -1;
}


//-------------------------------------------------
//  tabbed_text_iterator::next
//-------------------------------------------------

bool tabbed_text_iterator::next()
{
	const char *current_text = &m_text[m_position];
	const char *tabpos = strchr(current_text, '\t');

	if (tabpos != NULL)
	{
		int count = tabpos - current_text;
		m_buffer.cpy(current_text, count);
		m_position += count + 1;
		m_current = m_buffer;
		m_index++;
	}
	else if (*current_text != '\0')
	{
		m_current = current_text;
		m_position += strlen(m_current);
		m_index++;
	}
	else
	{
		m_current = NULL;
	}
	return m_current != NULL;
}
