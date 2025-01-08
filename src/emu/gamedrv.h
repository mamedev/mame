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


/// \defgroup machinedef Machine definition macros


//**************************************************************************
//  CONSTANTS
//**************************************************************************

// maxima
constexpr int MAX_DRIVER_NAME_CHARS = 16;

struct machine_flags
{
	enum type : u32
	{
		MASK_ORIENTATION    = 0x0000'0007,

		FLIP_X              = 0x0000'0001,
		FLIP_Y              = 0x0000'0002,
		SWAP_XY             = 0x0000'0004,
		ROT0                = 0x0000'0000,
		ROT90               = FLIP_X | SWAP_XY,
		ROT180              = FLIP_X | FLIP_Y,
		ROT270              = FLIP_Y | SWAP_XY,

		NOT_WORKING         = 0x0000'0040,
		SUPPORTS_SAVE       = 0x0000'0080,  // system supports save states
		NO_COCKTAIL         = 0x0000'0100,  // screen flip support is missing
		IS_BIOS_ROOT        = 0x0000'0200,  // this driver entry is a BIOS root
		REQUIRES_ARTWORK    = 0x0000'0400,  // requires external artwork for key game elements
		UNOFFICIAL          = 0x0000'0800,  // unofficial hardware modification
		NO_SOUND_HW         = 0x0000'1000,  // system has no sound output
		MECHANICAL          = 0x0000'2000,  // contains mechanical parts (pinball, redemption games, ...)
		IS_INCOMPLETE       = 0x0000'4000   // official system with blatantly incomplete hardware/software
	};
};

DECLARE_ENUM_BITWISE_OPERATORS(machine_flags::type);


/// \addtogroup machinedef
/// \{
/// \name System emulation status constants
///
/// Constants representing system emulation status flags.  May be
/// combined for use as FLAGS arguments to the #GAME, #GAMEL, #CONS,
/// #COMP and #SYST macros.
/// \{

// flags for machine drivers
constexpr u64 MACHINE_NOT_WORKING               = machine_flags::NOT_WORKING;               ///< Imperfect emulation prevents using the system as intended
constexpr u64 MACHINE_SUPPORTS_SAVE             = machine_flags::SUPPORTS_SAVE;             ///< All devices in the system supports save states (enables auto save feature, and won't show a warning on using save states)
constexpr u64 MACHINE_NO_COCKTAIL               = machine_flags::NO_COCKTAIL;               ///< The system supports screen flipping for use in a cocktail cabinet, but this feature is not properly emulated
constexpr u64 MACHINE_IS_BIOS_ROOT              = machine_flags::IS_BIOS_ROOT;              ///< The system represents an empty system board of some kind - clones are treated as separate systems rather than variants
constexpr u64 MACHINE_REQUIRES_ARTWORK          = machine_flags::REQUIRES_ARTWORK;          ///< The system requires external artwork for key functionality
constexpr u64 MACHINE_UNOFFICIAL                = machine_flags::UNOFFICIAL;                ///< The system represents an after-market or end-user modification to a system
constexpr u64 MACHINE_NO_SOUND_HW               = machine_flags::NO_SOUND_HW;               ///< The system has no sound output capability
constexpr u64 MACHINE_MECHANICAL                = machine_flags::MECHANICAL;                ///< The system depends on mechanical features for key functionality
constexpr u64 MACHINE_IS_INCOMPLETE             = machine_flags::IS_INCOMPLETE;             ///< The system represents an incomplete prototype

