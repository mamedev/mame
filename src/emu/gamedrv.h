// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    gamedrv.h

    Definitions for game drivers.

***************************************************************************/

#ifndef MAME_EMU_GAMEDRV_H
#define MAME_EMU_GAMEDRV_H

#pragma once


//**************************************************************************
//  CONSTANTS
//**************************************************************************

// maxima
constexpr int MAX_DRIVER_NAME_CHARS = 16;

// flags for game drivers
constexpr u32 ORIENTATION_MASK                  = 0x00000007;
constexpr u32 MACHINE_NOT_WORKING               = 0x00000008;
constexpr u32 MACHINE_UNEMULATED_PROTECTION     = 0x00000010;   // game's protection not fully emulated
constexpr u32 MACHINE_WRONG_COLORS              = 0x00000020;   // colors are totally wrong
constexpr u32 MACHINE_IMPERFECT_COLORS          = 0x00000040;   // colors are not 100% accurate, but close
constexpr u32 MACHINE_IMPERFECT_GRAPHICS        = 0x00000080;   // graphics are wrong/incomplete
constexpr u32 MACHINE_NO_COCKTAIL               = 0x00000100;   // screen flip support is missing
constexpr u32 MACHINE_NO_SOUND                  = 0x00000200;   // sound is missing
constexpr u32 MACHINE_IMPERFECT_SOUND           = 0x00000400;   // sound is known to be wrong
constexpr u32 MACHINE_SUPPORTS_SAVE             = 0x00000800;   // game supports save states
constexpr u32 MACHINE_IS_BIOS_ROOT              = 0x00001000;   // this driver entry is a BIOS root
constexpr u32 MACHINE_NO_STANDALONE             = 0x00002000;   // this driver cannot stand alone
constexpr u32 MACHINE_REQUIRES_ARTWORK          = 0x00004000;   // the driver requires external artwork for key elements of the game
constexpr u32 MACHINE_UNOFFICIAL                = 0x00008000;   // unofficial hardware change
constexpr u32 MACHINE_NO_SOUND_HW               = 0x00010000;   // sound hardware not available
constexpr u32 MACHINE_MECHANICAL                = 0x00020000;   // contains mechanical parts (pinball, redemption games,...)
constexpr u32 MACHINE_TYPE_ARCADE               = 0x00040000;   // arcade machine (coin operated machines)
constexpr u32 MACHINE_TYPE_CONSOLE              = 0x00080000;   // console system
constexpr u32 MACHINE_TYPE_COMPUTER             = 0x00100000;   // any kind of computer including home computers, minis, calcs,...
constexpr u32 MACHINE_TYPE_OTHER                = 0x00200000;   // any other emulated system that doesn't fit above (ex. clock, satellite receiver,...)
constexpr u32 MACHINE_IMPERFECT_KEYBOARD        = 0x00400000;   // keyboard is known to be wrong
constexpr u32 MACHINE_CLICKABLE_ARTWORK         = 0x00800000;   // marking that artwork is clickable and require mouse cursor
constexpr u32 MACHINE_IS_INCOMPLETE             = 0x01000000;   // any official game/system with blatantly incomplete HW or SW should be marked with this
constexpr u32 MACHINE_NODEVICE_MICROPHONE       = 0x02000000;   // any game/system that has unemulated recording voice device peripheral
constexpr u32 MACHINE_NODEVICE_CAMERA           = 0x04000000;   // any game/system that has unemulated capturing image device peripheral
constexpr u32 MACHINE_NODEVICE_PRINTER          = 0x08000000;   // any game/system that has unemulated grabbing of screen content device
constexpr u32 MACHINE_NODEVICE_LAN              = 0x10000000;   // any game/system that has unemulated multi-linking capability
constexpr u32 MACHINE_NODEVICE_WAN              = 0x20000000;   // any game/system that has unemulated networking capability

// useful combinations of flags
constexpr u32 MACHINE_IS_SKELETON               = MACHINE_NO_SOUND | MACHINE_NOT_WORKING; // mask for skelly games
constexpr u32 MACHINE_IS_SKELETON_MECHANICAL    = MACHINE_IS_SKELETON | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK; // mask for skelly mechanical games
constexpr u32 MACHINE_FATAL_FLAGS               = MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION | MACHINE_MECHANICAL; // red disclaimer
constexpr u32 MACHINE_WARNING_FLAGS             = MACHINE_WRONG_COLORS | MACHINE_IMPERFECT_COLORS | MACHINE_REQUIRES_ARTWORK | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_KEYBOARD | MACHINE_NO_SOUND | MACHINE_NO_COCKTAIL | MACHINE_NODEVICE_MICROPHONE | MACHINE_NODEVICE_CAMERA | MACHINE_NODEVICE_PRINTER | MACHINE_NODEVICE_LAN | MACHINE_NODEVICE_WAN;  // yellow disclaimer
constexpr u32 MACHINE_BTANB_FLAGS               = MACHINE_IS_INCOMPLETE | MACHINE_NO_SOUND_HW; // default disclaimer

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// static driver initialization callback
typedef void (*driver_init_func)(running_machine &machine);

// static POD structure describing each game driver entry
struct game_driver
{
	const char *                source_file;        // set this to __FILE__
	const char *                parent;             // if this is a clone, the name of the parent
	const char *                description;        // full name of the game
	const char *                year;               // year the game was released
	const char *                manufacturer;       // manufacturer of the game
	machine_config_constructor  machine_config;     // machine driver tokens
	ioport_constructor          ipt;                // pointer to constructor for input ports
	driver_init_func            driver_init;        // DRIVER_INIT callback
	const tiny_rom_entry *      rom;                // pointer to list of ROMs for the game
	const char *                compatible_with;
	const internal_layout *     default_layout;     // default internally defined layout
	u32                         flags;              // orientation and other flags; see defines above
	char                        name[MAX_DRIVER_NAME_CHARS + 1]; // short name of the game
};


