// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    cliopts.c

***************************************************************************/

#include "clifront.h"
#include "cliopts.h"

//**************************************************************************
//  COMMAND-LINE OPTIONS
//**************************************************************************

const options_entry cli_options::s_option_entries[] =
{
	/* core commands */
	{ nullptr,                            nullptr,       OPTION_HEADER,     "CORE COMMANDS" },
	{ CLICOMMAND_HELP ";h;?",           "0",       OPTION_COMMAND,    "show help message" },
	{ CLICOMMAND_VALIDATE ";valid",     "0",       OPTION_COMMAND,    "perform driver validation on all game drivers" },

	/* configuration commands */
	{ nullptr,                            nullptr,       OPTION_HEADER,     "CONFIGURATION COMMANDS" },
	{ CLICOMMAND_CREATECONFIG ";cc",    "0",       OPTION_COMMAND,    "create the default configuration file" },
	{ CLICOMMAND_SHOWCONFIG ";sc",      "0",       OPTION_COMMAND,    "display running parameters" },
	{ CLICOMMAND_SHOWUSAGE ";su",       "0",       OPTION_COMMAND,    "show this help" },

	/* frontend commands */
	{ nullptr,                            nullptr,       OPTION_HEADER,     "FRONTEND COMMANDS" },
	{ CLICOMMAND_LISTXML ";lx",         "0",       OPTION_COMMAND,    "all available info on driver in XML format" },
	{ CLICOMMAND_LISTFULL ";ll",        "0",       OPTION_COMMAND,    "short name, full name" },
	{ CLICOMMAND_LISTSOURCE ";ls",      "0",       OPTION_COMMAND,    "driver sourcefile" },
	{ CLICOMMAND_LISTCLONES ";lc",      "0",       OPTION_COMMAND,    "show clones" },
	{ CLICOMMAND_LISTBROTHERS ";lb",    "0",       OPTION_COMMAND,    "show \"brothers\", or other drivers from same sourcefile" },
	{ CLICOMMAND_LISTCRC,               "0",       OPTION_COMMAND,    "CRC-32s" },
	{ CLICOMMAND_LISTROMS ";lr",        "0",       OPTION_COMMAND,    "list required roms for a driver" },
	{ CLICOMMAND_LISTSAMPLES,           "0",       OPTION_COMMAND,    "list optional samples for a driver" },
	{ CLICOMMAND_VERIFYROMS,            "0",       OPTION_COMMAND,    "report romsets that have problems" },
	{ CLICOMMAND_VERIFYSAMPLES,         "0",       OPTION_COMMAND,    "report samplesets that have problems" },
	{ CLICOMMAND_ROMIDENT,              "0",       OPTION_COMMAND,    "compare files with known MAME roms" },
	{ CLICOMMAND_LISTDEVICES ";ld",     "0",       OPTION_COMMAND,    "list available devices" },
	{ CLICOMMAND_LISTSLOTS ";lslot",    "0",       OPTION_COMMAND,    "list available slots and slot devices" },
	{ CLICOMMAND_LISTMEDIA ";lm",       "0",       OPTION_COMMAND,    "list available media for the system" },
	{ CLICOMMAND_LISTSOFTWARE ";lsoft", "0",       OPTION_COMMAND,    "list known software for the system" },
	{ CLICOMMAND_VERIFYSOFTWARE ";vsoft", "0",     OPTION_COMMAND,    "verify known software for the system" },
	{ CLICOMMAND_GETSOFTLIST ";glist",  "0",       OPTION_COMMAND,    "retrieve software list by name" },
	{ CLICOMMAND_VERIFYSOFTLIST ";vlist", "0",     OPTION_COMMAND,    "verify software list by name" },
	{ nullptr }
};

//**************************************************************************
//  CLI OPTIONS
//**************************************************************************

//-------------------------------------------------
//  cli_options - constructor
//-------------------------------------------------

cli_options::cli_options()
: emu_options()
{
	add_entries(cli_options::s_option_entries);
}
