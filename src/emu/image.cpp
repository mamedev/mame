// license:BSD-3-Clause
// copyright-holders:Nathan Woods, Miodrag Milanovic
/***************************************************************************

    image.cpp

    Core image functions and definitions.


***************************************************************************/

#include <ctype.h>
#include <regex>

#include "emu.h"
#include "emuopts.h"
#include "image.h"
#include "config.h"
#include "xmlfile.h"

//**************************************************************************
//  STATIC VARIABLES
//**************************************************************************

static std::regex s_potenial_softlist_regex("\\w+(\\:\\w+\\:\\w+)?");


//**************************************************************************
//  IMAGE MANAGER
//**************************************************************************

//-------------------------------------------------
//  image_manager - constructor
//-------------------------------------------------

image_manager::image_manager(running_machine &machine)
	: m_machine(machine)
{
	// make sure that any required devices have been allocated
	for (device_image_interface &image : image_interface_iterator(machine.root_device()))
	{
		// ignore things not user loadable
		if (!image.user_loadable())
			continue;

		// is an image specified for this image
		const char *image_name_ptr = machine.options().value(image.instance_name());
		if ((image_name_ptr != nullptr) && (image_name_ptr[0] != '\0'))
		{
			image_init_result result = image_init_result::FAIL;
			std::string image_name(image_name_ptr);

			// mark init state
			image.set_init_phase();

			// try as a softlist
			if (std::regex_match(image_name, s_potenial_softlist_regex))
				result = image.load_software(image_name);

			// failing that, try as an image
			if (result != image_init_result::PASS)
				result = image.load(image_name);

			// did the image load fail?
			if (result != image_init_result::PASS)
			{
				// retrieve image error message
				std::string image_err = std::string(image.error());

				// unload all images
				unload_all();

				fatalerror_exitcode(machine, EMU_ERR_DEVICE, "Device %s load (%s) failed: %s",
					image.device().name(),
					image_name.c_str(),
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

	for (device_image_interface &image : image_interface_iterator(machine().root_device()))
	{
		// unload this image
		image.unload();
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
				for (device_image_interface &image : image_interface_iterator(machine().root_device()))
				{
					if (!strcmp(dev_instance, image.instance_name())) {
						working_directory = xml_get_attribute_string(node, "directory", nullptr);
						if (working_directory != nullptr)
							image.set_working_directory(working_directory);
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
		for (device_image_interface &image : image_interface_iterator(machine().root_device()))
		{
			dev_instance = image.instance_name();

			node = xml_add_child(parentnode, "device", nullptr);
			if (node != nullptr)
			{
				xml_set_attribute(node, "instance", dev_instance);
				xml_set_attribute(node, "directory", image.working_directory().c_str());
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
	osd_file::error filerr = file.open(filename);
	if (filerr == osd_file::error::NONE)
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

		for (device_image_interface &image : image_interface_iterator(machine().root_device()))
		{
			const char *filename = image.filename();

			/* and set the option */
			std::string error;
			machine().options().set_value(image.instance_name(), filename ? filename : "", OPTION_PRIORITY_CMDLINE, error);

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
	for (device_image_interface &image : image_interface_iterator(machine().root_device()))
	{
		if (image.filename() == nullptr && image.must_be_loaded())
			mandatory.append("\"").append(image.instance_name()).append("\", ");
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
	for (device_image_interface &image : image_interface_iterator(machine().root_device()))
	{
		image_init_result result = image.finish_load();

		/* did the image load fail? */
		if (result != image_init_result::PASS)
		{
			/* retrieve image error message */
			std::string image_err = std::string(image.error());

			/* unload all images */
			unload_all();

			fatalerror_exitcode(machine(), EMU_ERR_DEVICE, "Device %s load failed: %s",
				image.device().name(),
				image_err.c_str());
		}
	}
	/* add a callback for when we shut down */
	machine().add_notifier(MACHINE_NOTIFY_EXIT, machine_notify_delegate(FUNC(image_manager::unload_all), this));
}
