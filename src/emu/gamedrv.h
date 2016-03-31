// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    gamedrv.h

    Definitions for game drivers.

***************************************************************************/

#pragma once

#ifndef __GAMEDRV_H__
#define __GAMEDRV_H__


//**************************************************************************
//  CONSTANTS
//**************************************************************************

// maxima
const int MAX_DRIVER_NAME_CHARS = 8;

// flags for game drivers
const UINT32 ORIENTATION_MASK                  = 0x00000007;
const UINT32 MACHINE_NOT_WORKING               = 0x00000008;
const UINT32 MACHINE_UNEMULATED_PROTECTION     = 0x00000010;   // game's protection not fully emulated
const UINT32 MACHINE_WRONG_COLORS              = 0x00000020;   // colors are totally wrong
const UINT32 MACHINE_IMPERFECT_COLORS          = 0x00000040;   // colors are not 100% accurate, but close
const UINT32 MACHINE_IMPERFECT_GRAPHICS        = 0x00000080;   // graphics are wrong/incomplete
const UINT32 MACHINE_NO_COCKTAIL               = 0x00000100;   // screen flip support is missing
const UINT32 MACHINE_NO_SOUND                  = 0x00000200;   // sound is missing
const UINT32 MACHINE_IMPERFECT_SOUND           = 0x00000400;   // sound is known to be wrong
const UINT32 MACHINE_SUPPORTS_SAVE             = 0x00000800;   // game supports save states
const UINT32 MACHINE_IS_BIOS_ROOT              = 0x00001000;   // this driver entry is a BIOS root
const UINT32 MACHINE_NO_STANDALONE             = 0x00002000;   // this driver cannot stand alone
const UINT32 MACHINE_REQUIRES_ARTWORK          = 0x00004000;   // the driver requires external artwork for key elements of the game
const UINT32 MACHINE_UNOFFICIAL                = 0x00008000;   // unofficial hardware change
const UINT32 MACHINE_NO_SOUND_HW               = 0x00010000;   // sound hardware not available
const UINT32 MACHINE_MECHANICAL                = 0x00020000;   // contains mechanical parts (pinball, redemption games,...)
const UINT32 MACHINE_TYPE_ARCADE               = 0x00040000;   // arcade machine (coin operated machines)
const UINT32 MACHINE_TYPE_CONSOLE              = 0x00080000;   // console system
const UINT32 MACHINE_TYPE_COMPUTER             = 0x00100000;   // any kind of computer including home computers, minis, calcs,...
const UINT32 MACHINE_TYPE_OTHER                = 0x00200000;   // any other emulated system that doesn't fit above (ex. clock, satelite receiver,...)
const UINT32 MACHINE_IMPERFECT_KEYBOARD        = 0x00400000;   // keyboard is known to be wrong
const UINT32 MACHINE_CLICKABLE_ARTWORK         = 0x00800000;   // marking that artwork is clickable and require mouse cursor
const UINT32 MACHINE_IS_INCOMPLETE             = 0x01000000;   // any official game/system with blantantly incomplete HW or SW should be marked with this

// useful combinations of flags
const UINT32 MACHINE_IS_SKELETON               = MACHINE_NO_SOUND | MACHINE_NOT_WORKING; // mask for skelly games
const UINT32 MACHINE_IS_SKELETON_MECHANICAL    = MACHINE_IS_SKELETON | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK; // mask for skelly mechanical games



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// static driver initialization callback
typedef void (*driver_init_func)(running_machine &machine);

// static POD structure describing each game driver entry
struct game_driver
{
	const char *        source_file;                // set this to __FILE__
	const char *        parent;                     // if this is a clone, the name of the parent
	const char *        name;                       // short (8-character) name of the game
	const char *        description;                // full name of the game
	const char *        year;                       // year the game was released
	const char *        manufacturer;               // manufacturer of the game
	machine_config_constructor machine_config;      // machine driver tokens
	ioport_constructor  ipt;                        // pointer to constructor for input ports
	void                (*driver_init)(running_machine &machine); // DRIVER_INIT callback
	const rom_entry *   rom;                        // pointer to list of ROMs for the game
	const char *        compatible_with;
	UINT32              flags;                      // orientation and other flags; see defines below
	const internal_layout *        default_layout;             // default internally defined layout
};



//**************************************************************************
//  MACROS
//**************************************************************************

// wrappers for the DRIVER_INIT callback
#define DRIVER_INIT_NAME(name)      init_##name
#define DECLARE_DRIVER_INIT(name)   void DRIVER_INIT_NAME(name)() ATTR_COLD
#define DRIVER_INIT_MEMBER(cls,name) void cls::DRIVER_INIT_NAME(name)()
#define DRIVER_INIT_CALL(name)      DRIVER_INIT_NAME(name)()

