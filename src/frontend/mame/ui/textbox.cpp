// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

    ui/textbox.cpp

    Menu that displays a non-interactive text box

***************************************************************************/

#include "emu.h"
#include "textbox.h"

#include "ui/ui.h"
#include "ui/utils.h"

#include <string_view>


namespace ui {

//-------------------------------------------------
//  constructor
//-------------------------------------------------

menu_textbox::menu_textbox(mame_ui_manager &mui, render_container &container)
	: menu(mui, container)
	, m_layout()
	, m_layout_width(-1.0f)
	, m_desired_width(-1.0f)
	, m_top_line(0)
{
}


//-------------------------------------------------
//  destructor
//-------------------------------------------------

menu_textbox::~menu_textbox()
{
}


//-------------------------------------------------
//  reset_layout - force repopulate and scroll to
//  top
//-------------------------------------------------

void menu_textbox::reset_layout()
{
	m_layout = std::nullopt;
	m_top_line = 0;
}


//-------------------------------------------------
//  handle_key - handle basic navigation keys
//-------------------------------------------------

void menu_textbox::handle_key(int key)
{
	switch (key)
	{
	case IPT_UI_UP:
		--m_top_line;
		break;

	case IPT_UI_DOWN:
		++m_top_line;
		break;

	case IPT_UI_PAGE_UP:
		m_top_line -= m_visible_lines - 3;
		break;

	case IPT_UI_PAGE_DOWN:
		m_top_line += m_visible_lines - 3;
		break;

	case IPT_UI_HOME:
		m_top_line = 0;
		break;

	case IPT_UI_END:
		m_top_line = m_layout->lines() - m_visible_lines;
		break;
	}
}


//-------------------------------------------------
//  custom_mouse_scroll - handle scroll events
//-------------------------------------------------

bool menu_textbox::custom_mouse_scroll(int lines)
{
	m_top_line += lines;
	return true;
}


//-------------------------------------------------
//  draw - draw the menu
//-------------------------------------------------

void menu_textbox::draw(uint32_t flags)
{
	float const aspect = machine().render().ui_aspect(&container());
	float const line_height = ui().get_line_height();
	float const ud_arrow_width = line_height * aspect;
	float const gutter_width = 0.52f * line_height * aspect;
	float const visible_width = 1.0f - (2.0f * ui().box_lr_border() * aspect);
	float const visible_left = (1.0f - visible_width) * 0.5f;
	float const extra_height = 2.0f * line_height;
	float const visible_extra_menu_height = get_customtop() + get_custombottom() + extra_height;

	// determine effective positions taking into account the hilighting arrows
	float const maximum_width = visible_width - 2.0f * gutter_width;

	draw_background();
	map_mouse();

	// account for extra space at the top and bottom
	float visible_main_menu_height = 1.0f - 2.0f * ui().box_tb_border() - visible_extra_menu_height;
	m_visible_lines = int(std::trunc(visible_main_menu_height / line_height));
	visible_main_menu_height = float(m_visible_lines) * line_height;

	// compute top/left of inner menu area by centering, if the menu is at the bottom of the extra, adjust
	float const visible_top = ((1.0f - (visible_main_menu_height + visible_extra_menu_height)) * 0.5f) + get_customtop();

	// lay out the text if necessary
	if (!m_layout || (m_layout_width != maximum_width))
	{
		m_desired_width = populate_text(m_layout, maximum_width);
		m_layout_width = maximum_width;
	}

	// compute text box size
	float const x1 = visible_left + ((maximum_width - m_desired_width) * 0.5f);
	float const y1 = visible_top - ui().box_tb_border();
	float const x2 = visible_left + visible_width - ((maximum_width - m_desired_width) * 0.5f);
	float const y2 = visible_top + visible_main_menu_height + ui().box_tb_border() + extra_height;
	float const effective_left = x1 + gutter_width;
	float const line_x0 = x1 + 0.5f * UI_LINE_WIDTH;
	float const line_x1 = x2 - 0.5f * UI_LINE_WIDTH;
	float const separator = visible_top + float(m_visible_lines) * line_height;

	ui().draw_outlined_box(container(), x1, y1, x2, y2, ui().colors().background_color());

	int const visible_items = m_layout->lines();
	m_visible_lines = (std::min)(visible_items, m_visible_lines);
	m_top_line = (std::max)(0, m_top_line);
	if (m_top_line + m_visible_lines >= visible_items)
		m_top_line = visible_items - m_visible_lines;

	clear_hover();
	if (m_top_line)
	{
		// if we're on the top line, display the up arrow
		rgb_t fgcolor = ui().colors().text_color();
		if (mouse_in_rect(line_x0, visible_top, line_x1, visible_top + line_height))
		{
			fgcolor = ui().colors().mouseover_color();
			highlight(
					line_x0, visible_top,
					line_x1, visible_top + line_height,
					ui().colors().mouseover_bg_color());
			set_hover(HOVER_ARROW_UP);
		}
		draw_arrow(
				0.5f * (x1 + x2) - 0.5f * ud_arrow_width, visible_top + 0.25f * line_height,
				0.5f * (x1 + x2) + 0.5f * ud_arrow_width, visible_top + 0.75f * line_height,
				fgcolor, ROT0);
	}
	if ((m_top_line + m_visible_lines) < visible_items)
	{
		// if we're on the bottom line, display the down arrow
		float const line_y = visible_top + float(m_visible_lines - 1) * line_height;
		rgb_t fgcolor = ui().colors().text_color();
		if (mouse_in_rect(line_x0, line_y, line_x1, line_y + line_height))
		{
			fgcolor = ui().colors().mouseover_color();
			highlight(
					line_x0, line_y,
					line_x1, line_y + line_height,
					ui().colors().mouseover_bg_color());
			set_hover(HOVER_ARROW_DOWN);
		}
		draw_arrow(
				0.5f * (x1 + x2) - 0.5f * ud_arrow_width, line_y + 0.25f * line_height,
				0.5f * (x1 + x2) + 0.5f * ud_arrow_width, line_y + 0.75f * line_height,
				fgcolor, ROT0 ^ ORIENTATION_FLIP_Y);
	}

	// return the number of visible lines, minus 1 for top arrow and 1 for bottom arrow
	m_visible_items = m_visible_lines - (m_top_line ? 1 : 0) - (m_top_line + m_visible_lines != visible_items);
	m_layout->emit(
			container(),
			m_top_line ? (m_top_line + 1) : 0, m_visible_items,
			effective_left, visible_top + (m_top_line ? line_height : 0.0f));

	// add visual separator before the "return to prevous menu" item
	container().add_line(
			x1, separator + (0.5f * line_height),
			x2, separator + (0.5f * line_height),
			UI_LINE_WIDTH, ui().colors().text_color(), PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));

	menu_item const &pitem = item(0);
	std::string_view const itemtext = pitem.text;
	float const line_y0 = separator + line_height;
	float const line_y1 = line_y0 + line_height;

	if (mouse_in_rect(line_x0, line_y0, line_x1, line_y1) && is_selectable(pitem))
		set_hover(0);

	highlight(line_x0, line_y0, line_x1, line_y1, ui().colors().selected_bg_color());
	ui().draw_text_full(
			container(), itemtext,
			effective_left, line_y0, m_desired_width,
			text_layout::text_justify::CENTER, text_layout::word_wrapping::TRUNCATE,
			mame_ui_manager::NORMAL,
			ui().colors().selected_color(), ui().colors().selected_bg_color(),
			nullptr, nullptr);

	// if there is something special to add, do it by calling the virtual method
	custom_render(get_selection_ref(), get_customtop(), get_custombottom(), x1, y1, x2, y2);
}

} // namespace ui
