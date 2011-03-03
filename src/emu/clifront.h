/***************************************************************************

    clifront.h

    Command-line interface frontend for MAME.

****************************************************************************

    Copyright Aaron Giles
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name 'MAME' nor the names of its contributors may be
          used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

***************************************************************************/

#pragma once

#ifndef __CLIFRONT_H__
#define __CLIFRONT_H__

#include "emuopts.h"


//**************************************************************************
//  CONSTANTS
//**************************************************************************

// core commands
#define CLICOMMAND_HELP					"help"
#define CLICOMMAND_VALIDATE				"validate"

// configuration commands
#define CLICOMMAND_CREATECONFIG			"createconfig"
#define CLICOMMAND_SHOWCONFIG			"showconfig"
#define CLICOMMAND_SHOWUSAGE			"showusage"

// frontend commands
#define CLICOMMAND_LISTXML				"listxml"
#define CLICOMMAND_LISTFULL				"listfull"
#define CLICOMMAND_LISTSOURCE			"listsource"
#define CLICOMMAND_LISTCLONES			"listclones"
#define CLICOMMAND_LISTBROTHERS			"listbrothers"
#define CLICOMMAND_LISTCRC				"listcrc"
#define CLICOMMAND_LISTROMS				"listroms"
#define CLICOMMAND_LISTSAMPLES			"listsamples"
#define CLICOMMAND_VERIFYROMS			"verifyroms"
#define CLICOMMAND_VERIFYSAMPLES		"verifysamples"
#define CLICOMMAND_ROMIDENT				"romident"
#define CLICOMMAND_LISTDEVICES			"listdevices"
#define CLICOMMAND_LISTMEDIA			"listmedia"		// needed by MESS
#define CLICOMMAND_LISTSOFTWARE			"listsoftware"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class cli_options : public emu_options
{
public:
	// construction/destruction
	cli_options();

private:
	static const options_entry s_option_entries[];
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

int cli_execute(cli_options &options, osd_interface &osd, int argc, char **argv);

/* informational functions */
void cli_info_listxml(emu_options &options, const char *gamename);
void cli_info_listfull(emu_options &options, const char *gamename);
void cli_info_listsource(emu_options &options, const char *gamename);
void cli_info_listclones(emu_options &options, const char *gamename);
void cli_info_listbrothers(emu_options &options, const char *gamename);
void cli_info_listcrc(emu_options &options, const char *gamename);
void cli_info_listroms(emu_options &options, const char *gamename);
void cli_info_listsamples(emu_options &options, const char *gamename);
void cli_info_listdevices(emu_options &options, const char *gamename);

#endif	/* __CLIFRONT_H__ */
