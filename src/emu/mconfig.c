/***************************************************************************

    mconfig.c

    Machine configuration macros and functions.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#include "driver.h"
#include "devintrf.h"
#include <ctype.h>


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static void machine_config_detokenize(machine_config *config, const machine_config_token *tokens, const char *tagprefix, int depth);



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    create_tag - create a combined tag,
    allocating strings as necessary
-------------------------------------------------*/

INLINE const char *create_tag(astring *result, const char *prefix, const char *tag)
{
	if (prefix != NULL && prefix[0] != 0)
		astring_printf(result, "%s:%s", prefix, tag);
	else
		astring_cpyc(result, tag);
	return astring_c(result);
}



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
	machine_config_detokenize(config, tokens, "", 0);
	return config;
}


/*-------------------------------------------------
    machine_config_free - release memory allocated
    for a machine configuration
-------------------------------------------------*/

void machine_config_free(machine_config *config)
{
	int cpunum, soundnum;

	/* release the device list */
	while (config->devicelist != NULL)
		device_list_remove(&config->devicelist, config->devicelist->type, config->devicelist->tag);

	/* release the strings */
	for (cpunum = 0; cpunum < ARRAY_LENGTH(config->cputag); cpunum++)
		if (config->cputag[cpunum] != NULL)
			astring_free(config->cputag[cpunum]);
	for (soundnum = 0; soundnum < ARRAY_LENGTH(config->soundtag); soundnum++)
		if (config->soundtag[soundnum] != NULL)
			astring_free(config->soundtag[soundnum]);

	/* release the configuration itself */
	free(config);
}


/*-------------------------------------------------
    cpu_add - add a CPU during machine driver
    expansion
-------------------------------------------------*/

static cpu_config *cpu_add(machine_config *machine, const char *tag, cpu_type type, int cpuclock)
{
	int cpunum;

	for (cpunum = 0; cpunum < MAX_CPU; cpunum++)
		if (machine->cpu[cpunum].type == CPU_DUMMY)
		{
			if (machine->cputag[cpunum] == NULL)
				machine->cputag[cpunum] = astring_alloc();
			astring_cpyc(machine->cputag[cpunum], tag);
			machine->cpu[cpunum].tag = astring_c(machine->cputag[cpunum]);
			machine->cpu[cpunum].type = type;
			machine->cpu[cpunum].clock = cpuclock;
			return &machine->cpu[cpunum];
		}

	fatalerror("Out of CPU's!\n");
	return NULL;
}


/*-------------------------------------------------
    cpu_find - find a tagged CPU during machine
    driver expansion
-------------------------------------------------*/

static cpu_config *cpu_find(machine_config *machine, const char *tag)
{
	int cpunum;

	for (cpunum = 0; cpunum < MAX_CPU; cpunum++)
		if (machine->cpu[cpunum].tag && strcmp(machine->cpu[cpunum].tag, tag) == 0)
			return &machine->cpu[cpunum];

	fatalerror("Can't find CPU '%s'!\n", tag);
	return NULL;
}


/*-------------------------------------------------
    cpu_remove - remove a tagged CPU during
    machine driver expansion
-------------------------------------------------*/

static void cpu_remove(machine_config *machine, const char *tag)
{
	int cpunum;

	for (cpunum = 0; cpunum < MAX_CPU; cpunum++)
		if (machine->cpu[cpunum].tag && strcmp(machine->cpu[cpunum].tag, tag) == 0)
		{
			if (machine->cputag[cpunum] != NULL)
				astring_free(machine->cputag[cpunum]);
			memmove(&machine->cpu[cpunum], &machine->cpu[cpunum + 1], sizeof(machine->cpu[0]) * (MAX_CPU - cpunum - 1));
			memmove(&machine->cputag[cpunum], &machine->cputag[cpunum + 1], sizeof(machine->cputag[0]) * (MAX_CPU - cpunum - 1));
			memset(&machine->cpu[MAX_CPU - 1], 0, sizeof(machine->cpu[0]));
			memset(&machine->cputag[MAX_CPU - 1], 0, sizeof(machine->cputag[0]));
			return;
		}

	fatalerror("Can't find CPU '%s'!\n", tag);
}


