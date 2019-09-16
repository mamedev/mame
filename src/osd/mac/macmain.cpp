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
#include <unistd.h>

// MAME headers
#include "osdepend.h"
#include "emu.h"
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
	{ MACOPTION_INIPATH,                     INI_PATH,    OPTION_STRING,     "path to ini files" },

	// End of list
	{ nullptr }
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
	set_default_value(MACOPTION_INIPATH, ini_path.c_str());
}

//============================================================
//  main
//============================================================

// we do some special sauce on Win32...
int mac_run_emulator()
{
	int argc = *_NSGetArgc();
	char **argv = *_NSGetArgv();

	std::vector<std::string> args = osd_get_command_line(argc, argv);
	int res = 0;

	// disable I/O buffering
	setvbuf(stdout, (char *) nullptr, _IONBF, 0);
	setvbuf(stderr, (char *) nullptr, _IONBF, 0);

	// Initialize crash diagnostics
	diagnostics_module::get_instance()->init_crash_diagnostics();

	{
		mac_options options;
		mac_osd_interface osd(options);
		osd.register_options();
		res = emulator_info::start_frontend(options, osd, args);
	}

	exit(res);
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

#define _mac_VER #mac_MAJOR_VERSION "." #mac_MINOR_VERSION "." #mac_PATCHLEVEL

static void defines_verbose(void)
{
	osd_printf_verbose("Build version:      %s\n", emulator_info::get_build_version());
	osd_printf_verbose("Build architecure:  ");
	MACRO_VERBOSE(SDLMAME_ARCH);
	osd_printf_verbose("\n");
	osd_printf_verbose("Build defines 1:    ");
	MACRO_VERBOSE(SDLMAME_UNIX);
	MACRO_VERBOSE(SDLMAME_X11);
	MACRO_VERBOSE(SDLMAME_WIN32);
	MACRO_VERBOSE(SDLMAME_MACOSX);
	MACRO_VERBOSE(SDLMAME_DARWIN);
	MACRO_VERBOSE(SDLMAME_LINUX);
	MACRO_VERBOSE(SDLMAME_SOLARIS);
	MACRO_VERBOSE(SDLMAME_IRIX);
	MACRO_VERBOSE(SDLMAME_BSD);
	osd_printf_verbose("\n");
	osd_printf_verbose("Build defines 1:    ");
	MACRO_VERBOSE(LSB_FIRST);
	MACRO_VERBOSE(PTR64);
	MACRO_VERBOSE(MAME_NOASM);
	MACRO_VERBOSE(MAME_DEBUG);
	MACRO_VERBOSE(BIGENDIAN);
	MACRO_VERBOSE(CPP_COMPILE);
	MACRO_VERBOSE(SYNC_IMPLEMENTATION);
	osd_printf_verbose("\n");
	osd_printf_verbose("OpenGL defines: ");
	MACRO_VERBOSE(USE_OPENGL);
	MACRO_VERBOSE(USE_DISPATCH_GL);
	osd_printf_verbose("\n");
	osd_printf_verbose("Compiler defines A: ");
	MACRO_VERBOSE(__GNUC__);
	MACRO_VERBOSE(__GNUC_MINOR__);
	MACRO_VERBOSE(__GNUC_PATCHLEVEL__);
	MACRO_VERBOSE(__VERSION__);
	osd_printf_verbose("\n");
	osd_printf_verbose("Compiler defines B: ");
	MACRO_VERBOSE(__amd64__);
	MACRO_VERBOSE(__x86_64__);
	MACRO_VERBOSE(__unix__);
	MACRO_VERBOSE(__i386__);
	MACRO_VERBOSE(__ppc__);
	MACRO_VERBOSE(__ppc64__);
	osd_printf_verbose("\n");
	osd_printf_verbose("Compiler defines C: ");
	MACRO_VERBOSE(_FORTIFY_SOURCE);
	MACRO_VERBOSE(__USE_FORTIFY_LEVEL);
	osd_printf_verbose("\n");
}

//============================================================
//  osd_mac_info
//============================================================

static void osd_mac_info(void)
{
}


//============================================================
//  video_register
//============================================================

void mac_osd_interface::video_register()
{
	video_options_add("opengl", nullptr);
	video_options_add("bgfx", nullptr);
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

	osd_mac_info();

	defines_verbose();

	osd_common_t::init_subsystems();

	if (options().oslog())
	{
		using namespace std::placeholders;
		machine.add_logerror_callback(std::bind(&mac_osd_interface::output_oslog, this, _1));
	}
}
