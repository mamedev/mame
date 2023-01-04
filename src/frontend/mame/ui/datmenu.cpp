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
#include <limits>
#include <string_view>


namespace ui {

//-------------------------------------------------
//  ctor / dtor
//-------------------------------------------------

menu_dats_view::menu_dats_view(mame_ui_manager &mui, render_container &container, const ui_system_info *system)
	: menu_textbox(mui, container)
	, m_system(!system ? &system_list::instance().systems()[driver_list::find(mui.machine().system().name)] : system)
	, m_swinfo(nullptr)
	, m_issoft(false)
	, m_actual(0)

{
	set_process_flags(PROCESS_LR_ALWAYS | PROCESS_CUSTOM_NAV);
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
	: menu_textbox(mui, container)
	, m_system(nullptr)
	, m_swinfo(&swinfo)
	, m_issoft(true)
	, m_actual(0)
	, m_list(swinfo.listname)
	, m_short(swinfo.shortname)
	, m_long(swinfo.longname)
	, m_parent(swinfo.parentname)

{
	set_process_flags(PROCESS_LR_ALWAYS | PROCESS_CUSTOM_NAV);
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

void menu_dats_view::handle(event const *ev)
{
	if (ev)
	{
		switch (ev->iptkey)
		{
		case IPT_UI_LEFT:
			if (m_actual > 0)
			{
				m_actual--;
				reset_layout();
			}
			break;

		case IPT_UI_RIGHT:
			if ((m_actual + 1) < m_items_list.size())
			{
				m_actual++;
				reset_layout();
			}
			break;

		default:
			handle_key(ev->iptkey);
		}
	}
}

//-------------------------------------------------
//  populate
//-------------------------------------------------

void menu_dats_view::populate(float &customtop, float &custombottom)
{
	customtop = 2.0f * line_height() + 4.0f * tb_border();
	custombottom = line_height() + 3.0f * tb_border();
}

//-------------------------------------------------
//  perform our special rendering
//-------------------------------------------------

void menu_dats_view::custom_render(void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	float maxwidth = origx2 - origx1;
	float width;
	std::string_view const driver = m_issoft ? m_swinfo->longname : m_system->description;

	width = get_string_width(driver);
	width += 2 * lr_border();
	maxwidth = std::max(maxwidth, width);

	// compute our bounds
	float x1 = 0.5f - 0.5f * maxwidth;
	float x2 = x1 + maxwidth;
	float y1 = origy1 - top;
	float y2 = origy1 - 2.0f * tb_border() - line_height();

	// draw a box
	ui().draw_outlined_box(container(), x1, y1, x2, y2, UI_GREEN_COLOR);

	// take off the borders
	x1 += lr_border();
	x2 -= lr_border();
	y1 += tb_border();

	draw_text_normal(
			driver,
			x1, y1, x2 - x1,
			text_layout::text_justify::CENTER, ui::text_layout::word_wrapping::NEVER,
			ui().colors().text_color());

	maxwidth = 0;
	for (auto const &elem : m_items_list)
	{
		width = get_string_width(elem.label);
		maxwidth += width;
	}

	float space = (1.0f - maxwidth) / (m_items_list.size() * 2);

	// compute our bounds
	x1 -= lr_border();
	x2 += lr_border();
	y1 = y2 + tb_border();
	y2 += line_height() + 2.0f * tb_border();

	// draw a box
	ui().draw_outlined_box(container(), x1, y1, x2, y2, ui().colors().background_color());

	// take off the borders
	y1 += tb_border();

	// draw the text within it
	int x = 0;
	for (auto const &elem : m_items_list)
	{
		x1 += space;
		if (mouse_in_rect(x1 - (space / 2), y1, x1 + width + (space / 2), y2))
			set_hover(HOVER_INFO_TEXT + 1 + x);

		rgb_t const fcolor = (m_actual == x) ? rgb_t(0xff, 0xff, 0xff, 0x00) : ui().colors().text_color();
		rgb_t const bcolor = (m_actual == x) ? rgb_t(0xff, 0xff, 0xff, 0xff) : ui().colors().text_bg_color();
		width = get_string_width(elem.label);

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
				&width, nullptr,
				line_height());
		x1 += width + space;
		++x;
	}

	// bottom
	if (!m_items_list.empty())
	{
		std::string const revision(util::string_format(_("Revision: %1$s"), m_items_list[m_actual].revision));
		width = get_text_width(
				revision,
				0.0f, 0.0f, 1.0f,
				text_layout::text_justify::CENTER, text_layout::word_wrapping::TRUNCATE);
		width += 2 * lr_border();
		maxwidth = std::max(origx2 - origx1, width);

		// compute our bounds
		x1 = 0.5f - 0.5f * maxwidth;
		x2 = x1 + maxwidth;
		y1 = origy2 + tb_border();
		y2 = origy2 + bottom;

		// draw a box
		ui().draw_outlined_box(container(), x1, y1, x2, y2, UI_GREEN_COLOR);

		// take off the borders
		x1 += lr_border();
		x2 -= lr_border();
		y1 += tb_border();

		// draw the text within it
		draw_text_normal(
				revision,
				x1, y1, x2 - x1,
				text_layout::text_justify::CENTER, text_layout::word_wrapping::TRUNCATE,
				ui().colors().text_color());
	}
}

//-------------------------------------------------
//  custom mouse click handling
//-------------------------------------------------

bool menu_dats_view::custom_mouse_down()
{
	if ((hover() > HOVER_INFO_TEXT) && ((hover() - HOVER_INFO_TEXT) <= m_items_list.size()))
	{
		if ((hover() - HOVER_INFO_TEXT - 1) != m_actual)
		{
			m_actual = hover() - HOVER_INFO_TEXT - 1;
			reset_layout();
		}
		return true;
	}
	else
	{
		return false;
	}
}

//-------------------------------------------------
//  populate selected DAT text
//-------------------------------------------------

void menu_dats_view::populate_text(std::optional<text_layout> &layout, float &width, int &lines)
{
	if (!layout || (layout->width() != width))
	{
		std::string buffer;
		if (!m_items_list.empty())
		{
			if (m_issoft)
				get_data_sw(buffer);
			else
				get_data(buffer);
		}
		layout.emplace(create_layout(width));
		add_info_text(*layout, buffer, ui().colors().text_color());
		lines = std::numeric_limits<int>::max();
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
