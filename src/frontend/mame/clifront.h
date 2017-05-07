// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    clifront.h

    Command-line interface frontend for MAME.

***************************************************************************/
#ifndef MAME_FRONTEND_CLIFRONT_H
#define MAME_FRONTEND_CLIFRONT_H

#pragma once

#include "emuopts.h"

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
	cli_frontend(emu_options &options, osd_interface &osd);
	~cli_frontend();

	// execute based on the incoming argc/argv
	int execute(std::vector<std::string> &args);

	// direct access to the command operations
	void listxml(const std::string &gamename = "*");
	void listfull(const std::string &gamename = "*");
	void listsource(const std::string &gamename = "*");
	void listclones(const std::string &gamename = "*");
	void listbrothers(const std::string &gamename = "*");
	void listcrc(const std::string &gamename = "*");
	void listroms(const std::string &gamename = "*");
	void listsamples(const std::string &gamename = "*");
	void listdevices(const std::string &gamename = "*");
	void listslots(const std::string &gamename = "*");
	void listmedia(const std::string &gamename = "*");
	void listsoftware(const std::string &gamename = "*");
	void verifysoftware(const std::string &gamename = "*");
	void verifyroms(const std::string &gamename = "*");
	void verifysamples(const std::string &gamename = "*");
	void romident(const std::string &filename);
	void getsoftlist(const std::string &gamename = "*");
	void verifysoftlist(const std::string &gamename = "*");

private:
	// internal helpers
	void execute_commands(const char *exename);
	void display_help(const char *exename);
	void display_suggestions(const char *gamename);
	void output_single_softlist(FILE *out, software_list_device &swlist);
	void start_execution(mame_machine_manager *manager, std::vector<std::string> &args);

	// internal state
	emu_options &       m_options;
	osd_interface &     m_osd;
	int                 m_result;
};

#endif  /* MAME_FRONTEND_CLIFRONT_H */
