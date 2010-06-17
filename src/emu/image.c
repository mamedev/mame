/***************************************************************************

    image.c

    Core image functions and definitions.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#include "emu.h"
#include "emuopts.h"
#include "image.h"
#include "config.h"
#include "xmlfile.h"


/***************************************************************************
    INITIALIZATION HELPERS
***************************************************************************/
//============================================================
//  filename_basename
//============================================================

static const char *filename_basename(const char *filename)
{
	const char *c;

	// NULL begets NULL
	if (!filename)
		return NULL;

	// start at the end and return when we hit a slash or colon
	for (c = filename + strlen(filename) - 1; c >= filename; c--)
		if (*c == '\\' || *c == '/' || *c == ':')
			return c + 1;

	// otherwise, return the whole thing
	return filename;
}

/*-------------------------------------------------
    image_dirs_load - loads image device directory
    configuration items
-------------------------------------------------*/

static void image_dirs_load(running_machine *machine, int config_type, xml_data_node *parentnode)
{
	xml_data_node *node;
	const char *dev_instance;
	const char *working_directory;
	device_image_interface *image = NULL;
	
	if ((config_type == CONFIG_TYPE_GAME) && (parentnode != NULL))
	{
		for (node = xml_get_sibling(parentnode->child, "device"); node; node = xml_get_sibling(node->next, "device"))
		{
			dev_instance = xml_get_attribute_string(node, "instance", NULL);

			if ((dev_instance != NULL) && (dev_instance[0] != '\0'))
			{
				for (bool gotone = machine->devicelist.first(image); gotone; gotone = image->next(image))
				{
					if (!strcmp(dev_instance, image->image_config().instance_name())) {
						working_directory = xml_get_attribute_string(node, "directory", NULL);
						if (working_directory != NULL)
							image->set_working_directory(working_directory);
					}					
				}
			}
		}
	}
}



/*-------------------------------------------------
    image_dirs_save - saves out image device
    directories to the configuration file
-------------------------------------------------*/

static void image_dirs_save(running_machine *machine, int config_type, xml_data_node *parentnode)
{
	xml_data_node *node;
	const char *dev_instance;
	device_image_interface *image = NULL;

	/* only care about game-specific data */
	if (config_type == CONFIG_TYPE_GAME)
	{
		for (bool gotone = machine->devicelist.first(image); gotone; gotone = image->next(image))
		{
			dev_instance = image->image_config().instance_name();

			node = xml_add_child(parentnode, "device", NULL);
			if (node != NULL)
			{
				xml_set_attribute(node, "instance", dev_instance);
				xml_set_attribute(node, "directory", image->working_directory());
			}		
		}
	}
}


/*-------------------------------------------------
    image_device_init - initialize devices for a specific
    running_machine
-------------------------------------------------*/

void image_device_init(running_machine *machine)
{
	const char *image_name;
	device_image_interface *image = NULL;

	/* make sure that any required devices have been allocated */
    for (bool gotone = machine->devicelist.first(image); gotone; gotone = image->next(image))
	{
		/* is an image specified for this image */
		image_name = image_get_device_option(image);

		if ((image_name != NULL) && (image_name[0] != '\0'))
		{
			bool result = FALSE;

			/* try to load this image */
			result = image->load(image_name);

			/* did the image load fail? */
			if (!result)
			{
				/* retrieve image error message */
				const char *image_err = "error";// image_error(image);
				char *image_basename = auto_strdup(machine, filename_basename(image_name));

				/* unload all images */
				//image_unload_all(machine);

				fatalerror_exitcode(machine, MAMERR_DEVICE, "Device %s load (%s) failed: %s",
					image->image_config().name(),
					image_basename,
					image_err);
			}
		}
		else
		{
			/* no image... must this device be loaded? */
			if (image->image_config().must_be_loaded())
			{
				fatalerror_exitcode(machine, MAMERR_DEVICE, "Driver requires that device \"%s\" must have an image to load", image->image_config().instance_name());
			}
		}
	}
}
/***************************************************************************
    INITIALIZATION
***************************************************************************/

/*-------------------------------------------------
    image_init - start up the image system
-------------------------------------------------*/

void image_init(running_machine *machine)
{
	image_device_init(machine);
	config_register(machine, "image_directories", image_dirs_load, image_dirs_save);
}
