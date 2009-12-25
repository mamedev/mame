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

#define DRIVER_INIT_NAME(name)		driver_init_##name
#define DRIVER_INIT(name)			void DRIVER_INIT_NAME(name)(running_machine *machine)
#define DRIVER_INIT_CALL(name)		DRIVER_INIT_NAME(name)(machine)

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
#define VIDEO_UPDATE(name)			UINT32 VIDEO_UPDATE_NAME(name)(const device_config *screen, bitmap_t *bitmap, const rectangle *cliprect)
#define VIDEO_UPDATE_CALL(name)		VIDEO_UPDATE_NAME(name)(screen, bitmap, cliprect)


/* NULL versions */
#define driver_init_0				NULL
#define nvram_handler_0 			NULL
#define memcard_handler_0			NULL
#define machine_start_0 			NULL
#define machine_reset_0 			NULL
#define sound_start_0 				NULL
#define sound_reset_0 				NULL
#define video_start_0 				NULL
#define video_reset_0 				NULL
#define palette_init_0				NULL
#define video_eof_0 				NULL
#define video_update_0 				NULL


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
	const char *		compatible_with;
	UINT32				flags;						/* orientation and other flags; see defines below */
	const char *		default_layout;				/* default internally defined layout */
};



/***************************************************************************
    MACROS FOR BUILDING GAME DRIVERS
***************************************************************************/

#define GAME_NAME(name) driver_##name
#define GAME_EXTERN(name) extern const game_driver GAME_NAME(name)

#define GAME(YEAR,NAME,PARENT,MACHINE,INPUT,INIT,MONITOR,COMPANY,FULLNAME,FLAGS)	\
	GAMEL(YEAR,NAME,PARENT,MACHINE,INPUT,INIT,MONITOR,COMPANY,FULLNAME,FLAGS,((const char *)0))

#define GAMEL(YEAR,NAME,PARENT,MACHINE,INPUT,INIT,MONITOR,COMPANY,FULLNAME,FLAGS,LAYOUT)	\
const game_driver GAME_NAME(NAME) =			\
{											\
	__FILE__,								\
	#PARENT,								\
	#NAME,									\
	FULLNAME,								\
	#YEAR,									\
	COMPANY,								\
	MACHINE_DRIVER_NAME(MACHINE),				\
	INPUT_PORTS_NAME(INPUT),							\
	DRIVER_INIT_NAME(INIT),						\
	ROM_NAME(NAME),								\
	NULL,									\
	(MONITOR)|(FLAGS),						\
	&LAYOUT[0]								\
};



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

extern const game_driver * const drivers[];

GAME_EXTERN(empty);



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

const game_driver *driver_get_name(const char *name);
const game_driver *driver_get_clone(const game_driver *driver);

void driver_list_get_approx_matches(const game_driver * const driverlist[], const char *name, int matches, const game_driver **list);
int driver_list_get_count(const game_driver * const driverlist[]);


#endif	/* __DRIVER_H__ */
