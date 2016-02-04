// license:BSD-3-Clause
// copyright-holders:Dankan1890
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

/**************************************************
    MENU COMMAND
**************************************************/
//-------------------------------------------------
//  ctor / dtor
//-------------------------------------------------

ui_menu_command::ui_menu_command(running_machine &machine, render_container *container, const game_driver *driver) : ui_menu(machine, container)
{
	m_driver = (driver == nullptr) ? &machine.system() : driver;
}

ui_menu_command::~ui_menu_command()
{
}

//-------------------------------------------------
//  populate
//-------------------------------------------------

void ui_menu_command::populate()
{
	std::vector<std::string> text;
	machine().datfile().command_sub_menu(m_driver, text);

	if (!text.empty())
	{
		for (size_t menu_items = 0; menu_items < text.size(); menu_items++)
			item_append(text[menu_items].c_str(), nullptr, 0, (void *)(FPTR)menu_items);
	}
	else
		item_append("No available Command for this machine.", nullptr, MENU_FLAG_DISABLE, nullptr);

	item_append(MENU_SEPARATOR_ITEM, nullptr, 0, nullptr);
	customtop = machine().ui().get_line_height() + 3.0f * UI_BOX_TB_BORDER;

}

//-------------------------------------------------
//  handle
//-------------------------------------------------

void ui_menu_command::handle()
{
	// process the menu
	const ui_menu_event *m_event = process(0);

	if (m_event != nullptr && m_event->iptkey == IPT_UI_SELECT)
	{
		std::string m_title(item[selected].text);
		ui_menu::stack_push(auto_alloc_clear(machine(), <ui_menu_command_content>(machine(), container, m_title, m_driver)));
	}
}

//-------------------------------------------------
//  perform our special rendering
//-------------------------------------------------

void ui_menu_command::custom_render(void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	float width;
	ui_manager &mui = machine().ui();
	std::string tempbuf = std::string("Command Info - Game: ").append(m_driver->description);

	// get the size of the text
	mui.draw_text_full(container, tempbuf.c_str(), 0.0f, 0.0f, 1.0f, JUSTIFY_CENTER, WRAP_NEVER,
	                              DRAW_NONE, ARGB_WHITE, ARGB_BLACK, &width, nullptr);
	width += (2.0f * UI_BOX_LR_BORDER) + 0.01f;
	float maxwidth = MAX(width, origx2 - origx1);

	// compute our bounds
	float x1 = 0.5f - 0.5f * maxwidth;
	float x2 = x1 + maxwidth;
	float y1 = origy1 - top;
	float y2 = origy1 - UI_BOX_TB_BORDER;

	// draw a box
	mui.draw_outlined_box(container, x1, y1, x2, y2, UI_GREEN_COLOR);

	// take off the borders
	x1 += UI_BOX_LR_BORDER;
	x2 -= UI_BOX_LR_BORDER;
	y1 += UI_BOX_TB_BORDER;

	// draw the text within it
	mui.draw_text_full(container, tempbuf.c_str(), x1, y1, x2 - x1, JUSTIFY_CENTER, WRAP_NEVER,
	                              DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, nullptr, nullptr);

}

//-------------------------------------------------
//  ctor / dtor
//-------------------------------------------------

ui_menu_command_content::ui_menu_command_content(running_machine &machine, render_container *container, std::string p_title, const game_driver *driver) : ui_menu(machine, container)
{
	m_driver = (driver == nullptr) ? &machine.system() : driver;
	m_title = p_title;
}

ui_menu_command_content::~ui_menu_command_content()
{
}

//-------------------------------------------------
//  handle
//-------------------------------------------------

void ui_menu_command_content::handle()
{
	// process the menu
	process(0);
}

//-------------------------------------------------
//  populate
//-------------------------------------------------

