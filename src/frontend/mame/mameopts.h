// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    mameopts.h

    Options file and command line management.

***************************************************************************/

#pragma once

#ifndef __MAMEOPTS_H__
#define __MAMEOPTS_H__

#include "emuopts.h"

//**************************************************************************
//  CONSTANTS
//**************************************************************************
#undef OPTION_PRIORITY_CMDLINE

// option priorities
enum
{
	// command-line options are HIGH priority
	OPTION_PRIORITY_SUBCMD = OPTION_PRIORITY_HIGH,
	OPTION_PRIORITY_CMDLINE,

	// INI-based options are NORMAL priority, in increasing order:
	OPTION_PRIORITY_MAME_INI = OPTION_PRIORITY_NORMAL + 1,
	OPTION_PRIORITY_DEBUG_INI,
	OPTION_PRIORITY_ORIENTATION_INI,
	OPTION_PRIORITY_SYSTYPE_INI,
	OPTION_PRIORITY_SCREEN_INI,
	OPTION_PRIORITY_SOURCE_INI,
	OPTION_PRIORITY_GPARENT_INI,
	OPTION_PRIORITY_PARENT_INI,
	OPTION_PRIORITY_DRIVER_INI,
	OPTION_PRIORITY_INI,
};

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// forward references
struct game_driver;
class software_part;

class mame_options
{
	static const uint32_t OPTION_FLAG_DEVICE = 0x80000000;

public:
	// parsing wrappers
	static bool parse_command_line(emu_options &options, std::vector<std::string> &args, std::string &error_string);
	static void parse_standard_inis(emu_options &options, std::string &error_string, const game_driver *driver = nullptr);
	// FIXME: Couriersud: This should be in image_device_exit
	static void remove_device_options(emu_options &options);

	static const game_driver *system(const emu_options &options);
	static void set_system_name(emu_options &options, const char *name);
	static bool add_slot_options(emu_options &options, std::function<void(emu_options &options, const std::string &)> value_specifier = nullptr);

private:
	// device-specific option handling
	static void add_device_options(emu_options &options, std::function<void(emu_options &options, const std::string &)> value_specifier = nullptr);
	static void update_slot_options(emu_options &options, const software_part *swpart = nullptr);
	static void parse_slot_devices(emu_options &options, std::function<void(emu_options &options, const std::string &)> value_specifier);
	static std::string get_full_option_name(const device_image_interface &image);
	static bool reevaluate_slot_options(emu_options &options);

	// INI parsing helper
	static bool parse_one_ini(emu_options &options, const char *basename, int priority, std::string *error_string = nullptr);

	// softlist handling
	static std::map<std::string, std::string> evaluate_initial_softlist_options(emu_options &options);

	static int m_slot_options;
	static int m_device_options;
};

#endif  /* __MAMEOPTS_H__ */
