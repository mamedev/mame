/***************************************************************************

    image.c

    Core image functions and definitions.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

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
	NULL,
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

static void image_dirs_load(running_machine &machine, int config_type, xml_data_node *parentnode)
{
	xml_data_node *node;
	const char *dev_instance;
	const char *working_directory;

	if ((config_type == CONFIG_TYPE_GAME) && (parentnode != NULL))
	{
		for (node = xml_get_sibling(parentnode->child, "device"); node; node = xml_get_sibling(node->next, "device"))
		{
			dev_instance = xml_get_attribute_string(node, "instance", NULL);

			if ((dev_instance != NULL) && (dev_instance[0] != '\0'))
			{
				image_interface_iterator iter(machine.root_device());
				for (device_image_interface *image = iter.first(); image != NULL; image = iter.next())
				{
					if (!strcmp(dev_instance, image->instance_name())) {
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

static void image_dirs_save(running_machine &machine, int config_type, xml_data_node *parentnode)
{
	xml_data_node *node;
	const char *dev_instance;

	/* only care about game-specific data */
	if (config_type == CONFIG_TYPE_GAME)
	{
		image_interface_iterator iter(machine.root_device());
		for (device_image_interface *image = iter.first(); image != NULL; image = iter.next())
		{
			dev_instance = image->instance_name();

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
    write_config - emit current option statuses as
    INI files
-------------------------------------------------*/

static int write_config(emu_options &options, const char *filename, const game_driver *gamedrv)
{
	char buffer[128];
	int retval = 1;

	if (gamedrv != NULL)
	{
		sprintf(buffer, "%s.ini", gamedrv->name);
		filename = buffer;
	}

	emu_file file(options.ini_path(), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE);
	file_error filerr = file.open(filename);
	if (filerr == FILERR_NONE)
	{
		astring inistring;
		options.output_ini(inistring);
		file.puts(inistring);
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
		for (device_image_interface *image = iter.first(); image != NULL; image = iter.next())
		{
			const char *filename = image->filename();

			/* and set the option */
			astring error;
			machine.options().set_value(image->instance_name(), filename ? filename : "", OPTION_PRIORITY_CMDLINE, error);

			index++;
		}
	}

	/* write the config, if appropriate */
	if (machine.options().write_config())
		write_config(machine.options(), NULL, &machine.system());
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
	for (device_image_interface *image = iter.first(); image != NULL; image = iter.next())
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
	for (device_image_interface *image = iter.first(); image != NULL; image = iter.next())
	{
		/* is an image specified for this image */
		image_name = machine.options().device_option(*image);

		if ((image_name != NULL) && (image_name[0] != '\0'))
		{
			/* mark init state */
			image->set_init_phase();

			/* try to load this image */
			bool result = image->load(image_name);

			/* did the image load fail? */
			if (result)
			{
				/* retrieve image error message */
				astring image_err = astring(image->error());
				astring image_basename(image_name);

				/* unload all images */
				image_unload_all(machine);

				fatalerror_exitcode(machine, MAMERR_DEVICE, "Device %s load (%s) failed: %s",
					image->device().name(),
					image_basename.cstr(),
					image_err.cstr());
			}
		}
	}

	for (device_image_interface *image = iter.first(); image != NULL; image = iter.next())
	{
		/* is an image specified for this image */
		image_name = image->filename();

		if (!((image_name != NULL) && (image_name[0] != '\0')))
		{
			/* no image... must this device be loaded? */
			if (image->must_be_loaded())
			{
				fatalerror_exitcode(machine, MAMERR_DEVICE, "Driver requires that device \"%s\" must have an image to load", image->instance_name());
			}
		}
	}
}

/*-------------------------------------------------
    image_postdevice_init - initialize devices for a specific
    running_machine
-------------------------------------------------*/

void image_postdevice_init(running_machine &machine)
{
	/* make sure that any required devices have been allocated */
	image_interface_iterator iter(machine.root_device());
	for (device_image_interface *image = iter.first(); image != NULL; image = iter.next())
    {
			int result = image->finish_load();
			/* did the image load fail? */
			if (result)
			{
				/* retrieve image error message */
				astring image_err = astring(image->error());

				/* unload all images */
				image_unload_all(machine);

				fatalerror_exitcode(machine, MAMERR_DEVICE, "Device %s load failed: %s",
					image->device().name(),
					image_err.cstr());
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
	config_register(machine, "image_directories", config_saveload_delegate(FUNC(image_dirs_load), &machine), config_saveload_delegate(FUNC(image_dirs_save), &machine));
}


/****************************************************************************
  Battery functions

  These functions provide transparent access to battery-backed RAM on an
  image; typically for cartridges.
****************************************************************************/

static char *stripspace(const char *src)
{
	static char buff[512];
	if( src )
	{
		char *dst;
		while( *src && isspace(*src) )
			src++;
		strcpy(buff, src);
		dst = buff + strlen(buff);
		while( dst >= buff && isspace(*--dst) )
			*dst = '\0';
		return buff;
	}
	return NULL;
}

//============================================================
//  strip_extension
//============================================================

static char *strip_extension(const char *filename)
{
	char *newname;
	char *c;

	// NULL begets NULL
	if (!filename)
		return NULL;

	// allocate space for it
	newname = (char *) malloc(strlen(filename) + 1);
	if (!newname)
		return NULL;

	// copy in the name
	strcpy(newname, filename);

	// search backward for a period, failing if we hit a slash or a colon
	for (c = newname + strlen(newname) - 1; c >= newname; c--)
	{
		// if we hit a period, NULL terminate and break
		if (*c == '.')
		{
			*c = 0;
			break;
		}

		// if we hit a slash or colon just stop
		if (*c == '\\' || *c == '/' || *c == ':')
			break;
	}

	return newname;
}

/*-------------------------------------------------
    image_info_astring - populate an allocated
    string with the image info text
-------------------------------------------------*/

astring &image_info_astring(running_machine &machine, astring &string)
{
	string.printf("%s\n\n", machine.system().description);

#if 0
	if (mess_ram_size > 0)
	{
		char buf2[RAM_STRING_BUFLEN];
		string.catprintf("RAM: %s\n\n", ram_string(buf2, mess_ram_size));
	}
#endif

	image_interface_iterator iter(machine.root_device());
	for (device_image_interface *image = iter.first(); image != NULL; image = iter.next())
	{
		const char *name = image->filename();
		if (name != NULL)
		{
			const char *base_filename;
			const char *info;
			char *base_filename_noextension;

			base_filename = image->basename();
			base_filename_noextension = strip_extension(base_filename);

			/* display device type and filename */
			string.catprintf("%s: %s\n", image->device().name(), base_filename);

			/* display long filename, if present and doesn't correspond to name */
			info = image->longname();
			if (info && (!base_filename_noextension || mame_stricmp(info, base_filename_noextension)))
				string.catprintf("%s\n", info);

			/* display manufacturer, if available */
			info = image->manufacturer();
			if (info != NULL)
			{
				string.catprintf("%s", info);
				info = stripspace(image->year());
				if (info && *info)
					string.catprintf(", %s", info);
				string.catprintf("\n");
			}

			/* display supported information, if available */
			switch(image->supported()) {
				case SOFTWARE_SUPPORTED_NO : string.catprintf("Not supported\n"); break;
				case SOFTWARE_SUPPORTED_PARTIAL : string.catprintf("Partially supported\n"); break;
				default : break;
			}

			if (base_filename_noextension != NULL)
				free(base_filename_noextension);
		}
		else
		{
			string.catprintf("%s: ---\n", image->device().name());
		}
	}
	return string;
}


/*-------------------------------------------------
    image_battery_load_by_name - retrieves the battery
    backed RAM for an image. A filename may be supplied
    to the function.
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