void ui_menu_command_content::populate()
{
	machine().pause();
	std::string buffer;
	machine().datfile().load_command_info(buffer, m_title);
	if (!buffer.empty())
	{
		float line_height = machine().ui().get_line_height();
		float lr_arrow_width = 0.4f * line_height * machine().render().ui_aspect();
		float gutter_width = lr_arrow_width * 1.3f;
		std::vector<int> xstart;
		std::vector<int> xend;
		int total_lines;
		convert_command_glyph(buffer);
		machine().ui().wrap_text(container, buffer.c_str(), 0.0f, 0.0f, 1.0f - (2.0f * UI_BOX_LR_BORDER) - 0.02f - (2.0f * gutter_width),
		                         total_lines, xstart, xend);
		for (int r = 0; r < total_lines; r++)
		{
			std::string tempbuf(buffer.substr(xstart[r], xend[r] - xstart[r]));
			int first_dspace = tempbuf.find("  ");
			if (first_dspace > 0 )
			{
				std::string first_part(tempbuf.substr(0, first_dspace));
				std::string last_part(tempbuf.substr(first_dspace));
				strtrimspace(last_part);
				item_append(first_part.c_str(), last_part.c_str(), MENU_FLAG_UI_HISTORY, nullptr);
			}
			else
				item_append(tempbuf.c_str(), nullptr, MENU_FLAG_UI_HISTORY, nullptr);
		}
		item_append(MENU_SEPARATOR_ITEM, nullptr, 0, nullptr);
	}

	machine().resume();
	customtop = custombottom = machine().ui().get_line_height() + 3.0f * UI_BOX_TB_BORDER;
}

//-------------------------------------------------
//  perform our special rendering
//-------------------------------------------------

void ui_menu_command_content::custom_render(void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	float width;
	ui_manager &mui = machine().ui();

	// get the size of the text
	mui.draw_text_full(container, m_title.c_str(), 0.0f, 0.0f, 1.0f, JUSTIFY_CENTER, WRAP_NEVER,
	                              DRAW_NONE, ARGB_WHITE, ARGB_BLACK, &width, nullptr);
	width += (2.0f * UI_BOX_LR_BORDER) + 0.01f;
	float maxwidth = MAX(width, origx2 - origx1);

	// compute our bounds
	float x1 = 0.5f - 0.5f * maxwidth;
	float x2 = x1 + maxwidth;
	float y1 = origy1 - top;
	float y2 = origy1 - UI_BOX_TB_BORDER;

	// draw a box
	mui.draw_outlined_box(container, x1, y1, x2, y2, UI_GREEN_COLOR);

	// take off the borders
	x1 += UI_BOX_LR_BORDER;
	x2 -= UI_BOX_LR_BORDER;
	y1 += UI_BOX_TB_BORDER;

	// draw the text within it
	mui.draw_text_full(container, m_title.c_str(), x1, y1, x2 - x1, JUSTIFY_CENTER, WRAP_NEVER,
	                              DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, nullptr, nullptr);

	std::string tempbuf = std::string("Command Info - Game: ").append(m_driver->description);

	mui.draw_text_full(container, tempbuf.c_str(), 0.0f, 0.0f, 1.0f, JUSTIFY_CENTER, WRAP_NEVER,
	                              DRAW_NONE, ARGB_WHITE, ARGB_BLACK, &width, nullptr);
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
	mui.draw_text_full(container, tempbuf.c_str(), x1, y1, x2 - x1, JUSTIFY_CENTER, WRAP_NEVER,
	                              DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, nullptr, nullptr);
}

/**************************************************
    MENU SOFTWARE HISTORY
**************************************************/
//-------------------------------------------------
//  ctor / dtor
//-------------------------------------------------

ui_menu_history_sw::ui_menu_history_sw(running_machine &machine, render_container *container, ui_software_info *swinfo, const game_driver *driver) : ui_menu(machine, container)
{
	m_list = swinfo->listname;
	m_short = swinfo->shortname;
	m_long = swinfo->longname;
	m_parent = swinfo->parentname;
	m_driver = (driver == nullptr) ? &machine.system() : driver;
}

ui_menu_history_sw::ui_menu_history_sw(running_machine &machine, render_container *container, const game_driver *driver) : ui_menu(machine, container)
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
	m_driver = (driver == nullptr) ? &machine.system() : driver;
}

ui_menu_history_sw::~ui_menu_history_sw()
{
}

//-------------------------------------------------
//  handle
//-------------------------------------------------

void ui_menu_history_sw::handle()
{
	// process the menu
	process(0);
}

//-------------------------------------------------
//  populate
//-------------------------------------------------

void ui_menu_history_sw::populate()
{
	machine().pause();
	std::string buffer;
	machine().datfile().load_software_info(m_list, buffer, m_short, m_parent);
	if (!buffer.empty())
	{
		float line_height = machine().ui().get_line_height();
		float lr_arrow_width = 0.4f * line_height * machine().render().ui_aspect();
		float gutter_width = lr_arrow_width * 1.3f;
		std::vector<int> xstart;
		std::vector<int> xend;
		int total_lines;

		machine().ui().wrap_text(container, buffer.c_str(), 0.0f, 0.0f, 1.0f - (2.0f * UI_BOX_LR_BORDER) - 0.02f - (2.0f * gutter_width),
		                         total_lines, xstart, xend);

		for (int r = 0; r < total_lines; r++)
		{
			std::string tempbuf(buffer.substr(xstart[r], xend[r] - xstart[r]));
			item_append(tempbuf.c_str(), nullptr, MENU_FLAG_UI_HISTORY, nullptr);
		}
	}
	else
		item_append("No available History for this software.", nullptr, MENU_FLAG_DISABLE, nullptr);

	machine().resume();

	item_append(MENU_SEPARATOR_ITEM, nullptr, 0, nullptr);
	customtop = custombottom = machine().ui().get_line_height() + 3.0f * UI_BOX_TB_BORDER;
}

