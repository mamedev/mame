// license:BSD-3-Clause
// copyright-holders:R. Belmont
//============================================================
//
//  macmain.cpp - C++ side main file for MAME on the Mac
//
//  Mac OSD by R. Belmont
//
//============================================================

// oslog callback
#include <functional>

#define __ASSERT_MACROS_DEFINE_VERSIONS_WITHOUT_UNDERSCORES 0
#include <Carbon/Carbon.h>
#include <crt_externs.h>
#include <sys/sysctl.h>
#include <unistd.h>

// MAME headers
#include "corestr.h"
#include "osdepend.h"
#include "emu.h"
#include "main.h"
#include "emuopts.h"
#include "strconv.h"

// OSD headers
#include "video.h"
#include "osdmac.h"
#include "modules/lib/osdlib.h"
#include "modules/diagnostics/diagnostics_module.h"

//============================================================
//  OPTIONS
//============================================================

#ifndef INI_PATH
#define INI_PATH "$HOME/Library/Application Support/APP_NAME;$HOME/.APP_NAME;.;ini"
#endif // INI_PATH

//============================================================
//  Local variables
//============================================================

const options_entry mac_options::s_option_entries[] =
{
	{MACOPTION_INIPATH, INI_PATH, core_options::option_type::STRING, "path to ini files"},

	// End of list
	{nullptr}
};

//============================================================
//  mac_options
//============================================================

mac_options::mac_options()
: osd_options()
{
	std::string ini_path(INI_PATH);
	add_entries(mac_options::s_option_entries);
	strreplace(ini_path,"APP_NAME", emulator_info::get_appname_lower());
	set_default_value(MACOPTION_INIPATH, std::move(ini_path));
}

//============================================================
//  mac_run_emulator - this is the MAME entry point from the
//                     Cocoa shell
//============================================================

int mac_run_emulator(int argc, char *argv[])
{
	std::vector<std::string> args = osd_get_command_line(argc, argv);
	int res = 0;

	// disable I/O buffering
	setvbuf(stdout, (char *) nullptr, _IONBF, 0);
	setvbuf(stderr, (char *) nullptr, _IONBF, 0);

	// Initialize crash diagnostics
	diagnostics_module::get_instance()->init_crash_diagnostics();

	mac_options options;
	mac_osd_interface osd(options);
	osd.register_options();
	res = emulator_info::start_frontend(options, osd, args);

	return res;
}

//============================================================
//  constructor
//============================================================

mac_osd_interface::mac_osd_interface(mac_options &options)
: osd_common_t(options), m_options(options)
{
}


//============================================================
//  destructor
//============================================================

mac_osd_interface::~mac_osd_interface()
{
}


//============================================================
//  osd_exit
//============================================================

void mac_osd_interface::osd_exit()
{
	osd_common_t::osd_exit();
}

//============================================================
//  defines_verbose
//============================================================

