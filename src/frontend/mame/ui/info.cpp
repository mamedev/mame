// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Nathan Woods
/***************************************************************************

    ui/info.cpp

    System and image info screens

***************************************************************************/

#include "emu.h"

#include "ui/info.h"
#include "ui/ui.h"

#include "swinfo.h"

namespace ui {
/*-------------------------------------------------
  menu_game_info - handle the game information
  menu
 -------------------------------------------------*/

menu_game_info::menu_game_info(mame_ui_manager &mui, render_container &container) : menu(mui, container)
{
}

menu_game_info::~menu_game_info()
{
}

void menu_game_info::populate()
{
	std::string tempstring;
	item_append(ui().game_info_astring(tempstring), "", FLAG_MULTILINE, nullptr);
}

void menu_game_info::handle()
{
	// process the menu
	process(0);
}


/*-------------------------------------------------
  menu_image_info - handle the image information
  menu
 -------------------------------------------------*/

menu_image_info::menu_image_info(mame_ui_manager &mui, render_container &container) : menu(mui, container)
{
}

menu_image_info::~menu_image_info()
{
}

void menu_image_info::populate()
{
	item_append(machine().system().description, "", FLAG_DISABLE, nullptr);
	item_append("", "", FLAG_DISABLE, nullptr);

	for (device_image_interface &image : image_interface_iterator(machine().root_device()))
		image_info(&image);
}

void menu_image_info::handle()
{
	// process the menu
	process(0);
}


/*-------------------------------------------------
  image_info - display image info for a specific
  image interface device
-------------------------------------------------*/

void menu_image_info::image_info(device_image_interface *image)
{
	if (image->exists())
	{
		// display device type and filename
		item_append(image->brief_instance_name(), image->basename(), 0, nullptr);

		// if image has been loaded through softlist, let's add some more info
		if (image->software_entry())
		{
			// display long filename
			item_append(image->longname(), "", FLAG_DISABLE, nullptr);

			// display manufacturer and year
			item_append(string_format("%s, %s", image->manufacturer(), image->year()), "", FLAG_DISABLE, nullptr);

			// display supported information, if available
			switch (image->supported())
			{
				case SOFTWARE_SUPPORTED_NO:
					item_append(_("Not supported"), "", FLAG_DISABLE, nullptr);
					break;
				case SOFTWARE_SUPPORTED_PARTIAL:
					item_append(_("Partially supported"), "", FLAG_DISABLE, nullptr);
					break;
				default:
					break;
			}
		}
	}
	else
		item_append(image->brief_instance_name(), _("[empty]"), 0, nullptr);
	item_append("", "", FLAG_DISABLE, nullptr);
}

} // namespace ui
