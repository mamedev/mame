// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    gamedrv.h

    Definitions for game drivers.

***************************************************************************/

#ifndef MAME_EMU_GAMEDRV_H
#define MAME_EMU_GAMEDRV_H

#pragma once

#include <type_traits>


//**************************************************************************
//  CONSTANTS
//**************************************************************************

// maxima
constexpr int MAX_DRIVER_NAME_CHARS = 16;

struct machine_flags
{
	enum type : u32
	{
		MASK_ORIENTATION    = 0x00000007,
		MASK_TYPE           = 0x00000038,

		FLIP_X              = 0x00000001,
		FLIP_Y              = 0x00000002,
		SWAP_XY             = 0x00000004,
		ROT0                = 0x00000000,
		ROT90               = FLIP_X | SWAP_XY,
		ROT180              = FLIP_X | FLIP_Y,
		ROT270              = FLIP_Y | SWAP_XY,

		TYPE_ARCADE         = 0x00000008,   // coin-operated machine for public use
		TYPE_CONSOLE        = 0x00000010,   // console system
		TYPE_COMPUTER       = 0x00000018,   // any kind of computer including home computers, minis, calculators, ...
		TYPE_OTHER          = 0x00000038,   // any other emulated system (e.g. clock, satellite receiver, ...)

		NOT_WORKING         = 0x00000040,
		SUPPORTS_SAVE       = 0x00000080,   // system supports save states
		NO_COCKTAIL         = 0x00000100,   // screen flip support is missing
		IS_BIOS_ROOT        = 0x00000200,   // this driver entry is a BIOS root
		REQUIRES_ARTWORK    = 0x00000400,   // requires external artwork for key game elements
		CLICKABLE_ARTWORK   = 0x00000800,   // artwork is clickable and requires mouse cursor
		UNOFFICIAL          = 0x00001000,   // unofficial hardware modification
		NO_SOUND_HW         = 0x00002000,   // system has no sound output
		MECHANICAL          = 0x00004000,   // contains mechanical parts (pinball, redemption games, ...)
		IS_INCOMPLETE       = 0x00008000    // official system with blatantly incomplete hardware/software
	};
};

DECLARE_ENUM_BITWISE_OPERATORS(machine_flags::type);


// flags for machine drivers
constexpr u64 MACHINE_TYPE_ARCADE               = machine_flags::TYPE_ARCADE;
constexpr u64 MACHINE_TYPE_CONSOLE              = machine_flags::TYPE_CONSOLE;
constexpr u64 MACHINE_TYPE_COMPUTER             = machine_flags::TYPE_COMPUTER;
constexpr u64 MACHINE_TYPE_OTHER                = machine_flags::TYPE_OTHER;
constexpr u64 MACHINE_NOT_WORKING               = machine_flags::NOT_WORKING;
constexpr u64 MACHINE_SUPPORTS_SAVE             = machine_flags::SUPPORTS_SAVE;
constexpr u64 MACHINE_NO_COCKTAIL               = machine_flags::NO_COCKTAIL;
constexpr u64 MACHINE_IS_BIOS_ROOT              = machine_flags::IS_BIOS_ROOT;
constexpr u64 MACHINE_REQUIRES_ARTWORK          = machine_flags::REQUIRES_ARTWORK;
constexpr u64 MACHINE_CLICKABLE_ARTWORK         = machine_flags::CLICKABLE_ARTWORK;
constexpr u64 MACHINE_UNOFFICIAL                = machine_flags::UNOFFICIAL;
constexpr u64 MACHINE_NO_SOUND_HW               = machine_flags::NO_SOUND_HW;
constexpr u64 MACHINE_MECHANICAL                = machine_flags::MECHANICAL;
constexpr u64 MACHINE_IS_INCOMPLETE             = machine_flags::IS_INCOMPLETE;

// flags taht map to device feature flags
constexpr u64 MACHINE_UNEMULATED_PROTECTION     = 0x00000001'00000000;   // game's protection not fully emulated
constexpr u64 MACHINE_WRONG_COLORS              = 0x00000002'00000000;   // colors are totally wrong
constexpr u64 MACHINE_IMPERFECT_COLORS          = 0x00000004'00000000;   // colors are not 100% accurate, but close
constexpr u64 MACHINE_IMPERFECT_GRAPHICS        = 0x00000008'00000000;   // graphics are wrong/incomplete
constexpr u64 MACHINE_NO_SOUND                  = 0x00000010'00000000;   // sound is missing
constexpr u64 MACHINE_IMPERFECT_SOUND           = 0x00000020'00000000;   // sound is known to be wrong
constexpr u64 MACHINE_IMPERFECT_CONTROLS        = 0x00000040'00000000;   // controls are known to be imperfectly emulated
constexpr u64 MACHINE_NODEVICE_MICROPHONE       = 0x00000080'00000000;   // any game/system that has unemulated audio capture device
constexpr u64 MACHINE_NODEVICE_PRINTER          = 0x00000100'00000000;   // any game/system that has unemulated hardcopy output device
constexpr u64 MACHINE_NODEVICE_LAN              = 0x00000200'00000000;   // any game/system that has unemulated local networking
constexpr u64 MACHINE_IMPERFECT_TIMING          = 0x00000400'00000000;   // timing is known to be imperfectly emulated

