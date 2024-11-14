// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    ui/imgcntrl.cpp

    MAME's clunky built-in file manager

***************************************************************************/

#include "emu.h"
#include "ui/imgcntrl.h"

#include "ui/filecreate.h"
#include "ui/filesel.h"
#include "ui/midiinout.h"
#include "ui/swlist.h"
#include "ui/ui.h"

#include "bus/midi/midiinport.h"
#include "bus/midi/midioutport.h"

#include "audit.h"
#include "drivenum.h"
#include "emuopts.h"
#include "image.h"
#include "softlist_dev.h"

#include "util/zippath.h"


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
	, m_swi(nullptr)
	, m_swp(nullptr)
	, m_sld(nullptr)
{
	m_submenu_result.i = -1;

	if (m_image.software_list_name())
		m_sld = software_list_device::find_by_name(mui.machine().config(), m_image.software_list_name());
	m_swi = m_image.software_entry();
	m_swp = m_image.part_entry();

	// if there's no image mounted, check for a software item with compatible parts mounted elsewhere
	if (!m_image.exists() && m_image.image_interface())
	{
		assert(!m_swi);

		for (device_image_interface &other : image_interface_enumerator(mui.machine().root_device()))
		{
			if (other.loaded_through_softlist() && (!m_sld || (m_sld->list_name() == other.software_list_name())))
			{
				software_info const &swi = *other.software_entry();
				for (software_part const &swp : swi.parts())
				{
					if (swp.interface() == m_image.image_interface())
					{
						if (!m_sld)
							m_sld = software_list_device::find_by_name(mui.machine().config(), other.software_list_name());
						m_swi = &swi;
						break;
					}
				}
			}

			if (m_swi)
				break;
		}
	}

	if (m_swi)
	{
		m_state = START_OTHER_PART;
		m_current_directory = m_image.working_directory();

		// check to see if we've never initialized the working directory
		if (m_current_directory.empty())
		{
			m_current_directory = machine().image().setup_working_directory();
			m_image.set_working_directory(m_current_directory);
		}
	}
	else
	{
		m_state = START_FILE;

		// if the image exists, set the working directory to the parent directory
		if (m_image.exists())
		{
			m_current_file.assign(m_image.filename());
			m_current_directory = util::zippath_parent(m_current_file);
		}
		else
		{
			m_current_directory = m_image.working_directory();

			// check to see if we've never initialized the working directory
			if (m_current_directory.empty())
			{
				m_current_directory = machine().image().setup_working_directory();
				m_image.set_working_directory(m_current_directory);
			}
		}

		// check to see if the path exists; if not then set to current directory
		util::zippath_directory::ptr dir;
		if (util::zippath_directory::open(m_current_directory, dir))
			osd_get_full_path(m_current_directory, ".");
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
	auto entry = osd_stat(path);
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
	media_auditor::summary summary = auditor.audit_software(*m_sld, *m_swi, AUDIT_VALIDATE_FAST);
	// if everything looks good, load software
	if (summary == media_auditor::CORRECT || summary == media_auditor::BEST_AVAILABLE || summary == media_auditor::NONE_NEEDED)
	{
		auto [err, msg] = m_image.load_software(temp_name);
		if (err)
			machine().popmessage(_("Error loading software item: %1$s"), !msg.empty() ? msg : err.message());
		stack_pop();
	}
	else
	{
		machine().popmessage(_("The software selected is missing one or more required ROM or CHD images.\nPlease acquire the correct files or select a different one."));
		m_state = SELECT_SOFTLIST;
		menu_activated();
	}
}


//-------------------------------------------------
//  hook_load
//-------------------------------------------------

void menu_control_device_image::hook_load(const std::string &name)
{
	auto [err, msg] = m_image.load(name);
	if (err)
		machine().popmessage(_("Error loading media image: %1$s"), !msg.empty() ? msg : err.message());
	stack_pop();
}


//-------------------------------------------------
//  populate
//-------------------------------------------------

void menu_control_device_image::populate()
{
	throw emu_fatalerror("menu_control_device_image::populate: Shouldn't get here!");
}


//-------------------------------------------------
//  handle
//-------------------------------------------------

bool menu_control_device_image::handle(event const *ev)
{
	throw emu_fatalerror("menu_control_device_image::handle: Shouldn't get here!");
}


//-------------------------------------------------
//  menu_activated
//-------------------------------------------------

void menu_control_device_image::menu_activated()
{
	switch(m_state)
	{
	case START_FILE:
		menu::stack_push<menu_file_selector>(
				ui(), container(),
				&m_image,
				m_current_directory,
				m_current_file,
				true,
				m_image.image_interface() != nullptr,
				m_image.is_creatable(),
				[this] (menu_file_selector::result result, std::string &&directory, std::string &&file)
				{
					m_current_directory = std::move(directory);
					m_current_file = std::move(file);
					switch (result)
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
						break;

					case menu_file_selector::result::MIDI:
						m_state = START_MIDI;
						break;

					default: // return to system
						stack_pop();
						break;
					}
				});
		break;

	case START_SOFTLIST:
		m_sld = nullptr;
		menu::stack_push<menu_software>(ui(), container(), m_image.image_interface(), &m_sld);
		m_state = SELECT_SOFTLIST;
		break;

	case START_MIDI:
		m_midi = "";
		menu::stack_push<menu_midi_inout>(ui(), container(), m_image.device().type() == MIDIIN, &m_midi);
		m_state = SELECT_MIDI;
		break;

	case SELECT_MIDI:
		if(!m_midi.empty())
		{
			auto [err, msg] = m_image.load(m_midi);
			if (err)
                machine().popmessage(_("Error connecting to midi port: %1$s"), !msg.empty() ? msg : err.message());
		}
		stack_pop();
		break;

	case START_OTHER_PART:
		m_submenu_result.swparts = menu_software_parts::result::INVALID;
		menu::stack_push<menu_software_parts>(ui(), container(), m_swi, m_image.image_interface(), &m_swp, true, m_submenu_result.swparts);
		m_state = SELECT_OTHER_PART;
		break;

	case SELECT_SOFTLIST:
		if (!m_sld)
		{
			stack_pop();
		}
		else
		{
			m_software_info_name.clear();
			menu::stack_push<menu_software_list>(ui(), container(), m_sld, m_image.image_interface(), m_software_info_name);
			m_state = SELECT_PARTLIST;
		}
		break;

	case SELECT_PARTLIST:
		m_swi = m_sld->find(m_software_info_name);
		if (!m_swi)
		{
			m_state = START_SOFTLIST;
			menu_activated();
		}
		else if (m_swi->has_multiple_parts(m_image.image_interface()))
		{
			m_submenu_result.swparts = menu_software_parts::result::INVALID;
			m_swp = nullptr;
			menu::stack_push<menu_software_parts>(ui(), container(), m_swi, m_image.image_interface(), &m_swp, false, m_submenu_result.swparts);
			m_state = SELECT_ONE_PART;
		}
		else
		{
			m_swp = m_swi->find_part("", m_image.image_interface());
			load_software_part();
		}
		break;

	case SELECT_ONE_PART:
		switch (m_submenu_result.swparts)
		{
		case menu_software_parts::result::ENTRY:
			load_software_part();
			break;

		default: // return to list
			m_state = SELECT_SOFTLIST;
			menu_activated();
			break;
		}
		break;

	case SELECT_OTHER_PART:
		switch (m_submenu_result.swparts)
		{
		case menu_software_parts::result::ENTRY:
			load_software_part();
			break;

		case menu_software_parts::result::FMGR:
			m_state = START_FILE;
			menu_activated();
			break;

		case menu_software_parts::result::EMPTY:
			m_image.unload();
			stack_pop();
			break;

		case menu_software_parts::result::SWLIST:
			m_state = START_SOFTLIST;
			menu_activated();
			break;

		case menu_software_parts::result::INVALID: // return to system
			stack_pop();
			break;
		}
		break;

	case CREATE_FILE:
		{
			bool can_create, need_confirm;
			test_create(can_create, need_confirm);
			if (can_create)
			{
				if (need_confirm)
				{
					menu::stack_push<menu_confirm_save_as>(ui(), container(), m_create_confirmed);
					m_state = CREATE_CONFIRM;
				}
				else
				{
					m_state = DO_CREATE;
					menu_activated();
				}
			}
			else
			{
				m_state = START_FILE;
				menu_activated();
			}
		}
		break;

	case CREATE_CONFIRM:
		m_state = m_create_confirmed ? DO_CREATE : START_FILE;
		menu_activated();
		break;

	case CHECK_CREATE:
		m_state = m_create_ok ? CREATE_FILE : START_FILE;
		menu_activated();
		break;

	case DO_CREATE:
		{
			auto path = util::zippath_combine(m_current_directory, m_current_file);
			auto [err, msg] = m_image.create(path, nullptr, nullptr);
			if (err)
				machine().popmessage(_("Error creating media image: %1$s"), !msg.empty() ? msg : err.message());
			stack_pop();
		}
		break;
	}
}

} // namespace ui
