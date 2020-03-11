// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    ui/about.cpp

    "About" modal

***************************************************************************/

#include "emu.h"

#include "ui/about.h"
#include "ui/ui.h"
#include "ui/utils.h"
#include "osdcore.h"

namespace ui {

/**************************************************

 ABOUT MODAL

 **************************************************/


//-------------------------------------------------
//  ctor
//-------------------------------------------------

menu_about::menu_about(mame_ui_manager &mui, render_container &container)
	: menu(mui, container)
	, m_pause_checked(false)
	, m_was_paused(false)
{
	std::vector<char> text;
	osd_file::ptr file;
	uint64_t filesize = 0;

	osd_file::error error = osd_file::open("COPYING", OPEN_FLAG_READ, file, filesize);
	if (error != osd_file::error::NONE)
	{
		stack_pop();
		osd_printf_debug("Unable to open COPYING file\n");
		return;
	}

	uint32_t actual = 0;
	text.reserve(filesize);
	file->read(&text[0], 0, (uint32_t)filesize, actual);

	if (actual != (uint32_t)filesize)
	{
		stack_pop();
		osd_printf_debug("Unable to fully load COPYING file\n");
		return;
	}

	m_lines.clear();

	uint32_t i = 0;
	uint32_t line_index = 0;
	std::string curr_line;
	while (i < (uint32_t)filesize)
	{
		if (text[i] == 0x0a)
		{
			curr_line += '\n';
			m_lines.push_back(curr_line);
			curr_line = "";
			line_index++;
			i++;
			continue;
		}
		curr_line += text[i];
		i++;
	}
}


//-------------------------------------------------
//  dtor
//-------------------------------------------------

menu_about::~menu_about()
{
	// resume if appropriate (is the destructor really the right place
	// to do this sort of activity?)
	// TODO(mooglyguy): No, it isn't. We should have an explicit menu-exit callback.
	if (!m_was_paused)
		machine().resume();
}

//-------------------------------------------------
//  populate - populates the about modal
//-------------------------------------------------

void menu_about::populate(float &customtop, float &custombottom)
{
	// pause if appropriate
	if (!m_pause_checked)
	{
		m_was_paused = machine().paused();
		if (!m_was_paused)
			machine().pause();
		m_pause_checked = true;
	}

	for (std::string line : m_lines)
	{
		item_append(line.c_str(), "", 0, nullptr);
	}
	item_append(menu_item_type::SEPARATOR);
}


//-------------------------------------------------
//  handle - manages inputs in the about modal
//-------------------------------------------------

void menu_about::handle()
{
	// process the menu
	const event *event = process(0);

	// process the event
	if (event && (event->iptkey == IPT_UI_SELECT))
	{
		stack_pop();
	}
}
