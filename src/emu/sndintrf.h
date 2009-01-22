/***************************************************************************

    sndintrf.h

    Core sound interface functions and definitions.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __SNDINTRF_H__
#define __SNDINTRF_H__

#include "memory.h"
#include "mame.h"
#include "state.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

#define MAX_SOUND 32


/* Sound information constants */
enum
{
	/* --- the following bits of info are returned as 64-bit signed integers --- */
	SNDINFO_INT_FIRST = DEVINFO_INT_FIRST,

		SNDINFO_INT_TOKEN_BYTES = DEVINFO_INT_TOKEN_BYTES,	/* R/O: bytes to allocate for the token */

	SNDINFO_INT_CORE_SPECIFIC = DEVINFO_INT_DEVICE_SPECIFIC, /* R/W: core-specific values start here */

	/* --- the following bits of info are returned as pointers to data or functions --- */
	SNDINFO_PTR_FIRST = DEVINFO_PTR_FIRST,

	SNDINFO_PTR_CORE_SPECIFIC = DEVINFO_PTR_DEVICE_SPECIFIC, /* R/W: core-specific values start here */

	/* --- the following bits of info are returned as pointers to functions --- */
	SNDINFO_FCT_FIRST = DEVINFO_FCT_FIRST,

		SNDINFO_PTR_START = DEVINFO_FCT_START,				/* R/O: void *(*start)(const device_config *device, int clock) */
		SNDINFO_PTR_STOP = DEVINFO_FCT_STOP,				/* R/O: void (*stop)(const device_config *device) */
		SNDINFO_PTR_RESET = DEVINFO_FCT_RESET,				/* R/O: void (*reset)(const device_config *device) */

		SNDINFO_PTR_SET_INFO = DEVINFO_FCT_CLASS_SPECIFIC,	/* R/O: void (*set_info)(const device_config *device, UINT32 state, sndinfo *info) */
		SNDINFO_FCT_ALIAS,									/* R/O: alias to sound type for (type,index) identification */

	SNDINFO_FCT_CORE_SPECIFIC = DEVINFO_FCT_DEVICE_SPECIFIC, /* R/W: core-specific values start here */

	/* --- the following bits of info are returned as NULL-terminated strings --- */
	SNDINFO_STR_FIRST = DEVINFO_STR_FIRST,

		SNDINFO_STR_NAME = DEVINFO_STR_NAME,				/* R/O: name of the sound chip */
		SNDINFO_STR_CORE_FAMILY = DEVINFO_STR_FAMILY,		/* R/O: family of the sound chip */
		SNDINFO_STR_CORE_VERSION = DEVINFO_STR_VERSION,		/* R/O: version of the sound core */
		SNDINFO_STR_CORE_FILE = DEVINFO_STR_SOURCE_FILE,	/* R/O: file containing the sound core */
		SNDINFO_STR_CORE_CREDITS = DEVINFO_STR_CREDITS,		/* R/O: credits for the sound core */

	SNDINFO_STR_CORE_SPECIFIC = DEVINFO_STR_DEVICE_SPECIFIC	/* R/W: core-specific values start here */
};



/***************************************************************************
    MACROS
***************************************************************************/

#define SND_GET_INFO_NAME(name)		snd_get_info_##name
#define SND_GET_INFO(name)			void SND_GET_INFO_NAME(name)(const device_config *device, UINT32 state, sndinfo *info)
#define SND_GET_INFO_CALL(name)		SND_GET_INFO_NAME(name)(device, state, info)

#define SND_SET_INFO_NAME(name)		snd_set_info_##name
#define SND_SET_INFO(name)			void SND_SET_INFO_NAME(name)(const device_config *device, UINT32 state, sndinfo *info)
#define SND_SET_INFO_CALL(name)		SND_SET_INFO_NAME(name)(device, state, info)

#define SND_START_NAME(name)		snd_start_##name
#define SND_START(name)				void SND_START_NAME(name)(const device_config *device, int clock)
#define SND_START_CALL(name)		SND_START_NAME(name)(device, clock)

#define SND_STOP_NAME(name)			snd_stop_##name
#define SND_STOP(name)				void SND_STOP_NAME(name)(const device_config *device)
#define SND_STOP_CALL(name)			SND_STOP_NAME(name)(device)

#define SND_RESET_NAME(name)		snd_reset_##name
#define SND_RESET(name)				void SND_RESET_NAME(name)(const device_config *device)
#define SND_RESET_CALL(name)		SND_RESET_NAME(name)(device)



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* forward declaration of this union */
typedef union _sndinfo sndinfo;

/* define the various callback functions */
typedef void (*snd_get_info_func)(const device_config *device, UINT32 state, sndinfo *info);
typedef void (*snd_set_info_func)(const device_config *device, UINT32 state, sndinfo *info);
typedef void (*snd_start_func)(const device_config *device, int clock);
typedef void (*snd_stop_func)(const device_config *device);
typedef void (*snd_reset_func)(const device_config *device);

typedef snd_get_info_func sound_type;


/* sndinfo union used to pass data to/from the get_info/set_info functions */
union _sndinfo
{
	INT64	i;											/* generic integers */
	void *	p;											/* generic pointers */
	genf *  f;											/* generic function pointers */
	char *	s;											/* generic strings */
	sound_type type;									/* generic type */

