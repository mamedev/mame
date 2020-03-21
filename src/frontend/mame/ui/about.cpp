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

namespace {

#include "copying.ipp"

} // anonymous namespace

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

	for (char const *const *line = copying_text; *line; ++line)
		item_append(*line, "", 0, nullptr);

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

};
