/***************************************************************************

    ui/imginfo.c

    Image info screen

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#include "emu.h"
#include "ui/menu.h"
#include "ui/imginfo.h"


/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

//-------------------------------------------------
//  ctor
//-------------------------------------------------

ui_menu_image_info::ui_menu_image_info(running_machine &machine, render_container *container) : ui_menu(machine, container)
{
}


//-------------------------------------------------
//  dtor
//-------------------------------------------------

ui_menu_image_info::~ui_menu_image_info()
{
}


//-------------------------------------------------
//  populate
//-------------------------------------------------

void ui_menu_image_info::populate()
{
	astring tempstring;
	image_info_astring(machine(), tempstring);
	item_append(tempstring, NULL, MENU_FLAG_MULTILINE, NULL);
}


//-------------------------------------------------
//  handle
//-------------------------------------------------

void ui_menu_image_info::handle()
{
	// process the menu
	process(0);
}


//-------------------------------------------------
//  stripspace
//-------------------------------------------------

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


//-------------------------------------------------
//  strip_extension
//-------------------------------------------------

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


//-------------------------------------------------
//  image_info_astring - populate an allocated
//  string with the image info text
//-------------------------------------------------

void ui_menu_image_info::image_info_astring(running_machine &machine, astring &string)
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

			// display device type and filename
			string.catprintf("%s: %s\n", image->device().name(), base_filename);

			// display long filename, if present and doesn't correspond to name
			info = image->longname();
			if (info && (!base_filename_noextension || core_stricmp(info, base_filename_noextension)))
				string.catprintf("%s\n", info);

			// display manufacturer, if available
			info = image->manufacturer();
			if (info != NULL)
			{
				string.catprintf("%s", info);
				info = stripspace(image->year());
				if (info && *info)
					string.catprintf(", %s", info);
				string.catprintf("\n");
			}

			// display supported information, if available
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
}
