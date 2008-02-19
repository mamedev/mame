/***************************************************************************

    mconfig.c

    Machine configuration macros and functions.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#include "driver.h"
#include <ctype.h>


/***************************************************************************
    MACHINE_CONFIGURATIONS
***************************************************************************/

/*-------------------------------------------------
    machine_config_alloc - allocate a new
    machine configuration and populate it using
    the supplied constructor
-------------------------------------------------*/

machine_config *machine_config_alloc(void (*constructor)(machine_config *))
{
	machine_config *config;
	
	/* allocate a new configuration object */
	config = malloc_or_die(sizeof(*config));
	if (config == NULL)
		return NULL;
	memset(config, 0, sizeof(*config));
	
	/* call the function to construct the data */
	(*constructor)(config);

	/* if no screen tagged, tag screen 0 as main */
	if (config->screen[0].tag == NULL && config->screen[0].defstate.format != BITMAP_FORMAT_INVALID)
		config->screen[0].tag = "main";

	/* if no screens, set a dummy refresh for the main screen */
	if (config->screen[0].tag == NULL)
		config->screen[0].defstate.refresh = HZ_TO_ATTOSECONDS(60);
	
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
    machine_config_add_cpu - add a CPU during machine
    driver expansion
-------------------------------------------------*/

cpu_config *machine_config_add_cpu(machine_config *machine, const char *tag, cpu_type type, int cpuclock)
{
	int cpunum;

	for (cpunum = 0; cpunum < MAX_CPU; cpunum++)
		if (machine->cpu[cpunum].type == CPU_DUMMY)
		{
			machine->cpu[cpunum].tag = tag;
			machine->cpu[cpunum].type = type;
			machine->cpu[cpunum].clock = cpuclock;
			return &machine->cpu[cpunum];
		}

	fatalerror("Out of CPU's!\n");
	return NULL;
}


/*-------------------------------------------------
    machine_config_find_cpu - find a tagged CPU during
    machine driver expansion
-------------------------------------------------*/

cpu_config *machine_config_find_cpu(machine_config *machine, const char *tag)
{
	int cpunum;

	for (cpunum = 0; cpunum < MAX_CPU; cpunum++)
		if (machine->cpu[cpunum].tag && strcmp(machine->cpu[cpunum].tag, tag) == 0)
			return &machine->cpu[cpunum];

	fatalerror("Can't find CPU '%s'!\n", tag);
	return NULL;
}


/*-------------------------------------------------
    machine_config_remove_cpu - remove a tagged CPU
    during machine driver expansion
-------------------------------------------------*/

void machine_config_remove_cpu(machine_config *machine, const char *tag)
{
	int cpunum;

	for (cpunum = 0; cpunum < MAX_CPU; cpunum++)
		if (machine->cpu[cpunum].tag && strcmp(machine->cpu[cpunum].tag, tag) == 0)
		{
			memmove(&machine->cpu[cpunum], &machine->cpu[cpunum + 1], sizeof(machine->cpu[0]) * (MAX_CPU - cpunum - 1));
			memset(&machine->cpu[MAX_CPU - 1], 0, sizeof(machine->cpu[0]));
			return;
		}

	fatalerror("Can't find CPU '%s'!\n", tag);
}


/*-------------------------------------------------
    machine_config_add_speaker - add a speaker during
    machine driver expansion
-------------------------------------------------*/

speaker_config *machine_config_add_speaker(machine_config *machine, const char *tag, float x, float y, float z)
{
	int speakernum;

	for (speakernum = 0; speakernum < MAX_SPEAKER; speakernum++)
		if (machine->speaker[speakernum].tag == NULL)
		{
			machine->speaker[speakernum].tag = tag;
			machine->speaker[speakernum].x = x;
			machine->speaker[speakernum].y = y;
			machine->speaker[speakernum].z = z;
			return &machine->speaker[speakernum];
		}

	fatalerror("Out of speakers!\n");
	return NULL;
}


/*-------------------------------------------------
    machine_config_find_speaker - find a tagged speaker
    system during machine driver expansion
-------------------------------------------------*/

speaker_config *machine_config_find_speaker(machine_config *machine, const char *tag)
{
	int speakernum;

	for (speakernum = 0; speakernum < MAX_SPEAKER; speakernum++)
		if (machine->speaker[speakernum].tag && strcmp(machine->speaker[speakernum].tag, tag) == 0)
			return &machine->speaker[speakernum];

	fatalerror("Can't find speaker '%s'!\n", tag);
	return NULL;
}


/*-------------------------------------------------
    machine_config_remove_speaker - remove a tagged speaker
    system during machine driver expansion
-------------------------------------------------*/

void machine_config_remove_speaker(machine_config *machine, const char *tag)
{
	int speakernum;

	for (speakernum = 0; speakernum < MAX_SPEAKER; speakernum++)
		if (machine->speaker[speakernum].tag && strcmp(machine->speaker[speakernum].tag, tag) == 0)
		{
			memmove(&machine->speaker[speakernum], &machine->speaker[speakernum + 1], sizeof(machine->speaker[0]) * (MAX_SPEAKER - speakernum - 1));
			memset(&machine->speaker[MAX_SPEAKER - 1], 0, sizeof(machine->speaker[0]));
			return;
		}

	fatalerror("Can't find speaker '%s'!\n", tag);
}


/*-------------------------------------------------
    machine_config_add_sound - add a sound system during
    machine driver expansion
-------------------------------------------------*/

sound_config *machine_config_add_sound(machine_config *machine, const char *tag, sound_type type, int clock)
{
	int soundnum;

	for (soundnum = 0; soundnum < MAX_SOUND; soundnum++)
		if (machine->sound[soundnum].type == SOUND_DUMMY)
		{
			machine->sound[soundnum].tag = tag;
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
    machine_config_find_sound - find a tagged sound
    system during machine driver expansion
-------------------------------------------------*/

sound_config *machine_config_find_sound(machine_config *machine, const char *tag)
{
	int soundnum;

	for (soundnum = 0; soundnum < MAX_SOUND; soundnum++)
		if (machine->sound[soundnum].tag && strcmp(machine->sound[soundnum].tag, tag) == 0)
			return &machine->sound[soundnum];

	fatalerror("Can't find sound '%s'!\n", tag);
	return NULL;
}


/*-------------------------------------------------
    machine_config_remove_sound - remove a tagged sound
    system during machine driver expansion
-------------------------------------------------*/

void machine_config_remove_sound(machine_config *machine, const char *tag)
{
	int soundnum;

	for (soundnum = 0; soundnum < MAX_SOUND; soundnum++)
		if (machine->sound[soundnum].tag && strcmp(machine->sound[soundnum].tag, tag) == 0)
		{
			memmove(&machine->sound[soundnum], &machine->sound[soundnum + 1], sizeof(machine->sound[0]) * (MAX_SOUND - soundnum - 1));
			memset(&machine->sound[MAX_SOUND - 1], 0, sizeof(machine->sound[0]));
			return;
		}

	fatalerror("Can't find sound '%s'!\n", tag);
}


/*-------------------------------------------------
    machine_config_add_screen - add a screen during
    machine driver expansion
-------------------------------------------------*/

screen_config *machine_config_add_screen(machine_config *machine, const char *tag, int palbase)
{
	int screennum;

	for (screennum = 0; screennum < MAX_SCREENS; screennum++)
		if (machine->screen[screennum].tag == NULL)
		{
			machine->screen[screennum].tag = tag;
			machine->screen[screennum].palette_base = palbase;
			return &machine->screen[screennum];
		}

	fatalerror("Out of screens!\n");
	return NULL;
}


/*-------------------------------------------------
    machine_config_find_screen - find a tagged screen
    during machine driver expansion
-------------------------------------------------*/

screen_config *machine_config_find_screen(machine_config *machine, const char *tag)
{
	int screennum;

	for (screennum = 0; screennum < MAX_SCREENS; screennum++)
		if (machine->screen[screennum].tag && strcmp(machine->screen[screennum].tag, tag) == 0)
			return &machine->screen[screennum];

	fatalerror("Can't find screen '%s'!\n", tag);
	return NULL;
}


/*-------------------------------------------------
    machine_config_remove_screen - remove a tagged screen
    during machine driver expansion
-------------------------------------------------*/

void machine_config_remove_screen(machine_config *machine, const char *tag)
{
	int screennum;

	for (screennum = 0; screennum < MAX_SCREENS; screennum++)
		if (machine->screen[screennum].tag && strcmp(machine->screen[screennum].tag, tag) == 0)
		{
			memmove(&machine->screen[screennum], &machine->screen[screennum + 1], sizeof(machine->screen[0]) * (MAX_SCREENS - screennum - 1));
			memset(&machine->screen[MAX_SCREENS - 1], 0, sizeof(machine->screen[0]));
			return;
		}

	fatalerror("Can't find screen '%s'!\n", tag);
}
