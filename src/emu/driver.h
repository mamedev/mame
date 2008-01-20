/***************************************************************************

    driver.h

    Include this with all MAME files. Includes all the core system pieces.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __DRIVER_H__
#define __DRIVER_H__


/***************************************************************************
    MACROS (must be *before* the includes below)
***************************************************************************/

#define DRIVER_INIT(name)		void driver_init_##name(running_machine *machine)

#define NVRAM_HANDLER(name)		void nvram_handler_##name(running_machine *machine, mame_file *file, int read_or_write)

#define MEMCARD_HANDLER(name)	void memcard_handler_##name(running_machine *machine, mame_file *file, int action)

#define MACHINE_START(name)		void machine_start_##name(running_machine *machine)
#define MACHINE_RESET(name)		void machine_reset_##name(running_machine *machine)

#define SOUND_START(name)		void sound_start_##name(running_machine *machine)
#define SOUND_RESET(name)		void sound_reset_##name(running_machine *machine)

#define VIDEO_START(name)		void video_start_##name(running_machine *machine)
#define VIDEO_RESET(name)		void video_reset_##name(running_machine *machine)

#define PALETTE_INIT(name)		void palette_init_##name(running_machine *machine, UINT16 *colortable, const UINT8 *color_prom)
#define VIDEO_EOF(name)			void video_eof_##name(running_machine *machine)
#define VIDEO_UPDATE(name)		UINT32 video_update_##name(running_machine *machine, int screen, mame_bitmap *bitmap, const rectangle *cliprect)

/* NULL versions */
#define nvram_handler_NULL 		NULL
#define memcard_handler_NULL	NULL
#define machine_start_NULL 		NULL
#define machine_reset_NULL 		NULL
#define sound_start_NULL 		NULL
#define sound_reset_NULL 		NULL
#define video_start_NULL 		NULL
#define video_reset_NULL 		NULL
#define palette_init_NULL		NULL
#define video_eof_NULL 			NULL
#define video_update_NULL 		NULL



/***************************************************************************
    INCLUDES
***************************************************************************/

#include "cpuintrf.h"
#include "sndintrf.h"
#include "fileio.h"
#include "drawgfx.h"
#include "emupal.h"
#include "sound.h"
#include "input.h"
#include "inptport.h"
#include "output.h"
#include "tilemap.h"
#include "romload.h"
#include "drivers/xtal.h"
#include "machine/generic.h"
#include "audio/generic.h"
#include "video/generic.h"

#ifdef MESS
#include "messdrv.h"
#endif



/***************************************************************************
    CONSTANTS
***************************************************************************/

/* maxima */
#define MAX_DRIVER_NAME_CHARS	8
#define MAX_SPEAKER 			4

/* watchdog constants */
#define DEFAULT_60HZ_3S_VBLANK_WATCHDOG	180
#define DEFAULT_30HZ_3S_VBLANK_WATCHDOG	90



/* ----- VBLANK constants ----- */

/*
    VBLANK is the period when the video beam is outside of the visible area
    and returns from the bottom-right to the top-left of the screen to
    prepare for a new video frame. The duration of VBLANK is an important
    factor in how the game renders itself. In many cases, the game does
    video-related operations during this time, such as swapping buffers or
    updating sprite lists.

    Below are some predefined, TOTALLY ARBITRARY values for vblank_duration,
    which should be OK for most cases. I have NO IDEA how accurate they are
    compared to the real hardware, they could be completely wrong.
*/

/* The default is to have no VBLANK timing -- this is historical, and a bad idea */
#define DEFAULT_60HZ_VBLANK_DURATION		ATTOSECONDS_IN_USEC(0)
#define DEFAULT_30HZ_VBLANK_DURATION		ATTOSECONDS_IN_USEC(0)

/* If you use IPT_VBLANK, you need a duration different from 0 */
#define DEFAULT_REAL_60HZ_VBLANK_DURATION	ATTOSECONDS_IN_USEC(2500)
#define DEFAULT_REAL_30HZ_VBLANK_DURATION	ATTOSECONDS_IN_USEC(2500)



/* ----- flags for video_attributes ----- */

/* is the video hardware raster or vector based? */
#define VIDEO_TYPE_NONE                 0x0000
#define	VIDEO_TYPE_RASTER				0x0001
#define	VIDEO_TYPE_VECTOR				0x0002

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



/* ----- flags for game drivers ----- */

