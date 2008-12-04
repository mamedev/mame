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

/* Enum listing all the sound chips */
enum _sound_type
{
	SOUND_DUMMY,
	SOUND_CUSTOM,
	SOUND_SAMPLES,
	SOUND_DAC,
	SOUND_DMADAC,
	SOUND_DISCRETE,
	SOUND_AY8910,
	SOUND_AY8912,
	SOUND_AY8913,
	SOUND_AY8930,
	SOUND_YM2149,
	SOUND_YM3439,
	SOUND_YMZ284,
	SOUND_YMZ294,
	SOUND_YM2203,
	SOUND_YM2151,
	SOUND_YM2608,
	SOUND_YM2610,
	SOUND_YM2610B,
	SOUND_YM2612,
	SOUND_YM3438,
	SOUND_YM2413,
	SOUND_YM3812,
	SOUND_YM3526,
	SOUND_YMZ280B,
	SOUND_Y8950,
	SOUND_SN76477,
	SOUND_SN76489,
	SOUND_SN76489A,
	SOUND_SN76494,
	SOUND_SN76496,
	SOUND_GAMEGEAR,
	SOUND_SMSIII,
	SOUND_POKEY,
	SOUND_NES,
	SOUND_ASTROCADE,
	SOUND_NAMCO,
	SOUND_NAMCO_15XX,
	SOUND_NAMCO_CUS30,
	SOUND_NAMCO_52XX,
	SOUND_NAMCO_63701X,
	SOUND_SNKWAVE,
	SOUND_TMS36XX,
	SOUND_TMS3615,
	SOUND_TMS5100,
	SOUND_TMS5110,
	SOUND_TMS5110A,
	SOUND_CD2801,
	SOUND_TMC0281,
	SOUND_CD2802,
	SOUND_M58817,
	SOUND_TMC0285,
	SOUND_TMS5200,
	SOUND_TMS5220,
	SOUND_VLM5030,
	SOUND_OKIM6295,
	SOUND_OKIM6258,
	SOUND_MSM5205,
	SOUND_MSM5232,
	SOUND_UPD7759,
	SOUND_HC55516,
	SOUND_MC3417,
	SOUND_MC3418,
	SOUND_K005289,
	SOUND_K007232,
	SOUND_K051649,
	SOUND_K053260,
	SOUND_K054539,
	SOUND_SEGAPCM,
	SOUND_RF5C68,
	SOUND_CEM3394,
	SOUND_C140,
	SOUND_QSOUND,
	SOUND_SAA1099,
	SOUND_IREMGA20,
	SOUND_ES5503,
	SOUND_ES5505,
	SOUND_ES5506,
	SOUND_BSMT2000,
	SOUND_YMF262,
	SOUND_YMF278B,
	SOUND_GAELCO_CG1V,
	SOUND_GAELCO_GAE1,
	SOUND_X1_010,
	SOUND_MULTIPCM,
	SOUND_C6280,
	SOUND_TIA,
	SOUND_SP0250,
	SOUND_SCSP,
	SOUND_PSXSPU,
	SOUND_YMF271,
	SOUND_CDDA,
	SOUND_ICS2115,
	SOUND_ST0016,
	SOUND_NILE,
	SOUND_C352,
	SOUND_VRENDER0,
	SOUND_VOTRAX,
	SOUND_ES8712,
	SOUND_RF5C400,
	SOUND_SPEAKER,
	SOUND_CDP1869,
	SOUND_BEEP,
	SOUND_WAVE,
	SOUND_SID6581,
	SOUND_SID8580,
	SOUND_SP0256,
	SOUND_S14001A,
	SOUND_AICA,

	/* filters start here */
	SOUND_FILTER_VOLUME,
	SOUND_FILTER_RC,
	SOUND_FILTER_LOWPASS,

	SOUND_COUNT
};
typedef enum _sound_type sound_type;


/* Sound information constants */
enum
{
	/* --- the following bits of info are returned as 64-bit signed integers --- */
	SNDINFO_INT_FIRST = 0x00000,

