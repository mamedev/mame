// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    mameopts.cpp

    Options file and command line management.

***************************************************************************/

#include "emu.h"
#include "mameopts.h"

#include "drivenum.h"
#include "screen.h"
#include "softlist_dev.h"
#include "zippath.h"
#include "hashfile.h"

#include <ctype.h>
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
	const game_driver *cursystem = (driver == nullptr) ? system(options) : driver;
	if (cursystem == nullptr)
		return;

	// parse "vertical.ini" or "horizont.ini"
	if (cursystem->flags & ORIENTATION_SWAP_XY)
		parse_one_ini(options, "vertical", OPTION_PRIORITY_ORIENTATION_INI, &error_stream);
	else
		parse_one_ini(options, "horizont", OPTION_PRIORITY_ORIENTATION_INI, &error_stream);

	if (cursystem->flags & MACHINE_TYPE_ARCADE)
		parse_one_ini(options, "arcade", OPTION_PRIORITY_SYSTYPE_INI, &error_stream);
	else if (cursystem->flags & MACHINE_TYPE_CONSOLE)
		parse_one_ini(options ,"console", OPTION_PRIORITY_SYSTYPE_INI, &error_stream);
	else if (cursystem->flags & MACHINE_TYPE_COMPUTER)
		parse_one_ini(options, "computer", OPTION_PRIORITY_SYSTYPE_INI, &error_stream);
	else if (cursystem->flags & MACHINE_TYPE_OTHER)
		parse_one_ini(options, "othersys", OPTION_PRIORITY_SYSTYPE_INI, &error_stream);

	machine_config config(*cursystem, options);
	for (const screen_device &device : screen_device_iterator(config.root_device()))
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
	std::string sourcename = core_filename_extract_base(cursystem->source_file, true).insert(0, "source" PATH_SEPARATOR);
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
	int index = driver_list::find(core_filename_extract_base(options.system_name(), true).c_str());
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
	osd_file::error filerr = file.open(basename, ".ini");
	if (filerr != osd_file::error::NONE)
		return;

	// parse the file
	osd_printf_verbose("Parsing %s.ini\n", basename);
	try
	{
		options.parse_ini_file((util::core_file&)file, priority, false);
	}
	catch (options_exception &ex)
	{
		if (error_stream)
			util::stream_format(*error_stream, "While parsing %s:\n%s\n", ex.message(), file.fullpath(), ex.message());
		return;
	}

}
