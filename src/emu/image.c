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
				for (bool gotone = machine->m_devicelist.first(image); gotone; gotone = image->next(image))
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
		for (bool gotone = machine->m_devicelist.first(image); gotone; gotone = image->next(image))
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
    write_config - emit current option statuses as
    INI files
-------------------------------------------------*/

static int write_config(const char *filename, const game_driver *gamedrv)
{
	file_error filerr;
	mame_file *f;
	char buffer[128];
	int retval = 1;

	if (gamedrv != NULL)
	{
		sprintf(buffer, "%s.ini", gamedrv->name);
		filename = buffer;
	}

	filerr = mame_fopen(SEARCHPATH_INI, buffer, OPEN_FLAG_WRITE | OPEN_FLAG_CREATE, &f);
	if (filerr != FILERR_NONE)
		goto done;

	options_output_ini_file(mame_options(), mame_core_file(f));
	retval = 0;

done:
	if (f != NULL)
		mame_fclose(f);
	return retval;
}

/*-------------------------------------------------
    image_options_extract - extract device options
    out of core into the options
-------------------------------------------------*/

static void image_options_extract(running_machine *machine)
{
	/* only extract the device options if we've added them */
	if (options_get_bool(machine->options(), OPTION_ADDED_DEVICE_OPTIONS)) {
		int index = 0;
		device_image_interface *image = NULL;

		for (bool gotone = machine->m_devicelist.first(image); gotone; gotone = image->next(image))
		{
			const char *filename = image->filename();

			/* and set the option */
			options_set_string(machine->options(), image->image_config().instance_name() , filename ? filename : "", OPTION_PRIORITY_CMDLINE);

			index++;
		}
	}

	/* write the config, if appropriate */
	if (options_get_bool(machine->options(), OPTION_WRITECONFIG))
		write_config(NULL, machine->gamedrv);
}

/*-------------------------------------------------
    image_unload_all - unload all images and
    extract options
-------------------------------------------------*/

void image_unload_all(running_machine &machine)
{
    device_image_interface *image = NULL;

	// extract the options
	image_options_extract(&machine);

	for (bool gotone = machine.m_devicelist.first(image); gotone; gotone = image->next(image))
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
    for (bool gotone = machine->m_devicelist.first(image); gotone; gotone = image->next(image))
	{
		/* is an image specified for this image */
		image_name = image_get_device_option(image);

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
				const char *image_basename_str = image->basename();
				/* unload all images */
				image_unload_all(*machine);

				fatalerror_exitcode(machine, MAMERR_DEVICE, "Device %s load (%s) failed: %s",
					image->image_config().devconfig().name(),
					image_basename_str,
					image_err.cstr());
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

		image->call_get_devices();
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
    for (bool gotone = machine->m_devicelist.first(image); gotone; gotone = image->next(image))
    {
			int result = image->finish_load();
			/* did the image load fail? */
			if (result)
			{
				/* retrieve image error message */
				astring image_err = astring(image->error());
				const char *image_basename_str = image->basename();

				/* unload all images */
				image_unload_all(*machine);

				fatalerror_exitcode(machine, MAMERR_DEVICE, "Device %s load (%s) failed: %s",
					image->image_config().devconfig().name(),
					image_basename_str,
					image_err.cstr());
			}
	}

	/* add a callback for when we shut down */
	machine->add_notifier(MACHINE_NOTIFY_EXIT, image_unload_all);
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


/****************************************************************************
  Battery functions

  These functions provide transparent access to battery-backed RAM on an
  image; typically for cartridges.
****************************************************************************/

/*-------------------------------------------------
    open_battery_file_by_name - opens the battery backed
    NVRAM file for an image
-------------------------------------------------*/

static file_error open_battery_file_by_name(const char *filename, UINT32 openflags, mame_file **file)
{
    file_error filerr;
    filerr = mame_fopen(SEARCHPATH_NVRAM, filename, openflags, file);
    return filerr;
}

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

astring *image_info_astring(running_machine *machine, astring *string)
{
	device_image_interface *image = NULL;

	astring_printf(string, "%s\n\n", machine->gamedrv->description);

#if 0
	if (mess_ram_size > 0)
	{
		char buf2[RAM_STRING_BUFLEN];
		astring_catprintf(string, "RAM: %s\n\n", ram_string(buf2, mess_ram_size));
	}
#endif

	for (bool gotone = machine->m_devicelist.first(image); gotone; gotone = image->next(image))
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
			astring_catprintf(string, "%s: %s\n", image->image_config().devconfig().name(), base_filename);

			/* display long filename, if present and doesn't correspond to name */
			info = image->longname();
			if (info && (!base_filename_noextension || mame_stricmp(info, base_filename_noextension)))
				astring_catprintf(string, "%s\n", info);

			/* display manufacturer, if available */
			info = image->manufacturer();
			if (info != NULL)
			{
				astring_catprintf(string, "%s", info);
				info = stripspace(image->year());
				if (info && *info)
					astring_catprintf(string, ", %s", info);
				astring_catprintf(string,"\n");
			}

			/* display playable information, if available */
			info = image->playable();
			if (info != NULL)
				astring_catprintf(string, "%s\n", info);

			if (base_filename_noextension != NULL)
				free(base_filename_noextension);
		}
		else
		{
			astring_catprintf(string, "%s: ---\n", image->image_config().devconfig().name());
		}
	}
	return string;
}


