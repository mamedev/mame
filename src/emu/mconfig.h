/***************************************************************************

    mconfig.h

    Machine configuration macros and functions.

****************************************************************************

    Copyright Aaron Giles
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name 'MAME' nor the names of its contributors may be
          used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

***************************************************************************/

#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef __MCONFIG_H__
#define __MCONFIG_H__


#define NVRAM_HANDLER_NAME(name)	nvram_handler_##name
#define NVRAM_HANDLER(name)			void NVRAM_HANDLER_NAME(name)(running_machine *machine, mame_file *file, int read_or_write)
#define NVRAM_HANDLER_CALL(name)	NVRAM_HANDLER_NAME(name)(machine, file, read_or_write)

#define MEMCARD_HANDLER_NAME(name)	memcard_handler_##name
#define MEMCARD_HANDLER(name)		void MEMCARD_HANDLER_NAME(name)(running_machine *machine, mame_file *file, int action)
#define MEMCARD_HANDLER_CALL(name)	MEMCARD_HANDLER_NAME(name)(machine, file, action)

#define MACHINE_START_NAME(name)	machine_start_##name
#define MACHINE_START(name)			void MACHINE_START_NAME(name)(running_machine *machine)
#define MACHINE_START_CALL(name)	MACHINE_START_NAME(name)(machine)

#define MACHINE_RESET_NAME(name)	machine_reset_##name
#define MACHINE_RESET(name)			void MACHINE_RESET_NAME(name)(running_machine *machine)
#define MACHINE_RESET_CALL(name)	MACHINE_RESET_NAME(name)(machine)

#define SOUND_START_NAME(name)		sound_start_##name
#define SOUND_START(name)			void SOUND_START_NAME(name)(running_machine *machine)
#define SOUND_START_CALL(name)		SOUND_START_NAME(name)(machine)

#define SOUND_RESET_NAME(name)		sound_reset_##name
#define SOUND_RESET(name)			void SOUND_RESET_NAME(name)(running_machine *machine)
#define SOUND_RESET_CALL(name)		SOUND_RESET_NAME(name)(machine)

#define VIDEO_START_NAME(name)		video_start_##name
#define VIDEO_START(name)			void VIDEO_START_NAME(name)(running_machine *machine)
#define VIDEO_START_CALL(name)		VIDEO_START_NAME(name)(machine)

#define VIDEO_RESET_NAME(name)		video_reset_##name
#define VIDEO_RESET(name)			void VIDEO_RESET_NAME(name)(running_machine *machine)
#define VIDEO_RESET_CALL(name)		VIDEO_RESET_NAME(name)(machine)

#define PALETTE_INIT_NAME(name)		palette_init_##name
#define PALETTE_INIT(name)			void PALETTE_INIT_NAME(name)(running_machine *machine, const UINT8 *color_prom)
#define PALETTE_INIT_CALL(name)		PALETTE_INIT_NAME(name)(machine, color_prom)

#define VIDEO_EOF_NAME(name)		video_eof_##name
#define VIDEO_EOF(name)				void VIDEO_EOF_NAME(name)(running_machine *machine)
#define VIDEO_EOF_CALL(name)		VIDEO_EOF_NAME(name)(machine)

#define VIDEO_UPDATE_NAME(name)		video_update_##name
#define VIDEO_UPDATE(name)			UINT32 VIDEO_UPDATE_NAME(name)(screen_device *screen, bitmap_t *bitmap, const rectangle *cliprect)
#define VIDEO_UPDATE_CALL(name)		VIDEO_UPDATE_NAME(name)(screen, bitmap, cliprect)


// NULL versions
#define nvram_handler_0 			NULL
#define memcard_handler_0			NULL
#define machine_start_0 			NULL
#define machine_reset_0 			NULL
#define sound_start_0				NULL
#define sound_reset_0				NULL
#define video_start_0				NULL
#define video_reset_0				NULL
#define palette_init_0				NULL
#define video_eof_0 				NULL
#define video_update_0				NULL


class driver_data_t;

typedef void   (*nvram_handler_func)(running_machine *machine, mame_file *file, int read_or_write);
typedef void   (*memcard_handler_func)(running_machine *machine, mame_file *file, int action);
typedef void   (*machine_start_func)(running_machine *machine);
typedef void   (*machine_reset_func)(running_machine *machine);
typedef void   (*sound_start_func)(running_machine *machine);
typedef void   (*sound_reset_func)(running_machine *machine);
typedef void   (*video_start_func)(running_machine *machine);
typedef void   (*video_reset_func)(running_machine *machine);
typedef void   (*palette_init_func)(running_machine *machine, const UINT8 *color_prom);
typedef void   (*video_eof_func)(running_machine *machine);
typedef UINT32 (*video_update_func)(device_t *screen, bitmap_t *bitmap, const rectangle *cliprect);
typedef driver_data_t *(*driver_data_alloc_func)(running_machine &machine);



