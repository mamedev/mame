// license:BSD-3-Clause
// copyright-holders:Maurizio Petrarota
/*********************************************************************

    ui/datmenu.cpp

    Internal UI user interface.

*********************************************************************/

#include "emu.h"

#include "ui/ui.h"
#include "ui/datfile.h"
#include "ui/datmenu.h"
#include "ui/utils.h"

#include "mame.h"
#include "rendfont.h"
#include "softlist.h"

namespace ui {
//-------------------------------------------------
//  ctor / dtor
//-------------------------------------------------

menu_dats_view::menu_dats_view(mame_ui_manager &mui, render_container *container, const game_driver *driver)
	: menu(mui, container)
	, m_actual(0)
	, m_driver((driver == nullptr) ? &mui.machine().system() : driver)
	, m_issoft(false)

{
	for (device_image_interface &image : image_interface_iterator(mui.machine().root_device()))
	{
		if (image.filename())
		{
			m_list = strensure(image.software_list_name());
			m_short = strensure(image.software_entry()->shortname());
			m_long = strensure(image.software_entry()->longname());
			m_parent = strensure(image.software_entry()->parentname());
		}
	}

	init_items();
}

//-------------------------------------------------
//  ctor
//-------------------------------------------------

menu_dats_view::menu_dats_view(mame_ui_manager &mui, render_container *container, ui_software_info *swinfo, const game_driver *driver)
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
	if (mame_machine_manager::instance()->datfile().has_software(m_list, m_short, m_parent))
		m_items_list.emplace_back(_("Software History"), UI_HISTORY_LOAD, mame_machine_manager::instance()->datfile().rev_history());
	if (swinfo != nullptr && !swinfo->usage.empty())
		m_items_list.emplace_back(_("Software Usage"), 0, "");
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
	auto menu_event = process(FLAG_UI_DATS);
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

void menu_dats_view::populate()
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
//  perform our special rendering
//-------------------------------------------------

void menu_dats_view::custom_render(void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	float maxwidth = origx2 - origx1;
	float width;
	std::string driver = (m_issoft == true) ? m_swinfo->longname : m_driver->description;

	ui().draw_text_full(container, driver.c_str(), 0.0f, 0.0f, 1.0f, ui::text_layout::CENTER, ui::text_layout::TRUNCATE,
		mame_ui_manager::NONE, rgb_t::white, rgb_t::black, &width, nullptr);
	width += 2 * UI_BOX_LR_BORDER;
	maxwidth = MAX(maxwidth, width);

	// compute our bounds
	float x1 = 0.5f - 0.5f * maxwidth;
	float x2 = x1 + maxwidth;
	float y1 = origy1 - top;
	float y2 = origy1 - 2.0f * UI_BOX_TB_BORDER - ui().get_line_height();

	// draw a box
	ui().draw_outlined_box(container, x1, y1, x2, y2, UI_GREEN_COLOR);

	// take off the borders
	x1 += UI_BOX_LR_BORDER;
	x2 -= UI_BOX_LR_BORDER;
	y1 += UI_BOX_TB_BORDER;

	ui().draw_text_full(container, driver.c_str(), x1, y1, x2 - x1, ui::text_layout::CENTER, ui::text_layout::NEVER,
		mame_ui_manager::NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, nullptr, nullptr);

	maxwidth = 0;
	for (auto & elem : m_items_list)
	{
		ui().draw_text_full(container, elem.label.c_str(), 0.0f, 0.0f, 1.0f, ui::text_layout::CENTER, ui::text_layout::NEVER,
			mame_ui_manager::NONE, rgb_t::white, rgb_t::black, &width, nullptr);
		maxwidth += width;
	}

	float space = (1.0f - maxwidth) / (m_items_list.size() * 2);

	// compute our bounds
	x1 -= UI_BOX_LR_BORDER;
	x2 += UI_BOX_LR_BORDER;
	y1 = y2 + UI_BOX_TB_BORDER;
	y2 += ui().get_line_height() + 2.0f * UI_BOX_TB_BORDER;

	// draw a box
	ui().draw_outlined_box(container, x1, y1, x2, y2, UI_BACKGROUND_COLOR);

	// take off the borders
	y1 += UI_BOX_TB_BORDER;

	// draw the text within it
	int x = 0;
	for (auto & elem : m_items_list)
	{
		x1 += space;
		rgb_t fcolor = (m_actual == x) ? rgb_t(0xff, 0xff, 0xff, 0x00) : UI_TEXT_COLOR;
		rgb_t bcolor = (m_actual == x) ? rgb_t(0xff, 0xff, 0xff, 0xff) : UI_TEXT_BG_COLOR;
		ui().draw_text_full(container, elem.label.c_str(), x1, y1, 1.0f, ui::text_layout::LEFT, ui::text_layout::NEVER, mame_ui_manager::NONE, fcolor, bcolor, &width, nullptr);

		if (bcolor != UI_TEXT_BG_COLOR)
			ui().draw_textured_box(container, x1 - (space / 2), y1, x1 + width + (space / 2), y2, bcolor, rgb_t(255, 43, 43, 43),
				hilight_main_texture, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_TEXWRAP(TRUE));

		ui().draw_text_full(container, elem.label.c_str(), x1, y1, 1.0f, ui::text_layout::LEFT, ui::text_layout::NEVER, mame_ui_manager::NORMAL, fcolor, bcolor, &width, nullptr);
		x1 += width + space;
		++x;
	}

	// bottom
	std::string revision;
	revision.assign(_("Revision: ")).append(m_items_list[m_actual].revision);
	ui().draw_text_full(container, revision.c_str(), 0.0f, 0.0f, 1.0f, ui::text_layout::CENTER, ui::text_layout::TRUNCATE, mame_ui_manager::NONE, rgb_t::white, rgb_t::black, &width, nullptr);
	width += 2 * UI_BOX_LR_BORDER;
	maxwidth = MAX(origx2 - origx1, width);

	// compute our bounds
	x1 = 0.5f - 0.5f * maxwidth;
	x2 = x1 + maxwidth;
	y1 = origy2 + UI_BOX_TB_BORDER;
	y2 = origy2 + bottom;

	// draw a box
	ui().draw_outlined_box(container, x1, y1, x2, y2, UI_GREEN_COLOR);

	// take off the borders
	x1 += UI_BOX_LR_BORDER;
	x2 -= UI_BOX_LR_BORDER;
	y1 += UI_BOX_TB_BORDER;

	// draw the text within it
	ui().draw_text_full(container, revision.c_str(), x1, y1, x2 - x1, ui::text_layout::CENTER, ui::text_layout::TRUNCATE,
		mame_ui_manager::NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, nullptr, nullptr);
}

//-------------------------------------------------
//  load data from DATs
//-------------------------------------------------

void menu_dats_view::get_data()
{
	std::vector<int> xstart, xend;
	std::string buffer;
	if (m_items_list[m_actual].option == UI_COMMAND_LOAD)
	{
		std::vector<std::string> m_item;
		mame_machine_manager::instance()->datfile().command_sub_menu(m_driver, m_item);
		if (!m_item.empty())
		{
			for (auto & e : m_item)
			{
				std::string t_buffer;
				buffer.append(e).append("\n");
				mame_machine_manager::instance()->datfile().load_command_info(t_buffer, e);
				if (!t_buffer.empty()) buffer.append(t_buffer).append("\n");
			}
			convert_command_glyph(buffer);
		}
	}
	else
		mame_machine_manager::instance()->datfile().load_data_info(m_driver, buffer, m_items_list[m_actual].option);

	auto lines = ui().wrap_text(container, buffer.c_str(), 0.0f, 0.0f, 1.0f - (4.0f * UI_BOX_LR_BORDER), xstart, xend);
	for (int x = 0; x < lines; ++x)
	{
		std::string tempbuf(buffer.substr(xstart[x], xend[x] - xstart[x]));
		item_append(tempbuf, "", (FLAG_UI_DATS | FLAG_DISABLE), (void *)(FPTR)(x + 1));
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
	{
		if (m_swinfo->startempty == 1)
			mame_machine_manager::instance()->datfile().load_data_info(m_swinfo->driver, buffer, UI_HISTORY_LOAD);
		else
			mame_machine_manager::instance()->datfile().load_software_info(m_swinfo->listname, buffer, m_swinfo->shortname, m_swinfo->parentname);
	}

	auto lines = ui().wrap_text(container, buffer.c_str(), 0.0f, 0.0f, 1.0f - (4.0f * UI_BOX_LR_BORDER), xstart, xend);
	for (int x = 0; x < lines; ++x)
	{
		std::string tempbuf(buffer.substr(xstart[x], xend[x] - xstart[x]));
		item_append(tempbuf, "", (FLAG_UI_DATS | FLAG_DISABLE), (void *)(FPTR)(x + 1));
	}
}

void menu_dats_view::init_items()
{
	datfile_manager &datfile = mame_machine_manager::instance()->datfile();
	if (datfile.has_history(m_driver))
		m_items_list.emplace_back(_("History"), UI_HISTORY_LOAD, datfile.rev_history());
	if (datfile.has_mameinfo(m_driver))
		m_items_list.emplace_back(_("Mameinfo"), UI_MAMEINFO_LOAD, datfile.rev_mameinfo());
	if (datfile.has_messinfo(m_driver))
		m_items_list.emplace_back(_("Messinfo"), UI_MESSINFO_LOAD, datfile.rev_messinfo());
	if (datfile.has_sysinfo(m_driver))
		m_items_list.emplace_back(_("Sysinfo"), UI_SYSINFO_LOAD, datfile.rev_sysinfo());
	if (datfile.has_story(m_driver))
		m_items_list.emplace_back(_("Mamescore"), UI_STORY_LOAD, datfile.rev_storyinfo());
	if (datfile.has_gameinit(m_driver))
		m_items_list.emplace_back(_("Gameinit"), UI_GINIT_LOAD, datfile.rev_ginitinfo());
	if (datfile.has_command(m_driver))
		m_items_list.emplace_back(_("Command"), UI_COMMAND_LOAD, "");
}

} // namespace ui