//-------------------------------------------------
//  perform our special rendering
//-------------------------------------------------

void ui_menu_history_sw::custom_render(void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	float width;
	std::string tempbuf = std::string("Software info - ").append(m_long);
	ui_manager &mui = machine().ui();

	// get the size of the text
	mui.draw_text_full(container, tempbuf.c_str(), 0.0f, 0.0f, 1.0f, JUSTIFY_CENTER, WRAP_NEVER,
	                              DRAW_NONE, ARGB_WHITE, ARGB_BLACK, &width, nullptr);
	width += (2.0f * UI_BOX_LR_BORDER) + 0.01f;
	float maxwidth = MAX(width, origx2 - origx1);

	// compute our bounds
	float x1 = 0.5f - 0.5f * maxwidth;
	float x2 = x1 + maxwidth;
	float y1 = origy1 - top;
	float y2 = origy1 - UI_BOX_TB_BORDER;

	// draw a box
	mui.draw_outlined_box(container, x1, y1, x2, y2, UI_GREEN_COLOR);

	// take off the borders
	x1 += UI_BOX_LR_BORDER;
	x2 -= UI_BOX_LR_BORDER;
	y1 += UI_BOX_TB_BORDER;

	// draw the text within it
	mui.draw_text_full(container, tempbuf.c_str(), x1, y1, x2 - x1, JUSTIFY_CENTER, WRAP_NEVER,
	                              DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, nullptr, nullptr);

	tempbuf.assign("System driver: ").append(m_driver->description);

	mui.draw_text_full(container, tempbuf.c_str(), 0.0f, 0.0f, 1.0f, JUSTIFY_CENTER, WRAP_TRUNCATE,
	                              DRAW_NONE, ARGB_WHITE, ARGB_BLACK, &width, nullptr);
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
	mui.draw_text_full(container, tempbuf.c_str(), x1, y1, x2 - x1, JUSTIFY_CENTER, WRAP_TRUNCATE,
	                              DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, nullptr, nullptr);
}

/**************************************************
    MENU DATS
**************************************************/
//-------------------------------------------------
//  ctor / dtor
//-------------------------------------------------

ui_menu_dats::ui_menu_dats(running_machine &machine, render_container *container, int _flags, const game_driver *driver) : ui_menu(machine, container)
{
	m_driver = (driver == nullptr) ? &machine.system() : driver;
	m_flags = _flags;
}

ui_menu_dats::~ui_menu_dats()
{
}

//-------------------------------------------------
//  handle
//-------------------------------------------------

void ui_menu_dats::handle()
{
	// process the menu
	process(0);
}

//-------------------------------------------------
//  populate
//-------------------------------------------------

void ui_menu_dats::populate()
{
	machine().pause();
	switch (m_flags)
	{
		case UI_HISTORY_LOAD:
			if (!get_data(m_driver, m_flags))
				item_append("No available History for this machine.", nullptr, MENU_FLAG_DISABLE, nullptr);
			break;

		case UI_MAMEINFO_LOAD:
			if (!get_data(m_driver, m_flags))
				item_append("No available MameInfo for this machine.", nullptr, MENU_FLAG_DISABLE, nullptr);
			break;

		case UI_MESSINFO_LOAD:
			if (!get_data(m_driver, m_flags))
				item_append("No available MessInfo for this machine.", nullptr, MENU_FLAG_DISABLE, nullptr);
			break;

		case UI_STORY_LOAD:
			if (!get_data(m_driver, UI_STORY_LOAD))
				item_append("No available Mamescore for this machine.", nullptr, MENU_FLAG_DISABLE, nullptr);
			break;

		case UI_SYSINFO_LOAD:
			if (!get_data(m_driver, UI_SYSINFO_LOAD))
				item_append("No available Sysinfo for this machine.", nullptr, MENU_FLAG_DISABLE, nullptr);
			break;
	}

	machine().resume();
	item_append(MENU_SEPARATOR_ITEM, nullptr, 0, nullptr);
	customtop = custombottom = machine().ui().get_line_height() + 3.0f * UI_BOX_TB_BORDER;
}