/***************************************************************************
    CONSTANTS
***************************************************************************/

// by convention, tags should all lowercase and between 2-15 characters
#define MIN_TAG_LENGTH			2
#define MAX_TAG_LENGTH			15


// token types
enum
{
	MCONFIG_TOKEN_INVALID,
	MCONFIG_TOKEN_END,
	MCONFIG_TOKEN_INCLUDE,

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

	MCONFIG_TOKEN_DEVICE_ADD,
	MCONFIG_TOKEN_DEVICE_REPLACE,
	MCONFIG_TOKEN_DEVICE_REMOVE,
	MCONFIG_TOKEN_DEVICE_MODIFY,

	// device-specific tokens
	MCONFIG_TOKEN_DEVICE_CLOCK,
	MCONFIG_TOKEN_DEVICE_CONFIG,
	MCONFIG_TOKEN_DEVICE_INLINE_DATA16,
	MCONFIG_TOKEN_DEVICE_INLINE_DATA32,
	MCONFIG_TOKEN_DEVICE_INLINE_DATA64,

	// execute interface-specific tokens
	MCONFIG_TOKEN_DIEXEC_DISABLE,
	MCONFIG_TOKEN_DIEXEC_VBLANK_INT,
	MCONFIG_TOKEN_DIEXEC_PERIODIC_INT,

	// memory interface-specific tokens
	MCONFIG_TOKEN_DIMEMORY_MAP,

	// sound interface-specific tokens
	MCONFIG_TOKEN_DISOUND_ROUTE,
	MCONFIG_TOKEN_DISOUND_RESET,

	// legacy custom tokens
	MCONFIG_TOKEN_DEVICE_CONFIG_CUSTOM_1,
	MCONFIG_TOKEN_DEVICE_CONFIG_CUSTOM_2,
	MCONFIG_TOKEN_DEVICE_CONFIG_CUSTOM_3,
	MCONFIG_TOKEN_DEVICE_CONFIG_CUSTOM_4,
	MCONFIG_TOKEN_DEVICE_CONFIG_CUSTOM_5,
	MCONFIG_TOKEN_DEVICE_CONFIG_CUSTOM_6,
	MCONFIG_TOKEN_DEVICE_CONFIG_CUSTOM_7,
	MCONFIG_TOKEN_DEVICE_CONFIG_CUSTOM_8,
	MCONFIG_TOKEN_DEVICE_CONFIG_CUSTOM_9,

	MCONFIG_TOKEN_DEVICE_CONFIG_DATA32,
	MCONFIG_TOKEN_DEVICE_CONFIG_DATA64,
	MCONFIG_TOKEN_DEVICE_CONFIG_DATAFP32,
	MCONFIG_TOKEN_DEVICE_CONFIG_CUSTOM_FREE,
};


// ----- flags for video_attributes -----

// should VIDEO_UPDATE by called at the start of VBLANK or at the end?
#define	VIDEO_UPDATE_BEFORE_VBLANK		0x0000
#define	VIDEO_UPDATE_AFTER_VBLANK		0x0004

// indicates VIDEO_UPDATE will add container bits its
#define VIDEO_SELF_RENDER				0x0008

// automatically extend the palette creating a darker copy for shadows
#define VIDEO_HAS_SHADOWS				0x0010

// automatically extend the palette creating a brighter copy for highlights
#define VIDEO_HAS_HIGHLIGHTS			0x0020

// Mish 181099:  See comments in video/generic.c for details
#define VIDEO_BUFFERS_SPRITERAM			0x0040

// force VIDEO_UPDATE to be called even for skipped frames
#define VIDEO_ALWAYS_UPDATE				0x0080

// calls VIDEO_UPDATE for every visible scanline, even for skipped frames
#define VIDEO_UPDATE_SCANLINE			0x0100



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// forward references
struct gfx_decode_entry;


class machine_config
{
	DISABLE_COPYING(machine_config);

	friend class running_machine;

public:
	machine_config(const machine_config_token *tokens);
	~machine_config();

	void detokenize(const machine_config_token *tokens, const device_config *owner = NULL);

	driver_data_alloc_func	m_driver_data_alloc;		// allocator for driver data

	attotime				m_minimum_quantum;			// minimum scheduling quantum
	const char *			m_perfect_cpu_quantum;		// tag of CPU to use for "perfect" scheduling
	INT32					m_watchdog_vblank_count;	// number of VBLANKs until the watchdog kills us
	attotime				m_watchdog_time;			// length of time until the watchdog kills us

	machine_start_func		m_machine_start;			// one-time machine start callback
	machine_reset_func		m_machine_reset;			// machine reset callback

