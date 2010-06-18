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
    image_unload_all - unload all images and
    extract options
-------------------------------------------------*/

void image_unload_all(running_machine *machine)
{
    device_image_interface *image = NULL;

	// extract the options 
	//mess_options_extract(machine);

	for (bool gotone = machine->devicelist.first(image); gotone; gotone = image->next(image))
	{
		// unload this image
		image->unload();
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

			/* mark init state */
			image->set_init_phase();
			
			/* try to load this image */
			result = image->load(image_name);

			/* did the image load fail? */
			if (!result)
			{
				/* retrieve image error message */
				const char *image_err = image->error();
				const char *image_basename_str = image->basename();

				/* unload all images */
				image_unload_all(machine);

				fatalerror_exitcode(machine, MAMERR_DEVICE, "Device %s load (%s) failed: %s",
					image->image_config().name(),
					image_basename_str,
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

/*-------------------------------------------------
    image_postdevice_init - initialize devices for a specific
    running_machine
-------------------------------------------------*/

void image_postdevice_init(running_machine *machine)
{
	device_image_interface *image = NULL;

	/* make sure that any required devices have been allocated */
    for (bool gotone = machine->devicelist.first(image); gotone; gotone = image->next(image))
    {
			int result = image->finish_load();
			/* did the image load fail? */
			if (result)
			{
				/* retrieve image error message */
				const char *image_err = image->error();
				const char *image_basename_str = image->basename();

				/* unload all images */
				image_unload_all(machine);

				fatalerror_exitcode(machine, MAMERR_DEVICE, "Device %s load (%s) failed: %s",
					image->image_config().name(),
					image_basename_str,
					image_err);
			}
	}

	/* add a callback for when we shut down */
	add_exit_callback(machine, image_unload_all);
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