//-------------------------------------------------
//  perform our special rendering
//-------------------------------------------------

void ui_menu_dats::custom_render(void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	float width;
	std::string tempbuf, revision;
	datfile_manager &datfile = machine().datfile();
	ui_manager &mui = machine().ui();

	switch (m_flags)
	{
		case UI_HISTORY_LOAD:
			tempbuf.assign("History - Game / System: ").append(m_driver->description);
			revision.assign("History.dat Revision: ").append(datfile.rev_history());
			break;

		case UI_MESSINFO_LOAD:
			tempbuf.assign("MessInfo - System: ").append(m_driver->description);
			revision.assign("Messinfo.dat Revision: ").append(datfile.rev_messinfo());
			break;

		case UI_MAMEINFO_LOAD:
			tempbuf.assign("MameInfo - Game: ").append(m_driver->description);
			revision.assign("Mameinfo.dat Revision: ").append(datfile.rev_mameinfo());
			break;

		case UI_SYSINFO_LOAD:
			tempbuf.assign("Sysinfo - System: ").append(m_driver->description);
			revision.assign("Sysinfo.dat Revision: ").append(datfile.rev_sysinfo());
			break;

		case UI_STORY_LOAD:
			tempbuf.assign("MAMESCORE - Game: ").append(m_driver->description);
			revision.assign("Story.dat Revision: ").append(machine().datfile().rev_storyinfo());
			break;
	}

	// get the size of the text
	mui.draw_text_full(container, tempbuf.c_str(), 0.0f, 0.0f, 1.0f, JUSTIFY_CENTER, WRAP_NEVER,
	                              DRAW_NONE, ARGB_WHITE, ARGB_BLACK, &width, nullptr);
	width += (2.0f * UI_BOX_LR_BORDER) + 0.01f;
	float maxwidth = MAX(width, origx2 - origx1);

	// compute our bounds
	float x1 = 0.5f - 0.5f * maxwidth;
	float x2 = x1 + maxwidth;
	float y1 = origy1 - top;
	float y2 = origy1 - UI_BOX_TB_BORDER;

	// draw a box
	mui.draw_outlined_box(container, x1, y1, x2, y2, UI_GREEN_COLOR);

	// take off the borders
	x1 += UI_BOX_LR_BORDER;
	x2 -= UI_BOX_LR_BORDER;
	y1 += UI_BOX_TB_BORDER;

	// draw the text within it
	mui.draw_text_full(container, tempbuf.c_str(), x1, y1, x2 - x1, JUSTIFY_CENTER, WRAP_NEVER,
	                              DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, nullptr, nullptr);

	mui.draw_text_full(container, revision.c_str(), 0.0f, 0.0f, 1.0f, JUSTIFY_CENTER, WRAP_TRUNCATE,
	                              DRAW_NONE, ARGB_WHITE, ARGB_BLACK, &width, nullptr);
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

bool ui_menu_dats::get_data(const game_driver *driver, int flags)
{
	std::string buffer;
	machine().datfile().load_data_info(driver, buffer, flags);

	if (buffer.empty())
		return false;

	float line_height = machine().ui().get_line_height();
	float lr_arrow_width = 0.4f * line_height * machine().render().ui_aspect();
	float gutter_width = lr_arrow_width * 1.3f;
	std::vector<int> xstart;
	std::vector<int> xend;
	int tlines;

	machine().ui().wrap_text(container, buffer.c_str(), 0.0f, 0.0f, 1.0f - (2.0f * UI_BOX_LR_BORDER) - 0.02f - (2.0f * gutter_width), tlines, xstart, xend);
	for (int r = 0; r < tlines; r++)
	{
		std::string tempbuf(buffer.substr(xstart[r], xend[r] - xstart[r]));
		// special case for mamescore
		if (flags == UI_STORY_LOAD)
		{
			size_t last_underscore = tempbuf.find_last_of('_');
			if (last_underscore != std::string::npos)
			{
				std::string last_part(tempbuf.substr(last_underscore + 1));
				int primary = tempbuf.find("___");
				std::string first_part(tempbuf.substr(0, primary));
				item_append(first_part.c_str(), last_part.c_str(), MENU_FLAG_UI_HISTORY, nullptr);
			}
		}
		else
			item_append(tempbuf.c_str(), nullptr, MENU_FLAG_UI_HISTORY, nullptr);
	}
		return true;
}