// useful combinations of flags
constexpr u64 MACHINE_IS_SKELETON               = MACHINE_NO_SOUND | MACHINE_NOT_WORKING; // flag combination for skeleton drivers
constexpr u64 MACHINE_IS_SKELETON_MECHANICAL    = MACHINE_IS_SKELETON | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK; // flag combination for skeleton mechanical machines


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// static POD structure describing each game driver entry
class game_driver
{
public:
	typedef void (*machine_creator_wrapper)(machine_config &, device_t &);
	typedef void (*driver_init_wrapper)(device_t &);

	static constexpr device_t::feature_type unemulated_features(u64 flags)
	{
		return
				((flags & MACHINE_WRONG_COLORS)             ? device_t::feature::PALETTE    : device_t::feature::NONE) |
				((flags & MACHINE_NO_SOUND)                 ? device_t::feature::SOUND      : device_t::feature::NONE) |
				((flags & MACHINE_NODEVICE_MICROPHONE)      ? device_t::feature::MICROPHONE : device_t::feature::NONE) |
				((flags & MACHINE_NODEVICE_PRINTER)         ? device_t::feature::PRINTER    : device_t::feature::NONE) |
				((flags & MACHINE_NODEVICE_LAN)             ? device_t::feature::LAN        : device_t::feature::NONE);
	}

	static constexpr device_t::feature_type imperfect_features(u64 flags)
	{
		return
				((flags & MACHINE_UNEMULATED_PROTECTION)    ? device_t::feature::PROTECTION : device_t::feature::NONE) |
				((flags & MACHINE_IMPERFECT_COLORS)         ? device_t::feature::PALETTE    : device_t::feature::NONE) |
				((flags & MACHINE_IMPERFECT_GRAPHICS)       ? device_t::feature::GRAPHICS   : device_t::feature::NONE) |
				((flags & MACHINE_IMPERFECT_SOUND)          ? device_t::feature::SOUND      : device_t::feature::NONE) |
				((flags & MACHINE_IMPERFECT_CONTROLS)       ? device_t::feature::CONTROLS   : device_t::feature::NONE) |
				((flags & MACHINE_IMPERFECT_TIMING)         ? device_t::feature::TIMING     : device_t::feature::NONE);
	}

	device_type                 type;               // static type info for driver class
	const char *                parent;             // if this is a clone, the name of the parent
	const char *                year;               // year the game was released
	const char *                manufacturer;       // manufacturer of the game
	machine_creator_wrapper     machine_creator;    // machine driver tokens
	ioport_constructor          ipt;                // pointer to constructor for input ports
	driver_init_wrapper         driver_init;        // DRIVER_INIT callback
	const tiny_rom_entry *      rom;                // pointer to list of ROMs for the game
	const char *                compatible_with;
	const internal_layout *     default_layout;     // default internally defined layout
	machine_flags::type         flags;              // orientation and other flags; see defines above
	char                        name[MAX_DRIVER_NAME_CHARS + 1]; // short name of the game
};


//**************************************************************************
//  MACROS
//**************************************************************************

// wrappers for declaring and defining game drivers
#define GAME_NAME(name)         driver_##name
#define GAME_TRAITS_NAME(name)  driver_##name##traits
#define GAME_EXTERN(name)       extern game_driver const GAME_NAME(name)

