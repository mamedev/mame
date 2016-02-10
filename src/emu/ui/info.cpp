// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Nathan Woods
/***************************************************************************

    ui/info.c

    System and image info screens

***************************************************************************/

#include "emu.h"
#include "ui/menu.h"
#include "ui/info.h"
#include "ui/ui.h"
#include "softlist.h"

/*-------------------------------------------------
  menu_game_info - handle the game information
  menu
 -------------------------------------------------*/

ui_menu_game_info::ui_menu_game_info(running_machine &machine, render_container *container) : ui_menu(machine, container)
{
}

ui_menu_game_info::~ui_menu_game_info()
{
}

void ui_menu_game_info::populate()
{
	std::string tempstring;
	item_append(machine().ui().game_info_astring(tempstring).c_str(), nullptr, MENU_FLAG_MULTILINE, nullptr);
}

void ui_menu_game_info::handle()
{
	// process the menu
	process(0);
}


/*-------------------------------------------------
  ui_menu_image_info - handle the image information
  menu
 -------------------------------------------------*/

ui_menu_image_info::ui_menu_image_info(running_machine &machine, render_container *container) : ui_menu(machine, container)
{
}

ui_menu_image_info::~ui_menu_image_info()
{
}

void ui_menu_image_info::populate()
{
	item_append(machine().system().description, nullptr, MENU_FLAG_DISABLE, nullptr);
	item_append("", nullptr, MENU_FLAG_DISABLE, nullptr);

	image_interface_iterator iter(machine().root_device());
	for (device_image_interface *image = iter.first(); image != nullptr; image = iter.next())
			image_info(image);
}

void ui_menu_image_info::handle()
{
	// process the menu
	process(0);
}


/*-------------------------------------------------
  image_info - display image info for a specific
  image interface device
-------------------------------------------------*/

void ui_menu_image_info::image_info(device_image_interface *image)
{
	if (image->exists())
	{
		// display device type and filename
		item_append(image->brief_instance_name(), image->basename(), 0, nullptr);

		// if image has been loaded through softlist, let's add some more info
		if (image->software_entry())
		{
			std::string str;

			// display long filename
			item_append(image->longname(), "", MENU_FLAG_DISABLE, nullptr);

			// display manufacturer and year
			strcatprintf(str, "%s, %s", image->manufacturer(), image->year());
			item_append(str.c_str(), "", MENU_FLAG_DISABLE, nullptr);

			// display supported information, if available
			switch (image->supported())
			{
				case SOFTWARE_SUPPORTED_NO:
					item_append("Not supported", "", MENU_FLAG_DISABLE, nullptr);
					break;
				case SOFTWARE_SUPPORTED_PARTIAL:
					item_append("Partially supported", "", MENU_FLAG_DISABLE, nullptr);
					break;
				default:
					break;
			}
		}
	}
	else
		item_append(image->brief_instance_name(), "[empty]", 0, nullptr);
	item_append("", nullptr, MENU_FLAG_DISABLE, nullptr);
}
