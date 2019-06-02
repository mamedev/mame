// license:BSD-3-Clause
// copyright-holders:Maurizio Petrarota
/*********************************************************************

    ui/datmenu.cpp

    Internal UI user interface.

*********************************************************************/

#include "emu.h"

#include "ui/ui.h"
#include "ui/datmenu.h"
#include "ui/utils.h"

#include "mame.h"
#include "rendfont.h"
#include "softlist.h"
#include "uiinput.h"
#include "luaengine.h"

#include <cmath>


namespace ui {
//-------------------------------------------------
//  ctor / dtor
//-------------------------------------------------

menu_dats_view::menu_dats_view(mame_ui_manager &mui, render_container &container, const game_driver *driver)
	: menu(mui, container)
	, m_actual(0)
	, m_driver((driver == nullptr) ? &mui.machine().system() : driver)
	, m_swinfo(nullptr)
	, m_issoft(false)

{
	for (device_image_interface& image : image_interface_iterator(mui.machine().root_device()))
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
	if (mame_machine_manager::instance()->lua()->call_plugin("data_list", driver ? driver->name : "", lua_list))
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

menu_dats_view::menu_dats_view(mame_ui_manager &mui, render_container &container, const ui_software_info *swinfo, const game_driver *driver)
	: menu(mui, container)
	, m_actual(0)
	, m_driver((driver == nullptr) ? &mui.machine().system() : driver)
	, m_swinfo(swinfo)
	, m_list(swinfo->listname)
	, m_short(swinfo->shortname)
	, m_long(swinfo->longname)
	, m_parent(swinfo->parentname)
	, m_issoft(true)

{
	if (swinfo != nullptr && !swinfo->usage.empty())
		m_items_list.emplace_back(_("Software Usage"), 0, "");
	std::vector<std::string> lua_list;
	if(mame_machine_manager::instance()->lua()->call_plugin("data_list", std::string(m_short).append(1, ',').append(m_list).c_str(), lua_list))
	{
		int count = 1;
		for(std::string &item : lua_list)
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
//  handle
//-------------------------------------------------

void menu_dats_view::handle()
{
	const event *menu_event = process(FLAG_UI_DATS);
	if (menu_event != nullptr)
	{
		if (menu_event->iptkey == IPT_UI_LEFT && m_actual > 0)
		{
			m_actual--;
			reset(reset_options::SELECT_FIRST);
		}

		if (menu_event->iptkey == IPT_UI_RIGHT && m_actual < m_items_list.size() - 1)
		{
			m_actual++;
			reset(reset_options::SELECT_FIRST);
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

	(m_issoft == true) ? get_data_sw() : get_data();

	item_append(menu_item_type::SEPARATOR, (FLAG_UI_DATS | FLAG_LEFT_ARROW | FLAG_RIGHT_ARROW));
	customtop = 2.0f * ui().get_line_height() + 4.0f * UI_BOX_TB_BORDER;
	custombottom = ui().get_line_height() + 3.0f * UI_BOX_TB_BORDER;

	if (!paused)
		machine().resume();
}

//-------------------------------------------------
//  draw - draw dats menu
//-------------------------------------------------

void menu_dats_view::draw(uint32_t flags)
{
	float const line_height = ui().get_line_height();
	float const ud_arrow_width = line_height * machine().render().ui_aspect();
	float const gutter_width = 0.52f * line_height * machine().render().ui_aspect();
	float const visible_width = 1.0f - (2.0f * UI_BOX_LR_BORDER);
	float const visible_left = (1.0f - visible_width) * 0.5f;
	float const extra_height = 2.0f * line_height;
	float const visible_extra_menu_height = get_customtop() + get_custombottom() + extra_height;
	int const visible_items = item_count() - 2;

	// determine effective positions taking into account the hilighting arrows
	float const effective_width = visible_width - 2.0f * gutter_width;
	float const effective_left = visible_left + gutter_width;

	draw_background();
	map_mouse();

	// account for extra space at the top and bottom
	float visible_main_menu_height = 1.0f - 2.0f * UI_BOX_TB_BORDER - visible_extra_menu_height;
	m_visible_lines = int(std::trunc(visible_main_menu_height / line_height));
	visible_main_menu_height = float(m_visible_lines) * line_height;

	// compute top/left of inner menu area by centering, if the menu is at the bottom of the extra, adjust
	float const visible_top = ((1.0f - (visible_main_menu_height + visible_extra_menu_height)) * 0.5f) + get_customtop();

	// compute left box size
	float x1 = visible_left;
	float y1 = visible_top - UI_BOX_TB_BORDER;
	float x2 = x1 + visible_width;
	float y2 = visible_top + visible_main_menu_height + UI_BOX_TB_BORDER + extra_height;
	float line = visible_top + float(m_visible_lines) * line_height;

	ui().draw_outlined_box(container(), x1, y1, x2, y2, UI_BACKGROUND_COLOR);

	m_visible_lines = (std::min)(visible_items, m_visible_lines);
	top_line = (std::max)(0, top_line);
	if (top_line + m_visible_lines >= visible_items)
		top_line = visible_items - m_visible_lines;

	clear_hover();
	int const n_loop = (std::min)(visible_items, m_visible_lines);
	for (int linenum = 0; linenum < n_loop; linenum++)
	{
		float const line_y = visible_top + (float)linenum * line_height;
		int const itemnum = top_line + linenum;
		menu_item const &pitem = item(itemnum);
		char const *const itemtext = pitem.text.c_str();
		float const line_x0 = x1 + 0.5f * UI_LINE_WIDTH;
		float const line_y0 = line_y;
		float const line_x1 = x2 - 0.5f * UI_LINE_WIDTH;
		float const line_y1 = line_y + line_height;

		rgb_t fgcolor = UI_TEXT_COLOR;
		rgb_t bgcolor = UI_TEXT_BG_COLOR;

		if (!linenum && top_line)
		{
			// if we're on the top line, display the up arrow
			if (mouse_in_rect(line_x0, line_y0, line_x1, line_y1))
			{
				fgcolor = UI_MOUSEOVER_COLOR;
				bgcolor = UI_MOUSEOVER_BG_COLOR;
				highlight(line_x0, line_y0, line_x1, line_y1, bgcolor);
				set_hover(HOVER_ARROW_UP);
			}
			draw_arrow(
					0.5f * (x1 + x2) - 0.5f * ud_arrow_width, line_y + 0.25f * line_height,
					0.5f * (x1 + x2) + 0.5f * ud_arrow_width, line_y + 0.75f * line_height,
					fgcolor, ROT0);
		}
		else if ((linenum == m_visible_lines - 1) && (itemnum != visible_items - 1))
		{
			// if we're on the bottom line, display the down arrow
			if (mouse_in_rect(line_x0, line_y0, line_x1, line_y1))
			{
				fgcolor = UI_MOUSEOVER_COLOR;
				bgcolor = UI_MOUSEOVER_BG_COLOR;
				highlight(line_x0, line_y0, line_x1, line_y1, bgcolor);
				set_hover(HOVER_ARROW_DOWN);
			}
			draw_arrow(
					0.5f * (x1 + x2) - 0.5f * ud_arrow_width, line_y + 0.25f * line_height,
					0.5f * (x1 + x2) + 0.5f * ud_arrow_width, line_y + 0.75f * line_height,
					fgcolor, ROT0 ^ ORIENTATION_FLIP_Y);
		}
		else if (pitem.subtext.empty())
		{
			// draw dats text
			ui().draw_text_full(
					container(), itemtext,
					effective_left, line_y, effective_width,
					ui::text_layout::LEFT, ui::text_layout::NEVER,
					mame_ui_manager::NORMAL, fgcolor, bgcolor,
					nullptr, nullptr);
		}
	}

	for (size_t count = visible_items; count < item_count(); count++)
	{
		menu_item const &pitem = item(count);
		char const *const itemtext = pitem.text.c_str();
		float const line_x0 = x1 + 0.5f * UI_LINE_WIDTH;
		float const line_y0 = line;
		float const line_x1 = x2 - 0.5f * UI_LINE_WIDTH;
		float const line_y1 = line + line_height;
		rgb_t const fgcolor = UI_SELECTED_COLOR;
		rgb_t const bgcolor = UI_SELECTED_BG_COLOR;

		if (mouse_in_rect(line_x0, line_y0, line_x1, line_y1) && is_selectable(pitem))
			set_hover(count);

		if (pitem.type == menu_item_type::SEPARATOR)
		{
			container().add_line(
					visible_left, line + 0.5f * line_height, visible_left + visible_width, line + 0.5f * line_height,
					UI_LINE_WIDTH, UI_TEXT_COLOR, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
		}
		else
		{
			highlight(line_x0, line_y0, line_x1, line_y1, bgcolor);
			ui().draw_text_full(
					container(), itemtext,
					effective_left, line, effective_width,
					ui::text_layout::CENTER, ui::text_layout::TRUNCATE,
					mame_ui_manager::NORMAL, fgcolor, bgcolor,
					nullptr, nullptr);
		}
		line += line_height;
	}

	// if there is something special to add, do it by calling the virtual method
	custom_render(get_selection_ref(), get_customtop(), get_custombottom(), x1, y1, x2, y2);

	// return the number of visible lines, minus 1 for top arrow and 1 for bottom arrow
	m_visible_items = m_visible_lines - (top_line != 0) - (top_line + m_visible_lines != visible_items);
}

//-------------------------------------------------
//  perform our special rendering
//-------------------------------------------------

void menu_dats_view::custom_render(void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	float maxwidth = origx2 - origx1;
	float width;
	std::string driver = (m_issoft == true) ? m_swinfo->longname : m_driver->type.fullname();

	ui().draw_text_full(container(), driver.c_str(), 0.0f, 0.0f, 1.0f, ui::text_layout::CENTER, ui::text_layout::TRUNCATE,
		mame_ui_manager::NONE, rgb_t::white(), rgb_t::black(), &width, nullptr);
	width += 2 * UI_BOX_LR_BORDER;
	maxwidth = std::max(maxwidth, width);

	// compute our bounds
	float x1 = 0.5f - 0.5f * maxwidth;
	float x2 = x1 + maxwidth;
	float y1 = origy1 - top;
	float y2 = origy1 - 2.0f * UI_BOX_TB_BORDER - ui().get_line_height();

	// draw a box
	ui().draw_outlined_box(container(), x1, y1, x2, y2, UI_GREEN_COLOR);

	// take off the borders
	x1 += UI_BOX_LR_BORDER;
	x2 -= UI_BOX_LR_BORDER;
	y1 += UI_BOX_TB_BORDER;

	ui().draw_text_full(container(), driver.c_str(), x1, y1, x2 - x1, ui::text_layout::CENTER, ui::text_layout::NEVER,
		mame_ui_manager::NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, nullptr, nullptr);

	maxwidth = 0;
	for (auto & elem : m_items_list)
	{
		ui().draw_text_full(container(), elem.label.c_str(), 0.0f, 0.0f, 1.0f, ui::text_layout::CENTER, ui::text_layout::NEVER,
			mame_ui_manager::NONE, rgb_t::white(), rgb_t::black(), &width, nullptr);
		maxwidth += width;
	}

	float space = (1.0f - maxwidth) / (m_items_list.size() * 2);

	// compute our bounds
	x1 -= UI_BOX_LR_BORDER;
	x2 += UI_BOX_LR_BORDER;
	y1 = y2 + UI_BOX_TB_BORDER;
	y2 += ui().get_line_height() + 2.0f * UI_BOX_TB_BORDER;

	// draw a box
	ui().draw_outlined_box(container(), x1, y1, x2, y2, UI_BACKGROUND_COLOR);

	// take off the borders
	y1 += UI_BOX_TB_BORDER;

	// draw the text within it
	int x = 0;
	for (auto & elem : m_items_list)
	{
		x1 += space;
		rgb_t fcolor = (m_actual == x) ? rgb_t(0xff, 0xff, 0xff, 0x00) : UI_TEXT_COLOR;
		rgb_t bcolor = (m_actual == x) ? rgb_t(0xff, 0xff, 0xff, 0xff) : UI_TEXT_BG_COLOR;
		ui().draw_text_full(container(), elem.label.c_str(), x1, y1, 1.0f, ui::text_layout::LEFT, ui::text_layout::NEVER, mame_ui_manager::NONE, fcolor, bcolor, &width, nullptr);

		if (bcolor != UI_TEXT_BG_COLOR)
			ui().draw_textured_box(container(), x1 - (space / 2), y1, x1 + width + (space / 2), y2, bcolor, rgb_t(255, 43, 43, 43),
				hilight_main_texture(), PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_TEXWRAP(1));

		ui().draw_text_full(container(), elem.label.c_str(), x1, y1, 1.0f, ui::text_layout::LEFT, ui::text_layout::NEVER, mame_ui_manager::NORMAL, fcolor, bcolor, &width, nullptr);
		x1 += width + space;
		++x;
	}

	// bottom
	std::string revision;
	revision.assign(_("Revision: ")).append(m_items_list[m_actual].revision);
	ui().draw_text_full(container(), revision.c_str(), 0.0f, 0.0f, 1.0f, ui::text_layout::CENTER, ui::text_layout::TRUNCATE, mame_ui_manager::NONE, rgb_t::white(), rgb_t::black(), &width, nullptr);
	width += 2 * UI_BOX_LR_BORDER;
	maxwidth = std::max(origx2 - origx1, width);

	// compute our bounds
	x1 = 0.5f - 0.5f * maxwidth;
	x2 = x1 + maxwidth;
	y1 = origy2 + UI_BOX_TB_BORDER;
	y2 = origy2 + bottom;

	// draw a box
	ui().draw_outlined_box(container(), x1, y1, x2, y2, UI_GREEN_COLOR);

	// take off the borders
	x1 += UI_BOX_LR_BORDER;
	x2 -= UI_BOX_LR_BORDER;
	y1 += UI_BOX_TB_BORDER;

	// draw the text within it
	ui().draw_text_full(container(), revision.c_str(), x1, y1, x2 - x1, ui::text_layout::CENTER, ui::text_layout::TRUNCATE,
		mame_ui_manager::NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, nullptr, nullptr);
}

//-------------------------------------------------
//  load data from DATs
//-------------------------------------------------

void menu_dats_view::get_data()
{
	std::vector<int> xstart, xend;
	std::string buffer;
	mame_machine_manager::instance()->lua()->call_plugin("data", m_items_list[m_actual].option, buffer);

	float const line_height = ui().get_line_height();
	float const gutter_width = 0.52f * line_height * machine().render().ui_aspect();
	float const visible_width = 1.0f - (2.0f * UI_BOX_LR_BORDER);
	float const effective_width = visible_width - 2.0f * gutter_width;

	auto lines = ui().wrap_text(container(), buffer.c_str(), 0.0f, 0.0f, effective_width, xstart, xend);
	for (int x = 0; x < lines; ++x)
	{
		std::string tempbuf(buffer.substr(xstart[x], xend[x] - xstart[x]));
		if ((tempbuf[0] != '#') || x)
			item_append(tempbuf, "", (FLAG_UI_DATS | FLAG_DISABLE), (void *)(uintptr_t)(x + 1));
	}
}

void menu_dats_view::get_data_sw()
{
	std::vector<int> xstart;
	std::vector<int> xend;
	std::string buffer;
	if (m_items_list[m_actual].option == 0)
		buffer = m_swinfo->usage;
	else
		mame_machine_manager::instance()->lua()->call_plugin("data", m_items_list[m_actual].option - 1, buffer);

	auto lines = ui().wrap_text(container(), buffer.c_str(), 0.0f, 0.0f, 1.0f - (4.0f * UI_BOX_LR_BORDER), xstart, xend);
	for (int x = 0; x < lines; ++x)
	{
		std::string tempbuf(buffer.substr(xstart[x], xend[x] - xstart[x]));
		item_append(tempbuf, "", (FLAG_UI_DATS | FLAG_DISABLE), (void *)(uintptr_t)(x + 1));
	}
}

} // namespace ui
