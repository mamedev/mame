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
class mame_machine_manager;

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// cli_frontend handles command-line processing and emulator execution
class cli_frontend
{
	static const char s_softlist_xml_dtd[];

public:
	// construction/destruction
	cli_frontend(emu_options &options, osd_interface &osd);
	~cli_frontend();

	// execute based on the incoming argc/argv
	int execute(std::vector<std::string> &args);

private:
	struct info_command_struct
	{
		const char *option;
		int min_args;
		int max_args;
		void (cli_frontend::*function)(const std::vector<std::string> &args);
		const char *usage;
	};

	// commands
	void listxml(const std::vector<std::string> &args);
	void listfull(const std::vector<std::string> &args);
	void listsource(const std::vector<std::string> &args);
	void listclones(const std::vector<std::string> &args);
	void listbrothers(const std::vector<std::string> &args);
	void listcrc(const std::vector<std::string> &args);
	void listroms(const std::vector<std::string> &args);
	void listsamples(const std::vector<std::string> &args);
	void listdevices(const std::vector<std::string> &args);
	void listslots(const std::vector<std::string> &args);
	void listmedia(const std::vector<std::string> &args);
	void listsoftware(const std::vector<std::string> &args);
	void verifysoftware(const std::vector<std::string> &args);
	void verifyroms(const std::vector<std::string> &args);
	void verifysamples(const std::vector<std::string> &args);
	void romident(const std::vector<std::string> &args);
	void getsoftlist(const std::vector<std::string> &args);
	void verifysoftlist(const std::vector<std::string> &args);
	void version(const std::vector<std::string> &args);

	// internal helpers
	template <typename T, typename U> void apply_action(const std::vector<std::string> &args, T &&drvact, U &&devact);
	template <typename T> void apply_device_action(const std::vector<std::string> &args, T &&action);
	void execute_commands(const char *exename);
	void display_help(const char *exename);
	void output_single_softlist(FILE *out, software_list_device &swlist);
	void start_execution(mame_machine_manager *manager, const std::vector<std::string> &args);
	static const info_command_struct *find_command(const std::string &s);

	// internal state
	emu_options &       m_options;
	osd_interface &     m_osd;
	int                 m_result;
};

#endif  // MAME_FRONTEND_CLIFRONT_H
