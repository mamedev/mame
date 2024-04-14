// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont

#include "sdlopts.h"

// emu
#include "main.h"

// lib/util
#include "util/corestr.h"

#include <SDL2/SDL.h>

#include <string>

#if defined(SDLMAME_ANDROID)
#include "unistd.h"
#endif


namespace {

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
//  Local variables
//============================================================

const options_entry f_sdl_option_entries[] =
{
	{ SDLOPTION_INIPATH,                     INI_PATH,       core_options::option_type::MULTIPATH,  "path to ini files" },

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
	{ SDLOPTION_KEYMAP_FILE,                 "keymap.dat",   core_options::option_type::PATH,       "keymap filename" },

	// joystick mapping
	{ nullptr,                               nullptr,        core_options::option_type::HEADER,     "SDL INPUT OPTIONS" },
	{ SDLOPTION_ENABLE_TOUCH,                "0",            core_options::option_type::BOOLEAN,    "enable touch input support" },
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

} // anonymous namespace


//============================================================
//  sdl_options
//============================================================

sdl_options::sdl_options() : osd_options()
{
#if defined(SDLMAME_ANDROID)
	chdir(SDL_AndroidGetExternalStoragePath()); // FIXME: why is this here of all places?
#endif
	std::string ini_path(INI_PATH);
	add_entries(f_sdl_option_entries);
	strreplace(ini_path, "APP_NAME", emulator_info::get_appname_lower());
	set_default_value(SDLOPTION_INIPATH, std::move(ini_path));
}


//============================================================
//  osd_setup_osd_specific_emu_options
//============================================================

void osd_setup_osd_specific_emu_options(emu_options &opts)
{
	opts.add_entries(osd_options::s_option_entries);
	opts.add_entries(f_sdl_option_entries);
}
