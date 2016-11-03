// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    console.c

    Console interface frontend for MAME.

***************************************************************************/

#include "emu.h"
#include "console.h"
#include "linenoise-ng/include/linenoise.h"
#include "mame.h"

#include <atomic>
#include <thread>

static console_frontend *gConsole = nullptr;
//-------------------------------------------------
//  console_frontend - constructor
//-------------------------------------------------

console_frontend::console_frontend(emu_options &options, osd_interface &osd)
	: //m_options(options),
	  //m_osd(osd),
	  m_run(true),
	  m_wait(false),
	  m_prompt("\x1b[1;36m[MAME]\x1b[0m> ")
{
	using namespace std::placeholders;
	m_commands.insert(std::make_pair("quit", std::bind(&console_frontend::cmd_quit, this, _1)));
	m_commands.insert(std::make_pair("exit", std::bind(&console_frontend::cmd_quit, this, _1)));
	gConsole = this;
}


//-------------------------------------------------
//  ~console_frontend - destructor
//-------------------------------------------------

console_frontend::~console_frontend()
{
}

void console_frontend::completion(char const* prefix, linenoiseCompletions* lc)
{
	for (auto cmd : m_commands)
	{
		if (strncmp(prefix, cmd.first.c_str(), strlen(prefix)) == 0)
		{
			linenoiseAddCompletion(lc, cmd.first.c_str());
		}
	}
}

void console_frontend::cmd_quit(std::vector<std::string>& arg)
{
	printf("Exiting application\n");
	m_run.store(false);
	m_wait.store(false);
}

void console_frontend::read_console(std::string &cmdLine)
{
	while (m_run.load())
	{
		while (m_wait.load())
		{
			using namespace std::chrono_literals;
			std::this_thread::sleep_for(100ms);
		}
		if (!m_run.load()) break;
		char* result = linenoise(m_prompt.c_str());
		if (result == NULL)
		{
			continue;
		}
		else if (*result == '\0') {
			free(result);
			continue;
		}
		cmdLine = std::string(result);
		linenoiseHistoryAdd(result);
		//m_prompt = "\x1b[1;36m[MAME]\x1b[0m \x1b[1;32m[test]\x1b[0m> ";

		free(result);
		m_wait.store(true);
	}
}

void console_frontend::split_command(std::vector<std::string>& arg, std::string command)
{
	int len = command.length();
	bool qot = false, sqot = false;
	int arglen;
	for (int i = 0; i < len; i++) {
		int start = i;
		if (command[i] == '\"') {
			qot = true;
		}
		else if (command[i] == '\'') sqot = true;

		if (qot) {
			i++;
			start++;
			while (i<len && command[i] != '\"')
				i++;
			if (i<len)
				qot = false;
			arglen = i - start;
			i++;
		}
		else if (sqot) {
			i++;
			while (i<len && command[i] != '\'')
				i++;
			if (i<len)
				sqot = false;
			arglen = i - start;
			i++;
		}
		else {
			while (i<len && command[i] != ' ')
				i++;
			arglen = i - start;
		}
		arg.push_back(command.substr(start, arglen));
	}
}

static void completionHook(char const* prefix, linenoiseCompletions* lc)
{
	gConsole->completion(prefix, lc);
}

void console_frontend::start_console()
{
	linenoiseInstallWindowChangeHandler();
	std::string cmdLine = "";
	const char* file = "./history";

	linenoiseHistoryLoad(file);
	linenoiseSetCompletionCallback(completionHook);

	// Display app info
	printf("    _/      _/    _/_/    _/      _/  _/_/_/_/\n");
	printf("   _/_/  _/_/  _/    _/  _/_/  _/_/  _/       \n");
	printf("  _/  _/  _/  _/_/_/_/  _/  _/  _/  _/_/_/    \n");
	printf(" _/      _/  _/    _/  _/      _/  _/         \n");
	printf("_/      _/  _/    _/  _/      _/  _/_/_/_/    \n");
	printf("\n");
	printf("%s v%s\n%s\n\n", emulator_info::get_appname(), build_version, emulator_info::get_copyright_info());


	std::thread cinThread(&console_frontend::read_console, this, std::ref(cmdLine));

	while (m_run.load())
	{
		if (m_wait.load())
		{
			std::vector<std::string> arg;
			split_command(arg, cmdLine);
			auto command = m_commands.find(arg[0]);
			if (command != m_commands.end())
			{
				command->second(arg);
			}
			else {
				printf("Unknown command: %s\n", arg[0].c_str());
			}
			cmdLine.clear();
			m_wait.store(false);
		} else {
			using namespace std::chrono_literals;
			std::this_thread::sleep_for(100ms);
		}
	}

	m_run.store(false);
	cinThread.join();

	linenoiseHistorySave(file);
	linenoiseHistoryFree();
}
