// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

    ui/textbox.cpp

    Menu that displays a non-interactive text box

***************************************************************************/

#include "emu.h"
#include "textbox.h"

#include "ui/ui.h"

#include "uiinput.h"

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
	, m_line_bounds(0.0F, 0.0F)
	, m_visible_top(0.0F)
	, m_layout_width(-1.0F)
	, m_desired_width(-1.0F)
	, m_desired_lines(-1)
	, m_window_lines(0)
	, m_top_line(0)
	, m_pointer_action(pointer_action::NONE)
	, m_scroll_repeat(std::chrono::steady_clock::time_point::min())
	, m_base_pointer(0.0F, 0.0F)
	, m_last_pointer(0.0F, 0.0F)
	, m_scroll_base(0)
{
}


menu_textbox::~menu_textbox()
{
}


void menu_textbox::reset_layout()
{
	// force recompute and scroll to top
	m_layout.reset();
	m_top_line = 0;
	m_pointer_action = pointer_action::NONE;
}


void menu_textbox::recompute_metrics(uint32_t width, uint32_t height, float aspect)
{
	menu::recompute_metrics(width, height, aspect);

	m_layout.reset();
	m_pointer_action = pointer_action::NONE;
}


std::tuple<int, bool, bool> menu_textbox::custom_pointer_updated(bool changed, ui_event const &uievt)
{
	// no pointer input if we don't have up-to-date content on-screen
	if (!m_layout || (ui_event::type::POINTER_ABORT == uievt.event_type))
	{
		m_pointer_action = pointer_action::NONE;
		return std::make_tuple(IPT_INVALID, false, false);
	}

	// if nothing's happening, check for clicks
	if (pointer_idle())
	{
		if ((uievt.pointer_pressed & 0x01) && !(uievt.pointer_buttons & ~u32(0x01)))
		{
			auto const [x, y] = pointer_location();
			if ((x >= m_line_bounds.first) && (x < m_line_bounds.second))
			{
				if (m_top_line && pointer_in_line(y, 0))
				{
					// scroll up arrow
					--m_top_line;
					m_pointer_action = pointer_action::SCROLL_UP;
					m_scroll_repeat = std::chrono::steady_clock::now() + std::chrono::milliseconds(300);
					m_last_pointer = std::make_pair(x, y);
					return std::make_tuple(IPT_INVALID, true, true);
				}
				else if (((m_top_line + m_window_lines) < m_layout->lines()) && pointer_in_line(y, m_window_lines - 1))
				{
					// scroll down arrow
					++m_top_line;
					m_pointer_action = pointer_action::SCROLL_DOWN;
					m_scroll_repeat = std::chrono::steady_clock::now() + std::chrono::milliseconds(300);
					m_last_pointer = std::make_pair(x, y);
					return std::make_tuple(IPT_INVALID, true, true);
				}
				else if ((2 == uievt.pointer_clicks) && pointer_in_line(y, m_window_lines + 1))
				{
					// return to previous menu item
					// FIXME: this should really use the start point of the multi-click action
					m_pointer_action = pointer_action::CHECK_EXIT;
					return std::make_tuple(IPT_INVALID, true, false);
				}
				else if ((ui_event::pointer::TOUCH == uievt.pointer_type) && (y >= m_visible_top) && (y < (m_visible_top + (float(m_window_lines) * line_height()))))
				{
					m_pointer_action = pointer_action::SCROLL_DRAG;
					m_base_pointer = std::make_pair(x, y);
					m_last_pointer = m_base_pointer;
					m_scroll_base = m_top_line;
					return std::make_tuple(IPT_INVALID, true, false);
				}
			}
		}
		return std::make_tuple(IPT_INVALID, false, false);
	}

	// handle in-progress actions
	switch (m_pointer_action)
	{
	case pointer_action::NONE:
		break;

	case pointer_action::SCROLL_UP:
	case pointer_action::SCROLL_DOWN:
		{
			// check for re-entry
			bool redraw(false);
			float const linetop(m_visible_top + ((pointer_action::SCROLL_DOWN == m_pointer_action) ? (float(m_window_lines - 1) * line_height()) : 0.0F));
			float const linebottom(linetop + line_height());
			auto const [x, y] = pointer_location();
			bool const reentered(reentered_rect(m_last_pointer.first, m_last_pointer.second, x, y, m_line_bounds.first, linetop, m_line_bounds.second, linebottom));
			if (reentered)
			{
				auto const now(std::chrono::steady_clock::now());
				if (scroll_if_expired(now))
				{
					redraw = true;
					m_scroll_repeat = now + std::chrono::milliseconds(100);
				}
			}
			m_last_pointer = std::make_pair(x, y);
			if ((uievt.pointer_released & 0x01) || (uievt.pointer_pressed & ~u32(0x01)))
				m_pointer_action = pointer_action::NONE;
			return std::make_tuple(IPT_INVALID, pointer_action::NONE != m_pointer_action, redraw);
		}

	case pointer_action::SCROLL_DRAG:
		{
			// scroll if it moved
			auto const newtop(drag_scroll(
					pointer_location().second, m_base_pointer.second, m_last_pointer.second, -line_height(),
					m_scroll_base, 0, int(m_layout->lines() - m_window_lines)));
			bool const scrolled(newtop != m_top_line);
			m_top_line = newtop;

			// catch the end of the gesture
			if ((uievt.pointer_released & 0x01) || (uievt.pointer_pressed & ~u32(0x01)))
				m_pointer_action = pointer_action::NONE;
			return std::make_tuple(IPT_INVALID, pointer_action::NONE != m_pointer_action, scrolled);
		}

	case pointer_action::CHECK_EXIT:
		if (uievt.pointer_released & 0x01)
			return std::make_tuple((2 == uievt.pointer_clicks) ? IPT_UI_SELECT : IPT_INVALID, false, false);
		else if (uievt.pointer_buttons & ~u32(0x01))
			return std::make_tuple(IPT_INVALID, false, false);
		return std::make_tuple(IPT_INVALID, true, false);
	}
	return std::make_tuple(IPT_INVALID, false, false);
}


