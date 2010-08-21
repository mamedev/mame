/***************************************************************************

    machine.h

    Controls execution of the core MAME system.

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

#ifndef __MACHINE_H__
#define __MACHINE_H__

#include <time.h>



//**************************************************************************
//  CONSTANTS
//**************************************************************************

const int MAX_GFX_ELEMENTS = 32;


// machine phases
enum machine_phase
{
	MACHINE_PHASE_PREINIT,
	MACHINE_PHASE_INIT,
	MACHINE_PHASE_RESET,
	MACHINE_PHASE_RUNNING,
	MACHINE_PHASE_EXIT
};


// notification callback types
enum machine_notification
{
	MACHINE_NOTIFY_FRAME,
	MACHINE_NOTIFY_RESET,
	MACHINE_NOTIFY_PAUSE,
	MACHINE_NOTIFY_RESUME,
	MACHINE_NOTIFY_EXIT,
	MACHINE_NOTIFY_COUNT
};


// output channels
enum output_channel
{
    OUTPUT_CHANNEL_ERROR,
    OUTPUT_CHANNEL_WARNING,
    OUTPUT_CHANNEL_INFO,
    OUTPUT_CHANNEL_DEBUG,
    OUTPUT_CHANNEL_VERBOSE,
    OUTPUT_CHANNEL_LOG,
    OUTPUT_CHANNEL_COUNT
};


// debug flags
const int DEBUG_FLAG_ENABLED		= 0x00000001;		// debugging is enabled
const int DEBUG_FLAG_CALL_HOOK		= 0x00000002;		// CPU cores must call instruction hook
const int DEBUG_FLAG_WPR_PROGRAM	= 0x00000010;		// watchpoints are enabled for PROGRAM memory reads
const int DEBUG_FLAG_WPR_DATA		= 0x00000020;		// watchpoints are enabled for DATA memory reads
const int DEBUG_FLAG_WPR_IO			= 0x00000040;		// watchpoints are enabled for IO memory reads
const int DEBUG_FLAG_WPW_PROGRAM	= 0x00000100;		// watchpoints are enabled for PROGRAM memory writes
const int DEBUG_FLAG_WPW_DATA		= 0x00000200;		// watchpoints are enabled for DATA memory writes
const int DEBUG_FLAG_WPW_IO			= 0x00000400;		// watchpoints are enabled for IO memory writes
const int DEBUG_FLAG_OSD_ENABLED	= 0x00001000;		// The OSD debugger is enabled



//**************************************************************************
//  MACROS
//**************************************************************************

// global allocation helpers
#define auto_alloc(m, t)				pool_alloc(static_cast<running_machine *>(m)->m_respool, t)
#define auto_alloc_clear(m, t)			pool_alloc_clear(static_cast<running_machine *>(m)->m_respool, t)
#define auto_alloc_array(m, t, c)		pool_alloc_array(static_cast<running_machine *>(m)->m_respool, t, c)
#define auto_alloc_array_clear(m, t, c)	pool_alloc_array_clear(static_cast<running_machine *>(m)->m_respool, t, c)
#define auto_free(m, v)					pool_free(static_cast<running_machine *>(m)->m_respool, v)

#define auto_bitmap_alloc(m, w, h, f)	auto_alloc(m, bitmap_t(w, h, f))
#define auto_strdup(m, s)				strcpy(auto_alloc_array(m, char, strlen(s) + 1), s)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// forward declarations
class gfx_element;
class colortable_t;
class debug_view_manager;

typedef struct _mame_private mame_private;
typedef struct _cpuexec_private cpuexec_private;
typedef struct _timer_private timer_private;
typedef struct _state_private state_private;
typedef struct _memory_private memory_private;
typedef struct _palette_private palette_private;
typedef struct _tilemap_private tilemap_private;
typedef struct _streams_private streams_private;
typedef struct _devices_private devices_private;
typedef struct _romload_private romload_private;
typedef struct _sound_private sound_private;
typedef struct _input_private input_private;
typedef struct _input_port_private input_port_private;
typedef struct _ui_input_private ui_input_private;
typedef struct _cheat_private cheat_private;
typedef struct _debugcpu_private debugcpu_private;
typedef struct _debugvw_private debugvw_private;
typedef struct _generic_machine_private generic_machine_private;
typedef struct _generic_video_private generic_video_private;
typedef struct _generic_audio_private generic_audio_private;


// template specializations
typedef tagged_list<region_info> region_list;


// base class for all driver data structures
class driver_data_t : public bindable_object
{
public:
	driver_data_t(running_machine &machine);
	virtual ~driver_data_t();

	running_machine &	m_machine;
};


// memory region
class region_info
{
	DISABLE_COPYING(region_info);

	friend class running_machine;
	template<class T> friend class tagged_list;
	friend resource_pool_object<region_info>::~resource_pool_object();

	// construction/destruction
	region_info(running_machine &machine, const char *name, UINT32 length, UINT32 flags);
	~region_info();

public:
	// getters
	region_info *next() const { return m_next; }
	UINT8 *base() const { return (this != NULL) ? m_base.u8 : NULL; }
	UINT8 *end() const { return (this != NULL) ? m_base.u8 + m_length : NULL; }
	UINT32 bytes() const { return (this != NULL) ? m_length : 0; }
	const char *name() const { return m_name; }
	UINT32 flags() const { return m_flags; }

	// flag expansion
	endianness_t endianness() const { return ((m_flags & ROMREGION_ENDIANMASK) == ROMREGION_LE) ? ENDIANNESS_LITTLE : ENDIANNESS_BIG; }
	UINT8 width() const { return 1 << ((m_flags & ROMREGION_WIDTHMASK) >> 8); }
	bool invert() const { return ((m_flags & ROMREGION_INVERTMASK) != 0); }

	// data access
	UINT8 &u8(offs_t offset = 0) const { return m_base.u8[offset]; }
	UINT16 &u16(offs_t offset = 0) const { return m_base.u16[offset]; }
	UINT32 &u32(offs_t offset = 0) const { return m_base.u32[offset]; }
	UINT64 &u64(offs_t offset = 0) const { return m_base.u64[offset]; }

	// allow passing a region for any common pointer
	operator void *() const { return (this != NULL) ? m_base.v : NULL; }
	operator INT8 *() const { return (this != NULL) ? m_base.i8 : NULL; }
	operator UINT8 *() const { return (this != NULL) ? m_base.u8 : NULL; }
	operator INT16 *() const { return (this != NULL) ? m_base.i16 : NULL; }
	operator UINT16 *() const { return (this != NULL) ? m_base.u16 : NULL; }
	operator INT32 *() const { return (this != NULL) ? m_base.i32 : NULL; }
	operator UINT32 *() const { return (this != NULL) ? m_base.u32 : NULL; }
	operator INT64 *() const { return (this != NULL) ? m_base.i64 : NULL; }
	operator UINT64 *() const { return (this != NULL) ? m_base.u64 : NULL; }

private:
	// internal data
	running_machine &		m_machine;
	region_info *			m_next;
	astring					m_name;
	generic_ptr				m_base;
	UINT32					m_length;
	UINT32					m_flags;
};


// this structure holds generic pointers that are commonly used
struct generic_pointers
{
	generic_ptr				nvram;				// generic NVRAM
	UINT32					nvram_size;
	generic_ptr				videoram;			// videoram
	UINT32					videoram_size;
	generic_ptr				spriteram;			// spriteram
	UINT32					spriteram_size;
	generic_ptr				spriteram2;			// secondary spriteram
	UINT32					spriteram2_size;
	generic_ptr				buffered_spriteram;	// buffered spriteram
	generic_ptr				buffered_spriteram2;// secondary buffered spriteram
	generic_ptr				paletteram;			// palette RAM
	generic_ptr				paletteram2;		// secondary palette RAM
	bitmap_t *				tmpbitmap;			// temporary bitmap
};


// system time description, both local and UTC
class system_time
{
public:
	system_time();
	void set(time_t t);

	struct full_time
	{
		void set(struct tm &t);

		UINT8		second;		// seconds (0-59)
		UINT8		minute;		// minutes (0-59)
		UINT8		hour;		// hours (0-23)
		UINT8		mday;		// day of month (1-31)
		UINT8		month;		// month (0-11)
		INT32		year;		// year (1=1 AD)
		UINT8		weekday;	// day of week (0-6)
		UINT16		day;		// day of year (0-365)
		UINT8		is_dst;		// is this daylight savings?
	};

	INT64			time;		// number of seconds elapsed since midnight, January 1 1970 UTC
	full_time		local_time;	// local time
	full_time		utc_time;	// UTC coordinated time
};


// description of the currently-running machine
class running_machine : public bindable_object
{
	DISABLE_COPYING(running_machine);

	typedef void (*notify_callback)(running_machine &machine);
	typedef void (*logerror_callback)(running_machine &machine, const char *string);

public:
	// construction/destruction
	running_machine(const game_driver &driver, const machine_config &config, core_options &options, bool exit_to_game_select = false);
	~running_machine();

	// fetch items by name
	inline device_t *device(const char *tag);
	template<class T> inline T *device(const char *tag) { return downcast<T *>(device(tag)); }
	inline const input_port_config *port(const char *tag);
	inline const region_info *region(const char *tag);

	// configuration helpers
	UINT32 total_colors() const { return m_config.m_total_colors; }

	// getters
	const char *basename() const { return m_basename; }
	core_options *options() const { return &m_options; }
	machine_phase phase() const { return m_current_phase; }
	bool paused() const { return m_paused || (m_current_phase != MACHINE_PHASE_RUNNING); }
	bool scheduled_event_pending() const { return m_exit_pending || m_hard_reset_pending; }
	bool save_or_load_pending() const { return (m_saveload_pending_file); }
	bool exit_pending() const { return m_exit_pending; }
	bool new_driver_pending() const { return (m_new_driver_pending != NULL); }
	const char *new_driver_name() const { return m_new_driver_pending->name; }
	device_scheduler &scheduler() { return m_scheduler; }

	// immediate operations
	int run(bool firstrun);
	void pause();
	void resume();
	void add_notifier(machine_notification event, notify_callback callback);
	void call_notifiers(machine_notification which);
	void add_logerror_callback(logerror_callback callback);

	// scheduled operations
	void schedule_exit();
	void schedule_hard_reset();
	void schedule_soft_reset();
	void schedule_new_driver(const game_driver &driver);
	void schedule_save(const char *filename);
	void schedule_load(const char *filename);

	// time
	void base_datetime(system_time &systime);
	void current_datetime(system_time &systime);

	// regions
	region_info *region_alloc(const char *name, UINT32 length, UINT32 flags);
	void region_free(const char *name);

	// misc
	void CLIB_DECL logerror(const char *format, ...);
	void CLIB_DECL vlogerror(const char *format, va_list args);
	UINT32 rand();
	const char *describe_context();

	// internals
	resource_pool			m_respool;			// pool of resources for this machine
	region_list				m_regionlist;		// list of memory regions
	device_list				m_devicelist;		// list of running devices

	// configuration data
	const machine_config *	config;				// points to the constructed machine_config
	const machine_config &	m_config;			// points to the constructed machine_config
	ioport_list				m_portlist;			// points to a list of input port configurations

	// CPU information
	cpu_device *			firstcpu;			// first CPU (allows for quick iteration via typenext)
	address_space *			m_nonspecific_space;// a dummy address_space used for legacy compatibility

	// game-related information
	const game_driver *		gamedrv;			// points to the definition of the game machine
	const game_driver &		m_game;				// points to the definition of the game machine

	// video-related information
	gfx_element *			gfx[MAX_GFX_ELEMENTS];// array of pointers to graphic sets (chars, sprites)
	screen_device *			primary_screen;		// the primary screen device, or NULL if screenless
	palette_t *				palette;			// global palette object

	// palette-related information
	const pen_t *			pens;				// remapped palette pen numbers
	colortable_t *			colortable;			// global colortable for remapping
	pen_t *					shadow_table;		// table for looking up a shadowed pen
	bitmap_t *				priority_bitmap;	// priority bitmap

	// audio-related information
	int						sample_rate;		// the digital audio sample rate

	// debugger-related information
	UINT32					debug_flags;		// the current debug flags

	// UI-related
	bool					ui_active;			// ui active or not (useful for games / systems with keyboard inputs)

	// generic pointers
	generic_pointers		generic;			// generic pointers

	// internal core information
	mame_private *			mame_data;			// internal data from mame.c
	timer_private *			timer_data;			// internal data from timer.c
	state_private *			state_data;			// internal data from state.c
	memory_private *		memory_data;		// internal data from memory.c
	palette_private *		palette_data;		// internal data from palette.c
	tilemap_private *		tilemap_data;		// internal data from tilemap.c
	streams_private *		streams_data;		// internal data from streams.c
	devices_private *		devices_data;		// internal data from devices.c
	romload_private *		romload_data;		// internal data from romload.c
	sound_private *			sound_data;			// internal data from sound.c
	input_private *			input_data;			// internal data from input.c
	input_port_private *	input_port_data;	// internal data from inptport.c
	ui_input_private *		ui_input_data;		// internal data from uiinput.c
	cheat_private *			cheat_data;			// internal data from cheat.c
	debugcpu_private *		debugcpu_data;		// internal data from debugcpu.c
	generic_machine_private *generic_machine_data; // internal data from machine/generic.c
	generic_video_private *	generic_video_data;	// internal data from video/generic.c
	generic_audio_private *	generic_audio_data;	// internal data from audio/generic.c

	debug_view_manager *	m_debug_view;		// internal data from debugvw.c

	// driver-specific information
	template<class T>
	T *driver_data() const { return downcast<T *>(m_driver_data); }

private:
	void start();
	void set_saveload_filename(const char *filename);
	void fill_systime(system_time &systime, time_t t);
	void handle_saveload();

	static TIMER_CALLBACK( static_soft_reset );
	void soft_reset();

	static void logfile_callback(running_machine &machine, const char *buffer);

	// notifier callbacks
	struct notifier_callback_item
	{
		notifier_callback_item(notify_callback func);
		notifier_callback_item *	m_next;
		notify_callback				m_func;
	};
	notifier_callback_item *m_notifier_list[MACHINE_NOTIFY_COUNT];

	// logerror callbacks
	struct logerror_callback_item
	{
		logerror_callback_item(logerror_callback func);
		logerror_callback_item *	m_next;
		logerror_callback			m_func;
	};
	logerror_callback_item *m_logerror_list;

	device_scheduler		m_scheduler;		// scheduler object
	core_options &			m_options;

	astring					m_context;			// context string
	astring					m_basename;			// basename used for game-related paths

	machine_phase			m_current_phase;
	bool					m_paused;
	bool					m_hard_reset_pending;
	bool					m_exit_pending;
	bool					m_exit_to_game_select;
	const game_driver *		m_new_driver_pending;
	emu_timer *				m_soft_reset_timer;
	mame_file *				m_logfile;

	// load/save
	enum saveload_schedule
	{
		SLS_NONE,
		SLS_SAVE,
		SLS_LOAD
	};
	saveload_schedule		m_saveload_schedule;
	attotime				m_saveload_schedule_time;
	astring					m_saveload_pending_file;
	const char *			m_saveload_searchpath;

	// random number seed
	UINT32					m_rand_seed;

	// base time
	time_t					m_base_time;

	driver_data_t *			m_driver_data;		// drivers can hang data off of here instead of using globals
};



//**************************************************************************
//  INLINE FUNCTIONS
//**************************************************************************

inline device_t *running_machine::device(const char *tag)
{
	return m_devicelist.find(tag);
}

inline const input_port_config *running_machine::port(const char *tag)
{
	return m_portlist.find(tag);
}

inline const region_info *running_machine::region(const char *tag)
{
	return m_regionlist.find(tag);
}

inline UINT32 mame_rand(running_machine *machine)
{
	return machine->rand();
}

inline UINT8 *memory_region(running_machine *machine, const char *name)
{
	const region_info *region = machine->region(name);
	return (region != NULL) ? region->base() : NULL;
}

inline UINT32 memory_region_length(running_machine *machine, const char *name)
{
	const region_info *region = machine->region(name);
	return (region != NULL) ? region->bytes() : 0;
}


#endif	/* __MACHINE_H__ */
