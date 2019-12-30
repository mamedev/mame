
// only for oslog callback
#include <functional>

// standard includes
#if !defined(RETROMAME_WIN32)
#include <unistd.h>
#endif

// only for strconv.h
#if defined(RETROMAME_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

// MAME headers
#include "osdepend.h"
#include "emu.h"
#include "emuopts.h"
#include "strconv.h"

// OSD headers
#include "video.h"
#include "osdretro.h"
#include "modules/lib/osdlib.h"
#include "modules/diagnostics/diagnostics_module.h"

//============================================================
//  OPTIONS
//============================================================

#ifndef INI_PATH
#if defined(RETROMAME_WIN32)
	#define INI_PATH ".;ini;ini/presets"
#elif defined(RETROMAME_MACOSX)
	#define INI_PATH "$HOME/Library/Application Support/APP_NAME;$HOME/.APP_NAME;.;ini"
#else
	#define INI_PATH "$HOME/.APP_NAME;.;ini"
#endif // MACOSX
#endif // INI_PATH


//============================================================
//  Global variables
//============================================================

#if defined(RETROMAME_UNIX) || defined(RETROMAME_WIN32)
int retro_entered_debugger;
#endif

//============================================================
//  Local variables
//============================================================

const options_entry retro_options::s_option_entries[] =
{
	{ RETROOPTION_INIPATH,                     INI_PATH,    OPTION_STRING,     "path to ini files" },


	// video options
	{ nullptr,                                nullptr,       OPTION_HEADER,     "SDL VIDEO OPTIONS" },
// OS X can be trusted to have working hardware OpenGL, so default to it on for the best user experience
	{ RETROOPTION_CENTERH,                      "1",        OPTION_BOOLEAN,    "center horizontally within the view area" },
	{ RETROOPTION_CENTERV,                      "1",        OPTION_BOOLEAN,    "center vertically within the view area" },
	{ RETROOPTION_SCALEMODE ";sm",         OSDOPTVAL_NONE,  OPTION_STRING,     "Scale mode: none, hwblit, hwbest, yv12, yuy2, yv12x2, yuy2x2 (-video soft only)" },


	// joystick mapping
	{ nullptr,                               nullptr,   OPTION_HEADER,     "SDL JOYSTICK MAPPING" },
	{ RETROOPTION_JOYINDEX "1",                OSDOPTVAL_AUTO, OPTION_STRING,         "name of joystick mapped to joystick #1" },
	{ RETROOPTION_JOYINDEX "2",                OSDOPTVAL_AUTO, OPTION_STRING,         "name of joystick mapped to joystick #2" },
	{ RETROOPTION_JOYINDEX "3",                OSDOPTVAL_AUTO, OPTION_STRING,         "name of joystick mapped to joystick #3" },
	{ RETROOPTION_JOYINDEX "4",                OSDOPTVAL_AUTO, OPTION_STRING,         "name of joystick mapped to joystick #4" },
	{ RETROOPTION_JOYINDEX "5",                OSDOPTVAL_AUTO, OPTION_STRING,         "name of joystick mapped to joystick #5" },
	{ RETROOPTION_JOYINDEX "6",                OSDOPTVAL_AUTO, OPTION_STRING,         "name of joystick mapped to joystick #6" },
	{ RETROOPTION_JOYINDEX "7",                OSDOPTVAL_AUTO, OPTION_STRING,         "name of joystick mapped to joystick #7" },
	{ RETROOPTION_JOYINDEX "8",                OSDOPTVAL_AUTO, OPTION_STRING,         "name of joystick mapped to joystick #8" },
	{ RETROOPTION_SIXAXIS,                     "0",    OPTION_BOOLEAN,    "Use special handling for PS3 Sixaxis controllers" },

#if (USE_XINPUT)
	// lightgun mapping
	{ nullptr,                               nullptr,   OPTION_HEADER,     "SDL LIGHTGUN MAPPING" },
	{ RETROOPTION_LIGHTGUNINDEX "1",           OSDOPTVAL_AUTO, OPTION_STRING,         "name of lightgun mapped to lightgun #1" },
	{ RETROOPTION_LIGHTGUNINDEX "2",           OSDOPTVAL_AUTO, OPTION_STRING,         "name of lightgun mapped to lightgun #2" },
	{ RETROOPTION_LIGHTGUNINDEX "3",           OSDOPTVAL_AUTO, OPTION_STRING,         "name of lightgun mapped to lightgun #3" },
	{ RETROOPTION_LIGHTGUNINDEX "4",           OSDOPTVAL_AUTO, OPTION_STRING,         "name of lightgun mapped to lightgun #4" },
	{ RETROOPTION_LIGHTGUNINDEX "5",           OSDOPTVAL_AUTO, OPTION_STRING,         "name of lightgun mapped to lightgun #5" },
	{ RETROOPTION_LIGHTGUNINDEX "6",           OSDOPTVAL_AUTO, OPTION_STRING,         "name of lightgun mapped to lightgun #6" },
	{ RETROOPTION_LIGHTGUNINDEX "7",           OSDOPTVAL_AUTO, OPTION_STRING,         "name of lightgun mapped to lightgun #7" },
	{ RETROOPTION_LIGHTGUNINDEX "8",           OSDOPTVAL_AUTO, OPTION_STRING,         "name of lightgun mapped to lightgun #8" },
#endif

	{ nullptr,                               nullptr,   OPTION_HEADER,     "SDL MOUSE MAPPING" },
	{ RETROOPTION_MOUSEINDEX "1",              OSDOPTVAL_AUTO, OPTION_STRING,         "name of mouse mapped to mouse #1" },
	{ RETROOPTION_MOUSEINDEX "2",              OSDOPTVAL_AUTO, OPTION_STRING,         "name of mouse mapped to mouse #2" },
	{ RETROOPTION_MOUSEINDEX "3",              OSDOPTVAL_AUTO, OPTION_STRING,         "name of mouse mapped to mouse #3" },
	{ RETROOPTION_MOUSEINDEX "4",              OSDOPTVAL_AUTO, OPTION_STRING,         "name of mouse mapped to mouse #4" },
	{ RETROOPTION_MOUSEINDEX "5",              OSDOPTVAL_AUTO, OPTION_STRING,         "name of mouse mapped to mouse #5" },
	{ RETROOPTION_MOUSEINDEX "6",              OSDOPTVAL_AUTO, OPTION_STRING,         "name of mouse mapped to mouse #6" },
	{ RETROOPTION_MOUSEINDEX "7",              OSDOPTVAL_AUTO, OPTION_STRING,         "name of mouse mapped to mouse #7" },
	{ RETROOPTION_MOUSEINDEX "8",              OSDOPTVAL_AUTO, OPTION_STRING,         "name of mouse mapped to mouse #8" },

	{ nullptr,                               nullptr,   OPTION_HEADER,     "SDL KEYBOARD MAPPING" },
	{ RETROOPTION_KEYBINDEX "1",               OSDOPTVAL_AUTO, OPTION_STRING,         "name of keyboard mapped to keyboard #1" },
	{ RETROOPTION_KEYBINDEX "2",               OSDOPTVAL_AUTO, OPTION_STRING,         "name of keyboard mapped to keyboard #2" },
	{ RETROOPTION_KEYBINDEX "3",               OSDOPTVAL_AUTO, OPTION_STRING,         "name of keyboard mapped to keyboard #3" },
	{ RETROOPTION_KEYBINDEX "4",               OSDOPTVAL_AUTO, OPTION_STRING,         "name of keyboard mapped to keyboard #4" },
	{ RETROOPTION_KEYBINDEX "5",               OSDOPTVAL_AUTO, OPTION_STRING,         "name of keyboard mapped to keyboard #5" },
	{ RETROOPTION_KEYBINDEX "6",               OSDOPTVAL_AUTO, OPTION_STRING,         "name of keyboard mapped to keyboard #6" },
	{ RETROOPTION_KEYBINDEX "7",               OSDOPTVAL_AUTO, OPTION_STRING,         "name of keyboard mapped to keyboard #7" },
	{ RETROOPTION_KEYBINDEX "8",               OSDOPTVAL_AUTO, OPTION_STRING,         "name of keyboard mapped to keyboard #8" },

	// SDL low level driver options
	{ nullptr,                               nullptr,   OPTION_HEADER,     "SDL LOWLEVEL DRIVER OPTIONS" },
	{ RETROOPTION_VIDEODRIVER ";vd",           OSDOPTVAL_AUTO,  OPTION_STRING,        "sdl video driver to use ('x11', 'directfb', ... or 'auto' for SDL default" },
	{ RETROOPTION_RENDERDRIVER ";rd",          OSDOPTVAL_AUTO,  OPTION_STRING,        "sdl render driver to use ('software', 'opengl', 'directfb' ... or 'auto' for SDL default" },
	{ RETROOPTION_AUDIODRIVER ";ad",           OSDOPTVAL_AUTO,  OPTION_STRING,        "sdl audio driver to use ('alsa', 'arts', ... or 'auto' for SDL default" },


	// End of list
	{ nullptr }
};

