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
	item_append(machine().system().description, NULL, MENU_FLAG_DISABLE, NULL);
	item_append("", NULL, MENU_FLAG_DISABLE, NULL);

	image_interface_iterator iter(machine().root_device());
	for (device_image_interface *image = iter.first(); image != NULL; image = iter.next())
			image_info(image);
}


//-------------------------------------------------
//  handle
//-------------------------------------------------

void ui_menu_image_info::handle()
{
	// process the menu
	process(0);
}


//-------------------------------------------------
//  image_info - display image info for a specific
//  image interface device
//-------------------------------------------------

void ui_menu_image_info::image_info(device_image_interface *image)
{
	if (image->exists())
	{
		// display device type and filename
		item_append(image->brief_instance_name(), image->basename(), 0, NULL);

		// if image has been loaded through softlist, let's add some more info
		if (image->software_entry())
		{
			astring string;

			// display long filename
			item_append(image->longname(), "", MENU_FLAG_DISABLE, NULL);
			
			// display manufacturer and year
			string.catprintf("%s, %s", image->manufacturer(), image->year());
			item_append(string, "", MENU_FLAG_DISABLE, NULL);

			// display supported information, if available
			switch (image->supported()) 
			{
				case SOFTWARE_SUPPORTED_NO: 
					item_append("Not supported", "", MENU_FLAG_DISABLE, NULL);
					break;
				case SOFTWARE_SUPPORTED_PARTIAL: 
					item_append("Partially supported", "", MENU_FLAG_DISABLE, NULL);
					break;
				default: 
					break;
			}
		}
	}
	else
		item_append(image->brief_instance_name(), "[empty]", 0, NULL);
	item_append("", NULL, MENU_FLAG_DISABLE, NULL);
}
