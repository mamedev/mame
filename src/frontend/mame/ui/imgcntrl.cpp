// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    ui/imgcntrl.cpp

    MAME's clunky built-in file manager

***************************************************************************/

#include "emu.h"

#include "ui/imgcntrl.h"

#include "ui/ui.h"
#include "ui/filesel.h"
#include "ui/filecreate.h"
#include "ui/swlist.h"

#include "audit.h"
#include "drivenum.h"
#include "emuopts.h"
#include "softlist.h"
#include "zippath.h"


namespace ui {
/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

//-------------------------------------------------
//  ctor
//-------------------------------------------------

menu_control_device_image::menu_control_device_image(mame_ui_manager &mui, render_container &container, device_image_interface &image)
	: menu(mui, container)
	, m_image(image)
	, m_create_ok(false)
	, m_create_confirmed(false)
{
	m_submenu_result.i = -1;

	if (m_image.software_list_name())
		m_sld = software_list_device::find_by_name(mui.machine().config(), m_image.software_list_name());
	else
		m_sld = nullptr;
	m_swi = m_image.software_entry();
	m_swp = m_image.part_entry();

	if (m_swi != nullptr)
	{
		m_state = START_OTHER_PART;
		m_current_directory = m_image.working_directory();
	}
	else
	{
		m_state = START_FILE;

		// if the image exists, set the working directory to the parent directory
		if (m_image.exists())
		{
			m_current_file.assign(m_image.filename());
			util::zippath_parent(m_current_directory, m_current_file);
		}
		else
		{
			m_current_directory = m_image.working_directory();
		}

		// check to see if the path exists; if not clear it
		if (util::zippath_opendir(m_current_directory, nullptr) != osd_file::error::NONE)
			m_current_directory.clear();
	}
}


//-------------------------------------------------
//  dtor
//-------------------------------------------------

menu_control_device_image::~menu_control_device_image()
{
}


//-------------------------------------------------
//  test_create - creates a new disk image
//-------------------------------------------------

void menu_control_device_image::test_create(bool &can_create, bool &need_confirm)
{
	// assemble the full path
	auto path = util::zippath_combine(m_current_directory, m_current_file);

	// does a file or a directory exist at the path
	auto entry = osd_stat(path.c_str());
	auto file_type = (entry != nullptr) ? entry->type : osd::directory::entry::entry_type::NONE;

	switch(file_type)
	{
		case osd::directory::entry::entry_type::NONE:
			// no file/dir here - always create
			can_create = true;
			need_confirm = false;
			break;

		case osd::directory::entry::entry_type::FILE:
			// a file exists here - ask for permission from the user
			can_create = true;
			need_confirm = true;
			break;

		case osd::directory::entry::entry_type::DIR:
			// a directory exists here - we can't save over it
			ui().popup_time(5, "%s", _("Cannot save over directory"));
			can_create = false;
			need_confirm = false;
			break;

		default:
			can_create = false;
			need_confirm = false;
			fatalerror("Unexpected\n");
	}
}


//-------------------------------------------------
//  load_software_part
//-------------------------------------------------

void menu_control_device_image::load_software_part()
{
	std::string temp_name = string_format("%s:%s:%s", m_sld->list_name(), m_swi->shortname(), m_swp->name());

	driver_enumerator drivlist(machine().options(), machine().options().system_name());
	drivlist.next();
	media_auditor auditor(drivlist);
	media_auditor::summary summary = auditor.audit_software(m_sld->list_name(), (software_info *)m_swi, AUDIT_VALIDATE_FAST);
	// if everything looks good, load software
	if (summary == media_auditor::CORRECT || summary == media_auditor::BEST_AVAILABLE || summary == media_auditor::NONE_NEEDED)
	{
		m_image.load_software(temp_name);
		stack_pop();
	}
	else
	{
		machine().popmessage(_("The software selected is missing one or more required ROM or CHD images. Please select a different one."));
		m_state = SELECT_SOFTLIST;
	}
}


//-------------------------------------------------
//  hook_load
//-------------------------------------------------

void menu_control_device_image::hook_load(const std::string &name)
{
	if (m_image.is_reset_on_load()) m_image.set_init_phase();
	m_image.load(name);
	stack_pop();
}


//-------------------------------------------------
//  populate
//-------------------------------------------------

void menu_control_device_image::populate()
{
}


//-------------------------------------------------
//  handle
//-------------------------------------------------

void menu_control_device_image::handle()
{
	switch(m_state)
	{
	case START_FILE:
		m_submenu_result.filesel = menu_file_selector::result::INVALID;
		menu::stack_push<menu_file_selector>(ui(), container(), &m_image, m_current_directory, m_current_file, true, m_image.image_interface()!=nullptr, m_image.is_creatable(), m_submenu_result.filesel);
		m_state = SELECT_FILE;
		break;

	case START_SOFTLIST:
		m_sld = nullptr;
		menu::stack_push<menu_software>(ui(), container(), m_image.image_interface(), &m_sld);
		m_state = SELECT_SOFTLIST;
		break;

	case START_OTHER_PART:
		m_submenu_result.swparts = menu_software_parts::result::INVALID;
		menu::stack_push<menu_software_parts>(ui(), container(), m_swi, m_swp->interface().c_str(), &m_swp, true, m_submenu_result.swparts);
		m_state = SELECT_OTHER_PART;
		break;

	case SELECT_SOFTLIST:
		if (!m_sld)
		{
			stack_pop();
			break;
		}
		m_software_info_name.clear();
		menu::stack_push<menu_software_list>(ui(), container(), m_sld, m_image.image_interface(), m_software_info_name);
		m_state = SELECT_PARTLIST;
		break;

	case SELECT_PARTLIST:
		m_swi = m_sld->find(m_software_info_name.c_str());
		if (!m_swi)
			m_state = START_SOFTLIST;
		else if (m_swi->has_multiple_parts(m_image.image_interface()))
		{
			m_submenu_result.swparts = menu_software_parts::result::INVALID;
			m_swp = nullptr;
			menu::stack_push<menu_software_parts>(ui(), container(), m_swi, m_image.image_interface(), &m_swp, false, m_submenu_result.swparts);
			m_state = SELECT_ONE_PART;
		}
		else
		{
			m_swp = &m_swi->parts().front();
			load_software_part();
		}
		break;

	case SELECT_ONE_PART:
		switch(m_submenu_result.swparts) {
		case menu_software_parts::result::ENTRY: {
			load_software_part();
			break;
		}

		default: // return to list
			m_state = SELECT_SOFTLIST;
			break;

		}
		break;

	case SELECT_OTHER_PART:
		switch(m_submenu_result.swparts) {
		case menu_software_parts::result::ENTRY:
			load_software_part();
			break;

		case menu_software_parts::result::FMGR:
			m_state = START_FILE;
			handle();
			break;

		case menu_software_parts::result::EMPTY:
			m_image.unload();
			stack_pop();
			break;

		case menu_software_parts::result::SWLIST:
			m_state = START_SOFTLIST;
			handle();
			break;

		case menu_software_parts::result::INVALID: // return to system
			stack_pop();
			break;

		}
		break;

	case SELECT_FILE:
		switch(m_submenu_result.filesel)
		{
		case menu_file_selector::result::EMPTY:
			m_image.unload();
			stack_pop();
			break;

		case menu_file_selector::result::FILE:
			hook_load(m_current_file);
			break;

		case menu_file_selector::result::CREATE:
			menu::stack_push<menu_file_create>(ui(), container(), &m_image, m_current_directory, m_current_file, m_create_ok);
			m_state = CHECK_CREATE;
			break;

		case menu_file_selector::result::SOFTLIST:
			m_state = START_SOFTLIST;
			handle();
			break;

		default: // return to system
			stack_pop();
			break;
		}
		break;

	case CREATE_FILE: {
		bool can_create, need_confirm;
		test_create(can_create, need_confirm);
		if(can_create) {
			if(need_confirm) {
				menu::stack_push<menu_confirm_save_as>(ui(), container(), &m_create_confirmed);
				m_state = CREATE_CONFIRM;
			} else {
				m_state = DO_CREATE;
				handle();
			}
		} else {
			m_state = START_FILE;
			handle();
		}
		break;
	}

	case CREATE_CONFIRM:
		m_state = m_create_confirmed ? DO_CREATE : START_FILE;
		handle();
		break;

	case CHECK_CREATE:
		m_state = m_create_ok ? CREATE_FILE : START_FILE;
		handle();
		break;

	case DO_CREATE: {
		auto path = util::zippath_combine(m_current_directory, m_current_file);
		image_init_result err = m_image.create(path, nullptr, nullptr);
		if (err != image_init_result::PASS)
			machine().popmessage("Error: %s", m_image.error());
		stack_pop();
		break;
	}
	}
}

} // namespace ui
