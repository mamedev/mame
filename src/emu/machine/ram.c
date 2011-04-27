/*************************************************************************

    RAM device

    Provides a configurable amount of RAM to drivers

**************************************************************************/

#include <stdio.h>
#include <ctype.h>

#include "emu.h"
#include "emuopts.h"
#include "ram.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

#define RAM_STRING_BUFLEN	16
#define MAX_RAM_OPTIONS		16


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _ram_state ram_state;
struct _ram_state
{
	UINT32 size; /* total amount of ram configured */
	UINT8 *ram;  /* pointer to the start of ram */
};


/*****************************************************************************
    INLINE FUNCTIONS
*****************************************************************************/

INLINE ram_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == RAM);

	return (ram_state *)downcast<legacy_device_base *>(device)->token();
}


/*****************************************************************************
    HELPER FUNCTIONS
*****************************************************************************/

/*-------------------------------------------------
    ram_parse_string - convert a ram string to an
    integer value
-------------------------------------------------*/

UINT32 ram_parse_string(const char *s)
{
	UINT32 ram;
	char suffix = '\0';

	s += sscanf(s, "%u%c", &ram, &suffix);

	switch(tolower(suffix))
	{
	case 'k':
		/* kilobytes */
		ram *= 1024;
		break;

	case 'm':
		/* megabytes */
		ram *= 1024*1024;
		break;

	case '\0':
		/* no suffix */
		break;

	default:
		/* parse failure */
		ram = 0;
		break;
	}

	return ram;
}

/*****************************************************************************
    DEVICE INTERFACE
*****************************************************************************/

static DEVICE_START( ram )
{
	ram_state *ram = get_safe_token(device);
	ram_config *config = (ram_config *)downcast<const legacy_device_base *>(device)->inline_config();

	/* the device named 'ram' can get ram options from command line */
	if (strcmp(device->tag(), RAM_TAG) == 0)
	{
		const char *ramsize_string = device->machine().options().ram_size();

		if ((ramsize_string != NULL) && (ramsize_string[0] != '\0'))
			ram->size = ram_parse_string(ramsize_string);
	}

	/* if we didn't get a size yet, use the default */
	if (ram->size == 0)
		ram->size = ram_parse_string(config->default_size);

	/* allocate space for the ram */
	ram->ram = auto_alloc_array(device->machine(), UINT8, ram->size);

	/* reset ram to the default value */
	memset(ram->ram, config->default_value, ram->size);

	/* register for state saving */
	device->save_item(NAME(ram->size));
	device->save_pointer(NAME(ram->ram), ram->size);
}

static DEVICE_VALIDITY_CHECK( ram )
{
	ram_config *config = (ram_config *)downcast<const legacy_device_base *>(device)->inline_config();
	const char *ramsize_string = NULL;
	int is_valid = FALSE;
	UINT32 specified_ram = 0;
	int error = FALSE;
	const char *gamename_option = NULL;

	/* verify default ram value */
	if (config!=NULL && ram_parse_string(config->default_size) == 0)
	{
		mame_printf_error("%s: '%s' has an invalid default RAM option: %s\n", driver->source_file, driver->name, config->default_size);
		error = TRUE;
	}

	/* command line options are only parsed for the device named RAM_TAG */
	if (device->tag()!=NULL && strcmp(device->tag(), RAM_TAG) == 0)
	{
		/* verify command line ram option */
		ramsize_string = options.ram_size();
		gamename_option = options.system_name();

		if ((ramsize_string != NULL) && (ramsize_string[0] != '\0'))
		{
			specified_ram = ram_parse_string(ramsize_string);

			if (specified_ram == 0)
			{
				mame_printf_error("%s: '%s' cannot recognize the RAM option %s\n", driver->source_file, driver->name, ramsize_string);
				error = TRUE;
			}
			if (gamename_option != NULL && *gamename_option != 0 && strcmp(gamename_option, driver->name) == 0)
			{
				/* compare command line option to default value */
				if (ram_parse_string(config->default_size) == specified_ram)
					is_valid = TRUE;

				/* verify extra ram options */
				if (config->extra_options != NULL)
				{
					int j;
					int size = strlen(config->extra_options);
					char * const s = mame_strdup(config->extra_options);
					char * const e = s + size;
					char *p = s;
					for (j=0;j<size;j++) {
						if (p[j]==',') p[j]=0;
					}

					/* try to parse each option */
					while(p <= e)
					{
						UINT32 option_ram_size = ram_parse_string(p);

						if (option_ram_size == 0)
						{
							mame_printf_error("%s: '%s' has an invalid RAM option: %s\n", driver->source_file, driver->name, p);
							error = TRUE;
						}

						if (option_ram_size == specified_ram)
							is_valid = TRUE;

						p += strlen(p);
						if (p == e)
							break;
						p += 1;
					}

					osd_free(s);
				}

			} else {
				/* if not for this driver then return ok */
				is_valid = TRUE;
			}
		}
		else
		{
			/* not specifying the ramsize on the command line is valid as well */
			is_valid = TRUE;
		}
	}
	else
		is_valid = TRUE;

	if (!is_valid)
	{
		mame_printf_error("%s: '%s' cannot recognize the RAM option %s", driver->source_file, driver->name, ramsize_string);
		mame_printf_error(" (valid options are %s", config->default_size);

		if (config->extra_options != NULL)
			mame_printf_error(",%s).\n", config->extra_options);
		else
			mame_printf_error(").\n");

		error = TRUE;
	}

	return error;

}


DEVICE_GET_INFO( ram )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(ram_state);			break;
		case DEVINFO_INT_INLINE_CONFIG_BYTES:			info->i = sizeof(ram_config);				break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME(ram);	break;
		case DEVINFO_FCT_STOP:							/* Nothing */								break;
		case DEVINFO_FCT_RESET:							/* Nothing */								break;
		case DEVINFO_FCT_VALIDITY_CHECK:				info->p = (void*)DEVICE_VALIDITY_CHECK_NAME(ram); break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "RAM");				break;
		case DEVINFO_STR_FAMILY:						strcpy(info->s, "RAM");						break;
		case DEVINFO_STR_VERSION:						strcpy(info->s, "1.00");					break;
		case DEVINFO_STR_SOURCE_FILE:					strcpy(info->s, __FILE__);					break;
		case DEVINFO_STR_CREDITS:						strcpy(info->s, "Copyright MESS Team");		break;
	}
}

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

UINT32 ram_get_size(device_t *device)
{
	ram_state *ram = get_safe_token(device);
	return ram->size;
}

UINT8 *ram_get_ptr(device_t *device)
{
	ram_state *ram = get_safe_token(device);
	return ram->ram;
}

DEFINE_LEGACY_DEVICE(RAM, ram);
