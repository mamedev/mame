/***************************************************************************

    sndintrf.c

    Core sound interface functions and definitions.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

****************************************************************************

    Still to do:
        * fix drivers that used to use ADPCM
        * many cores do their own resampling; they should stop
        * many cores mix to a separate buffer; no longer necessary

***************************************************************************/

#include "driver.h"



/***************************************************************************
    DEBUGGING
***************************************************************************/

#define VERBOSE			(0)

#define VPRINTF(x) do { if (VERBOSE) mame_printf_debug x; } while (0)



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _sndintrf_data sndintrf_data;
struct _sndintrf_data
{
	snd_class_header	intf;	 		/* copy of the interface data */
	sound_type		sndtype; 		/* type index of this sound chip */
	sound_type		aliastype;		/* aliased type index of this sound chip */
	device_config *	device;			/* dummy device for now */
	int				index; 			/* index of this sound chip */
};



/***************************************************************************
    VALIDATION MACROS
***************************************************************************/

#define VERIFY_SNDNUM(name) \
	assert_always(sndnum >= 0 && sndnum < totalsnd, #name "() called with invalid sound num!")

#define VERIFY_SNDTI(name) \
	int sndnum = sndti_to_sndnum(sndtype, sndindex); \
	assert_always(sndindex >= 0 && sndindex < totalsnd && sndnum != -1, #name "() called with invalid (type,index) pair!")

#define VERIFY_SNDTYPE(name) \



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

static sndintrf_data sound[MAX_SOUND];
static sndintrf_data *current_sound_start;
static int totalsnd;



/***************************************************************************
    INITIALIZATION
***************************************************************************/

/*-------------------------------------------------
    sndintrf_init - discover all linked sound
    systems and build a matrix for sound
    (type,index) pairs for the current machine
-------------------------------------------------*/

void sndintrf_init(running_machine *machine)
{
	/* zap the sound data structures */
	memset(sound, 0, sizeof(sound));
	totalsnd = 0;
}


/*-------------------------------------------------
    sndintrf_init_sound - initialize data for a
    particular sndnum
-------------------------------------------------*/

static DEVICE_GET_INFO( sndclass )
{
	sndintrf_data *snddata = device->inline_config;
	(*snddata->intf.get_info)(device, state, (sndinfo *)info);
}

int sndintrf_init_sound(running_machine *machine, int sndnum, const char *tag, sound_type sndtype, int clock, const void *config)
{
	snd_start_func start;
	sndintrf_data *info = &sound[sndnum];
	int index;
	int sndidx;

	info->device = auto_malloc(sizeof(*info->device) + strlen(tag));
	memset(info->device, 0, sizeof(*info->device) + strlen(tag));
	info->device->type = DEVICE_GET_INFO_NAME(sndclass);
	info->device->machine = machine;
	strcpy(info->device->tag, tag);
	info->device->static_config = config;
	info->device->region = memory_region(info->device->machine, info->device->tag);
	info->device->regionbytes = memory_region_length(info->device->machine, info->device->tag);

	/* hack: stash the info pointer in the inline_config */
	info->device->inline_config = info;

	/* fill in the type and interface */
	info->intf.get_info = sndtype;
	info->intf.set_info = (snd_set_info_func)sndtype_get_info_fct(sndtype, SNDINFO_PTR_SET_INFO);
	info->sndtype = sndtype;
	info->aliastype = (sound_type)sndtype_get_info_fct(sndtype, SNDINFO_FCT_ALIAS);
	if (info->aliastype == 0)
		info->aliastype = sndtype;
	info->device->clock = clock;

	/* compute the index */
	for (index = 0, sndidx = 0; sndidx < totalsnd; sndidx++)
		if (sound[sndidx].aliastype == info->aliastype)
			index++;
	info->index = index;
	totalsnd++;

	/* start the chip, tagging all its streams */
	current_sound_start = &sound[sndnum];
	start = (snd_start_func)sndtype_get_info_fct(sndtype, SNDINFO_PTR_START);
	info->device->token = (*start)(info->device, clock);
	current_sound_start = NULL;
	VPRINTF(("  token = %p\n", info->device->token));

	/* if that failed, die */
	if (info->device->token == NULL)
		return 1;

	return 0;
}


/*-------------------------------------------------
    sndintrf_exit_sound - tear down data for a
    particular sndnum
-------------------------------------------------*/

void sndintrf_exit_sound(int sndnum)
{
	sndinfo info;

	info.stop = NULL;
	(*sound[sndnum].intf.get_info)(NULL, SNDINFO_PTR_STOP, &info);

	/* stop the chip */
	if (info.stop)
		(*info.stop)(sound[sndnum].device);
}



/***************************************************************************
    HELPERS
***************************************************************************/

/*-------------------------------------------------
    sndintrf_register_token - register a token
    from within the sound_start routine
-------------------------------------------------*/

void sndintrf_register_token(void *token)
{
	if (current_sound_start)
		current_sound_start->device->token = token;
}


/*-------------------------------------------------
    sndti_exists - return TRUE if a (type,index)
    pair describes an existing chip
-------------------------------------------------*/

int sndti_exists(sound_type type, int index)
{
	return (sndti_to_sndnum(type, index) != -1);
}


/*-------------------------------------------------
    sndti_to_sndnum - map a (type,index) pair to
    a sound number
-------------------------------------------------*/

int sndti_to_sndnum(sound_type type, int index)
{
	int sndnum;
	for (sndnum = 0; sndnum < totalsnd; sndnum++)
		if (sound[sndnum].aliastype == type)
			if (index-- == 0)
				return sndnum;
	return -1;
}


/*-------------------------------------------------
    sndnum_to_sndti - map a sound number to a
    (type,index) pair
-------------------------------------------------*/

sound_type sndnum_to_sndti(int sndnum, int *index)
{
	if (index != NULL)
		*index = sound[sndnum].index;
	return sound[sndnum].aliastype;
}


/*-------------------------------------------------
    sndtype_count - count the number of a
    given type
-------------------------------------------------*/

int sndtype_count(sound_type sndtype)
{
	int index;
	int count = 0;

	for (index = 0; index < totalsnd; index++)
		if (sound[index].sndtype == sndtype)
			count++;

	return count;
}



/***************************************************************************
    CHIP INTERFACES BY INDEX
***************************************************************************/

/*-------------------------------------------------
    Get info accessors
-------------------------------------------------*/

INT64 sndnum_get_info_int(int sndnum, UINT32 state)
{
	sndinfo info;

	VERIFY_SNDNUM(sndnum_get_info_int);
	info.i = 0;
	(*sound[sndnum].intf.get_info)(sound[sndnum].device, state, &info);
	return info.i;
}

void *sndnum_get_info_ptr(int sndnum, UINT32 state)
{
	sndinfo info;

	VERIFY_SNDNUM(sndnum_get_info_ptr);
	info.p = NULL;
	(*sound[sndnum].intf.get_info)(sound[sndnum].device, state, &info);
	return info.p;
}

genf *sndnum_get_info_fct(int sndnum, UINT32 state)
{
	sndinfo info;

	VERIFY_SNDNUM(sndnum_get_info_fct);
	info.f = NULL;
	(*sound[sndnum].intf.get_info)(sound[sndnum].device, state, &info);
	return info.f;
}

const char *sndnum_get_info_string(int sndnum, UINT32 state)
{
	extern char *get_temp_string_buffer(void);
	sndinfo info;

	VERIFY_SNDNUM(sndnum_get_info_string);
	info.s = get_temp_string_buffer();
	(*sound[sndnum].intf.get_info)(sound[sndnum].device, state, &info);
	return info.s;
}


/*-------------------------------------------------
    Set info accessors
-------------------------------------------------*/

void sndnum_set_info_int(int sndnum, UINT32 state, INT64 data)
{
	sndinfo info;
	VERIFY_SNDNUM(sndnum_set_info_int);
	info.i = data;
	(*sound[sndnum].intf.set_info)(sound[sndnum].device, state, &info);
}

void sndnum_set_info_ptr(int sndnum, UINT32 state, void *data)
{
	sndinfo info;
	VERIFY_SNDNUM(sndnum_set_info_ptr);
	info.p = data;
	(*sound[sndnum].intf.set_info)(sound[sndnum].device, state, &info);
}

void sndnum_set_info_fct(int sndnum, UINT32 state, genf *data)
{
	sndinfo info;
	VERIFY_SNDNUM(sndnum_set_info_ptr);
	info.f = data;
	(*sound[sndnum].intf.set_info)(sound[sndnum].device, state, &info);
}


/*-------------------------------------------------
    Misc accessors
-------------------------------------------------*/

void sndnum_reset(int sndnum)
{
	sndinfo info;

	VERIFY_SNDNUM(sndnum_reset);
	info.reset = NULL;
	(*sound[sndnum].intf.get_info)(NULL, SNDINFO_PTR_RESET, &info);
	if (info.reset)
		(info.reset)(sound[sndnum].device);
}

int sndnum_clock(int sndnum)
{
	VERIFY_SNDNUM(sndnum_clock);
	return sound[sndnum].device->clock;
}

void *sndnum_token(int sndnum)
{
	VERIFY_SNDNUM(sndnum_token);
	return sound[sndnum].device->token;
}



/***************************************************************************
    CHIP INTERFACES BY (TYPE,INDEX) PAIR
***************************************************************************/

/*-------------------------------------------------
    Get info accessors
-------------------------------------------------*/

INT64 sndti_get_info_int(sound_type sndtype, int sndindex, UINT32 state)
{
	sndinfo info;

	VERIFY_SNDTI(sndti_get_info_int);
	info.i = 0;
	(*sound[sndnum].intf.get_info)(sound[sndnum].device, state, &info);
	return info.i;
}

void *sndti_get_info_ptr(sound_type sndtype, int sndindex, UINT32 state)
{
	sndinfo info;

	VERIFY_SNDTI(sndti_get_info_ptr);
	info.p = NULL;
	(*sound[sndnum].intf.get_info)(sound[sndnum].device, state, &info);
	return info.p;
}

genf *sndti_get_info_fct(sound_type sndtype, int sndindex, UINT32 state)
{
	sndinfo info;

	VERIFY_SNDTI(sndti_get_info_fct);
	info.f = NULL;
	(*sound[sndnum].intf.get_info)(sound[sndnum].device, state, &info);
	return info.f;
}

const char *sndti_get_info_string(sound_type sndtype, int sndindex, UINT32 state)
{
	extern char *get_temp_string_buffer(void);
	sndinfo info;

	VERIFY_SNDTI(sndti_get_info_string);
	info.s = get_temp_string_buffer();
	(*sound[sndnum].intf.get_info)(sound[sndnum].device, state, &info);
	return info.s;
}


/*-------------------------------------------------
    Set info accessors
-------------------------------------------------*/

void sndti_set_info_int(sound_type sndtype, int sndindex, UINT32 state, INT64 data)
{
	sndinfo info;

	VERIFY_SNDTI(sndti_set_info_int);
	info.i = data;
	(*sound[sndnum].intf.set_info)(sound[sndnum].device, state, &info);
}

void sndti_set_info_ptr(sound_type sndtype, int sndindex, UINT32 state, void *data)
{
	sndinfo info;

	VERIFY_SNDTI(sndti_set_info_ptr);
	info.p = data;
	(*sound[sndnum].intf.set_info)(sound[sndnum].device, state, &info);
}

void sndti_set_info_fct(sound_type sndtype, int sndindex, UINT32 state, genf *data)
{
	sndinfo info;

	VERIFY_SNDTI(sndti_set_info_ptr);
	info.f = data;
	(*sound[sndnum].intf.set_info)(sound[sndnum].device, state, &info);
}


/*-------------------------------------------------
    Misc accessors
-------------------------------------------------*/

void sndti_reset(sound_type sndtype, int sndindex)
{
	sndinfo info;

	VERIFY_SNDTI(sndti_reset);
	info.reset = NULL;
	(*sound[sndnum].intf.get_info)(NULL, SNDINFO_PTR_RESET, &info);

	if (info.reset)
		(*info.reset)(sound[sndnum].device);
}

int sndti_clock(sound_type sndtype, int sndindex)
{
	VERIFY_SNDTI(sndti_clock);
	return sound[sndnum].device->clock;
}

void *sndti_token(sound_type sndtype, int sndindex)
{
	VERIFY_SNDTI(sndti_token);
	return sound[sndnum].device->token;
}



/***************************************************************************
    CHIP INTERFACES BY TYPE
***************************************************************************/

/*-------------------------------------------------
    Get info accessors
-------------------------------------------------*/

INT64 sndtype_get_info_int(sound_type sndtype, UINT32 state)
{
	snd_get_info_func get_info = sndtype;
	sndinfo info;

	VERIFY_SNDTYPE(sndtype_get_info_int);
	info.i = 0;
	(*get_info)(NULL, state, &info);
	return info.i;
}

void *sndtype_get_info_ptr(sound_type sndtype, UINT32 state)
{
	snd_get_info_func get_info = sndtype;
	sndinfo info;

	VERIFY_SNDTYPE(sndtype_get_info_ptr);
	info.p = NULL;
	(*get_info)(NULL, state, &info);
	return info.p;
}

genf *sndtype_get_info_fct(sound_type sndtype, UINT32 state)
{
	snd_get_info_func get_info = sndtype;
	sndinfo info;

	VERIFY_SNDTYPE(sndtype_get_info_fct);
	info.f = NULL;
	(*get_info)(NULL, state, &info);
	return info.f;
}

const char *sndtype_get_info_string(sound_type sndtype, UINT32 state)
{
	extern char *get_temp_string_buffer(void);
	snd_get_info_func get_info = sndtype;
	sndinfo info;

	VERIFY_SNDTYPE(sndtype_get_info_string);
	info.s = get_temp_string_buffer();
	(*get_info)(NULL, state, &info);
	return info.s;
}