/*-------------------------------------------------
    sound_add - add a sound system during
    machine driver expansion
-------------------------------------------------*/

static sound_config *sound_add(machine_config *machine, const char *tag, sound_type type, int clock)
{
	int soundnum;

	for (soundnum = 0; soundnum < MAX_SOUND; soundnum++)
		if (machine->sound[soundnum].type == SOUND_DUMMY)
		{
			if (machine->soundtag[soundnum] == NULL)
				machine->soundtag[soundnum] = astring_alloc();
			astring_cpyc(machine->soundtag[soundnum], tag);
			machine->sound[soundnum].tag = astring_c(machine->soundtag[soundnum]);
			machine->sound[soundnum].type = type;
			machine->sound[soundnum].clock = clock;
			machine->sound[soundnum].config = NULL;
			machine->sound[soundnum].routes = 0;
			return &machine->sound[soundnum];
		}

	fatalerror("Out of sounds!\n");
	return NULL;
}


/*-------------------------------------------------
    sound_find - find a tagged sound system during
    machine driver expansion
-------------------------------------------------*/

static sound_config *sound_find(machine_config *machine, const char *tag)
{
	int soundnum;

	for (soundnum = 0; soundnum < MAX_SOUND; soundnum++)
		if (machine->sound[soundnum].tag && strcmp(machine->sound[soundnum].tag, tag) == 0)
			return &machine->sound[soundnum];

	fatalerror("Can't find sound '%s'!\n", tag);
	return NULL;
}


/*-------------------------------------------------
    sound_remove - remove a tagged sound system
    during machine driver expansion
-------------------------------------------------*/

static void sound_remove(machine_config *machine, const char *tag)
{
	int soundnum;

	for (soundnum = 0; soundnum < MAX_SOUND; soundnum++)
		if (machine->sound[soundnum].tag && strcmp(machine->sound[soundnum].tag, tag) == 0)
		{
			if (machine->soundtag[soundnum] != NULL)
				astring_free(machine->soundtag[soundnum]);
			memmove(&machine->sound[soundnum], &machine->sound[soundnum + 1], sizeof(machine->sound[0]) * (MAX_SOUND - soundnum - 1));
			memmove(&machine->soundtag[soundnum], &machine->soundtag[soundnum + 1], sizeof(machine->soundtag[0]) * (MAX_SOUND - soundnum - 1));
			memset(&machine->sound[MAX_SOUND - 1], 0, sizeof(machine->sound[0]));
			memset(&machine->soundtag[MAX_SOUND - 1], 0, sizeof(machine->soundtag[0]));
			return;
		}

	fatalerror("Can't find sound '%s'!\n", tag);
}


/*-------------------------------------------------
    machine_config_detokenize - detokenize a
    machine config
-------------------------------------------------*/

