// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Nathan Woods, Maurizio Petrarota
/*********************************************************************

    ui/menu.cpp

    Internal MAME menus for the user interface.

*********************************************************************/

#include "emu.h"
#include "ui/menu.h"

#include "ui/ui.h"
#include "ui/mainmenu.h"
#include "ui/miscmenu.h"

#include "cheat.h"
#include "mame.h"

#include "corestr.h"
#include "drivenum.h"
#include "fileio.h"
#include "rendutil.h"
#include "uiinput.h"

#include "osdepend.h"

#include <cassert>
#include <cmath>
#include <cstdlib>
#include <limits>
#include <type_traits>


namespace ui {

/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

menu::global_state &menu::get_global_state(mame_ui_manager &ui)
{
	return ui.get_session_data<menu, global_state_wrapper>(ui);
}

//-------------------------------------------------
//  exclusive_input_pressed - return true if the
//  given key is pressed and we haven't already
//  reported a key
//-------------------------------------------------

bool menu::exclusive_input_pressed(int &iptkey, int key, int repeat)
{
	if ((iptkey == IPT_INVALID) && machine().ui_input().pressed_repeat(key, repeat))
	{
		iptkey = key;
		return true;
	}
	else
	{
		return false;
	}
}



/***************************************************************************
    CORE SYSTEM MANAGEMENT
***************************************************************************/

menu::global_state::global_state(mame_ui_manager &ui)
	: widgets_manager(ui.machine())
	, m_ui(ui)
	, m_bgrnd_bitmap()
	, m_bgrnd_texture(nullptr, ui.machine().render())
	, m_stack()
	, m_free()
	, m_hide(false)
	, m_current_pointer(-1)
	, m_pointer_type(ui_event::pointer::UNKNOWN)
	, m_pointer_buttons(0U)
	, m_pointer_x(-1.0F)
	, m_pointer_y(-1.0F)
	, m_pointer_hit(false)
{
	render_manager &render(ui.machine().render());

	// create a texture for main menu background
	m_bgrnd_texture.reset(render.texture_alloc(render_texture::hq_scale));
	if (ui.options().use_background_image() && (&ui.machine().system() == &GAME_NAME(___empty)))
	{
		m_bgrnd_bitmap = std::make_unique<bitmap_argb32>(0, 0);
		emu_file backgroundfile(".", OPEN_FLAG_READ);
		if (!backgroundfile.open("background.jpg"))
		{
			render_load_jpeg(*m_bgrnd_bitmap, backgroundfile);
			backgroundfile.close();
		}

		if (!m_bgrnd_bitmap->valid() && !backgroundfile.open("background.png"))
		{
			render_load_png(*m_bgrnd_bitmap, backgroundfile);
			backgroundfile.close();
		}

		if (m_bgrnd_bitmap->valid())
			m_bgrnd_texture->set_bitmap(*m_bgrnd_bitmap, m_bgrnd_bitmap->cliprect(), TEXFORMAT_ARGB32);
		else
			m_bgrnd_bitmap->reset();
	}
}


menu::global_state::~global_state()
{
	stack_reset();
	clear_free_list();
}


void menu::global_state::stack_push(std::unique_ptr<menu> &&menu)
{
	if (m_stack && m_stack->is_active())
	{
		m_stack->m_active = false;
		m_stack->menu_deactivated();
	}
	menu->m_parent = std::move(m_stack);
	m_stack = std::move(menu);

	ui_event uievt;
	while (m_stack->machine().ui_input().pop_event(&uievt))
	{
		switch (uievt.event_type)
		{
		case ui_event::type::POINTER_UPDATE:
		case ui_event::type::POINTER_LEAVE:
		case ui_event::type::POINTER_ABORT:
			use_pointer(m_stack->machine().render().ui_target(), m_stack->container(), uievt);
			break;
		default:
			break;
		}
	}
	m_stack->machine().ui_input().reset();
}


void menu::global_state::stack_pop()
{
	if (m_stack)
	{
		if (m_stack->is_one_shot())
			m_hide = true;
		if (m_stack->is_active())
		{
			m_stack->m_active = false;
			m_stack->menu_deactivated();
		}
		m_stack->menu_dismissed();
		std::unique_ptr<menu> menu(std::move(m_stack));
		m_stack = std::move(menu->m_parent);
		menu->m_parent = std::move(m_free);
		m_free = std::move(menu);

		ui_event uievt;
		while (m_free->machine().ui_input().pop_event(&uievt))
		{
			switch (uievt.event_type)
			{
			case ui_event::type::POINTER_UPDATE:
			case ui_event::type::POINTER_LEAVE:
			case ui_event::type::POINTER_ABORT:
				use_pointer(m_free->machine().render().ui_target(), m_free->container(), uievt);
				break;
			default:
				break;
			}
		}
		m_free->machine().ui_input().reset();
	}
}


void menu::global_state::stack_reset()
{
	while (m_stack)
		stack_pop();
}


void menu::global_state::clear_free_list()
{
	// free stack is in reverse order - unwind it properly
	std::unique_ptr<menu> reversed;
	while (m_free)
	{
		std::unique_ptr<menu> menu(std::move(m_free));
		m_free = std::move(menu->m_parent);
		menu->m_parent = std::move(reversed);
		reversed = std::move(menu);
	}
	while (reversed)
		reversed = std::move(reversed->m_parent);
}


bool menu::global_state::stack_has_special_main_menu() const
{
	for (auto menu = m_stack.get(); menu != nullptr; menu = menu->m_parent.get())
	{
		if (menu->is_special_main_menu())
			return true;
	}
	return false;
}


uint32_t menu::global_state::ui_handler(render_container &container)
{
	// if we have no menus stacked up, start with the main menu
	if (!m_stack)
		stack_push(std::make_unique<menu_main>(m_ui, container));

	while (true)
	{
		// ensure topmost menu is active - need a loop because it could push another menu
		while (m_stack && !m_stack->is_active())
		{
			m_stack->activate_menu();
			if (m_stack && m_stack->is_active())
			{
				// menu activated - draw it to ensure it's on-screen before it can process input
				m_stack->check_metrics();
				m_stack->do_rebuild();
				m_stack->validate_selection(1);
				m_stack->do_draw_menu();
				assert(m_stack);
				assert(m_stack->is_active());

				// display pointer if appropriate
				mame_ui_manager::display_pointer pointers[1]{ { m_stack->machine().render().ui_target(), m_pointer_type, m_pointer_x, m_pointer_y } };
				if ((0 <= m_current_pointer) && (ui_event::pointer::TOUCH != m_pointer_type))
					m_ui.set_pointers(std::begin(pointers), std::end(pointers));
				else
					m_ui.set_pointers(std::begin(pointers), std::begin(pointers));

				return mame_ui_manager::HANDLER_UPDATE;
			}
		}

		// update the menu state
		m_hide = false;
		bool need_update(m_stack && m_stack->do_handle());

		// clear up anything pending being released
		clear_free_list();

		// if the menus are to be hidden, return a cancel here
		if (m_ui.is_menu_active() && (m_hide || !m_stack))
		{
			if (m_stack)
			{
				if (m_stack->is_one_shot())
				{
					stack_pop();
				}
				else if (m_stack->is_active())
				{
					m_stack->m_active = false;
					m_stack->menu_deactivated();
				}
			}

			// forget about pointers while menus aren't handling events
			m_current_pointer = -1;
			m_pointer_type = ui_event::pointer::UNKNOWN;
			m_pointer_buttons = 0U;
			m_pointer_x = -1.0F;
			m_pointer_y = -1.0F;
			m_pointer_hit = false;

			return mame_ui_manager::HANDLER_CANCEL;
		}

		// if the menu is still active, draw it, otherwise try again
		if (m_stack->is_active())
		{
			m_stack->do_draw_menu();

			// display pointer if appropriate
			mame_ui_manager::display_pointer pointers[1]{ { m_stack->machine().render().ui_target(), m_pointer_type, m_pointer_x, m_pointer_y } };
			if ((0 <= m_current_pointer) && (ui_event::pointer::TOUCH != m_pointer_type))
				m_ui.set_pointers(std::begin(pointers), std::end(pointers));
			else
				m_ui.set_pointers(std::begin(pointers), std::begin(pointers));

			return need_update ? mame_ui_manager::HANDLER_UPDATE : 0;
		}
	}
}


std::pair<bool, bool> menu::global_state::use_pointer(render_target &target, render_container &container, ui_event const &uievt)
{
	if (&target != uievt.target)
		return std::make_pair(false, false);

	switch (uievt.event_type)
	{
	case ui_event::type::POINTER_UPDATE:
		// if it's our current pointer, just update it
		if (uievt.pointer_id == m_current_pointer)
		{
			assert(uievt.pointer_type == m_pointer_type);
			assert(uievt.pointer_buttons == ((m_pointer_buttons & ~uievt.pointer_released) | uievt.pointer_pressed));

			m_pointer_buttons = uievt.pointer_buttons;
			m_pointer_hit = target.map_point_container(
					uievt.pointer_x,
					uievt.pointer_y,
					container,
					m_pointer_x,
					m_pointer_y);
			return std::make_pair(true, false);
		}

		// don't change if the current pointer has buttons pressed and this one doesn't
		if ((0 > m_current_pointer) || (!m_pointer_buttons && (!m_pointer_hit || uievt.pointer_pressed)))
		{
			float x, y;
			bool const hit(target.map_point_container(uievt.pointer_x, uievt.pointer_y, container, x, y));
			if ((0 > m_current_pointer) || uievt.pointer_pressed || (!m_pointer_hit && hit))
			{
				m_current_pointer = uievt.pointer_id;
				m_pointer_type = uievt.pointer_type;
				m_pointer_buttons = uievt.pointer_buttons;
				m_pointer_x = x;
				m_pointer_y = y;
				m_pointer_hit = hit;
				return std::make_pair(true, true);
			}
		}

		// keep current pointer
		return std::make_pair(false, false);

	case ui_event::type::POINTER_LEAVE:
	case ui_event::type::POINTER_ABORT:
		// irrelevant if it isn't our current pointer
		if (uievt.pointer_id != m_current_pointer)
			return std::make_pair(false, false);

		assert(uievt.pointer_type == m_pointer_type);
		assert(uievt.pointer_released == m_pointer_buttons);

		// keep the coordinates where we lost the pointer
		m_current_pointer = -1;
		m_pointer_buttons = 0U;
		m_pointer_hit = target.map_point_container(
				uievt.pointer_x,
				uievt.pointer_y,
				container,
				m_pointer_x,
				m_pointer_y);
		return std::make_pair(true, false);

	default:
		std::abort();
	}
}



/***************************************************************************
    CORE MENU MANAGEMENT
***************************************************************************/

//-------------------------------------------------
//  menu - menu constructor
//-------------------------------------------------

menu::menu(mame_ui_manager &mui, render_container &container)
	: m_global_state(get_global_state(mui))
	, m_ui(mui)
	, m_container(container)
	, m_parent()
	, m_heading()
	, m_items()
	, m_rebuilding(false)
	, m_last_size(0, 0)
	, m_last_aspect(0.0F)
	, m_line_height(0.0F)
	, m_gutter_width(0.0F)
	, m_tb_border(0.0F)
	, m_lr_border(0.0F)
	, m_lr_arrow_width(0.0F)
	, m_ud_arrow_width(0.0F)
	, m_items_left(0.0F)
	, m_items_right(0.0F)
	, m_items_top(0.0F)
	, m_adjust_top(0.0F)
	, m_adjust_bottom(0.0F)
	, m_decrease_left(0.0F)
	, m_increase_left(0.0F)
	, m_show_up_arrow(false)
	, m_show_down_arrow(false)
	, m_items_drawn(false)
	, m_pointer_state(track_pointer::IDLE)
	, m_pointer_down(0.0F, 0.0F)
	, m_pointer_updated(0.0F, 0.0F)
	, m_pointer_line(0)
	, m_pointer_repeat(std::chrono::steady_clock::time_point::min())
	, m_accumulated_wheel(0)
	, m_process_flags(0)
	, m_selected(0)
	, m_special_main_menu(false)
	, m_one_shot(false)
	, m_needs_prev_menu_item(true)
	, m_active(false)
	, m_customtop(0.0F)
	, m_custombottom(0.0F)
	, m_resetpos(0)
	, m_resetref(nullptr)
{
	reset(reset_options::SELECT_FIRST);

	top_line = 0;
	m_visible_lines = 0;
	m_visible_items = 0;
}


//-------------------------------------------------
//  ~menu - menu destructor
//-------------------------------------------------

menu::~menu()
{
}


//-------------------------------------------------
//  reset - free all items in the menu
//-------------------------------------------------

void menu::reset(reset_options options)
{
	// don't accept pointer input until the menu has been redrawn
	m_items_drawn = false;
	m_pointer_state = track_pointer::IDLE;

	// based on the reset option, set the reset info
	m_resetpos = 0;
	m_resetref = nullptr;
	if (options == reset_options::REMEMBER_POSITION)
		m_resetpos = m_selected;
	else if (options == reset_options::REMEMBER_REF)
		m_resetref = get_selection_ref();

	// reset the item count back to 0
	m_items.clear();
	m_visible_items = 0;
	m_selected = 0;
}


//-------------------------------------------------
//  set_special_main_menu - set whether the
//  menu has special needs
//-------------------------------------------------

void menu::set_special_main_menu(bool special)
{
	m_special_main_menu = special;
}


//-------------------------------------------------
//  item_append - append a new item to the
//  end of the menu
//-------------------------------------------------

int menu::item_append(menu_item_type type, uint32_t flags)
{
	assert(menu_item_type::SEPARATOR == type);
	if (type == menu_item_type::SEPARATOR)
		return item_append(MENU_SEPARATOR_ITEM, flags, nullptr, menu_item_type::SEPARATOR);
	else
		return -1;
}

//-------------------------------------------------
//  item_append - append a new item to the
//  end of the menu
//-------------------------------------------------

int menu::item_append(std::string &&text, std::string &&subtext, uint32_t flags, void *ref, menu_item_type type)
{
	assert(m_rebuilding);

	// allocate a new item and populate it
	menu_item pitem(type, ref, flags);
	pitem.set_text(std::move(text));
	pitem.set_subtext(std::move(subtext));

	// append to array
	auto index = m_items.size();
	if (!m_items.empty() && m_needs_prev_menu_item)
	{
		m_items.emplace(m_items.end() - 1, std::move(pitem));
		--index;
	}
	else
	{
		m_items.emplace_back(std::move(pitem));
	}

	// update the selection if we need to
	if ((m_resetpos == index) || (m_resetref && (m_resetref == ref)))
		m_selected = index;
	if (m_resetpos == (m_items.size() - 1))
		m_selected = m_items.size() - 1;

	return int(std::make_signed_t<decltype(index)>(index));
}


//-------------------------------------------------
//  item_append_on_off - append a new "On"/"Off"
//  item to the end of the menu
//-------------------------------------------------

int menu::item_append_on_off(const std::string &text, bool state, uint32_t flags, void *ref, menu_item_type type)
{
	if (flags & FLAG_DISABLE)
		ref = nullptr;
	else
		flags |= state ? FLAG_LEFT_ARROW : FLAG_RIGHT_ARROW;

	return item_append(std::string(text), state ? _("On") : _("Off"), flags, ref, type);
}


//-------------------------------------------------
//  set_custom_space - set space required for
//  custom rendering above and below menu
//-------------------------------------------------

void menu::set_custom_space(float top, float bottom)
{
	m_customtop = top;
	m_custombottom = bottom;
}


//-------------------------------------------------
//  set_selection - changes the index
//  of the currently selected menu item
//-------------------------------------------------

void menu::set_selection(void *selected_itemref)
{
	m_selected = -1;
	for (int itemnum = 0; itemnum < m_items.size(); itemnum++)
	{
		if (m_items[itemnum].ref() == selected_itemref)
		{
			m_selected = itemnum;
			break;
		}
	}
}



/***************************************************************************
    INTERNAL MENU PROCESSING
***************************************************************************/

//-------------------------------------------------
//  do_draw_menu - draw a menu
//-------------------------------------------------

void menu::do_draw_menu()
{
	// if we're not running the emulation, draw parent menus in the background
	auto const draw_parent =
			[] (auto &self, menu *parent) -> bool
			{
				if (!parent || !(parent->is_special_main_menu() || self(self, parent->m_parent.get())))
					return false;
				else
					parent->draw(PROCESS_NOINPUT);
				return true;
			};
	if (draw_parent(draw_parent, m_parent.get()))
		container().add_rect(0.0F, 0.0F, 1.0F, 1.0F, rgb_t(114, 0, 0, 0), PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));

