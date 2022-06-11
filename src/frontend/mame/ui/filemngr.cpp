// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/*********************************************************************

    ui/filemngr.cpp

    MESS's clunky built-in file manager

    TODO
        - Restrict directory listing by file extension

*********************************************************************/

#include "emu.h"
#include "ui/filemngr.h"

#include "ui/filesel.h"
#include "ui/floppycntrl.h"
#include "ui/imgcntrl.h"
#include "ui/miscmenu.h"
#include "ui/ui.h"

#include "softlist.h"


namespace ui {

/***************************************************************************
    FILE MANAGER
***************************************************************************/

//-------------------------------------------------
//  ctor
//-------------------------------------------------

menu_file_manager::menu_file_manager(mame_ui_manager &mui, render_container &container, const char *warnings)
	: menu(mui, container)
	, selected_device(nullptr)
	, m_warnings(warnings ? warnings : "")
{
	// The warning string is used when accessing from the force_file_manager call, i.e.
	// when the file manager is loaded top front in the case of mandatory image devices
	set_heading(_("File Manager"));
}


//-------------------------------------------------
//  dtor
//-------------------------------------------------

menu_file_manager::~menu_file_manager()
{
}


//-------------------------------------------------
//  custom_render - perform our special rendering
//-------------------------------------------------

void menu_file_manager::custom_render(void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	// access the path
	std::string_view path = selected_device && selected_device->exists() ? selected_device->filename() : std::string_view();
	extra_text_render(top, bottom, origx1, origy1, origx2, origy2, std::string_view(), path);
}


void menu_file_manager::fill_image_line(device_image_interface *img, std::string &instance, std::string &filename)
{
	// get the image type/id
	instance = string_format("%s (%s)", img->instance_name(), img->brief_instance_name());

	// get the base name
	if (img->basename() != nullptr)
	{
		filename.assign(img->basename());

		// if the image has been loaded through softlist, also show the loaded part
		if (img->loaded_through_softlist())
		{
			const software_part *tmp = img->part_entry();
			if (!tmp->name().empty())
			{
				filename.append(" (");
				filename.append(tmp->name());
				// also check if this part has a specific part_id (e.g. "Map Disc", "Bonus Disc", etc.), and in case display it
				if (img->get_feature("part_id") != nullptr)
				{
					filename.append(": ");
					filename.append(img->get_feature("part_id"));
				}
				filename.append(")");
			}
		}
	}
	else
		filename.assign("---");
}

//-------------------------------------------------
//  populate
//-------------------------------------------------

void menu_file_manager::populate(float &customtop, float &custombottom)
{
	std::string tmp_inst, tmp_name;

	if (!m_warnings.empty())
		item_append(m_warnings, FLAG_DISABLE, nullptr);

	// cycle through all devices for this system
	bool missing_mandatory = false;
	std::unordered_set<std::string> devtags;
	for (device_t &dev : device_enumerator(machine().root_device()))
	{
		bool tag_appended = false;
		if (!devtags.insert(dev.tag()).second)
			continue;

		// check whether it owns an image interface
		image_interface_enumerator subiter(dev);
		if (subiter.first() != nullptr)
		{
			// if so, cycle through all its image interfaces
			for (device_image_interface &scan : subiter)
			{
				if (!scan.user_loadable())
					continue;

				// if it is a child device, and not something further down the device tree, we want it in the menu!
				if (strcmp(scan.device().owner()->tag(), dev.tag()) == 0)
					if (devtags.insert(scan.device().tag()).second)
					{
						if (!scan.basename() && scan.must_be_loaded())
							missing_mandatory = true;

						// check whether we already had some devices with the same owner: if not, output the owner tag!
						if (!tag_appended)
						{
							item_append(string_format(_("[root%1$s]"), dev.tag()), FLAG_UI_HEADING | FLAG_DISABLE, nullptr);
							tag_appended = true;
						}

						// finally, append the image interface to the menu
						fill_image_line(&scan, tmp_inst, tmp_name);
						item_append(tmp_inst, tmp_name, 0, (void *)&scan);
					}
			}
		}
	}
	item_append(menu_item_type::SEPARATOR);

	if (m_warnings.empty() || !missing_mandatory)
		item_append(m_warnings.empty() ? _("Reset System") : _("Start System"), 0, (void *)1);

	custombottom = ui().get_line_height() + 3.0f * ui().box_tb_border();
}


//-------------------------------------------------
//  handle
//-------------------------------------------------

void menu_file_manager::handle(event const *ev)
{
	// process the menu
	if (ev && ev->itemref && (ev->iptkey == IPT_UI_SELECT))
	{
		if ((uintptr_t)ev->itemref == 1)
		{
			machine().schedule_hard_reset();
		}
		else
		{
			selected_device = (device_image_interface *) ev->itemref;
			if (selected_device)
			{
				floppy_image_device *floppy_device = dynamic_cast<floppy_image_device *>(selected_device);
				if (floppy_device)
					menu::stack_push<menu_control_floppy_image>(ui(), container(), *floppy_device);
				else
					menu::stack_push<menu_control_device_image>(ui(), container(), *selected_device);

				// reset the existing menu
				reset(reset_options::REMEMBER_POSITION);
			}
		}
	}
}

// force file manager menu
void menu_file_manager::force_file_manager(mame_ui_manager &mui, render_container &container, const char *warnings)
{
	// drop any existing menus and start the file manager
	menu::stack_reset(mui);
	menu::stack_push_special_main<menu_file_manager>(mui, container, warnings);
	mui.show_menu();

	// make sure MAME is paused
	mui.machine().pause();
}

} // namespace ui