// wrappers for declaring and defining game drivers
#define GAME_NAME(name) driver_##name
#define GAME_EXTERN(name) extern const game_driver GAME_NAME(name)

// standard GAME() macro
#define GAME(YEAR,NAME,PARENT,MACHINE,INPUT,CLASS,INIT,MONITOR,COMPANY,FULLNAME,FLAGS)  \
extern const game_driver GAME_NAME(NAME) =  \
{                                           \
	__FILE__,                               \
	#PARENT,                                \
	#NAME,                                  \
	FULLNAME,                               \
	#YEAR,                                  \
	COMPANY,                                \
	MACHINE_CONFIG_NAME(MACHINE),           \
	INPUT_PORTS_NAME(INPUT),                \
	&driver_device::driver_init_wrapper<CLASS, &CLASS::init_##INIT>,    \
	ROM_NAME(NAME),                         \
	nullptr,                                   \
	(MONITOR)|(FLAGS)|MACHINE_TYPE_ARCADE,     \
	nullptr                             \
};

// standard macro with additional layout
#define GAMEL(YEAR,NAME,PARENT,MACHINE,INPUT,CLASS,INIT,MONITOR,COMPANY,FULLNAME,FLAGS,LAYOUT)  \
extern const game_driver GAME_NAME(NAME) =  \
{                                           \
	__FILE__,                               \
	#PARENT,                                \
	#NAME,                                  \
	FULLNAME,                               \
	#YEAR,                                  \
	COMPANY,                                \
	MACHINE_CONFIG_NAME(MACHINE),           \
	INPUT_PORTS_NAME(INPUT),                \
	&driver_device::driver_init_wrapper<CLASS, &CLASS::init_##INIT>,    \
	ROM_NAME(NAME),                         \
	nullptr,                                   \
	(MONITOR)|(FLAGS)|MACHINE_TYPE_ARCADE,     \
	&LAYOUT                              \
};

// standard console definition macro
#define CONS(YEAR,NAME,PARENT,COMPAT,MACHINE,INPUT,CLASS,INIT,COMPANY,FULLNAME,FLAGS)   \
extern const game_driver GAME_NAME(NAME) =  \
{                                           \
	__FILE__,                               \
	#PARENT,                                \
	#NAME,                                  \
	FULLNAME,                               \
	#YEAR,                                  \
	COMPANY,                                \
	MACHINE_CONFIG_NAME(MACHINE),           \
	INPUT_PORTS_NAME(INPUT),                \
	&driver_device::driver_init_wrapper<CLASS, &CLASS::init_##INIT>,    \
	ROM_NAME(NAME),                         \
	#COMPAT,                                \
	ROT0|(FLAGS)|MACHINE_TYPE_CONSOLE,         \
	nullptr                                    \
};

// standard computer definition macro
#define COMP(YEAR,NAME,PARENT,COMPAT,MACHINE,INPUT,CLASS,INIT,COMPANY,FULLNAME,FLAGS)   \
extern const game_driver GAME_NAME(NAME) =  \
{                                           \
	__FILE__,                               \
	#PARENT,                                \
	#NAME,                                  \
	FULLNAME,                               \
	#YEAR,                                  \
	COMPANY,                                \
	MACHINE_CONFIG_NAME(MACHINE),           \
	INPUT_PORTS_NAME(INPUT),                \
	&driver_device::driver_init_wrapper<CLASS, &CLASS::init_##INIT>,    \
	ROM_NAME(NAME),                         \
	#COMPAT,                                \
	ROT0|(FLAGS)|MACHINE_TYPE_COMPUTER,        \
	nullptr                                    \
};

// standard system definition macro
#define SYST(YEAR,NAME,PARENT,COMPAT,MACHINE,INPUT,CLASS,INIT,COMPANY,FULLNAME,FLAGS)   \
extern const game_driver GAME_NAME(NAME) =  \
{                                           \
	__FILE__,                               \
	#PARENT,                                \
	#NAME,                                  \
	FULLNAME,                               \
	#YEAR,                                  \
	COMPANY,                                \
	MACHINE_CONFIG_NAME(MACHINE),           \
	INPUT_PORTS_NAME(INPUT),                \
	&driver_device::driver_init_wrapper<CLASS, &CLASS::init_##INIT>,    \
	ROM_NAME(NAME),                         \
	#COMPAT,                                \
	ROT0|(FLAGS)|MACHINE_TYPE_OTHER,           \
	nullptr                                    \
};


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

GAME_EXTERN(___empty);

#endif