	// draw the menu proper
	draw(m_process_flags);
}


//-------------------------------------------------
//  draw - draw the menu itself
//-------------------------------------------------

void menu::draw(uint32_t flags)
{
	bool const customonly = (flags & PROCESS_CUSTOM_ONLY);
	float const max_width = 1.0F - ((lr_border() + (x_aspect() * UI_LINE_WIDTH)) * 2.0F);

	if (is_special_main_menu())
		draw_background();

	// compute the width and height of the full menu
	float visible_width = 0;
	float visible_main_menu_height = 0;
	for (auto const &pitem : m_items)
	{
		// compute width of left hand side
		float total_width = gutter_width() + get_string_width(pitem.text()) + gutter_width();

		// add in width of right hand side
		if (!pitem.subtext().empty())
			total_width += 2.0F * gutter_width() + get_string_width(pitem.subtext());
		else if (pitem.flags() & FLAG_UI_HEADING)
			total_width += 4.0F * ud_arrow_width();

		// track the maximum
		visible_width = std::max(total_width, visible_width);

		// track the height as well
		visible_main_menu_height += line_height();
	}

	// lay out the heading if present
	std::optional<text_layout> heading_layout;
	if (m_heading)
	{
		heading_layout.emplace(create_layout(max_width - (gutter_width() * 2.0F), text_layout::text_justify::CENTER));
		heading_layout->add_text(*m_heading, ui().colors().text_color());

		// readjust visible width if heading width exceeds that of the menu
		visible_width = std::max(gutter_width() + heading_layout->actual_width() + gutter_width(), visible_width);
	}

	// account for extra space at the top and bottom
	float const top_extra_menu_height = m_customtop + (heading_layout ? (heading_layout->actual_height() + (tb_border() * 3.0F)) : 0.0F);
	float const visible_extra_menu_height = top_extra_menu_height + m_custombottom;

	// add a little bit of slop for rounding
	visible_width += 0.01F;
	visible_main_menu_height += 0.01F;

	// if we are too wide or too tall, clamp it down
	visible_width = std::min(visible_width, max_width);

	// if the menu and extra menu won't fit, take away part of the regular menu, it will scroll
	if (visible_main_menu_height + visible_extra_menu_height + 2.0F * tb_border() > 1.0F)
		visible_main_menu_height = 1.0F - 2.0F * tb_border() - visible_extra_menu_height;

	m_visible_lines = std::min(int(std::floor(visible_main_menu_height / line_height())), int(unsigned(m_items.size())));
	visible_main_menu_height = float(m_visible_lines) * line_height();

	// compute top/left of inner menu area by centering
	float const visible_left = (1.0F - visible_width) * 0.5F;
	m_items_top = std::round((((1.0F - visible_main_menu_height - visible_extra_menu_height) * 0.5F) + top_extra_menu_height) * float(m_last_size.second)) / float(m_last_size.second);

	// first add us a box
	float const x1 = visible_left - lr_border();
	float const y1 = m_items_top - tb_border();
	float const x2 = visible_left + visible_width + lr_border();
	float const y2 = m_items_top + visible_main_menu_height + tb_border();
	if (!customonly)
	{
		if (heading_layout)
		{
			ui().draw_outlined_box(
					container(),
					x1, y1 - top_extra_menu_height,
					x2, y1 - m_customtop - tb_border(),
					UI_GREEN_COLOR);
			heading_layout->emit(container(), (1.0F - heading_layout->width()) * 0.5F, y1 - top_extra_menu_height + tb_border());
		}

		ui().draw_outlined_box(
				container(),
				x1, y1,
				x2, y2,
				ui().colors().background_color());
	}

	if ((m_selected >= (top_line + m_visible_lines)) || (m_selected < (top_line + 1)))
		top_line = m_selected - (m_visible_lines / 2);
	if (top_line < 0 || is_first_selected())
		top_line = 0;
	else if ((top_line > (m_items.size() - m_visible_lines)) || is_last_selected())
		top_line = m_items.size() - m_visible_lines;
	else if (m_selected >= (top_line + m_visible_lines - 2))
		top_line = m_selected - m_visible_lines + ((m_selected == (m_items.size() - 1)) ? 1: 2);

	// if scrolling, show arrows
	m_show_up_arrow = (m_items.size() > m_visible_lines) && !first_item_visible();
	m_show_down_arrow = (m_items.size() > m_visible_lines) && !last_item_visible();

	// set the number of visible lines, minus 1 for top arrow and 1 for bottom arrow
	m_visible_items = m_visible_lines - (m_show_up_arrow ? 1 : 0) - (m_show_down_arrow ? 1 : 0);

	// determine effective positions taking into account the hilighting arrows
	float const effective_width = visible_width - 2.0F * gutter_width();
	float const effective_left = visible_left + gutter_width();

	// loop over visible lines
	bool selected_subitem_too_big = false;
	m_items_left = x1 + 0.5F * UI_LINE_WIDTH;
	m_items_right = x2 - 0.5F * UI_LINE_WIDTH;
	if (customonly)
	{
		m_items_drawn = false;
		switch (m_pointer_state)
		{
		case track_pointer::IDLE:
		case track_pointer::IGNORED:
		case track_pointer::COMPLETED:
		case track_pointer::CUSTOM:
			break;
		case track_pointer::TRACK_LINE:
		case track_pointer::SCROLL:
		case track_pointer::ADJUST:
			m_pointer_state = track_pointer::COMPLETED;
		}
	}
	else
	{
		m_adjust_top = 1.0F;
		m_adjust_bottom = 0.0F;
		m_decrease_left = -1.0F;
		m_increase_left = -1.0F;
		m_items_drawn = true;
		for (int linenum = 0; linenum < m_visible_lines; linenum++)
		{
			auto const itemnum = top_line + linenum;
			menu_item const &pitem = m_items[itemnum];
			std::string_view const itemtext = pitem.text();
			rgb_t fgcolor = ui().colors().text_color();
			rgb_t bgcolor = ui().colors().text_bg_color();
			rgb_t fgcolor2 = ui().colors().subitem_color();
			rgb_t fgcolor3 = ui().colors().clone_color();
			float const line_y0 = m_items_top + (float(linenum) * line_height());
			float const line_y1 = line_y0 + line_height();

			// work out what we're dealing with
			bool const uparrow = !linenum && m_show_up_arrow;
			bool const downarrow = (linenum == (m_visible_lines - 1)) && m_show_down_arrow;

			// highlight if necessary
			if (is_selected(itemnum))
			{
				// if we're selected, draw with a different background
				fgcolor = fgcolor2 = fgcolor3 = ui().colors().selected_color();
				bgcolor = ui().colors().selected_bg_color();
			}
			else if (uparrow || downarrow || is_selectable(pitem))
			{
				bool pointerline(linenum == m_pointer_line);
				if ((track_pointer::ADJUST == m_pointer_state) && pointerline)
				{
					// use the hover background if an adjust gesture is attempted on an item that isn't selected
					fgcolor = fgcolor2 = fgcolor3 = ui().colors().mouseover_color();
					bgcolor = ui().colors().mouseover_bg_color();
				}
				else if (have_pointer() && pointer_in_rect(m_items_left, line_y0, m_items_right, line_y1))
				{
					if ((track_pointer::TRACK_LINE == m_pointer_state) && pointerline)
					{
						// use the selected background for an item being selected
						fgcolor = fgcolor2 = fgcolor3 = ui().colors().selected_color();
						bgcolor = ui().colors().selected_bg_color();
					}
					else if (track_pointer::IDLE == m_pointer_state)
					{
						// else if the pointer is over this item, draw with a different background
						fgcolor = fgcolor2 = fgcolor3 = ui().colors().mouseover_color();
						bgcolor = ui().colors().mouseover_bg_color();
					}
				}
				else if ((track_pointer::TRACK_LINE == m_pointer_state) && pointerline)
				{
					// use the hover background if the pointer moved out of the tracked item
					fgcolor = fgcolor2 = fgcolor3 = ui().colors().mouseover_color();
					bgcolor = ui().colors().mouseover_bg_color();
				}
			}

			// if we have some background hilighting to do, add a quad behind everything else
			if (bgcolor != ui().colors().text_bg_color())
				highlight(m_items_left, line_y0, m_items_right, line_y1, bgcolor);

			if (uparrow || downarrow)
			{
				// if we're on the top or bottom line, display the up or down arrow
				draw_arrow(
						0.5F * (x1 + x2 - ud_arrow_width()),
						line_y0 + (0.25F * line_height()),
						0.5F * (x1 + x2 + ud_arrow_width()),
						line_y0 + (0.75F * line_height()),
						fgcolor,
						downarrow ? (ROT0 ^ ORIENTATION_FLIP_Y) : ROT0);
			}
			else if (pitem.type() == menu_item_type::SEPARATOR)
			{
				// if we're just a divider, draw a line
				container().add_line(visible_left, line_y0 + 0.5F * line_height(), visible_left + visible_width, line_y0 + 0.5F * line_height(), UI_LINE_WIDTH, ui().colors().border_color(), PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
			}
			else if (pitem.subtext().empty())
			{
				// if we don't have a subitem, just draw the string centered
				if (pitem.flags() & FLAG_UI_HEADING)
				{
					float heading_width = get_string_width(itemtext);
					container().add_line(visible_left, line_y0 + 0.5F * line_height(), visible_left + ((visible_width - heading_width) / 2) - lr_border(), line_y0 + 0.5F * line_height(), UI_LINE_WIDTH, ui().colors().border_color(), PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
					container().add_line(visible_left + visible_width - ((visible_width - heading_width) / 2) + lr_border(), line_y0 + 0.5F * line_height(), visible_left + visible_width, line_y0 + 0.5F * line_height(), UI_LINE_WIDTH, ui().colors().border_color(), PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
				}
				ui().draw_text_full(
						container(),
						itemtext,
						effective_left, line_y0, effective_width,
						text_layout::text_justify::CENTER, text_layout::word_wrapping::TRUNCATE,
						mame_ui_manager::NORMAL, fgcolor, bgcolor,
						nullptr, nullptr,
						line_height());
			}
			else
			{
				// otherwise, draw the item on the left and the subitem text on the right
				bool const subitem_invert(pitem.flags() & FLAG_INVERT);
				float item_width, subitem_width;

				// draw the left-side text
				ui().draw_text_full(
						container(),
						itemtext,
						effective_left, line_y0, effective_width,
						text_layout::text_justify::LEFT, text_layout::word_wrapping::TRUNCATE,
						mame_ui_manager::NORMAL, fgcolor, bgcolor,
						&item_width, nullptr,
						line_height());

				if (pitem.flags() & FLAG_COLOR_BOX)
				{
					rgb_t color = rgb_t((uint32_t)strtoul(pitem.subtext().c_str(), nullptr, 16));

					// give 2 spaces worth of padding
					subitem_width = get_string_width("FF00FF00");

					ui().draw_outlined_box(
							container(),
							effective_left + effective_width - subitem_width, line_y0 + (UI_LINE_WIDTH * 2.0F),
							effective_left + effective_width, line_y1 - (UI_LINE_WIDTH * 2.0F),
							color);
				}
				else
				{
					std::string_view subitem_text(pitem.subtext());

					// give 2 spaces worth of padding
					item_width += 2.0F * gutter_width();

					// if the subitem doesn't fit here, display dots
					if (get_string_width(subitem_text) > effective_width - item_width)
					{
						subitem_text = "...";
						if (is_selected(itemnum))
							selected_subitem_too_big = true;
					}

					// customize subitem text color
					if (!core_stricmp(pitem.subtext(), _("On")))
						fgcolor2 = rgb_t(0x00,0xff,0x00);

					if (!core_stricmp(pitem.subtext(), _("Off")))
						fgcolor2 = rgb_t(0xff,0x00,0x00);

					if (!core_stricmp(pitem.subtext(), _("Auto")))
						fgcolor2 = rgb_t(0xff,0xff,0x00);

					// draw the subitem right-justified
					ui().draw_text_full(
							container(),
							subitem_text,
							effective_left + item_width, line_y0, effective_width - item_width,
							text_layout::text_justify::RIGHT, text_layout::word_wrapping::TRUNCATE,
							mame_ui_manager::NORMAL, subitem_invert ? fgcolor3 : fgcolor2, bgcolor,
							&subitem_width, nullptr,
							line_height());
				}

				// apply arrows
				if (is_selected(itemnum) && (pitem.flags() & (FLAG_LEFT_ARROW | FLAG_RIGHT_ARROW)))
				{
					m_adjust_top = line_y0 + (0.1F * line_height());
					m_adjust_bottom = line_y0 + (0.9F * line_height());
					if (pitem.flags() & FLAG_LEFT_ARROW)
					{
						m_decrease_left = effective_left + effective_width - subitem_width - gutter_width();
						float const r = m_decrease_left + lr_arrow_width();
						draw_arrow(m_decrease_left, m_adjust_top, r, m_adjust_bottom, fgcolor, ROT90 ^ ORIENTATION_FLIP_X);
					}
					if (pitem.flags() & FLAG_RIGHT_ARROW)
					{
						float const r = effective_left + effective_width + gutter_width();
						m_increase_left = r - lr_arrow_width();
						draw_arrow(m_increase_left, m_adjust_top, r, m_adjust_bottom, fgcolor, ROT90);
					}
				}
			}
		}
	}

	// if the selected subitem is too big, display it in a separate offset box
	if (selected_subitem_too_big)
	{
		menu_item const &pitem = selected_item();
		bool const subitem_invert(pitem.flags() & FLAG_INVERT);
		auto const linenum = m_selected - top_line;
		float const line_y = m_items_top + float(linenum) * line_height();

		// compute the multi-line target width/height
		auto const [target_width, target_height] = get_text_dimensions(
				pitem.subtext(),
				0, 0, visible_width * 0.75F,
				text_layout::text_justify::RIGHT, text_layout::word_wrapping::WORD);

		// determine the target location
		float const target_x = visible_left + visible_width - target_width - lr_border();
		float target_y = line_y + line_height() + tb_border();
		if (target_y + target_height + tb_border() > visible_main_menu_height)
			target_y = line_y - target_height - tb_border();

		// add a box around that
		ui().draw_outlined_box(
				container(),
				target_x - lr_border(), target_y - tb_border(),
				target_x + target_width + lr_border(), target_y + target_height + tb_border(),
				subitem_invert ? ui().colors().selected_bg_color() : ui().colors().background_color());

		ui().draw_text_full(
				container(),
				pitem.subtext(),
				target_x, target_y, target_width,
				text_layout::text_justify::RIGHT, text_layout::word_wrapping::WORD,
				mame_ui_manager::NORMAL, ui().colors().selected_color(), ui().colors().selected_bg_color(),
				nullptr, nullptr);
	}

	// if there is something special to add, do it by calling the virtual method
	custom_render(flags, get_selection_ref(), m_customtop, m_custombottom, x1, y1, x2, y2);
}

void menu::recompute_metrics(uint32_t width, uint32_t height, float aspect)
{
	float const ui_line_height = ui().get_line_height();

	// force whole pixels for line height, gutters and borders
	m_line_height = std::floor(ui_line_height * float(height)) / float(height);
	m_gutter_width = std::floor(0.5F * ui_line_height * aspect * float(width)) / float(width);
	m_tb_border = std::floor(ui().box_tb_border() * float(height)) / float(height);
	m_lr_border = std::floor(ui().box_lr_border() * aspect * float(width)) / float(width);

	m_lr_arrow_width = 0.4F * m_line_height * aspect;
	m_ud_arrow_width = m_line_height * aspect;

	// don't accept pointer input until the menu has been redrawn
	m_items_drawn = false;
	m_pointer_state = track_pointer::IDLE;

}

void menu::custom_render(uint32_t flags, void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
}


//-------------------------------------------------
//  handle_events - generically handle
//  input events for a menu
//-------------------------------------------------

bool menu::handle_events(uint32_t flags, event &ev)
{
	bool need_update = false;
	bool stop = false;
	ui_event local_menu_event;

	// loop while we have interesting events
	while (!stop && machine().ui_input().pop_event(&local_menu_event))
	{
		switch (local_menu_event.event_type)
		{
		// deal with pointer-like input (mouse, pen, touch, etc.)
		case ui_event::type::POINTER_UPDATE:
			{
				auto const [key, redraw] = handle_pointer_update(flags, local_menu_event);
				need_update = need_update || redraw;
				if (IPT_INVALID != key)
				{
					ev.iptkey = key;
					stop = true;
				}
			}
			break;

		// pointer left the normal way, possibly releasing buttons
		case ui_event::type::POINTER_LEAVE:
			{
				auto const [key, redraw] = handle_pointer_leave(flags, local_menu_event);
				need_update = need_update || redraw;
				if (IPT_INVALID != key)
				{
					ev.iptkey = key;
					stop = true;
				}
			}
			break;

		// pointer left in some abnormal way - cancel any associated actions
		case ui_event::type::POINTER_ABORT:
			{
				auto const [key, redraw] = handle_pointer_abort(flags, local_menu_event);
				need_update = need_update || redraw;
				if (IPT_INVALID != key)
				{
					ev.iptkey = key;
					stop = true;
				}
			}
			break;

		// caught scroll event
		case ui_event::type::MOUSE_WHEEL:
			if ((track_pointer::IDLE == m_pointer_state) || (track_pointer::IGNORED == m_pointer_state))
			{
				// the value is scaled to 120 units per "click"
				m_accumulated_wheel += local_menu_event.zdelta * local_menu_event.num_lines;
				int const lines((m_accumulated_wheel + ((0 < local_menu_event.zdelta) ? 36 : -36)) / 120);
				if (!lines)
					break;
				m_accumulated_wheel -= lines * 120;

				if (!custom_mouse_scroll(-lines) && !(flags & (PROCESS_ONLYCHAR | PROCESS_CUSTOM_NAV)))
				{
					if (lines > 0)
					{
						if (is_first_selected())
						{
							select_last_item();
						}
						else
						{
							m_selected -= lines;
							validate_selection(-1);
						}
						top_line -= (m_selected <= top_line && top_line != 0);
						if (m_selected <= top_line && m_visible_items != m_visible_lines)
							top_line -= lines;
					}
					else
					{
						if (is_last_selected())
						{
							select_first_item();
						}
						else
						{
							m_selected -= lines;
							validate_selection(1);
						}
						top_line += (m_selected >= top_line + m_visible_items + (top_line != 0));
						if (m_selected >= (top_line + m_visible_items + (top_line != 0)))
							top_line -= lines;
					}
				}
			}
			break;

		// translate CHAR events into specials
		case ui_event::type::IME_CHAR:
			if ((track_pointer::IDLE == m_pointer_state) || (track_pointer::IGNORED == m_pointer_state))
			{
				ev.iptkey = IPT_SPECIAL;
				ev.unichar = local_menu_event.ch;
				stop = true;
			}
			break;

		// ignore everything else
		default:
			break;
		}
	}

	// deal with repeating scroll arrows
	if ((track_pointer::TRACK_LINE == m_pointer_state) && ((!m_pointer_line && m_show_up_arrow) || ((m_pointer_line == (m_visible_lines - 1)) && m_show_down_arrow)))
	{
		float const linetop(m_items_top + (float(m_pointer_line) * line_height()));
		float const linebottom(m_items_top + (float(m_pointer_line + 1) * line_height()));
		if (pointer_in_rect(m_items_left, linetop, m_items_right, linebottom))
		{
			if (std::chrono::steady_clock::now() >= m_pointer_repeat)
			{
				if (!m_pointer_line)
				{
					// scroll up
					assert(0 < top_line);
					--top_line;
					if (!top_line)
						m_pointer_state = track_pointer::COMPLETED;
				}
				else
				{
					// scroll down
					assert(m_items.size() > (top_line + m_visible_lines));
					++top_line;
					if (m_items.size() == (top_line + m_visible_lines))
						m_pointer_state = track_pointer::COMPLETED;
				}
				force_visible_selection();
				need_update = true;
				m_pointer_repeat += std::chrono::milliseconds(100);
			}
		}
	}

	return need_update;
}


//-------------------------------------------------
//  handle_pointer_update - handle a regular
//  pointer update
//-------------------------------------------------

std::pair<int, bool> menu::handle_pointer_update(uint32_t flags, ui_event const &uievt)
{
	// decide whether to make this our current pointer
	render_target &target(machine().render().ui_target());
	auto const [ours, changed] = m_global_state.use_pointer(target, container(), uievt);
	if (!ours)
	{
		return std::make_pair(IPT_INVALID, false);
	}
	else if (changed)
	{
		// if the active pointer changed, ignore if any buttons were already down
		if (uievt.pointer_buttons != uievt.pointer_pressed)
		{
			m_pointer_state = track_pointer::IGNORED;
			return std::make_pair(IPT_INVALID, false);
		}
		else
		{
			m_pointer_state = track_pointer::IDLE;
		}
	}
	else if ((track_pointer::IGNORED == m_pointer_state) || (track_pointer::COMPLETED == m_pointer_state))
	{
		// stop ignoring the pointer if all buttons were released
		if (uievt.pointer_buttons == uievt.pointer_pressed)
			m_pointer_state = track_pointer::IDLE;
		else
			return std::make_pair(IPT_INVALID, false);
	}

	// give derived class a chance to handle it
	if ((track_pointer::IDLE == m_pointer_state) || (track_pointer::CUSTOM == m_pointer_state))
	{
		auto const [key, take, redraw] = custom_pointer_updated(changed, uievt);
		if (take)
		{
			m_pointer_state = track_pointer::CUSTOM;
			return std::make_pair(key, redraw);
		}
		else if (track_pointer::CUSTOM == m_pointer_state)
		{
			if (uievt.pointer_buttons)
			{
				m_pointer_state = track_pointer::COMPLETED;
				return std::make_pair(key, redraw);
			}
			else
			{
				m_pointer_state = track_pointer::IDLE;
			}
		}

		if (IPT_INVALID != key)
			return std::make_pair(key, redraw);
	}

	// ignore altogether if menu hasn't been drawn or flags say so
	if (!m_items_drawn || (flags & (PROCESS_CUSTOM_ONLY | PROCESS_ONLYCHAR)))
	{
		if (uievt.pointer_pressed)
		{
			if (track_pointer::IDLE == m_pointer_state)
				m_pointer_state = track_pointer::IGNORED;
		}
		else if (!uievt.pointer_buttons)
		{
			if ((track_pointer::IGNORED == m_pointer_state) || (track_pointer::COMPLETED == m_pointer_state))
				m_pointer_state = track_pointer::IDLE;
		}
		return std::make_pair(IPT_INVALID, false);
	}

	switch (m_pointer_state)
	{
	case track_pointer::IDLE:
		// ignore anything other than left click for now
		if ((uievt.pointer_pressed & 0x01) && !(uievt.pointer_buttons & ~u32(0x1)))
			return handle_primary_down(flags, uievt);
		else if (uievt.pointer_pressed)
			m_pointer_state = track_pointer::IGNORED;
		break;

	case track_pointer::IGNORED:
	case track_pointer::COMPLETED:
	case track_pointer::CUSTOM:
		std::abort(); // won't get here - handled earlier

	case track_pointer::TRACK_LINE:
		{
			auto const result(update_line_click(uievt));

			// treat anything else being pressed as cancelling the click sequence
			if (uievt.pointer_buttons & ~u32(0x01))
				m_pointer_state = track_pointer::COMPLETED;
			else if (!uievt.pointer_buttons)
				m_pointer_state = track_pointer::IDLE;

			return result;
		}

	case track_pointer::SCROLL:
		{
			bool const redraw(update_drag_scroll(uievt));

			// treat anything else being pressed as cancelling the drag
			if (uievt.pointer_buttons & ~u32(0x01))
				m_pointer_state = track_pointer::COMPLETED;
			else if (!uievt.pointer_buttons)
				m_pointer_state = track_pointer::IDLE;

			return std::make_pair(IPT_INVALID, redraw);
		}

	case track_pointer::ADJUST:
		{
			auto const result(update_drag_adjust(uievt));

			// treat anything else being pressed as cancelling the drag
			if (uievt.pointer_buttons & ~u32(0x01))
				m_pointer_state = track_pointer::COMPLETED;
			else if (!uievt.pointer_buttons)
				m_pointer_state = track_pointer::IDLE;

			return result;
		}
	}

	return std::make_pair(IPT_INVALID, false);
}


//-------------------------------------------------
//  handle_pointer_leave - handle a pointer
//  leaving the window
//-------------------------------------------------

std::pair<int, bool> menu::handle_pointer_leave(uint32_t flags, ui_event const &uievt)
{
	// ignore pointer input in windows other than the one that displays the UI
	render_target &target(machine().render().ui_target());
	auto const [ours, changed] = m_global_state.use_pointer(target, container(), uievt);
	assert(!changed);
	if (!ours)
		return std::make_pair(IPT_INVALID, false);

	int key(IPT_INVALID);
	bool redraw(false);
	switch (m_pointer_state)
	{
	case track_pointer::IDLE:
	case track_pointer::CUSTOM:
		std::tie(key, std::ignore, redraw) = custom_pointer_updated(changed, uievt);
		break;

	case track_pointer::IGNORED:
	case track_pointer::COMPLETED:
		break; // nothing to do

	case track_pointer::TRACK_LINE:
		std::tie(key, redraw) = update_line_click(uievt);
		break;

	case track_pointer::SCROLL:
		redraw = update_drag_scroll(uievt);
		break;

	case track_pointer::ADJUST:
		std::tie(key, redraw) = update_drag_adjust(uievt);
		break;
	}

	m_pointer_state = track_pointer::IDLE;
	return std::make_pair(key, redraw);
}


//-------------------------------------------------
//  handle_pointer_abort - handle a pointer
//  leaving in an abnormal way
//-------------------------------------------------

std::pair<int, bool> menu::handle_pointer_abort(uint32_t flags, ui_event const &uievt)
{
	// ignore pointer input in windows other than the one that displays the UI
	render_target &target(machine().render().ui_target());
	auto const [ours, changed] = m_global_state.use_pointer(target, container(), uievt);
	assert(!changed);
	if (!ours)
		return std::make_pair(IPT_INVALID, false);

	int key(IPT_INVALID);
	bool redraw(false);
	if (track_pointer::CUSTOM == m_pointer_state)
		std::tie(key, std::ignore, redraw) = custom_pointer_updated(false, uievt);
	else if (track_pointer::TRACK_LINE == m_pointer_state)
		redraw = true;
	m_pointer_state = track_pointer::IDLE;
	return std::make_pair(key, redraw);
}


//-------------------------------------------------
//  handle_primary_down - handle the primary
//  action for a pointer device
//-------------------------------------------------

std::pair<int, bool> menu::handle_primary_down(uint32_t flags, ui_event const &uievt)
{
	// we handle touch differently to mouse or pen
	bool const is_touch(ui_event::pointer::TOUCH == uievt.pointer_type);
	auto const [x, y] = pointer_location(); // FIXME: need starting location for multi-click actions

	// check increase/decrease arrows first
	// FIXME: should repeat if appropriate
	if (!is_touch && (y >= m_adjust_top) && (y < m_adjust_bottom))
	{
		if ((x >= m_decrease_left) && (x < (m_decrease_left + lr_arrow_width())))
		{
			m_pointer_state = track_pointer::COMPLETED;
			return std::make_pair(IPT_UI_LEFT, false);
		}
		else if ((x >= m_increase_left) && (x < (m_increase_left + lr_arrow_width())))
		{
			m_pointer_state = track_pointer::COMPLETED;
			return std::make_pair(IPT_UI_RIGHT, false);
		}
	}

	// work out if we’re pointing at an item
	if ((x < m_items_left) || (x >= m_items_right) || (y < m_items_top) || (y >= (m_items_top + (float(m_visible_lines) * line_height()))))
	{
		m_pointer_state = track_pointer::IGNORED;
		return std::make_pair(IPT_INVALID, false);
	}
	auto const lineno(int((y - m_items_top) / line_height()));
	assert(lineno >= 0);
	assert(lineno < m_visible_lines);

	// map to an action
	if (!lineno && m_show_up_arrow)
	{
		// scroll up
		assert(0 < top_line);
		--top_line;
		force_visible_selection();
		if (top_line)
		{
			m_pointer_state = track_pointer::TRACK_LINE;
			m_pointer_down = std::make_pair(x, y);
			m_pointer_updated = m_pointer_down;
			m_pointer_line = lineno;
			m_pointer_repeat = std::chrono::steady_clock::now() + std::chrono::milliseconds(300);
		}
		else
		{
			m_pointer_state = track_pointer::COMPLETED;
		}
		return std::make_pair(IPT_INVALID, true);
	}
	else if ((lineno == (m_visible_lines - 1)) && m_show_down_arrow)
	{
		// scroll down
		assert(m_items.size() > (top_line + m_visible_lines));
		++top_line;
		force_visible_selection();
		if (m_items.size() > (top_line + m_visible_lines))
		{
			m_pointer_state = track_pointer::TRACK_LINE;
			m_pointer_down = std::make_pair(x, y);
			m_pointer_updated = m_pointer_down;
			m_pointer_line = lineno;
			m_pointer_repeat = std::chrono::steady_clock::now() + std::chrono::milliseconds(300);
		}
		else
		{
			m_pointer_state = track_pointer::COMPLETED;
		}
		return std::make_pair(IPT_INVALID, true);
	}
	else
	{
		m_pointer_state = track_pointer::TRACK_LINE;
		m_pointer_down = std::make_pair(x, y);
		m_pointer_updated = m_pointer_down;
		m_pointer_line = lineno;

		int const itemno(lineno + top_line);
		assert(itemno >= 0);
		assert(itemno < m_items.size());
		return std::make_pair(IPT_INVALID, is_selectable(m_items[itemno]));
	}

	// nothing to do
	m_pointer_state = track_pointer::IGNORED;
	return std::make_pair(IPT_INVALID, false);
}


//-------------------------------------------------
//  update_line_click - track pointer after
//  clicking a menu line
//-------------------------------------------------

std::pair<int, bool> menu::update_line_click(ui_event const &uievt)
{
	assert(track_pointer::TRACK_LINE == m_pointer_state);
	assert((uievt.pointer_buttons | uievt.pointer_released) & 0x01);

	// arrows should scroll while held
	if ((!m_pointer_line && m_show_up_arrow) || ((m_pointer_line == (m_visible_lines - 1)) && m_show_down_arrow))
	{
		// check for re-entry
		bool redraw(false);
		auto const [x, y] = pointer_location();
		float const linetop(m_items_top + (float(m_pointer_line) * line_height()));
		float const linebottom(m_items_top + (float(m_pointer_line + 1) * line_height()));
		bool const reentered(reentered_rect(m_pointer_updated.first, m_pointer_updated.second, x, y, m_items_left, linetop, m_items_right, linebottom));
		if (reentered)
		{
			auto const now(std::chrono::steady_clock::now());
			if (now >= m_pointer_repeat)
			{
				if (!m_pointer_line)
				{
					// scroll up
					assert(0 < top_line);
					--top_line;
					if (!top_line)
						m_pointer_state = track_pointer::COMPLETED;
				}
				else
				{
					// scroll down
					assert(m_items.size() > (top_line + m_visible_lines));
					++top_line;
					if (m_items.size() == (top_line + m_visible_lines))
						m_pointer_state = track_pointer::COMPLETED;
				}
				force_visible_selection();
				redraw = true;
				m_pointer_repeat = now + std::chrono::milliseconds(100);
			}
		}

		// keep the pointer location where we updated
		m_pointer_updated = std::make_pair(x, y);
		return std::make_pair(IPT_INVALID, redraw);
	}

	// check for conversion of a tap to a finger drag
	auto const drag_result(check_touch_drag(uievt));
	if (track_pointer::TRACK_LINE != m_pointer_state)
		return drag_result;

	// only take action if the primary button was released
	if (!(uievt.pointer_released & 0x01))
		return std::make_pair(IPT_INVALID, false);

	// nothing to do if the item isn't selectable
	int const itemno = m_pointer_line + top_line;
	assert(itemno >= 0);
	assert(itemno < m_items.size());
	if (!is_selectable(m_items[itemno]))
		return std::make_pair(IPT_INVALID, false);

	// treat multi-click actions as not moving, otherwise check that pointer is still over the line
	if (0 >= uievt.pointer_clicks)
	{
		auto const [x, y] = pointer_location();
		if ((x < m_items_left) || (x >= m_items_right) || (int((y - m_items_top) / line_height()) != m_pointer_line))
			return std::make_pair(IPT_INVALID, true);
	}

	// anything other than a double-click just selects the item
	m_selected = itemno;
	if (2 != uievt.pointer_clicks)
		return std::make_pair(IPT_INVALID, true);

	// activate regular items by simulating UI Select
	if (!is_last_selected() || !m_needs_prev_menu_item)
		return std::make_pair(IPT_UI_SELECT, true);

	// handle the magic final item that dismisses the menu
	stack_pop();
	if (is_special_main_menu())
		machine().schedule_exit();
	return std::make_pair(IPT_UI_BACK, true);
}


//-------------------------------------------------
//  update_drag_scroll - update menu position in
//  response to a touch drag
//-------------------------------------------------

bool menu::update_drag_scroll(ui_event const &uievt)
{
	assert(track_pointer::SCROLL == m_pointer_state);
	assert((uievt.pointer_buttons | uievt.pointer_released) & 0x01);

	// get target location
	int const newtop(drag_scroll(
			pointer_location().second, m_pointer_down.second, m_pointer_updated.second, -line_height(),
			m_pointer_line, 0, int(m_items.size() - m_visible_lines)));
	if (newtop == top_line)
		return false;

	// scroll and move the selection if necessary to keep it in the visible range
	top_line = newtop;
	force_visible_selection();
	return true;
}


//-------------------------------------------------
//  update_drag_adjust - adjust value on
//  horizontal drag
//-------------------------------------------------

std::pair<int, bool> menu::update_drag_adjust(ui_event const &uievt)
{
	assert(track_pointer::ADJUST == m_pointer_state);
	assert((uievt.pointer_buttons | uievt.pointer_released) & 0x01);

	// this is ugly because adjustment is implemented by faking keystrokes - can't give a count/distance

	// set thresholds depending on the direction for hysteresis
	int const target(drag_scroll(
			pointer_location().first, m_pointer_updated.first, m_pointer_updated.first, line_height() * x_aspect(),
			0, std::numeric_limits<int>::min(), std::numeric_limits<int>::max()));

	// ensure the item under the pointer is selected and adjustable
	if ((top_line + m_pointer_line) == m_selected)
	{
		if (0 < target)
		{
			if (m_items[m_selected].flags() & FLAG_RIGHT_ARROW)
				return std::make_pair(IPT_UI_RIGHT, true);
		}
		else if (0 > target)
		{
			if (m_items[m_selected].flags() & FLAG_LEFT_ARROW)
				return std::make_pair(IPT_UI_LEFT, true);
		}
	}

	// looks like it wasn't to be
	return std::make_pair(IPT_INVALID, false);
}


//-------------------------------------------------
//  check_touch_drag - check for conversion of a
//  touch to a scroll or adjust slide
//-------------------------------------------------

std::pair<int, bool> menu::check_touch_drag(ui_event const &uievt)
{
	// we handle touch differently to mouse or pen
	if (ui_event::pointer::TOUCH != uievt.pointer_type)
		return std::make_pair(IPT_INVALID, false);

	// check distances
	auto const [x, y] = pointer_location();
	auto const [h, v] = check_drag_conversion(x, y, m_pointer_down.first, m_pointer_down.second, line_height());
	if (h)
	{
		// only the selected line can be adjusted
		if ((top_line + m_pointer_line) == m_selected)
		{
			m_pointer_state = track_pointer::ADJUST;
			return update_drag_adjust(uievt);
		}
		else
		{
			m_pointer_state = track_pointer::COMPLETED;
		}
	}
	else if (v)
	{
		m_pointer_state = track_pointer::SCROLL;
		m_pointer_line = top_line;
		return std::make_pair(IPT_INVALID, update_drag_scroll(uievt));
	}

	// no update needed
	return std::make_pair(IPT_INVALID, false);
}


//-------------------------------------------------
//  handle_keys - generically handle
//  keys for a menu
//-------------------------------------------------

bool menu::handle_keys(uint32_t flags, int &iptkey)
{
	// bail if no items (happens if event handling triggered an item reset)
	if (m_items.empty())
		return false;

	bool const ignorepause = (flags & PROCESS_IGNOREPAUSE) || stack_has_special_main_menu();

	// if we hit select, return true or pop the stack, depending on the item
	if (exclusive_input_pressed(iptkey, IPT_UI_SELECT, 0))
	{
		if (is_last_selected() && m_needs_prev_menu_item)
		{
			iptkey = IPT_INVALID;
			stack_pop();
			if (is_special_main_menu())
				machine().schedule_exit();
		}
		return false;
	}

	// UI configure hides the menus
	if (!(flags & PROCESS_NOKEYS) && exclusive_input_pressed(iptkey, IPT_UI_MENU, 0) && !m_global_state.stack_has_special_main_menu())
	{
		if (is_one_shot())
			stack_pop();
		else
			m_global_state.hide_menu();
		return true;
	}

	// bail out
	if (flags & PROCESS_ONLYCHAR)
		return false;

	// hitting back also pops the stack
	if (exclusive_input_pressed(iptkey, IPT_UI_BACK, 0))
	{
		if (!custom_ui_back())
		{
			iptkey = IPT_INVALID;
			stack_pop();
			if (is_special_main_menu())
				machine().schedule_exit();
		}
		return false;
	}

	// validate the current selection
	validate_selection(1);

	// swallow left/right keys if they are not appropriate
	bool const ignoreleft = !(flags & PROCESS_LR_ALWAYS) && !(selected_item().flags() & FLAG_LEFT_ARROW);
	bool const ignoreright = !(flags & PROCESS_LR_ALWAYS) && !(selected_item().flags() & FLAG_RIGHT_ARROW);

	// accept left/right/prev/next keys as-is with repeat if appropriate
	if (!ignoreleft && exclusive_input_pressed(iptkey, IPT_UI_LEFT, (flags & PROCESS_LR_REPEAT) ? 6 : 0))
		return false;
	if (!ignoreright && exclusive_input_pressed(iptkey, IPT_UI_RIGHT, (flags & PROCESS_LR_REPEAT) ? 6 : 0))
		return false;

	// keep track of whether we changed anything
	bool updated(false);

	// up backs up by one item
	if (exclusive_input_pressed(iptkey, IPT_UI_UP, 6))
	{
		if (flags & PROCESS_CUSTOM_NAV)
		{
			return updated;
		}
		else if (is_first_selected())
		{
			select_last_item();
		}
		else
		{
			--m_selected;
			validate_selection(-1);
		}
		top_line -= (m_selected <= top_line && top_line != 0);
		if (m_selected <= top_line && m_visible_items != m_visible_lines)
			top_line--;
		updated = true;
	}

	// down advances by one item
	if (exclusive_input_pressed(iptkey, IPT_UI_DOWN, 6))
	{
		if (flags & PROCESS_CUSTOM_NAV)
		{
			return updated;
		}
		else if (is_last_selected())
		{
			select_first_item();
		}
		else
		{
			++m_selected;
			validate_selection(1);
		}
		top_line += (m_selected >= top_line + m_visible_items + (top_line != 0));
		if (m_selected >= (top_line + m_visible_items + (top_line != 0)))
			top_line++;
		updated = true;
	}

	// page up backs up by m_visible_items
	if (exclusive_input_pressed(iptkey, IPT_UI_PAGE_UP, 6))
	{
		if (flags & PROCESS_CUSTOM_NAV)
			return updated;
		m_selected -= m_visible_items;
		top_line -= m_visible_items - (last_item_visible() ? 1 : 0);
		if (m_selected < 0)
			m_selected = 0;
		validate_selection(1);
		updated = true;
	}

	// page down advances by m_visible_items
	if (exclusive_input_pressed(iptkey, IPT_UI_PAGE_DOWN, 6))
	{
		if (flags & PROCESS_CUSTOM_NAV)
			return updated;
		m_selected += m_visible_lines - 2 + is_first_selected();
		top_line += m_visible_lines - 2;

		if (m_selected > m_items.size() - 1)
			m_selected = m_items.size() - 1;
		validate_selection(-1);
		updated = true;
	}

	// home goes to the start
	if (exclusive_input_pressed(iptkey, IPT_UI_HOME, 0))
	{
		if (flags & PROCESS_CUSTOM_NAV)
			return updated;
		select_first_item();
		updated = true;
	}

	// end goes to the last
	if (exclusive_input_pressed(iptkey, IPT_UI_END, 0))
	{
		if (flags & PROCESS_CUSTOM_NAV)
			return updated;
		select_last_item();
		updated = true;
	}

	// pause enables/disables pause
	if (!ignorepause && exclusive_input_pressed(iptkey, IPT_UI_PAUSE, 0))
	{
		if (machine().paused())
			machine().resume();
		else
			machine().pause();
	}

	// see if any other UI keys are pressed
	if (iptkey == IPT_INVALID)
	{
		for (int code = IPT_UI_FIRST + 1; code < IPT_UI_LAST; code++)
		{
			switch (code)
			{
			case IPT_UI_LEFT:
				if (ignoreleft)
					continue;
				break;
			case IPT_UI_RIGHT:
				if (ignoreright)
					continue;
				break;
			case IPT_UI_PAUSE:
				if (ignorepause)
					continue;
				break;
			}
			if (exclusive_input_pressed(iptkey, code, 0))
				break;
		}
	}
	return updated;
}


//-------------------------------------------------
//  default handler implementations
//-------------------------------------------------

bool menu::custom_ui_back()
{
	return false;
}

std::tuple<int, bool, bool> menu::custom_pointer_updated(bool changed, ui_event const &uievt)
{
	return std::make_tuple(IPT_INVALID, false, false);
}

bool menu::custom_mouse_scroll(int lines)
{
	return false;
}

void menu::menu_activated()
{
}

void menu::menu_deactivated()
{
}

void menu::menu_dismissed()
{
}


//-------------------------------------------------
//  select_first_item - select the first item in
//  the menu
//-------------------------------------------------

void menu::select_first_item()
{
	m_selected = top_line = 0;
	validate_selection(1);
}


//-------------------------------------------------
//  select_last_item - select the last item in the
//  menu
//-------------------------------------------------

void menu::select_last_item()
{
	m_selected = top_line = m_items.size() - 1;
	validate_selection(-1);
}


//-------------------------------------------------
//  validate_selection - validate the
//  current selection and ensure it is on a
//  correct item
//-------------------------------------------------

void menu::validate_selection(int scandir)
{
	// clamp to be in range
	if (m_selected < 0)
		m_selected = 0;
	else if (m_selected >= m_items.size())
		m_selected = m_items.size() - 1;

	// skip past unselectable items
	while (!is_selectable(m_items[m_selected]))
		m_selected = (m_selected + m_items.size() + scandir) % m_items.size();
}


//-------------------------------------------------
//  activate_menu - handle becoming top of the
//  menu stack
//-------------------------------------------------

void menu::activate_menu()
{
	m_items_drawn = false;
	m_pointer_state = track_pointer::IDLE;
	m_accumulated_wheel = 0;
	m_active = true;
	menu_activated();
}


//-------------------------------------------------
//  check_metrics - recompute metrics if target
//  geometry has changed
//-------------------------------------------------

bool menu::check_metrics()
{
	render_manager &render(machine().render());
	render_target &target(render.ui_target());
	std::pair<uint32_t, uint32_t> const uisize(target.width(), target.height());
	float const aspect = render.ui_aspect(&container());
	if ((uisize == m_last_size) && (std::fabs(1.0F - (aspect / m_last_aspect)) < 1e-6F))
		return false;

	m_last_size = uisize;
	m_last_aspect = aspect;
	recompute_metrics(uisize.first, uisize.second, aspect);
	return true;
}


//-------------------------------------------------
//  do_rebuild - get the subclass to populate
//  the menu items
//-------------------------------------------------

bool menu::do_rebuild()
{
	if (!m_items.empty())
		return false;

	m_rebuilding = true;
	try
	{
		// add an item to return - this is a really hacky way of doing this
		if (m_needs_prev_menu_item)
			item_append(_("Return to Previous Menu"), 0, nullptr);

		// let implementation add other items
		populate();
	}
	catch (...)
	{
		m_items.clear();
		m_rebuilding = false;
		throw;
	}
	m_rebuilding = false;
	return true;
}


//-------------------------------------------------
//  force_visible_selection - if the selected item
//  is not visible, move the selection it it's
//  within the visible portion of the menu
//-------------------------------------------------

void menu::force_visible_selection()
{
	int const first(top_line ? (top_line + 1) : 0);
	int const last(top_line + m_visible_lines - ((m_items.size() > (top_line + m_visible_lines)) ? 1 : 0));
	if (first > m_selected)
	{
		m_selected = first;
		while (!is_selectable(m_items[m_selected]))
			++m_selected;
		assert(last > m_selected);
	}
	else if (last <= m_selected)
	{
		m_selected = last - 1;
		while (!is_selectable(m_items[m_selected]))
			--m_selected;
		assert(first <= m_selected);
	}
}



/***************************************************************************
    MENU STACK MANAGEMENT
***************************************************************************/

bool menu::do_handle()
{
	bool need_update = false;

	// let OSD do its thing
	machine().osd().check_osd_inputs();

	// recompute metrics if necessary
	if (check_metrics())
		need_update = true;

	// get the implementation to rebuild the list of items if necessary
	if (do_rebuild())
		need_update = true;
	validate_selection(1);

	// reset the event
	std::optional<event> result;
	result.emplace();
	result->itemref = nullptr;
	result->item = nullptr;
	result->iptkey = IPT_INVALID;

	// process input
	uint32_t flags(m_process_flags);
	if (!(flags & (PROCESS_NOKEYS | PROCESS_NOINPUT)))
	{
		// read events
		if (handle_events(flags, *result))
			need_update = true;

		switch (m_pointer_state)
		{
		case track_pointer::IDLE:
		case track_pointer::IGNORED:
			// handle keys if we don't already have an event and we aren't tracking a pointer action
			if ((IPT_INVALID == result->iptkey) && handle_keys(flags, result->iptkey))
				need_update = true;
			break;
		default:
			// ignore keys pressed while tracking a pointer action
			for (int code = IPT_UI_FIRST + 1; IPT_UI_LAST > code; ++code)
				machine().ui_input().pressed(code);
			break;
		}
	}

	// deal with stack push/pop and rebuild
	if (!is_active())
		return false;
	if (do_rebuild())
	{
		validate_selection(1);
		need_update = true;
	}

	// update the selected item in the event and let the implementation handle it
	if ((result->iptkey != IPT_INVALID) && selection_valid())
	{
		result->itemref = get_selection_ref();
		result->item = &m_items[m_selected];
	}
	else
	{
		result.reset();
	}
	need_update = handle(result ? &*result : nullptr) || need_update;

	// the implementation had another chance to push/pop or rebuild
	if (!is_active())
		return false;
	if (do_rebuild())
	{
		validate_selection(1);
		return true;
	}
	return need_update;
}


/***************************************************************************
    UI SYSTEM INTERACTION
***************************************************************************/

//-------------------------------------------------
//  ui_menu_ui_handler - displays the current menu
//  and calls the menu handler
//-------------------------------------------------

delegate<uint32_t (render_container &)> menu::get_ui_handler(mame_ui_manager &mui)
{
	global_state &state(get_global_state(mui));
	return delegate<uint32_t (render_container &)>(&global_state::ui_handler, &state);
}

/***************************************************************************
    MENU HELPERS
***************************************************************************/

//-------------------------------------------------
//  create_layout
//-------------------------------------------------

text_layout menu::create_layout(float width, text_layout::text_justify justify, text_layout::word_wrapping wrap)
{
	return text_layout(*ui().get_font(), line_height() * x_aspect(), line_height(), width, justify, wrap);
}


//-------------------------------------------------
//  highlight
//-------------------------------------------------

void menu::highlight(float x0, float y0, float x1, float y1, rgb_t bgcolor)
{
	container().add_quad(x0, y0, x1, y1, bgcolor, m_global_state.hilight_texture(), PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_TEXWRAP(1) | PRIMFLAG_PACKABLE);
}


//-------------------------------------------------
//  draw_arrow
//-------------------------------------------------

void menu::draw_arrow(float x0, float y0, float x1, float y1, rgb_t fgcolor, uint32_t orientation)
{
	container().add_quad(x0, y0, x1, y1, fgcolor, m_global_state.arrow_texture(), PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_TEXORIENT(orientation) | PRIMFLAG_PACKABLE);
}


//-------------------------------------------------
//  extra_text_draw_box - generically adds header
//  or footer text
//-------------------------------------------------

void menu::extra_text_draw_box(float origx1, float origx2, float origy, float yspan, std::string_view text, int direction)
{
	// get the size of the text
	auto layout = create_layout();
	layout.add_text(text);

	// position this extra text
	float x1, y1, x2, y2;
	extra_text_position(origx1, origx2, origy, yspan, layout, direction, x1, y1, x2, y2);

	// draw a box
	ui().draw_outlined_box(container(), x1, y1, x2, y2, ui().colors().background_color());

	// take off the borders
	x1 += lr_border();
	y1 += tb_border();

	// draw the text within it
	layout.emit(container(), x1, y1);
}


void menu::draw_background()
{
	// draw background image if available
	if (ui().options().use_background_image() && m_global_state.bgrnd_bitmap() && m_global_state.bgrnd_bitmap()->valid())
		container().add_quad(0.0F, 0.0F, 1.0F, 1.0F, rgb_t::white(), m_global_state.bgrnd_texture(), PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
}


//-------------------------------------------------
//  extra_text_position - given extra text that has
//  been put into a layout, position it
//-------------------------------------------------

void menu::extra_text_position(float origx1, float origx2, float origy, float yspan, text_layout &layout,
	int direction, float &x1, float &y1, float &x2, float &y2)
{
	float width = layout.actual_width() + (2 * lr_border());
	float maxwidth = std::max(width, origx2 - origx1);

	// compute our bounds
	x1 = 0.5F - (0.5F * maxwidth);
	x2 = x1 + maxwidth;
	y1 = origy + (yspan * direction);
	y2 = origy + (tb_border() * direction);

	if (y1 > y2)
		std::swap(y1, y2);
}


//-------------------------------------------------
//  extra_text_render - generically adds header
//  and footer text
//-------------------------------------------------

void menu::extra_text_render(float top, float bottom, float origx1, float origy1, float origx2, float origy2, std::string_view header, std::string_view footer)
{
	if (!header.empty())
		extra_text_draw_box(origx1, origx2, origy1, top, header, -1);
	if (!footer.empty())
		extra_text_draw_box(origx1, origx2, origy2, bottom, footer, +1);
}

} // namespace ui