//============================================================
//  sdl_options
//============================================================

retro_options::retro_options()
: osd_options()
{
	std::string ini_path(INI_PATH);
	add_entries(retro_options::s_option_entries);
	strreplace(ini_path,"APP_NAME", emulator_info::get_appname_lower());
	set_default_value(RETROOPTION_INIPATH, ini_path.c_str());
}

//============================================================
//  main
//============================================================

retro_osd_interface *retro_global_osd;

// translated to utf8_main
int mmain(int argc, char *argv[])
{
	int res = 0;

	std::vector<std::string> args(argv, argv+argc);

	static retro_options retro_global_options;
	// disable I/O buffering
	setvbuf(stdout, (char *) nullptr, _IONBF, 0);
	setvbuf(stderr, (char *) nullptr, _IONBF, 0);
#if !defined(RETROMAME_WIN32)
	// Initialize crash diagnostics
	diagnostics_module::get_instance()->init_crash_diagnostics();
#endif
#if defined(RETROMAME_ANDROID)
	/* Enable standard application logging */
//	SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_VERBOSE);
#endif

	// FIXME: this should be done differently

#ifdef RETROMAME_UNIX
	//sdl_entered_debugger = 0;
#if (!defined(SDLMAME_MACOSX)) && (!defined(SDLMAME_HAIKU)) && (!defined(SDLMAME_EMSCRIPTEN)) && (!defined(SDLMAME_ANDROID))  && (!defined(RETROMAME))
	FcInit();
#endif
#endif

	{
		retro_global_osd= global_alloc(retro_osd_interface(retro_global_options));
		retro_global_osd->register_options();
		res =  emulator_info::start_frontend(retro_global_options, *retro_global_osd,args);
		return res;
	}

#ifdef RETROMAME_UNIX
#if (!defined(SDLMAME_MACOSX)) && (!defined(SDLMAME_HAIKU)) && (!defined(SDLMAME_EMSCRIPTEN)) && (!defined(SDLMAME_ANDROID)) && (!defined(RETROMAME))
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

retro_osd_interface::retro_osd_interface(retro_options &options)
: osd_common_t(options), m_options(options)
{
}


//============================================================
//  destructor
//============================================================

retro_osd_interface::~retro_osd_interface()
{
}


//============================================================
//  osd_exit
//============================================================

void retro_osd_interface::osd_exit()
{
	osd_common_t::osd_exit();
}

//============================================================
//  video_register
//============================================================

void retro_osd_interface::video_register()
{
	video_options_add("soft", nullptr);
}


//============================================================
//  output_oslog
//============================================================

void retro_osd_interface::output_oslog(const char *buffer)
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

void retro_osd_interface::init(running_machine &machine)
{
	// call our parent
	osd_common_t::init(machine);

#if 0
	const char *stemp;

	/* get number of processors */
	stemp = options().numprocessors();
#endif

	osd_num_processors = 0;


	osd_common_t::init_subsystems();

 	retro_switch_to_main_thread();

	if (options().oslog())
	{
		using namespace std::placeholders;
		machine.add_logerror_callback(std::bind(&retro_osd_interface::output_oslog, this, _1));
	}

}


//============================================================
//  customize_input_type_list
//============================================================
void retro_osd_interface::customize_input_type_list(std::vector<input_type_entry> &typelist)
{
	// This function is called on startup, before reading the
	// configuration from disk. Scan the list, and change the
	// default control mappings you want. It is quite possible
	// you won't need to change a thing.

	// loop over the defaults
	for (input_type_entry &entry : typelist)
		switch (entry.type())
		{
			// Retro:  Select + X => UI_CONFIGURE (Menu)
			case IPT_UI_CONFIGURE:
				entry.defseq(SEQ_TYPE_STANDARD).set(KEYCODE_TAB, input_seq::or_code, JOYCODE_SELECT, JOYCODE_BUTTON3);
				break;

			// Retro:  Select + Start => CANCEL
			case IPT_UI_CANCEL:
				entry.defseq(SEQ_TYPE_STANDARD).set(KEYCODE_ESC, input_seq::or_code, JOYCODE_SELECT, JOYCODE_START);
				break;

			// leave everything else alone
			default:
				break;
		}


}
