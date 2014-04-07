/*********************************************************************

    ui/filemngr.c

    MESS's clunky built-in file manager

    TODO
        - Restrict directory listing by file extension
        - Support file manager invocation from the main menu for
          required images

*********************************************************************/

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

#include "emu.h"
#include "ui/ui.h"
#include "ui/swlist.h"
#include "ui/filemngr.h"
#include "ui/filesel.h"


/***************************************************************************
    FILE MANAGER
***************************************************************************/

//-------------------------------------------------
//  ctor
//-------------------------------------------------

ui_menu_file_manager::ui_menu_file_manager(running_machine &machine, render_container *container) : ui_menu(machine, container)
{
}


//-------------------------------------------------
//  dtor
//-------------------------------------------------

ui_menu_file_manager::~ui_menu_file_manager()
{
}


//-------------------------------------------------
//  custom_render - perform our special rendering
//-------------------------------------------------

void ui_menu_file_manager::custom_render(void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	const char *path;

	// access the path
	path = selected_device ? selected_device->filename() : NULL;
	extra_text_render(container, top, bottom,
						origx1, origy1, origx2, origy2, NULL, path);
}


//-------------------------------------------------
//  populate
//-------------------------------------------------

void ui_menu_file_manager::populate()
{
	astring buffer;
	astring tmp_name;

	// cycle through all devices for this system
	image_interface_iterator iter(machine().root_device());
	for (device_image_interface *image = iter.first(); image != NULL; image = iter.next())
	{
		// get the image type/id
		buffer.printf(
			"%s (%s)",
			image->device().name(), image->brief_instance_name());

		// get the base name
		if (image->basename() != NULL)
		{
			tmp_name.cpy(image->basename());

			// if the image has been loaded through softlist, also show the loaded part
			if (image->part_entry() != NULL)
			{
				const software_part *tmp = image->part_entry();
				if (tmp->name() != NULL)
				{
					tmp_name.cat(" (");
					tmp_name.cat(tmp->name());
					// also check if this part has a specific part_id (e.g. "Map Disc", "Bonus Disc", etc.), and in case display it
					if (image->get_feature("part_id") != NULL)
					{
						tmp_name.cat(": ");
						tmp_name.cat(image->get_feature("part_id"));
					}
					tmp_name.cat(")");
				}
			}
		}
		else
			tmp_name.cpy("---");

		// record the menu item
		item_append(buffer, tmp_name.cstr(), 0, (void *) image);
	}

	custombottom = machine().ui().get_line_height() + 3.0f * UI_BOX_TB_BORDER;
}


//-------------------------------------------------
//  handle
//-------------------------------------------------

void ui_menu_file_manager::handle()
{
	// update the selected device
	selected_device = (device_image_interface *) get_selection();

	// process the menu
	const ui_menu_event *event = process(0);
	if (event != NULL && event->iptkey == IPT_UI_SELECT)
	{
		selected_device = (device_image_interface *) event->itemref;
		if (selected_device != NULL)
		{
			ui_menu::stack_push(selected_device->get_selection_menu(machine(), container));

			// reset the existing menu
			reset(UI_MENU_RESET_REMEMBER_POSITION);
		}
	}
}