#define ORIENTATION_MASK        		0x0007
#define GAME_NOT_WORKING				0x0008
#define GAME_UNEMULATED_PROTECTION		0x0010	/* game's protection not fully emulated */
#define GAME_WRONG_COLORS				0x0020	/* colors are totally wrong */
#define GAME_IMPERFECT_COLORS			0x0040	/* colors are not 100% accurate, but close */
#define GAME_IMPERFECT_GRAPHICS			0x0080	/* graphics are wrong/incomplete */
#define GAME_NO_COCKTAIL				0x0100	/* screen flip support is missing */
#define GAME_NO_SOUND					0x0200	/* sound is missing */
#define GAME_IMPERFECT_SOUND			0x0400	/* sound is known to be wrong */
#define GAME_SUPPORTS_SAVE				0x0800	/* game supports save states */
#define GAME_IS_BIOS_ROOT				0x1000	/* this driver entry is a BIOS root */
#define GAME_NO_STANDALONE				0x2000	/* this driver cannot stand alone */

#ifdef MESS
#define GAME_COMPUTER               	0x8000  /* Driver is a computer (needs full keyboard) */
#define GAME_COMPUTER_MODIFIED      	0x4000	/* Official? Hack */
#endif



/* ----- flags to return from video_update ----- */
#define UPDATE_HAS_NOT_CHANGED			0x0001	/* the video has not changed */



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* In mamecore.h: typedef struct _machine_config machine_config; */
struct _machine_config
{
	UINT32				driver_data_size;			/* amount of memory needed for driver_data */

	cpu_config			cpu[MAX_CPU];				/* array of CPUs in the system */
	UINT32				cpu_slices_per_frame;		/* number of times to interleave execution per frame */
	INT32				watchdog_vblank_count;		/* number of VBLANKs until the watchdog kills us */
	attotime			watchdog_time;				/* length of time until the watchdog kills us */

	void 				(*machine_start)(running_machine *machine);		/* one-time machine start callback */
	void 				(*machine_reset)(running_machine *machine);		/* machine reset callback */

	void 				(*nvram_handler)(running_machine *machine, mame_file *file, int read_or_write); /* NVRAM save/load callback  */
	void 				(*memcard_handler)(running_machine *machine, mame_file *file, int action); /* memory card save/load callback  */

	UINT32				video_attributes;			/* flags describing the video system */
	const gfx_decode_entry *gfxdecodeinfo;			/* pointer to array of graphics decoding information */
	UINT32				total_colors;				/* total number of colors in the palette */
	UINT32				color_table_len;			/* length of the color indirection table */
	const char *		default_layout;				/* default layout for this machine */
	screen_config		screen[MAX_SCREENS];		/* total number of screens */

	void 				(*init_palette)(running_machine *machine, UINT16 *colortable, const UINT8 *color_prom); /* one-time palette init callback  */
	void				(*video_start)(running_machine *machine);		/* one-time video start callback */
	void				(*video_reset)(running_machine *machine);		/* video reset callback */
	void				(*video_eof)(running_machine *machine);			/* end-of-frame video callback */
	UINT32				(*video_update)(running_machine *machine, int screen, mame_bitmap *bitmap, const rectangle *cliprect); /* video update callback */

	sound_config		sound[MAX_SOUND];			/* array of sound chips in the system */
	speaker_config		speaker[MAX_SPEAKER];		/* array of speakers in the system */

	void				(*sound_start)(running_machine *machine);		/* one-time sound start callback */
	void				(*sound_reset)(running_machine *machine);		/* sound reset callback */
};


/* In mamecore.h: typedef struct _game_driver game_driver; */
struct _game_driver
{
	const char *		source_file;				/* set this to __FILE__ */
	const char *		parent;						/* if this is a clone, the name of the parent */
	const char *		name;						/* short (8-character) name of the game */
	const char *		description;				/* full name of the game */
	const char *		year;						/* year the game was released */
	const char *		manufacturer;				/* manufacturer of the game */
	void 				(*drv)(machine_config *);	/* machine driver constructor */
	const input_port_token *ipt;					/* pointer to array of input port tokens */
	void				(*driver_init)(running_machine *machine); /* DRIVER_INIT callback */
	const rom_entry *	rom;						/* pointer to list of ROMs for the game */

#ifdef MESS
	void (*sysconfig_ctor)(struct SystemConfigurationParamBlock *cfg);
	const char *		compatible_with;
#endif

