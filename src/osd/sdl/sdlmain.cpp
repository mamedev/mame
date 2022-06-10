// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont
//============================================================
//
//  sdlmain.c - main file for SDLMAME.
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================

// only for oslog callback
#include <functional>

#ifdef SDLMAME_UNIX
#if (!defined(SDLMAME_MACOSX)) && (!defined(SDLMAME_EMSCRIPTEN)) && (!defined(SDLMAME_ANDROID))
#ifndef SDLMAME_HAIKU
#include <fontconfig/fontconfig.h>
#endif
#endif
#ifdef SDLMAME_MACOSX
#define __ASSERT_MACROS_DEFINE_VERSIONS_WITHOUT_UNDERSCORES 0
#include <Carbon/Carbon.h>
#endif
#endif

// standard includes
#if !defined(SDLMAME_WIN32)
#include <unistd.h>
#endif

// only for strconv.h
#if defined(SDLMAME_WIN32)
#include <windows.h>
#endif

#include <SDL2/SDL.h>

// MAME headers
#include "corestr.h"
#include "osdepend.h"
#include "emu.h"
#include "emuopts.h"
#include "strconv.h"

// OSD headers
#include "video.h"
#include "osdsdl.h"
#include "modules/lib/osdlib.h"
#include "modules/diagnostics/diagnostics_module.h"

//============================================================
//  OPTIONS
//============================================================

#ifndef INI_PATH
#if defined(SDLMAME_WIN32)
	#define INI_PATH ".;ini;ini/presets"
#elif defined(SDLMAME_MACOSX)
	#define INI_PATH "$HOME/Library/Application Support/APP_NAME;$HOME/.APP_NAME;.;ini"
#else
	#define INI_PATH "$HOME/.APP_NAME;.;ini"
#endif // MACOSX
#endif // INI_PATH


//============================================================
//  Global variables
//============================================================

#if defined(SDLMAME_UNIX) || defined(SDLMAME_WIN32)
int sdl_entered_debugger;
#endif

//============================================================
//  Local variables
//============================================================

