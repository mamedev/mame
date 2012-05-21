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
#include "drivenum.h"


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
#define CLICOMMAND_LISTSLOTS			"listslots"
#define CLICOMMAND_LISTMEDIA			"listmedia"		// needed by MESS
#define CLICOMMAND_LISTSOFTWARE			"listsoftware"
#define CLICOMMAND_GETSOFTLIST			"getsoftlist"



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


// cli_frontend handles command-line processing and emulator execution
class cli_frontend
{
	typedef tagmap_t<FPTR> int_map;
public:
	// construction/destruction
	cli_frontend(cli_options &options, osd_interface &osd);
	~cli_frontend();

	// execute based on the incoming argc/argv
	int execute(int argc, char **argv);

	// direct access to the command operations
	void listxml(const char *gamename = "*");
	void listfull(const char *gamename = "*");
	void listsource(const char *gamename = "*");
	void listclones(const char *gamename = "*");
	void listbrothers(const char *gamename = "*");
	void listcrc(const char *gamename = "*");
	void listroms(const char *gamename = "*");
	void listsamples(const char *gamename = "*");
	void listdevices(const char *gamename = "*");
	void listslots(const char *gamename = "*");
	void listmedia(const char *gamename = "*");
	void listsoftware(const char *gamename = "*");
	void verifyroms(const char *gamename = "*");
	void verifysamples(const char *gamename = "*");
	void romident(const char *filename);
	void getsoftlist(const char *gamename = "*");

private:
	// internal helpers
	void execute_commands(const char *exename);
	void display_help();
	void display_suggestions(const char *gamename);
	void output_single_softlist(FILE *out,software_list *list, const char *listname);

	// internal state
	cli_options &		m_options;
	osd_interface &		m_osd;
	int					m_result;
};


// media_identifier class identifies media by hash via a search in
// the driver database
class media_identifier
{
public:
	// construction/destruction
	media_identifier(cli_options &options);

	// getters
	int total() const { return m_total; }
	int matches() const { return m_matches; }
	int nonroms() const { return m_nonroms; }

	// operations
	void reset() { m_total = m_matches = m_nonroms = 0; }
	void identify(const char *name);
	void identify_file(const char *name);
	void identify_data(const char *name, const UINT8 *data, int length);
	int find_by_hash(const hash_collection &hashes, int length);

private:
	// internal state
	driver_enumerator	m_drivlist;
	int					m_total;
	int					m_matches;
	int					m_nonroms;
};



#endif	/* __CLIFRONT_H__ */
