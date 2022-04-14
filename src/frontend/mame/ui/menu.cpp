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
#include "ui/utils.h"
#include "ui/miscmenu.h"

#include "cheat.h"
#include "mame.h"

#include "corestr.h"
#include "drivenum.h"
#include "fileio.h"
#include "rendutil.h"
#include "uiinput.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <utility>


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
		m_ui.machine().ui_input().reset();
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

	// ensure topmost menu is active - need a loop because it could push another menu
	while (m_stack && !m_stack->is_active())
	{
		m_stack->m_active = true;
		m_stack->menu_activated();
	}

	// update the menu state
	m_hide = false;
	if (m_stack)
		m_stack->do_handle();

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
		return UI_HANDLER_CANCEL;
	}

	return 0;
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
	, m_items()
	, m_process_flags(0)
	, m_selected(0)
	, m_hover(1)
	, m_special_main_menu(false)
	, m_one_shot(false)
	, m_needs_prev_menu_item(true)
	, m_active(false)
	, m_event()
	, m_customtop(0.0f)
	, m_custombottom(0.0f)
	, m_resetpos(0)
	, m_resetref(nullptr)
	, m_mouse_hit(false)
	, m_mouse_button(false)
	, m_mouse_x(-1.0f)
	, m_mouse_y(-1.0f)
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

void menu::item_append(menu_item_type type, uint32_t flags)
{
	if (type == menu_item_type::SEPARATOR)
		item_append(MENU_SEPARATOR_ITEM, flags, nullptr, menu_item_type::SEPARATOR);
}

//-------------------------------------------------
//  item_append - append a new item to the
//  end of the menu
//-------------------------------------------------

void menu::item_append(std::string &&text, std::string &&subtext, uint32_t flags, void *ref, menu_item_type type)
{
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
}


//-------------------------------------------------
//  item_append_on_off - append a new "On"/"Off"
//  item to the end of the menu
//-------------------------------------------------

void menu::item_append_on_off(const std::string &text, bool state, uint32_t flags, void *ref, menu_item_type type)
{
	if (flags & FLAG_DISABLE)
		ref = nullptr;
	else
		flags |= state ? FLAG_LEFT_ARROW : FLAG_RIGHT_ARROW;

	item_append(std::string(text), state ? _("On") : _("Off"), flags, ref, type);
}


//-------------------------------------------------
//  process - process a menu, drawing it
//  and returning any interesting events
//-------------------------------------------------

const menu::event *menu::process()
{
	// reset the event
	m_event.iptkey = IPT_INVALID;

	// first make sure our selection is valid
	validate_selection(1);

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
		container().add_rect(0.0f, 0.0f, 1.0f, 1.0f, rgb_t(114, 0, 0, 0), PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));

	// draw the menu proper
	draw(m_process_flags);

	// process input
	if (!(m_process_flags & (PROCESS_NOKEYS | PROCESS_NOINPUT)))
	{
		// read events
		handle_events(m_process_flags, m_event);

		// handle the keys if we don't already have an event
		if (m_event.iptkey == IPT_INVALID)
			handle_keys(m_process_flags, m_event.iptkey);
	}

	// update the selected item in the event
	if ((m_event.iptkey != IPT_INVALID) && selection_valid())
	{
		m_event.itemref = get_selection_ref();
		m_event.item = &m_items[m_selected];
		return &m_event;
	}
	else
	{
		return nullptr;
	}
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
//  draw - draw a menu
//-------------------------------------------------