#define MAC_EXPAND_STR(_m) #_m
#define MACRO_VERBOSE(_mac) \
	do { \
		if (strcmp(MAC_EXPAND_STR(_mac), #_mac) != 0) \
			osd_printf_verbose("%s=%s ", #_mac, MAC_EXPAND_STR(_mac)); \
	} while (0)

static void defines_verbose(void)
{
	osd_printf_verbose("Build version:      %s\n", emulator_info::get_build_version());

#if defined(__arm64__)
	const char *const build_arch = "arm64";
#elif defined(__x86_64__)
	const char *const build_arch = "x86-64";
#else
	const char *const build_arch = "unknown";
#endif

	// get the CPU brand string ("Apple M3 Pro", "Intel(R) Core(TM) ...")
	char brand[64] = "unknown CPU";
	size_t size = sizeof(brand);
	sysctlbyname("machdep.cpu.brand_string", brand, &size, nullptr, 0);

	// detect an Intel build running on Apple Silicon under Rosetta 2
	int translated = 0;
	size = sizeof(translated);
	if (sysctlbyname("sysctl.proc_translated", &translated, &size, nullptr, 0) != 0)
	{
		translated = 0;
	}

	osd_printf_verbose("Build architecture: %s on %s%s\n", build_arch, brand, translated ? " (Rosetta 2)" : "");
	osd_printf_verbose("Build defines 1:    ");
	MACRO_VERBOSE(SDLMAME_UNIX);
	MACRO_VERBOSE(SDLMAME_MACOSX);
	MACRO_VERBOSE(SDLMAME_DARWIN);
	osd_printf_verbose("\n");
	osd_printf_verbose("Build defines 1:    ");
	MACRO_VERBOSE(LSB_FIRST);
	MACRO_VERBOSE(MAME_NOASM);
	MACRO_VERBOSE(MAME_DEBUG);
	MACRO_VERBOSE(BIGENDIAN);
	MACRO_VERBOSE(CPP_COMPILE);
	MACRO_VERBOSE(SYNC_IMPLEMENTATION);
	osd_printf_verbose("\n");
	osd_printf_verbose("OpenGL defines: ");
	MACRO_VERBOSE(USE_OPENGL);
	osd_printf_verbose("\n");
	osd_printf_verbose("Compiler defines A: ");
	MACRO_VERBOSE(__GNUC__);
	MACRO_VERBOSE(__GNUC_MINOR__);
	MACRO_VERBOSE(__GNUC_PATCHLEVEL__);
	MACRO_VERBOSE(__VERSION__);
	osd_printf_verbose("\n");
	osd_printf_verbose("Compiler defines B: ");
	MACRO_VERBOSE(__arm64__);
	MACRO_VERBOSE(__amd64__);
	MACRO_VERBOSE(__x86_64__);
	MACRO_VERBOSE(__unix__);
	osd_printf_verbose("\n");
	osd_printf_verbose("Compiler defines C: ");
	MACRO_VERBOSE(_FORTIFY_SOURCE);
	MACRO_VERBOSE(__USE_FORTIFY_LEVEL);
	osd_printf_verbose("\n");
}

//============================================================
//  output_oslog
//============================================================

void mac_osd_interface::output_oslog(const char *buffer)
{
	fputs(buffer, stderr);
}


//============================================================
//  osd_setup_osd_specific_emu_options
//============================================================

void osd_setup_osd_specific_emu_options(emu_options &opts)
{
	opts.add_entries(osd_options::s_option_entries);
}


//============================================================
//  init
//============================================================

void mac_osd_interface::init(running_machine &machine)
{
	// call our parent
	osd_common_t::init(machine);

	const char *stemp;

	// determine if we are benchmarking, and adjust options appropriately
	int bench = options().bench();
	if (bench > 0)
	{
		options().set_value(OPTION_SLEEP, false, OPTION_PRIORITY_MAXIMUM);
		options().set_value(OPTION_THROTTLE, false, OPTION_PRIORITY_MAXIMUM);
		options().set_value(OSDOPTION_SOUND, "none", OPTION_PRIORITY_MAXIMUM);
		options().set_value(OSDOPTION_VIDEO, "none", OPTION_PRIORITY_MAXIMUM);
		options().set_value(OPTION_SECONDS_TO_RUN, bench, OPTION_PRIORITY_MAXIMUM);
	}

	/* get number of processors */
	stemp = options().numprocessors();

	osd_num_processors = 0;

	if (strcmp(stemp, "auto") != 0)
	{
		osd_num_processors = atoi(stemp);
		if (osd_num_processors < 1)
		{
			osd_printf_warning("numprocessors < 1 doesn't make much sense. Assuming auto ...\n");
			osd_num_processors = 0;
		}
	}

	defines_verbose();

	osd_common_t::init_subsystems();

	if (options().oslog())
	{
		using namespace std::placeholders;
		machine.add_logerror_callback(std::bind(&mac_osd_interface::output_oslog, this, _1));
	}
}
