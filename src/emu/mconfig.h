/***************************************************************************

    mconfig.h

    Machine configuration macros and functions.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __MCONFIG_H__
#define __MCONFIG_H__

#include "devintrf.h"
#include "cpuexec.h"
#include "sndintrf.h"
#include <stddef.h>


/***************************************************************************
    CONSTANTS
***************************************************************************/

/* by convention, tags should all lowercase and this length or less */
#define MAX_TAG_LENGTH			15


/* token types */
enum
{
	MCONFIG_TOKEN_INVALID,
	MCONFIG_TOKEN_END,
	MCONFIG_TOKEN_INCLUDE,

	MCONFIG_TOKEN_DEVICE_ADD,
	MCONFIG_TOKEN_DEVICE_REMOVE,
	MCONFIG_TOKEN_DEVICE_MODIFY,
	MCONFIG_TOKEN_DEVICE_CLOCK,
	MCONFIG_TOKEN_DEVICE_MAP,
	MCONFIG_TOKEN_DEVICE_CONFIG,
	MCONFIG_TOKEN_DEVICE_CONFIG_DATA32,
	MCONFIG_TOKEN_DEVICE_CONFIG_DATA64,
	MCONFIG_TOKEN_DEVICE_CONFIG_DATAFP32,
	MCONFIG_TOKEN_DEVICE_CONFIG_CUSTOM_1,
	MCONFIG_TOKEN_DEVICE_CONFIG_CUSTOM_2,
	MCONFIG_TOKEN_DEVICE_CONFIG_CUSTOM_3,
	MCONFIG_TOKEN_DEVICE_CONFIG_CUSTOM_4,
	MCONFIG_TOKEN_DEVICE_CONFIG_CUSTOM_5,
	MCONFIG_TOKEN_DEVICE_CONFIG_CUSTOM_6,
	MCONFIG_TOKEN_DEVICE_CONFIG_CUSTOM_7,
	MCONFIG_TOKEN_DEVICE_CONFIG_CUSTOM_8,
	MCONFIG_TOKEN_DEVICE_CONFIG_CUSTOM_9,
	MCONFIG_TOKEN_DEVICE_CONFIG_CUSTOM_FREE,

	MCONFIG_TOKEN_DRIVER_DATA,
	MCONFIG_TOKEN_QUANTUM_TIME,
	MCONFIG_TOKEN_QUANTUM_PERFECT_CPU,
	MCONFIG_TOKEN_WATCHDOG_VBLANK,
	MCONFIG_TOKEN_WATCHDOG_TIME,

	MCONFIG_TOKEN_MACHINE_START,
	MCONFIG_TOKEN_MACHINE_RESET,
	MCONFIG_TOKEN_NVRAM_HANDLER,
	MCONFIG_TOKEN_MEMCARD_HANDLER,

	MCONFIG_TOKEN_VIDEO_ATTRIBUTES,
	MCONFIG_TOKEN_GFXDECODE,
	MCONFIG_TOKEN_PALETTE_LENGTH,
	MCONFIG_TOKEN_DEFAULT_LAYOUT,

	MCONFIG_TOKEN_PALETTE_INIT,
	MCONFIG_TOKEN_VIDEO_START,
	MCONFIG_TOKEN_VIDEO_RESET,
	MCONFIG_TOKEN_VIDEO_EOF,
	MCONFIG_TOKEN_VIDEO_UPDATE,

	MCONFIG_TOKEN_SOUND_START,
	MCONFIG_TOKEN_SOUND_RESET,
};


/* ----- flags for video_attributes ----- */

/* should VIDEO_UPDATE by called at the start of VBLANK or at the end? */
#define	VIDEO_UPDATE_BEFORE_VBLANK		0x0000
#define	VIDEO_UPDATE_AFTER_VBLANK		0x0004

/* indicates VIDEO_UPDATE will add container bits its */
#define VIDEO_SELF_RENDER				0x0008

/* automatically extend the palette creating a darker copy for shadows */
#define VIDEO_HAS_SHADOWS				0x0010

/* automatically extend the palette creating a brighter copy for highlights */
#define VIDEO_HAS_HIGHLIGHTS			0x0020

/* Mish 181099:  See comments in video/generic.c for details */
#define VIDEO_BUFFERS_SPRITERAM			0x0040

/* force VIDEO_UPDATE to be called even for skipped frames */
#define VIDEO_ALWAYS_UPDATE				0x0080

