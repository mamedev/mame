// license:BSD-3-Clause
// copyright-holders:Maurizio Petrarota
/*********************************************************************

    ui/datmenu.cpp

    Internal UI user interface.

*********************************************************************/

#include "emu.h"
#include "ui/ui.h"
#include "ui/menu.h"
#include "drivenum.h"
#include "rendfont.h"
#include "ui/datfile.h"
#include "ui/datmenu.h"
#include "ui/utils.h"
#include "softlist.h"

//-------------------------------------------------
//  ctor / dtor
//-------------------------------------------------

ui_menu_dats_view::ui_menu_dats_view(running_machine &machine, render_container *container, const game_driver *driver)
	: ui_menu(machine, container)
	, m_actual(0)
	, m_driver((driver == nullptr) ? &machine.system() : driver)
	, m_issoft(false)

{
	image_interface_iterator iter(machine.root_device());
	for (device_image_interface *image = iter.first(); image != nullptr; image = iter.next())
	{
		if (image->filename())
		{
			m_list = strensure(image->software_list_name());
			m_short = strensure(image->software_entry()->shortname());
			m_long = strensure(image->software_entry()->longname());
			m_parent = strensure(image->software_entry()->parentname());
		}
	}

	init_items();
}

//-------------------------------------------------
//  ctor
//-------------------------------------------------

ui_menu_dats_view::ui_menu_dats_view(running_machine &machine, render_container *container, ui_software_info *swinfo, const game_driver *driver)
	: ui_menu(machine, container)
	, m_actual(0)
	, m_driver((driver == nullptr) ? &machine.system() : driver)
	, m_swinfo(swinfo)
	, m_list(swinfo->listname)
	, m_short(swinfo->shortname)
	, m_long(swinfo->longname)
	, m_parent(swinfo->parentname)
	, m_issoft(true)

{
	if (machine.datfile().has_software(m_list, m_short, m_parent))
		m_items_list.emplace_back(_("Software History"), UI_HISTORY_LOAD, machine.datfile().rev_history());
	if (swinfo != nullptr && !swinfo->usage.empty())
		m_items_list.emplace_back(_("Software Usage"), 0, "");
}

//-------------------------------------------------
//  dtor
//-------------------------------------------------

ui_menu_dats_view::~ui_menu_dats_view()
{
}

//-------------------------------------------------
//  handle
//-------------------------------------------------

void ui_menu_dats_view::handle()
{
	const ui_menu_event *m_event = process(MENU_FLAG_UI_DATS);
	if (m_event != nullptr)
	{
		if (m_event->iptkey == IPT_UI_LEFT && m_actual > 0)
		{
			m_actual--;
			reset(UI_MENU_RESET_SELECT_FIRST);
		}

		if (m_event->iptkey == IPT_UI_RIGHT && m_actual < m_items_list.size() - 1)
		{
			m_actual++;
			reset(UI_MENU_RESET_SELECT_FIRST);
		}
	}
}

//-------------------------------------------------
//  populate
//-------------------------------------------------

void ui_menu_dats_view::populate()
{
	bool paused = machine().paused();
	if (!paused)
		machine().pause();

	(m_issoft == true) ? get_data_sw() : get_data();

	item_append(MENU_SEPARATOR_ITEM, nullptr, (MENU_FLAG_UI_DATS | MENU_FLAG_LEFT_ARROW | MENU_FLAG_RIGHT_ARROW), nullptr);
	customtop = 2.0f * machine().ui().get_line_height() + 4.0f * UI_BOX_TB_BORDER;
	custombottom = machine().ui().get_line_height() + 3.0f * UI_BOX_TB_BORDER;

	if (!paused)
		machine().resume();
}

//-------------------------------------------------
//  perform our special rendering
//-------------------------------------------------

