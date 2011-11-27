//============================================================
//
//  sdlmain.c - main file for SDLMAME.
//
//  Copyright (c) 1996-2012, Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================

// standard sdl header
#include <SDL/SDL.h>
#include <SDL/SDL_version.h>

#ifdef SDLMAME_UNIX
#ifndef SDLMAME_MACOSX
#include <SDL/SDL_ttf.h>
#include <fontconfig/fontconfig.h>
#endif
#ifdef SDLMAME_MACOSX
#include <Carbon/Carbon.h>
#endif
#endif

// standard includes
#if !defined(SDLMAME_WIN32) && !defined(SDLMAME_OS2)
#include <unistd.h>
#endif

#ifdef SDLMAME_OS2
#define INCL_DOS
#include <os2.h>
#endif

// MAME headers
#include "osdepend.h"
#include "emu.h"
#include "clifront.h"
#include "emuopts.h"

// OSD headers
#include "video.h"
#include "input.h"
#include "output.h"
#include "osdsdl.h"
#include "sdlos.h"
#include "netdev.h"

// we override SDL's normal startup on Win32
// please see sdlprefix.h as well

#if defined(SDLMAME_X11) && (SDL_MAJOR_VERSION == 1) && (SDL_MINOR_VERSION == 2)
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#endif

#include "watchdog.h"

//============================================================
//  OPTIONS
//============================================================

#ifndef INI_PATH
#if defined(SDLMAME_WIN32) || defined(SDLMAME_MACOSX) || defined(SDLMAME_OS2)
	#define INI_PATH ".;ini"
#else
	#define INI_PATH "$HOME/.APP_NAME;.;ini"
#endif // MACOSX
#endif // INI_PATH


//============================================================
//  Global variables
//============================================================

//============================================================
//  Local variables
//============================================================