static void machine_config_detokenize(machine_config *config, const machine_config_token *tokens, const char *tagprefix, int depth)
{
	UINT32 entrytype = MCONFIG_TOKEN_INVALID;
	astring *tempstring = astring_alloc();
	device_config *device = NULL;
	sound_config *sound = NULL;
	cpu_config *cpu = NULL;

	/* loop over tokens until we hit the end */
	while (entrytype != MCONFIG_TOKEN_END)
	{
		int size, offset, type, bits, in, out;
		UINT32 data32, clock, gain;
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
				machine_config_detokenize(config, TOKEN_GET_PTR(tokens, tokenptr), tagprefix, depth + 1);
				break;

			/* device management */
			case MCONFIG_TOKEN_DEVICE_ADD:
				devtype = TOKEN_GET_PTR(tokens, devtype);
				tag = TOKEN_GET_STRING(tokens);
				device = device_list_add(&config->devicelist, devtype, create_tag(tempstring, tagprefix, tag));
				break;

			case MCONFIG_TOKEN_DEVICE_REMOVE:
				devtype = TOKEN_GET_PTR(tokens, devtype);
				tag = TOKEN_GET_STRING(tokens);
				device_list_remove(&config->devicelist, devtype, create_tag(tempstring, tagprefix, tag));
				device = NULL;
				break;

			case MCONFIG_TOKEN_DEVICE_MODIFY:
				devtype = TOKEN_GET_PTR(tokens, devtype);
				tag = TOKEN_GET_STRING(tokens);
				device = (device_config *)device_list_find_by_tag(config->devicelist, devtype, create_tag(tempstring, tagprefix, tag));
				if (device == NULL)
					fatalerror("Unable to find device: type=%s tag=%s\n", devtype_name(devtype), astring_c(tempstring));
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


			/* add/modify/remove/replace CPUs */
			case MCONFIG_TOKEN_CPU_ADD:
				TOKEN_UNGET_UINT32(tokens);
				TOKEN_GET_UINT64_UNPACK3(tokens, entrytype, 8, type, 24, clock, 32);
				tag = TOKEN_GET_STRING(tokens);
				cpu = cpu_add(config, create_tag(tempstring, tagprefix, tag), type, clock);
				break;

			case MCONFIG_TOKEN_CPU_MODIFY:
				tag = TOKEN_GET_STRING(tokens);
				cpu = cpu_find(config, create_tag(tempstring, tagprefix, tag));
				break;

			case MCONFIG_TOKEN_CPU_REMOVE:
				tag = TOKEN_GET_STRING(tokens);
				cpu_remove(config, create_tag(tempstring, tagprefix, tag));
				cpu = NULL;
				break;

			case MCONFIG_TOKEN_CPU_REPLACE:
				TOKEN_UNGET_UINT32(tokens);
				TOKEN_GET_UINT64_UNPACK3(tokens, entrytype, 8, type, 24, clock, 32);
				tag = TOKEN_GET_STRING(tokens);
				cpu = cpu_find(config, create_tag(tempstring, tagprefix, tag));
				if (cpu == NULL)
					fatalerror("Unable to find CPU: tag=%s\n", astring_c(tempstring));
				cpu->type = type;
				cpu->clock = clock;
				break;

			/* CPU parameters */
			case MCONFIG_TOKEN_CPU_FLAGS:
				assert(cpu != NULL);
				TOKEN_UNGET_UINT32(tokens);
				TOKEN_GET_UINT32_UNPACK2(tokens, entrytype, 8, cpu->flags, 24);
				break;

			case MCONFIG_TOKEN_CPU_CONFIG:
				assert(cpu != NULL);
				cpu->reset_param = TOKEN_GET_PTR(tokens, voidptr);
				break;

			case MCONFIG_TOKEN_CPU_PROGRAM_MAP:
				assert(cpu != NULL);
				cpu->address_map[ADDRESS_SPACE_PROGRAM][0] = TOKEN_GET_PTR(tokens, addrmap);
				cpu->address_map[ADDRESS_SPACE_PROGRAM][1] = TOKEN_GET_PTR(tokens, addrmap);
				break;

			case MCONFIG_TOKEN_CPU_DATA_MAP:
				assert(cpu != NULL);
				cpu->address_map[ADDRESS_SPACE_DATA][0] = TOKEN_GET_PTR(tokens, addrmap);
				cpu->address_map[ADDRESS_SPACE_DATA][1] = TOKEN_GET_PTR(tokens, addrmap);
				break;

			case MCONFIG_TOKEN_CPU_IO_MAP:
				assert(cpu != NULL);
				cpu->address_map[ADDRESS_SPACE_IO][0] = TOKEN_GET_PTR(tokens, addrmap);
				cpu->address_map[ADDRESS_SPACE_IO][1] = TOKEN_GET_PTR(tokens, addrmap);
				break;

			case MCONFIG_TOKEN_CPU_VBLANK_INT:
				assert(cpu != NULL);
				TOKEN_UNGET_UINT32(tokens);
				TOKEN_GET_UINT32_UNPACK1(tokens, entrytype, 8);
				cpu->vblank_interrupt_screen = TOKEN_GET_STRING(tokens);
				cpu->vblank_interrupt = TOKEN_GET_PTR(tokens, interrupt);
				cpu->vblank_interrupts_per_frame = 1;
				break;

			case MCONFIG_TOKEN_CPU_VBLANK_INT_HACK:
				assert(cpu != NULL);
				TOKEN_UNGET_UINT32(tokens);
				TOKEN_GET_UINT32_UNPACK2(tokens, entrytype, 8, cpu->vblank_interrupts_per_frame, 24);
				cpu->vblank_interrupt = TOKEN_GET_PTR(tokens, interrupt);
				break;

			case MCONFIG_TOKEN_CPU_PERIODIC_INT:
				assert(cpu != NULL);
				TOKEN_UNGET_UINT32(tokens);
				TOKEN_GET_UINT32_UNPACK2(tokens, entrytype, 8, data32, 24);
				cpu->timed_interrupt_period = HZ_TO_ATTOSECONDS(data32);
				cpu->timed_interrupt = TOKEN_GET_PTR(tokens, interrupt);
				break;

			/* core parameters */
			case MCONFIG_TOKEN_DRIVER_DATA:
				TOKEN_UNGET_UINT32(tokens);
				TOKEN_GET_UINT32_UNPACK2(tokens, entrytype, 8, config->driver_data_size, 24);
				break;

			case MCONFIG_TOKEN_INTERLEAVE:
				TOKEN_UNGET_UINT32(tokens);
				TOKEN_GET_UINT32_UNPACK2(tokens, entrytype, 8, config->cpu_slices_per_frame, 24);
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

			/* add/remove/replace sounds */
			case MCONFIG_TOKEN_SOUND_ADD:
				TOKEN_UNGET_UINT32(tokens);
				TOKEN_GET_UINT64_UNPACK3(tokens, entrytype, 8, type, 24, clock, 32);
				tag = TOKEN_GET_STRING(tokens);
				sound = sound_add(config, create_tag(tempstring, tagprefix, tag), type, clock);
				break;

			case MCONFIG_TOKEN_SOUND_REMOVE:
				sound_remove(config, TOKEN_GET_STRING(tokens));
				break;

			case MCONFIG_TOKEN_SOUND_MODIFY:
				tag = TOKEN_GET_STRING(tokens);
				sound = sound_find(config, create_tag(tempstring, tagprefix, tag));
				if (sound == NULL)
					fatalerror("Unable to find sound: tag=%s\n", astring_c(tempstring));
				sound->routes = 0;
				break;

			case MCONFIG_TOKEN_SOUND_CONFIG:
				assert(sound != NULL);
				sound->config = TOKEN_GET_PTR(tokens, voidptr);
				break;

			case MCONFIG_TOKEN_SOUND_REPLACE:
				TOKEN_UNGET_UINT32(tokens);
				TOKEN_GET_UINT64_UNPACK3(tokens, entrytype, 8, type, 24, clock, 32);
				tag = TOKEN_GET_STRING(tokens);
				sound = sound_find(config, create_tag(tempstring, tagprefix, tag));
				if (sound == NULL)
					fatalerror("Unable to find sound: tag=%s\n", astring_c(tempstring));
				sound->type = type;
				sound->clock = clock;
				sound->config = NULL;
				sound->routes = 0;
				break;

			case MCONFIG_TOKEN_SOUND_ROUTE:
				assert(sound != NULL);
				TOKEN_UNGET_UINT32(tokens);
				TOKEN_GET_UINT64_UNPACK4(tokens, entrytype, 8, out, -12, in, -12, gain, 32);
				sound->route[sound->routes].input = in;
				sound->route[sound->routes].output = out;
				sound->route[sound->routes].gain = (float)gain / 16777216.0f;
				sound->route[sound->routes].target = TOKEN_GET_STRING(tokens);
				sound->routes++;
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
				machine_config_detokenize(config, tokens, create_tag(tempstring, tagprefix, device->tag), depth + 1);
		}

	astring_free(tempstring);
}
