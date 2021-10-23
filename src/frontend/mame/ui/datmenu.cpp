// license:BSD-3-Clause
// copyright-holders:Maurizio Petrarota, Vas Crabb
/*********************************************************************

    ui/datmenu.cpp

    Internal UI user interface.

*********************************************************************/

#include "emu.h"
#include "ui/datmenu.h"

#include "ui/systemlist.h"
#include "ui/ui.h"
#include "ui/utils.h"

#include "luaengine.h"
#include "mame.h"

#include "drivenum.h"
#include "rendfont.h"
#include "softlist.h"
#include "uiinput.h"

#include <cmath>
#include <string_view>


namespace ui {

//-------------------------------------------------
//  ctor / dtor
//-------------------------------------------------

menu_dats_view::menu_dats_view(mame_ui_manager &mui, render_container &container, const ui_system_info *system)
	: menu(mui, container)
	, m_system(!system ? &system_list::instance().systems()[driver_list::find(mui.machine().system().name)] : system)
	, m_swinfo(nullptr)
	, m_issoft(false)
	, m_layout()
	, m_actual(0)

{
	for (device_image_interface& image : image_interface_enumerator(mui.machine().root_device()))
	{
		if (image.filename())
		{
			m_list = strensure(image.software_list_name());
			m_short = image.software_entry()->shortname();
			m_long = image.software_entry()->longname();
			m_parent = image.software_entry()->parentname();
		}
	}
	std::vector<std::string> lua_list;
	if (mame_machine_manager::instance()->lua()->call_plugin("data_list", system ? system->driver->name : "", lua_list))
	{
		int count = 0;
		for (std::string& item : lua_list)
		{
			std::string version;
			mame_machine_manager::instance()->lua()->call_plugin("data_version", count, version);
			m_items_list.emplace_back(item.c_str(), count, std::move(version));
			count++;
		}
	}
}

//-------------------------------------------------
//  ctor
//-------------------------------------------------

menu_dats_view::menu_dats_view(mame_ui_manager &mui, render_container &container, const ui_software_info &swinfo)
	: menu(mui, container)
	, m_system(nullptr)
	, m_swinfo(&swinfo)
	, m_issoft(true)
	, m_layout()
	, m_actual(0)
	, m_list(swinfo.listname)
	, m_short(swinfo.shortname)
	, m_long(swinfo.longname)
	, m_parent(swinfo.parentname)

{
	if (!swinfo.infotext.empty())
		m_items_list.emplace_back(_("Software List Info"), 0, "");
	std::vector<std::string> lua_list;
	if (mame_machine_manager::instance()->lua()->call_plugin("data_list", std::string(m_short).append(1, ',').append(m_list).c_str(), lua_list))
	{
		int count = 1;
		for (std::string &item : lua_list)
		{
			std::string version;
			mame_machine_manager::instance()->lua()->call_plugin("data_version", count - 1, version);
			m_items_list.emplace_back(item.c_str(), count, std::move(version));
			count++;
		}
	}
}

//-------------------------------------------------
//  dtor
//-------------------------------------------------

menu_dats_view::~menu_dats_view()
{
}

//-------------------------------------------------
//  add text to layout
//-------------------------------------------------

void menu_dats_view::add_info_text(text_layout &layout, std::string_view text, rgb_t color, float size)
{
	char justify = 'l'; // left justify by default
	if ((text.length() > 3) && (text[0] == '#') && (text[1] == 'j'))
	{
		auto const eol = text.find('\n');
		if ((std::string_view::npos != eol) && (2 < eol))
		{
			justify = text[2];
			text.remove_prefix(eol + 1);
		}
	}

	if ('2' == justify)
	{
		while (!text.empty())
		{
			// pop a line from the front
			auto const eol = text.find('\n');
			std::string_view const line = (std::string_view::npos != eol)
					? text.substr(0, eol + 1)
					: text;
			text.remove_prefix(line.length());

			// split on the first tab
			auto const split = line.find('\t');
			if (std::string_view::npos != split)
			{
				layout.add_text(line.substr(0, split), text_layout::text_justify::LEFT, color, rgb_t::transparent(), size);
				layout.add_text(" ", text_layout::text_justify::LEFT, color, rgb_t::transparent(), size);
				layout.add_text(line.substr(split + 1), text_layout::text_justify::RIGHT, color, rgb_t::transparent(), size);
			}
			else
			{
				layout.add_text(line, text_layout::text_justify::LEFT, color, rgb_t::transparent(), size);
			}
		}
	}
	else
	{
		// use the same alignment for all the text
		auto const j =
				('c' == justify) ? text_layout::text_justify::CENTER :
				('r' == justify) ? text_layout::text_justify::RIGHT :
				text_layout::text_justify::LEFT;
		layout.add_text(text, j, color, rgb_t::transparent(), size);
	}

}

//-------------------------------------------------
//  handle
//-------------------------------------------------

void menu_dats_view::handle()
{
	event const *const menu_event = process(PROCESS_LR_ALWAYS | PROCESS_CUSTOM_NAV);
	if (menu_event)
	{
		switch (menu_event->iptkey)
		{
		case IPT_UI_LEFT:
			if (m_actual > 0)
			{
				m_actual--;
				reset(reset_options::SELECT_FIRST);
			}
			break;

		case IPT_UI_RIGHT:
			if ((m_actual + 1) < m_items_list.size())
			{
				m_actual++;
				reset(reset_options::SELECT_FIRST);
			}
			break;

		case IPT_UI_UP:
			--top_line;
			break;

		case IPT_UI_DOWN:
			++top_line;
			break;

		case IPT_UI_PAGE_UP:
			top_line -= m_visible_lines - 3;
			break;

		case IPT_UI_PAGE_DOWN:
			top_line += m_visible_lines - 3;
			break;

		case IPT_UI_HOME:
			top_line = 0;
			break;

		case IPT_UI_END:
			top_line = m_layout->lines() - m_visible_lines;
			break;
		}
	}
}

//-------------------------------------------------
//  populate
//-------------------------------------------------

void menu_dats_view::populate(float &customtop, float &custombottom)
{
	bool paused = machine().paused();
	if (!paused)
		machine().pause();

	m_layout = std::nullopt;

	customtop = 2.0f * ui().get_line_height() + 4.0f * ui().box_tb_border();
	custombottom = ui().get_line_height() + 3.0f * ui().box_tb_border();

	if (!paused)
		machine().resume();
}

//-------------------------------------------------
//  draw - draw dats menu
//-------------------------------------------------

void menu_dats_view::draw(uint32_t flags)
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
	float const effective_width = visible_width - 2.0f * gutter_width;
	float const effective_left = visible_left + gutter_width;

