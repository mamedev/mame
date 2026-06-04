// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    mameopts.h

    Options file and command line management.

***************************************************************************/

#ifndef MAME_FRONTEND_MAMEOPTS_H
#define MAME_FRONTEND_MAMEOPTS_H

#pragma once

#include "emuopts.h"

//**************************************************************************
//  CONSTANTS
//**************************************************************************
#undef OPTION_PRIORITY_CMDLINE

// option priorities
enum
{
	// command-line options override everything (HIGH priority)
	OPTION_PRIORITY_SUBCMD = OPTION_PRIORITY_HIGH,
	OPTION_PRIORITY_CMDLINE,

	// user INI files, in increasing order of specificity (NORMAL priority)
	OPTION_PRIORITY_MAME_INI = OPTION_PRIORITY_NORMAL + 1,
	OPTION_PRIORITY_DEBUG_INI,
	OPTION_PRIORITY_ORIENTATION_INI,
	OPTION_PRIORITY_SCREEN_INI,
	OPTION_PRIORITY_SOURCE_INI,
	OPTION_PRIORITY_GPARENT_INI,
	OPTION_PRIORITY_PARENT_INI,
	OPTION_PRIORITY_DRIVER_INI,
	OPTION_PRIORITY_INI,

	// hardcoded bootstrap INI, overridden by everything else (DEFAULT priority)
	OPTION_PRIORITY_BOOTSTRAP = OPTION_PRIORITY_DEFAULT + 1,
};

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// forward references
class game_driver;
class software_part;

class mame_options
{
public:
	// parsing wrappers
	static void parse_standard_inis(emu_options &options, std::ostream &error_stream, const game_driver *driver = nullptr);
	static const game_driver *system(const emu_options &options);
	static void populate_hashpath_from_args_and_inis(emu_options &options, const std::vector<std::string> &args);

private:
	// INI parsing helper
	static void parse_one_ini(emu_options &options, const char *basename, int priority, std::ostream *error_stream = nullptr);
};

#endif  // MAME_FRONTEND_MAMEOPTS_H