// flags that map to device feature flags
constexpr u64 MACHINE_UNEMULATED_PROTECTION     = 0x00000001'00000000;                      ///< Some form of protection is imperfectly emulated (e.g. copy protection or anti-tampering)
constexpr u64 MACHINE_WRONG_COLORS              = 0x00000002'00000000;                      ///< Colours are completely wrong
constexpr u64 MACHINE_IMPERFECT_COLORS          = 0x00000004'00000000;                      ///< Colours are close but not completely accurate
constexpr u64 MACHINE_IMPERFECT_GRAPHICS        = 0x00000008'00000000;                      ///< Graphics are emulated incorrectly for the system
constexpr u64 MACHINE_NO_SOUND                  = 0x00000010'00000000;                      ///< The system has sound output, but it is not emulated
constexpr u64 MACHINE_IMPERFECT_SOUND           = 0x00000020'00000000;                      ///< Sound is known to be imperfectly emulated for the system
constexpr u64 MACHINE_IMPERFECT_CONTROLS        = 0x00000040'00000000;                      ///< Controls or inputs are emulated imperfectly for the system
constexpr u64 MACHINE_NODEVICE_MICROPHONE       = 0x00000080'00000000;                      ///< The system has unemulated audio capture functionality
constexpr u64 MACHINE_NODEVICE_PRINTER          = 0x00000100'00000000;                      ///< The system has unemulated printer functionality
constexpr u64 MACHINE_NODEVICE_LAN              = 0x00000200'00000000;                      ///< The system has unemulated local area networking
constexpr u64 MACHINE_IMPERFECT_TIMING          = 0x00000400'00000000;                      ///< Timing is known to be imperfectly emulated for the system

/// \}
/// \}


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

/// \brief Static system description
///
/// A plain data structure providing static information about a system.
/// Used to allow multiple systems to be implemented using a single
/// system device class (an implementation of #driver_device).
class game_driver
{
public:
	typedef void (*machine_creator_wrapper)(machine_config &, device_t &);
	typedef void (*driver_init_wrapper)(device_t &);

	/// \brief Get unemulated system features
	///
	/// Converts system flags corresponding to unemulated device
	/// features to a device feature type bit field.
	/// \param [in] flags A system flags bit field.
	/// \return A device feature type bit field corresponding to
	///   unemulated features declared in the \p flags argument.
	static constexpr device_t::feature_type unemulated_features(u64 flags)
	{
		return
				((flags & MACHINE_WRONG_COLORS)             ? device_t::feature::PALETTE    : device_t::feature::NONE) |
				((flags & MACHINE_NO_SOUND)                 ? device_t::feature::SOUND      : device_t::feature::NONE) |
				((flags & MACHINE_NODEVICE_MICROPHONE)      ? device_t::feature::MICROPHONE : device_t::feature::NONE) |
				((flags & MACHINE_NODEVICE_PRINTER)         ? device_t::feature::PRINTER    : device_t::feature::NONE) |
				((flags & MACHINE_NODEVICE_LAN)             ? device_t::feature::LAN        : device_t::feature::NONE);
	}