/* calls VIDEO_UPDATE for every visible scanline, even for skipped frames */
#define VIDEO_UPDATE_SCANLINE			0x0100



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* In mamecore.h: typedef struct _machine_config machine_config; */
struct _machine_config
{
	UINT32					driver_data_size;		/* amount of memory needed for driver_data */

	attotime				minimum_quantum;		/* minimum scheduling quantum */
	const char *			perfect_cpu_quantum;	/* tag of CPU to use for "perfect" scheduling */
	INT32					watchdog_vblank_count;	/* number of VBLANKs until the watchdog kills us */
	attotime				watchdog_time;			/* length of time until the watchdog kills us */

	machine_start_func		machine_start;			/* one-time machine start callback */
	machine_reset_func		machine_reset;			/* machine reset callback */

	nvram_handler_func		nvram_handler;			/* NVRAM save/load callback  */
	memcard_handler_func	memcard_handler;		/* memory card save/load callback  */

	UINT32					video_attributes;		/* flags describing the video system */
	const gfx_decode_entry *gfxdecodeinfo;			/* pointer to array of graphics decoding information */
	UINT32					total_colors;			/* total number of colors in the palette */
	const char *			default_layout;			/* default layout for this machine */

	palette_init_func		init_palette;			/* one-time palette init callback  */
	video_start_func		video_start;			/* one-time video start callback */
	video_reset_func		video_reset;			/* video reset callback */
	video_eof_func			video_eof;				/* end-of-frame video callback */
	video_update_func 		video_update; 			/* video update callback */

	sound_start_func		sound_start;			/* one-time sound start callback */
	sound_reset_func		sound_reset;			/* sound reset callback */

	device_config *			devicelist;				/* list head for devices */
};



/***************************************************************************
    MACROS FOR BUILDING MACHINE DRIVERS
***************************************************************************/

/* this type is used to encode machine configuration definitions */
typedef union _machine_config_token machine_config_token;
union _machine_config_token
{
	TOKEN_COMMON_FIELDS
	const machine_config_token *tokenptr;
	const gfx_decode_entry *gfxdecode;
	const addrmap_token *addrmap;
	device_type devtype;
	driver_init_func driver_init;
	nvram_handler_func nvram_handler;
	memcard_handler_func memcard_handler;
	machine_start_func machine_start;
	machine_reset_func machine_reset;
	sound_start_func sound_start;
	sound_reset_func sound_reset;
	video_start_func video_start;
	video_reset_func video_reset;
	palette_init_func palette_init;
	video_eof_func video_eof;
	video_update_func video_update;
};


/* helper macro for returning the size of a field within a struct; similar to offsetof() */
#define structsizeof(_struct, _field) sizeof(((_struct *)NULL)->_field)


/* start/end tags for the machine driver */
#define MACHINE_DRIVER_NAME(_name) \
	machine_config_##_name

#define MACHINE_DRIVER_START(_name) \
	const machine_config_token MACHINE_DRIVER_NAME(_name)[] = {

#define MACHINE_DRIVER_END \
	TOKEN_UINT32_PACK1(MCONFIG_TOKEN_END, 8) };

/* use this to declare external references to a machine driver */
#define MACHINE_DRIVER_EXTERN(_name) \
	extern const machine_config_token MACHINE_DRIVER_NAME(_name)[]


/* importing data from other machine drivers */
#define MDRV_IMPORT_FROM(_name) \
	TOKEN_UINT32_PACK1(MCONFIG_TOKEN_INCLUDE, 8), \
	TOKEN_PTR(tokenptr, MACHINE_DRIVER_NAME(_name)),


/* core parameters */
#define MDRV_DRIVER_DATA(_struct) \
	TOKEN_UINT32_PACK2(MCONFIG_TOKEN_DRIVER_DATA, 8, sizeof(_struct), 24),