	draw_background();
	map_mouse();

	// account for extra space at the top and bottom
	float visible_main_menu_height = 1.0f - 2.0f * ui().box_tb_border() - visible_extra_menu_height;
	m_visible_lines = int(std::trunc(visible_main_menu_height / line_height));
	visible_main_menu_height = float(m_visible_lines) * line_height;

	// compute top/left of inner menu area by centering, if the menu is at the bottom of the extra, adjust
	float const visible_top = ((1.0f - (visible_main_menu_height + visible_extra_menu_height)) * 0.5f) + get_customtop();

	// compute text box size
	float const x1 = visible_left;
	float const y1 = visible_top - ui().box_tb_border();
	float const x2 = x1 + visible_width;
	float const y2 = visible_top + visible_main_menu_height + ui().box_tb_border() + extra_height;
	float const line_x0 = x1 + 0.5f * UI_LINE_WIDTH;
	float const line_x1 = x2 - 0.5f * UI_LINE_WIDTH;
	float const separator = visible_top + float(m_visible_lines) * line_height;

	ui().draw_outlined_box(container(), x1, y1, x2, y2, ui().colors().background_color());

	if (!m_layout || (m_layout->width() != effective_width))
	{
		std::string buffer;
		if (!m_items_list.empty())
		{
			if (m_issoft)
				get_data_sw(buffer);
			else
				get_data(buffer);
		}
		m_layout.emplace(ui().create_layout(container(), effective_width));
		add_info_text(*m_layout, buffer, ui().colors().text_color());
	}
	int const visible_items = m_layout->lines();
	m_visible_lines = (std::min)(visible_items, m_visible_lines);
	top_line = (std::max)(0, top_line);
	if (top_line + m_visible_lines >= visible_items)
		top_line = visible_items - m_visible_lines;