const options_entry sdl_options::s_option_entries[] =
{
	{ SDLOPTION_INIPATH,                     INI_PATH,    OPTION_STRING,     "path to ini files" },

	// debugging options
	{ NULL,                                   NULL,       OPTION_HEADER,     "DEBUGGING OPTIONS" },
	{ SDLOPTION_OSLOG,                        "0",        OPTION_BOOLEAN,    "output error.log data to the system debugger" },
	{ SDLOPTION_WATCHDOG ";wdog",             "0",        OPTION_INTEGER,    "force the program to terminate if no updates within specified number of seconds" },

	// performance options
	{ NULL,                                   NULL,       OPTION_HEADER,     "PERFORMANCE OPTIONS" },
	{ SDLOPTION_MULTITHREADING ";mt",         "0",        OPTION_BOOLEAN,    "enable multithreading; this enables rendering and blitting on a separate thread" },
	{ SDLOPTION_NUMPROCESSORS ";np",         "auto",      OPTION_INTEGER,	 "number of processors; this overrides the number the system reports" },
	{ SDLOPTION_SDLVIDEOFPS,                  "0",        OPTION_BOOLEAN,    "show sdl video performance" },
	{ SDLOPTION_BENCH,                        "0",        OPTION_INTEGER,    "benchmark for the given number of emulated seconds; implies -video none -nosound -nothrottle" },
	// video options
	{ NULL,                                   NULL,       OPTION_HEADER,     "VIDEO OPTIONS" },
// OS X can be trusted to have working hardware OpenGL, so default to it on for the best user experience
#ifdef SDLMAME_MACOSX
	{ SDLOPTION_VIDEO,                   SDLOPTVAL_OPENGL,  OPTION_STRING,   "video output method: soft or opengl" },
#else
	{ SDLOPTION_VIDEO,                   SDLOPTVAL_SOFT,  OPTION_STRING,     "video output method: soft or opengl" },
#endif
	{ SDLOPTION_NUMSCREENS,                   "1",        OPTION_INTEGER,    "number of screens to create; SDLMAME only supports 1 at this time" },
	{ SDLOPTION_WINDOW ";w",                  "0",        OPTION_BOOLEAN,    "enable window mode; otherwise, full screen mode is assumed" },
	{ SDLOPTION_MAXIMIZE ";max",              "1",        OPTION_BOOLEAN,    "default to maximized windows; otherwise, windows will be minimized" },
	{ SDLOPTION_KEEPASPECT ";ka",             "1",        OPTION_BOOLEAN,    "constrain to the proper aspect ratio" },
	{ SDLOPTION_UNEVENSTRETCH ";ues",         "1",        OPTION_BOOLEAN,    "allow non-integer stretch factors" },
	{ SDLOPTION_CENTERH,                      "1",        OPTION_BOOLEAN,    "center horizontally within the view area" },
	{ SDLOPTION_CENTERV,                      "1",        OPTION_BOOLEAN,    "center vertically within the view area" },
	#if (SDL_VERSION_ATLEAST(1,2,10))
	{ SDLOPTION_WAITVSYNC,                    "0",        OPTION_BOOLEAN,    "enable waiting for the start of VBLANK before flipping screens; reduces tearing effects" },
	{ SDLOPTION_SYNCREFRESH,                  "0",        OPTION_BOOLEAN,    "enable using the start of VBLANK for throttling instead of the game time" },
	#endif
#if (SDL_VERSION_ATLEAST(1,3,0))
	{ SDLOPTION_SCALEMODE ";sm",         SDLOPTVAL_NONE,  OPTION_STRING,     "Scale mode: none, hwblit, hwbest, yv12, yuy2, yv12x2, yuy2x2 (-video soft only)" },
#else
	{ SDLOPTION_SCALEMODE ";sm",         SDLOPTVAL_NONE,  OPTION_STRING,     "Scale mode: none, async, yv12, yuy2, yv12x2, yuy2x2 (-video soft only)" },
#endif
#if USE_OPENGL
	// OpenGL specific options
	{ NULL,                                   NULL,   OPTION_HEADER,  "OpenGL-SPECIFIC OPTIONS" },
	{ SDLOPTION_FILTER ";glfilter;flt",       "1",    OPTION_BOOLEAN, "enable bilinear filtering on screen output" },
	{ SDLOPTION_PRESCALE,                     "1",    OPTION_INTEGER,                 "scale screen rendering by this amount in software" },
	{ SDLOPTION_GL_FORCEPOW2TEXTURE,          "0",    OPTION_BOOLEAN, "force power of two textures  (default no)" },
	{ SDLOPTION_GL_NOTEXTURERECT,             "0",    OPTION_BOOLEAN, "don't use OpenGL GL_ARB_texture_rectangle (default on)" },
	{ SDLOPTION_GL_VBO,                       "1",    OPTION_BOOLEAN, "enable OpenGL VBO,  if available (default on)" },
	{ SDLOPTION_GL_PBO,                       "1",    OPTION_BOOLEAN, "enable OpenGL PBO,  if available (default on)" },
	{ SDLOPTION_GL_GLSL,                      "0",    OPTION_BOOLEAN, "enable OpenGL GLSL, if available (default off)" },
	{ SDLOPTION_GLSL_FILTER,				  "1",    OPTION_STRING,  "enable OpenGL GLSL filtering instead of FF filtering 0-plain, 1-bilinear (default)" },
	{ SDLOPTION_SHADER_MAME "0",     SDLOPTVAL_NONE,  OPTION_STRING,  "custom OpenGL GLSL shader set mame bitmap 0" },
	{ SDLOPTION_SHADER_MAME "1",     SDLOPTVAL_NONE,  OPTION_STRING,  "custom OpenGL GLSL shader set mame bitmap 1" },
	{ SDLOPTION_SHADER_MAME "2",     SDLOPTVAL_NONE,  OPTION_STRING,  "custom OpenGL GLSL shader set mame bitmap 2" },
	{ SDLOPTION_SHADER_MAME "3",     SDLOPTVAL_NONE,  OPTION_STRING,  "custom OpenGL GLSL shader set mame bitmap 3" },
	{ SDLOPTION_SHADER_MAME "4",     SDLOPTVAL_NONE,  OPTION_STRING,  "custom OpenGL GLSL shader set mame bitmap 4" },
	{ SDLOPTION_SHADER_MAME "5",     SDLOPTVAL_NONE,  OPTION_STRING,  "custom OpenGL GLSL shader set mame bitmap 5" },
	{ SDLOPTION_SHADER_MAME "6",     SDLOPTVAL_NONE,  OPTION_STRING,  "custom OpenGL GLSL shader set mame bitmap 6" },
	{ SDLOPTION_SHADER_MAME "7",     SDLOPTVAL_NONE,  OPTION_STRING,  "custom OpenGL GLSL shader set mame bitmap 7" },
	{ SDLOPTION_SHADER_MAME "8",     SDLOPTVAL_NONE,  OPTION_STRING,  "custom OpenGL GLSL shader set mame bitmap 8" },
	{ SDLOPTION_SHADER_MAME "9",     SDLOPTVAL_NONE,  OPTION_STRING,  "custom OpenGL GLSL shader set mame bitmap 9" },
	{ SDLOPTION_SHADER_SCREEN "0",   SDLOPTVAL_NONE,  OPTION_STRING,  "custom OpenGL GLSL shader screen bitmap 0" },
	{ SDLOPTION_SHADER_SCREEN "1",   SDLOPTVAL_NONE,  OPTION_STRING,  "custom OpenGL GLSL shader screen bitmap 1" },
	{ SDLOPTION_SHADER_SCREEN "2",   SDLOPTVAL_NONE,  OPTION_STRING,  "custom OpenGL GLSL shader screen bitmap 2" },
	{ SDLOPTION_SHADER_SCREEN "3",   SDLOPTVAL_NONE,  OPTION_STRING,  "custom OpenGL GLSL shader screen bitmap 3" },
	{ SDLOPTION_SHADER_SCREEN "4",   SDLOPTVAL_NONE,  OPTION_STRING,  "custom OpenGL GLSL shader screen bitmap 4" },
	{ SDLOPTION_SHADER_SCREEN "5",   SDLOPTVAL_NONE,  OPTION_STRING,  "custom OpenGL GLSL shader screen bitmap 5" },
	{ SDLOPTION_SHADER_SCREEN "6",   SDLOPTVAL_NONE,  OPTION_STRING,  "custom OpenGL GLSL shader screen bitmap 6" },
	{ SDLOPTION_SHADER_SCREEN "7",   SDLOPTVAL_NONE,  OPTION_STRING,  "custom OpenGL GLSL shader screen bitmap 7" },
	{ SDLOPTION_SHADER_SCREEN "8",   SDLOPTVAL_NONE,  OPTION_STRING,  "custom OpenGL GLSL shader screen bitmap 8" },
	{ SDLOPTION_SHADER_SCREEN "9",   SDLOPTVAL_NONE,  OPTION_STRING,  "custom OpenGL GLSL shader screen bitmap 9" },
	{ SDLOPTION_GL_GLSL_VID_ATTR,			 "1",    OPTION_BOOLEAN,  "enable OpenGL GLSL handling of brightness and contrast. Better RGB game performance for free. (default)" },
#endif

	// per-window options
	{ NULL,                                   NULL,             OPTION_HEADER,    "PER-WINDOW VIDEO OPTIONS" },
	{ SDLOPTION_SCREEN,                   SDLOPTVAL_AUTO,   OPTION_STRING,    "explicit name of the first screen; 'auto' here will try to make a best guess" },
	{ SDLOPTION_ASPECT ";screen_aspect",  SDLOPTVAL_AUTO,   OPTION_STRING,    "aspect ratio for all screens; 'auto' here will try to make a best guess" },
	{ SDLOPTION_RESOLUTION ";r",          SDLOPTVAL_AUTO,   OPTION_STRING,    "preferred resolution for all screens; format is <width>x<height>[@<refreshrate>] or 'auto'" },
	{ SDLOPTION_VIEW,                     SDLOPTVAL_AUTO,   OPTION_STRING,    "preferred view for all screens" },

	{ SDLOPTION_SCREEN "0",                  SDLOPTVAL_AUTO,   OPTION_STRING,    "explicit name of the first screen; 'auto' here will try to make a best guess" },
	{ SDLOPTION_ASPECT "0",                  SDLOPTVAL_AUTO,   OPTION_STRING,    "aspect ratio of the first screen; 'auto' here will try to make a best guess" },
	{ SDLOPTION_RESOLUTION "0;r0",        SDLOPTVAL_AUTO,   OPTION_STRING,    "preferred resolution of the first screen; format is <width>x<height>[@<refreshrate>] or 'auto'" },
	{ SDLOPTION_VIEW "0",                    SDLOPTVAL_AUTO,   OPTION_STRING,    "preferred view for the first screen" },

	{ SDLOPTION_SCREEN "1",                  SDLOPTVAL_AUTO,   OPTION_STRING,    "explicit name of the second screen; 'auto' here will try to make a best guess" },
	{ SDLOPTION_ASPECT "1",                  SDLOPTVAL_AUTO,   OPTION_STRING,    "aspect ratio of the second screen; 'auto' here will try to make a best guess" },
	{ SDLOPTION_RESOLUTION "1;r1",        SDLOPTVAL_AUTO,   OPTION_STRING,    "preferred resolution of the second screen; format is <width>x<height>[@<refreshrate>] or 'auto'" },
	{ SDLOPTION_VIEW "1",                    SDLOPTVAL_AUTO,   OPTION_STRING,    "preferred view for the second screen" },

	{ SDLOPTION_SCREEN "2",                  SDLOPTVAL_AUTO,   OPTION_STRING,    "explicit name of the third screen; 'auto' here will try to make a best guess" },
	{ SDLOPTION_ASPECT "2",                  SDLOPTVAL_AUTO,   OPTION_STRING,    "aspect ratio of the third screen; 'auto' here will try to make a best guess" },
	{ SDLOPTION_RESOLUTION "2;r2",        SDLOPTVAL_AUTO,   OPTION_STRING,    "preferred resolution of the third screen; format is <width>x<height>[@<refreshrate>] or 'auto'" },
	{ SDLOPTION_VIEW "2",                    SDLOPTVAL_AUTO,   OPTION_STRING,    "preferred view for the third screen" },

	{ SDLOPTION_SCREEN "3",                  SDLOPTVAL_AUTO,   OPTION_STRING,    "explicit name of the fourth screen; 'auto' here will try to make a best guess" },
	{ SDLOPTION_ASPECT "3",                  SDLOPTVAL_AUTO,   OPTION_STRING,    "aspect ratio of the fourth screen; 'auto' here will try to make a best guess" },
	{ SDLOPTION_RESOLUTION "3;r3",        SDLOPTVAL_AUTO,   OPTION_STRING,    "preferred resolution of the fourth screen; format is <width>x<height>[@<refreshrate>] or 'auto'" },
	{ SDLOPTION_VIEW "3",                    SDLOPTVAL_AUTO,   OPTION_STRING,    "preferred view for the fourth screen" },

	// full screen options
	{ NULL,                                   NULL,  OPTION_HEADER,     "FULL SCREEN OPTIONS" },
	{ SDLOPTION_SWITCHRES,                    "0",   OPTION_BOOLEAN,    "enable resolution switching" },
	#ifdef SDLMAME_X11
	{ SDLOPTION_USEALLHEADS,	             "0",	  OPTION_BOOLEAN,    "split full screen image across monitors" },
	#endif

	// sound options
	{ NULL,                                   NULL,  OPTION_HEADER,     "SOUND OPTIONS" },
	{ SDLOPTION_AUDIO_LATENCY,                "2",   OPTION_INTEGER,    "set audio latency (increase to reduce glitches, decrease for responsiveness)" },

	// keyboard mapping
	{ NULL, 		                          NULL,  OPTION_HEADER,     "SDL KEYBOARD MAPPING" },
	{ SDLOPTION_KEYMAP,                      "0",    OPTION_BOOLEAN,    "enable keymap" },
	{ SDLOPTION_KEYMAP_FILE,                 "keymap.dat", OPTION_STRING, "keymap filename" },
#ifdef SDLMAME_MACOSX
	{ SDLOPTION_UIMODEKEY,					 "DELETE", OPTION_STRING,   "Key to toggle keyboard mode" },
#else
	{ SDLOPTION_UIMODEKEY,			         "SCRLOCK", OPTION_STRING,  "Key to toggle keyboard mode" },
#endif	// SDLMAME_MACOSX

	// joystick mapping
	{ NULL, 		                         NULL,   OPTION_HEADER,     "SDL JOYSTICK MAPPING" },
	{ SDLOPTION_JOYINDEX "1",                SDLOPTVAL_AUTO, OPTION_STRING,         "name of joystick mapped to joystick #1" },
	{ SDLOPTION_JOYINDEX "2",                SDLOPTVAL_AUTO, OPTION_STRING,         "name of joystick mapped to joystick #2" },
	{ SDLOPTION_JOYINDEX "3",                SDLOPTVAL_AUTO, OPTION_STRING,         "name of joystick mapped to joystick #3" },
	{ SDLOPTION_JOYINDEX "4",                SDLOPTVAL_AUTO, OPTION_STRING,         "name of joystick mapped to joystick #4" },
	{ SDLOPTION_JOYINDEX "5",                SDLOPTVAL_AUTO, OPTION_STRING,         "name of joystick mapped to joystick #5" },
	{ SDLOPTION_JOYINDEX "6",                SDLOPTVAL_AUTO, OPTION_STRING,         "name of joystick mapped to joystick #6" },
	{ SDLOPTION_JOYINDEX "7",                SDLOPTVAL_AUTO, OPTION_STRING,         "name of joystick mapped to joystick #7" },
	{ SDLOPTION_JOYINDEX "8",                SDLOPTVAL_AUTO, OPTION_STRING,         "name of joystick mapped to joystick #8" },
	{ SDLOPTION_SIXAXIS,			         "0",	 OPTION_BOOLEAN,    "Use special handling for PS3 Sixaxis controllers" },

#if (SDL_VERSION_ATLEAST(1,3,0))
	{ NULL, 		                         NULL,   OPTION_HEADER,     "SDL MOUSE MAPPING" },
	{ SDLOPTION_MOUSEINDEX "1",              SDLOPTVAL_AUTO, OPTION_STRING,         "name of mouse mapped to mouse #1" },
	{ SDLOPTION_MOUSEINDEX "2",              SDLOPTVAL_AUTO, OPTION_STRING,         "name of mouse mapped to mouse #2" },
	{ SDLOPTION_MOUSEINDEX "3",              SDLOPTVAL_AUTO, OPTION_STRING,         "name of mouse mapped to mouse #3" },
	{ SDLOPTION_MOUSEINDEX "4",              SDLOPTVAL_AUTO, OPTION_STRING,         "name of mouse mapped to mouse #4" },
	{ SDLOPTION_MOUSEINDEX "5",              SDLOPTVAL_AUTO, OPTION_STRING,         "name of mouse mapped to mouse #5" },
	{ SDLOPTION_MOUSEINDEX "6",              SDLOPTVAL_AUTO, OPTION_STRING,         "name of mouse mapped to mouse #6" },
	{ SDLOPTION_MOUSEINDEX "7",              SDLOPTVAL_AUTO, OPTION_STRING,         "name of mouse mapped to mouse #7" },
	{ SDLOPTION_MOUSEINDEX "8",              SDLOPTVAL_AUTO, OPTION_STRING,         "name of mouse mapped to mouse #8" },

	{ NULL, 		                         NULL,   OPTION_HEADER,     "SDL KEYBOARD MAPPING" },
	{ SDLOPTION_KEYBINDEX "1",               SDLOPTVAL_AUTO, OPTION_STRING,         "name of keyboard mapped to keyboard #1" },
	{ SDLOPTION_KEYBINDEX "2",               SDLOPTVAL_AUTO, OPTION_STRING,         "name of keyboard mapped to keyboard #2" },
	{ SDLOPTION_KEYBINDEX "3",               SDLOPTVAL_AUTO, OPTION_STRING,         "name of keyboard mapped to keyboard #3" },
	{ SDLOPTION_KEYBINDEX "4",               SDLOPTVAL_AUTO, OPTION_STRING,         "name of keyboard mapped to keyboard #4" },
	{ SDLOPTION_KEYBINDEX "5",               SDLOPTVAL_AUTO, OPTION_STRING,         "name of keyboard mapped to keyboard #5" },
	{ SDLOPTION_KEYBINDEX "6",               SDLOPTVAL_AUTO, OPTION_STRING,         "name of keyboard mapped to keyboard #6" },
	{ SDLOPTION_KEYBINDEX "7",               SDLOPTVAL_AUTO, OPTION_STRING,         "name of keyboard mapped to keyboard #7" },
	{ SDLOPTION_KEYBINDEX "8",               SDLOPTVAL_AUTO, OPTION_STRING,         "name of keyboard mapped to keyboard #8" },
#endif
	// SDL low level driver options
	{ NULL, 		                         NULL,   OPTION_HEADER,     "SDL LOWLEVEL DRIVER OPTIONS" },
	{ SDLOPTION_VIDEODRIVER ";vd",           SDLOPTVAL_AUTO,  OPTION_STRING,        "sdl video driver to use ('x11', 'directfb', ... or 'auto' for SDL default" },
#if (SDL_VERSION_ATLEAST(1,3,0))
	{ SDLOPTION_RENDERDRIVER ";rd",          SDLOPTVAL_AUTO,  OPTION_STRING,        "sdl render driver to use ('software', 'opengl', 'directfb' ... or 'auto' for SDL default" },
#endif
	{ SDLOPTION_AUDIODRIVER ";ad",           SDLOPTVAL_AUTO,  OPTION_STRING,        "sdl audio driver to use ('alsa', 'arts', ... or 'auto' for SDL default" },
#if USE_OPENGL
	{ SDLOPTION_GL_LIB,                      SDLOPTVAL_GLLIB, OPTION_STRING,        "alternative libGL.so to use; 'auto' for system default" },
#endif

	// End of list
	{ NULL }
};