	snd_set_info_func	set_info;						/* SNDINFO_PTR_SET_INFO */
	snd_start_func		start;							/* SNDINFO_PTR_START */
	snd_stop_func		stop;							/* SNDINFO_PTR_STOP */
	snd_reset_func		reset;							/* SNDINFO_PTR_RESET */
};



/***************************************************************************
    CHIP INTERFACES BY INDEX
***************************************************************************/

/* get info accessors */
INT64 sndnum_get_info_int(int sndnum, UINT32 state);
void *sndnum_get_info_ptr(int sndnum, UINT32 state);
genf *sndnum_get_info_fct(int sndnum, UINT32 state);
const char *sndnum_get_info_string(int sndnum, UINT32 state);

/* set info accessors */
void sndnum_set_info_int(int sndnum, UINT32 state, INT64 data);
void sndnum_set_info_ptr(int sndnum, UINT32 state, void *data);
void sndnum_set_info_fct(int sndnum, UINT32 state, genf *data);

#define sndnum_get_name(sndnum)					sndnum_get_info_string(sndnum, SNDINFO_STR_NAME)
#define sndnum_get_core_family(sndnum)			sndnum_get_info_string(sndnum, SNDINFO_STR_CORE_FAMILY)
#define sndnum_get_core_version(sndnum)			sndnum_get_info_string(sndnum, SNDINFO_STR_CORE_VERSION)
#define sndnum_get_core_file(sndnum)			sndnum_get_info_string(sndnum, SNDINFO_STR_CORE_FILE)
#define sndnum_get_core_credits(sndnum)			sndnum_get_info_string(sndnum, SNDINFO_STR_CORE_CREDITS)

/* misc accessors */
void sndnum_reset(int sndnum);
int sndnum_clock(int sndnum);
void *sndnum_token(int sndnum);



/***************************************************************************
    CHIP INTERFACES BY (TYPE,INDEX) PAIR
***************************************************************************/

/* get info accessors */
INT64 sndti_get_info_int(sound_type sndtype, int sndindex, UINT32 state);
void *sndti_get_info_ptr(sound_type sndtype, int sndindex, UINT32 state);
genf *sndti_get_info_fct(sound_type sndtype, int sndindex, UINT32 state);
const char *sndti_get_info_string(sound_type sndtype, int sndindex, UINT32 state);

#define sndti_get_name(sndtype, sndindex)			sndti_get_info_string(sndtype, sndindex, SNDINFO_STR_NAME)
#define sndti_get_core_family(sndtype, sndindex)	sndti_get_info_string(sndtype, sndindex, SNDINFO_STR_CORE_FAMILY)
#define sndti_get_core_version(sndtype, sndindex)	sndti_get_info_string(sndtype, sndindex, SNDINFO_STR_CORE_VERSION)
#define sndti_get_core_file(sndtype, sndindex)		sndti_get_info_string(sndtype, sndindex, SNDINFO_STR_CORE_FILE)
#define sndti_get_core_credits(sndtype, sndindex)	sndti_get_info_string(sndtype, sndindex, SNDINFO_STR_CORE_CREDITS)

/* misc accessors */
void sndti_reset(sound_type sndtype, int sndindex);
int sndti_clock(sound_type sndtype, int sndindex);
void *sndti_token(sound_type sndtype, int sndindex);

/* driver gain controls on chip outputs */
void sndti_set_output_gain(sound_type sndtype, int sndindex, int output, float gain);



/***************************************************************************
    CHIP INTERFACES BY TYPE
***************************************************************************/

/* get info accessors */
INT64 sndtype_get_info_int(sound_type sndtype, UINT32 state);
void *sndtype_get_info_ptr(sound_type sndtype, UINT32 state);
genf *sndtype_get_info_fct(sound_type sndtype, UINT32 state);
const char *sndtype_get_info_string(sound_type sndtype, UINT32 state);

#define sndtype_get_name(sndtype)					sndtype_get_info_string(sndtype, SNDINFO_STR_NAME)
#define sndtype_get_core_family(sndtype)			sndtype_get_info_string(sndtype, SNDINFO_STR_CORE_FAMILY)
#define sndtype_get_core_version(sndtype)			sndtype_get_info_string(sndtype, SNDINFO_STR_CORE_VERSION)
#define sndtype_get_core_file(sndtype)				sndtype_get_info_string(sndtype, SNDINFO_STR_CORE_FILE)
#define sndtype_get_core_credits(sndtype)			sndtype_get_info_string(sndtype, SNDINFO_STR_CORE_CREDITS)



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* Initialization/Tear down */
void sndintrf_init(running_machine *machine);
int sndintrf_init_sound(running_machine *machine, int sndnum, const char *tag, sound_type sndtype, int clock, const void *config);
void sndintrf_exit_sound(int sndnum);

/* Misc helpers */
int sndti_exists(sound_type sndtype, int sndindex);
int sndti_to_sndnum(sound_type type, int index);
sound_type sndnum_to_sndti(int sndnum, int *index);
int sndtype_count(sound_type sndtype);

#define SOUND_DUMMY NULL


#endif	/* __SNDINTRF_H__ */
