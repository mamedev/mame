/***************************************************************************

    ui/imginfo.c

    Image info screen

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#include "emu.h"
#include "ui/menu.h"
#include "ui/imginfo.h"


/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

//-------------------------------------------------
//  ctor
//-------------------------------------------------

ui_menu_image_info::ui_menu_image_info(running_machine &machine, render_container *container) : ui_menu(machine, container)
{
}


//-------------------------------------------------
//  dtor
//-------------------------------------------------

ui_menu_image_info::~ui_menu_image_info()
{
}


//-------------------------------------------------
//  populate
//-------------------------------------------------

void ui_menu_image_info::populate()
{
	astring tempstring;
	item_append(image_info_astring(machine(), tempstring), NULL, MENU_FLAG_MULTILINE, NULL);
}


//-------------------------------------------------
//  handle
//-------------------------------------------------

void ui_menu_image_info::handle()
{
	// process the menu
	process(0);
}
