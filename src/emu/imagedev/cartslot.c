/**********************************************************************

    cartslot.c

    Cartridge device

**********************************************************************/

#include <ctype.h>
#include "emu.h"
#include "cartslot.h"

/***************************************************************************
    CONSTANTS
***************************************************************************/

enum _process_mode
{
	PROCESS_CLEAR,
	PROCESS_LOAD
};
typedef enum _process_mode process_mode;


/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

INLINE const cartslot_config *get_config(const device_t *device)
{
	assert(device != NULL);
	assert(device->type() == CARTSLOT);
	return (const cartslot_config *) downcast<const legacy_device_base *>(device)->inline_config();
}

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    load_cartridge
-------------------------------------------------*/

static int load_cartridge(device_image_interface *image, const rom_entry *romrgn, const rom_entry *roment, process_mode mode)
{
	const char *region;
	const char *type;
	UINT32 flags;
	offs_t offset, length, read_length, pos = 0, len;
	UINT8 *ptr;
	UINT8 clear_val;
	int datawidth, littleendian, i, j;
	device_t *cpu;

	astring regiontag;
	image->device().siblingtag(regiontag, ROMREGION_GETTAG(romrgn));
	region = regiontag.cstr();
	offset = ROM_GETOFFSET(roment);
	length = ROM_GETLENGTH(roment);
	flags = ROM_GETFLAGS(roment);
	ptr = ((UINT8 *) image->device().machine().region(region)->base()) + offset;

	if (mode == PROCESS_LOAD)
	{
		if (image->software_entry() == NULL)
		{
			/* must this be full size */
			if (flags & ROM_FULLSIZE)
			{
				if (image->length() != length)
					return IMAGE_INIT_FAIL;
			}

			/* read the ROM */
			pos = read_length = image->fread(ptr, length);

			/* reset the ROM to the initial point. */
			/* eventually, we could add a flag to allow the ROM to continue instead of restarting whenever a new cart region is present */
			image->fseek(0, SEEK_SET);
		}
		else
		{
			/* must this be full size */
			if (flags & ROM_FULLSIZE)
			{
				if (image->get_software_region_length("rom") != length)
					return IMAGE_INIT_FAIL;
			}

			/* read the ROM */
			pos = read_length = image->get_software_region_length("rom");
			memcpy(ptr, image->get_software_region("rom"), read_length);
		}

		/* do we need to mirror the ROM? */
		if (flags & ROM_MIRROR)
		{
			while(pos < length)
			{
				len = MIN(read_length, length - pos);
				memcpy(ptr + pos, ptr, len);
				pos += len;
			}
		}

		/* postprocess this region */
		type = regiontag.cstr();
		littleendian = ROMREGION_ISLITTLEENDIAN(romrgn);
		datawidth = ROMREGION_GETWIDTH(romrgn) / 8;

		/* if the region is inverted, do that now */
		device_memory_interface *memory;
		cpu = image->device().machine().device(type);
		if (cpu!=NULL && cpu->interface(memory))
		{
			datawidth = cpu->memory().space_config(AS_PROGRAM)->m_databus_width / 8;
			littleendian = (cpu->memory().space_config()->m_endianness == ENDIANNESS_LITTLE);
		}

		/* swap the endianness if we need to */
#ifdef LSB_FIRST
		if (datawidth > 1 && !littleendian)
#else
		if (datawidth > 1 && littleendian)
#endif
		{
			for (i = 0; i < length; i += datawidth)
			{
				UINT8 temp[8];
				memcpy(temp, &ptr[i], datawidth);
				for (j = datawidth - 1; j >= 0; j--)
					ptr[i + j] = temp[datawidth - 1 - j];
			}
		}
	}

	/* clear out anything that remains */
	if (!(flags & ROM_NOCLEAR))
	{
		clear_val = (flags & ROM_FILL_FF) ? 0xFF : 0x00;
		memset(ptr + pos, clear_val, length - pos);
	}
	return IMAGE_INIT_PASS;
}



/*-------------------------------------------------
    process_cartridge
-------------------------------------------------*/

static int process_cartridge(device_image_interface *image, process_mode mode)
{
	const rom_source *source;
	const rom_entry *romrgn, *roment;
	int result = 0;

	for (source = rom_first_source(image->device().machine().config()); source != NULL; source = rom_next_source(*source))
	{
		for (romrgn = rom_first_region(*source); romrgn != NULL; romrgn = rom_next_region(romrgn))
		{
			roment = romrgn + 1;
			while(!ROMENTRY_ISREGIONEND(roment))
			{
				if (ROMENTRY_GETTYPE(roment) == ROMENTRYTYPE_CARTRIDGE)
				{
					astring regiontag;
					image->device().siblingtag(regiontag, roment->_hashdata);

					if (strcmp(regiontag.cstr(),image->device().tag())==0)
					{
						result |= load_cartridge(image, romrgn, roment, mode);

						/* if loading failed in any cart region, stop loading */
						if (result)
							return result;
					}
				}
				roment++;
			}
		}
	}

	return IMAGE_INIT_PASS;
}

