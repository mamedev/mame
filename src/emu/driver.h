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

#include "devintrf.h"

#define DRIVER_INIT(name)			void driver_init_##name(running_machine *machine)
#define DRIVER_INIT_CALL(name)		driver_init_##name(machine)

#define NVRAM_HANDLER(name)			void nvram_handler_##name(running_machine *machine, mame_file *file, int read_or_write)
#define NVRAM_HANDLER_CALL(name)	nvram_handler_##name(machine, file, read_or_write)

#define MEMCARD_HANDLER(name)		void memcard_handler_##name(running_machine *machine, mame_file *file, int action)
#define MEMCARD_HANDLER_CALL(name)	memcard_handler_##name(machine, file, action)

#define MACHINE_START(name)			void machine_start_##name(running_machine *machine)
#define MACHINE_START_CALL(name)	machine_start_##name(machine)

#define MACHINE_RESET(name)			void machine_reset_##name(running_machine *machine)
#define MACHINE_RESET_CALL(name)	machine_reset_##name(machine)

#define SOUND_START(name)			void sound_start_##name(running_machine *machine)
#define SOUND_START_CALL(name)		sound_start_##name(machine)

#define SOUND_RESET(name)			void sound_reset_##name(running_machine *machine)
#define SOUND_RESET_CALL(name)		sound_reset_##name(machine)

#define VIDEO_START(name)			void video_start_##name(running_machine *machine)
#define VIDEO_START_CALL(name)		video_start_##name(machine)

#define VIDEO_RESET(name)			void video_reset_##name(running_machine *machine)
#define VIDEO_RESET_CALL(name)		video_reset_##name(machine)

#define PALETTE_INIT(name)			void palette_init_##name(running_machine *machine, const UINT8 *color_prom)
#define PALETTE_INIT_CALL(name)		palette_init_##name(machine, color_prom)

#define VIDEO_EOF(name)				void video_eof_##name(running_machine *machine)
#define VIDEO_EOF_CALL(name)		video_eof_##name(machine)

#define VIDEO_UPDATE(name)			UINT32 video_update_##name(const device_config *screen, bitmap_t *bitmap, const rectangle *cliprect)
#define VIDEO_UPDATE_CALL(name)		video_update_##name(screen, bitmap, cliprect)


/* NULL versions */
#define nvram_handler_NULL 			NULL
#define memcard_handler_NULL		NULL
#define machine_start_NULL 			NULL
#define machine_reset_NULL 			NULL
#define sound_start_NULL 			NULL
#define sound_reset_NULL 			NULL
#define video_start_NULL 			NULL
#define video_reset_NULL 			NULL
#define palette_init_NULL			NULL
#define video_eof_NULL 				NULL
#define video_update_NULL 			NULL


typedef void   (*driver_init_func)(running_machine *machine);
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
typedef UINT32 (*video_update_func)(const device_config *screen, bitmap_t *bitmap, const rectangle *cliprect);



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
#include "mconfig.h"
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



/* ----- flags for game drivers ----- */

#define ORIENTATION_MASK        		0x00000007
#define GAME_NOT_WORKING				0x00000008
#define GAME_UNEMULATED_PROTECTION		0x00000010	/* game's protection not fully emulated */
#define GAME_WRONG_COLORS				0x00000020	/* colors are totally wrong */
#define GAME_IMPERFECT_COLORS			0x00000040	/* colors are not 100% accurate, but close */
#define GAME_IMPERFECT_GRAPHICS			0x00000080	/* graphics are wrong/incomplete */
#define GAME_NO_COCKTAIL				0x00000100	/* screen flip support is missing */
#define GAME_NO_SOUND					0x00000200	/* sound is missing */
#define GAME_IMPERFECT_SOUND			0x00000400	/* sound is known to be wrong */
#define GAME_SUPPORTS_SAVE				0x00000800	/* game supports save states */
#define GAME_IS_BIOS_ROOT				0x00001000	/* this driver entry is a BIOS root */
#define GAME_NO_STANDALONE				0x00002000	/* this driver cannot stand alone */
#define GAME_REQUIRES_ARTWORK			0x00004000	/* the driver requires external artwork for key elements of the game */


#ifdef MESS
#define GAME_COMPUTER_MODIFIED      	0x00008000	/* Official? Hack */
#define GAME_COMPUTER               	0x00010000  /* Driver is a computer (needs full keyboard) */
#endif



/* ----- flags to return from video_update ----- */
#define UPDATE_HAS_NOT_CHANGED			0x0001	/* the video has not changed */



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* In mamecore.h: typedef struct _game_driver game_driver; */
struct _game_driver
{
	const char *		source_file;				/* set this to __FILE__ */
	const char *		parent;						/* if this is a clone, the name of the parent */
	const char *		name;						/* short (8-character) name of the game */
	const char *		description;				/* full name of the game */
	const char *		year;						/* year the game was released */
	const char *		manufacturer;				/* manufacturer of the game */
	const machine_config_token *machine_config;		/* machine driver tokens */
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
	machine_config_##MACHINE,				\
	ipt_##INPUT,							\
	driver_init_##INIT,						\
	rom_##NAME,								\
	(MONITOR)|(FLAGS),						\
	&LAYOUT[0]								\
};

/* this allows to leave the INIT field empty in the GAME() macro call */
#define driver_init_0 0
#define ipt_0 0



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

extern const game_driver * const drivers[];

extern const game_driver driver_empty;



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

const game_driver *driver_get_name(const char *name);
const game_driver *driver_get_clone(const game_driver *driver);

void driver_list_get_approx_matches(const game_driver * const driverlist[], const char *name, int matches, const game_driver **list);
int driver_list_get_count(const game_driver * const driverlist[]);


#endif	/* __DRIVER_H__ */