//**************************************************************************
//  MACROS
//**************************************************************************

// wrappers for the DRIVER_INIT callback
#define DRIVER_INIT_NAME(name)          init_##name
#define DECLARE_DRIVER_INIT(name)       void DRIVER_INIT_NAME(name)() ATTR_COLD
#define DRIVER_INIT_MEMBER(cls, name)   void cls::DRIVER_INIT_NAME(name)()
#define DRIVER_INIT_CALL(name)          DRIVER_INIT_NAME(name)()

// wrappers for declaring and defining game drivers
#define GAME_NAME(name)         driver_##name
#define GAME_EXTERN(name)       extern game_driver const GAME_NAME(name)

// standard GAME() macro
#define GAME(YEAR,NAME,PARENT,MACHINE,INPUT,CLASS,INIT,MONITOR,COMPANY,FULLNAME,FLAGS) \
extern game_driver const GAME_NAME(NAME)                                \
{                                                                       \
	__FILE__,                                                           \
	#PARENT,                                                            \
	FULLNAME,                                                           \
	#YEAR,                                                              \
	COMPANY,                                                            \
	MACHINE_CONFIG_NAME(MACHINE),                                       \
	INPUT_PORTS_NAME(INPUT),                                            \
	&driver_device::driver_init_wrapper<CLASS, &CLASS::init_##INIT>,    \
	ROM_NAME(NAME),                                                     \
	nullptr,                                                            \
	nullptr,                                                            \
	(MONITOR) | (FLAGS) | MACHINE_TYPE_ARCADE,                          \
	#NAME                                                               \
};

// standard macro with additional layout
#define GAMEL(YEAR,NAME,PARENT,MACHINE,INPUT,CLASS,INIT,MONITOR,COMPANY,FULLNAME,FLAGS,LAYOUT) \
extern game_driver const GAME_NAME(NAME)                                \
{                                                                       \
	__FILE__,                                                           \
	#PARENT,                                                            \
	FULLNAME,                                                           \
	#YEAR,                                                              \
	COMPANY,                                                            \
	MACHINE_CONFIG_NAME(MACHINE),                                       \
	INPUT_PORTS_NAME(INPUT),                                            \
	&driver_device::driver_init_wrapper<CLASS, &CLASS::init_##INIT>,    \
	ROM_NAME(NAME),                                                     \
	nullptr,                                                            \
	&LAYOUT,                                                            \
	(MONITOR) | (FLAGS) | MACHINE_TYPE_ARCADE,                          \
	#NAME                                                               \
};


// standard console definition macro
#define CONS(YEAR,NAME,PARENT,COMPAT,MACHINE,INPUT,CLASS,INIT,COMPANY,FULLNAME,FLAGS) \
extern game_driver const GAME_NAME(NAME)                                \
{                                                                       \
	__FILE__,                                                           \
	#PARENT,                                                            \
	FULLNAME,                                                           \
	#YEAR,                                                              \
	COMPANY,                                                            \
	MACHINE_CONFIG_NAME(MACHINE),                                       \
	INPUT_PORTS_NAME(INPUT),                                            \
	&driver_device::driver_init_wrapper<CLASS, &CLASS::init_##INIT>,    \
	ROM_NAME(NAME),                                                     \
	#COMPAT,                                                            \
	nullptr,                                                            \
	ROT0 | (FLAGS) | MACHINE_TYPE_CONSOLE,                              \
	#NAME                                                               \
};

// standard computer definition macro
#define COMP(YEAR,NAME,PARENT,COMPAT,MACHINE,INPUT,CLASS,INIT,COMPANY,FULLNAME,FLAGS) \
extern game_driver const GAME_NAME(NAME)                                \
{                                                                       \
	__FILE__,                                                           \
	#PARENT,                                                            \
	FULLNAME,                                                           \
	#YEAR,                                                              \
	COMPANY,                                                            \
	MACHINE_CONFIG_NAME(MACHINE),                                       \
	INPUT_PORTS_NAME(INPUT),                                            \
	&driver_device::driver_init_wrapper<CLASS, &CLASS::init_##INIT>,    \
	ROM_NAME(NAME),                                                     \
	#COMPAT,                                                            \
	nullptr,                                                            \
	ROT0 | (FLAGS) | MACHINE_TYPE_COMPUTER,                             \
	#NAME                                                               \
};

// standard system definition macro
#define SYST(YEAR,NAME,PARENT,COMPAT,MACHINE,INPUT,CLASS,INIT,COMPANY,FULLNAME,FLAGS) \
extern game_driver const GAME_NAME(NAME)                                \
{                                                                       \
	__FILE__,                                                           \
	#PARENT,                                                            \
	FULLNAME,                                                           \
	#YEAR,                                                              \
	COMPANY,                                                            \
	MACHINE_CONFIG_NAME(MACHINE),                                       \
	INPUT_PORTS_NAME(INPUT),                                            \
	&driver_device::driver_init_wrapper<CLASS, &CLASS::init_##INIT>,    \
	ROM_NAME(NAME),                                                     \
	#COMPAT,                                                            \
	nullptr,                                                            \
	ROT0 | (FLAGS) | MACHINE_TYPE_OTHER,                                \
	#NAME                                                               \
};


#endif // MAME_EMU_GAMEDRV_H
