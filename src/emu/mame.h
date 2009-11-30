/***************************************************************************

    mame.h

    Controls execution of the core MAME system.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#ifndef __MAME_H__
#define __MAME_H__

#include "mamecore.h"
#include "video.h"
#include "crsshair.h"
#include "restrack.h"
#include "options.h"
#include "inptport.h"
#include "cpuintrf.h"
#include <stdarg.h>

#ifdef MESS
#include "image.h"
#include "uimess.h"
#endif /* MESS */


/***************************************************************************
    CONSTANTS
***************************************************************************/

/* return values from run_game */
enum
{
	MAMERR_NONE				= 0,	/* no error */
	MAMERR_FAILED_VALIDITY	= 1,	/* failed validity checks */
	MAMERR_MISSING_FILES	= 2,	/* missing files */
	MAMERR_FATALERROR		= 3,	/* some other fatal error */
	MAMERR_DEVICE			= 4,	/* device initialization error (MESS-specific) */
	MAMERR_NO_SUCH_GAME		= 5,	/* game was specified but doesn't exist */
	MAMERR_INVALID_CONFIG	= 6,	/* some sort of error in configuration */
	MAMERR_IDENT_NONROMS	= 7,	/* identified all non-ROM files */
	MAMERR_IDENT_PARTIAL	= 8,	/* identified some files but not all */
	MAMERR_IDENT_NONE		= 9		/* identified no files */
};


/* program phases */
enum
{
	MAME_PHASE_PREINIT,
	MAME_PHASE_INIT,
	MAME_PHASE_RESET,
	MAME_PHASE_RUNNING,
	MAME_PHASE_EXIT
};


/* debug flags */
#define DEBUG_FLAG_ENABLED		0x00000001		/* debugging is enabled */
#define DEBUG_FLAG_CALL_HOOK	0x00000002		/* CPU cores must call instruction hook */
#define DEBUG_FLAG_WPR_PROGRAM	0x00000010		/* watchpoints are enabled for PROGRAM memory reads */
#define DEBUG_FLAG_WPR_DATA		0x00000020		/* watchpoints are enabled for DATA memory reads */
#define DEBUG_FLAG_WPR_IO		0x00000040		/* watchpoints are enabled for IO memory reads */
#define DEBUG_FLAG_WPW_PROGRAM	0x00000100		/* watchpoints are enabled for PROGRAM memory writes */
#define DEBUG_FLAG_WPW_DATA		0x00000200		/* watchpoints are enabled for DATA memory writes */
#define DEBUG_FLAG_WPW_IO		0x00000400		/* watchpoints are enabled for IO memory writes */


/* maxima */
#define MAX_GFX_ELEMENTS		32


/* MESS vs. MAME abstractions */
#ifndef MESS
#define APPNAME					"MAME"
#define APPNAME_LOWER			"mame"
#define CONFIGNAME				"mame"
#define APPLONGNAME				"M.A.M.E."
#define CAPGAMENOUN				"GAME"
#define CAPSTARTGAMENOUN		"Game"
#define GAMENOUN				"game"
#define GAMESNOUN				"games"
#define HISTORYNAME				"History"
#define COPYRIGHT				"Copyright Nicola Salmoria\nand the MAME team\nhttp://mamedev.org"
#else
#define APPNAME					"MESS"
#define APPNAME_LOWER			"mess"
#define CONFIGNAME				"mess"
#define APPLONGNAME				"M.E.S.S."
#define CAPGAMENOUN				"SYSTEM"
#define CAPSTARTGAMENOUN		"System"
#define GAMENOUN				"system"
#define GAMESNOUN				"systems"
#define HISTORYNAME				"System Info"
#define COPYRIGHT				"Copyright the MESS team\nhttp://mess.org"
#endif


/* output channels */
enum _output_channel
{
    OUTPUT_CHANNEL_ERROR,
    OUTPUT_CHANNEL_WARNING,
    OUTPUT_CHANNEL_INFO,
    OUTPUT_CHANNEL_DEBUG,
    OUTPUT_CHANNEL_VERBOSE,
    OUTPUT_CHANNEL_LOG,
    OUTPUT_CHANNEL_COUNT
};
typedef enum _output_channel output_channel;



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* output channel callback */
typedef void (*output_callback_func)(void *param, const char *format, va_list argptr);


/* forward type declarations */
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


/* structure to hold a pointer/size pair for generic pointers */
typedef union _generic_ptr generic_ptr;
union _generic_ptr
{
	void *		v;
	UINT8 *		u8;
	UINT16 *	u16;
	UINT32 *	u32;
	UINT64 *	u64;
};


