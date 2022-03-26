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

#define DEBUG_CONFIG        0

//**************************************************************************
//  CONFIGURATION MANAGER
//**************************************************************************

//-------------------------------------------------
//  configuration_manager - constructor
//-------------------------------------------------


configuration_manager::configuration_manager(running_machine &machine)
	: m_machine(machine)
{
}

/*************************************
 *
 *  Register to be involved in config
 *  save/load
 *
 *************************************/

void configuration_manager::config_register(const char *nodename, load_delegate load, save_delegate save)
{
	config_element element;
	element.name = nodename;
	element.load = std::move(load);
	element.save = std::move(save);

	m_typelist.emplace_back(std::move(element));
}



/*************************************
 *
 *  Settings save/load frontend
 *
 *************************************/

bool configuration_manager::load_settings()
{
	// loop over all registrants and call their init function
	for (const auto &type : m_typelist)
		type.load(config_type::INIT, config_level::DEFAULT, nullptr);

	// now load the controller file
	char const *const controller = machine().options().ctrlr();
	if (controller && *controller)
	{
		// open the config file
		emu_file file(machine().options().ctrlr_path(), OPEN_FLAG_READ);
		osd_printf_verbose("Attempting to parse: %s.cfg\n", controller);
		std::error_condition const filerr = file.open(std::string(controller) + ".cfg");

		if (filerr)
			throw emu_fatalerror("Could not open controller file %s.cfg (%s:%d %s)", controller, filerr.category().name(), filerr.value(), filerr.message());

		// load the XML
		if (!load_xml(file, config_type::CONTROLLER))
			throw emu_fatalerror("Could not load controller file %s.cfg", controller);
	}

	// next load the defaults file
	emu_file file(machine().options().cfg_directory(), OPEN_FLAG_READ);
	std::error_condition filerr = file.open("default.cfg");
	osd_printf_verbose("Attempting to parse: default.cfg\n");
	if (!filerr)
		load_xml(file, config_type::DEFAULT);

	// finally, load the game-specific file
	filerr = file.open(machine().basename() + ".cfg");
	osd_printf_verbose("Attempting to parse: %s.cfg\n",machine().basename());
	const bool loaded = !filerr && load_xml(file, config_type::SYSTEM);

	// loop over all registrants and call their final function
	for (const auto &type : m_typelist)
		type.load(config_type::FINAL, config_level::DEFAULT, nullptr);

	// if we didn't find a saved config, return false so the main core knows that it
	// is the first time the game is run and it should display the disclaimer.
	return loaded;
}


void configuration_manager::save_settings()
{
	// loop over all registrants and call their init function
	for (const auto &type : m_typelist)
		type.save(config_type::INIT, nullptr);

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
	for (const auto &type : m_typelist)
		type.save(config_type::FINAL, nullptr);
}



/*************************************
 *
 *  XML file load
 *
 *************************************/

bool configuration_manager::load_xml(emu_file &file, config_type which_type)
{
	// read the file
	util::xml::file::ptr const root(util::xml::file::read(file, nullptr));
	if (!root)
	{
		osd_printf_verbose("Error parsing XML configuration file %s\n", file.filename());
		return false;
	}

	// find the config node
	util::xml::data_node const *const confignode = root->get_child("mameconfig");
	if (!confignode)
	{
		osd_printf_verbose("Could not find root mameconfig element in configuration file %s\n", file.filename());
		return false;
	}

	// validate the config data version
	int const version = confignode->get_attribute_int("version", 0);
	if (version != CONFIG_VERSION)
	{
		osd_printf_verbose("Configuration file %s has unsupported version %d\n", file.filename(), version);
		return false;
	}

	// strip off all the path crap from the source filename
	const char *srcfile = strrchr(machine().system().type.source(), '/');
	if (!srcfile)
		srcfile = strrchr(machine().system().type.source(), '\\');
	if (!srcfile)
		srcfile = strrchr(machine().system().type.source(), ':');
	if (!srcfile)
		srcfile = machine().system().type.source();
	else
		srcfile++;

	// loop over all system nodes in the file
	int count = 0;
	for (util::xml::data_node const *systemnode = confignode->get_child("system"); systemnode; systemnode = systemnode->get_next_sibling("system"))
	{
		// look up the name of the system here; skip if none
		const char *name = systemnode->get_attribute_string("name", "");

		// based on the file type, determine whether we have a match
		config_level level = config_level::DEFAULT;
		switch (which_type)
		{
		case config_type::SYSTEM:
			// only match on the specific system name
			if (strcmp(name, machine().system().name))
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
				else if (!strcmp(name, machine().system().name))
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
						((clone_of = driver_list::clone(machine().system())) != -1 && !strcmp(name, driver_list::driver(clone_of).name)) ||
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
		for (const auto &type : m_typelist)
			type.load(which_type, level, systemnode->get_child(type.name.c_str()));
		count++;
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
	for (const auto &type : m_typelist)
	{
		util::xml::data_node *const curnode = systemnode->add_child(type.name.c_str(), nullptr);
		if (!curnode)
			return false;
		type.save(which_type, curnode);

		// if nothing was added, just nuke the node
		if (!curnode->get_value() && !curnode->get_first_child() && !curnode->count_attributes())
			curnode->delete_node();
	}

	// flush the file
	root->write(file);

	// free and get out of here
	return true;
}