// static game traits
#define GAME_DRIVER_TRAITS(NAME, FULLNAME) \
namespace { \
	struct GAME_TRAITS_NAME(NAME) { static constexpr char const shortname[] = #NAME, fullname[] = FULLNAME, source[] = __FILE__; }; \
	constexpr char const GAME_TRAITS_NAME(NAME)::shortname[], GAME_TRAITS_NAME(NAME)::fullname[], GAME_TRAITS_NAME(NAME)::source[]; \
}
#define GAME_DRIVER_TYPE(NAME, CLASS, FLAGS) \
driver_device_creator< \
		CLASS, \
		(GAME_TRAITS_NAME(NAME)::shortname), \
		(GAME_TRAITS_NAME(NAME)::fullname), \
		(GAME_TRAITS_NAME(NAME)::source), \
		game_driver::unemulated_features(FLAGS), \
		game_driver::imperfect_features(FLAGS)>

// standard GAME() macro
#define GAME(YEAR,NAME,PARENT,MACHINE,INPUT,CLASS,INIT,MONITOR,COMPANY,FULLNAME,FLAGS) \
GAME_DRIVER_TRAITS(NAME,FULLNAME)                                       \
extern game_driver const GAME_NAME(NAME)                                \
{                                                                       \
	GAME_DRIVER_TYPE(NAME, CLASS, FLAGS),                               \
	#PARENT,                                                            \
	#YEAR,                                                              \
	COMPANY,                                                            \
	[] (machine_config &config, device_t &owner) { downcast<CLASS &>(owner).MACHINE(config); }, \
	INPUT_PORTS_NAME(INPUT),                                            \
	[] (device_t &owner) { downcast<CLASS &>(owner).INIT(); },          \
	ROM_NAME(NAME),                                                     \
	nullptr,                                                            \
	nullptr,                                                            \
	machine_flags::type(u32((MONITOR) | (FLAGS) | MACHINE_TYPE_ARCADE)),\
	#NAME                                                               \
};

// standard macro with additional layout
#define GAMEL(YEAR,NAME,PARENT,MACHINE,INPUT,CLASS,INIT,MONITOR,COMPANY,FULLNAME,FLAGS,LAYOUT) \
GAME_DRIVER_TRAITS(NAME,FULLNAME)                                       \
extern game_driver const GAME_NAME(NAME)                                \
{                                                                       \
	GAME_DRIVER_TYPE(NAME, CLASS, FLAGS),                               \
	#PARENT,                                                            \
	#YEAR,                                                              \
	COMPANY,                                                            \
	[] (machine_config &config, device_t &owner) { downcast<CLASS &>(owner).MACHINE(config); }, \
	INPUT_PORTS_NAME(INPUT),                                            \
	[] (device_t &owner) { downcast<CLASS &>(owner).INIT(); },          \
	ROM_NAME(NAME),                                                     \
	nullptr,                                                            \
	&LAYOUT,                                                            \
	machine_flags::type(u32((MONITOR) | (FLAGS) | MACHINE_TYPE_ARCADE)),\
	#NAME                                                               \
};


// standard console definition macro
#define CONS(YEAR,NAME,PARENT,COMPAT,MACHINE,INPUT,CLASS,INIT,COMPANY,FULLNAME,FLAGS) \
GAME_DRIVER_TRAITS(NAME,FULLNAME)                                       \
extern game_driver const GAME_NAME(NAME)                                \
{                                                                       \
	GAME_DRIVER_TYPE(NAME, CLASS, FLAGS),                               \
	#PARENT,                                                            \
	#YEAR,                                                              \
	COMPANY,                                                            \
	[] (machine_config &config, device_t &owner) { downcast<CLASS &>(owner).MACHINE(config); }, \
	INPUT_PORTS_NAME(INPUT),                                            \
	[] (device_t &owner) { downcast<CLASS &>(owner).INIT(); },          \
	ROM_NAME(NAME),                                                     \
	#COMPAT,                                                            \
	nullptr,                                                            \
	machine_flags::type(u32(ROT0 | (FLAGS) | MACHINE_TYPE_CONSOLE)),    \
	#NAME                                                               \
};

// standard computer definition macro
#define COMP(YEAR,NAME,PARENT,COMPAT,MACHINE,INPUT,CLASS,INIT,COMPANY,FULLNAME,FLAGS) \
GAME_DRIVER_TRAITS(NAME,FULLNAME)                                       \
extern game_driver const GAME_NAME(NAME)                                \
{                                                                       \
	GAME_DRIVER_TYPE(NAME, CLASS, FLAGS),                               \
	#PARENT,                                                            \
	#YEAR,                                                              \
	COMPANY,                                                            \
	[] (machine_config &config, device_t &owner) { downcast<CLASS &>(owner).MACHINE(config); }, \
	INPUT_PORTS_NAME(INPUT),                                            \
	[] (device_t &owner) { downcast<CLASS &>(owner).INIT(); },          \
	ROM_NAME(NAME),                                                     \
	#COMPAT,                                                            \
	nullptr,                                                            \
	machine_flags::type(u32(ROT0 | (FLAGS) | MACHINE_TYPE_COMPUTER)),   \
	#NAME                                                               \
};

// standard system definition macro
#define SYST(YEAR,NAME,PARENT,COMPAT,MACHINE,INPUT,CLASS,INIT,COMPANY,FULLNAME,FLAGS) \
GAME_DRIVER_TRAITS(NAME,FULLNAME)                                       \
extern game_driver const GAME_NAME(NAME)                                \
{                                                                       \
	GAME_DRIVER_TYPE(NAME, CLASS, FLAGS),                               \
	#PARENT,                                                            \
	#YEAR,                                                              \
	COMPANY,                                                            \
	[] (machine_config &config, device_t &owner) { downcast<CLASS &>(owner).MACHINE(config); }, \
	INPUT_PORTS_NAME(INPUT),                                            \
	[] (device_t &owner) { downcast<CLASS &>(owner).INIT(); },          \
	ROM_NAME(NAME),                                                     \
	#COMPAT,                                                            \
	nullptr,                                                            \
	machine_flags::type(u32(ROT0 | (FLAGS) | MACHINE_TYPE_OTHER)),      \
	#NAME                                                               \
};


#endif // MAME_EMU_GAMEDRV_H
