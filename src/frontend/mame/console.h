// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    console.c

    Console interface frontend for MAME.

***************************************************************************/
#ifndef MAME_FRONTEND_CONSOLE_H
#define MAME_FRONTEND_CONSOLE_H

#pragma once

#include "emu.h"

struct linenoiseCompletions;

class console_frontend
{
public:
	// construction/destruction
	console_frontend(emu_options &options, osd_interface &osd);
	~console_frontend();

	void start_console();
	void completion(char const* prefix, linenoiseCompletions* lc);
	
private:
	void read_console(std::string &cmdLine);
	void split_command(std::vector<std::string>& arg, std::string command);

	void cmd_quit(std::vector<std::string>& arg);
	// internal state
	//emu_options &       m_options;
	//osd_interface &     m_osd;
	std::atomic<bool>   m_run;
	std::atomic<bool>   m_wait;
	std::string		    m_prompt;

	std::unordered_map<std::string, std::function<void(std::vector<std::string>& arg)>> m_commands;
};

#endif  /* MAME_FRONTEND_CONSOLE_H */
