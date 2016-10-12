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


//-------------------------------------------------
//  console_frontend - constructor
//-------------------------------------------------

console_frontend::console_frontend(emu_options &options, osd_interface &osd)
	: m_options(options),
	  m_osd(osd)
{
}


//-------------------------------------------------
//  ~console_frontend - destructor
//-------------------------------------------------

console_frontend::~console_frontend()
{
}


/*
static const char* examples[] = {
	"db", "hello", "hallo", "hans", "hansekogge", "seamann", "quetzalcoatl", "quit", "power", NULL
};

void completionHook(char const* prefix, linenoiseCompletions* lc) {
	size_t i;

	for (i = 0; examples[i] != NULL; ++i) {
		if (strncmp(prefix, examples[i], strlen(prefix)) == 0) {
			linenoiseAddCompletion(lc, examples[i]);
		}
	}
}
*/

static void read_console(std::atomic<bool>& run, std::atomic<bool>& wait, std::string &cmdLine)
{
	char const* prompt = "\x1b[1;36m[MAME]\x1b[0m> ";

	while (run.load())
	{
		while (wait.load())
		{
			using namespace std::chrono_literals;
			std::this_thread::sleep_for(100ms);
		}
		char* result = linenoise(prompt);
		if (result == NULL)
		{
			continue;
		}
/*		else if (!strncmp(result, "/history", 8)) {
			// Display the current history. 
			for (int index = 0; ; ++index) {
				char* hist = linenoiseHistoryLine(index);
				if (hist == NULL) break;
				printf("%4d: %s\n", index, hist);
				free(hist);
			}
		}*/
		else if (*result == '\0') {
			free(result);
			continue;
		}
		cmdLine = std::string(result);
		linenoiseHistoryAdd(result);
		//prompt = "\x1b[1;36m[MAME]\x1b[0m \x1b[1;32m[test]\x1b[0m> ";

		free(result);
		wait.store(true);
	}
}

void console_frontend::start_console()
{
	linenoiseInstallWindowChangeHandler();
	std::string cmdLine = "";
	const char* file = "./history";

	linenoiseHistoryLoad(file);
	//linenoiseSetCompletionCallback(completionHook);

	// Display app info
	printf("    _/      _/    _/_/    _/      _/  _/_/_/_/\n");
	printf("   _/_/  _/_/  _/    _/  _/_/  _/_/  _/       \n");
	printf("  _/  _/  _/  _/_/_/_/  _/  _/  _/  _/_/_/    \n");
	printf(" _/      _/  _/    _/  _/      _/  _/         \n");
	printf("_/      _/  _/    _/  _/      _/  _/_/_/_/    \n");
	printf("\n");
	printf("%s v%s\n%s\n\n", emulator_info::get_appname(), build_version, emulator_info::get_copyright_info());

	std::atomic<bool> run(true), wait(false);
	std::thread cinThread(read_console, std::ref(run), std::ref(wait), std::ref(cmdLine));

	while (run.load())
	{
		if (wait.load())
		{
			//printf("command %s\n", cmdLine.c_str());
			cmdLine.clear();
			wait.store(false);
		} else {
			using namespace std::chrono_literals;
			std::this_thread::sleep_for(100ms);
		}
	}

	run.store(false);
	cinThread.join();

	linenoiseHistorySave(file);
	linenoiseHistoryFree();
}
