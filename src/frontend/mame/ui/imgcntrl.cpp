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

#include "util/path.h"
#include "util/zippath.h"


namespace ui {

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

//-------------------------------------------------
//  ctor
//-------------------------------------------------

menu_control_device_image::menu_control_device_image(mame_ui_manager &mui, render_target &target, device_image_interface &image)
	: menu(mui, target)
	, m_image(image)
	, m_swi(nullptr)
	, m_swp(nullptr)
	, m_sld(nullptr)
{
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
//  test_create
//-------------------------------------------------

void menu_control_device_image::test_create(std::string const &path, bool &can_create, bool &need_confirm)
{
	// does a file or a directory exist at the path
	auto entry = osd_stat(path);
	auto file_type = (entry != nullptr) ? entry->type : osd::directory::entry::entry_type::NONE;

	switch (file_type)
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
//  hook_load
//-------------------------------------------------

bool menu_control_device_image::hook_load(std::string const &name)
{
	auto [err, msg] = m_image.load(name);
	if (err)
	{
		machine().popmessage(_("Error loading media image: %1$s"), !msg.empty() ? msg : err.message());
		return false;
	}
	else
	{
		return true;
	}
}


//-------------------------------------------------
//  hook_create
//-------------------------------------------------

bool menu_control_device_image::hook_create(std::string_view path)
{
	auto [err, msg] = m_image.create(path, nullptr, nullptr);
	if (!err)
		return true;

	machine().popmessage(_("Error creating media image: %1$s"), !msg.empty() ? msg : err.message());
	return false;
}


//-------------------------------------------------
//  start_file
//-------------------------------------------------

void menu_control_device_image::start_file()
{
	menu::stack_push<menu_file_selector>(
			ui(), target(),
			m_image,
			m_current_directory,
			m_current_file,
			true,
			m_image.image_interface() != nullptr,
			m_image.is_creatable(),
			[this] (menu_file_selector::result result, std::string const &directory, std::string const &file)
			{
				m_current_directory = directory;
				m_current_file = file;
				switch (result)
				{
				case menu_file_selector::result::EMPTY:
					m_image.unload();
					stack_pop(); // pop the file selection menu
					stack_pop(); // pop the fake menu that orchestrates the other menus
					break;

				case menu_file_selector::result::FILE:
					if (hook_load(file))
					{
						stack_pop(); // pop the file selection menu
						stack_pop(); // pop the fake menu that orchestrates the other menus
					}
					break;

				case menu_file_selector::result::CREATE:
					menu::stack_push<menu_file_create>(
							ui(),
							target(),
							m_image,
							directory,
							std::string(core_filename_extract_base(m_current_file)),
							[this] (std::string const &name) { create_file(m_current_directory, name); });
					break;

				case menu_file_selector::result::SOFTLIST:
					stack_pop(); // pop the file selection menu
					start_softlist();
					break;

				case menu_file_selector::result::MIDI:
					stack_pop(); // pop the file selection menu
					start_midi();
					break;
				}
			});
}


//-------------------------------------------------
//  start_softlist
//-------------------------------------------------

void menu_control_device_image::start_softlist()
{
	menu::stack_push<menu_software>(
			ui(),
			target(),
			m_image.image_interface(),
			[this] (software_list_device *sld)
			{
				if (!sld)
				{
					stack_pop(); // pop the software lists menu
					stack_pop(); // pop the fake menu that orchestrates the other menus
				}
				else
				{
					menu::stack_push<menu_software_list>(
							ui(),
							target(),
							*sld,
							m_image.image_interface(),
							[this, sld] (std::string_view item) { software_item_selected(*sld, item); });
				}
			});
}


//-------------------------------------------------
//  start_midi
//-------------------------------------------------

void menu_control_device_image::start_midi()
{
	menu::stack_push<menu_midi_inout>(
			ui(),
			target(),
			m_image.device().type() == MIDIIN,
			[this] (std::string const &port)
			{
				auto const [err, msg] = m_image.load(port);
				if (err)
				{
					machine().popmessage(_("Error opening MIDI port: %1$s"), !msg.empty() ? msg : err.message());
				}
				else
				{
					stack_pop(); // pop the MIDI port selection menu
					stack_pop(); // pop the fake menu that orchestrates the other menus
				}
			});
}


//-------------------------------------------------
//  load_software_part
//-------------------------------------------------

bool menu_control_device_image::load_software_part(software_list_device &sld, software_info const &swi, software_part const &swp)
{
	std::string temp_name = string_format("%s:%s:%s", sld.list_name(), swi.shortname(), swp.name());

	driver_enumerator drivlist(machine().options(), machine().options().system_name());
	drivlist.next();
	media_auditor auditor(drivlist);
	media_auditor::summary summary = auditor.audit_software(sld, swi, AUDIT_VALIDATE_FAST);

	// if everything looks good, load software
	if ((summary == media_auditor::CORRECT) || (summary == media_auditor::BEST_AVAILABLE) || (summary == media_auditor::NONE_NEEDED))
	{
		auto [err, msg] = m_image.load_software(temp_name);
		if (!err)
			return true;

		machine().popmessage(_("Error loading software item: %1$s"), !msg.empty() ? msg : err.message());
		return false;
	}
	else
	{
		machine().popmessage(_("Files required for the selected software are missing or incorrect."));
		return false;
	}
}


//-------------------------------------------------
//  software_item_selected
//-------------------------------------------------

void menu_control_device_image::software_item_selected(software_list_device &sld, std::string_view item)
{
	auto *const swi = sld.find(item);
	assert(swi);

	if (swi->has_multiple_parts(m_image.image_interface()))
	{
		menu::stack_push<menu_software_parts>(
				ui(),
				target(),
				*swi,
				m_image.image_interface(),
				false,
				[this, &sld, swi] (menu_software_parts::result action, software_part const *swp)
				{
					assert(menu_software_parts::result::ENTRY == action);
					assert(swp);

					if (load_software_part(sld, *swi, *swp))
					{
						stack_pop(); // pop the software parts menu
						stack_pop(); // pop the software items menu
						stack_pop(); // pop the software lists menu
						stack_pop(); // pop the fake menu that orchestrates the other menus
					}
				});
	}
	else
	{
		auto *const swp = swi->find_part("", m_image.image_interface());
		assert(swp);

		if (load_software_part(sld, *swi, *swp))
		{
			stack_pop(); // pop the software items menu
			stack_pop(); // pop the software lists menu
			stack_pop(); // pop the fake menu that orchestrates the other menus
		}
	}
}


//-------------------------------------------------
//  other_part_selected
//-------------------------------------------------

void menu_control_device_image::other_part_selected(menu_software_parts::result action, software_part const *swp)
{
	switch (action)
	{
	case menu_software_parts::result::ENTRY:
		assert(swp);
		if (load_software_part(*m_sld, *m_swi, *swp))
		{
			stack_pop(); // pop the software parts menu
			stack_pop(); // pop the fake menu that orchestrates the other menus
		}
		break;

	case menu_software_parts::result::FMGR:
		stack_pop(); // pop the software parts menu
		start_file();
		break;

	case menu_software_parts::result::EMPTY:
		m_image.unload();
		stack_pop(); // pop the software parts menu
		stack_pop(); // pop the fake menu that orchestrates the other menus
		break;

	case menu_software_parts::result::SWLIST:
		stack_pop(); // pop the software parts menu
		start_softlist();
		break;
	}
}


//-------------------------------------------------
//  create_file
//-------------------------------------------------

void menu_control_device_image::create_file(std::string const &directory, std::string const &name)
{
	auto path = util::zippath_combine(directory, name);
	bool can_create, need_confirm;
	test_create(path, can_create, need_confirm);
	if (!can_create)
		return;

	if (need_confirm)
	{
		menu::stack_push<menu_confirm_save_as>(
				ui(),
				target(),
				[this, path] ()
				{
					stack_pop(); // pop the confirm overwrite menu
					if (hook_create(path))
					{
						stack_pop(); // pop the create file menu
						stack_pop(); // pop the file selection menu
						stack_pop(); // pop the fake menu that orchestrates the other menus
					}
				});
	}
	else
	{
		if (hook_create(path))
		{
			stack_pop(); // pop the create file menu
			stack_pop(); // pop the file selection menu
			stack_pop(); // pop the fake menu that orchestrates the other menus
		}
	}
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
	switch (m_state)
	{
	case START_FILE:
		start_file();
		m_state = DONE;
		break;

	case START_OTHER_PART:
		menu::stack_push<menu_software_parts>(
				ui(),
				target(),
				*m_swi,
				m_image.image_interface(),
				true,
				[this] (menu_software_parts::result action, software_part const *swp) { other_part_selected(action, swp); });
		m_state = DONE;
		break;

	case DONE:
		stack_pop();
		break;
	}
}

} // namespace ui
