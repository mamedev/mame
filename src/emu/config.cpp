// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles
/***************************************************************************

    config.cpp

    Configuration file I/O.

***************************************************************************/

#include "emu.h"
#include "config.h"

#include "drivenum.h"
#include "emuopts.h"
#include "fileio.h"

#include "xmlfile.h"

#include <system_error>
#include <utility>

#define DEBUG_CONFIG        0


//**************************************************************************
//  CONFIGURATION MANAGER
//**************************************************************************

/*************************************
 *
 *  Construction/destruction
 *
 *************************************/

configuration_manager::configuration_manager(running_machine &machine) :
	m_machine(machine),
	m_typelist()
{
}


configuration_manager::~configuration_manager()
{
}



/*************************************
 *
 *  Register to be involved in config
 *  save/load
 *
 *************************************/

void configuration_manager::config_register(std::string_view name, load_delegate &&load, save_delegate &&save)
{
	m_typelist.emplace(name, config_handler{ std::move(load), std::move(save) });
}



/*************************************
 *
 *  Settings save/load frontend
 *
 *************************************/

bool configuration_manager::load_settings()
{
	// loop over all registrants and call their init function
	for (auto const &type : m_typelist)
		type.second.load(config_type::INIT, config_level::DEFAULT, nullptr);

	// now load the controller file
	char const *const controller = machine().options().ctrlr();
	if (controller && *controller)
	{
		emu_file file(machine().options().ctrlr_path(), OPEN_FLAG_READ);
		if (!attempt_load(machine().system(), file, std::string(controller) + ".cfg", config_type::CONTROLLER))
			throw emu_fatalerror("Could not load controller configuration file %s.cfg", controller);
	}

	// next load the defaults file
	emu_file file(machine().options().cfg_directory(), OPEN_FLAG_READ);
	attempt_load(machine().system(), file, "default.cfg", config_type::DEFAULT);

	// load the system-specific file
	bool const loaded = attempt_load(machine().system(), file, machine().basename() + ".cfg", config_type::SYSTEM);

	// loop over all registrants and call their final function
	for (auto const &type : m_typelist)
		type.second.load(config_type::FINAL, config_level::DEFAULT, nullptr);

	// if we didn't find a saved config, return false so the main core knows that it
	// is the first time the system has been run
	return loaded;
}


