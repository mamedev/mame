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
	util::xml::data_node empty;

	/* loop over all registrants and call their init function */
	for (auto type : m_typelist)
		type.load(config_type::INIT, empty);

	/* now load the controller file */
	if (controller[0] != 0)
	{
		/* open the config file */
		emu_file file(machine().options().ctrlr_path(), OPEN_FLAG_READ);
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
	if (filerr == osd_file::error::NONE)
		load_xml(file, config_type::DEFAULT);

	/* finally, load the game-specific file */
	filerr = file.open(machine().basename(), ".cfg");
	if (filerr == osd_file::error::NONE)
		loaded = load_xml(file, config_type::GAME);

	/* loop over all registrants and call their final function */
	for (auto type : m_typelist)
		type.load(config_type::FINAL, empty);

	/* if we didn't find a saved config, return 0 so the main core knows that it */
	/* is the first time the game is run and it should diplay the disclaimer. */
	return loaded;
}


void configuration_manager::save_settings()
{
	/* loop over all registrants and call their init function */
	util::xml::data_node empty;
	for (auto type : m_typelist)
		type.save(config_type::INIT, empty);

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
		type.save(config_type::FINAL, empty);
}



/*************************************
 *
 *  XML file load
 *
 *************************************/

int configuration_manager::load_xml(emu_file &file, config_type which_type)
{
	/* read the file */
	util::xml::document_node root(file);
	if (!root)
		return 0;

	/* find the config node */
	util::xml::data_node const confignode = root.child("mameconfig");
	if (!confignode)
		return 0;

	/* validate the config data version */
	int const version = confignode.attribute("version").as_int( 0);
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
	for (util::xml::data_node const systemnode : confignode.children("system"))
	{
		/* look up the name of the system here; skip if none */
		const char *name = systemnode.attribute("name").as_string( "");

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
			type.load(which_type, systemnode.child(type.name.c_str()));
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
	util::xml::document_node root;

	/* create a config node */
	util::xml::data_node confignode = root.append_child("mameconfig");
	if (!confignode)
		return 0;
	confignode.get_or_append_attribute("version").set_value( CONFIG_VERSION);

	/* create a system node */
	util::xml::data_node systemnode = confignode.append_child("system");
	if (!systemnode)
		return 0;
	systemnode.get_or_append_attribute("name").set_value( (which_type == config_type::DEFAULT) ? "default" : machine().system().name);

	/* create the input node and write it out */
	/* loop over all registrants and call their save function */
	for (auto type : m_typelist)
	{
		util::xml::data_node curnode = systemnode.append_child(type.name.c_str());
		if (!curnode)
			return 0;
		type.save(which_type, curnode);

		/* if nothing was added, just nuke the node */
		if (curnode.text().empty() && !curnode.first_child())
			curnode.parent().remove_child(curnode);
	}

	/* flush the file */
	root.file_write(file);

	/* free and get out of here */
	return 1;
}