bool menu_textbox::custom_mouse_scroll(int lines)
{
	m_top_line += lines;
	return true;
}


bool menu_textbox::handle(event const *ev)
{
	// deal with repeating scroll arrows
	bool scrolled(false);
	if ((pointer_action::SCROLL_UP == m_pointer_action) || (pointer_action::SCROLL_DOWN == m_pointer_action))
	{
		float const linetop(m_visible_top + ((pointer_action::SCROLL_DOWN == m_pointer_action) ? (float(m_window_lines - 1) * line_height()) : 0.0F));
		float const linebottom(linetop + line_height());
		if (pointer_in_rect(m_line_bounds.first, linetop, m_line_bounds.second, linebottom))
		{
			while (scroll_if_expired(std::chrono::steady_clock::now()))
			{
				scrolled = true;
				m_scroll_repeat += std::chrono::milliseconds(100);
			}
		}
	}

	if (ev)
	{
		switch (ev->iptkey)
		{
		case IPT_UI_SELECT:
			stack_pop();
			return true;

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
		}
	}

	return scrolled;
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

	// account for extra space at the top and bottom and the separator/item for closing
	float visible_main_menu_height = 1.0F - (2.0F * tb_border()) - visible_extra_menu_height;
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
	m_visible_top = ((1.0F - (visible_main_menu_height + visible_extra_menu_height)) * 0.5F) + get_customtop();

	// get width required to draw the sole menu item
	menu_item const &pitem = item(0);
	std::string_view const itemtext = pitem.text();
	float const itemwidth = gutter_width() + get_string_width(itemtext) + gutter_width();
	float const draw_width = std::min(maximum_width, std::max(itemwidth, m_desired_width));

	// compute text box size
	float const x1 = visible_left + ((maximum_width - draw_width) * 0.5F);
	float const y1 = m_visible_top - tb_border();
	float const x2 = visible_left + visible_width - ((maximum_width - draw_width) * 0.5F);
	float const y2 = m_visible_top + visible_main_menu_height + tb_border() + extra_height;
	float const effective_left = x1 + gutter_width();
	m_line_bounds = std::make_pair(x1 + (0.5F * UI_LINE_WIDTH), x2 - (0.5F * UI_LINE_WIDTH));
	float const separator = m_visible_top + float(m_window_lines) * line_height();

	ui().draw_outlined_box(container(), x1, y1, x2, y2, ui().colors().background_color());

	int const desired_lines = m_layout->lines();
	int const drawn_lines = (std::min)(desired_lines, m_window_lines);
	m_top_line = (std::max)(0, m_top_line);
	if ((m_top_line + drawn_lines) >= desired_lines)
		m_top_line = desired_lines - drawn_lines;

	if (m_top_line)
	{
		// if we're not showing the top line, display the up arrow
		rgb_t fgcolor(ui().colors().text_color());
		bool const hovered(pointer_in_rect(m_line_bounds.first, m_visible_top, m_line_bounds.second, m_visible_top + line_height()));
		if (hovered && (pointer_action::SCROLL_UP == m_pointer_action))
		{
			fgcolor = ui().colors().selected_color();
			highlight(
					m_line_bounds.first, m_visible_top,
					m_line_bounds.second, m_visible_top + line_height(),
					ui().colors().selected_bg_color());
		}
		else if ((hovered && pointer_idle()) || (pointer_action::SCROLL_UP == m_pointer_action))
		{
			fgcolor = ui().colors().mouseover_color();
			highlight(
					m_line_bounds.first, m_visible_top,
					m_line_bounds.second, m_visible_top + line_height(),
					ui().colors().mouseover_bg_color());
		}
		draw_arrow(
				0.5F * (x1 + x2 - ud_arrow_width()), m_visible_top + (0.25F * line_height()),
				0.5F * (x1 + x2 + ud_arrow_width()), m_visible_top + (0.75F * line_height()),
				fgcolor, ROT0);
	}
	if ((m_top_line + m_window_lines) < desired_lines)
	{
		// if we're not showing the bottom line, display the down arrow
		float const line_y(m_visible_top + float(m_window_lines - 1) * line_height());
		rgb_t fgcolor(ui().colors().text_color());
		bool const hovered(pointer_in_rect(m_line_bounds.first, line_y, m_line_bounds.second, line_y + line_height()));
		if (hovered && (pointer_action::SCROLL_DOWN == m_pointer_action))
		{
			fgcolor = ui().colors().selected_color();
			highlight(
					m_line_bounds.first, line_y,
					m_line_bounds.second, line_y + line_height(),
					ui().colors().selected_bg_color());
		}
		else if ((hovered && pointer_idle()) || (pointer_action::SCROLL_DOWN == m_pointer_action))
		{
			fgcolor = ui().colors().mouseover_color();
			highlight(
					m_line_bounds.first, line_y,
					m_line_bounds.second, line_y + line_height(),
					ui().colors().mouseover_bg_color());
		}
		draw_arrow(
				0.5F * (x1 + x2 - ud_arrow_width()), line_y + (0.25F * line_height()),
				0.5F * (x1 + x2 + ud_arrow_width()), line_y + (0.75F * line_height()),
				fgcolor, ROT0 ^ ORIENTATION_FLIP_Y);
	}

	// draw visible lines, minus 1 for top arrow and 1 for bottom arrow
	auto const text_lines = drawn_lines - (m_top_line ? 1 : 0) - ((m_top_line + drawn_lines) != desired_lines);
	m_layout->emit(
			container(),
			m_top_line ? (m_top_line + 1) : 0, text_lines,
			effective_left, m_visible_top + (m_top_line ? line_height() : 0.0F));

	// add visual separator before the "return to prevous menu" item
	container().add_line(
			x1, separator + (0.5F * line_height()),
			x2, separator + (0.5F * line_height()),
			UI_LINE_WIDTH, ui().colors().text_color(), PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));

	float const line_y0 = m_visible_top + float(m_window_lines + 1) * line_height();
	float const line_y1 = line_y0 + line_height();

	highlight(m_line_bounds.first, line_y0, m_line_bounds.second, line_y1, ui().colors().selected_bg_color());
	ui().draw_text_full(
			container(), itemtext,
			effective_left, line_y0, draw_width,
			text_layout::text_justify::CENTER, text_layout::word_wrapping::TRUNCATE,
			mame_ui_manager::NORMAL,
			ui().colors().selected_color(), ui().colors().selected_bg_color(),
			nullptr, nullptr,
			line_height());

	// if there is something special to add, do it by calling the virtual method
	custom_render(flags, get_selection_ref(), get_customtop(), get_custombottom(), x1, y1, x2, y2);
}


bool menu_textbox::scroll_if_expired(std::chrono::steady_clock::time_point now)
{
	if (now < m_scroll_repeat)
		return false;

	if (pointer_action::SCROLL_DOWN == m_pointer_action)
	{
		if ((m_top_line + m_window_lines) < m_layout->lines())
			++m_top_line;
		if ((m_top_line + m_window_lines) == m_layout->lines())
			m_pointer_action = pointer_action::NONE;
	}
	else
	{
		if (0 < m_top_line)
			--m_top_line;
		if (!m_top_line)
			m_pointer_action = pointer_action::NONE;
	}
	return true;
}


inline bool menu_textbox::pointer_in_line(float y, int line) const
{
	float const top(m_visible_top + (float(line) * line_height()));
	return (top <= y) && ((top + line_height()) > y);
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


void menu_fixed_textbox::custom_render(uint32_t flags, void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	std::string_view const toptext[] = { m_heading };
	draw_text_box(
			std::begin(toptext), std::end(toptext),
			origx1, origx2, origy1 - top, origy1 - tb_border(),
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

} // namespace ui