	nvram_handler_func		m_nvram_handler;			// NVRAM save/load callback
	memcard_handler_func	m_memcard_handler;			// memory card save/load callback

	UINT32					m_video_attributes;			// flags describing the video system
	const gfx_decode_entry *m_gfxdecodeinfo;			// pointer to array of graphics decoding information
	UINT32					m_total_colors;				// total number of colors in the palette
	const char *			m_default_layout;			// default layout for this machine

	palette_init_func		m_init_palette;				// one-time palette init callback
	video_start_func		m_video_start;				// one-time video start callback
	video_reset_func		m_video_reset;				// video reset callback
	video_eof_func			m_video_eof;				// end-of-frame video callback
	video_update_func		m_video_update;				// video update callback

	sound_start_func		m_sound_start;				// one-time sound start callback
	sound_reset_func		m_sound_reset;				// sound reset callback

	device_config_list		m_devicelist;				// list of device configs

private:
	int						m_parse_level;				// nested parsing level
};



/***************************************************************************
    MACROS FOR BUILDING MACHINE DRIVERS
***************************************************************************/

// this type is used to encode machine configuration definitions
union machine_config_token
{
	TOKEN_COMMON_FIELDS
	const machine_config_token *tokenptr;
	const gfx_decode_entry *gfxdecode;
	address_map_constructor addrmap;
	device_type devtype;
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
	device_interrupt_func cpu_interrupt;
	driver_data_alloc_func driver_data_alloc;
};


// helper macro for returning the size of a field within a struct; similar to offsetof()
#define structsizeof(_struct, _field) sizeof(((_struct *)NULL)->_field)

#define DEVCONFIG_OFFSETOF(_class, _member)	\
	((FPTR)&static_cast<_class *>(reinterpret_cast<device_config *>(1000000))->_member - 1000000)

#define DEVCONFIG_SIZEOF(_class, _member)	\
	sizeof(reinterpret_cast<_class *>(NULL)->_member)


// start/end tags for the machine driver
#define MACHINE_DRIVER_NAME(_name) \
	machine_config_##_name

#define MACHINE_DRIVER_START(_name) \
	const machine_config_token MACHINE_DRIVER_NAME(_name)[] = {

#define MACHINE_DRIVER_END \
	TOKEN_UINT32_PACK1(MCONFIG_TOKEN_END, 8) };

// use this to declare external references to a machine driver
#define MACHINE_DRIVER_EXTERN(_name) \
	extern const machine_config_token MACHINE_DRIVER_NAME(_name)[]


// importing data from other machine drivers
#define MDRV_IMPORT_FROM(_name) \
	TOKEN_UINT32_PACK1(MCONFIG_TOKEN_INCLUDE, 8), \
	TOKEN_PTR(tokenptr, MACHINE_DRIVER_NAME(_name)),


// core parameters
#define MDRV_DRIVER_DATA(_class) \
	TOKEN_UINT32_PACK1(MCONFIG_TOKEN_DRIVER_DATA, 8), \
	TOKEN_PTR(m_driver_data_alloc, _class::alloc),

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


// core functions
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


// core video parameters
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


// core video functions
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


// core sound functions
#define MDRV_SOUND_START(_func) \
	TOKEN_UINT32_PACK1(MCONFIG_TOKEN_SOUND_START, 8), \
	TOKEN_PTR(sound_start, SOUND_START_NAME(_func)),

#define MDRV_SOUND_RESET(_func) \
	TOKEN_UINT32_PACK1(MCONFIG_TOKEN_SOUND_RESET, 8), \
	TOKEN_PTR(sound_reset, SOUND_RESET_NAME(_func)),


// add/remove devices
#define MDRV_DEVICE_ADD(_tag, _type, _clock) \
	TOKEN_UINT64_PACK2(MCONFIG_TOKEN_DEVICE_ADD, 8, _clock, 32), \
	TOKEN_PTR(devtype, _type), \
	TOKEN_STRING(_tag),

#define MDRV_DEVICE_REPLACE(_tag, _type, _clock) \
	TOKEN_UINT64_PACK2(MCONFIG_TOKEN_DEVICE_REPLACE, 8, _clock, 32), \
	TOKEN_PTR(devtype, _type), \
	TOKEN_STRING(_tag),

#define MDRV_DEVICE_REMOVE(_tag) \
	TOKEN_UINT32_PACK1(MCONFIG_TOKEN_DEVICE_REMOVE, 8), \
	TOKEN_STRING(_tag),

#define MDRV_DEVICE_MODIFY(_tag)	\
	TOKEN_UINT32_PACK1(MCONFIG_TOKEN_DEVICE_MODIFY, 8), \
	TOKEN_STRING(_tag),


#endif	/* __MCONFIG_H__ */