	/// \brief Get imperfectly emulated system features
	///
	/// Converts system flags corresponding to imperfectly emulated
	/// device features to a device feature type bit field.
	/// \param [in] flags A system flags bit field.
	/// \return A device feature type bit field corresponding to
	///   imperfectly emulated features declared in the \p flags
	///   argument.
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
	const char *                parent;             // name of the parent or BIOS system if applicable
	const char *                year;               // year the game was released
	const char *                manufacturer;       // manufacturer of the game
	machine_creator_wrapper     machine_creator;    // machine driver tokens
	ioport_constructor          ipt;                // pointer to constructor for input ports
	driver_init_wrapper         driver_init;        // DRIVER_INIT callback
	const tiny_rom_entry *      rom;                // pointer to list of ROMs for the game
	const char *                compatible_with;
	const internal_layout *     default_layout;     // default internally defined layout
	machine_flags::type         flags;              // orientation and other flags; see defines above
	char                        name[MAX_DRIVER_NAME_CHARS + 1]; // short name of the system
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


/// \addtogroup machinedef
/// \{

/// \brief Define a system
///
/// Must be used in the global namespace.
///
/// Creates an appropriately named and populated #game_driver structure
/// describing the system.
/// \param YEAR The year that the system was first made available.  Must
///   be a token containing only the digits zero to nine, question mark
///   and plus sign.
/// \param NAME The short name of the system, used for identification,
///   and in filesystem paths for assets and data.  Must be a token no
///   longer than sixteen characters, containing only ASCII lowercase
///   letters, digits and underscores.  Must be globally unique across
///   systems and devices.
/// \param PARENT Short name of the parent or BIOS system if applicable,
///   or a single digit zero otherwise.
/// \param MACHINE Function used to buid machine configuration for the
///   system.  Must be a public member function of the system device
///   class (\p CLASS argument), returning void and taking a reference
///   to a #machine_config object as a parameter.
/// \param INPUT Input port definitions for the root device of the
///   system, usually defined using #INPUT_PORTS_START and associated
///   macros.
/// \param CLASS Class to instantiate as the root device of the system.
///   Must be an implementation of #driver_device.
/// \param INIT Initialisation function called after all child devices
///   have started, but before the driver start functions are called.
///   Often used for tasks like decrypting ROMs.  Must be a public
///   member function of the system device class (\p CLASS argument),
///   returning void and accepting no parameters.  The function
///   #driver_device::empty_init is supplied for systems that don't need
///   to perform additional tasks.
/// \param MONITOR Screen orientation flags applied to all screens in
///   the system, after the individual screens' orientation flags are
///   applied.  Usually one #ROT0, #ROT90, #ROT180 or #ROT270.
/// \param COMPANY Name of the developer or distributor of the system.
///   Must be a string.
/// \param FULLNAME Display name for the system.  Must be a string, and
///   must be globally unique across systems and devices.
/// \param FLAGS Bitwise combination of emulation status flags for the
///   system, in addition to flags supplied by the system device class
///   (see #device_t::unemulated_features and
///   #device_t::imperfect_features).  It is advisable to supply
///   unemulated and imperfectly emulated feature flags that apply to
///   all systems implemented using the class in the class itself to
///   avoid repetition.
/// \sa GAMEL SYST
#define GAME(YEAR, NAME, PARENT, MACHINE, INPUT, CLASS, INIT, MONITOR, COMPANY, FULLNAME, FLAGS) \
GAME_DRIVER_TRAITS(NAME, FULLNAME)                                      \
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
	machine_flags::type(u32((MONITOR) | (FLAGS))),                      \
	#NAME                                                               \
};


/// \brief Define a system with an additional internal layout
///
/// Equivalent to the #GAME macro, but with the additional ability to
/// supply a system-specific internal artwork layout.  Views from the
/// system-specific layout are available in addition to any views from
/// layouts specified in the machine configuration.  Must be used in the
/// global namespace.
///
/// Creates an appropriately named and populated #game_driver structure
/// describing the system.
/// \param YEAR The year that the system was first made available.  Must
///   be a token containing only the digits zero to nine, question mark
///   and plus sign.
/// \param NAME The short name of the system, used for identification,
///   and in filesystem paths for assets and data.  Must be a token no
///   longer than sixteen characters, containing only ASCII lowercase
///   letters, digits and underscores.  Must be globally unique across
///   systems and devices.
/// \param PARENT Short name of the parent or BIOS system if applicable,
///   or a single digit zero otherwise.
/// \param MACHINE Function used to buid machine configuration for the
///   system.  Must be a public member function of the system device
///   class (\p CLASS argument), returning void and taking a reference
///   to a #machine_config object as a parameter.
/// \param INPUT Input port definitions for the root device of the
///   system, usually defined using #INPUT_PORTS_START and associated
///   macros.
/// \param CLASS Class to instantiate as the root device of the system.
///   Must be an implementation of #driver_device.
/// \param INIT Initialisation function called after all child devices
///   have started, but before the driver start functions are called.
///   Often used for tasks like decrypting ROMs.  Must be a public
///   member function of the system device class (\p CLASS argument),
///   returning void and accepting no parameters.  The function
///   #driver_device::empty_init is supplied for systems that don't need
///   to perform additional tasks.
/// \param MONITOR Screen orientation flags applied to all screens in
///   the system, after the individual screens' orientation flags are
///   applied.  Usually one #ROT0, #ROT90, #ROT180 or #ROT270.
/// \param COMPANY Name of the developer or distributor of the system.
///   Must be a string.
/// \param FULLNAME Display name for the system.  Must be a string, and
///   must be globally unique across systems and devices.
/// \param FLAGS Bitwise combination of emulation status flags for the
///   system, in addition to flags supplied by the system device class
///   (see #device_t::unemulated_features and
///   #device_t::imperfect_features).  It is advisable to supply
///   unemulated and imperfectly emulated feature flags that apply to
///   all systems implemented using the class in the class itself to
///   avoid repetition.
/// \param LAYOUT An #internal_layout structure providing additional
///   internal artwork for the system.
/// \sa GAME SYST
#define GAMEL(YEAR, NAME, PARENT, MACHINE, INPUT, CLASS, INIT, MONITOR, COMPANY, FULLNAME, FLAGS, LAYOUT) \
GAME_DRIVER_TRAITS(NAME, FULLNAME)                                      \
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
	machine_flags::type(u32((MONITOR) | (FLAGS))),                      \
	#NAME                                                               \
};


