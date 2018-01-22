// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles
/***************************************************************************

    config.c

    Configuration file I/O.
***************************************************************************/

#include "emu.h"
#include "emuopts.h"
#include "drivenum.h"
#include "config.h"
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

void configuration_manager::config_register(const char* nodename, config_load_delegate load, config_save_delegate save)
{
	config_element element;
	element.name = nodename;
	element.load = load;
	element.save = save;

	m_typelist.push_back(element);
}



/*************************************
 *
 *  Settings save/load frontend
 *
 *************************************/

int configuration_manager::load_settings()
{
	const char *controller = machine().options().ctrlr();
	int loaded = 0;

	/* loop over all registrants and call their init function */
	for (auto type : m_typelist)
		type.load(config_type::INIT, nullptr);

	/* now load the controller file */
	if (controller[0] != 0)
	{
		/* open the config file */
		emu_file file(machine().options().ctrlr_path(), OPEN_FLAG_READ);
		osd_printf_verbose("Attempting to parse: %s.cfg\n",controller);
		osd_file::error filerr = file.open(controller, ".cfg");

		if (filerr != osd_file::error::NONE)
			throw emu_fatalerror("Could not load controller file %s.cfg", controller);

		/* load the XML */
		if (!load_xml(file, config_type::CONTROLLER))
			throw emu_fatalerror("Could not load controller file %s.cfg", controller);
	}

	/* next load the defaults file */
	emu_file file(machine().options().cfg_directory(), OPEN_FLAG_READ);
	osd_file::error filerr = file.open("default.cfg");
	osd_printf_verbose("Attempting to parse: default.cfg\n");
	if (filerr == osd_file::error::NONE)
		load_xml(file, config_type::DEFAULT);

	/* finally, load the game-specific file */
	filerr = file.open(machine().basename(), ".cfg");
	osd_printf_verbose("Attempting to parse: %s.cfg\n",machine().basename());
	if (filerr == osd_file::error::NONE)
		loaded = load_xml(file, config_type::GAME);

	/* loop over all registrants and call their final function */
	for (auto type : m_typelist)
		type.load(config_type::FINAL, nullptr);

	/* if we didn't find a saved config, return 0 so the main core knows that it */
	/* is the first time the game is run and it should display the disclaimer. */
	return loaded;
}


void configuration_manager::save_settings()
{
	/* loop over all registrants and call their init function */
	for (auto type : m_typelist)
		type.save(config_type::INIT, nullptr);

	/* save the defaults file */
	emu_file file(machine().options().cfg_directory(), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
	osd_file::error filerr = file.open("default.cfg");
	if (filerr == osd_file::error::NONE)
		save_xml(file, config_type::DEFAULT);

	/* finally, save the game-specific file */
	filerr = file.open(machine().basename(), ".cfg");
	if (filerr == osd_file::error::NONE)
		save_xml(file, config_type::GAME);

	/* loop over all registrants and call their final function */
	for (auto type : m_typelist)
		type.save(config_type::FINAL, nullptr);
}



/*************************************
 *
 *  XML file load
 *
 *************************************/

int configuration_manager::load_xml(emu_file &file, config_type which_type)
{
	/* read the file */
	util::xml::file::ptr const root(util::xml::file::read(file, nullptr));
	if (!root)
		return 0;

	/* find the config node */
	util::xml::data_node const *const confignode = root->get_child("mameconfig");
	if (!confignode)
		return 0;

	/* validate the config data version */
	int const version = confignode->get_attribute_int("version", 0);
	if (version != CONFIG_VERSION)
		return 0;

	/* strip off all the path crap from the source filename */
	const char *srcfile = strrchr(machine().system().type.source(), '/');
	if (!srcfile)
		srcfile = strrchr(machine().system().type.source(), '\\');
	if (!srcfile)
		srcfile = strrchr(machine().system().type.source(), ':');
	if (!srcfile)
		srcfile = machine().system().type.source();
	else
		srcfile++;

	/* loop over all system nodes in the file */
	int count = 0;
	for (util::xml::data_node const *systemnode = confignode->get_child("system"); systemnode; systemnode = systemnode->get_next_sibling("system"))
	{
		/* look up the name of the system here; skip if none */
		const char *name = systemnode->get_attribute_string("name", "");

		/* based on the file type, determine whether we have a match */
		switch (which_type)
		{
		case config_type::GAME:
			/* only match on the specific game name */
			if (strcmp(name, machine().system().name) != 0)
				continue;
			break;

		case config_type::DEFAULT:
			/* only match on default */
			if (strcmp(name, "default") != 0)
				continue;
			break;

		case config_type::CONTROLLER:
			{
				int clone_of;
				/* match on: default, game name, source file name, parent name, grandparent name */
				if (strcmp(name, "default") != 0 &&
					strcmp(name, machine().system().name) != 0 &&
					strcmp(name, srcfile) != 0 &&
					((clone_of = driver_list::clone(machine().system())) == -1 || strcmp(name, driver_list::driver(clone_of).name) != 0) &&
					(clone_of == -1 || ((clone_of = driver_list::clone(clone_of)) == -1) || strcmp(name, driver_list::driver(clone_of).name) != 0))
					continue;
				break;
			}

		default:
			break;
		}

		/* log that we are processing this entry */
		if (DEBUG_CONFIG)
			osd_printf_debug("Entry: %s -- processing\n", name);

		/* loop over all registrants and call their load function */
		for (auto type : m_typelist)
			type.load(which_type, systemnode->get_child(type.name.c_str()));
		count++;
	}

	/* error if this isn't a valid game match */
	if (count == 0)
		return 0;

	return 1;
}



/*************************************
 *
 *  XML file save
 *
 *************************************/

int configuration_manager::save_xml(emu_file &file, config_type which_type)
{
	util::xml::file::ptr root(util::xml::file::create());

	/* if we don't have a root, bail */
	if (!root)
		return 0;

	/* create a config node */
	util::xml::data_node *const confignode = root->add_child("mameconfig", nullptr);
	if (!confignode)
		return 0;
	confignode->set_attribute_int("version", CONFIG_VERSION);

	/* create a system node */
	util::xml::data_node *const systemnode = confignode->add_child("system", nullptr);
	if (!systemnode)
		return 0;
	systemnode->set_attribute("name", (which_type == config_type::DEFAULT) ? "default" : machine().system().name);

	/* create the input node and write it out */
	/* loop over all registrants and call their save function */
	for (auto type : m_typelist)
	{
		util::xml::data_node *const curnode = systemnode->add_child(type.name.c_str(), nullptr);
		if (!curnode)
			return 0;
		type.save(which_type, curnode);

		/* if nothing was added, just nuke the node */
		if (!curnode->get_value() && !curnode->get_first_child())
			curnode->delete_node();
	}

	/* flush the file */
	root->write(file);

	/* free and get out of here */
	return 1;
}