//============================================================
//  OS2 specific
//============================================================

#ifdef SDLMAME_OS2
void MorphToPM()
{
  PPIB pib;
  PTIB tib;

  DosGetInfoBlocks(&tib, &pib);

  // Change flag from VIO to PM:
  if (pib->pib_ultype==2) pib->pib_ultype = 3;
}
#endif

//============================================================
//  sdl_options
//============================================================

sdl_options::sdl_options()
{
	astring ini_path(INI_PATH);
	add_entries(s_option_entries);
	ini_path.replace(0, "APP_NAME", emulator_info::get_appname_lower());
	set_default_value(SDLOPTION_INIPATH, ini_path.cstr());
}


//============================================================
//  main
//============================================================

// we do some special sauce on Win32...

#if !defined(SDLMAME_WIN32)
int main(int argc, char **argv)
{
	int res = 0;

#else

/* gee */
extern "C" DECLSPEC void SDLCALL SDL_SetModuleHandle(void *hInst);

// translated to utf8_main
int main(int argc, char *argv[])
{
	int res = 0;

#if	!(SDL_VERSION_ATLEAST(1,3,0))
	/* Load SDL dynamic link library */
	if ( SDL_Init(SDL_INIT_NOPARACHUTE) < 0 ) {
		fprintf(stderr, "WinMain() error: %s", SDL_GetError());
		return(FALSE);
	}
	SDL_SetModuleHandle(GetModuleHandle(NULL));
#endif
#endif
	// disable I/O buffering
	setvbuf(stdout, (char *) NULL, _IONBF, 0);
	setvbuf(stderr, (char *) NULL, _IONBF, 0);

	#ifdef SDLMAME_UNIX
	#ifndef SDLMAME_MACOSX
	if (TTF_Init() == -1)
	{
		printf("SDL_ttf failed: %s\n", TTF_GetError());
	}
	FcInit();
	#endif
	#endif

	#ifdef SDLMAME_OS2
	MorphToPM();
	#endif

#if defined(SDLMAME_X11) && (SDL_MAJOR_VERSION == 1) && (SDL_MINOR_VERSION == 2)
	if (SDL_Linked_Version()->patch < 10)
	/* workaround for SDL choosing a 32-bit ARGB visual */
	{
		Display *display;
		if ((display = XOpenDisplay(NULL)) && (DefaultDepth(display, DefaultScreen(display)) >= 24))
		{
			XVisualInfo vi;
			char buf[130];
			if (XMatchVisualInfo(display, DefaultScreen(display), 24, TrueColor, &vi)) {
				snprintf(buf, sizeof(buf), "0x%lx", vi.visualid);
				osd_setenv(SDLENV_VISUALID, buf, 0);
			}
		}
		if (display)
			XCloseDisplay(display);
	}
#endif

	{
		sdl_osd_interface osd;
		sdl_options options;
		cli_frontend frontend(options, osd);
		res = frontend.execute(argc, argv);
	}

#ifdef MALLOC_DEBUG
	{
		void check_unfreed_mem(void);
		check_unfreed_mem();
	}
#endif

	// already called...
	//SDL_Quit();

	#ifdef SDLMAME_UNIX
	#ifndef SDLMAME_MACOSX
	TTF_Quit();
	FcFini();
	#endif
	#endif

	exit(res);

	return res;
}



//============================================================
//  output_oslog
//============================================================

static void output_oslog(running_machine &machine, const char *buffer)
{
	fputs(buffer, stderr);
}



//============================================================
//  constructor
//============================================================

sdl_osd_interface::sdl_osd_interface()
{
	m_watchdog = NULL;
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

void sdl_osd_interface::osd_exit(running_machine &machine)
{
	#ifdef SDLMAME_NETWORK
		sdlnetdev_deinit(machine);
	#endif

	if (!SDLMAME_INIT_IN_WORKER_THREAD)
		SDL_Quit();
}

//============================================================
//  defines_verbose
//============================================================

#define MAC_EXPAND_STR(_m) #_m
#define MACRO_VERBOSE(_mac) \
	do { \
		if (strcmp(MAC_EXPAND_STR(_mac), #_mac) != 0) \
			mame_printf_verbose("%s=%s ", #_mac, MAC_EXPAND_STR(_mac)); \
	} while (0)

#define _SDL_VER #SDL_MAJOR_VERSION "." #SDL_MINOR_VERSION "." #SDL_PATCHLEVEL

static void defines_verbose(void)
{
	mame_printf_verbose("Build version:      %s\n", build_version);
	mame_printf_verbose("Build architecure:  ");
	MACRO_VERBOSE(SDLMAME_ARCH);
	mame_printf_verbose("\n");
	mame_printf_verbose("Build defines 1:    ");
	MACRO_VERBOSE(SDLMAME_UNIX);
	MACRO_VERBOSE(SDLMAME_X11);
	MACRO_VERBOSE(SDLMAME_WIN32);
	MACRO_VERBOSE(SDLMAME_OS2);
	MACRO_VERBOSE(SDLMAME_MACOSX);
	MACRO_VERBOSE(SDLMAME_DARWIN);
	MACRO_VERBOSE(SDLMAME_LINUX);
	MACRO_VERBOSE(SDLMAME_SOLARIS);
	MACRO_VERBOSE(SDLMAME_NOASM);
	MACRO_VERBOSE(SDLMAME_IRIX);
	MACRO_VERBOSE(SDLMAME_BSD);
	mame_printf_verbose("\n");
	mame_printf_verbose("Build defines 1:    ");
	MACRO_VERBOSE(LSB_FIRST);
	MACRO_VERBOSE(PTR64);
	MACRO_VERBOSE(MAME_DEBUG);
	MACRO_VERBOSE(NO_DEBUGBER);
	MACRO_VERBOSE(BIGENDIAN);
	MACRO_VERBOSE(CPP_COMPILE);
	MACRO_VERBOSE(DISTRO);
	MACRO_VERBOSE(SYNC_IMPLEMENTATION);
	mame_printf_verbose("\n");
	mame_printf_verbose("SDL/OpenGL defines: ");
	mame_printf_verbose("SDL_COMPILEDVERSION=%d ", SDL_COMPILEDVERSION);
	MACRO_VERBOSE(USE_OPENGL);
	MACRO_VERBOSE(USE_DISPATCH_GL);
	mame_printf_verbose("\n");
	mame_printf_verbose("Compiler defines A: ");
	MACRO_VERBOSE(__GNUC__);
	MACRO_VERBOSE(__GNUC_MINOR__);
	MACRO_VERBOSE(__GNUC_PATCHLEVEL__);
	MACRO_VERBOSE(__VERSION__);
	mame_printf_verbose("\n");
	mame_printf_verbose("Compiler defines B: ");
	MACRO_VERBOSE(__amd64__);
	MACRO_VERBOSE(__x86_64__);
	MACRO_VERBOSE(__unix__);
	MACRO_VERBOSE(__i386__);
	MACRO_VERBOSE(__ppc__);
	MACRO_VERBOSE(__ppc64__);
	mame_printf_verbose("\n");
	mame_printf_verbose("Compiler defines C: ");
	MACRO_VERBOSE(_FORTIFY_SOURCE);
	MACRO_VERBOSE(__USE_FORTIFY_LEVEL);
	mame_printf_verbose("\n");
}

//============================================================
//  osd_sdl_info
//============================================================

static void osd_sdl_info(void)
{
#if SDL_VERSION_ATLEAST(1,3,0)
	int i, num = SDL_GetNumVideoDrivers();

	mame_printf_verbose("Available videodrivers: ");
	for (i=0;i<num;i++)
	{
		const char *name = SDL_GetVideoDriver(i);
		mame_printf_verbose("%s ", name);
	}
	mame_printf_verbose("\n");
	mame_printf_verbose("Current Videodriver: %s\n", SDL_GetCurrentVideoDriver());
	num = SDL_GetNumVideoDisplays();
	for (i=0;i<num;i++)
	{
		SDL_DisplayMode mode;
		int j;

		mame_printf_verbose("\tDisplay #%d\n", i);
		if (SDL_GetDesktopDisplayMode(i, &mode));
			mame_printf_verbose("\t\tDesktop Mode:         %dx%d-%d@%d\n", mode.w, mode.h, SDL_BITSPERPIXEL(mode.format), mode.refresh_rate);
		if (SDL_GetCurrentDisplayMode(i, &mode));
			mame_printf_verbose("\t\tCurrent Display Mode: %dx%d-%d@%d\n", mode.w, mode.h, SDL_BITSPERPIXEL(mode.format), mode.refresh_rate);
		mame_printf_verbose("\t\tRenderdrivers:\n");
		for (j=0; j<SDL_GetNumRenderDrivers(); j++)
		{
			SDL_RendererInfo info;
			SDL_GetRenderDriverInfo(j, &info);
			mame_printf_verbose("\t\t\t%10s (%dx%d)\n", info.name, info.max_texture_width, info.max_texture_height);
		}
	}

	mame_printf_verbose("Available audio drivers: \n");
	num = SDL_GetNumAudioDrivers();
	for (i=0;i<num;i++)
	{
		mame_printf_verbose("\t%-20s\n", SDL_GetAudioDriver(i));
	}
#endif
}


//============================================================
//  init
//============================================================

void sdl_osd_interface::init(running_machine &machine)
{
	// call our parent
	osd_interface::init(machine);

	sdl_options &options = downcast<sdl_options &>(machine.options());
	const char *stemp;

	// determine if we are benchmarking, and adjust options appropriately
	int bench = options.bench();
	astring error_string;
	if (bench > 0)
	{
		options.set_value(OPTION_THROTTLE, false, OPTION_PRIORITY_MAXIMUM, error_string);
		options.set_value(OPTION_SOUND, false, OPTION_PRIORITY_MAXIMUM, error_string);
		options.set_value(SDLOPTION_VIDEO, "none", OPTION_PRIORITY_MAXIMUM, error_string);
		options.set_value(OPTION_SECONDS_TO_RUN, bench, OPTION_PRIORITY_MAXIMUM, error_string);
		assert(!error_string);
	}

	// Some driver options - must be before audio init!
	stemp = options.audio_driver();
	if (stemp != NULL && strcmp(stemp, SDLOPTVAL_AUTO) != 0)
	{
		mame_printf_verbose("Setting SDL audiodriver '%s' ...\n", stemp);
		osd_setenv(SDLENV_AUDIODRIVER, stemp, 1);
	}

	stemp = options.video_driver();
	if (stemp != NULL && strcmp(stemp, SDLOPTVAL_AUTO) != 0)
	{
		mame_printf_verbose("Setting SDL videodriver '%s' ...\n", stemp);
		osd_setenv(SDLENV_VIDEODRIVER, stemp, 1);
	}

	if (SDL_VERSION_ATLEAST(1,3,0))
	{
		stemp = options.render_driver();
		if (stemp != NULL && strcmp(stemp, SDLOPTVAL_AUTO) != 0)
		{
			mame_printf_verbose("Setting SDL renderdriver '%s' ...\n", stemp);
			osd_setenv(SDLENV_RENDERDRIVER, stemp, 1);
		}
	}

	/* Set the SDL environment variable for drivers wanting to load the
     * lib at startup.
     */
#if USE_OPENGL
	/* FIXME: move lib loading code from drawogl.c here */

	stemp = options.gl_lib();
	if (stemp != NULL && strcmp(stemp, SDLOPTVAL_AUTO) != 0)
	{
		osd_setenv("SDL_VIDEO_GL_DRIVER", stemp, 1);
		mame_printf_verbose("Setting SDL_VIDEO_GL_DRIVER = '%s' ...\n", stemp);
	}
#endif

	/* get number of processors */
	stemp = options.numprocessors();

	sdl_num_processors = 0;

	if (strcmp(stemp, "auto") != 0)
	{
		sdl_num_processors = atoi(stemp);
		if (sdl_num_processors < 1)
		{
			mame_printf_warning("Warning: numprocessors < 1 doesn't make much sense. Assuming auto ...\n");
			sdl_num_processors = 0;
		}
	}

	/* Initialize SDL */

	if (!SDLMAME_INIT_IN_WORKER_THREAD)
	{
#if (SDL_VERSION_ATLEAST(1,3,0))
		if (SDL_InitSubSystem(SDL_INIT_TIMER|SDL_INIT_AUDIO| SDL_INIT_VIDEO| SDL_INIT_JOYSTICK|SDL_INIT_NOPARACHUTE)) {
#else
		if (SDL_Init(SDL_INIT_TIMER|SDL_INIT_AUDIO| SDL_INIT_VIDEO| SDL_INIT_JOYSTICK|SDL_INIT_NOPARACHUTE)) {
#endif
			mame_printf_error("Could not initialize SDL %s\n", SDL_GetError());
			exit(-1);
		}
		osd_sdl_info();
	}
	// must be before sdlvideo_init!
	machine.add_notifier(MACHINE_NOTIFY_EXIT, machine_notify_delegate(FUNC(osd_exit), &machine));

	defines_verbose();

	if (!SDLMAME_HAS_DEBUGGER)
		if (machine.debug_flags & DEBUG_FLAG_OSD_ENABLED)
		{
			mame_printf_error("sdlmame: -debug not supported on X11-less builds\n\n");
			osd_exit(machine);
			exit(-1);
		}

	if (sdlvideo_init(machine))
	{
		osd_exit(machine);
		mame_printf_error("sdlvideo_init: Initialization failed!\n\n\n");
		fflush(stderr);
		fflush(stdout);
		exit(-1);
	}

	sdlinput_init(machine);
	sdlaudio_init(machine);
	sdloutput_init(machine);

#ifdef SDLMAME_NETWORK
	sdlnetdev_init(machine);
#endif

	if (options.oslog())
		machine.add_logerror_callback(output_oslog);

	/* now setup watchdog */

	int watchdog_timeout = options.watchdog();
	int str = options.seconds_to_run();

	/* only enable watchdog if seconds_to_run is enabled *and* relatively short (time taken from ui.c) */
	if ((watchdog_timeout != 0) && (str > 0) && (str < 60*5 ))
	{
		m_watchdog = auto_alloc(machine, watchdog);
		m_watchdog->setTimeout(watchdog_timeout);
	}

#if (SDL_VERSION_ATLEAST(1,3,0))
	SDL_EventState(SDL_TEXTINPUT, SDL_TRUE);
#else
	SDL_EnableUNICODE(SDL_TRUE);
#endif
}

#ifdef SDLMAME_UNIX
#define POINT_SIZE 144.0

#ifdef SDLMAME_MACOSX

#define EXTRA_HEIGHT 1.0
#define EXTRA_WIDTH 1.15

//-------------------------------------------------
//  font_open - attempt to "open" a handle to the
//  font with the given name
//-------------------------------------------------

osd_font sdl_osd_interface::font_open(const char *_name, int &height)
{
	CFStringRef font_name = NULL;
	CTFontRef ct_font = NULL;
	CTFontDescriptorRef font_descriptor;
	CGAffineTransform affine_transform = CGAffineTransformIdentity;

	astring name(_name);

	if (name == "default")
	{
		name = "LucidaGrande";
	}

	/* handle bdf fonts in the core */
	if (name.len() > 4)
		if (name.toupper().substr(name.len()-4,4) == ".BDF" )
			return NULL;

	font_name = CFStringCreateWithCString( NULL, _name, kCFStringEncodingUTF8 );

	if( font_name != NULL )
	{
      font_descriptor = CTFontDescriptorCreateWithNameAndSize( font_name, POINT_SIZE );

      if( font_descriptor != NULL )
      {
         ct_font = CTFontCreateWithFontDescriptor( font_descriptor, POINT_SIZE, &affine_transform );

         CFRelease( font_descriptor );
      }
   }

   CFRelease( font_name );

   if (!ct_font)
	{
		printf("WARNING: Couldn't find/open font %s, using MAME default\n", name.cstr());
		return NULL;
	}

   CFStringRef real_name = CTFontCopyPostScriptName( ct_font );
   char real_name_c_string[255];
   CFStringGetCString ( real_name, real_name_c_string, 255, kCFStringEncodingUTF8 );
   mame_printf_verbose("Matching font: %s\n", real_name_c_string);
   CFRelease( real_name );

   CGFloat line_height = 0.0;
   line_height += CTFontGetAscent(ct_font);
   line_height += CTFontGetDescent(ct_font);
   line_height += CTFontGetLeading(ct_font);
   height = ceilf(line_height * EXTRA_HEIGHT);

   return (osd_font)ct_font;
}

//-------------------------------------------------
//  font_close - release resources associated with
//  a given OSD font
//-------------------------------------------------

void sdl_osd_interface::font_close(osd_font font)
{
   CTFontRef ct_font = (CTFontRef)font;

   if( ct_font != NULL )
   {
      CFRelease( ct_font );
   }
}

//-------------------------------------------------
//  font_get_bitmap - allocate and populate a
//  BITMAP_FORMAT_ARGB32 bitmap containing the
//  pixel values MAKE_ARGB(0xff,0xff,0xff,0xff)
//  or MAKE_ARGB(0x00,0xff,0xff,0xff) for each
//  pixel of a black & white font
//-------------------------------------------------

bitmap_t *sdl_osd_interface::font_get_bitmap(osd_font font, unicode_char chnum, INT32 &width, INT32 &xoffs, INT32 &yoffs)
{
   UniChar uni_char;
   CGGlyph glyph;
   bitmap_t *bitmap = (bitmap_t *)NULL;
   CTFontRef ct_font = (CTFontRef)font;
   const CFIndex count = 1;
   CGRect bounding_rect, success_rect;
   CGContextRef context_ref;

   if( chnum == ' ' )
   {
      uni_char = 'n';
      CTFontGetGlyphsForCharacters( ct_font, &uni_char, &glyph, count );
      success_rect = CTFontGetBoundingRectsForGlyphs( ct_font, kCTFontDefaultOrientation, &glyph, &bounding_rect, count );
      uni_char = chnum;
      CTFontGetGlyphsForCharacters( ct_font, &uni_char, &glyph, count );
   }
   else
   {
      uni_char = chnum;
      CTFontGetGlyphsForCharacters( ct_font, &uni_char, &glyph, count );
      success_rect = CTFontGetBoundingRectsForGlyphs( ct_font, kCTFontDefaultOrientation, &glyph, &bounding_rect, count );
   }

   if( CGRectEqualToRect( success_rect, CGRectNull ) == false )
   {
      size_t bitmap_width;
      size_t bitmap_height;

      bitmap_width = ceilf(bounding_rect.size.width * EXTRA_WIDTH);
      bitmap_width = bitmap_width == 0 ? 1 : bitmap_width;

      bitmap_height = ceilf( (CTFontGetAscent(ct_font) + CTFontGetDescent(ct_font) + CTFontGetLeading(ct_font)) * EXTRA_HEIGHT);

      xoffs = yoffs = 0;
      width = bitmap_width;

      size_t bits_per_component;
      CGColorSpaceRef color_space;
      CGBitmapInfo bitmap_info = kCGBitmapByteOrder32Host | kCGImageAlphaPremultipliedFirst;

      color_space = CGColorSpaceCreateDeviceRGB();
      bits_per_component = 8;

      bitmap = auto_alloc(machine(), bitmap_t(bitmap_width, bitmap_height, BITMAP_FORMAT_ARGB32));

      context_ref = CGBitmapContextCreate( bitmap->base, bitmap_width, bitmap_height, bits_per_component, bitmap->rowpixels*4, color_space, bitmap_info );

      if( context_ref != NULL )
      {
         CGFontRef font_ref;
         font_ref = CTFontCopyGraphicsFont( ct_font, NULL );
         CGContextSetTextPosition(context_ref, -bounding_rect.origin.x*EXTRA_WIDTH, CTFontGetDescent(ct_font)+CTFontGetLeading(ct_font) );
         CGContextSetRGBFillColor(context_ref, 1.0, 1.0, 1.0, 1.0);
         CGContextSetFont( context_ref, font_ref );
         CGContextSetFontSize( context_ref, POINT_SIZE );
         CGContextShowGlyphs( context_ref, &glyph, count );
         CGFontRelease( font_ref );
         CGContextRelease( context_ref );
      }

      CGColorSpaceRelease( color_space );
   }

   return bitmap;
}
#else // UNIX but not OSX

static TTF_Font * TTF_OpenFont_Magic(astring name, int fsize)
{
	emu_file file(OPEN_FLAG_READ);
	if (file.open(name) == FILERR_NONE)
	{
		unsigned char buffer[5] = { 0xff, 0xff, 0xff, 0xff, 0xff };
		unsigned char magic[5] = { 0x00, 0x01, 0x00, 0x00, 0x00 };
		file.read(buffer,5);
		if (memcmp(buffer, magic, 5))
			return NULL;
	}
	return TTF_OpenFont(name.cstr(), POINT_SIZE);
}

static TTF_Font *search_font_config(astring name, bool bold, bool italic, bool underline, bool &bakedstyles)
{
	TTF_Font *font = (TTF_Font *)NULL;
	FcConfig *config;
	FcPattern *pat;
	FcObjectSet *os;
	FcFontSet *fontset;
	FcValue val;

	config = FcConfigGetCurrent();
	pat = FcPatternCreate();
	os = FcObjectSetCreate();
	FcPatternAddString(pat, FC_FAMILY, (const FcChar8 *)name.cstr());

	// try and get a font with the requested styles baked-in
	if (bold)
	{
		if (italic)
		{
			FcPatternAddString(pat, FC_STYLE, (const FcChar8 *)"Bold Italic");
		}
		else
		{
			FcPatternAddString(pat, FC_STYLE, (const FcChar8 *)"Bold");
		}
	}
	else if (italic)
	{
		FcPatternAddString(pat, FC_STYLE, (const FcChar8 *)"Italic");
	}
	else
	{
		FcPatternAddString(pat, FC_STYLE, (const FcChar8 *)"Regular");
	}

	FcPatternAddString(pat, FC_FONTFORMAT, (const FcChar8 *)"TrueType");

	FcObjectSetAdd(os, FC_FILE);
	fontset = FcFontList(config, pat, os);

	for (int i = 0; i < fontset->nfont; i++)
	{
		if (FcPatternGet(fontset->fonts[i], FC_FILE, 0, &val) != FcResultMatch)
		{
			continue;
		}

		if (val.type != FcTypeString)
		{
			continue;
		}

		mame_printf_verbose("Matching font: %s\n", val.u.s);
		{
			astring match_name((const char*)val.u.s);
			font = TTF_OpenFont_Magic(match_name, POINT_SIZE);
		}

		if (font)
		{
			bakedstyles = true;
			break;
		}
	}

	// didn't get a font above?  try again with no baked-in styles
	if (!font)
	{
		FcPatternDestroy(pat);
		FcFontSetDestroy(fontset);

		pat = FcPatternCreate();
		FcPatternAddString(pat, FC_FAMILY, (const FcChar8 *)name.cstr());
		FcPatternAddString(pat, FC_STYLE, (const FcChar8 *)"Regular");
		FcPatternAddString(pat, FC_FONTFORMAT, (const FcChar8 *)"TrueType");
		fontset = FcFontList(config, pat, os);

		for (int i = 0; i < fontset->nfont; i++)
		{
			if (FcPatternGet(fontset->fonts[i], FC_FILE, 0, &val) != FcResultMatch)
			{
				continue;
			}

			if (val.type != FcTypeString)
			{
				continue;
			}

			mame_printf_verbose("Matching unstyled font: %s\n", val.u.s);
			{
				astring match_name((const char*)val.u.s);
				font = TTF_OpenFont_Magic(match_name, POINT_SIZE);
			}

			if (font)
			{
				break;
			}
		}
	}

	FcPatternDestroy(pat);
	FcObjectSetDestroy(os);
	FcFontSetDestroy(fontset);
	return font;
}

//-------------------------------------------------
//  font_open - attempt to "open" a handle to the
//  font with the given name
//-------------------------------------------------

osd_font sdl_osd_interface::font_open(const char *_name, int &height)
{
	TTF_Font *font = (TTF_Font *)NULL;
	bool bakedstyles = false;
	int style = 0;

	// accept qualifiers from the name
	astring name(_name);

	if (name == "default")
	{
		name = "Liberation Sans";
	}

	bool bold = (name.replace(0, "[B]", "") + name.replace(0, "[b]", "") > 0);
	bool italic = (name.replace(0, "[I]", "") + name.replace(0, "[i]", "") > 0);
	bool underline = (name.replace(0, "[U]", "") + name.replace(0, "[u]", "") > 0);
	bool strike = (name.replace(0, "[S]", "") + name.replace(0, "[s]", "") > 0);

	// first up, try it as a filename
	font = TTF_OpenFont_Magic(name, POINT_SIZE);

	// if no success, try the font path

	if (!font)
	{
		mame_printf_verbose("Searching font %s in -%s\n", name.cstr(), OPTION_FONTPATH);
		emu_file file(machine().options().font_path(), OPEN_FLAG_READ);
		if (file.open(name) == FILERR_NONE)
		{
			astring full_name = file.fullpath();
			font = TTF_OpenFont_Magic(full_name, POINT_SIZE);
			if (font)
				mame_printf_verbose("Found font %s\n", full_name.cstr());
		}
	}

	// if that didn't work, crank up the FontConfig database
	if (!font)
	{
		font = search_font_config(name, bold, italic, underline, bakedstyles);
	}

	if (!font)
	{
		printf("WARNING: Couldn't find/open TrueType font %s, using MAME default\n", name.cstr());
		return NULL;
	}

	// apply styles
	if (!bakedstyles)
	{
		style |= bold ? TTF_STYLE_BOLD : 0;
		style |= italic ? TTF_STYLE_ITALIC : 0;
	}
	style |= underline ? TTF_STYLE_UNDERLINE : 0;
	// SDL_ttf 2.0.9 and earlier does not define TTF_STYLE_STRIKETHROUGH
#if SDL_VERSIONNUM(TTF_MAJOR_VERSION, TTF_MINOR_VERSION, TTF_PATCHLEVEL) > SDL_VERSIONNUM(2,0,9)
	style |= strike ? TTF_STYLE_STRIKETHROUGH : 0;
#else
	if (strike)
		mame_printf_warning("Ignoring strikethrough for SDL_TTF with version less 2.0.10\n");
#endif // PATCHLEVEL
	TTF_SetFontStyle(font, style);

	height = TTF_FontLineSkip(font);

	return (osd_font)font;
}

//-------------------------------------------------
//  font_close - release resources associated with
//  a given OSD font
//-------------------------------------------------

void sdl_osd_interface::font_close(osd_font font)
{
	TTF_Font *ttffont;

	ttffont = (TTF_Font *)font;

	TTF_CloseFont(ttffont);
}

//-------------------------------------------------
//  font_get_bitmap - allocate and populate a
//  BITMAP_FORMAT_ARGB32 bitmap containing the
//  pixel values MAKE_ARGB(0xff,0xff,0xff,0xff)
//  or MAKE_ARGB(0x00,0xff,0xff,0xff) for each
//  pixel of a black & white font
//-------------------------------------------------

bitmap_t *sdl_osd_interface::font_get_bitmap(osd_font font, unicode_char chnum, INT32 &width, INT32 &xoffs, INT32 &yoffs)
{
	TTF_Font *ttffont;
	bitmap_t *bitmap = (bitmap_t *)NULL;
	SDL_Surface *drawsurf;
	SDL_Color fcol = { 0xff, 0xff, 0xff };
	UINT16 ustr[16];

	ttffont = (TTF_Font *)font;

	memset(ustr,0,sizeof(ustr));
	ustr[0] = (UINT16)chnum;
	drawsurf = TTF_RenderUNICODE_Solid(ttffont, ustr, fcol);

	// was nothing returned?
	if (drawsurf)
	{
		// allocate a MAME destination bitmap
		bitmap = auto_alloc(machine(), bitmap_t(drawsurf->w, drawsurf->h, BITMAP_FORMAT_ARGB32));

		// copy the rendered character image into it
		for (int y = 0; y < bitmap->height; y++)
		{
			UINT32 *dstrow = BITMAP_ADDR32(bitmap, y, 0);
			UINT8 *srcrow = (UINT8 *)drawsurf->pixels;

			srcrow += (y * drawsurf->pitch);

			for (int x = 0; x < drawsurf->w; x++)
			{
				dstrow[x] = srcrow[x] ? MAKE_ARGB(0xff,0xff,0xff,0xff) : MAKE_ARGB(0x00,0xff,0xff,0xff);
			}
		}

		// what are these?
		xoffs = yoffs = 0;
		width = drawsurf->w;

		SDL_FreeSurface(drawsurf);
	}

	return bitmap;
}
#endif	// not OSX
#else	// not UNIX
//-------------------------------------------------
//  font_open - attempt to "open" a handle to the
//  font with the given name
//-------------------------------------------------

osd_font sdl_osd_interface::font_open(const char *_name, int &height)
{
	return (osd_font)NULL;
}

//-------------------------------------------------
//  font_close - release resources associated with
//  a given OSD font
//-------------------------------------------------

void sdl_osd_interface::font_close(osd_font font)
{
}

//-------------------------------------------------
//  font_get_bitmap - allocate and populate a
//  BITMAP_FORMAT_ARGB32 bitmap containing the
//  pixel values MAKE_ARGB(0xff,0xff,0xff,0xff)
//  or MAKE_ARGB(0x00,0xff,0xff,0xff) for each
//  pixel of a black & white font
//-------------------------------------------------

bitmap_t *sdl_osd_interface::font_get_bitmap(osd_font font, unicode_char chnum, INT32 &width, INT32 &xoffs, INT32 &yoffs)
{
	return (bitmap_t *)NULL;
}
#endif