void configuration_manager::save_settings()
{
	// loop over all registrants and call their init function
	for (auto const &type : m_typelist)
		type.second.save(config_type::INIT, nullptr);

	// save the defaults file
	emu_file file(machine().options().cfg_directory(), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
	std::error_condition filerr = file.open("default.cfg");
	if (!filerr)
		save_xml(file, config_type::DEFAULT);

	// finally, save the system-specific file
	filerr = file.open(machine().basename() + ".cfg");
	if (!filerr)
		save_xml(file, config_type::SYSTEM);

	// loop over all registrants and call their final function
	for (auto const &type : m_typelist)
		type.second.save(config_type::FINAL, nullptr);
}



/*************************************
 *
 *  XML file load
 *
 *************************************/

bool configuration_manager::attempt_load(game_driver const &system, emu_file &file, std::string_view name, config_type which_type)
{
	std::error_condition const filerr = file.open(std::string(name));
	if (std::errc::no_such_file_or_directory == filerr)
	{
		osd_printf_verbose("Configuration file %s not found\n", name);
		return false;
	}
	else if (filerr)
	{
		osd_printf_warning(
				"Error opening configuration file %s (%s:%d %s)\n",
				name,
				filerr.category().name(),
				filerr.value(),
				filerr.message());
		return false;
	}
	else
	{
		osd_printf_verbose("Attempting to parse: %s\n", name);
		return load_xml(system, file, which_type);
	}
}


bool configuration_manager::load_xml(game_driver const &system, emu_file &file, config_type which_type)
{
	// read the file
	util::xml::file::ptr const root(util::xml::file::read(file, nullptr));
	if (!root)
	{
		osd_printf_warning("Error parsing XML configuration file %s\n", file.filename());
		return false;
	}

	// find the config node
	util::xml::data_node const *const confignode = root->get_child("mameconfig");
	if (!confignode)
	{
		osd_printf_warning("Could not find root mameconfig element in configuration file %s\n", file.filename());
		return false;
	}

	// validate the config data version
	int const version = confignode->get_attribute_int("version", 0);
	if (version != CONFIG_VERSION)
	{
		osd_printf_warning("Configuration file %s has unsupported version %d\n", file.filename(), version);
		return false;
	}

	// strip off all the path crap from the source filename
	char const *srcfile = strrchr(system.type.source(), '/');
	if (!srcfile)
		srcfile = strrchr(system.type.source(), '\\');
	if (!srcfile)
		srcfile = strrchr(system.type.source(), ':');
	if (!srcfile)
		srcfile = system.type.source();
	else
		srcfile++;

	// loop over all system nodes in the file
	int count = 0;
	for (util::xml::data_node const *systemnode = confignode->get_child("system"); systemnode; systemnode = systemnode->get_next_sibling("system"))
	{
		// look up the name of the system here; skip if none
		char const *name = systemnode->get_attribute_string("name", "");

		// based on the file type, determine whether we have a match
		config_level level = config_level::DEFAULT;
		switch (which_type)
		{
		case config_type::SYSTEM:
			// only match on the specific system name
			if (strcmp(name, system.name))
			{
				osd_printf_verbose("Ignoring configuration for system %s in system configuration file %s\n", name, file.filename());
				continue;
			}
			level = config_level::SYSTEM;
			break;

		case config_type::DEFAULT:
			// only match on default
			if (strcmp(name, "default"))
			{
				osd_printf_verbose("Ignoring configuration for system %s in default configuration file %s\n", name, file.filename());
				continue;
			}
			level = config_level::DEFAULT;
			break;

		case config_type::CONTROLLER:
			{
				// match on: default, system name, source file name, parent name, grandparent name
				int clone_of;
				if (!strcmp(name, "default"))
				{
					osd_printf_verbose("Applying default configuration from controller configuration file %s\n", file.filename());
					level = config_level::DEFAULT;
				}
				else if (!strcmp(name, system.name))
				{
					osd_printf_verbose("Applying configuration for system %s from controller configuration file %s\n", name, file.filename());
					level = config_level::SYSTEM;
				}
				else if (!strcmp(name, srcfile))
				{
					osd_printf_verbose("Applying configuration for source file %s from controller configuration file %s\n", name, file.filename());
					level = config_level::SOURCE;
				}
				else if (
						((clone_of = driver_list::clone(system)) != -1 && !strcmp(name, driver_list::driver(clone_of).name)) ||
						(clone_of != -1 && ((clone_of = driver_list::clone(clone_of)) != -1) && !strcmp(name, driver_list::driver(clone_of).name)))
				{
					osd_printf_verbose("Applying configuration for parent/BIOS %s from controller configuration file %s\n", name, file.filename());
					level = (driver_list::driver(clone_of).flags & MACHINE_IS_BIOS_ROOT) ? config_level::BIOS : config_level::PARENT;
				}
				else
				{
					continue;
				}
				break;
			}

		default:
			break;
		}

		// log that we are processing this entry
		if (DEBUG_CONFIG)
			osd_printf_debug("Entry: %s -- processing\n", name);

		// loop over all registrants and call their load function
		for (auto const &type : m_typelist)
			type.second.load(which_type, level, systemnode->get_child(type.first.c_str()));
		count++;

		// save unhandled settings for default and system types
		if (config_type::DEFAULT == which_type)
			save_unhandled(m_unhandled_default, *systemnode);
		else if (config_type::SYSTEM == which_type)
			save_unhandled(m_unhandled_system, *systemnode);
	}

	// error if this isn't a valid match
	return count != 0;
}



/*************************************
 *
 *  XML file save
 *
 *************************************/

bool configuration_manager::save_xml(emu_file &file, config_type which_type)
{
	// if we cant't create a root node, bail
	util::xml::file::ptr root(util::xml::file::create());
	if (!root)
		return false;

	// create a config node
	util::xml::data_node *const confignode = root->add_child("mameconfig", nullptr);
	if (!confignode)
		return false;
	confignode->set_attribute_int("version", CONFIG_VERSION);

	// create a system node
	util::xml::data_node *const systemnode = confignode->add_child("system", nullptr);
	if (!systemnode)
		return false;
	systemnode->set_attribute("name", (which_type == config_type::DEFAULT) ? "default" : machine().system().name);

	// loop over all registrants and call their save function
	util::xml::data_node *curnode = nullptr;
	for (auto const &type : m_typelist)
	{
		if (!curnode || (type.first != curnode->get_name()))
			curnode = systemnode->add_child(type.first.c_str(), nullptr);
		if (!curnode)
			return false;
		type.second.save(which_type, curnode);

		// if nothing was added, just nuke the node
		if (!curnode->get_value() && !curnode->get_first_child() && !curnode->count_attributes())
		{
			curnode->delete_node();
			curnode = nullptr;
		}
	}

	// restore unhandled settings
	if ((config_type::DEFAULT == which_type) && m_unhandled_default)
		restore_unhandled(*m_unhandled_default, *systemnode);
	else if ((config_type::SYSTEM == which_type) && m_unhandled_system)
		restore_unhandled(*m_unhandled_system, *systemnode);

	// write out the file
	root->write(file);

	// free and get out of here
	return true;
}



/*************************************
 *
 *  Preserving unhandled settings
 *
 *************************************/

void configuration_manager::save_unhandled(
		std::unique_ptr<util::xml::file> &unhandled,
		util::xml::data_node const &systemnode)
{
	for (util::xml::data_node const *curnode = systemnode.get_first_child(); curnode; curnode = curnode->get_next_sibling())
	{
		auto const handler = m_typelist.lower_bound(curnode->get_name());
		if ((m_typelist.end() == handler) || (handler->first != curnode->get_name()))
		{
			if (!unhandled)
				unhandled = util::xml::file::create();
			curnode->copy_into(*unhandled);
		}
	}
}


void configuration_manager::restore_unhandled(
		util::xml::file const &unhandled,
		util::xml::data_node &systemnode)
{
	for (util::xml::data_node const *curnode = unhandled.get_first_child(); curnode; curnode = curnode->get_next_sibling())
		curnode->copy_into(systemnode);
}