/* this structure holds generic pointers that are commonly used */
typedef struct _generic_pointers generic_pointers;
struct _generic_pointers
{
	generic_ptr				nvram;				/* generic NVRAM */
	UINT32					nvram_size;
	generic_ptr				videoram;			/* videoram */
	UINT32					videoram_size;
	generic_ptr				colorram;			/* color ram */
	UINT32					colorram_size;
	generic_ptr				spriteram;			/* spriteram */
	UINT32					spriteram_size;
	generic_ptr				spriteram2;			/* secondary spriteram */
	UINT32					spriteram2_size;
	generic_ptr				buffered_spriteram;	/* buffered spriteram */
	generic_ptr				buffered_spriteram2;/* secondary buffered spriteram */
	generic_ptr				buffered_spriteram3;/* tertiary buffered spriteram */
	generic_ptr				paletteram;			/* palette RAM */
	generic_ptr				paletteram2;		/* secondary palette RAM */
	bitmap_t *				tmpbitmap;			/* temporary bitmap */
};


/* description of the currently-running machine */
/* typedef struct _running_machine running_machine; -- in mamecore.h */
struct _running_machine
{
	/* configuration data */
	const machine_config *	config;				/* points to the constructed machine_config */
	input_port_list			portlist;			/* points to a list of input port configurations */

	/* CPU information */
	const device_config *	firstcpu;			/* first CPU (allows for quick iteration via typenext) */

	/* game-related information */
	const game_driver *		gamedrv;			/* points to the definition of the game machine */
	const char *			basename;			/* basename used for game-related paths */

	/* video-related information */
	gfx_element *			gfx[MAX_GFX_ELEMENTS];/* array of pointers to graphic sets (chars, sprites) */
	const device_config *	primary_screen;		/* the primary screen device, or NULL if screenless */
	palette_t *				palette;			/* global palette object */

	/* palette-related information */
	const pen_t *			pens;				/* remapped palette pen numbers */
	struct _colortable_t *	colortable;			/* global colortable for remapping */
	pen_t *					shadow_table;		/* table for looking up a shadowed pen */
	bitmap_t *				priority_bitmap;	/* priority bitmap */

	/* audio-related information */
	int						sample_rate;		/* the digital audio sample rate */

	/* debugger-related information */
	UINT32					debug_flags;		/* the current debug flags */

	/* generic pointers */
	generic_pointers		generic;			/* generic pointers */

	/* internal core information */
	mame_private *			mame_data;			/* internal data from mame.c */
	cpuexec_private *		cpuexec_data;		/* internal data from cpuexec.c */
	timer_private *			timer_data;			/* internal data from timer.c */
	state_private *			state_data;			/* internal data from state.c */
	memory_private *		memory_data;		/* internal data from memory.c */
	palette_private *		palette_data;		/* internal data from palette.c */
	tilemap_private *		tilemap_data;		/* internal data from tilemap.c */
	streams_private *		streams_data;		/* internal data from streams.c */
	devices_private *		devices_data;		/* internal data from devices.c */
	romload_private *		romload_data;		/* internal data from romload.c */
	sound_private *			sound_data;			/* internal data from sound.c */
	input_private *			input_data;			/* internal data from input.c */
	input_port_private *	input_port_data;	/* internal data from inptport.c */
	ui_input_private *		ui_input_data;		/* internal data from uiinput.c */
	cheat_private *			cheat_data;			/* internal data from cheat.c */
	debugcpu_private *		debugcpu_data;		/* internal data from debugcpu.c */
	debugvw_private *		debugvw_data;		/* internal data from debugvw.c */
	generic_machine_private *generic_machine_data; /* internal data from machine/generic.c */
	generic_video_private *	generic_video_data;	/* internal data from video/generic.c */
	generic_audio_private *	generic_audio_data;	/* internal data from audio/generic.c */
#ifdef MESS
	images_private *		images_data;		/* internal data from image.c */
	ui_mess_private *		ui_mess_data;		/* internal data from uimess.c */
#endif /* MESS */

	/* driver-specific information */
	void *					driver_data;		/* drivers can hang data off of here instead of using globals */
};


typedef struct _mame_system_tm mame_system_tm;
struct _mame_system_tm
{
	UINT8 second;	/* seconds (0-59) */
	UINT8 minute;	/* minutes (0-59) */
	UINT8 hour;		/* hours (0-23) */
	UINT8 mday;		/* day of month (1-31) */
	UINT8 month;	/* month (0-11) */
	INT32 year;		/* year (1=1 AD) */
	UINT8 weekday;	/* day of week (0-6) */
	UINT16 day;		/* day of year (0-365) */
	UINT8 is_dst;	/* is this daylight savings? */
};


typedef struct _mame_system_time mame_system_time;
struct _mame_system_time
{
	INT64 time;					/* number of seconds elapsed since midnight, January 1 1970 UTC */
	mame_system_tm local_time;	/* local time */
	mame_system_tm utc_time;	/* UTC coordinated time */
};



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

