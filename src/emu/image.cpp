// license:BSD-3-Clause
// copyright-holders:Nathan Woods, Miodrag Milanovic
/***************************************************************************

    image.cpp

    Core image functions and definitions.


***************************************************************************/

#include <ctype.h>

#include "emu.h"
#include "emuopts.h"
#include "image.h"
#include "config.h"
#include "xmlfile.h"
#include "softlist.h"


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

		// find the image option in image_options()
		auto iter = machine.options().image_options().find(image.instance_name());

		// GROSS HACK - if we began our journey with a single device configuration (e.g. - a single
		// cartridge system) but later added a device of that type, image.instance_name() will become
		// something different.  We're going to try to accomodate that specific scenario here
		//
		// Specific example: 'mame snes -cart1 sufami -cart2 poipoi' - the instance_name() starts out
		// as "cartridge" but at the end becomes "cartridge1"
		if (iter == machine.options().image_options().end()
			&& (image.instance_name().rbegin() != image.instance_name().rend())
			&& (*image.instance_name().rbegin() == '1'))
		{
			std::string alternate_instance_name = image.instance_name().substr(0, image.instance_name().size() - 1);
			iter = machine.options().image_options().find(alternate_instance_name);

			// If we found something, we need to write it back (so later checks work).  We also need to redo
			// the find; the act of erasing the old value breaks the iterator
			if (iter != machine.options().image_options().end())
			{
				std::string temp = std::move(iter->second);
				machine.options().image_options()[image.instance_name()] = std::move(temp);
				machine.options().image_options().erase(alternate_instance_name);
				iter = machine.options().image_options().find(image.instance_name());
			}
		}

		// is an image specified for this image?
		if (iter != machine.options().image_options().end() && !iter->second.empty())
		{
			// we do have a startup image specified - load it
			const std::string &startup_image(iter->second);
			image_init_result result = image_init_result::FAIL;

			// try as a softlist
			if (software_name_parse(startup_image))
				result = image.load_software(startup_image);

			// failing that, try as an image
			if (result != image_init_result::PASS)
				result = image.load(startup_image);

			// failing that, try creating it (if appropriate)
			if (result != image_init_result::PASS && image.support_command_line_image_creation())
				result = image.create(startup_image);

			// did the image load fail?
			if (result != image_init_result::PASS)
			{
				// retrieve image error message
				std::string image_err = std::string(image.error());

				// unload all images
				unload_all();

				fatalerror_exitcode(machine, EMU_ERR_DEVICE, "Device %s load (%s) failed: %s",
					image.device().name(),
					startup_image.c_str(),
					image_err.c_str());
			}
		}
	}

	machine.configuration().config_register("image_directories", config_load_delegate(&image_manager::config_load, this), config_save_delegate(&image_manager::config_save, this));
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

void image_manager::config_load(config_type cfg_type, util::xml::data_node const *parentnode)
{
	if ((cfg_type == config_type::GAME) && (parentnode != nullptr))
	{
		for (util::xml::data_node const *node = parentnode->get_child("device"); node; node = node->get_next_sibling("device"))
		{
			const char *const dev_instance = node->get_attribute_string("instance", nullptr);

			if ((dev_instance != nullptr) && (dev_instance[0] != '\0'))
			{
				for (device_image_interface &image : image_interface_iterator(machine().root_device()))
				{
					if (!strcmp(dev_instance, image.instance_name().c_str()))
					{
						const char *const working_directory = node->get_attribute_string("directory", nullptr);
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

void image_manager::config_save(config_type cfg_type, util::xml::data_node *parentnode)
{
	/* only care about game-specific data */
	if (cfg_type == config_type::GAME)
	{
		for (device_image_interface &image : image_interface_iterator(machine().root_device()))
		{
			const char *const dev_instance = image.instance_name().c_str();

			util::xml::data_node *const node = parentnode->add_child("device", nullptr);
			if (node != nullptr)
			{
				node->set_attribute("instance", dev_instance);
				node->set_attribute("directory", image.working_directory().c_str());
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
	for (device_image_interface &image : image_interface_iterator(machine().root_device()))
	{
		// only perform this activity for devices where is_reset_on_load() is false; for devices
		// where this is true, manipulation of this value is done in reset_and_load()
		if (!image.is_reset_on_load())
		{
			// we have to assemble the image option differently for software lists and for normal images
			std::string image_opt;
			if (image.exists())
			{
				if (image.loaded_through_softlist())
					image_opt = util::string_format("%s:%s:%s", image.software_list_name(), image.full_software_name(), image.brief_instance_name());
				else
					image_opt = image.filename();
			}

			// and set the option
			machine().options().image_options()[image.instance_name()] = std::move(image_opt);
		}
	}

	// write the config, if appropriate
	if (machine().options().write_config())
		write_config(machine().options(), nullptr, &machine().system());
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
	machine().add_notifier(MACHINE_NOTIFY_EXIT, machine_notify_delegate(&image_manager::unload_all, this));
}
