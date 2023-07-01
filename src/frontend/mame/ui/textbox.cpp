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
#include <utility>


namespace ui {

namespace {

inline std::string_view split_column(std::string_view &line)
{
	auto const split = line.find('\t');
	if (std::string::npos == split)
	{
		return std::exchange(line, std::string_view());
	}
	else
	{
		std::string_view result = line.substr(0, split);
		line.remove_prefix(split + 1);
		return result;
	}
}


template <typename T, typename U, typename V>
void populate_three_column_layout(std::string_view text, T &&l, U &&c, V &&r)
{
	while (!text.empty())
	{
		// pop a line from the front
		auto const eol = text.find('\n');
		std::string_view line = (std::string_view::npos != eol)
				? text.substr(0, eol + 1)
				: text;
		text.remove_prefix(line.length());

		// left-justify up to the first tab
		std::string_view const lcol = split_column(line);
		if (!lcol.empty())
			l(lcol);

		// centre up to the second tab
		if (!line.empty())
		{
			std::string_view const ccol = split_column(line);
			if (!ccol.empty())
				c(ccol);
		}

		// right-justify the rest
		if (!line.empty())
			r(line);
	}
}

} // anonymous namespace



//-------------------------------------------------
//  menu_textbox - base text box menu class
//-------------------------------------------------

menu_textbox::menu_textbox(mame_ui_manager &mui, render_container &container)
	: menu(mui, container)
	, m_layout()
	, m_layout_width(-1.0F)
	, m_desired_width(-1.0F)
	, m_desired_lines(-1)
	, m_window_lines(0)
	, m_top_line(0)
{
}


menu_textbox::~menu_textbox()
{
}


void menu_textbox::reset_layout()
{
	// force recompute and scroll to top
	m_layout = std::nullopt;
	m_top_line = 0;
}


bool menu_textbox::handle_key(int key)
{
	switch (key)
	{
	case IPT_UI_UP:
		--m_top_line;
		return true;

	case IPT_UI_DOWN:
		++m_top_line;
		return true;

	case IPT_UI_PAGE_UP:
		m_top_line -= m_window_lines - 3;
		return true;

	case IPT_UI_PAGE_DOWN:
		m_top_line += m_window_lines - 3;
		return true;

	case IPT_UI_HOME:
		m_top_line = 0;
		return true;

	case IPT_UI_END:
		m_top_line = m_layout->lines() - m_window_lines;
		return true;

	default:
		return false;
	}
}


void menu_textbox::recompute_metrics(uint32_t width, uint32_t height, float aspect)
{
	menu::recompute_metrics(width, height, aspect);

	m_layout = std::nullopt;
}


bool menu_textbox::custom_mouse_scroll(int lines)
{
	m_top_line += lines;
	return true;
}


void menu_textbox::draw(uint32_t flags)
{
	float const visible_width = 1.0F - (2.0F * lr_border());
	float const visible_left = (1.0F - visible_width) * 0.5F;
	float const extra_height = 2.0F * line_height();
	float const visible_extra_menu_height = get_customtop() + get_custombottom() + extra_height;

	// determine effective positions
	float const maximum_width = visible_width - (2.0F * gutter_width());

	draw_background();
	map_mouse();

	// account for extra space at the top and bottom and the separator/item for closing
	float visible_main_menu_height = 1.0F - 2.0F * tb_border() - visible_extra_menu_height;
	m_window_lines = int(std::trunc(visible_main_menu_height / line_height()));

	// lay out the text if necessary
	if (!m_layout || (m_layout_width != maximum_width))
	{
		m_desired_width = maximum_width;
		populate_text(m_layout, m_desired_width, m_desired_lines);
		m_layout_width = maximum_width;
	}
	m_window_lines = (std::min)(m_desired_lines, m_window_lines);
	visible_main_menu_height = float(m_window_lines) * line_height();

	// compute top/left of inner menu area by centering, if the menu is at the bottom of the extra, adjust
	float const visible_top = ((1.0F - (visible_main_menu_height + visible_extra_menu_height)) * 0.5F) + get_customtop();

	// get width required to draw the sole menu item
	menu_item const &pitem = item(0);
	std::string_view const itemtext = pitem.text();
	float const itemwidth = gutter_width() + get_string_width(itemtext) + gutter_width();
	float const draw_width = std::min(maximum_width, std::max(itemwidth, m_desired_width));

	// compute text box size
	float const x1 = visible_left + ((maximum_width - draw_width) * 0.5F);
	float const y1 = visible_top - tb_border();
	float const x2 = visible_left + visible_width - ((maximum_width - draw_width) * 0.5F);
	float const y2 = visible_top + visible_main_menu_height + tb_border() + extra_height;
	float const effective_left = x1 + gutter_width();
	float const line_x0 = x1 + 0.5F * UI_LINE_WIDTH;
	float const line_x1 = x2 - 0.5F * UI_LINE_WIDTH;
	float const separator = visible_top + float(m_window_lines) * line_height();

	ui().draw_outlined_box(container(), x1, y1, x2, y2, ui().colors().background_color());

	int const visible_items = m_layout->lines();
	m_window_lines = (std::min)(visible_items, m_window_lines);
	m_top_line = (std::max)(0, m_top_line);
	if (m_top_line + m_window_lines >= visible_items)
		m_top_line = visible_items - m_window_lines;

	clear_hover();
	if (m_top_line)
	{
		// if we're on the top line, display the up arrow
		rgb_t fgcolor = ui().colors().text_color();
		if (mouse_in_rect(line_x0, visible_top, line_x1, visible_top + line_height()))
		{
			fgcolor = ui().colors().mouseover_color();
			highlight(
					line_x0, visible_top,
					line_x1, visible_top + line_height(),
					ui().colors().mouseover_bg_color());
			set_hover(HOVER_ARROW_UP);
		}
		draw_arrow(
				0.5F * (x1 + x2 - ud_arrow_width()), visible_top + (0.25F * line_height()),
				0.5F * (x1 + x2 + ud_arrow_width()), visible_top + (0.75F * line_height()),
				fgcolor, ROT0);
	}
	if ((m_top_line + m_window_lines) < visible_items)
	{
		// if we're on the bottom line, display the down arrow
		float const line_y = visible_top + float(m_window_lines - 1) * line_height();
		rgb_t fgcolor = ui().colors().text_color();
		if (mouse_in_rect(line_x0, line_y, line_x1, line_y + line_height()))
		{
			fgcolor = ui().colors().mouseover_color();
			highlight(
					line_x0, line_y,
					line_x1, line_y + line_height(),
					ui().colors().mouseover_bg_color());
			set_hover(HOVER_ARROW_DOWN);
		}
		draw_arrow(
				0.5F * (x1 + x2 - ud_arrow_width()), line_y + (0.25F * line_height()),
				0.5F * (x1 + x2 + ud_arrow_width()), line_y + (0.75F * line_height()),
				fgcolor, ROT0 ^ ORIENTATION_FLIP_Y);
	}

	// draw visible lines, minus 1 for top arrow and 1 for bottom arrow
	auto const text_lines = m_window_lines - (m_top_line ? 1 : 0) - (m_top_line + m_window_lines != visible_items);
	m_layout->emit(
			container(),
			m_top_line ? (m_top_line + 1) : 0, text_lines,
			effective_left, visible_top + (m_top_line ? line_height() : 0.0F));

	// add visual separator before the "return to prevous menu" item
	container().add_line(
			x1, separator + (0.5F * line_height()),
			x2, separator + (0.5F * line_height()),
			UI_LINE_WIDTH, ui().colors().text_color(), PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));