extern const char mame_disclaimer[];
extern char giant_string_buffer[];

extern const char build_version[];



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/


/* ----- core system management ----- */

/* execute as configured by the OPTION_GAMENAME option on the specified options */
int mame_execute(core_options *options);

/* accesses the core_options for the currently running emulation */
core_options *mame_options(void);

/* return the current phase */
int mame_get_phase(running_machine *machine);

/* request callback on frame update */
void add_frame_callback(running_machine *machine, void (*callback)(running_machine *));

/* request callback on reset */
void add_reset_callback(running_machine *machine, void (*callback)(running_machine *));

/* request callback on pause */
void add_pause_callback(running_machine *machine, void (*callback)(running_machine *, int));

/* request callback on termination */
void add_exit_callback(running_machine *machine, void (*callback)(running_machine *));

/* handle update tasks for a frame boundary */
void mame_frame_update(running_machine *machine);

/* return true if the given machine is valid */
int mame_is_valid_machine(running_machine *machine);



/* ----- global system states ----- */

/* schedule an exit */
void mame_schedule_exit(running_machine *machine);

/* schedule a hard reset */
void mame_schedule_hard_reset(running_machine *machine);

/* schedule a soft reset */
void mame_schedule_soft_reset(running_machine *machine);

/* schedule a new driver */
void mame_schedule_new_driver(running_machine *machine, const game_driver *driver);

/* schedule a save */
void mame_schedule_save(running_machine *machine, const char *filename);

/* schedule a load */
void mame_schedule_load(running_machine *machine, const char *filename);

/* is a save or load pending? */
int mame_is_save_or_load_pending(running_machine *machine);

/* is a scheduled event pending? */
int mame_is_scheduled_event_pending(running_machine *machine);

/* pause the system */
void mame_pause(running_machine *machine, int pause);

/* get the current pause state */
int mame_is_paused(running_machine *machine);



/* ----- memory region management ----- */

/* allocate a new memory region */
UINT8 *memory_region_alloc(running_machine *machine, const char *name, UINT32 length, UINT32 flags);

/* free an allocated memory region */
void memory_region_free(running_machine *machine, const char *name);

/* return a pointer to a specified memory region */
UINT8 *memory_region(running_machine *machine, const char *name);

/* return the size (in bytes) of a specified memory region */
UINT32 memory_region_length(running_machine *machine, const char *name);

/* return the flags (defined in romload.h) for a specified memory region */
UINT32 memory_region_flags(running_machine *machine, const char *name);

/* return the name of the next memory region (or the first if name == NULL) */
const char *memory_region_next(running_machine *machine, const char *name);



/* ----- output management ----- */

/* set the output handler for a channel, returns the current one */
void mame_set_output_channel(output_channel channel, output_callback_func callback, void *param, output_callback_func *prevcb, void **prevparam);

/* built-in default callbacks */
void mame_file_output_callback(void *param, const char *format, va_list argptr);
void mame_null_output_callback(void *param, const char *format, va_list argptr);

/* calls to be used by the code */
void mame_printf_error(const char *format, ...) ATTR_PRINTF(1,2);
void mame_printf_warning(const char *format, ...) ATTR_PRINTF(1,2);
void mame_printf_info(const char *format, ...) ATTR_PRINTF(1,2);
void mame_printf_verbose(const char *format, ...) ATTR_PRINTF(1,2);
void mame_printf_debug(const char *format, ...) ATTR_PRINTF(1,2);

/* discourage the use of printf directly */
/* sadly, can't do this because of the ATTR_PRINTF under GCC */
/*
#undef printf
#define printf !MUST_USE_MAME_PRINTF_*_CALLS_WITHIN_THE_CORE!
*/


/* ----- miscellaneous bits & pieces ----- */

/* pop-up a user visible message */
void CLIB_DECL popmessage(const char *format,...) ATTR_PRINTF(1,2);

/* log to the standard error.log file */
void CLIB_DECL logerror(const char *format,...) ATTR_PRINTF(1,2);

/* adds a callback to be called on logerror() */
void add_logerror_callback(running_machine *machine, void (*callback)(running_machine *, const char *));

/* parse the configured INI files */
void mame_parse_ini_files(core_options *options, const game_driver *driver);

/* standardized random number generator */
UINT32 mame_rand(running_machine *machine);

/* retrieve the base system time */
void mame_get_base_datetime(running_machine *machine, mame_system_time *systime);

/* retrieve the current system time */
void mame_get_current_datetime(running_machine *machine, mame_system_time *systime);



#ifdef MESS
#include "mess.h"
#endif /* MESS */

#endif	/* __MAME_H__ */