#define MDRV_QUANTUM_TIME(_time) \
	TOKEN_UINT32_PACK1(MCONFIG_TOKEN_QUANTUM_TIME, 8), \
	TOKEN_UINT64(UINT64_ATTOTIME_IN_##_time),

#define MDRV_QUANTUM_PERFECT_CPU(_cputag) \
	TOKEN_UINT32_PACK1(MCONFIG_TOKEN_QUANTUM_PERFECT_CPU, 8), \
	TOKEN_STRING(_cputag),

#define MDRV_WATCHDOG_VBLANK_INIT(_count) \
	TOKEN_UINT32_PACK2(MCONFIG_TOKEN_WATCHDOG_VBLANK, 8, _count, 24),

#define MDRV_WATCHDOG_TIME_INIT(_time) \
	TOKEN_UINT32_PACK1(MCONFIG_TOKEN_WATCHDOG_TIME, 8), \
	TOKEN_UINT64(UINT64_ATTOTIME_IN_##_time),


/* core functions */
#define MDRV_MACHINE_START(_func) \
	TOKEN_UINT32_PACK1(MCONFIG_TOKEN_MACHINE_START, 8), \
	TOKEN_PTR(machine_start, MACHINE_START_NAME(_func)),

#define MDRV_MACHINE_RESET(_func) \
	TOKEN_UINT32_PACK1(MCONFIG_TOKEN_MACHINE_RESET, 8), \
	TOKEN_PTR(machine_reset, MACHINE_RESET_NAME(_func)),

#define MDRV_NVRAM_HANDLER(_func) \
	TOKEN_UINT32_PACK1(MCONFIG_TOKEN_NVRAM_HANDLER, 8), \
	TOKEN_PTR(nvram_handler, NVRAM_HANDLER_NAME(_func)),

#define MDRV_MEMCARD_HANDLER(_func) \
	TOKEN_UINT32_PACK1(MCONFIG_TOKEN_MEMCARD_HANDLER, 8), \
	TOKEN_PTR(memcard_handler, MEMCARD_HANDLER_NAME(_func)),


/* core video parameters */
#define MDRV_VIDEO_ATTRIBUTES(_flags) \
	TOKEN_UINT32_PACK2(MCONFIG_TOKEN_VIDEO_ATTRIBUTES, 8, _flags, 24),

#define MDRV_GFXDECODE(_gfx) \
	TOKEN_UINT32_PACK1(MCONFIG_TOKEN_GFXDECODE, 8), \
	TOKEN_PTR(gfxdecode, GFXDECODE_NAME(_gfx)),

#define MDRV_PALETTE_LENGTH(_length) \
	TOKEN_UINT32_PACK2(MCONFIG_TOKEN_PALETTE_LENGTH, 8, _length, 24),

#define MDRV_DEFAULT_LAYOUT(_layout) \
	TOKEN_UINT32_PACK1(MCONFIG_TOKEN_DEFAULT_LAYOUT, 8), \
	TOKEN_STRING(&(_layout)[0]),


/* core video functions */
#define MDRV_PALETTE_INIT(_func) \
	TOKEN_UINT32_PACK1(MCONFIG_TOKEN_PALETTE_INIT, 8), \
	TOKEN_PTR(palette_init, PALETTE_INIT_NAME(_func)),

#define MDRV_VIDEO_START(_func) \
	TOKEN_UINT32_PACK1(MCONFIG_TOKEN_VIDEO_START, 8), \
	TOKEN_PTR(video_start, VIDEO_START_NAME(_func)),

#define MDRV_VIDEO_RESET(_func) \
	TOKEN_UINT32_PACK1(MCONFIG_TOKEN_VIDEO_RESET, 8), \
	TOKEN_PTR(video_reset, VIDEO_RESET_NAME(_func)),

#define MDRV_VIDEO_EOF(_func) \
	TOKEN_UINT32_PACK1(MCONFIG_TOKEN_VIDEO_EOF, 8), \
	TOKEN_PTR(video_eof, VIDEO_EOF_NAME(_func)),

#define MDRV_VIDEO_UPDATE(_func) \
	TOKEN_UINT32_PACK1(MCONFIG_TOKEN_VIDEO_UPDATE, 8), \
	TOKEN_PTR(video_update, VIDEO_UPDATE_NAME(_func)),


/* core sound functions */
#define MDRV_SOUND_START(_func) \
	TOKEN_UINT32_PACK1(MCONFIG_TOKEN_SOUND_START, 8), \
	TOKEN_PTR(sound_start, SOUND_START_NAME(_func)),

#define MDRV_SOUND_RESET(_func) \
	TOKEN_UINT32_PACK1(MCONFIG_TOKEN_SOUND_RESET, 8), \
	TOKEN_PTR(sound_start, SOUND_RESET_NAME(_func)),


/* add/remove devices */
#define MDRV_DEVICE_ADD(_tag, _type, _clock) \
	TOKEN_UINT64_PACK2(MCONFIG_TOKEN_DEVICE_ADD, 8, _clock, 32), \
	TOKEN_PTR(devtype, _type), \
	TOKEN_STRING(_tag),

#define MDRV_DEVICE_REMOVE(_tag) \
	TOKEN_UINT32_PACK1(MCONFIG_TOKEN_DEVICE_REMOVE, 8), \
	TOKEN_STRING(_tag),

#define MDRV_DEVICE_MODIFY(_tag)	\
	TOKEN_UINT32_PACK1(MCONFIG_TOKEN_DEVICE_MODIFY, 8), \
	TOKEN_STRING(_tag),


/* configure devices */
#define MDRV_DEVICE_CONFIG(_config) \
	TOKEN_UINT32_PACK1(MCONFIG_TOKEN_DEVICE_CONFIG, 8), \
	TOKEN_PTR(voidptr, &(_config)),

#define MDRV_DEVICE_CONFIG_CLEAR() \
	TOKEN_UINT32_PACK1(MCONFIG_TOKEN_DEVICE_CONFIG, 8), \
	TOKEN_PTR(voidptr, NULL),

#define MDRV_DEVICE_CLOCK(_clock) \
	TOKEN_UINT64_PACK2(MCONFIG_TOKEN_DEVICE_CLOCK, 8, _clock, 32),

#define MDRV_DEVICE_ADDRESS_MAP(_space, _map) \
	TOKEN_UINT32_PACK2(MCONFIG_TOKEN_DEVICE_MAP, 8, _space, 8), \
	TOKEN_PTR(addrmap, (const addrmap_token *)ADDRESS_MAP_NAME(_map)),

#define MDRV_DEVICE_PROGRAM_MAP(_map) \
	MDRV_DEVICE_ADDRESS_MAP(ADDRESS_SPACE_PROGRAM, _map)

#define MDRV_DEVICE_DATA_MAP(_map) \
	MDRV_DEVICE_ADDRESS_MAP(ADDRESS_SPACE_DATA, _map)

#define MDRV_DEVICE_IO_MAP(_map) \
	MDRV_DEVICE_ADDRESS_MAP(ADDRESS_SPACE_IO, _map)



/* inline device configurations that require 32 bits of storage in the token */
#define MDRV_DEVICE_CONFIG_DATA32_EXPLICIT(_size, _offset, _val) \
	TOKEN_UINT32_PACK3(MCONFIG_TOKEN_DEVICE_CONFIG_DATA32, 8, _size, 4, _offset, 12), \
	TOKEN_UINT32((UINT32)(_val)),

#define MDRV_DEVICE_CONFIG_DATA32(_struct, _field, _val) \
	MDRV_DEVICE_CONFIG_DATA32_EXPLICIT(structsizeof(_struct, _field), offsetof(_struct, _field), _val)

#define MDRV_DEVICE_CONFIG_DATA32_ARRAY(_struct, _field, _index, _val) \
	MDRV_DEVICE_CONFIG_DATA32_EXPLICIT(structsizeof(_struct, _field[0]), offsetof(_struct, _field) + (_index) * structsizeof(_struct, _field[0]), _val)

#define MDRV_DEVICE_CONFIG_DATA32_ARRAY_MEMBER(_struct, _field, _index, _memstruct, _member, _val) \
	MDRV_DEVICE_CONFIG_DATA32_EXPLICIT(structsizeof(_memstruct, _member), offsetof(_struct, _field) + (_index) * structsizeof(_struct, _field[0]) + offsetof(_memstruct, _member), _val)


/* inline device configurations that require 32 bits of fixed-point storage in the token */
#define MDRV_DEVICE_CONFIG_DATAFP32_EXPLICIT(_size, _offset, _val, _fixbits) \
	TOKEN_UINT32_PACK4(MCONFIG_TOKEN_DEVICE_CONFIG_DATAFP32, 8, _size, 4, _fixbits, 6, _offset, 12), \
	TOKEN_UINT32((INT32)((float)(_val) * (float)(1 << (_fixbits)))),

#define MDRV_DEVICE_CONFIG_DATAFP32(_struct, _field, _val, _fixbits) \
	MDRV_DEVICE_CONFIG_DATAFP32_EXPLICIT(structsizeof(_struct, _field), offsetof(_struct, _field), _val, _fixbits)

#define MDRV_DEVICE_CONFIG_DATAFP32_ARRAY(_struct, _field, _index, _val, _fixbits) \
	MDRV_DEVICE_CONFIG_DATAFP32_EXPLICIT(structsizeof(_struct, _field[0]), offsetof(_struct, _field) + (_index) * structsizeof(_struct, _field[0]), _val, _fixbits)

#define MDRV_DEVICE_CONFIG_DATAFP32_ARRAY_MEMBER(_struct, _field, _index, _memstruct, _member, _val, _fixbits) \
	MDRV_DEVICE_CONFIG_DATAFP32_EXPLICIT(structsizeof(_memstruct, _member), offsetof(_struct, _field) + (_index) * structsizeof(_struct, _field[0]) + offsetof(_memstruct, _member), _val, _fixbits)


/* inline device configurations that require 64 bits of storage in the token */
#define MDRV_DEVICE_CONFIG_DATA64_EXPLICIT(_size, _offset, _val) \
	TOKEN_UINT32_PACK3(MCONFIG_TOKEN_DEVICE_CONFIG_DATA64, 8, _size, 4, _offset, 12), \
	TOKEN_UINT64((UINT64)(_val)),

#define MDRV_DEVICE_CONFIG_DATA64(_struct, _field, _val) \
	MDRV_DEVICE_CONFIG_DATA64_EXPLICIT(structsizeof(_struct, _field), offsetof(_struct, _field), _val)

#define MDRV_DEVICE_CONFIG_DATA64_ARRAY(_struct, _field, _index, _val) \
	MDRV_DEVICE_CONFIG_DATA64_EXPLICIT(structsizeof(_struct, _field[0]), offsetof(_struct, _field) + (_index) * structsizeof(_struct, _field[0]), _val)

#define MDRV_DEVICE_CONFIG_DATA64_ARRAY_MEMBER(_struct, _field, _index, _memstruct, _member, _val) \
	MDRV_DEVICE_CONFIG_DATA64_EXPLICIT(structsizeof(_memstruct, _member), offsetof(_struct, _field) + (_index) * structsizeof(_struct, _field[0]) + offsetof(_memstruct, _member), _val)


/* inline device configurations that require a pointer-sized amount of storage in the token */
#ifdef PTR64
#define MDRV_DEVICE_CONFIG_DATAPTR_EXPLICIT(_struct, _size, _offset) MDRV_DEVICE_CONFIG_DATA64_EXPLICIT(_struct, _size, _offset)
#define MDRV_DEVICE_CONFIG_DATAPTR(_struct, _field, _val) MDRV_DEVICE_CONFIG_DATA64(_struct, _field, _val)
#define MDRV_DEVICE_CONFIG_DATAPTR_ARRAY(_struct, _field, _index, _val) MDRV_DEVICE_CONFIG_DATA64_ARRAY(_struct, _field, _index, _val)
#define MDRV_DEVICE_CONFIG_DATAPTR_ARRAY_MEMBER(_struct, _field, _index, _memstruct, _member, _val) MDRV_DEVICE_CONFIG_DATA64_ARRAY_MEMBER(_struct, _field, _index, _memstruct, _member, _val)
#else
#define MDRV_DEVICE_CONFIG_DATAPTR_EXPLICIT(_struct, _size, _offset) MDRV_DEVICE_CONFIG_DATA32_EXPLICIT(_struct, _size, _offset)
#define MDRV_DEVICE_CONFIG_DATAPTR(_struct, _field, _val) MDRV_DEVICE_CONFIG_DATA32(_struct, _field, _val)
#define MDRV_DEVICE_CONFIG_DATAPTR_ARRAY(_struct, _field, _index, _val) MDRV_DEVICE_CONFIG_DATA32_ARRAY(_struct, _field, _index, _val)
#define MDRV_DEVICE_CONFIG_DATAPTR_ARRAY_MEMBER(_struct, _field, _index, _memstruct, _member, _val) MDRV_DEVICE_CONFIG_DATA32_ARRAY_MEMBER(_struct, _field, _index, _memstruct, _member, _val)
#endif



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/


/* ----- machine configurations ----- */

/* allocate a new machine configuration and populate it using the supplied constructor */
machine_config *machine_config_alloc(const machine_config_token *tokens);

/* release memory allocated for a machine configuration */
void machine_config_free(machine_config *config);


#endif	/* __MCONFIG_H__ */