const options_entry sdl_options::s_option_entries[] =
{
	{ SDLOPTION_INIPATH,                     INI_PATH,       core_options::option_type::STRING,     "path to ini files" },

	// performance options
	{ nullptr,                               nullptr,        core_options::option_type::HEADER,     "SDL PERFORMANCE OPTIONS" },
	{ SDLOPTION_SDLVIDEOFPS,                 "0",            core_options::option_type::BOOLEAN,    "show sdl video performance" },
	// video options
	{ nullptr,                               nullptr,        core_options::option_type::HEADER,     "SDL VIDEO OPTIONS" },
// OS X can be trusted to have working hardware OpenGL, so default to it on for the best user experience
	{ SDLOPTION_CENTERH,                     "1",            core_options::option_type::BOOLEAN,    "center horizontally within the view area" },
	{ SDLOPTION_CENTERV,                     "1",            core_options::option_type::BOOLEAN,    "center vertically within the view area" },
	{ SDLOPTION_SCALEMODE ";sm",             OSDOPTVAL_NONE, core_options::option_type::STRING,     "Scale mode: none, hwblit, hwbest, yv12, yuy2, yv12x2, yuy2x2 (-video soft only)" },

	// full screen options
#ifdef SDLMAME_X11
	{ nullptr,                               nullptr,        core_options::option_type::HEADER,     "SDL FULL SCREEN OPTIONS" },
	{ SDLOPTION_USEALLHEADS,                 "0",            core_options::option_type::BOOLEAN,    "split full screen image across monitors" },
	{ SDLOPTION_ATTACH_WINDOW,               "",             core_options::option_type::STRING,     "attach to arbitrary window" },
#endif // SDLMAME_X11

	// keyboard mapping
	{ nullptr,                               nullptr,        core_options::option_type::HEADER,     "SDL KEYBOARD MAPPING" },
	{ SDLOPTION_KEYMAP,                      "0",            core_options::option_type::BOOLEAN,    "enable keymap" },
	{ SDLOPTION_KEYMAP_FILE,                 "keymap.dat",   core_options::option_type::STRING,     "keymap filename" },

	// joystick mapping
	{ nullptr,                               nullptr,        core_options::option_type::HEADER,     "SDL JOYSTICK MAPPING" },
	{ SDLOPTION_JOYINDEX "1",                OSDOPTVAL_AUTO, core_options::option_type::STRING,     "name of joystick mapped to joystick #1" },
	{ SDLOPTION_JOYINDEX "2",                OSDOPTVAL_AUTO, core_options::option_type::STRING,     "name of joystick mapped to joystick #2" },
	{ SDLOPTION_JOYINDEX "3",                OSDOPTVAL_AUTO, core_options::option_type::STRING,     "name of joystick mapped to joystick #3" },
	{ SDLOPTION_JOYINDEX "4",                OSDOPTVAL_AUTO, core_options::option_type::STRING,     "name of joystick mapped to joystick #4" },
	{ SDLOPTION_JOYINDEX "5",                OSDOPTVAL_AUTO, core_options::option_type::STRING,     "name of joystick mapped to joystick #5" },
	{ SDLOPTION_JOYINDEX "6",                OSDOPTVAL_AUTO, core_options::option_type::STRING,     "name of joystick mapped to joystick #6" },
	{ SDLOPTION_JOYINDEX "7",                OSDOPTVAL_AUTO, core_options::option_type::STRING,     "name of joystick mapped to joystick #7" },
	{ SDLOPTION_JOYINDEX "8",                OSDOPTVAL_AUTO, core_options::option_type::STRING,     "name of joystick mapped to joystick #8" },
	{ SDLOPTION_SIXAXIS,                     "0",            core_options::option_type::BOOLEAN,    "use special handling for PS3 Sixaxis controllers" },

#if (USE_XINPUT)
	// lightgun mapping
	{ nullptr,                               nullptr,        core_options::option_type::HEADER,     "SDL LIGHTGUN MAPPING" },
	{ SDLOPTION_LIGHTGUNINDEX "1",           OSDOPTVAL_AUTO, core_options::option_type::STRING,     "name of lightgun mapped to lightgun #1" },
	{ SDLOPTION_LIGHTGUNINDEX "2",           OSDOPTVAL_AUTO, core_options::option_type::STRING,     "name of lightgun mapped to lightgun #2" },
	{ SDLOPTION_LIGHTGUNINDEX "3",           OSDOPTVAL_AUTO, core_options::option_type::STRING,     "name of lightgun mapped to lightgun #3" },
	{ SDLOPTION_LIGHTGUNINDEX "4",           OSDOPTVAL_AUTO, core_options::option_type::STRING,     "name of lightgun mapped to lightgun #4" },
	{ SDLOPTION_LIGHTGUNINDEX "5",           OSDOPTVAL_AUTO, core_options::option_type::STRING,     "name of lightgun mapped to lightgun #5" },
	{ SDLOPTION_LIGHTGUNINDEX "6",           OSDOPTVAL_AUTO, core_options::option_type::STRING,     "name of lightgun mapped to lightgun #6" },
	{ SDLOPTION_LIGHTGUNINDEX "7",           OSDOPTVAL_AUTO, core_options::option_type::STRING,     "name of lightgun mapped to lightgun #7" },
	{ SDLOPTION_LIGHTGUNINDEX "8",           OSDOPTVAL_AUTO, core_options::option_type::STRING,     "name of lightgun mapped to lightgun #8" },
#endif

	{ nullptr,                               nullptr,        core_options::option_type::HEADER,     "SDL MOUSE MAPPING" },
	{ SDLOPTION_MOUSEINDEX "1",              OSDOPTVAL_AUTO, core_options::option_type::STRING,     "name of mouse mapped to mouse #1" },
	{ SDLOPTION_MOUSEINDEX "2",              OSDOPTVAL_AUTO, core_options::option_type::STRING,     "name of mouse mapped to mouse #2" },
	{ SDLOPTION_MOUSEINDEX "3",              OSDOPTVAL_AUTO, core_options::option_type::STRING,     "name of mouse mapped to mouse #3" },
	{ SDLOPTION_MOUSEINDEX "4",              OSDOPTVAL_AUTO, core_options::option_type::STRING,     "name of mouse mapped to mouse #4" },
	{ SDLOPTION_MOUSEINDEX "5",              OSDOPTVAL_AUTO, core_options::option_type::STRING,     "name of mouse mapped to mouse #5" },
	{ SDLOPTION_MOUSEINDEX "6",              OSDOPTVAL_AUTO, core_options::option_type::STRING,     "name of mouse mapped to mouse #6" },
	{ SDLOPTION_MOUSEINDEX "7",              OSDOPTVAL_AUTO, core_options::option_type::STRING,     "name of mouse mapped to mouse #7" },
	{ SDLOPTION_MOUSEINDEX "8",              OSDOPTVAL_AUTO, core_options::option_type::STRING,     "name of mouse mapped to mouse #8" },

	{ nullptr,                               nullptr,        core_options::option_type::HEADER,     "SDL KEYBOARD MAPPING" },
	{ SDLOPTION_KEYBINDEX "1",               OSDOPTVAL_AUTO, core_options::option_type::STRING,     "name of keyboard mapped to keyboard #1" },
	{ SDLOPTION_KEYBINDEX "2",               OSDOPTVAL_AUTO, core_options::option_type::STRING,     "name of keyboard mapped to keyboard #2" },
	{ SDLOPTION_KEYBINDEX "3",               OSDOPTVAL_AUTO, core_options::option_type::STRING,     "name of keyboard mapped to keyboard #3" },
	{ SDLOPTION_KEYBINDEX "4",               OSDOPTVAL_AUTO, core_options::option_type::STRING,     "name of keyboard mapped to keyboard #4" },
	{ SDLOPTION_KEYBINDEX "5",               OSDOPTVAL_AUTO, core_options::option_type::STRING,     "name of keyboard mapped to keyboard #5" },
	{ SDLOPTION_KEYBINDEX "6",               OSDOPTVAL_AUTO, core_options::option_type::STRING,     "name of keyboard mapped to keyboard #6" },
	{ SDLOPTION_KEYBINDEX "7",               OSDOPTVAL_AUTO, core_options::option_type::STRING,     "name of keyboard mapped to keyboard #7" },
	{ SDLOPTION_KEYBINDEX "8",               OSDOPTVAL_AUTO, core_options::option_type::STRING,     "name of keyboard mapped to keyboard #8" },

	// SDL low level driver options
	{ nullptr,                               nullptr,         core_options::option_type::HEADER,    "SDL LOW-LEVEL DRIVER OPTIONS" },
	{ SDLOPTION_VIDEODRIVER ";vd",           OSDOPTVAL_AUTO,  core_options::option_type::STRING,    "SDL video driver to use ('x11', 'directfb', ... or 'auto' for SDL default" },
	{ SDLOPTION_RENDERDRIVER ";rd",          OSDOPTVAL_AUTO,  core_options::option_type::STRING,    "SDL render driver to use ('software', 'opengl', 'directfb' ... or 'auto' for SDL default" },
	{ SDLOPTION_AUDIODRIVER ";ad",           OSDOPTVAL_AUTO,  core_options::option_type::STRING,    "SDL audio driver to use ('alsa', 'arts', ... or 'auto' for SDL default" },
#if USE_OPENGL
	{ SDLOPTION_GL_LIB,                      SDLOPTVAL_GLLIB, core_options::option_type::STRING,    "alternative libGL.so to use; 'auto' for system default" },
#endif

	// End of list
	{ nullptr }
};

