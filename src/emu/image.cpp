// license:BSD-3-Clause
// copyright-holders:Nathan Woods, Miodrag Milanovic
/***************************************************************************

    image.c

    Core image functions and definitions.


***************************************************************************/
#include <ctype.h>

#include "emu.h"
#include "emuopts.h"
#include "image.h"
#include "config.h"
#include "xmlfile.h"

//**************************************************************************
//  IMAGE MANAGER
//**************************************************************************

//-------------------------------------------------
//  image_manager - constructor
//-------------------------------------------------

image_manager::image_manager(running_machine &machine)
	: m_machine(machine)
{
	const char *image_name;

	/* make sure that any required devices have been allocated */
	image_interface_iterator iter(machine.root_device());
	for (device_image_interface *image = iter.first(); image != nullptr; image = iter.next())
	{
		/* is an image specified for this image */
		image_name = machine.options().value(image->instance_name());

		if ((image_name != nullptr) && (image_name[0] != '\0'))
		{
			/* mark init state */
			image->set_init_phase();

			/* try to load this image */
			bool result = image->load(image_name);

			/* did the image load fail? */
			if (result)
			{
				/* retrieve image error message */
				std::string image_err = std::string(image->error());
				std::string image_basename(image_name);

				/* unload all images */
				unload_all();

				fatalerror_exitcode(machine, MAMERR_DEVICE, "Device %s load (%s) failed: %s",
					image->device().name(),
					image_basename.c_str(),
					image_err.c_str());
			}
		}
	}

	machine.configuration().config_register("image_directories", config_saveload_delegate(FUNC(image_manager::config_load), this), config_saveload_delegate(FUNC(image_manager::config_save), this));
}

//-------------------------------------------------
//  unload_all - unload all images and
//  extract options
//-------------------------------------------------
void image_manager::unload_all()
{
	// extract the options
	options_extract();

	image_interface_iterator iter(machine().root_device());
	for (device_image_interface *image = iter.first(); image != nullptr; image = iter.next())
	{
		// unload this image
		image->unload();
	}
}

void image_manager::config_load(config_type cfg_type, xml_data_node *parentnode)
{
	xml_data_node *node;
	const char *dev_instance;
	const char *working_directory;

	if ((cfg_type == config_type::CONFIG_TYPE_GAME) && (parentnode != nullptr))
	{
		for (node = xml_get_sibling(parentnode->child, "device"); node; node = xml_get_sibling(node->next, "device"))
		{
			dev_instance = xml_get_attribute_string(node, "instance", nullptr);

			if ((dev_instance != nullptr) && (dev_instance[0] != '\0'))
			{
				image_interface_iterator iter(machine().root_device());
				for (device_image_interface *image = iter.first(); image != nullptr; image = iter.next())
				{
					if (!strcmp(dev_instance, image->instance_name())) {
						working_directory = xml_get_attribute_string(node, "directory", nullptr);
						if (working_directory != nullptr)
							image->set_working_directory(working_directory);
					}
				}
			}
		}
	}
}

/*-------------------------------------------------
    config_save - saves out image device
    directories to the configuration file
-------------------------------------------------*/

void image_manager::config_save(config_type cfg_type, xml_data_node *parentnode)
{
	xml_data_node *node;
	const char *dev_instance;

	/* only care about game-specific data */
	if (cfg_type == config_type::CONFIG_TYPE_GAME)
	{
		image_interface_iterator iter(machine().root_device());
		for (device_image_interface *image = iter.first(); image != nullptr; image = iter.next())
		{
			dev_instance = image->instance_name();

			node = xml_add_child(parentnode, "device", nullptr);
			if (node != nullptr)
			{
				xml_set_attribute(node, "instance", dev_instance);
				xml_set_attribute(node, "directory", image->working_directory());
			}
		}
	}
}

/*-------------------------------------------------
    write_config - emit current option statuses as
    INI files
-------------------------------------------------*/

int image_manager::write_config(emu_options &options, const char *filename, const game_driver *gamedrv)
{
	char buffer[128];
	int retval = 1;

	if (gamedrv != nullptr)
	{
		sprintf(buffer, "%s.ini", gamedrv->name);
		filename = buffer;
	}

	emu_file file(options.ini_path(), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE);
	file_error filerr = file.open(filename);
	if (filerr == FILERR_NONE)
	{
		std::string inistring = options.output_ini();
		file.puts(inistring.c_str());
		retval = 0;
	}
	return retval;
}

/*-------------------------------------------------
    options_extract - extract device options
    out of core into the options
-------------------------------------------------*/

void image_manager::options_extract()
{
	/* only extract the device options if we've added them
	   no need to assert in case they are missing */
	{
		int index = 0;

		image_interface_iterator iter(machine().root_device());
		for (device_image_interface *image = iter.first(); image != nullptr; image = iter.next())
		{
			const char *filename = image->filename();

			/* and set the option */
			std::string error;
			machine().options().set_value(image->instance_name(), filename ? filename : "", OPTION_PRIORITY_CMDLINE, error);

			index++;
		}
	}

	/* write the config, if appropriate */
	if (machine().options().write_config())
		write_config(machine().options(), nullptr, &machine().system());
}


/*-------------------------------------------------
 image_mandatory_scan - search for devices which
 need an image to be loaded
 -------------------------------------------------*/

std::string &image_manager::mandatory_scan(std::string &mandatory)
{
	mandatory.clear();
	// make sure that any required image has a mounted file
	image_interface_iterator iter(machine().root_device());
	for (device_image_interface *image = iter.first(); image != nullptr; image = iter.next())
	{
		if (image->filename() == nullptr && image->must_be_loaded())
			mandatory.append("\"").append(image->instance_name()).append("\", ");
	}
	return mandatory;
}

/*-------------------------------------------------
    postdevice_init - initialize devices for a specific
    running_machine
-------------------------------------------------*/

void image_manager::postdevice_init()
{
	/* make sure that any required devices have been allocated */
	image_interface_iterator iter(machine().root_device());
	for (device_image_interface *image = iter.first(); image != nullptr; image = iter.next())
	{
			int result = image->finish_load();
			/* did the image load fail? */
			if (result)
			{
				/* retrieve image error message */
				std::string image_err = std::string(image->error());

				/* unload all images */
				unload_all();

				fatalerror_exitcode(machine(), MAMERR_DEVICE, "Device %s load failed: %s",
					image->device().name(),
					image_err.c_str());
			}
	}
	/* add a callback for when we shut down */
	machine().add_notifier(MACHINE_NOTIFY_EXIT, machine_notify_delegate(FUNC(image_manager::unload_all), this));
}