/*-------------------------------------------------
    image_battery_load_by_name - retrieves the battery
    backed RAM for an image. A filename may be supplied
    to the function.
-------------------------------------------------*/

void image_battery_load_by_name(const char *filename, void *buffer, int length, int fill)
{
    file_error filerr;
    mame_file *file;
    int bytes_read = 0;

    assert_always(buffer && (length > 0), "Must specify sensical buffer/length");

    /* try to open the battery file and read it in, if possible */
    filerr = open_battery_file_by_name(filename, OPEN_FLAG_READ, &file);
    if (filerr == FILERR_NONE)
    {
        bytes_read = mame_fread(file, buffer, length);
        mame_fclose(file);
    }

    /* fill remaining bytes (if necessary) */
    memset(((char *) buffer) + bytes_read, fill, length - bytes_read);
}

/*-------------------------------------------------
    image_battery_save_by_name - stores the battery
    backed RAM for an image. A filename may be supplied
    to the function.
-------------------------------------------------*/
void image_battery_save_by_name(const char *filename, const void *buffer, int length)
{
    file_error filerr;
    mame_file *file;

    assert_always(buffer && (length > 0), "Must specify sensical buffer/length");

    /* try to open the battery file and write it out, if possible */
    filerr = open_battery_file_by_name(filename, OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS, &file);
    if (filerr == FILERR_NONE)
    {
        mame_fwrite(file, buffer, length);
        mame_fclose(file);
    }
}

/*-------------------------------------------------
    image_from_absolute_index - retreives index number
    of image in device list
-------------------------------------------------*/
device_image_interface *image_from_absolute_index(running_machine *machine, int absolute_index)
{
	device_image_interface *image = NULL;
	int cnt = 0;
	/* make sure that any required devices have been allocated */
    for (bool gotone = machine->m_devicelist.first(image); gotone; gotone = image->next(image))
	{
		if (cnt==absolute_index) return image;
		cnt++;
	}
	return NULL;
}

/*-------------------------------------------------
    image_add_device_with_subdevices - adds
    device with parameters sent, and all subdevices
    from it's machine config devices list
-------------------------------------------------*/
void image_add_device_with_subdevices(device_t *owner, device_type type, const char *tag, UINT32 clock)
{
	astring tempstring;
	device_list *device_list = &owner->machine->m_devicelist;
	machine_config *config = (machine_config *)owner->machine->config;

	device_config *devconfig = type(*config, owner->subtag(tempstring,tag), &owner->baseconfig(), clock);
	device_t *device = device_list->append(devconfig->tag(), devconfig->alloc_device(*owner->machine));

	machine_config_constructor machconfig = device->machine_config_additions();
	if (machconfig != NULL)
    {
    	(*machconfig)(*config, devconfig);
        for (const device_config *config_dev = config->m_devicelist.first(); config_dev != NULL; config_dev = config_dev->next())
        {
			if (config_dev->owner()==devconfig) {
				device_list->append(config_dev->tag(), config_dev->alloc_device(*owner->machine));
			}
        }
    }
	config->m_devicelist.append(devconfig->tag(), devconfig);
}