void ui_menu_dats_view::custom_render(void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	ui_manager &mui = machine().ui();
	float maxwidth = origx2 - origx1;
	float width;
	std::string driver = (m_issoft == true) ? m_swinfo->longname : m_driver->description;

	mui.draw_text_full(container, driver.c_str(), 0.0f, 0.0f, 1.0f, JUSTIFY_CENTER, WRAP_TRUNCATE,
		DRAW_NONE, ARGB_WHITE, ARGB_BLACK, &width, nullptr);
	width += 2 * UI_BOX_LR_BORDER;
	maxwidth = MAX(origx2 - origx1, width);

	// compute our bounds
	float x1 = 0.5f - 0.5f * maxwidth;
	float x2 = x1 + maxwidth;
	float y1 = origy1 - top;
	float y2 = origy1 - 2.0f * UI_BOX_TB_BORDER - mui.get_line_height();

	// draw a box
	mui.draw_outlined_box(container, x1, y1, x2, y2, UI_GREEN_COLOR);

	// take off the borders
	x1 += UI_BOX_LR_BORDER;
	x2 -= UI_BOX_LR_BORDER;
	y1 += UI_BOX_TB_BORDER;

	mui.draw_text_full(container, driver.c_str(), x1, y1, x2 - x1, JUSTIFY_CENTER, WRAP_NEVER,
		DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, nullptr, nullptr);

	maxwidth = 0;
	for (auto & elem : m_items_list)
	{
		mui.draw_text_full(container, elem.label.c_str(), 0.0f, 0.0f, 1.0f, JUSTIFY_CENTER, WRAP_NEVER,
			DRAW_NONE, ARGB_WHITE, ARGB_BLACK, &width, nullptr);
		maxwidth += width;
	}

	float space = (1.0f - maxwidth) / (m_items_list.size() * 2);

	// compute our bounds
	x1 -= UI_BOX_LR_BORDER;
	x2 += UI_BOX_LR_BORDER;
	y1 = y2 + UI_BOX_TB_BORDER;
	y2 += mui.get_line_height() + 2.0f * UI_BOX_TB_BORDER;

	// draw a box
	mui.draw_outlined_box(container, x1, y1, x2, y2, UI_BACKGROUND_COLOR);

	// take off the borders
	x2 -= UI_BOX_LR_BORDER;
	y1 += UI_BOX_TB_BORDER;

	// draw the text within it
	int x = 0;
	for (auto & elem : m_items_list)
	{
		x1 += space;
		rgb_t fcolor = (m_actual == x) ? rgb_t(0xff, 0xff, 0xff, 0x00) : UI_TEXT_COLOR;
		rgb_t bcolor = (m_actual == x) ? rgb_t(0xff, 0xff, 0xff, 0xff) : UI_TEXT_BG_COLOR;
		mui.draw_text_full(container, elem.label.c_str(), x1, y1, 1.0f, JUSTIFY_LEFT, WRAP_NEVER, DRAW_NONE, fcolor, bcolor, &width, nullptr);

		if (bcolor != UI_TEXT_BG_COLOR)
			mui.draw_textured_box(container, x1 - (space / 2), y1, x1 + width + (space / 2), y2, bcolor, rgb_t(255, 43, 43, 43),
				hilight_main_texture, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_TEXWRAP(TRUE));

		mui.draw_text_full(container, elem.label.c_str(), x1, y1, 1.0f, JUSTIFY_LEFT, WRAP_NEVER, DRAW_NORMAL, fcolor, bcolor, &width, nullptr);
		x1 += width + space;
		++x;
	}

	// bottom
	std::string revision;
	revision.assign(_("Revision: ")).append(m_items_list[m_actual].revision);
	mui.draw_text_full(container, revision.c_str(), 0.0f, 0.0f, 1.0f, JUSTIFY_CENTER, WRAP_TRUNCATE, DRAW_NONE, ARGB_WHITE, ARGB_BLACK, &width, nullptr);
	width += 2 * UI_BOX_LR_BORDER;
	maxwidth = MAX(origx2 - origx1, width);

	// compute our bounds
	x1 = 0.5f - 0.5f * maxwidth;
	x2 = x1 + maxwidth;
	y1 = origy2 + UI_BOX_TB_BORDER;
	y2 = origy2 + bottom;

	// draw a box
	mui.draw_outlined_box(container, x1, y1, x2, y2, UI_GREEN_COLOR);

	// take off the borders
	x1 += UI_BOX_LR_BORDER;
	x2 -= UI_BOX_LR_BORDER;
	y1 += UI_BOX_TB_BORDER;

	// draw the text within it
	mui.draw_text_full(container, revision.c_str(), x1, y1, x2 - x1, JUSTIFY_CENTER, WRAP_TRUNCATE,
		DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, nullptr, nullptr);
}

//-------------------------------------------------
//  load data from DATs
//-------------------------------------------------

void ui_menu_dats_view::get_data()
{
	std::vector<int> xstart;
	std::vector<int> xend;
	std::string buffer;
	std::vector<std::string> m_item;
	if (m_items_list[m_actual].option == UI_COMMAND_LOAD)
	{
		machine().datfile().command_sub_menu(m_driver, m_item);
		if (!m_item.empty())
		{
			for (size_t x = 0; x < m_item.size(); ++x)
			{
				std::string t_buffer;
				buffer.append(m_item[x]).append("\n");
				machine().datfile().load_command_info(t_buffer, m_item[x]);
				if (!t_buffer.empty())
					buffer.append(t_buffer).append("\n");
			}
			convert_command_glyph(buffer);
		}
	}
	else
		machine().datfile().load_data_info(m_driver, buffer, m_items_list[m_actual].option);

	int totallines = machine().ui().wrap_text(container, buffer.c_str(), 0.0f, 0.0f, 1.0f - (4.0f * UI_BOX_LR_BORDER), xstart, xend);
	for (int x = 0; x < totallines; ++x)
	{
		std::string tempbuf(buffer.substr(xstart[x], xend[x] - xstart[x]));
		item_append(tempbuf.c_str(), nullptr, (MENU_FLAG_UI_DATS | MENU_FLAG_DISABLE), (void *)(FPTR)(x + 1));
	}
}

void ui_menu_dats_view::get_data_sw()
{
	std::vector<int> xstart;
	std::vector<int> xend;
	std::string buffer;
	if (m_items_list[m_actual].option == 0)
		buffer = m_swinfo->usage;
	else
	{
		if (m_swinfo->startempty == 1)
			machine().datfile().load_data_info(m_swinfo->driver, buffer, UI_HISTORY_LOAD);
		else
			machine().datfile().load_software_info(m_swinfo->listname, buffer, m_swinfo->shortname, m_swinfo->parentname);
	}

	int totallines = machine().ui().wrap_text(container, buffer.c_str(), 0.0f, 0.0f, 1.0f - (4.0f * UI_BOX_LR_BORDER), xstart, xend);
	for (int x = 0; x < totallines; ++x)
	{
		std::string tempbuf(buffer.substr(xstart[x], xend[x] - xstart[x]));
		item_append(tempbuf.c_str(), nullptr, (MENU_FLAG_UI_DATS | MENU_FLAG_DISABLE), (void *)(FPTR)(x + 1));
	}
}

void ui_menu_dats_view::init_items()
{
	datfile_manager &datfile = machine().datfile();
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
