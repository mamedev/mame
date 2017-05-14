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

// static POD structure describing each game driver entry
class game_driver
{
public:
	class driver_init_helper
	{
	public:
		void operator()(running_machine &machine) const { m_function(*this, machine); }
	protected:
		constexpr driver_init_helper(void (*function)(driver_init_helper const &, running_machine &)) : m_function(function) { }
		constexpr driver_init_helper(driver_init_helper const &) = default;
	private:
		void (* const m_function)(driver_init_helper const &, running_machine &);
	};

	template <class DriverClass> class driver_init_helper_impl : public driver_init_helper
	{
	public:
		constexpr driver_init_helper_impl(void (DriverClass::*method)()) : driver_init_helper(&driver_init_helper_impl<DriverClass>::invoke), m_method(method) { }
		constexpr driver_init_helper_impl(driver_init_helper_impl<DriverClass> const &) = default;
	private:
		static void invoke(driver_init_helper const &helper, running_machine &machine);
		void (DriverClass::*const m_method)();
	};

	template <class DriverClass> static constexpr auto make_driver_init(void (DriverClass::*method)()) { return driver_init_helper_impl<DriverClass>(method); }

	device_type                 type;               // static type info for driver class
	const char *                parent;             // if this is a clone, the name of the parent
	const char *                year;               // year the game was released
	const char *                manufacturer;       // manufacturer of the game
	machine_config_constructor  machine_config;     // machine driver tokens
	ioport_constructor          ipt;                // pointer to constructor for input ports
	driver_init_helper const &  driver_init;        // DRIVER_INIT callback
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
#define GAME_TRAITS_NAME(name)  driver_##name##traits
#define GAME_EXTERN(name)       extern game_driver const GAME_NAME(name)

// static game traits
#define GAME_DRIVER_TRAITS(NAME, FULLNAME) \
namespace { \
	struct GAME_TRAITS_NAME(NAME) { static constexpr char const shortname[] = #NAME, fullname[] = FULLNAME, source[] = __FILE__; }; \
	constexpr char const GAME_TRAITS_NAME(NAME)::shortname[], GAME_TRAITS_NAME(NAME)::fullname[], GAME_TRAITS_NAME(NAME)::source[]; \
}
#define GAME_DRIVER_TYPE(NAME, CLASS) driver_device_creator<CLASS, (GAME_TRAITS_NAME(NAME)::shortname), (GAME_TRAITS_NAME(NAME)::fullname), (GAME_TRAITS_NAME(NAME)::source)>

// standard GAME() macro
#define GAME(YEAR,NAME,PARENT,MACHINE,INPUT,CLASS,INIT,MONITOR,COMPANY,FULLNAME,FLAGS) \
GAME_DRIVER_TRAITS(NAME,FULLNAME)                                       \
extern game_driver const GAME_NAME(NAME)                                \
{                                                                       \
	GAME_DRIVER_TYPE(NAME, CLASS),                                      \
	#PARENT,                                                            \
	#YEAR,                                                              \
	COMPANY,                                                            \
	MACHINE_CONFIG_NAME(MACHINE),                                       \
	INPUT_PORTS_NAME(INPUT),                                            \
	game_driver::make_driver_init(&CLASS::init_##INIT),                 \
	ROM_NAME(NAME),                                                     \
	nullptr,                                                            \
	nullptr,                                                            \
	(MONITOR) | (FLAGS) | MACHINE_TYPE_ARCADE,                          \
	#NAME                                                               \
};

// standard macro with additional layout
#define GAMEL(YEAR,NAME,PARENT,MACHINE,INPUT,CLASS,INIT,MONITOR,COMPANY,FULLNAME,FLAGS,LAYOUT) \
GAME_DRIVER_TRAITS(NAME,FULLNAME)                                       \
extern game_driver const GAME_NAME(NAME)                                \
{                                                                       \
	GAME_DRIVER_TYPE(NAME, CLASS),                                      \
	#PARENT,                                                            \
	#YEAR,                                                              \
	COMPANY,                                                            \
	MACHINE_CONFIG_NAME(MACHINE),                                       \
	INPUT_PORTS_NAME(INPUT),                                            \
	game_driver::make_driver_init(&CLASS::init_##INIT),                 \
	ROM_NAME(NAME),                                                     \
	nullptr,                                                            \
	&LAYOUT,                                                            \
	(MONITOR) | (FLAGS) | MACHINE_TYPE_ARCADE,                          \
	#NAME                                                               \
};


// standard console definition macro
#define CONS(YEAR,NAME,PARENT,COMPAT,MACHINE,INPUT,CLASS,INIT,COMPANY,FULLNAME,FLAGS) \
GAME_DRIVER_TRAITS(NAME,FULLNAME)                                       \
extern game_driver const GAME_NAME(NAME)                                \
{                                                                       \
	GAME_DRIVER_TYPE(NAME, CLASS),                                      \
	#PARENT,                                                            \
	#YEAR,                                                              \
	COMPANY,                                                            \
	MACHINE_CONFIG_NAME(MACHINE),                                       \
	INPUT_PORTS_NAME(INPUT),                                            \
	game_driver::make_driver_init(&CLASS::init_##INIT),                 \
	ROM_NAME(NAME),                                                     \
	#COMPAT,                                                            \
	nullptr,                                                            \
	ROT0 | (FLAGS) | MACHINE_TYPE_CONSOLE,                              \
	#NAME                                                               \
};

// standard computer definition macro
#define COMP(YEAR,NAME,PARENT,COMPAT,MACHINE,INPUT,CLASS,INIT,COMPANY,FULLNAME,FLAGS) \
GAME_DRIVER_TRAITS(NAME,FULLNAME)                                       \
extern game_driver const GAME_NAME(NAME)                                \
{                                                                       \
	GAME_DRIVER_TYPE(NAME, CLASS),                                      \
	#PARENT,                                                            \
	#YEAR,                                                              \
	COMPANY,                                                            \
	MACHINE_CONFIG_NAME(MACHINE),                                       \
	INPUT_PORTS_NAME(INPUT),                                            \
	game_driver::make_driver_init(&CLASS::init_##INIT),                 \
	ROM_NAME(NAME),                                                     \
	#COMPAT,                                                            \
	nullptr,                                                            \
	ROT0 | (FLAGS) | MACHINE_TYPE_COMPUTER,                             \
	#NAME                                                               \
};

// standard system definition macro
#define SYST(YEAR,NAME,PARENT,COMPAT,MACHINE,INPUT,CLASS,INIT,COMPANY,FULLNAME,FLAGS) \
GAME_DRIVER_TRAITS(NAME,FULLNAME)                                       \
extern game_driver const GAME_NAME(NAME)                                \
{                                                                       \
	GAME_DRIVER_TYPE(NAME, CLASS),                                      \
	#PARENT,                                                            \
	#YEAR,                                                              \
	COMPANY,                                                            \
	MACHINE_CONFIG_NAME(MACHINE),                                       \
	INPUT_PORTS_NAME(INPUT),                                            \
	game_driver::make_driver_init(&CLASS::init_##INIT),                 \
	ROM_NAME(NAME),                                                     \
	#COMPAT,                                                            \
	nullptr,                                                            \
	ROT0 | (FLAGS) | MACHINE_TYPE_OTHER,                                \
	#NAME                                                               \
};


#endif // MAME_EMU_GAMEDRV_H