/*-------------------------------------------------
    DEVICE_START( cartslot )
-------------------------------------------------*/

static DEVICE_START( cartslot )
{
	const cartslot_config *config = get_config(device);

	/* if this cartridge has a custom DEVICE_START, use it */
	if (config->device_start != NULL)
	{
		(*config->device_start)(device);
	}
}


/*-------------------------------------------------
    DEVICE_IMAGE_LOAD( cartslot )
-------------------------------------------------*/

static DEVICE_IMAGE_LOAD( cartslot )
{
	device_t *device = &image.device();
	const cartslot_config *config = get_config(device);

	/* if this cartridge has a custom DEVICE_IMAGE_LOAD, use it */
	if (config->device_load != NULL)
		return (*config->device_load)(image);

	/* otherwise try the normal route */
	return process_cartridge(&image, PROCESS_LOAD);
}


/*-------------------------------------------------
    DEVICE_IMAGE_UNLOAD( cartslot )
-------------------------------------------------*/

static DEVICE_IMAGE_UNLOAD( cartslot )
{
	device_t *device = &image.device();
	const cartslot_config *config = get_config(device);

	/* if this cartridge has a custom DEVICE_IMAGE_UNLOAD, use it */
	if (config->device_unload != NULL)
	{
		(*config->device_unload)(image);
		return;
	}
	process_cartridge(&image, PROCESS_CLEAR);
}

/*-------------------------------------------------
    DEVICE_IMAGE_SOFTLIST_LOAD(cartslot)
-------------------------------------------------*/
static DEVICE_IMAGE_SOFTLIST_LOAD(cartslot)
{
	load_software_part_region( &image.device(), swlist, swname, start_entry );
	return TRUE;
}

/*-------------------------------------------------
    DEVICE_GET_INFO( cartslot )
-------------------------------------------------*/

DEVICE_GET_INFO( cartslot )
{
	switch(state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:				info->i = 0; break;
		case DEVINFO_INT_INLINE_CONFIG_BYTES:		info->i = sizeof(cartslot_config); break;
		case DEVINFO_INT_IMAGE_TYPE:				info->i = IO_CARTSLOT; break;
		case DEVINFO_INT_IMAGE_READABLE:			info->i = 1; break;
		case DEVINFO_INT_IMAGE_WRITEABLE:			info->i = 0; break;
		case DEVINFO_INT_IMAGE_CREATABLE:			info->i = 0; break;
		case DEVINFO_INT_IMAGE_RESET_ON_LOAD:		info->i = 1; break;
		case DEVINFO_INT_IMAGE_MUST_BE_LOADED:
			if ( device && downcast<const legacy_image_device_base *>(device)->inline_config()) {
				info->i = get_config(device)->must_be_loaded;
			} else {
				info->i = 0;
			}
			break;

		/* --- the following bits of info are returned as pointers to functions --- */
		case DEVINFO_FCT_START:						info->start = DEVICE_START_NAME(cartslot);					break;
		case DEVINFO_FCT_IMAGE_LOAD:				info->f = (genf *) DEVICE_IMAGE_LOAD_NAME(cartslot);		break;
		case DEVINFO_FCT_IMAGE_UNLOAD:				info->f = (genf *) DEVICE_IMAGE_UNLOAD_NAME(cartslot);		break;
		case DEVINFO_FCT_IMAGE_SOFTLIST_LOAD:		info->f = (genf *) DEVICE_IMAGE_SOFTLIST_LOAD_NAME(cartslot);	break;
		case DEVINFO_FCT_IMAGE_PARTIAL_HASH:
			if ( device && downcast<const legacy_image_device_base *>(device)->inline_config() && get_config(device)->device_partialhash) {
				info->f = (genf *) get_config(device)->device_partialhash;
			} else {
				info->f = NULL;
			}
			break;
		case DEVINFO_FCT_IMAGE_DISPLAY_INFO:
			if ( device && downcast<const legacy_image_device_base *>(device)->inline_config() && get_config(device)->device_displayinfo) {
				info->f = (genf *) get_config(device)->device_displayinfo;
			} else {
				info->f = NULL;
			}
			break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:						strcpy(info->s, "Cartslot"); break;
		case DEVINFO_STR_FAMILY:					strcpy(info->s, "Cartslot"); break;
		case DEVINFO_STR_SOURCE_FILE:				strcpy(info->s, __FILE__); break;
		case DEVINFO_STR_IMAGE_FILE_EXTENSIONS:
			if ( device && downcast<const legacy_image_device_base *>(device)->inline_config() && get_config(device)->extensions )
			{
				strcpy(info->s, get_config(device)->extensions);
			}
			else
			{
				strcpy(info->s, "bin");
			}
			break;
		case DEVINFO_STR_IMAGE_INTERFACE:
			if ( device && downcast<const legacy_image_device_base *>(device)->inline_config() && get_config(device)->interface )
			{
				strcpy(info->s, get_config(device)->interface );
			}
			break;
	}
}

DEFINE_LEGACY_IMAGE_DEVICE(CARTSLOT, cartslot);