	UINT32				flags;						/* orientation and other flags; see defines below */
	const char *		default_layout;				/* default internally defined layout */
};



/***************************************************************************
    MACROS FOR BUILDING MACHINE DRIVERS
***************************************************************************/

/* use this to declare external references to a machine driver */
#define MACHINE_DRIVER_EXTERN(game)										\
	void construct_##game(machine_config *machine)						\


/* start/end tags for the machine driver */
#define MACHINE_DRIVER_START(game) 										\
	void construct_##game(machine_config *machine)						\
	{																	\
		cpu_config *cpu = NULL;											\
		sound_config *sound = NULL;										\
		screen_config *screen = &machine->screen[0];					\
		(void)cpu;														\
		(void)sound;													\
		(void)screen;													\

#define MACHINE_DRIVER_END 												\
	}																	\


/* importing data from other machine drivers */
#define MDRV_IMPORT_FROM(game) 											\
	construct_##game(machine); 											\


/* add/modify/remove/replace CPUs */
#define MDRV_CPU_ADD_TAG(tag, type, clock)								\
	cpu = driver_add_cpu(machine, (tag), CPU_##type, (clock));			\

#define MDRV_CPU_ADD(type, clock)										\
	MDRV_CPU_ADD_TAG(NULL, type, clock)									\

#define MDRV_CPU_MODIFY(tag)											\
	cpu = driver_find_cpu(machine, tag);								\

#define MDRV_CPU_REMOVE(tag)											\
	driver_remove_cpu(machine, tag);									\
	cpu = NULL;															\

#define MDRV_CPU_REPLACE(tag, _type, _clock)								\
	cpu = driver_find_cpu(machine, tag);								\
	cpu->type = (CPU_##_type);											\
	cpu->clock = (_clock);												\


/* CPU parameters */
#define MDRV_CPU_FLAGS(_flags)											\
	if (cpu)															\
		cpu->flags = (_flags);											\

#define MDRV_CPU_CONFIG(config)											\
	if (cpu)															\
		cpu->reset_param = &(config);									\

#define MDRV_CPU_PROGRAM_MAP(readmem, writemem)							\
	if (cpu)															\
	{																	\
		cpu->construct_map[ADDRESS_SPACE_PROGRAM][0] = (construct_map_##readmem); \
		cpu->construct_map[ADDRESS_SPACE_PROGRAM][1] = (construct_map_##writemem); \
	}																	\

#define MDRV_CPU_DATA_MAP(readmem, writemem)							\
	if (cpu)															\
	{																	\
		cpu->construct_map[ADDRESS_SPACE_DATA][0] = (construct_map_##readmem); \
		cpu->construct_map[ADDRESS_SPACE_DATA][1] = (construct_map_##writemem); \
	}																	\

#define MDRV_CPU_IO_MAP(readmem, writemem)								\
	if (cpu)															\
	{																	\
		cpu->construct_map[ADDRESS_SPACE_IO][0] = (construct_map_##readmem); \
		cpu->construct_map[ADDRESS_SPACE_IO][1] = (construct_map_##writemem); \
	}																	\

#define MDRV_CPU_VBLANK_INT(func, rate)									\
	if (cpu)															\
	{																	\
		cpu->vblank_interrupt = func;									\
		cpu->vblank_interrupts_per_frame = (rate);						\
	}																	\

#define MDRV_CPU_PERIODIC_INT(func, rate)								\
	if (cpu)															\
	{																	\
		cpu->timed_interrupt = func;									\
		cpu->timed_interrupt_period = HZ_TO_ATTOSECONDS(rate);			\
	}																	\


/* core parameters */
#define MDRV_DRIVER_DATA(_struct)										\
	machine->driver_data_size = sizeof(_struct);						\

#define MDRV_INTERLEAVE(interleave)										\
	machine->cpu_slices_per_frame = (interleave);						\

#define MDRV_WATCHDOG_VBLANK_INIT(watch_count)							\
	machine->watchdog_vblank_count = (watch_count);						\

#define MDRV_WATCHDOG_TIME_INIT(time)									\
	machine->watchdog_time = (time);									\


/* core functions */
#define MDRV_MACHINE_START(name)										\
	machine->machine_start = machine_start_##name;						\

#define MDRV_MACHINE_RESET(name)										\
	machine->machine_reset = machine_reset_##name;						\

#define MDRV_NVRAM_HANDLER(name)										\
	machine->nvram_handler = nvram_handler_##name;						\

#define MDRV_MEMCARD_HANDLER(name)										\
	machine->memcard_handler = memcard_handler_##name;					\


/* core video parameters */
#define MDRV_VIDEO_ATTRIBUTES(flags)									\
	machine->video_attributes = (flags);								\

#define MDRV_GFXDECODE(gfx)												\
	machine->gfxdecodeinfo = (gfxdecodeinfo_##gfx);										\

#define MDRV_PALETTE_LENGTH(length)										\
	machine->total_colors = (length);									\

#define MDRV_COLORTABLE_LENGTH(length)									\
	machine->color_table_len = (length);								\

#define MDRV_DEFAULT_LAYOUT(layout)										\
	machine->default_layout = &(layout)[0];								\


/* core video functions */
#define MDRV_PALETTE_INIT(name)											\
	machine->init_palette = palette_init_##name;						\

#define MDRV_VIDEO_START(name)											\
	machine->video_start = video_start_##name;							\

#define MDRV_VIDEO_RESET(name)											\
	machine->video_reset = video_reset_##name;							\

#define MDRV_VIDEO_EOF(name)											\
	machine->video_eof = video_eof_##name;								\

#define MDRV_VIDEO_UPDATE(name)											\
	machine->video_update = video_update_##name;						\


/* add/remove screens */
#define MDRV_SCREEN_ADD(tag, palbase)									\
	screen = driver_add_screen(machine, (tag), (palbase));				\

#define MDRV_SCREEN_REMOVE(tag)											\
	driver_remove_screen(machine, tag);									\

#define MDRV_SCREEN_MODIFY(tag)											\
	screen = driver_find_screen(machine, tag);							\

#define MDRV_SCREEN_FORMAT(_format)										\
	screen->defstate.format = (_format);								\

#define MDRV_SCREEN_RAW_PARAMS(pixclock, htotal, hbend, hbstart, vtotal, vbend, vbstart) \
	screen->defstate.refresh = HZ_TO_ATTOSECONDS(pixclock) * (htotal) * (vtotal); \
	screen->defstate.vblank = (screen->defstate.refresh / (vtotal)) * ((vtotal) - ((vbstart) - (vbend))); \
	screen->defstate.width = (htotal);									\
	screen->defstate.height = (vtotal);									\
	screen->defstate.visarea.min_x = (hbend);							\
	screen->defstate.visarea.max_x = (hbstart) - 1;						\
	screen->defstate.visarea.min_y = (vbend);							\
	screen->defstate.visarea.max_y = (vbstart) - 1;						\

#define MDRV_SCREEN_REFRESH_RATE(rate)									\
	screen->defstate.refresh = HZ_TO_ATTOSECONDS(rate);					\

#define MDRV_SCREEN_VBLANK_TIME(time)									\
	screen->defstate.vblank = time;										\
	screen->defstate.oldstyle_vblank_supplied = 1;						\

#define MDRV_SCREEN_SIZE(_width, _height)								\
	screen->defstate.width = (_width);									\
	screen->defstate.height = (_height);								\

#define MDRV_SCREEN_VISIBLE_AREA(minx, maxx, miny, maxy)				\
	screen->defstate.visarea.min_x = (minx);							\
	screen->defstate.visarea.max_x = (maxx);							\
	screen->defstate.visarea.min_y = (miny);							\
	screen->defstate.visarea.max_y = (maxy);							\

#define MDRV_SCREEN_DEFAULT_POSITION(_xscale, _xoffs, _yscale, _yoffs)	\
	screen->xoffset = (_xoffs);											\
	screen->xscale = (_xscale);											\
	screen->yoffset = (_yoffs);											\
	screen->yscale = (_yscale);											\


/* add/remove speakers */
#define MDRV_SPEAKER_ADD(tag, x, y, z)									\
	driver_add_speaker(machine, (tag), (x), (y), (z));					\

#define MDRV_SPEAKER_REMOVE(tag)										\
	driver_remove_speaker(machine, (tag));								\

#define MDRV_SPEAKER_STANDARD_MONO(tag)									\
	MDRV_SPEAKER_ADD(tag, 0.0, 0.0, 1.0)								\

#define MDRV_SPEAKER_STANDARD_STEREO(tagl, tagr)						\
	MDRV_SPEAKER_ADD(tagl, -0.2, 0.0, 1.0)								\
	MDRV_SPEAKER_ADD(tagr, 0.2, 0.0, 1.0)								\


/* core sound functions */
#define MDRV_SOUND_START(name)											\
	machine->sound_start = sound_start_##name;							\

#define MDRV_SOUND_RESET(name)											\
	machine->sound_reset = sound_reset_##name;							\


/* add/remove/replace sounds */
#define MDRV_SOUND_ADD_TAG(tag, type, clock)							\
	sound = driver_add_sound(machine, (tag), SOUND_##type, (clock));	\

#define MDRV_SOUND_ADD(type, clock)										\
	MDRV_SOUND_ADD_TAG(NULL, type, clock)								\

#define MDRV_SOUND_REMOVE(tag)											\
	driver_remove_sound(machine, tag);									\

#define MDRV_SOUND_MODIFY(tag)											\
	sound = driver_find_sound(machine, tag);							\
	sound->routes = 0;													\

#define MDRV_SOUND_CONFIG(_config)										\
	if (sound)															\
		sound->config = &(_config);										\

#define MDRV_SOUND_REPLACE(tag, _type, _clock)							\
	sound = driver_find_sound(machine, tag);							\
	if (sound)															\
	{																	\
		sound->type = SOUND_##_type;								\
		sound->clock = (_clock);										\
		sound->config = NULL;											\
		sound->routes = 0;												\
	}																	\

#define MDRV_SOUND_ROUTE_EX(_output, _target, _gain, _input)			\
	if (sound)															\
	{																	\
		sound->route[sound->routes].output = (_output);					\
		sound->route[sound->routes].target = (_target);					\
		sound->route[sound->routes].gain = (_gain);						\
		sound->route[sound->routes].input = (_input);					\
		sound->routes++;												\
	}																	\

#define MDRV_SOUND_ROUTE(_output, _target, _gain)						\
	MDRV_SOUND_ROUTE_EX(_output, _target, _gain, -1)					\


/***************************************************************************
    MACROS FOR BUILDING GAME DRIVERS
***************************************************************************/

#define GAME(YEAR,NAME,PARENT,MACHINE,INPUT,INIT,MONITOR,COMPANY,FULLNAME,FLAGS)	\
	GAMEL(YEAR,NAME,PARENT,MACHINE,INPUT,INIT,MONITOR,COMPANY,FULLNAME,FLAGS,((const char *)0))

#define GAMEL(YEAR,NAME,PARENT,MACHINE,INPUT,INIT,MONITOR,COMPANY,FULLNAME,FLAGS,LAYOUT)	\
const game_driver driver_##NAME =					\
{											\
	__FILE__,								\
	#PARENT,								\
	#NAME,									\
	FULLNAME,								\
	#YEAR,									\
	COMPANY,								\
	construct_##MACHINE,					\
	ipt_##INPUT,							\
	driver_init_##INIT,						\
	rom_##NAME,								\
	(MONITOR)|(FLAGS),						\
	&LAYOUT[0]								\
};

/* this allows to leave the INIT field empty in the GAME() macro call */
#define driver_init_0 0



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

extern const game_driver * const drivers[];

extern const game_driver driver_empty;


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

void expand_machine_driver(void (*constructor)(machine_config *), machine_config *output);

cpu_config *driver_add_cpu(machine_config *machine, const char *tag, cpu_type type, int cpuclock);
cpu_config *driver_find_cpu(machine_config *machine, const char *tag);
void driver_remove_cpu(machine_config *machine, const char *tag);

speaker_config *driver_add_speaker(machine_config *machine, const char *tag, float x, float y, float z);
speaker_config *driver_find_speaker(machine_config *machine, const char *tag);
void driver_remove_speaker(machine_config *machine, const char *tag);

sound_config *driver_add_sound(machine_config *machine, const char *tag, sound_type type, int clock);
sound_config *driver_find_sound(machine_config *machine, const char *tag);
void driver_remove_sound(machine_config *machine, const char *tag);

screen_config *driver_add_screen(machine_config *machine, const char *tag, int palbase);
screen_config *driver_find_screen(machine_config *machine, const char *tag);
void driver_remove_screen(machine_config *machine, const char *tag);

const game_driver *driver_get_name(const char *name);
const game_driver *driver_get_clone(const game_driver *driver);

void driver_list_get_approx_matches(const game_driver * const driverlist[], const char *name, int matches, const game_driver **list);
int driver_list_get_count(const game_driver * const driverlist[]);


#endif	/* __DRIVER_H__ */
