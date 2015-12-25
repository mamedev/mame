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



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct config_type
{
	config_type *           next;               /* next in line */
	const char *            name;               /* node name */
	config_saveload_delegate load;              /* load callback */
	config_saveload_delegate save;              /* save callback */
};



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

static config_type *typelist;



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static int config_load_xml(running_machine &machine, emu_file &file, int type);
static int config_save_xml(running_machine &machine, emu_file &file, int type);



/***************************************************************************
    CORE IMPLEMENTATION
***************************************************************************/

/*************************************
 *
 *  Reset the configuration callbacks
 *
 *************************************/

void config_init(running_machine &machine)
{
	typelist = nullptr;
}



/*************************************
 *
 *  Register to be involved in config
 *  save/load
 *
 *************************************/

void config_register(running_machine &machine, const char *nodename, config_saveload_delegate load, config_saveload_delegate save)
{
	config_type *newtype;
	config_type **ptype;

	/* allocate a new type */
	newtype = auto_alloc(machine, config_type);
	newtype->next = nullptr;
	newtype->name = nodename;
	newtype->load = load;
	newtype->save = save;

	/* add us to the end */
	for (ptype = &typelist; *ptype; ptype = &(*ptype)->next) { }
	*ptype = newtype;
}



/*************************************
 *
 *  Settings save/load frontend
 *
 *************************************/

int config_load_settings(running_machine &machine)
{
	const char *controller = machine.options().ctrlr();
	config_type *type;
	int loaded = 0;

	/* loop over all registrants and call their init function */
	for (type = typelist; type; type = type->next)
		type->load(CONFIG_TYPE_INIT, nullptr);

	/* now load the controller file */
	if (controller[0] != 0)
	{
		/* open the config file */
		emu_file file(machine.options().ctrlr_path(), OPEN_FLAG_READ);
		file_error filerr = file.open(controller, ".cfg");

		if (filerr != FILERR_NONE)
			throw emu_fatalerror("Could not load controller file %s.cfg", controller);

		/* load the XML */
		if (!config_load_xml(machine, file, CONFIG_TYPE_CONTROLLER))
			throw emu_fatalerror("Could not load controller file %s.cfg", controller);
	}

	/* next load the defaults file */
	emu_file file(machine.options().cfg_directory(), OPEN_FLAG_READ);
	file_error filerr = file.open("default.cfg");
	if (filerr == FILERR_NONE)
		config_load_xml(machine, file, CONFIG_TYPE_DEFAULT);

	/* finally, load the game-specific file */
	filerr = file.open(machine.basename(), ".cfg");
	if (filerr == FILERR_NONE)
		loaded = config_load_xml(machine, file, CONFIG_TYPE_GAME);

	/* loop over all registrants and call their final function */
	for (type = typelist; type; type = type->next)
		type->load(CONFIG_TYPE_FINAL, nullptr);

	/* if we didn't find a saved config, return 0 so the main core knows that it */
	/* is the first time the game is run and it should diplay the disclaimer. */
	return loaded;
}


