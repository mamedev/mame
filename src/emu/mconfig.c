/***************************************************************************

    mconfig.c

    Machine configuration macros and functions.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#include "emu.h"
#include <ctype.h>


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static void machine_config_detokenize(machine_config *config, const machine_config_token *tokens, const device_config *owner);



/***************************************************************************
    MACHINE CONFIGURATIONS
***************************************************************************/

/*-------------------------------------------------
    machine_config_alloc - allocate a new
    machine configuration and populate it using
    the supplied constructor
-------------------------------------------------*/

machine_config *machine_config_alloc(const machine_config_token *tokens)
{
	machine_config *config;

	/* allocate a new configuration object */
	config = global_alloc_clear(machine_config);

	/* parse tokens into the config */
	machine_config_detokenize(config, tokens, NULL);

	/* process any device-specific machine configurations */
	for (const device_config *device = config->devicelist.first(); device != NULL; device = device->next())
	{
		tokens = device->machine_config_tokens();
		if (tokens != NULL)
			machine_config_detokenize(config, tokens, device);
	}

	// then notify all devices that their configuration is complete
	for (device_config *device = config->devicelist.first(); device != NULL; device = device->next())
		device->config_complete();

	return config;
}


/*-------------------------------------------------
    machine_config_free - release memory allocated
    for a machine configuration
-------------------------------------------------*/

void machine_config_free(machine_config *config)
{
	/* release the configuration itself */
	global_free(config);
}


/*-------------------------------------------------
    machine_config_detokenize - detokenize a
    machine config
-------------------------------------------------*/

