// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    clifront.h

    Command-line interface frontend for MAME.

***************************************************************************/

#pragma once

#ifndef __CLIOPTS_H__
#define __CLIOPTS_H__

#include "emuopts.h"

//**************************************************************************
//  CONSTANTS
//**************************************************************************

// core commands
#define CLICOMMAND_HELP                 "help"
#define CLICOMMAND_VALIDATE             "validate"

// configuration commands
#define CLICOMMAND_CREATECONFIG         "createconfig"
#define CLICOMMAND_SHOWCONFIG           "showconfig"
#define CLICOMMAND_SHOWUSAGE            "showusage"

// frontend commands
#define CLICOMMAND_LISTXML              "listxml"
#define CLICOMMAND_LISTFULL             "listfull"
#define CLICOMMAND_LISTSOURCE           "listsource"
#define CLICOMMAND_LISTCLONES           "listclones"
#define CLICOMMAND_LISTBROTHERS         "listbrothers"
#define CLICOMMAND_LISTTREE             "listtree"
#define CLICOMMAND_LISTCRC              "listcrc"
#define CLICOMMAND_LISTROMS             "listroms"
#define CLICOMMAND_LISTSAMPLES          "listsamples"
#define CLICOMMAND_VERIFYROMS           "verifyroms"
#define CLICOMMAND_VERIFYSAMPLES        "verifysamples"
#define CLICOMMAND_ROMIDENT             "romident"
#define CLICOMMAND_LISTDEVICES          "listdevices"
#define CLICOMMAND_LISTSLOTS            "listslots"
#define CLICOMMAND_LISTMEDIA            "listmedia"     // needed by MESS
#define CLICOMMAND_LISTSOFTWARE         "listsoftware"
#define CLICOMMAND_VERIFYSOFTWARE       "verifysoftware"
#define CLICOMMAND_GETSOFTLIST          "getsoftlist"
#define CLICOMMAND_VERIFYSOFTLIST       "verifysoftlist"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// cli_options wraps the general emu options with CLI-specific additions
class cli_options : public emu_options
{
public:
	// construction/destruction
	cli_options();

private:
	static const options_entry s_option_entries[];
};

#endif  /* __CLIFRONT_H__ */