void config_save_settings(running_machine &machine)
{
	config_type *type;

	/* loop over all registrants and call their init function */
	for (type = typelist; type; type = type->next)
		type->save(CONFIG_TYPE_INIT, nullptr);

	/* save the defaults file */
	emu_file file(machine.options().cfg_directory(), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
	file_error filerr = file.open("default.cfg");
	if (filerr == FILERR_NONE)
		config_save_xml(machine, file, CONFIG_TYPE_DEFAULT);

	/* finally, save the game-specific file */
	filerr = file.open(machine.basename(), ".cfg");
	if (filerr == FILERR_NONE)
		config_save_xml(machine, file, CONFIG_TYPE_GAME);

	/* loop over all registrants and call their final function */
	for (type = typelist; type; type = type->next)
		type->save(CONFIG_TYPE_FINAL, nullptr);
}



/*************************************
 *
 *  XML file load
 *
 *************************************/

static int config_load_xml(running_machine &machine, emu_file &file, int which_type)
{
	xml_data_node *root, *confignode, *systemnode;
	config_type *type;
	const char *srcfile;
	int version, count;

	/* read the file */
	root = xml_file_read(file, nullptr);
	if (!root)
		goto error;

	/* find the config node */
	confignode = xml_get_sibling(root->child, "mameconfig");
	if (!confignode)
		goto error;

	/* validate the config data version */
	version = xml_get_attribute_int(confignode, "version", 0);
	if (version != CONFIG_VERSION)
		goto error;

	/* strip off all the path crap from the source filename */
	srcfile = strrchr(machine.system().source_file, '/');
	if (!srcfile)
		srcfile = strrchr(machine.system().source_file, '\\');
	if (!srcfile)
		srcfile = strrchr(machine.system().source_file, ':');
	if (!srcfile)
		srcfile = machine.system().source_file;
	else
		srcfile++;

	/* loop over all system nodes in the file */
	count = 0;
	for (systemnode = xml_get_sibling(confignode->child, "system"); systemnode; systemnode = xml_get_sibling(systemnode->next, "system"))
	{
		/* look up the name of the system here; skip if none */
		const char *name = xml_get_attribute_string(systemnode, "name", "");

		/* based on the file type, determine whether we have a match */
		switch (which_type)
		{
			case CONFIG_TYPE_GAME:
				/* only match on the specific game name */
				if (strcmp(name, machine.system().name) != 0)
					continue;
				break;

			case CONFIG_TYPE_DEFAULT:
				/* only match on default */
				if (strcmp(name, "default") != 0)
					continue;
				break;

			case CONFIG_TYPE_CONTROLLER:
			{
				int clone_of;
				/* match on: default, game name, source file name, parent name, grandparent name */
				if (strcmp(name, "default") != 0 &&
					strcmp(name, machine.system().name) != 0 &&
					strcmp(name, srcfile) != 0 &&
					((clone_of = driver_list::clone(machine.system())) == -1 || strcmp(name, driver_list::driver(clone_of).name) != 0) &&
					(clone_of == -1 || ((clone_of = driver_list::clone(clone_of)) == -1) || strcmp(name, driver_list::driver(clone_of).name) != 0))
					continue;
				break;
			}
		}

		/* log that we are processing this entry */
		if (DEBUG_CONFIG)
			osd_printf_debug("Entry: %s -- processing\n", name);

		/* loop over all registrants and call their load function */
		for (type = typelist; type; type = type->next)
			type->load(which_type, xml_get_sibling(systemnode->child, type->name));
		count++;
	}

	/* error if this isn't a valid game match */
	if (count == 0)
		goto error;

	/* free the parser */
	xml_file_free(root);
	return 1;

error:
	if (root)
		xml_file_free(root);
	return 0;
}



/*************************************
 *
 *  XML file save
 *
 *************************************/

static int config_save_xml(running_machine &machine, emu_file &file, int which_type)
{
	xml_data_node *root = xml_file_create();
	xml_data_node *confignode, *systemnode;
	config_type *type;

	/* if we don't have a root, bail */
	if (!root)
		return 0;

	/* create a config node */
	confignode = xml_add_child(root, "mameconfig", nullptr);
	if (!confignode)
		goto error;
	xml_set_attribute_int(confignode, "version", CONFIG_VERSION);

	/* create a system node */
	systemnode = xml_add_child(confignode, "system", nullptr);
	if (!systemnode)
		goto error;
	xml_set_attribute(systemnode, "name", (which_type == CONFIG_TYPE_DEFAULT) ? "default" : machine.system().name);

	/* create the input node and write it out */
	/* loop over all registrants and call their save function */
	for (type = typelist; type; type = type->next)
	{
		xml_data_node *curnode = xml_add_child(systemnode, type->name, nullptr);
		if (!curnode)
			goto error;
		type->save(which_type, curnode);

		/* if nothing was added, just nuke the node */
		if (!curnode->value && !curnode->child)
			xml_delete_node(curnode);
	}

	/* flush the file */
	xml_file_write(root, file);

	/* free and get out of here */
	xml_file_free(root);
	return 1;

error:
	xml_file_free(root);
	return 0;
}