static void machine_config_detokenize(machine_config *config, const machine_config_token *tokens, const device_config *owner)
{
	UINT32 entrytype = MCONFIG_TOKEN_INVALID;
	device_config *device = NULL;
	astring tempstring;

	/* loop over tokens until we hit the end */
	while (entrytype != MCONFIG_TOKEN_END)
	{
		device_type devtype;
		const char *tag;
		UINT64 data64;
		UINT32 clock;

		/* unpack the token from the first entry */
		TOKEN_GET_UINT32_UNPACK1(tokens, entrytype, 8);
		switch (entrytype)
		{
			/* end */
			case MCONFIG_TOKEN_END:
				break;

			/* including */
			case MCONFIG_TOKEN_INCLUDE:
				machine_config_detokenize(config, TOKEN_GET_PTR(tokens, tokenptr), owner);
				break;

			/* device management */
			case MCONFIG_TOKEN_DEVICE_ADD:
				TOKEN_UNGET_UINT32(tokens);
				TOKEN_GET_UINT64_UNPACK2(tokens, entrytype, 8, clock, 32);
				devtype = TOKEN_GET_PTR(tokens, devtype);
				tag = owner->subtag(tempstring, TOKEN_GET_STRING(tokens));
				device = config->devicelist.append(tag, (*devtype)(*config, tag, owner, clock));
				break;

			case MCONFIG_TOKEN_DEVICE_REPLACE:
				TOKEN_UNGET_UINT32(tokens);
				TOKEN_GET_UINT64_UNPACK2(tokens, entrytype, 8, clock, 32);
				devtype = TOKEN_GET_PTR(tokens, devtype);
				tag = owner->subtag(tempstring, TOKEN_GET_STRING(tokens));
				device = config->devicelist.replace(tag, (*devtype)(*config, tag, owner, clock));
				break;

			case MCONFIG_TOKEN_DEVICE_REMOVE:
				tag = TOKEN_GET_STRING(tokens);
				config->devicelist.remove(owner->subtag(tempstring, tag));
				device = NULL;
				break;

			case MCONFIG_TOKEN_DEVICE_MODIFY:
				tag = TOKEN_GET_STRING(tokens);
				device = config->devicelist.find(owner->subtag(tempstring, tag));
				if (device == NULL)
					fatalerror("Unable to find device: tag=%s\n", tempstring.cstr());
				break;

			case MCONFIG_TOKEN_DEVICE_CLOCK:
			case MCONFIG_TOKEN_DEVICE_CONFIG:
			case MCONFIG_TOKEN_DEVICE_INLINE_DATA16:
			case MCONFIG_TOKEN_DEVICE_INLINE_DATA32:
			case MCONFIG_TOKEN_DEVICE_INLINE_DATA64:

			case MCONFIG_TOKEN_DIEXEC_DISABLE:
			case MCONFIG_TOKEN_DIEXEC_VBLANK_INT:
			case MCONFIG_TOKEN_DIEXEC_PERIODIC_INT:

			case MCONFIG_TOKEN_DIMEMORY_MAP:

			case MCONFIG_TOKEN_DISOUND_ROUTE:
			case MCONFIG_TOKEN_DISOUND_RESET:

			case MCONFIG_TOKEN_DEVICE_CONFIG_CUSTOM_1:
			case MCONFIG_TOKEN_DEVICE_CONFIG_CUSTOM_2:
			case MCONFIG_TOKEN_DEVICE_CONFIG_CUSTOM_3:
			case MCONFIG_TOKEN_DEVICE_CONFIG_CUSTOM_4:
			case MCONFIG_TOKEN_DEVICE_CONFIG_CUSTOM_5:
			case MCONFIG_TOKEN_DEVICE_CONFIG_CUSTOM_6:
			case MCONFIG_TOKEN_DEVICE_CONFIG_CUSTOM_7:
			case MCONFIG_TOKEN_DEVICE_CONFIG_CUSTOM_8:
			case MCONFIG_TOKEN_DEVICE_CONFIG_CUSTOM_9:
			case MCONFIG_TOKEN_DEVICE_CONFIG_DATA32:
			case MCONFIG_TOKEN_DEVICE_CONFIG_DATA64:
			case MCONFIG_TOKEN_DEVICE_CONFIG_DATAFP32:
			case MCONFIG_TOKEN_DEVICE_CONFIG_CUSTOM_FREE:
				assert(device != NULL);
				device->process_token(entrytype, tokens);
				break;


			/* core parameters */
			case MCONFIG_TOKEN_DRIVER_DATA:
				config->driver_data_alloc = TOKEN_GET_PTR(tokens, driver_data_alloc);
				break;

			case MCONFIG_TOKEN_QUANTUM_TIME:
				TOKEN_EXTRACT_UINT64(tokens, data64);
				config->minimum_quantum = UINT64_ATTOTIME_TO_ATTOTIME(data64);
				break;

			case MCONFIG_TOKEN_QUANTUM_PERFECT_CPU:
				config->perfect_cpu_quantum = TOKEN_GET_STRING(tokens);
				break;

			case MCONFIG_TOKEN_WATCHDOG_VBLANK:
				TOKEN_UNGET_UINT32(tokens);
				TOKEN_GET_UINT32_UNPACK2(tokens, entrytype, 8, config->watchdog_vblank_count, 24);
				break;

			case MCONFIG_TOKEN_WATCHDOG_TIME:
				TOKEN_EXTRACT_UINT64(tokens, data64);
				config->watchdog_time = UINT64_ATTOTIME_TO_ATTOTIME(data64);
				break;

			/* core functions */
			case MCONFIG_TOKEN_MACHINE_START:
				config->machine_start = TOKEN_GET_PTR(tokens, machine_start);
				break;

			case MCONFIG_TOKEN_MACHINE_RESET:
				config->machine_reset = TOKEN_GET_PTR(tokens, machine_reset);
				break;

			case MCONFIG_TOKEN_NVRAM_HANDLER:
				config->nvram_handler = TOKEN_GET_PTR(tokens, nvram_handler);
				break;

			case MCONFIG_TOKEN_MEMCARD_HANDLER:
				config->memcard_handler = TOKEN_GET_PTR(tokens, memcard_handler);
				break;

			/* core video parameters */
			case MCONFIG_TOKEN_VIDEO_ATTRIBUTES:
				TOKEN_UNGET_UINT32(tokens);
				TOKEN_GET_UINT32_UNPACK2(tokens, entrytype, 8, config->video_attributes, 24);
				break;

			case MCONFIG_TOKEN_GFXDECODE:
				config->gfxdecodeinfo = TOKEN_GET_PTR(tokens, gfxdecode);
				break;

			case MCONFIG_TOKEN_PALETTE_LENGTH:
				TOKEN_UNGET_UINT32(tokens);
				TOKEN_GET_UINT32_UNPACK2(tokens, entrytype, 8, config->total_colors, 24);
				break;

			case MCONFIG_TOKEN_DEFAULT_LAYOUT:
				config->default_layout = TOKEN_GET_STRING(tokens);
				break;

			/* core video functions */
			case MCONFIG_TOKEN_PALETTE_INIT:
				config->init_palette = TOKEN_GET_PTR(tokens, palette_init);
				break;

			case MCONFIG_TOKEN_VIDEO_START:
				config->video_start = TOKEN_GET_PTR(tokens, video_start);
				break;

			case MCONFIG_TOKEN_VIDEO_RESET:
				config->video_reset = TOKEN_GET_PTR(tokens, video_reset);
				break;

			case MCONFIG_TOKEN_VIDEO_EOF:
				config->video_eof = TOKEN_GET_PTR(tokens, video_eof);
				break;

			case MCONFIG_TOKEN_VIDEO_UPDATE:
				config->video_update = TOKEN_GET_PTR(tokens, video_update);
				break;

			/* core sound functions */
			case MCONFIG_TOKEN_SOUND_START:
				config->sound_start = TOKEN_GET_PTR(tokens, sound_start);
				break;

			case MCONFIG_TOKEN_SOUND_RESET:
				config->sound_reset = TOKEN_GET_PTR(tokens, sound_reset);
				break;

			default:
				fatalerror("Invalid token %d in machine config\n", entrytype);
				break;
		}
	}
}