void menu::draw(uint32_t flags)
{
	// first draw the FPS counter
	if (ui().show_fps_counter())
	{
		ui().draw_text_full(
				container(),
				machine().video().speed_text(),
				0.0f, 0.0f, 1.0f,
				text_layout::text_justify::RIGHT, text_layout::word_wrapping::WORD,
				mame_ui_manager::OPAQUE_, rgb_t::white(), rgb_t::black(),
				nullptr, nullptr);
	}

	bool const customonly = (flags & PROCESS_CUSTOM_ONLY);
	bool const noinput = (flags & PROCESS_NOINPUT);
	float const aspect = machine().render().ui_aspect(&container());
	float const line_height = ui().get_line_height();
	float const lr_arrow_width = 0.4f * line_height * aspect;
	float const ud_arrow_width = line_height * aspect;
	float const gutter_width = lr_arrow_width * 1.3f;
	float const lr_border = ui().box_lr_border() * aspect;

	if (is_special_main_menu())
		draw_background();

	// compute the width and height of the full menu
	float visible_width = 0;
	float visible_main_menu_height = 0;
	for (auto const &pitem : m_items)
	{
		// compute width of left hand side
		float total_width = gutter_width + ui().get_string_width(pitem.text()) + gutter_width;

		// add in width of right hand side
		if (!pitem.subtext().empty())
			total_width += 2.0f * gutter_width + ui().get_string_width(pitem.subtext());
		else if (pitem.flags() & FLAG_UI_HEADING)
			total_width += 4.0f * ud_arrow_width;

		// track the maximum
		if (total_width > visible_width)
			visible_width = total_width;

		// track the height as well
		visible_main_menu_height += line_height;
	}

	// account for extra space at the top and bottom
	float const visible_extra_menu_height = m_customtop + m_custombottom;

	// add a little bit of slop for rounding
	visible_width += 0.01f;
	visible_main_menu_height += 0.01f;

	// if we are too wide or too tall, clamp it down
	visible_width = std::min(visible_width, 1.0f - ((lr_border + (aspect * UI_LINE_WIDTH)) * 2.0f));

	// if the menu and extra menu won't fit, take away part of the regular menu, it will scroll
	if (visible_main_menu_height + visible_extra_menu_height + 2.0f * ui().box_tb_border() > 1.0f)
		visible_main_menu_height = 1.0f - 2.0f * ui().box_tb_border() - visible_extra_menu_height;

	m_visible_lines = std::min(int(std::floor(visible_main_menu_height / line_height)), int(unsigned(m_items.size())));
	visible_main_menu_height = float(m_visible_lines) * line_height;

	// compute top/left of inner menu area by centering
	float const visible_left = (1.0f - visible_width) * 0.5f;
	float const visible_top = ((1.0f - visible_main_menu_height - visible_extra_menu_height) * 0.5f) + m_customtop;

	// first add us a box
	float const x1 = visible_left - lr_border;
	float const y1 = visible_top - ui().box_tb_border();
	float const x2 = visible_left + visible_width + lr_border;
	float const y2 = visible_top + visible_main_menu_height + ui().box_tb_border();
	if (!customonly)
		ui().draw_outlined_box(container(), x1, y1, x2, y2, ui().colors().background_color());

	if ((m_selected >= (top_line + m_visible_lines)) || (m_selected < (top_line + 1)))
		top_line = m_selected - (m_visible_lines / 2);
	if (top_line < 0 || is_first_selected())
		top_line = 0;
	else if ((top_line > (m_items.size() - m_visible_lines)) || is_last_selected())
		top_line = m_items.size() - m_visible_lines;
	else if (m_selected >= (top_line + m_visible_lines - 2))
		top_line = m_selected - m_visible_lines + ((m_selected == (m_items.size() - 1)) ? 1: 2);

	// if scrolling, show arrows
	bool const show_top_arrow((m_items.size() > m_visible_lines) && !first_item_visible());
	bool const show_bottom_arrow((m_items.size() > m_visible_lines) && !last_item_visible());

	// set the number of visible lines, minus 1 for top arrow and 1 for bottom arrow
	m_visible_items = m_visible_lines - (show_top_arrow ? 1 : 0) - (show_bottom_arrow ? 1 : 0);

	// determine effective positions taking into account the hilighting arrows
	float const effective_width = visible_width - 2.0f * gutter_width;
	float const effective_left = visible_left + gutter_width;

	// locate mouse
	if (!customonly && !noinput)
		map_mouse();
	else
		ignore_mouse();

	// loop over visible lines
	m_hover = m_items.size() + 1;
	bool selected_subitem_too_big = false;
	float const line_x0 = x1 + 0.5f * UI_LINE_WIDTH;
	float const line_x1 = x2 - 0.5f * UI_LINE_WIDTH;
	if (!customonly)
	{
		for (int linenum = 0; linenum < m_visible_lines; linenum++)
		{
			auto const itemnum = top_line + linenum;
			menu_item const &pitem = m_items[itemnum];
			std::string_view const itemtext = pitem.text();
			rgb_t fgcolor = ui().colors().text_color();
			rgb_t bgcolor = ui().colors().text_bg_color();
			rgb_t fgcolor2 = ui().colors().subitem_color();
			rgb_t fgcolor3 = ui().colors().clone_color();
			float const line_y0 = visible_top + (float)linenum * line_height;
			float const line_y1 = line_y0 + line_height;

			// work out what we're dealing with
			bool const uparrow = !linenum && show_top_arrow;
			bool const downarrow = (linenum == (m_visible_lines - 1)) && show_bottom_arrow;

			// set the hover if this is our item
			bool const hovered = mouse_in_rect(line_x0, line_y0, line_x1, line_y1);
			if (hovered)
			{
				if (uparrow)
					m_hover = HOVER_ARROW_UP;
				else if (downarrow)
					m_hover = HOVER_ARROW_DOWN;
				else if (is_selectable(pitem))
					m_hover = itemnum;
			}

			if (is_selected(itemnum))
			{
				// if we're selected, draw with a different background
				fgcolor = fgcolor2 = fgcolor3 = ui().colors().selected_color();
				bgcolor = ui().colors().selected_bg_color();
			}
			else if (hovered && (uparrow || downarrow || is_selectable(pitem)))
			{
				// else if the mouse is over this item, draw with a different background
				fgcolor = fgcolor2 = fgcolor3 = ui().colors().mouseover_color();
				bgcolor = ui().colors().mouseover_bg_color();
			}

			// if we have some background hilighting to do, add a quad behind everything else
			if (bgcolor != ui().colors().text_bg_color())
				highlight(line_x0, line_y0, line_x1, line_y1, bgcolor);

			if (uparrow)
			{
				// if we're on the top line, display the up arrow
				draw_arrow(
						0.5f * (x1 + x2) - 0.5f * ud_arrow_width,
						line_y0 + 0.25f * line_height,
						0.5f * (x1 + x2) + 0.5f * ud_arrow_width,
						line_y0 + 0.75f * line_height,
						fgcolor,
						ROT0);
			}
			else if (downarrow)
			{
				// if we're on the bottom line, display the down arrow
				draw_arrow(
						0.5f * (x1 + x2) - 0.5f * ud_arrow_width,
						line_y0 + 0.25f * line_height,
						0.5f * (x1 + x2) + 0.5f * ud_arrow_width,
						line_y0 + 0.75f * line_height,
						fgcolor,
						ROT0 ^ ORIENTATION_FLIP_Y);
			}
			else if (pitem.type() == menu_item_type::SEPARATOR)
			{
				// if we're just a divider, draw a line
				container().add_line(visible_left, line_y0 + 0.5f * line_height, visible_left + visible_width, line_y0 + 0.5f * line_height, UI_LINE_WIDTH, ui().colors().border_color(), PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
			}
			else if (pitem.subtext().empty())
			{
				// if we don't have a subitem, just draw the string centered
				if (pitem.flags() & FLAG_UI_HEADING)
				{
					float heading_width = ui().get_string_width(itemtext);
					container().add_line(visible_left, line_y0 + 0.5f * line_height, visible_left + ((visible_width - heading_width) / 2) - lr_border, line_y0 + 0.5f * line_height, UI_LINE_WIDTH, ui().colors().border_color(), PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
					container().add_line(visible_left + visible_width - ((visible_width - heading_width) / 2) + lr_border, line_y0 + 0.5f * line_height, visible_left + visible_width, line_y0 + 0.5f * line_height, UI_LINE_WIDTH, ui().colors().border_color(), PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
				}
				ui().draw_text_full(
						container(),
						itemtext,
						effective_left, line_y0, effective_width,
						text_layout::text_justify::CENTER, text_layout::word_wrapping::TRUNCATE,
						mame_ui_manager::NORMAL, fgcolor, bgcolor,
						nullptr, nullptr);
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
						&item_width, nullptr);

				if (pitem.flags() & FLAG_COLOR_BOX)
				{
					rgb_t color = rgb_t((uint32_t)strtoul(pitem.subtext().c_str(), nullptr, 16));

					// give 2 spaces worth of padding
					subitem_width = ui().get_string_width("FF00FF00");

					ui().draw_outlined_box(
							container(),
							effective_left + effective_width - subitem_width, line_y0 + (UI_LINE_WIDTH * 2.0f),
							effective_left + effective_width, line_y1 - (UI_LINE_WIDTH * 2.0f),
							color);
				}
				else
				{
					std::string_view subitem_text(pitem.subtext());

					// give 2 spaces worth of padding
					item_width += 2.0f * gutter_width;

					// if the subitem doesn't fit here, display dots
					if (ui().get_string_width(subitem_text) > effective_width - item_width)
					{
						subitem_text = "...";
						if (is_selected(itemnum))
							selected_subitem_too_big = true;
					}

					// customize subitem text color
					if (!core_stricmp(pitem.subtext().c_str(), _("On")))
						fgcolor2 = rgb_t(0x00,0xff,0x00);

					if (!core_stricmp(pitem.subtext().c_str(), _("Off")))
						fgcolor2 = rgb_t(0xff,0x00,0x00);

					if (!core_stricmp(pitem.subtext().c_str(), _("Auto")))
						fgcolor2 = rgb_t(0xff,0xff,0x00);

					// draw the subitem right-justified
					ui().draw_text_full(
							container(),
							subitem_text,
							effective_left + item_width, line_y0, effective_width - item_width,
							text_layout::text_justify::RIGHT, text_layout::word_wrapping::TRUNCATE,
							mame_ui_manager::NORMAL, subitem_invert ? fgcolor3 : fgcolor2, bgcolor,
							&subitem_width, nullptr);
				}

				// apply arrows
				if (is_selected(itemnum) && (pitem.flags() & FLAG_LEFT_ARROW))
				{
					float const l = effective_left + effective_width - subitem_width - gutter_width;
					float const r = l + lr_arrow_width;
					draw_arrow(
								l, line_y0 + 0.1f * line_height, r, line_y0 + 0.9f * line_height,
								fgcolor,
								ROT90 ^ ORIENTATION_FLIP_X);
					if (mouse_in_rect(l, line_y0 + 0.1f * line_height, r, line_y0 + 0.9f * line_height))
						m_hover = HOVER_UI_LEFT;
				}
				if (is_selected(itemnum) && (pitem.flags() & FLAG_RIGHT_ARROW))
				{
					float const r = effective_left + effective_width + gutter_width;
					float const l = r - lr_arrow_width;
					draw_arrow(
							l, line_y0 + 0.1f * line_height, r, line_y0 + 0.9f * line_height,
							fgcolor,
							ROT90);
					if (mouse_in_rect(l, line_y0 + 0.1f * line_height, r, line_y0 + 0.9f * line_height))
						m_hover = HOVER_UI_RIGHT;
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
		float const line_y = visible_top + (float)linenum * line_height;
		float target_width, target_height;

		// compute the multi-line target width/height
		ui().draw_text_full(
				container(),
				pitem.subtext(),
				0, 0, visible_width * 0.75f,
				text_layout::text_justify::RIGHT, text_layout::word_wrapping::WORD,
				mame_ui_manager::NONE, rgb_t::white(), rgb_t::black(),
				&target_width, &target_height);

		// determine the target location
		float const target_x = visible_left + visible_width - target_width - lr_border;
		float target_y = line_y + line_height + ui().box_tb_border();
		if (target_y + target_height + ui().box_tb_border() > visible_main_menu_height)
			target_y = line_y - target_height - ui().box_tb_border();

		// add a box around that
		ui().draw_outlined_box(
				container(),
				target_x - lr_border, target_y - ui().box_tb_border(),
				target_x + target_width + lr_border, target_y + target_height + ui().box_tb_border(),
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
	custom_render(get_selection_ref(), m_customtop, m_custombottom, x1, y1, x2, y2);
}

void menu::custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2)
{
}


//-------------------------------------------------
//  map_mouse - map mouse pointer location to menu
//  coordinates
//-------------------------------------------------

void menu::map_mouse()
{
	ignore_mouse();
	int32_t mouse_target_x, mouse_target_y;
	render_target *const mouse_target = machine().ui_input().find_mouse(&mouse_target_x, &mouse_target_y, &m_mouse_button);
	if (mouse_target)
	{
		if (mouse_target->map_point_container(mouse_target_x, mouse_target_y, container(), m_mouse_x, m_mouse_y))
			m_mouse_hit = true;
	}
}


//-------------------------------------------------
//  ignore_mouse - set members to ignore mouse
//  input
//-------------------------------------------------

void menu::ignore_mouse()
{
	m_mouse_hit = false;
	m_mouse_button = false;
	m_mouse_x = -1.0f;
	m_mouse_y = -1.0f;
}


//-------------------------------------------------
//  handle_events - generically handle
//  input events for a menu
//-------------------------------------------------

void menu::handle_events(uint32_t flags, event &ev)
{
	bool stop = false;
	ui_event local_menu_event;

	// loop while we have interesting events
	while (!stop && machine().ui_input().pop_event(&local_menu_event))
	{
		switch (local_menu_event.event_type)
		{
		// if we are hovering over a valid item, select it with a single click
		case ui_event::type::MOUSE_DOWN:
			if (custom_mouse_down())
				return;

			if (!(flags & PROCESS_ONLYCHAR))
			{
				if (m_hover >= 0 && m_hover < m_items.size())
				{
					m_selected = m_hover;
				}
				else if (m_hover == HOVER_ARROW_UP)
				{
					if (flags & PROCESS_CUSTOM_NAV)
					{
						ev.iptkey = IPT_UI_PAGE_UP;
						stop = true;
					}
					else
					{
						m_selected -= m_visible_items;
						if (m_selected < 0)
							m_selected = 0;
						top_line -= m_visible_items - (last_item_visible() ? 1 : 0);
					}
				}
				else if (m_hover == HOVER_ARROW_DOWN)
				{
					if (flags & PROCESS_CUSTOM_NAV)
					{
						ev.iptkey = IPT_UI_PAGE_DOWN;
						stop = true;
					}
					else
					{
						m_selected += m_visible_lines - 2 + is_first_selected();
						if (m_selected > m_items.size() - 1)
							m_selected = m_items.size() - 1;
						top_line += m_visible_lines - 2;
					}
				}
				else if (m_hover == HOVER_UI_LEFT)
				{
					ev.iptkey = IPT_UI_LEFT;
					stop = true;
				}
				else if (m_hover == HOVER_UI_RIGHT)
				{
					ev.iptkey = IPT_UI_RIGHT;
					stop = true;
				}
			}
			break;

		// if we are hovering over a valid item, fake a UI_SELECT with a double-click
		case ui_event::type::MOUSE_DOUBLE_CLICK:
			if (!(flags & PROCESS_ONLYCHAR) && m_hover >= 0 && m_hover < m_items.size())
			{
				m_selected = m_hover;
				ev.iptkey = IPT_UI_SELECT;
				if (is_last_selected() && m_needs_prev_menu_item)
				{
					ev.iptkey = IPT_UI_CANCEL;
					stack_pop();
					if (is_special_main_menu())
						machine().schedule_exit();
				}
				stop = true;
			}
			break;

		// caught scroll event
		case ui_event::type::MOUSE_WHEEL:
			if (!custom_mouse_scroll((0 < local_menu_event.zdelta) ? -local_menu_event.num_lines : local_menu_event.num_lines) && !(flags & (PROCESS_ONLYCHAR | PROCESS_CUSTOM_NAV)))
			{
				if (local_menu_event.zdelta > 0)
				{
					if (is_first_selected())
					{
						select_last_item();
					}
					else
					{
						m_selected -= local_menu_event.num_lines;
						validate_selection(-1);
					}
					top_line -= (m_selected <= top_line && top_line != 0);
					if (m_selected <= top_line && m_visible_items != m_visible_lines)
						top_line -= local_menu_event.num_lines;
				}
				else
				{
					if (is_last_selected())
					{
						select_first_item();
					}
					else
					{
						m_selected += local_menu_event.num_lines;
						validate_selection(1);
					}
					top_line += (m_selected >= top_line + m_visible_items + (top_line != 0));
					if (m_selected >= (top_line + m_visible_items + (top_line != 0)))
						top_line += local_menu_event.num_lines;
				}
			}
			break;

		// translate CHAR events into specials
		case ui_event::type::IME_CHAR:
			ev.iptkey = IPT_SPECIAL;
			ev.unichar = local_menu_event.ch;
			stop = true;
			break;

		// ignore everything else
		default:
			break;
		}
	}
}


//-------------------------------------------------
//  handle_keys - generically handle
//  keys for a menu
//-------------------------------------------------

void menu::handle_keys(uint32_t flags, int &iptkey)
{
	bool const ignorepause = (flags & PROCESS_IGNOREPAUSE) || stack_has_special_main_menu();

	// bail if no items
	if (m_items.empty())
		return;

	// if we hit select, return true or pop the stack, depending on the item
	if (exclusive_input_pressed(iptkey, IPT_UI_SELECT, 0))
	{
		if (is_last_selected() && m_needs_prev_menu_item)
		{
			iptkey = IPT_UI_CANCEL;
			stack_pop();
			if (is_special_main_menu())
				machine().schedule_exit();
		}
		return;
	}

	// UI configure hides the menus
	if (!(flags & PROCESS_NOKEYS) && exclusive_input_pressed(iptkey, IPT_UI_CONFIGURE, 0) && !m_global_state.stack_has_special_main_menu())
	{
		if (is_one_shot())
			stack_pop();
		else
			m_global_state.hide_menu();
		return;
	}

	// bail out
	if (flags & PROCESS_ONLYCHAR)
		return;

	// hitting cancel also pops the stack
	if (exclusive_input_pressed(iptkey, IPT_UI_CANCEL, 0))
	{
		if (!custom_ui_cancel())
		{
			stack_pop();
			if (is_special_main_menu())
				machine().schedule_exit();
		}
		return;
	}

	// validate the current selection
	validate_selection(1);

	// swallow left/right keys if they are not appropriate
	bool const ignoreleft = !(flags & PROCESS_LR_ALWAYS) && !(selected_item().flags() & FLAG_LEFT_ARROW);
	bool const ignoreright = !(flags & PROCESS_LR_ALWAYS) && !(selected_item().flags() & FLAG_RIGHT_ARROW);

	// accept left/right/prev/next keys as-is with repeat if appropriate
	if (!ignoreleft && exclusive_input_pressed(iptkey, IPT_UI_LEFT, (flags & PROCESS_LR_REPEAT) ? 6 : 0))
		return;
	if (!ignoreright && exclusive_input_pressed(iptkey, IPT_UI_RIGHT, (flags & PROCESS_LR_REPEAT) ? 6 : 0))
		return;

	// up backs up by one item
	if (exclusive_input_pressed(iptkey, IPT_UI_UP, 6))
	{
		if (flags & PROCESS_CUSTOM_NAV)
		{
			return;
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
	}

	// down advances by one item
	if (exclusive_input_pressed(iptkey, IPT_UI_DOWN, 6))
	{
		if (flags & PROCESS_CUSTOM_NAV)
		{
			return;
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
	}

	// page up backs up by m_visible_items
	if (exclusive_input_pressed(iptkey, IPT_UI_PAGE_UP, 6))
	{
		if (flags & PROCESS_CUSTOM_NAV)
			return;
		m_selected -= m_visible_items;
		top_line -= m_visible_items - (last_item_visible() ? 1 : 0);
		if (m_selected < 0)
			m_selected = 0;
		validate_selection(1);
	}

	// page down advances by m_visible_items
	if (exclusive_input_pressed(iptkey, IPT_UI_PAGE_DOWN, 6))
	{
		if (flags & PROCESS_CUSTOM_NAV)
			return;
		m_selected += m_visible_lines - 2 + is_first_selected();
		top_line += m_visible_lines - 2;

		if (m_selected > m_items.size() - 1)
			m_selected = m_items.size() - 1;
		validate_selection(-1);
	}

	// home goes to the start
	if (exclusive_input_pressed(iptkey, IPT_UI_HOME, 0))
	{
		if (flags & PROCESS_CUSTOM_NAV)
			return;
		select_first_item();
	}

	// end goes to the last
	if (exclusive_input_pressed(iptkey, IPT_UI_END, 0))
	{
		if (flags & PROCESS_CUSTOM_NAV)
			return;
		select_last_item();
	}

	// pause enables/disables pause
	if (!ignorepause && exclusive_input_pressed(iptkey, IPT_UI_PAUSE, 0))
	{
		if (machine().paused())
			machine().resume();
		else
			machine().pause();
	}

	// handle a toggle cheats request
	if (machine().ui_input().pressed_repeat(IPT_UI_TOGGLE_CHEAT, 0))
		mame_machine_manager::instance()->cheat().set_enable(!mame_machine_manager::instance()->cheat().enabled());

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



/***************************************************************************
    MENU STACK MANAGEMENT
***************************************************************************/

void menu::do_handle()
{
	if (m_items.empty())
	{
		// add an item to return - this is a really hacky way of doing this
		if (m_needs_prev_menu_item)
			item_append(_("Return to Previous Menu"), 0, nullptr);

		// let implementation add other items
		populate(m_customtop, m_custombottom);
	}
	handle(process());
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
	auto layout = ui().create_layout(container());
	layout.add_text(text);

	// position this extra text
	float x1, y1, x2, y2;
	extra_text_position(origx1, origx2, origy, yspan, layout, direction, x1, y1, x2, y2);

	// draw a box
	ui().draw_outlined_box(container(), x1, y1, x2, y2, ui().colors().background_color());

	// take off the borders
	x1 += ui().box_lr_border() * machine().render().ui_aspect(&container());
	y1 += ui().box_tb_border();

	// draw the text within it
	layout.emit(container(), x1, y1);
}


void menu::draw_background()
{
	// draw background image if available
	if (ui().options().use_background_image() && m_global_state.bgrnd_bitmap() && m_global_state.bgrnd_bitmap()->valid())
		container().add_quad(0.0f, 0.0f, 1.0f, 1.0f, rgb_t::white(), m_global_state.bgrnd_texture(), PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
}


//-------------------------------------------------
//  extra_text_position - given extra text that has
//  been put into a layout, position it
//-------------------------------------------------

void menu::extra_text_position(float origx1, float origx2, float origy, float yspan, text_layout &layout,
	int direction, float &x1, float &y1, float &x2, float &y2)
{
	float width = layout.actual_width() + (2 * ui().box_lr_border() * machine().render().ui_aspect(&container()));
	float maxwidth = std::max(width, origx2 - origx1);

	// compute our bounds
	x1 = 0.5f - 0.5f * maxwidth;
	x2 = x1 + maxwidth;
	y1 = origy + (yspan * direction);
	y2 = origy + (ui().box_tb_border() * direction);

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
