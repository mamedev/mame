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
//  construct for currently running or specified
//  system
//-------------------------------------------------

menu_dats_view::menu_dats_view(mame_ui_manager &mui, render_container &container, const ui_system_info *system)
	: menu_textbox(mui, container)
	, m_system(!system ? &system_list::instance().systems()[driver_list::find(mui.machine().system().name)] : system)
	, m_swinfo(nullptr)
	, m_issoft(false)
	, m_current_tab(0)
	, m_tab_line(1.0F, 0.0F)
	, m_clicked_tab(-1)
{
	set_process_flags(PROCESS_LR_ALWAYS | PROCESS_CUSTOM_NAV);
	for (device_image_interface& image : image_interface_enumerator(mui.machine().root_device()))
	{
		if (image.loaded_through_softlist())
		{
			m_list = image.software_list_name();
			m_short = image.software_entry()->shortname();
			m_long = image.software_entry()->longname();
			m_parent = image.software_entry()->parentname();
			break;
		}
	}

	std::vector<std::string> lua_list;
	if (mame_machine_manager::instance()->lua()->call_plugin("data_list", system ? system->driver->name : "", lua_list))
	{
		int count = 0;
		m_items_list.reserve(lua_list.size());
		for (std::string& item : lua_list)
		{
			std::string version;
			mame_machine_manager::instance()->lua()->call_plugin("data_version", count, version);
			m_items_list.emplace_back(std::move(item), count, std::move(version));
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
	, m_current_tab(0)
	, m_list(swinfo.listname)
	, m_short(swinfo.shortname)
	, m_long(swinfo.longname)
	, m_parent(swinfo.parentname)
	, m_tab_line(1.0F, 0.0F)
	, m_clicked_tab(-1)
{
	set_process_flags(PROCESS_LR_ALWAYS | PROCESS_CUSTOM_NAV);

	std::vector<std::string> lua_list;
	bool const retrieved(mame_machine_manager::instance()->lua()->call_plugin("data_list", std::string(m_short).append(1, ',').append(m_list).c_str(), lua_list));

	if (!swinfo.infotext.empty() || retrieved)
		m_items_list.reserve((!swinfo.infotext.empty() ? 1 : 0) + (retrieved ? lua_list.size() : 0));

	if (!swinfo.infotext.empty())
		m_items_list.emplace_back(_("Software List Info"), -1, "");

	if (retrieved)
	{
		int count = 0;
		for (std::string &item : lua_list)
		{
			std::string version;
			mame_machine_manager::instance()->lua()->call_plugin("data_version", count, version);
			m_items_list.emplace_back(std::move(item), count, std::move(version));
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

bool menu_dats_view::handle(event const *ev)
{
	if (ev)
	{
		// don't bother with parent event handling if we need to redraw anyway
		switch (ev->iptkey)
		{
		case IPT_UI_LEFT:
			if (m_current_tab > 0)
			{
				m_current_tab--;
				m_tab_line = std::make_pair(1.0F, 0.0F);
				m_clicked_tab = -1;
				reset_layout();
				return true;
			}
			break;

		case IPT_UI_RIGHT:
			if ((m_current_tab + 1) < m_items_list.size())
			{
				m_current_tab++;
				m_tab_line = std::make_pair(1.0F, 0.0F);
				m_clicked_tab = -1;
				reset_layout();
				return true;
			}
			break;
		}
	}

	return menu_textbox::handle(ev);
}

//-------------------------------------------------
//  populate
//-------------------------------------------------

void menu_dats_view::populate()
{
}

//-------------------------------------------------
//  recompute metrics
//-------------------------------------------------

void menu_dats_view::recompute_metrics(uint32_t width, uint32_t height, float aspect)
{
	menu_textbox::recompute_metrics(width, height, aspect);

	m_tab_line = std::make_pair(1.0F, 0.0F);
	m_clicked_tab = -1;

	set_custom_space(2.0F * line_height() + 4.0F * tb_border(), line_height() + 3.0F * tb_border());
}

//-------------------------------------------------
//  perform our special rendering
//-------------------------------------------------

void menu_dats_view::custom_render(uint32_t flags, void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	float maxwidth;
	std::string_view const driver = m_issoft ? m_swinfo->longname : m_system->description;

	maxwidth = std::max(origx2 - origx1, get_string_width(driver) + (2.0F * lr_border()));

	// compute our bounds
	float x1 = 0.5F - (0.5F * maxwidth);
	float x2 = x1 + maxwidth;
	float y1 = origy1 - top;
	float y2 = y1 + (2.0F * tb_border()) + line_height();

	// draw a box
	ui().draw_outlined_box(container(), x1, y1, x2, y2, UI_GREEN_COLOR);

	draw_text_normal(
			driver,
			x1 + lr_border(), y1 + tb_border(), x2 - x1 - (2.0F * lr_border()),
			text_layout::text_justify::CENTER, ui::text_layout::word_wrapping::NEVER,
			ui().colors().text_color());

	// draw a box
	ui().draw_outlined_box(container(), x1, origy1 - line_height() - tb_border(), x2, origy1, ui().colors().background_color());

	// calculate geometry of tab line
	if (m_tab_line.first >= m_tab_line.second)
	{
		m_tab_line = std::make_pair(origy1 - line_height(), origy1);

		// FIXME: deal with overflow when there are a lot of tabs
		float total(0.0F);
		for (auto const &elem : m_items_list)
			total += get_string_width(elem.label);
		float const space((1.0F - total) / (m_items_list.size() * 2.0F));

		float left(x1 + (space * 0.5F));
		for (auto &elem : m_items_list)
		{
			float const width(get_string_width(elem.label));
			elem.bounds = std::make_pair(left, left + width + space);
			left += width + (space * 2.0F);
		}
	}

	// draw the text within it
	for (int i = 0; m_items_list.size() > i; ++i)
	{
		auto &elem(m_items_list[i]);

		rgb_t fgcolor;
		rgb_t bgcolor;
		if (i == m_current_tab)
		{
			fgcolor = rgb_t(0xff, 0xff, 0xff, 0x00);
			bgcolor = rgb_t(0xff, 0xff, 0xff, 0xff);
			ui().draw_textured_box(
					container(),
					elem.bounds.first, m_tab_line.first, elem.bounds.second, m_tab_line.second,
					bgcolor, rgb_t(255, 43, 43, 43),
					hilight_main_texture(), PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_TEXWRAP(1));
		}
		else
		{
			bool const hovered(pointer_in_rect(elem.bounds.first, m_tab_line.first, elem.bounds.second, m_tab_line.second));
			if ((i == m_clicked_tab) && hovered)
			{
				fgcolor = ui().colors().selected_color();
				bgcolor = ui().colors().selected_bg_color();
				highlight(elem.bounds.first, m_tab_line.first, elem.bounds.second, m_tab_line.second, bgcolor);
			}
			else if ((i == m_clicked_tab) || (hovered && pointer_idle()))
			{
				fgcolor = ui().colors().mouseover_color();
				bgcolor = ui().colors().mouseover_bg_color();
				highlight(elem.bounds.first, m_tab_line.first, elem.bounds.second, m_tab_line.second, bgcolor);
			}
			else
			{
				fgcolor = ui().colors().text_color();
				bgcolor = ui().colors().text_bg_color();
			}
		}

		ui().draw_text_full(
				container(),
				elem.label,
				elem.bounds.first, m_tab_line.first, elem.bounds.second - elem.bounds.first,
				text_layout::text_justify::CENTER, text_layout::word_wrapping::NEVER,
				mame_ui_manager::NORMAL, fgcolor, bgcolor,
				nullptr, nullptr,
				line_height());
	}

	// bottom
	if (!m_items_list.empty())
	{
		std::string const revision(util::string_format(_("Revision: %1$s"), m_items_list[m_current_tab].revision));
		float const width(get_text_width(
				revision,
				0.0F, 0.0F, 1.0F,
				text_layout::text_justify::CENTER, text_layout::word_wrapping::TRUNCATE));
		maxwidth = std::max(origx2 - origx1, width + (2.0F * lr_border()));

		// compute our bounds
		x1 = 0.5F - (0.5F * maxwidth);
		x2 = x1 + maxwidth;
		y1 = origy2 + tb_border();
		y2 = origy2 + bottom;

		// draw a box
		ui().draw_outlined_box(container(), x1, y1, x2, y2, UI_GREEN_COLOR);

		// draw the text within it
		draw_text_normal(
				revision,
				x1 + lr_border(), y1 + tb_border(), x2 - x1 - (2.0F * lr_border()),
				text_layout::text_justify::CENTER, text_layout::word_wrapping::TRUNCATE,
				ui().colors().text_color());
	}
}

//-------------------------------------------------
//  custom pointer handling
//-------------------------------------------------

std::tuple<int, bool, bool> menu_dats_view::custom_pointer_updated(bool changed, ui_event const &uievt)
{
	if (0 <= m_clicked_tab)
	{
		if ((ui_event::type::POINTER_ABORT != uievt.event_type) && uievt.pointer_released & 0x01)
		{
			// primary button released - take action if still on the tab
			bool const hit(pointer_in_rect(
						m_items_list[m_clicked_tab].bounds.first, m_tab_line.first,
						m_items_list[m_clicked_tab].bounds.second, m_tab_line.second));
			if (hit && (m_current_tab != m_clicked_tab))
			{
				m_current_tab = m_clicked_tab;
				m_tab_line = std::make_pair(1.0F, 0.0F);
				reset_layout();
			}
			m_clicked_tab = -1;
			return std::make_tuple(hit ? IPT_CUSTOM : IPT_INVALID, false, true);
		}
		else if ((ui_event::type::POINTER_ABORT == uievt.event_type) || (uievt.pointer_buttons & ~u32(0x01)))
		{
			// treat pressing another button as cancellation
			m_clicked_tab = -1;
			return std::make_tuple(IPT_INVALID, false, true);
		}
		return std::make_tuple(IPT_INVALID, true, false);
	}
	else if (pointer_idle() && (uievt.pointer_pressed & 0x01) && !(uievt.pointer_buttons & ~u32(0x01)))
	{
		// primary click - see if it's over a tab
		auto const [x, y] = pointer_location();
		if ((y >= m_tab_line.first) && (y < m_tab_line.second))
		{
			for (int i = 0; m_items_list.size() > i; ++i)
			{
				if ((x >= m_items_list[i].bounds.first) && (x < m_items_list[i].bounds.second))
				{
					m_clicked_tab = i;
					return std::make_tuple(IPT_INVALID, true, true);
				}
			}
		}
	}

	// let the base class have a look
	return menu_textbox::custom_pointer_updated(changed, uievt);
}

//-------------------------------------------------
//  populate selected DAT text
//-------------------------------------------------

void menu_dats_view::populate_text(std::optional<text_layout> &layout, float &width, int &lines)
{
	if (!layout || (layout->width() != width))
	{
		m_tab_line = std::make_pair(1.0F, 0.0F);
		m_clicked_tab = -1;

		std::string buffer;
		if (!m_items_list.empty())
		{
			if (0 > m_items_list[m_current_tab].option)
				buffer = m_swinfo->infotext;
			else
				mame_machine_manager::instance()->lua()->call_plugin("data", m_items_list[m_current_tab].option, buffer);
		}
		layout.emplace(create_layout(width));
		add_info_text(*layout, buffer, ui().colors().text_color());
		lines = std::numeric_limits<int>::max();
	}
}

} // namespace ui