/// \brief Define a system with software compatibility grouping
///
/// Equivalent to the #GAME macro, but allows software-compatible
/// systems to be grouped.
///
/// Creates an appropriately named and populated #game_driver structure
/// describing the system.
/// \param YEAR The year that the system was first made available.  Must
///   be a token containing only the digits zero to nine, question mark
///   and plus sign.
/// \param NAME The short name of the system, used for identification,
///   and in filesystem paths for assets and data.  Must be a token no
///   longer than sixteen characters, containing only ASCII lowercase
///   letters, digits and underscores.  Must be globally unique across
///   systems and devices.
/// \param PARENT Short name of the parent or BIOS system if applicable,
///   or a single digit zero otherwise.
/// \param COMPAT Short name of a system that this system is compatible
///   with if applicable, or a single digit zero otherwise.
/// \param MACHINE Function used to buid machine configuration for the
///   system.  Must be a public member function of the system device
///   class (\p CLASS argument), returning void and taking a reference
///   to a #machine_config object as a parameter.
/// \param INPUT Input port definitions for the root device of the
///   system, usually defined using #INPUT_PORTS_START and associated
///   macros.
/// \param CLASS Class to instantiate as the root device of the system.
///   Must be an implementation of #driver_device.
/// \param INIT Initialisation function called after all child devices
///   have started, but before the driver start functions are called.
///   Often used for tasks like decrypting ROMs.  Must be a public
///   member function of the system device class (\p CLASS argument),
///   returning void and accepting no parameters.  The function
///   #driver_device::empty_init is supplied for systems that don't need
///   to perform additional tasks.
/// \param COMPANY Name of the developer or distributor of the system.
///   Must be a string.
/// \param FULLNAME Display name for the system.  Must be a string, and
///   must be globally unique across systems and devices.
/// \param FLAGS Bitwise combination of emulation status flags for the
///   system, in addition to flags supplied by the system device class
///   (see #device_t::unemulated_features and
///   #device_t::imperfect_features).  It is advisable to supply
///   unemulated and imperfectly emulated feature flags that apply to
///   all systems implemented using the class in the class itself to
///   avoid repetition.  Screen orientation flags may be included here.
/// \sa GAME GAMEL
#define SYST(YEAR, NAME, PARENT, COMPAT, MACHINE, INPUT, CLASS, INIT, COMPANY, FULLNAME, FLAGS) \
		GAME_DRIVER_TRAITS(NAME, FULLNAME)                                      \
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
			machine_flags::type(u32(ROT0 | (FLAGS))),                           \
			#NAME                                                               \
		};


#define CONS(YEAR, NAME, PARENT, COMPAT, MACHINE, INPUT, CLASS, INIT, COMPANY, FULLNAME, FLAGS) \
		SYST(YEAR, NAME, PARENT, COMPAT, MACHINE, INPUT, CLASS, INIT, COMPANY, FULLNAME, FLAGS)

#define COMP(YEAR, NAME, PARENT, COMPAT, MACHINE, INPUT, CLASS, INIT, COMPANY, FULLNAME, FLAGS) \
		SYST(YEAR, NAME, PARENT, COMPAT, MACHINE, INPUT, CLASS, INIT, COMPANY, FULLNAME, FLAGS)

/// \}

#endif // MAME_EMU_GAMEDRV_H