	float const line_y0 = separator + line_height();
	float const line_y1 = line_y0 + line_height();

	if (mouse_in_rect(line_x0, line_y0, line_x1, line_y1) && is_selectable(pitem))
		set_hover(0);

	highlight(line_x0, line_y0, line_x1, line_y1, ui().colors().selected_bg_color());
	ui().draw_text_full(
			container(), itemtext,
			effective_left, line_y0, draw_width,
			text_layout::text_justify::CENTER, text_layout::word_wrapping::TRUNCATE,
			mame_ui_manager::NORMAL,
			ui().colors().selected_color(), ui().colors().selected_bg_color(),
			nullptr, nullptr,
			line_height());

	// if there is something special to add, do it by calling the virtual method
	custom_render(get_selection_ref(), get_customtop(), get_custombottom(), x1, y1, x2, y2);
}



//-------------------------------------------------
//  menu_fixed_textbox - text box with three-
//  column content supplied at construction
//-------------------------------------------------

menu_fixed_textbox::menu_fixed_textbox(
		mame_ui_manager &mui,
		render_container &container,
		std::string &&heading,
		std::string &&content)
	: menu_textbox(mui, container)
	, m_heading(std::move(heading))
	, m_content(std::move(content))
{
}


menu_fixed_textbox::~menu_fixed_textbox()
{
}


void menu_fixed_textbox::recompute_metrics(uint32_t width, uint32_t height, float aspect)
{
	menu_textbox::recompute_metrics(width, height, aspect);

	set_custom_space(line_height() + 3.0F * tb_border(), 0.0F);
}


void menu_fixed_textbox::custom_render(void *selectedref, float top, float bottom, float x1, float y1, float x2, float y2)
{
	std::string_view const toptext[] = { m_heading };
	draw_text_box(
			std::begin(toptext), std::end(toptext),
			x1, x2, y1 - top, y1 - tb_border(),
			text_layout::text_justify::CENTER, text_layout::word_wrapping::NEVER, false,
			ui().colors().text_color(), UI_GREEN_COLOR);
}


void menu_fixed_textbox::populate_text(
		std::optional<text_layout> &layout,
		float &width,
		int &lines)
{
	// ugly - use temporary layouts to compute required width
	{
		text_layout l(create_layout(width));
		text_layout c(create_layout(width));
		text_layout r(create_layout(width));
		populate_three_column_layout(
				m_content,
				[&l] (std::string_view s)
				{
					l.add_text(s, text_layout::text_justify::LEFT);
					if (s.back() != '\n')
						l.add_text("\n", text_layout::text_justify::LEFT);
				},
				[&c] (std::string_view s) {
					c.add_text(s, text_layout::text_justify::LEFT);
					if (s.back() != '\n')
						c.add_text("\n", text_layout::text_justify::LEFT);
				},
				[&r] (std::string_view s) {
					r.add_text(s, text_layout::text_justify::LEFT);
					if (s.back() != '\n')
						r.add_text("\n", text_layout::text_justify::LEFT);
				});
		width = (std::min)(l.actual_width() + c.actual_width() + r.actual_width(), width);
	}

	// now do it for real
	layout.emplace(create_layout(width));
	rgb_t const color = ui().colors().text_color();
	populate_three_column_layout(
			m_content,
			[&layout, color] (std::string_view s) { layout->add_text(s, text_layout::text_justify::LEFT, color); },
			[&layout, color] (std::string_view s) { layout->add_text(s, text_layout::text_justify::CENTER, color); },
			[&layout, color] (std::string_view s) { layout->add_text(s, text_layout::text_justify::RIGHT, color); });
	lines = layout->lines();
}


void menu_fixed_textbox::populate()
{
}


bool menu_fixed_textbox::handle(event const *ev)
{
	return ev && handle_key(ev->iptkey);
}

} // namespace ui
