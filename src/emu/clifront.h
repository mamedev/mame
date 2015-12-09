// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    clifront.h

    Command-line interface frontend for MAME.

***************************************************************************/

#pragma once

#ifndef __CLIFRONT_H__
#define __CLIFRONT_H__

#include "emu.h"
#include "cliopts.h"

// don't include osd_interface in header files
class osd_interface;

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// cli_frontend handles command-line processing and emulator execution
class cli_frontend
{
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
	static int compare_devices(const void *i1, const void *i2);
	void listdevices(const char *gamename = "*");
	void listslots(const char *gamename = "*");
	void listmedia(const char *gamename = "*");
	void listsoftware(const char *gamename = "*");
	void verifysoftware(const char *gamename = "*");
	void verifyroms(const char *gamename = "*");
	void verifysamples(const char *gamename = "*");
	void romident(const char *filename);
	void getsoftlist(const char *gamename = "*");
	void verifysoftlist(const char *gamename = "*");

private:
	// internal helpers
	void execute_commands(const char *exename);
	void display_help();
	void display_suggestions(const char *gamename);
	void output_single_softlist(FILE *out, software_list_device &swlist);

	// internal state
	cli_options &       m_options;
	osd_interface &     m_osd;
	int                 m_result;
	UINT64              m_start_memory;
};




#endif  /* __CLIFRONT_H__ */
