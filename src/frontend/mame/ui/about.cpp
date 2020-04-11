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

#include "emu/copying.ipp"

} // anonymous namespace

/**************************************************

 ABOUT MODAL

**************************************************/


//-------------------------------------------------
//  ctor
//-------------------------------------------------

menu_about::menu_about(mame_ui_manager &mui, render_container &container)
	: menu(mui, container)
{
}


//-------------------------------------------------
//  dtor
//-------------------------------------------------

menu_about::~menu_about()
{
}


//-------------------------------------------------
//  populate - populates the about modal
//-------------------------------------------------

void menu_about::populate(float &customtop, float &custombottom)
{
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
