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
#include "ui/prscntrl.h"
#include "ui/miscmenu.h"
#include "ui/ui.h"

#include "softlist.h"

#include <string_view>
#include <unordered_set>
#include <utility>


namespace ui {

/***************************************************************************
    FILE MANAGER
***************************************************************************/

//-------------------------------------------------
//  ctor
//-------------------------------------------------

menu_file_manager::menu_file_manager(mame_ui_manager &mui, render_container &container, const char *warnings)
	: menu(mui, container)
	, m_warnings(warnings ? warnings : "")
	, m_selected_device(nullptr)
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
//  recompute_metrics - recompute metrics
//-------------------------------------------------

void menu_file_manager::recompute_metrics(uint32_t width, uint32_t height, float aspect)
{
	menu::recompute_metrics(width, height, aspect);

	set_custom_space(0.0F, line_height() + 3.0F * tb_border());
}


//-------------------------------------------------
//  custom_render - perform our special rendering
//-------------------------------------------------

void menu_file_manager::custom_render(uint32_t flags, void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	// access the path
	std::string_view path = m_selected_device && m_selected_device->exists() ? m_selected_device->filename() : std::string_view();
	extra_text_render(top, bottom, origx1, origy1, origx2, origy2, std::string_view(), path);
}


void menu_file_manager::fill_image_line(device_image_interface &img, std::string &instance, std::string &filename)
{
	// get the image type/id
	instance = string_format("%s (%s)", img.instance_name(), img.brief_instance_name());

	// get the base name
	if (img.basename())
	{
		filename.assign(img.basename());

		// if the image has been loaded through softlist, also show the loaded part
		if (img.loaded_through_softlist())
		{
			const software_part *tmp = img.part_entry();
			if (!tmp->name().empty())
			{
				filename.append(" (");
				filename.append(tmp->name());
				// also check if this part has a specific part_id (e.g. "Map Disc", "Bonus Disc", etc.), and in case display it
				if (img.get_feature("part_id") != nullptr)
				{
					filename.append(": ");
					filename.append(img.get_feature("part_id"));
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

void menu_file_manager::populate()
{
	m_notifiers.clear();

	if (!m_warnings.empty())
		item_append(m_warnings, FLAG_DISABLE, nullptr);

	// cycle through all devices for this system
	bool missing_mandatory = false;
	std::unordered_set<std::string> devtags;
	std::string tmp_inst, tmp_name;
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
				if (scan.has_preset_images_selection())
				{
					if (devtags.insert(scan.device().tag()).second)
					{
						item_append(string_format(_("[root%1$s]"), scan.device().owner()->tag()), FLAG_UI_HEADING | FLAG_DISABLE, nullptr);
						int const index = item_append(scan.image_type_name(), scan.preset_images_list()[scan.current_preset_image_id()], 0, (void *)&scan);
						m_notifiers.emplace_back(scan.add_media_change_notifier(
								[this, index, &scan] (device_image_interface::media_change_event ev)
								{
									item(index).set_subtext(scan.preset_images_list()[scan.current_preset_image_id()]);
								}));
					}
				}
				else if (scan.user_loadable())
				{
					// if it is a child device, and not something further down the device tree, we want it in the menu!
					if (!strcmp(scan.device().owner()->tag(), dev.tag()))
					{
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
							fill_image_line(scan, tmp_inst, tmp_name);
							int const index = item_append(std::move(tmp_inst), std::move(tmp_name), 0, (void *)&scan);
							m_notifiers.emplace_back(scan.add_media_change_notifier(
									[this, index, &scan] (device_image_interface::media_change_event ev)
									{
										std::string text, subtext;
										fill_image_line(scan, text, subtext);
										item(index).set_subtext(std::move(subtext));
									}));
						}
					}
				}
			}
		}
	}
	item_append(menu_item_type::SEPARATOR);

	if (m_warnings.empty() || !missing_mandatory)
		item_append(m_warnings.empty() ? _("Reset System") : _("Start System"), 0, (void *)1);
}


//-------------------------------------------------
//  handle
//-------------------------------------------------

bool menu_file_manager::handle(event const *ev)
{
	bool result = false;

	if (ev)
	{
		if ((uintptr_t)ev->itemref == 1)
		{
			if (m_selected_device)
			{
				m_selected_device = nullptr;
				result = true;
			}

			if (IPT_UI_SELECT == ev->iptkey)
				machine().schedule_hard_reset();
		}
		else
		{
			if (ev->itemref != m_selected_device)
			{
				m_selected_device = (device_image_interface *)ev->itemref;
				result = true;
			}

			if (m_selected_device && (IPT_UI_SELECT == ev->iptkey))
			{
				if (m_selected_device->has_preset_images_selection())
				{
					menu::stack_push<menu_control_device_preset>(ui(), container(), *m_selected_device);
				}
				else
				{
					floppy_image_device *floppy_device = dynamic_cast<floppy_image_device *>(m_selected_device);
					if (floppy_device)
						menu::stack_push<menu_control_floppy_image>(ui(), container(), *floppy_device);
					else
						menu::stack_push<menu_control_device_image>(ui(), container(), *m_selected_device);
				}
			}
		}
	}

	return result;
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