//============================================================
//  sdl_options
//============================================================

sdl_options::sdl_options()
: osd_options()
{
#if defined (SDLMAME_ANDROID)
	chdir (SDL_AndroidGetExternalStoragePath());
#endif
	std::string ini_path(INI_PATH);
	add_entries(sdl_options::s_option_entries);
	strreplace(ini_path,"APP_NAME", emulator_info::get_appname_lower());
	set_default_value(SDLOPTION_INIPATH, std::move(ini_path));
}

//============================================================
//  main
//============================================================

// we do some special sauce on Win32...

#if defined(SDLMAME_WIN32)
/* gee */
extern "C" DECLSPEC void SDLCALL SDL_SetModuleHandle(void *hInst);
#endif

int main(int argc, char** argv)
{
	std::vector<std::string> args = osd_get_command_line(argc, argv);
	int res = 0;

	// disable I/O buffering
	setvbuf(stdout, (char *) nullptr, _IONBF, 0);
	setvbuf(stderr, (char *) nullptr, _IONBF, 0);

	// Initialize crash diagnostics
	diagnostics_module::get_instance()->init_crash_diagnostics();

#if defined(SDLMAME_ANDROID)
	/* Enable standard application logging */
	SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_VERBOSE);
#endif

	// FIXME: this should be done differently

