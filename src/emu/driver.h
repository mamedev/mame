/***************************************************************************

    driver.h

    Definitions relating to game drivers.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef __DRIVER_H__
#define __DRIVER_H__


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
#define GAME_UNOFFICIAL     			0x00008000	/* unofficial hardware change */
#define GAME_NO_SOUND_HW				0x00010000	/* sound hardware not available */


/* ----- flags to return from video_update ----- */
#define UPDATE_HAS_NOT_CHANGED			0x0001	/* the video has not changed */



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef void   (*driver_init_func)(running_machine *machine);


struct game_driver
{
	const char *		source_file;				/* set this to __FILE__ */
	const char *		parent;						/* if this is a clone, the name of the parent */
	const char *		name;						/* short (8-character) name of the game */
	const char *		description;				/* full name of the game */
	const char *		year;						/* year the game was released */
	const char *		manufacturer;				/* manufacturer of the game */
	machine_config_constructor machine_config;		/* machine driver tokens */
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


#define DRIVER_INIT_NAME(name)		driver_init_##name
#define DRIVER_INIT(name)			void DRIVER_INIT_NAME(name)(running_machine *machine)
#define DRIVER_INIT_CALL(name)		DRIVER_INIT_NAME(name)(machine)

#define driver_init_0				NULL


#define GAME_NAME(name) driver_##name
#define GAME_EXTERN(name) extern const game_driver GAME_NAME(name)

#define GAME(YEAR,NAME,PARENT,MACHINE,INPUT,INIT,MONITOR,COMPANY,FULLNAME,FLAGS)	\
	GAMEL(YEAR,NAME,PARENT,MACHINE,INPUT,INIT,MONITOR,COMPANY,FULLNAME,FLAGS,((const char *)0))

#define GAMEL(YEAR,NAME,PARENT,MACHINE,INPUT,INIT,MONITOR,COMPANY,FULLNAME,FLAGS,LAYOUT)	\
extern const game_driver GAME_NAME(NAME) =	\
{											\
	__FILE__,								\
	#PARENT,								\
	#NAME,									\
	FULLNAME,								\
	#YEAR,									\
	COMPANY,								\
	MACHINE_DRIVER_NAME(MACHINE),			\
	INPUT_PORTS_NAME(INPUT),				\
	DRIVER_INIT_NAME(INIT),					\
	ROM_NAME(NAME),							\
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
const game_driver *driver_get_compatible(const game_driver *drv);

void driver_list_get_approx_matches(const game_driver * const driverlist[], const char *name, int matches, const game_driver **list);
int driver_list_get_count(const game_driver * const driverlist[]);


#endif