	clear_hover();
	if (top_line)
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
	if ((top_line + m_visible_lines) < visible_items)
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
	m_visible_items = m_visible_lines - (top_line ? 1 : 0) - (top_line + m_visible_lines != visible_items);
	m_layout->emit(
			container(),
			top_line ? (top_line + 1) : 0, m_visible_items,
			effective_left, visible_top + (top_line ? line_height : 0.0f));

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
			effective_left, line_y0, effective_width,
			text_layout::text_justify::CENTER, text_layout::word_wrapping::TRUNCATE,
			mame_ui_manager::NORMAL,
			ui().colors().selected_color(), ui().colors().selected_bg_color(),
			nullptr, nullptr);

	// if there is something special to add, do it by calling the virtual method
	custom_render(get_selection_ref(), get_customtop(), get_custombottom(), x1, y1, x2, y2);
}

//-------------------------------------------------
//  perform our special rendering
//-------------------------------------------------

void menu_dats_view::custom_render(void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	float maxwidth = origx2 - origx1;
	float width;
	std::string_view const driver = m_issoft ? m_swinfo->longname : m_system->description;

	float const lr_border = ui().box_lr_border() * machine().render().ui_aspect(&container());
	ui().draw_text_full(
			container(),
			driver,
			0.0f, 0.0f, 1.0f,
			ui::text_layout::text_justify::CENTER, text_layout::word_wrapping::TRUNCATE,
			mame_ui_manager::NONE, rgb_t::white(), rgb_t::black(), &width, nullptr);
	width += 2 * lr_border;
	maxwidth = std::max(maxwidth, width);

	// compute our bounds
	float x1 = 0.5f - 0.5f * maxwidth;
	float x2 = x1 + maxwidth;
	float y1 = origy1 - top;
	float y2 = origy1 - 2.0f * ui().box_tb_border() - ui().get_line_height();

	// draw a box
	ui().draw_outlined_box(container(), x1, y1, x2, y2, UI_GREEN_COLOR);

	// take off the borders
	x1 += lr_border;
	x2 -= lr_border;
	y1 += ui().box_tb_border();

	ui().draw_text_full(
			container(),
			driver,
			x1, y1, x2 - x1,
			text_layout::text_justify::CENTER, ui::text_layout::word_wrapping::NEVER,
			mame_ui_manager::NORMAL, ui().colors().text_color(), ui().colors().text_bg_color());

	maxwidth = 0;
	for (auto const &elem : m_items_list)
	{
		ui().draw_text_full(
				container(),
				elem.label,
				0.0f, 0.0f, 1.0f,
				text_layout::text_justify::CENTER, text_layout::word_wrapping::NEVER,
				mame_ui_manager::NONE, rgb_t::white(), rgb_t::black(),
				&width, nullptr);
		maxwidth += width;
	}

	float space = (1.0f - maxwidth) / (m_items_list.size() * 2);

	// compute our bounds
	x1 -= lr_border;
	x2 += lr_border;
	y1 = y2 + ui().box_tb_border();
	y2 += ui().get_line_height() + 2.0f * ui().box_tb_border();

	// draw a box
	ui().draw_outlined_box(container(), x1, y1, x2, y2, ui().colors().background_color());

	// take off the borders
	y1 += ui().box_tb_border();

	// draw the text within it
	int x = 0;
	for (auto const &elem : m_items_list)
	{
		x1 += space;
		if (mouse_in_rect(x1 - (space / 2), y1, x1 + width + (space / 2), y2))
			set_hover(HOVER_INFO_TEXT + 1 + x);

		rgb_t const fcolor = (m_actual == x) ? rgb_t(0xff, 0xff, 0xff, 0x00) : ui().colors().text_color();
		rgb_t const bcolor = (m_actual == x) ? rgb_t(0xff, 0xff, 0xff, 0xff) : ui().colors().text_bg_color();
		ui().draw_text_full(
				container(),
				elem.label,
				x1, y1, 1.0f,
				text_layout::text_justify::LEFT, text_layout::word_wrapping::NEVER,
				mame_ui_manager::NONE, fcolor, bcolor,
				&width, nullptr);

		if (bcolor != ui().colors().text_bg_color())
		{
			ui().draw_textured_box(
					container(),
					x1 - (space / 2), y1, x1 + width + (space / 2), y2,
					bcolor, rgb_t(255, 43, 43, 43),
					hilight_main_texture(), PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_TEXWRAP(1));
		}

		ui().draw_text_full(
				container(),
				elem.label,
				x1, y1, 1.0f,
				text_layout::text_justify::LEFT, text_layout::word_wrapping::NEVER,
				mame_ui_manager::NORMAL, fcolor, bcolor,
				&width, nullptr);
		x1 += width + space;
		++x;
	}

	// bottom
	if (!m_items_list.empty())
	{
		std::string const revision(util::string_format(_("Revision: %1$s"), m_items_list[m_actual].revision));
		ui().draw_text_full(
				container(),
				revision,
				0.0f, 0.0f, 1.0f,
				text_layout::text_justify::CENTER, text_layout::word_wrapping::TRUNCATE,
				mame_ui_manager::NONE, rgb_t::white(), rgb_t::black(),
				&width, nullptr);
		width += 2 * lr_border;
		maxwidth = std::max(origx2 - origx1, width);

		// compute our bounds
		x1 = 0.5f - 0.5f * maxwidth;
		x2 = x1 + maxwidth;
		y1 = origy2 + ui().box_tb_border();
		y2 = origy2 + bottom;

		// draw a box
		ui().draw_outlined_box(container(), x1, y1, x2, y2, UI_GREEN_COLOR);

		// take off the borders
		x1 += lr_border;
		x2 -= lr_border;
		y1 += ui().box_tb_border();

		// draw the text within it
		ui().draw_text_full(
				container(),
				revision,
				x1, y1, x2 - x1,
				text_layout::text_justify::CENTER, text_layout::word_wrapping::TRUNCATE,
				mame_ui_manager::NORMAL, ui().colors().text_color(), ui().colors().text_bg_color());
	}
}

//-------------------------------------------------
//  load data from DATs
//-------------------------------------------------

bool menu_dats_view::custom_mouse_down()
{
	if ((hover() > HOVER_INFO_TEXT) && ((hover() - HOVER_INFO_TEXT) <= m_items_list.size()))
	{
		if ((hover() - HOVER_INFO_TEXT - 1) != m_actual)
		{
			m_actual = hover() - HOVER_INFO_TEXT - 1;
			reset(reset_options::SELECT_FIRST);
		}
		return true;
	}
	else
	{
		return false;
	}
}

//-------------------------------------------------
//  load data from DATs
//-------------------------------------------------

void menu_dats_view::get_data(std::string &buffer)
{
	mame_machine_manager::instance()->lua()->call_plugin("data", m_items_list[m_actual].option, buffer);
}

void menu_dats_view::get_data_sw(std::string &buffer)
{
	if (m_items_list[m_actual].option == 0)
		buffer = m_swinfo->infotext;
	else
		mame_machine_manager::instance()->lua()->call_plugin("data", m_items_list[m_actual].option - 1, buffer);
}

} // namespace ui
