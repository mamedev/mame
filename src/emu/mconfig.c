/***************************************************************************

    mconfig.c

    Machine configuration macros and functions.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#include "driver.h"
#include "devintrf.h"
#include <ctype.h>

#if defined(_MSC_VER)
#pragma optimize ("", off)
#endif



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static void machine_config_detokenize(machine_config *config, const machine_config_token *tokens, const device_config *owner, int depth);



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
	config = malloc_or_die(sizeof(*config));
	memset(config, 0, sizeof(*config));

	/* parse tokens into the config */
	machine_config_detokenize(config, tokens, NULL, 0);
	return config;
}


/*-------------------------------------------------
    machine_config_free - release memory allocated
    for a machine configuration
-------------------------------------------------*/

void machine_config_free(machine_config *config)
{
	/* release the device list */
	while (config->devicelist != NULL)
		device_list_remove(&config->devicelist, config->devicelist->type, config->devicelist->tag);

	/* release the configuration itself */
	free(config);
}


/*-------------------------------------------------
    machine_config_detokenize - detokenize a
    machine config
-------------------------------------------------*/

static void machine_config_detokenize(machine_config *config, const machine_config_token *tokens, const device_config *owner, int depth)
{
	UINT32 entrytype = MCONFIG_TOKEN_INVALID;
	astring *tempstring = astring_alloc();
	device_config *device = NULL;

	/* loop over tokens until we hit the end */
	while (entrytype != MCONFIG_TOKEN_END)
	{
		int size, offset, bits;
		UINT32 data32, clock;
		device_type devtype;
		const char *tag;
		UINT64 data64;

		/* unpack the token from the first entry */
		TOKEN_GET_UINT32_UNPACK1(tokens, entrytype, 8);
		switch (entrytype)
		{
			/* end */
			case MCONFIG_TOKEN_END:
				break;

			/* including */
			case MCONFIG_TOKEN_INCLUDE:
				machine_config_detokenize(config, TOKEN_GET_PTR(tokens, tokenptr), owner, depth + 1);
				break;

			/* device management */
			case MCONFIG_TOKEN_DEVICE_ADD:
				TOKEN_UNGET_UINT32(tokens);
				TOKEN_GET_UINT64_UNPACK2(tokens, entrytype, 8, clock, 32);
				devtype = TOKEN_GET_PTR(tokens, devtype);
				tag = TOKEN_GET_STRING(tokens);
				device = device_list_add(&config->devicelist, owner, devtype, device_build_tag(tempstring, owner, tag), clock);
				break;

			case MCONFIG_TOKEN_DEVICE_REMOVE:
				devtype = TOKEN_GET_PTR(tokens, devtype);
				tag = TOKEN_GET_STRING(tokens);
				device_list_remove(&config->devicelist, devtype, device_build_tag(tempstring, owner, tag));
				device = NULL;
				break;

			case MCONFIG_TOKEN_DEVICE_MODIFY:
				devtype = TOKEN_GET_PTR(tokens, devtype);
				tag = TOKEN_GET_STRING(tokens);
				device = (device_config *)device_list_find_by_tag(config->devicelist, devtype, device_build_tag(tempstring, owner, tag));
				if (device == NULL)
					fatalerror("Unable to find device: type=%s tag=%s\n", devtype_get_name(devtype), astring_c(tempstring));
				break;

			case MCONFIG_TOKEN_DEVICE_CLOCK:
				assert(device != NULL);
				TOKEN_UNGET_UINT32(tokens);
				TOKEN_GET_UINT64_UNPACK2(tokens, entrytype, 8, device->clock, 32);
				break;

			case MCONFIG_TOKEN_DEVICE_CONFIG:
				assert(device != NULL);
				device->static_config = TOKEN_GET_PTR(tokens, voidptr);
				break;

			case MCONFIG_TOKEN_DEVICE_CONFIG_DATA32:
				assert(device != NULL);
				TOKEN_UNGET_UINT32(tokens);
				TOKEN_GET_UINT32_UNPACK3(tokens, entrytype, 8, size, 6, offset, 12);
				data32 = TOKEN_GET_UINT32(tokens);
				switch (size)
				{
					case 1: *(UINT8 *) ((UINT8 *)device->inline_config + offset) = data32; break;
					case 2: *(UINT16 *)((UINT8 *)device->inline_config + offset) = data32; break;
					case 4: *(UINT32 *)((UINT8 *)device->inline_config + offset) = data32; break;
				}
				break;

			case MCONFIG_TOKEN_DEVICE_CONFIG_DATA64:
				assert(device != NULL);
				TOKEN_UNGET_UINT32(tokens);
				TOKEN_GET_UINT32_UNPACK3(tokens, entrytype, 8, size, 6, offset, 12);
				TOKEN_EXTRACT_UINT64(tokens, data64);
				switch (size)
				{
					case 1: *(UINT8 *) ((UINT8 *)device->inline_config + offset) = data64; break;
					case 2: *(UINT16 *)((UINT8 *)device->inline_config + offset) = data64; break;
					case 4: *(UINT32 *)((UINT8 *)device->inline_config + offset) = data64; break;
					case 8: *(UINT64 *)((UINT8 *)device->inline_config + offset) = data64; break;
				}
				break;

			case MCONFIG_TOKEN_DEVICE_CONFIG_DATAFP32:
				assert(device != NULL);
				TOKEN_UNGET_UINT32(tokens);
				TOKEN_GET_UINT32_UNPACK4(tokens, entrytype, 8, size, 6, bits, 6, offset, 12);
				data32 = TOKEN_GET_UINT32(tokens);
				switch (size)
				{
					case 4: *(float *)((UINT8 *)device->inline_config + offset) = (float)(INT32)data32 / (float)(1 << bits); break;
					case 8: *(double *)((UINT8 *)device->inline_config + offset) = (double)(INT32)data32 / (double)(1 << bits); break;
				}
				break;

			case MCONFIG_TOKEN_DEVICE_CONFIG_DATAFP64:
				assert(device != NULL);
				TOKEN_UNGET_UINT32(tokens);
				TOKEN_GET_UINT32_UNPACK4(tokens, entrytype, 8, size, 6, bits, 6, offset, 12);
				TOKEN_EXTRACT_UINT64(tokens, data64);
				switch (size)
				{
					case 4: *(float *)((UINT8 *)device->inline_config + offset) = (float)(INT64)data64 / (float)((UINT64)1 << bits); break;
					case 8: *(double *)((UINT8 *)device->inline_config + offset) = (double)(INT64)data64 / (double)((UINT64)1 << bits); break;
				}
				break;


			/* core parameters */
			case MCONFIG_TOKEN_DRIVER_DATA:
				TOKEN_UNGET_UINT32(tokens);
				TOKEN_GET_UINT32_UNPACK2(tokens, entrytype, 8, config->driver_data_size, 24);
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

	/* if we are the outermost level, process any device-specific machine configurations */
	if (depth == 0)
		for (device = config->devicelist; device != NULL; device = device->next)
		{
			tokens = device_get_info_ptr(device, DEVINFO_PTR_MACHINE_CONFIG);
			if (tokens != NULL)
				machine_config_detokenize(config, tokens, device, depth + 1);
		}

	astring_free(tempstring);
}
