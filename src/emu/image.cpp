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
#include "formats/ioprocs.h"

/* ----------------------------------------------------------------------- */

static int image_fseek_thunk(void *file, INT64 offset, int whence)
{
	device_image_interface *image = (device_image_interface *) file;
	return image->fseek(offset, whence);
}

static size_t image_fread_thunk(void *file, void *buffer, size_t length)
{
	device_image_interface *image = (device_image_interface *) file;
	return image->fread(buffer, length);
}

static size_t image_fwrite_thunk(void *file, const void *buffer, size_t length)
{
	device_image_interface *image = (device_image_interface *) file;
	return image->fwrite(buffer, length);
}

static UINT64 image_fsize_thunk(void *file)
{
	device_image_interface *image = (device_image_interface *) file;
	return image->length();
}

/* ----------------------------------------------------------------------- */

struct io_procs image_ioprocs =
{
	nullptr,
	image_fseek_thunk,
	image_fread_thunk,
	image_fwrite_thunk,
	image_fsize_thunk
};

/***************************************************************************
    INITIALIZATION HELPERS
***************************************************************************/

/*-------------------------------------------------
    image_dirs_load - loads image device directory
    configuration items
-------------------------------------------------*/

static void image_dirs_load(running_machine &machine, config_type cfg_type, xml_data_node *parentnode)
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
				image_interface_iterator iter(machine.root_device());
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
    image_dirs_save - saves out image device
    directories to the configuration file
-------------------------------------------------*/

static void image_dirs_save(running_machine &machine, config_type cfg_type, xml_data_node *parentnode)
{
	xml_data_node *node;
	const char *dev_instance;

	/* only care about game-specific data */
	if (cfg_type == config_type::CONFIG_TYPE_GAME)
	{
		image_interface_iterator iter(machine.root_device());
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

static int write_config(emu_options &options, const char *filename, const game_driver *gamedrv)
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
    image_options_extract - extract device options
    out of core into the options
-------------------------------------------------*/

static void image_options_extract(running_machine &machine)
{
	/* only extract the device options if we've added them
	   no need to assert in case they are missing */
	{
		int index = 0;

		image_interface_iterator iter(machine.root_device());
		for (device_image_interface *image = iter.first(); image != nullptr; image = iter.next())
		{
			const char *filename = image->filename();

			/* and set the option */
			std::string error;
			machine.options().set_value(image->instance_name(), filename ? filename : "", OPTION_PRIORITY_CMDLINE, error);

			index++;
		}
	}

	/* write the config, if appropriate */
	if (machine.options().write_config())
		write_config(machine.options(), nullptr, &machine.system());
}

/*-------------------------------------------------
    image_unload_all - unload all images and
    extract options
-------------------------------------------------*/

void image_unload_all(running_machine &machine)
{
	// extract the options
	image_options_extract(machine);

	image_interface_iterator iter(machine.root_device());
	for (device_image_interface *image = iter.first(); image != nullptr; image = iter.next())
	{
		// unload this image
		image->unload();
	}
}
/*-------------------------------------------------
    image_device_init - initialize devices for a specific
    running_machine
-------------------------------------------------*/

void image_device_init(running_machine &machine)
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
				image_unload_all(machine);

				fatalerror_exitcode(machine, MAMERR_DEVICE, "Device %s load (%s) failed: %s",
					image->device().name(),
					image_basename.c_str(),
					image_err.c_str());
			}
		}
	}
}

/*-------------------------------------------------
 image_mandatory_scan - search for devices which
 need an image to be loaded
 -------------------------------------------------*/

std::string &image_mandatory_scan(running_machine &machine, std::string &mandatory)
{
	mandatory.clear();
	// make sure that any required image has a mounted file
	image_interface_iterator iter(machine.root_device());
	for (device_image_interface *image = iter.first(); image != nullptr; image = iter.next())
	{
		if (image->filename() == nullptr && image->must_be_loaded())
			mandatory.append("\"").append(image->instance_name()).append("\", ");
	}
	return mandatory;
}

/*-------------------------------------------------
    image_postdevice_init - initialize devices for a specific
    running_machine
-------------------------------------------------*/

void image_postdevice_init(running_machine &machine)
{
	/* make sure that any required devices have been allocated */
	image_interface_iterator iter(machine.root_device());
	for (device_image_interface *image = iter.first(); image != nullptr; image = iter.next())
	{
			int result = image->finish_load();
			/* did the image load fail? */
			if (result)
			{
				/* retrieve image error message */
				std::string image_err = std::string(image->error());

				/* unload all images */
				image_unload_all(machine);

				fatalerror_exitcode(machine, MAMERR_DEVICE, "Device %s load failed: %s",
					image->device().name(),
					image_err.c_str());
			}
	}

	/* add a callback for when we shut down */
	machine.add_notifier(MACHINE_NOTIFY_EXIT, machine_notify_delegate(FUNC(image_unload_all), &machine));
}
/***************************************************************************
    INITIALIZATION
***************************************************************************/

/*-------------------------------------------------
    image_init - start up the image system
-------------------------------------------------*/

void image_init(running_machine &machine)
{
	image_device_init(machine);
	machine.configuration().config_register("image_directories", config_saveload_delegate(FUNC(image_dirs_load), &machine), config_saveload_delegate(FUNC(image_dirs_save), &machine));
}


/****************************************************************************
  Battery functions

  These functions provide transparent access to battery-backed RAM on an
  image; typically for cartridges.
****************************************************************************/

/*-------------------------------------------------
    image_battery_load_by_name - retrieves the battery
    backed RAM for an image. A filename may be supplied
    to the function.

    The function comes in two flavors, depending on
    what should happen when no battery is available:
    we could fill the memory with a given value, or
    pass a default battery (for a pre-initialized
    battery from factory)
-------------------------------------------------*/

void image_battery_load_by_name(emu_options &options, const char *filename, void *buffer, int length, int fill)
{
	file_error filerr;
	int bytes_read = 0;

	assert_always(buffer && (length > 0), "Must specify sensical buffer/length");

	/* try to open the battery file and read it in, if possible */
	emu_file file(options.nvram_directory(), OPEN_FLAG_READ);
	filerr = file.open(filename);
	if (filerr == FILERR_NONE)
		bytes_read = file.read(buffer, length);

	/* fill remaining bytes (if necessary) */
	memset(((char *) buffer) + bytes_read, fill, length - bytes_read);
}

void image_battery_load_by_name(emu_options &options, const char *filename, void *buffer, int length, void *def_buffer)
{
	file_error filerr;
	int bytes_read = 0;

	assert_always(buffer && (length > 0), "Must specify sensical buffer/length");

	/* try to open the battery file and read it in, if possible */
	emu_file file(options.nvram_directory(), OPEN_FLAG_READ);
	filerr = file.open(filename);
	if (filerr == FILERR_NONE)
		bytes_read = file.read(buffer, length);

	/* if no file was present, copy the default battery */
	if (bytes_read == 0 && def_buffer)
		memcpy((char *) buffer, (char *) def_buffer, length);
}

/*-------------------------------------------------
    image_battery_save_by_name - stores the battery
    backed RAM for an image. A filename may be supplied
    to the function.
-------------------------------------------------*/
void image_battery_save_by_name(emu_options &options, const char *filename, const void *buffer, int length)
{
	assert_always(buffer && (length > 0), "Must specify sensical buffer/length");

	/* try to open the battery file and write it out, if possible */
	emu_file file(options.nvram_directory(), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
	file_error filerr = file.open(filename);
	if (filerr == FILERR_NONE)
		file.write(buffer, length);
}