#ifdef SDLMAME_UNIX
	sdl_entered_debugger = 0;
#if (!defined(SDLMAME_MACOSX)) && (!defined(SDLMAME_HAIKU)) && (!defined(SDLMAME_EMSCRIPTEN)) && (!defined(SDLMAME_ANDROID))
	FcInit();
#endif
#endif

	{
		sdl_options options;
		sdl_osd_interface osd(options);
		osd.register_options();
		res = emulator_info::start_frontend(options, osd, args);
	}

#ifdef SDLMAME_UNIX
#if (!defined(SDLMAME_MACOSX)) && (!defined(SDLMAME_HAIKU)) && (!defined(SDLMAME_EMSCRIPTEN)) && (!defined(SDLMAME_ANDROID))
	if (!sdl_entered_debugger)
	{
		FcFini();
	}
#endif
#endif

	exit(res);
}

//============================================================
//  constructor
//============================================================

sdl_osd_interface::sdl_osd_interface(sdl_options &options)
: osd_common_t(options), m_options(options)
{
}


//============================================================
//  destructor
//============================================================

sdl_osd_interface::~sdl_osd_interface()
{
}


//============================================================
//  osd_exit
//============================================================

void sdl_osd_interface::osd_exit()
{
	osd_common_t::osd_exit();

	SDL_QuitSubSystem(SDL_INIT_VIDEO);
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

#define _SDL_VER #SDL_MAJOR_VERSION "." #SDL_MINOR_VERSION "." #SDL_PATCHLEVEL

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
	osd_printf_verbose("SDL/OpenGL defines: ");
	osd_printf_verbose("SDL_COMPILEDVERSION=%d ", SDL_COMPILEDVERSION);
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
//  osd_sdl_info
//============================================================

static void osd_sdl_info(void)
{
	int i, num = SDL_GetNumVideoDrivers();

	osd_printf_verbose("Available videodrivers: ");
	for (i=0;i<num;i++)
	{
		const char *name = SDL_GetVideoDriver(i);
		osd_printf_verbose("%s ", name);
	}
	osd_printf_verbose("\n");
	osd_printf_verbose("Current Videodriver: %s\n", SDL_GetCurrentVideoDriver());
	num = SDL_GetNumVideoDisplays();
	for (i=0;i<num;i++)
	{
		SDL_DisplayMode mode;
		int j;

		osd_printf_verbose("\tDisplay #%d\n", i);
		if (SDL_GetDesktopDisplayMode(i, &mode))
			osd_printf_verbose("\t\tDesktop Mode:         %dx%d-%d@%d\n", mode.w, mode.h, SDL_BITSPERPIXEL(mode.format), mode.refresh_rate);
		if (SDL_GetCurrentDisplayMode(i, &mode))
			osd_printf_verbose("\t\tCurrent Display Mode: %dx%d-%d@%d\n", mode.w, mode.h, SDL_BITSPERPIXEL(mode.format), mode.refresh_rate);
		osd_printf_verbose("\t\tRenderdrivers:\n");
		for (j=0; j<SDL_GetNumRenderDrivers(); j++)
		{
			SDL_RendererInfo info;
			SDL_GetRenderDriverInfo(j, &info);
			osd_printf_verbose("\t\t\t%10s (%dx%d)\n", info.name, info.max_texture_width, info.max_texture_height);
		}
	}

	osd_printf_verbose("Available audio drivers: \n");
	num = SDL_GetNumAudioDrivers();
	for (i=0;i<num;i++)
	{
		osd_printf_verbose("\t%-20s\n", SDL_GetAudioDriver(i));
	}
}


//============================================================
//  video_register
//============================================================

void sdl_osd_interface::video_register()
{
	video_options_add("soft", nullptr);
	video_options_add("accel", nullptr);
#if USE_OPENGL
	video_options_add("opengl", nullptr);
#endif
	video_options_add("bgfx", nullptr);
	//video_options_add("auto", nullptr); // making d3d video default one
}


//============================================================
//  output_oslog
//============================================================

void sdl_osd_interface::output_oslog(const char *buffer)
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

void sdl_osd_interface::init(running_machine &machine)
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

	// Some driver options - must be before audio init!
	stemp = options().audio_driver();
	if (stemp != nullptr && strcmp(stemp, OSDOPTVAL_AUTO) != 0)
	{
		osd_printf_verbose("Setting SDL audiodriver '%s' ...\n", stemp);
		osd_setenv(SDLENV_AUDIODRIVER, stemp, 1);
	}

	stemp = options().video_driver();
	if (stemp != nullptr)
	{
		if (strcmp(stemp, OSDOPTVAL_AUTO) != 0)
		{
			osd_printf_verbose("Setting SDL videodriver '%s' ...\n", stemp);
			osd_setenv(SDLENV_VIDEODRIVER, stemp, 1);
		}
		else
		{
#if defined(__linux__)
			// bgfx does not work with wayland
			osd_printf_verbose("Setting SDL videodriver '%s' ...\n", "x11");
			osd_setenv(SDLENV_VIDEODRIVER, "x11", 1);
#endif
		}
	}

	stemp = options().render_driver();
	if (stemp != nullptr)
	{
		if (strcmp(stemp, OSDOPTVAL_AUTO) != 0)
		{
			osd_printf_verbose("Setting SDL renderdriver '%s' ...\n", stemp);
			//osd_setenv(SDLENV_RENDERDRIVER, stemp, 1);
			SDL_SetHint(SDL_HINT_RENDER_DRIVER, stemp);
		}
		else
		{
#if defined(SDLMAME_WIN32)
			// OpenGL renderer has less issues with mode switching on windows
			osd_printf_verbose("Setting SDL renderdriver '%s' ...\n", "opengl");
			//osd_setenv(SDLENV_RENDERDRIVER, stemp, 1);
			SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");
#endif
		}
	}

	/* Set the SDL environment variable for drivers wanting to load the
	 * lib at startup.
	 */
#if USE_OPENGL
	/* FIXME: move lib loading code from drawogl.c here */

	stemp = options().gl_lib();
	if (stemp != nullptr && strcmp(stemp, OSDOPTVAL_AUTO) != 0)
	{
		osd_setenv("SDL_VIDEO_GL_DRIVER", stemp, 1);
		osd_printf_verbose("Setting SDL_VIDEO_GL_DRIVER = '%s' ...\n", stemp);
	}
#endif

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

	/* Initialize SDL */

	if (SDL_InitSubSystem(SDL_INIT_VIDEO))
	{
		osd_printf_error("Could not initialize SDL %s\n", SDL_GetError());
		exit(-1);
	}
	osd_sdl_info();

	defines_verbose();

	osd_common_t::init_subsystems();

	if (options().oslog())
	{
		using namespace std::placeholders;
		machine.add_logerror_callback(std::bind(&sdl_osd_interface::output_oslog, this, _1));
	}



#ifdef SDLMAME_EMSCRIPTEN
	SDL_EventState(SDL_TEXTINPUT, SDL_FALSE);
#else
	SDL_EventState(SDL_TEXTINPUT, SDL_TRUE);
#endif
}
