// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    mameopts.cpp

    Options file and command line management.

***************************************************************************/

#include "emu.h"
#include "mameopts.h"

#include "drivenum.h"
#include "fileio.h"
#include "screen.h"
#include "softlist_dev.h"
#include "zippath.h"
#include "hashfile.h"
#include "clifront.h"

#include <cctype>
#include <stack>


//-------------------------------------------------
//  parse_standard_inis - parse the standard set
//  of INI files
//-------------------------------------------------

void mame_options::parse_standard_inis(emu_options &options, std::ostream &error_stream, const game_driver *driver)
{
	// parse the INI file defined by the platform (e.g., "mame.ini")
	// we do this twice so that the first file can change the INI path
	parse_one_ini(options, emulator_info::get_configname(), OPTION_PRIORITY_MAME_INI);
	parse_one_ini(options, emulator_info::get_configname(), OPTION_PRIORITY_MAME_INI, &error_stream);

	// debug mode: parse "debug.ini" as well
	if (options.debug())
		parse_one_ini(options, "debug", OPTION_PRIORITY_DEBUG_INI, &error_stream);

	// if we have a valid system driver, parse system-specific INI files
	game_driver const *const cursystem = !driver ? system(options) : driver;
	if (!cursystem)
		return;

	// parse "vertical.ini" or "horizont.ini"
	if (cursystem->flags & ORIENTATION_SWAP_XY)
		parse_one_ini(options, "vertical", OPTION_PRIORITY_ORIENTATION_INI, &error_stream);
	else
		parse_one_ini(options, "horizont", OPTION_PRIORITY_ORIENTATION_INI, &error_stream);

	switch (cursystem->flags & machine_flags::MASK_TYPE)
	{
	case machine_flags::TYPE_ARCADE:
		parse_one_ini(options, "arcade", OPTION_PRIORITY_SYSTYPE_INI, &error_stream);
		break;
	case machine_flags::TYPE_CONSOLE:
		parse_one_ini(options ,"console", OPTION_PRIORITY_SYSTYPE_INI, &error_stream);
		break;
	case machine_flags::TYPE_COMPUTER:
		parse_one_ini(options, "computer", OPTION_PRIORITY_SYSTYPE_INI, &error_stream);
		break;
	case machine_flags::TYPE_OTHER:
		parse_one_ini(options, "othersys", OPTION_PRIORITY_SYSTYPE_INI, &error_stream);
		break;
	default:
		break;
	}

	machine_config config(*cursystem, options);
	for (const screen_device &device : screen_device_enumerator(config.root_device()))
	{
		// parse "raster.ini" for raster games
		if (device.screen_type() == SCREEN_TYPE_RASTER)
		{
			parse_one_ini(options, "raster", OPTION_PRIORITY_SCREEN_INI, &error_stream);
			break;
		}
		// parse "vector.ini" for vector games
		if (device.screen_type() == SCREEN_TYPE_VECTOR)
		{
			parse_one_ini(options, "vector", OPTION_PRIORITY_SCREEN_INI, &error_stream);
			break;
		}
		// parse "lcd.ini" for lcd games
		if (device.screen_type() == SCREEN_TYPE_LCD)
		{
			parse_one_ini(options, "lcd", OPTION_PRIORITY_SCREEN_INI, &error_stream);
			break;
		}
	}

	// next parse "source/<sourcefile>.ini"
	std::string sourcename = std::string(core_filename_extract_base(cursystem->type.source(), true)).insert(0, "source" PATH_SEPARATOR);
	parse_one_ini(options, sourcename.c_str(), OPTION_PRIORITY_SOURCE_INI, &error_stream);

	// then parse the grandparent, parent, and system-specific INIs
	int parent = driver_list::clone(*cursystem);
	int gparent = (parent != -1) ? driver_list::clone(parent) : -1;
	if (gparent != -1)
		parse_one_ini(options, driver_list::driver(gparent).name, OPTION_PRIORITY_GPARENT_INI, &error_stream);
	if (parent != -1)
		parse_one_ini(options, driver_list::driver(parent).name, OPTION_PRIORITY_PARENT_INI, &error_stream);
	parse_one_ini(options, cursystem->name, OPTION_PRIORITY_DRIVER_INI, &error_stream);
}


//-------------------------------------------------
//  system - return a pointer to the specified
//  system driver, or nullptr if no match
//-------------------------------------------------

const game_driver *mame_options::system(const emu_options &options)
{
	int index = driver_list::find(std::string(core_filename_extract_base(options.system_name(), true)).c_str());
	return (index != -1) ? &driver_list::driver(index) : nullptr;
}


//-------------------------------------------------
//  parse_one_ini - parse a single INI file
//-------------------------------------------------

void mame_options::parse_one_ini(emu_options &options, const char *basename, int priority, std::ostream *error_stream)
{
	// don't parse if it has been disabled
	if (!options.read_config())
		return;

	// open the file; if we fail, that's ok
	emu_file file(options.ini_path(), OPEN_FLAG_READ);
	osd_printf_verbose("Attempting load of %s.ini\n", basename);
	std::error_condition const filerr = file.open(std::string(basename) + ".ini");
	if (filerr)
		return;

	// parse the file
	osd_printf_verbose("Parsing %s.ini\n", basename);
	try
	{
		options.parse_ini_file((util::core_file&)file, priority, priority < OPTION_PRIORITY_DRIVER_INI, false);
	}
	catch (options_exception &ex)
	{
		if (error_stream)
			util::stream_format(*error_stream, "While parsing %s:\n%s\n", file.fullpath(), ex.message());
		return;
	}

}


//-------------------------------------------------
//  populate_hashpath_from_args_and_inis
//-------------------------------------------------

void mame_options::populate_hashpath_from_args_and_inis(emu_options &options, const std::vector<std::string> &args)
{
	// The existence of this function comes from the fact that for softlist options to be properly
	// evaluated, we need to have the hashpath variable set.  The problem is that the hashpath may
	// be set anywhere on the command line, but also in any of the myriad INI files that we parse, some
	// of which may be system specific (e.g. - nes.ini) or otherwise influenced by the system (e.g. - vector.ini)
	//
	// I think that it is terrible that we have to do a completely independent pass on the command line and every
	// argument simply because any one of these things might be setting - hashpath.Unless we invest the effort in
	// building some sort of "late binding" apparatus for options(e.g. - delay evaluation of softlist options until
	// we've scoured all INIs for hashpath) that can completely straddle the command line and the INI worlds, doing
	// this is the best that we can do IMO.

	// parse the command line
	emu_options temp_options(emu_options::option_support::GENERAL_AND_SYSTEM);

	// pick up whatever changes the osd did to the default inipath
	temp_options.set_default_value(OPTION_INIPATH, options.ini_path());

	try
	{
		temp_options.parse_command_line(args, OPTION_PRIORITY_CMDLINE, true);
	}
	catch (options_exception &)
	{
		// Something is very long; we have bigger problems than -hashpath possibly
		// being in never-never land.  Punt and let the main code fail
		return;
	}

	// if we have an auxillary verb, hashpath is irrelevant
	if (!temp_options.command().empty())
		return;

	// read INI files
	if (temp_options.read_config())
	{
		std::ostringstream error_stream;
		parse_standard_inis(temp_options, error_stream);
	}

	// and fish out hashpath
	const auto entry = temp_options.get_entry(OPTION_HASHPATH);
	if (entry)
	{
		try
		{
			options.set_value(OPTION_HASHPATH, entry->value(), entry->priority());
		}
		catch (options_exception &)
		{
		}
	}
}