	SNDINFO_INT_ALIAS = SNDINFO_INT_FIRST,				/* R/O: alias to sound type for (type,index) identification */

	SNDINFO_INT_CORE_SPECIFIC = 0x08000,				/* R/W: core-specific values start here */

	/* --- the following bits of info are returned as pointers to data or functions --- */
	SNDINFO_PTR_FIRST = 0x10000,

	SNDINFO_PTR_SET_INFO = SNDINFO_PTR_FIRST,			/* R/O: void (*set_info)(void *token, UINT32 state, sndinfo *info) */
	SNDINFO_PTR_START,									/* R/O: void *(*start)(int index, int clock, const void *config) */
	SNDINFO_PTR_STOP,									/* R/O: void (*stop)(void *token) */
	SNDINFO_PTR_RESET,									/* R/O: void (*reset)(void *token) */

	SNDINFO_PTR_CORE_SPECIFIC = 0x18000,				/* R/W: core-specific values start here */

	/* --- the following bits of info are returned as NULL-terminated strings --- */
	SNDINFO_STR_FIRST = 0x20000,

	SNDINFO_STR_NAME = SNDINFO_STR_FIRST,				/* R/O: name of the sound chip */
	SNDINFO_STR_CORE_FAMILY,							/* R/O: family of the sound chip */
	SNDINFO_STR_CORE_VERSION,							/* R/O: version of the sound core */
	SNDINFO_STR_CORE_FILE,								/* R/O: file containing the sound core */
	SNDINFO_STR_CORE_CREDITS,							/* R/O: credits for the sound core */

	SNDINFO_STR_CORE_SPECIFIC = 0x28000					/* R/W: core-specific values start here */
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
#define SND_START(name)				void *SND_START_NAME(name)(const device_config *device, const char *tag, int sndindex, int clock, const void *config)
#define SND_START_CALL(name)		SND_START_NAME(name)(device, tag, sndindex, clock, config)

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
typedef void *(*snd_start_func)(const device_config *device, const char *tag, int sndindex, int clock, const void *config);
typedef void (*snd_stop_func)(const device_config *device);
typedef void (*snd_reset_func)(const device_config *device);


/* sndinfo union used to pass data to/from the get_info/set_info functions */
union _sndinfo
{
	INT64	i;											/* generic integers */
	void *	p;											/* generic pointers */
	genf *  f;											/* generic function pointers */
	const char *s;										/* generic strings */

	snd_set_info_func	set_info;						/* SNDINFO_PTR_SET_INFO */
	snd_start_func		start;							/* SNDINFO_PTR_START */
	snd_stop_func		stop;							/* SNDINFO_PTR_STOP */
	snd_reset_func		reset;							/* SNDINFO_PTR_RESET */
};

typedef struct _snd_class_header snd_class_header;
struct _snd_class_header
{
	int						index;					/* index of this SND */
	sound_type				sndtype; 				/* type index of this SND */

	/* table of core functions */
	snd_get_info_func	get_info;
	snd_set_info_func	set_info;
	snd_start_func		start;
	snd_stop_func		stop;
	snd_reset_func		reset;
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

/* set info accessors */
void sndti_set_info_int(sound_type sndtype, int sndindex, UINT32 state, INT64 data);
void sndti_set_info_ptr(sound_type sndtype, int sndindex, UINT32 state, void *data);
void sndti_set_info_fct(sound_type sndtype, int sndindex, UINT32 state, genf *data);

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
int sndintrf_init_sound(int sndnum, const char *tag, sound_type sndtype, int clock, const void *config);
void sndintrf_exit_sound(int sndnum);
void sndintrf_register_token(void *token);

/* Misc helpers */
int sndti_exists(sound_type sndtype, int sndindex);
int sndti_to_sndnum(sound_type type, int index);
sound_type sndnum_to_sndti(int sndnum, int *index);
int sndtype_count(sound_type sndtype);


#endif	/* __SNDINTRF_H__ */
