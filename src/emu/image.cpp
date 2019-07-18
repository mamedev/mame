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
		const std::string &startup_image(machine.options().image_option(image.instance_name()).value());

		// is an image specified for this image?
		if (!startup_image.empty())
		{
			// we do have a startup image specified - load it
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
				std::string startup_image_name = startup_image;

				// unload the bad image
				image.unload();

				// make sure it is removed from the ini file too
				machine.options().image_option(image.instance_name()).specify("");
				if (machine.options().write_config())
					write_config(machine.options(), nullptr, &machine.system());

				fatalerror_exitcode(machine, EMU_ERR_DEVICE, "Device %s load (-%s %s) failed: %s",
					image.device().name(),
					image.instance_name().c_str(),
					startup_image_name.c_str(),
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
		// There are two scenarios where we want to extract the option:
		//
		//  1.  When for the device, is_reset_on_load() is false (mounting devices for which is reset_and_load()
		//      is true forces a reset, hence the name)
		//
		//  2.  When is_reset_on_load(), and this results in a device being unmounted (unmounting is_reset_and_load()
		//      doesn't force an unmount).
		//
		//  Note that:
		//	1.  As a part of scenario #2 above , we cannot extract the option when the image in question is a part of an
		//		active reset_on_load; hence the check for is_reset_and_loading() (see issue #2414)
		//
		//	2.	We have to account for the following two scenarios:
		//		- The image device was unmounted, in which case the option will no longer exist
		//		- The image device is still there, but with a different name (e.g. - floppydisk vs floppydisk1), hence
		//		  the need to use the canonical option name (which in the above example, will always be 'floppydisk1')

		// The very first step in this is to find the option; if it is not there, this is all irrelevant
		auto image_opt = machine().options().find_image_option_canonical(image.device().tag());
		if (image_opt)
		{
			// We've found the image option; check for the two scenarios outlined above
			if (!image.is_reset_on_load()
				|| (!image.exists() && !image.is_reset_and_loading() && image_opt->value().empty()))
			{
				// we have to assemble the image option differently for software lists and for normal images
				std::string new_image_opt_value;
				if (image.exists())
				{
					if (!image.loaded_through_softlist())
						new_image_opt_value = image.filename();
					else if (image.part_entry() && !image.part_entry()->name().empty())
						new_image_opt_value = util::string_format("%s:%s:%s", image.software_list_name(), image.full_software_name(), image.part_entry()->name());
					else
						new_image_opt_value = util::string_format("%s:%s", image.software_list_name(), image.full_software_name());
				}

				// and set the option; note that we have to account for the following two scenarios:
				//  - The image device was unmounted, in which case the option will no longer exist
				//	- The image device is still there, but with a different name (e.g. - floppydisk vs floppydisk1), hence
				//    the need to use the canonical option name (which in the above example, will always be 'floppydisk1')
				image_opt->specify(std::move(new_image_opt_value));
			}
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
