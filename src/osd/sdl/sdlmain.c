//============================================================
//
//  sdlmain.c - main file for SDLMAME.
//
//  Copyright (c) 1996-2010, Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================

// standard sdl header
#include <SDL/SDL.h>
#include <SDL/SDL_version.h>

// standard includes

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

// we override SDL's normal startup on Win32
// please see sdlprefix.h as well

#if defined(SDLMAME_X11) && (SDL_MAJOR_VERSION == 1) && (SDL_MINOR_VERSION == 2)
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#endif

#ifdef SDLMAME_OS2
#define INCL_DOS
#include <os2.h>

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
//  LOCAL VARIABLES
//============================================================

#ifdef MESS
static char cwd[512];
#endif

//============================================================
//  OPTIONS
//============================================================

#ifndef INI_PATH
#if defined(SDLMAME_WIN32) || defined(SDLMAME_MACOSX) || defined(SDLMAME_OS2)
	#define INI_PATH ".;ini"
#else
#ifdef MESS
	#define INI_PATH "$HOME/.mess;.;ini"
#else
	#define INI_PATH "$HOME/.mame;.;ini"
#endif // MESS
#endif // MACOSX
#endif // INI_PATH

static const options_entry mame_sdl_options[] =
{
	{ SDLOPTION_INIPATH,                     INI_PATH,     0,                "path to ini files" },

	// debugging options
	{ NULL,                                   NULL,       OPTION_HEADER,     "DEBUGGING OPTIONS" },
	{ SDLOPTION_OSLOG,                        "0",        OPTION_BOOLEAN,    "output error.log data to the system debugger" },

	// performance options
	{ NULL,                                   NULL,       OPTION_HEADER,     "PERFORMANCE OPTIONS" },
	{ SDLOPTION_MULTITHREADING ";mt",         "0",        OPTION_BOOLEAN,    "enable multithreading; this enables rendering and blitting on a separate thread" },
	{ SDLOPTION_NUMPROCESSORS ";np",         "auto",      0,				 "number of processors; this overrides the number the system reports" },
	{ SDLOPTION_SDLVIDEOFPS,                  "0",        OPTION_BOOLEAN,    "show sdl video performance" },

	// video options
	{ NULL,                                   NULL,       OPTION_HEADER,     "VIDEO OPTIONS" },
	{ SDLOPTION_VIDEO,                   SDLOPTVAL_SOFT,  0,                 "video output method: soft or opengl" },
	{ SDLOPTION_NUMSCREENS,                   "1",        0,                 "number of screens to create; SDLMAME only supports 1 at this time" },
	{ SDLOPTION_WINDOW ";w",                  "0",        OPTION_BOOLEAN,    "enable window mode; otherwise, full screen mode is assumed" },
	{ SDLOPTION_MAXIMIZE ";max",              "1",        OPTION_BOOLEAN,    "default to maximized windows; otherwise, windows will be minimized" },
	{ SDLOPTION_KEEPASPECT ";ka",             "1",        OPTION_BOOLEAN,    "constrain to the proper aspect ratio" },
	{ SDLOPTION_UNEVENSTRETCH ";ues",         "1",        OPTION_BOOLEAN,    "allow non-integer stretch factors" },
	{ SDLOPTION_EFFECT,                  SDLOPTVAL_NONE,  0,                 "name of a PNG file to use for visual effects, or 'none'" },
	{ SDLOPTION_CENTERH,                      "1",        OPTION_BOOLEAN,    "center horizontally within the view area" },
	{ SDLOPTION_CENTERV,                      "1",        OPTION_BOOLEAN,    "center vertically within the view area" },
	#if (SDL_VERSION_ATLEAST(1,2,10))
	{ SDLOPTION_WAITVSYNC,                    "0",        OPTION_BOOLEAN,    "enable waiting for the start of VBLANK before flipping screens; reduces tearing effects" },
	#endif
#if (SDL_VERSION_ATLEAST(1,3,0))
	{ SDLOPTION_SCALEMODE ";sm",         SDLOPTVAL_NONE,  0,                 "Scale mode: none, hwblit, hwbest, yv12, yuy2, yv12x2, yuy2x2 (-video soft only)" },
#else
	{ SDLOPTION_SCALEMODE ";sm",         SDLOPTVAL_NONE,  0,                 "Scale mode: none, async, yv12, yuy2, yv12x2, yuy2x2 (-video soft only)" },
#endif
#if USE_OPENGL
	// OpenGL specific options
	{ NULL,                                   NULL,   OPTION_HEADER,  "OpenGL-SPECIFIC OPTIONS" },
	{ SDLOPTION_FILTER ";glfilter;flt",       "1",    OPTION_BOOLEAN, "enable bilinear filtering on screen output" },
	{ SDLOPTION_PRESCALE,                     "1",        0,                 "scale screen rendering by this amount in software" },
	{ SDLOPTION_GL_FORCEPOW2TEXTURE,          "0",    OPTION_BOOLEAN, "force power of two textures  (default no)" },
	{ SDLOPTION_GL_NOTEXTURERECT,             "0",    OPTION_BOOLEAN, "don't use OpenGL GL_ARB_texture_rectangle (default on)" },
	{ SDLOPTION_GL_VBO,                       "1",    OPTION_BOOLEAN, "enable OpenGL VBO,  if available (default on)" },
	{ SDLOPTION_GL_PBO,                       "1",    OPTION_BOOLEAN, "enable OpenGL PBO,  if available (default on)" },
	{ SDLOPTION_GL_GLSL,                      "0",    OPTION_BOOLEAN, "enable OpenGL GLSL, if available (default off)" },
	{ SDLOPTION_GLSL_FILTER,				  "1",    0,              "enable OpenGL GLSL filtering instead of FF filtering 0-plain, 1-bilinear (default)" },
	{ SDLOPTION_SHADER_MAME("0"),    SDLOPTVAL_NONE,  0,              "custom OpenGL GLSL shader set mame bitmap 0" },
	{ SDLOPTION_SHADER_MAME("1"),    SDLOPTVAL_NONE,  0,              "custom OpenGL GLSL shader set mame bitmap 1" },
	{ SDLOPTION_SHADER_MAME("2"),    SDLOPTVAL_NONE,  0,              "custom OpenGL GLSL shader set mame bitmap 2" },
	{ SDLOPTION_SHADER_MAME("3"),    SDLOPTVAL_NONE,  0,              "custom OpenGL GLSL shader set mame bitmap 3" },
	{ SDLOPTION_SHADER_MAME("4"),    SDLOPTVAL_NONE,  0,              "custom OpenGL GLSL shader set mame bitmap 4" },
	{ SDLOPTION_SHADER_MAME("5"),    SDLOPTVAL_NONE,  0,              "custom OpenGL GLSL shader set mame bitmap 5" },
	{ SDLOPTION_SHADER_MAME("6"),    SDLOPTVAL_NONE,  0,              "custom OpenGL GLSL shader set mame bitmap 6" },
	{ SDLOPTION_SHADER_MAME("7"),    SDLOPTVAL_NONE,  0,              "custom OpenGL GLSL shader set mame bitmap 7" },
	{ SDLOPTION_SHADER_MAME("8"),    SDLOPTVAL_NONE,  0,              "custom OpenGL GLSL shader set mame bitmap 8" },
	{ SDLOPTION_SHADER_MAME("9"),    SDLOPTVAL_NONE,  0,              "custom OpenGL GLSL shader set mame bitmap 9" },
	{ SDLOPTION_SHADER_SCREEN("0"),  SDLOPTVAL_NONE,  0,              "custom OpenGL GLSL shader screen bitmap 0" },
	{ SDLOPTION_SHADER_SCREEN("1"),  SDLOPTVAL_NONE,  0,              "custom OpenGL GLSL shader screen bitmap 1" },
	{ SDLOPTION_SHADER_SCREEN("2"),  SDLOPTVAL_NONE,  0,              "custom OpenGL GLSL shader screen bitmap 2" },
	{ SDLOPTION_SHADER_SCREEN("3"),  SDLOPTVAL_NONE,  0,              "custom OpenGL GLSL shader screen bitmap 3" },
	{ SDLOPTION_SHADER_SCREEN("4"),  SDLOPTVAL_NONE,  0,              "custom OpenGL GLSL shader screen bitmap 4" },
	{ SDLOPTION_SHADER_SCREEN("5"),  SDLOPTVAL_NONE,  0,              "custom OpenGL GLSL shader screen bitmap 5" },
	{ SDLOPTION_SHADER_SCREEN("6"),  SDLOPTVAL_NONE,  0,              "custom OpenGL GLSL shader screen bitmap 6" },
	{ SDLOPTION_SHADER_SCREEN("7"),  SDLOPTVAL_NONE,  0,              "custom OpenGL GLSL shader screen bitmap 7" },
	{ SDLOPTION_SHADER_SCREEN("8"),  SDLOPTVAL_NONE,  0,              "custom OpenGL GLSL shader screen bitmap 8" },
	{ SDLOPTION_SHADER_SCREEN("9"),  SDLOPTVAL_NONE,  0,              "custom OpenGL GLSL shader screen bitmap 9" },
	{ SDLOPTION_GL_GLSL_VID_ATTR,			 "1",    OPTION_BOOLEAN,  "enable OpenGL GLSL handling of brightness and contrast. Better RGB game performance for free. (default)" },
#endif

	// per-window options
	{ NULL,                                   NULL, OPTION_HEADER,    "PER-WINDOW VIDEO OPTIONS" },
	{ SDLOPTION_SCREEN(""),                   SDLOPTVAL_AUTO,   0,    "explicit name of the first screen; 'auto' here will try to make a best guess" },
	{ SDLOPTION_ASPECT("") ";screen_aspect",  SDLOPTVAL_AUTO,   0,    "aspect ratio for all screens; 'auto' here will try to make a best guess" },
	{ SDLOPTION_RESOLUTION("") ";r",          SDLOPTVAL_AUTO,   0,    "preferred resolution for all screens; format is <width>x<height>[@<refreshrate>] or 'auto'" },
	{ SDLOPTION_VIEW(""),                     SDLOPTVAL_AUTO,   0,    "preferred view for all screens" },

	{ SDLOPTION_SCREEN("0"),                  SDLOPTVAL_AUTO,   0,    "explicit name of the first screen; 'auto' here will try to make a best guess" },
	{ SDLOPTION_ASPECT("0"),                  SDLOPTVAL_AUTO,   0,    "aspect ratio of the first screen; 'auto' here will try to make a best guess" },
	{ SDLOPTION_RESOLUTION("0") ";r0",        SDLOPTVAL_AUTO,   0,    "preferred resolution of the first screen; format is <width>x<height>[@<refreshrate>] or 'auto'" },
	{ SDLOPTION_VIEW("0"),                    SDLOPTVAL_AUTO,   0,    "preferred view for the first screen" },

	{ SDLOPTION_SCREEN("1"),                  SDLOPTVAL_AUTO,   0,    "explicit name of the second screen; 'auto' here will try to make a best guess" },
	{ SDLOPTION_ASPECT("1"),                  SDLOPTVAL_AUTO,   0,    "aspect ratio of the second screen; 'auto' here will try to make a best guess" },
	{ SDLOPTION_RESOLUTION("1") ";r1",        SDLOPTVAL_AUTO,   0,    "preferred resolution of the second screen; format is <width>x<height>[@<refreshrate>] or 'auto'" },
	{ SDLOPTION_VIEW("1"),                    SDLOPTVAL_AUTO,   0,    "preferred view for the second screen" },

	{ SDLOPTION_SCREEN("2"),                  SDLOPTVAL_AUTO,   0,    "explicit name of the third screen; 'auto' here will try to make a best guess" },
	{ SDLOPTION_ASPECT("2"),                  SDLOPTVAL_AUTO,   0,    "aspect ratio of the third screen; 'auto' here will try to make a best guess" },
	{ SDLOPTION_RESOLUTION("2") ";r2",        SDLOPTVAL_AUTO,   0,    "preferred resolution of the third screen; format is <width>x<height>[@<refreshrate>] or 'auto'" },
	{ SDLOPTION_VIEW("2"),                    SDLOPTVAL_AUTO,   0,    "preferred view for the third screen" },

	{ SDLOPTION_SCREEN("3"),                  SDLOPTVAL_AUTO,   0,    "explicit name of the fourth screen; 'auto' here will try to make a best guess" },
	{ SDLOPTION_ASPECT("3"),                  SDLOPTVAL_AUTO,   0,    "aspect ratio of the fourth screen; 'auto' here will try to make a best guess" },
	{ SDLOPTION_RESOLUTION("3") ";r3",        SDLOPTVAL_AUTO,   0,    "preferred resolution of the fourth screen; format is <width>x<height>[@<refreshrate>] or 'auto'" },
	{ SDLOPTION_VIEW("3"),                    SDLOPTVAL_AUTO,   0,    "preferred view for the fourth screen" },

	// full screen options
	{ NULL,                                   NULL,  OPTION_HEADER,     "FULL SCREEN OPTIONS" },
	{ SDLOPTION_SWITCHRES,                    "0",   OPTION_BOOLEAN,    "enable resolution switching" },
	#ifdef SDLMAME_X11
	{ SDLOPTION_USEALLHEADS,	             "0",	  OPTION_BOOLEAN,    "split full screen image across monitors" },
	#endif

	// sound options
	{ NULL,                                   NULL,  OPTION_HEADER,     "SOUND OPTIONS" },
	{ SDLOPTION_AUDIO_LATENCY,                "3",   0,                 "set audio latency (increase to reduce glitches, decrease for responsiveness)" },

	// keyboard mapping
	{ NULL, 		                          NULL,  OPTION_HEADER,     "SDL KEYBOARD MAPPING" },
	{ SDLOPTION_KEYMAP,                      "0",    OPTION_BOOLEAN,    "enable keymap" },
	{ SDLOPTION_KEYMAP_FILE,                 "keymap.dat", 0,           "keymap filename" },
#ifdef MESS
#ifdef SDLMAME_MACOSX	// work around for SDL 1.2.11 on Mac - 1.2.12 should not require this
	{ SDLOPTION_UIMODEKEY,			"DELETE", 0,                  "Key to toggle MESS keyboard mode" },
#else
	{ SDLOPTION_UIMODEKEY,			"SCROLLOCK", 0,                  "Key to toggle MESS keyboard mode" },
#endif	// SDLMAME_MACOSX
#endif	// MESS

	// joystick mapping
	{ NULL, 		                         NULL,   OPTION_HEADER,     "SDL JOYSTICK MAPPING" },
	{ SDLOPTION_JOYINDEX "1",                SDLOPTVAL_AUTO, 0,         "name of joystick mapped to joystick #1" },
	{ SDLOPTION_JOYINDEX "2",                SDLOPTVAL_AUTO, 0,         "name of joystick mapped to joystick #2" },
	{ SDLOPTION_JOYINDEX "3",                SDLOPTVAL_AUTO, 0,         "name of joystick mapped to joystick #3" },
	{ SDLOPTION_JOYINDEX "4",                SDLOPTVAL_AUTO, 0,         "name of joystick mapped to joystick #4" },
	{ SDLOPTION_JOYINDEX "5",                SDLOPTVAL_AUTO, 0,         "name of joystick mapped to joystick #5" },
	{ SDLOPTION_JOYINDEX "6",                SDLOPTVAL_AUTO, 0,         "name of joystick mapped to joystick #6" },
	{ SDLOPTION_JOYINDEX "7",                SDLOPTVAL_AUTO, 0,         "name of joystick mapped to joystick #7" },
	{ SDLOPTION_JOYINDEX "8",                SDLOPTVAL_AUTO, 0,         "name of joystick mapped to joystick #8" },
	{ SDLOPTION_SIXAXIS,			         "0",	 OPTION_BOOLEAN,    "Use special handling for PS3 Sixaxis controllers" },

#if (SDL_VERSION_ATLEAST(1,3,0))
	{ NULL, 		                         NULL,   OPTION_HEADER,     "SDL MOUSE MAPPING" },
	{ SDLOPTION_MOUSEINDEX "1",              SDLOPTVAL_AUTO, 0,         "name of mouse mapped to mouse #1" },
	{ SDLOPTION_MOUSEINDEX "2",              SDLOPTVAL_AUTO, 0,         "name of mouse mapped to mouse #2" },
	{ SDLOPTION_MOUSEINDEX "3",              SDLOPTVAL_AUTO, 0,         "name of mouse mapped to mouse #3" },
	{ SDLOPTION_MOUSEINDEX "4",              SDLOPTVAL_AUTO, 0,         "name of mouse mapped to mouse #4" },
	{ SDLOPTION_MOUSEINDEX "5",              SDLOPTVAL_AUTO, 0,         "name of mouse mapped to mouse #5" },
	{ SDLOPTION_MOUSEINDEX "6",              SDLOPTVAL_AUTO, 0,         "name of mouse mapped to mouse #6" },
	{ SDLOPTION_MOUSEINDEX "7",              SDLOPTVAL_AUTO, 0,         "name of mouse mapped to mouse #7" },
	{ SDLOPTION_MOUSEINDEX "8",              SDLOPTVAL_AUTO, 0,         "name of mouse mapped to mouse #8" },

	{ NULL, 		                         NULL,   OPTION_HEADER,     "SDL KEYBOARD MAPPING" },
	{ SDLOPTION_KEYBINDEX "1",               SDLOPTVAL_AUTO, 0,         "name of keyboard mapped to keyboard #1" },
	{ SDLOPTION_KEYBINDEX "2",               SDLOPTVAL_AUTO, 0,         "name of keyboard mapped to keyboard #2" },
	{ SDLOPTION_KEYBINDEX "3",               SDLOPTVAL_AUTO, 0,         "name of keyboard mapped to keyboard #3" },
	{ SDLOPTION_KEYBINDEX "4",               SDLOPTVAL_AUTO, 0,         "name of keyboard mapped to keyboard #4" },
	{ SDLOPTION_KEYBINDEX "5",               SDLOPTVAL_AUTO, 0,         "name of keyboard mapped to keyboard #5" },
	{ SDLOPTION_KEYBINDEX "6",               SDLOPTVAL_AUTO, 0,         "name of keyboard mapped to keyboard #6" },
	{ SDLOPTION_KEYBINDEX "7",               SDLOPTVAL_AUTO, 0,         "name of keyboard mapped to keyboard #7" },
	{ SDLOPTION_KEYBINDEX "8",               SDLOPTVAL_AUTO, 0,         "name of keyboard mapped to keyboard #8" },
#endif
	// SDL low level driver options
	{ NULL, 		                         NULL,   OPTION_HEADER,     "SDL LOWLEVEL DRIVER OPTIONS" },
	{ SDLOPTION_VIDEODRIVER ";vd",           SDLOPTVAL_AUTO,  0,        "sdl video driver to use ('x11', 'directfb', ... or 'auto' for SDL default" },
#if (SDL_VERSION_ATLEAST(1,3,0))
	{ SDLOPTION_RENDERDRIVER ";rd",          SDLOPTVAL_AUTO,  0,        "sdl render driver to use ('software', 'opengl', 'directfb' ... or 'auto' for SDL default" },
#endif
	{ SDLOPTION_AUDIODRIVER ";ad",           SDLOPTVAL_AUTO,  0,        "sdl audio driver to use ('alsa', 'arts', ... or 'auto' for SDL default" },
#if USE_OPENGL
	{ SDLOPTION_GL_LIB,                      SDLOPTVAL_GLLIB, 0,        "alternative libGL.so to use; 'auto' for system default" },
#endif

	// End of list
	{ NULL }
};

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

	#ifdef SDLMAME_OS2
	MorphToPM();
	#endif

	#ifdef MESS
	getcwd(cwd, 511);
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

	res = cli_execute(argc, argv, mame_sdl_options);

#ifdef MALLOC_DEBUG
	{
		void check_unfreed_mem(void);
		check_unfreed_mem();
	}
#endif

	// already called...
	//SDL_Quit();

	exit(res);

	return res;
}



//============================================================
//  output_oslog
//============================================================

static void output_oslog(running_machine *machine, const char *buffer)
{
	fputs(buffer, stderr);
}



//============================================================
//  osd_exit
//============================================================

static void osd_exit(running_machine *machine)
{

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
	int i, cur, num = SDL_GetNumVideoDrivers();

	mame_printf_verbose("Available videodrivers: ");
	for (i=0;i<num;i++)
	{
		const char *name = SDL_GetVideoDriver(i);
		mame_printf_verbose("%s ", name);
	}
	mame_printf_verbose("\n");
	mame_printf_verbose("Current Videodriver: %s\n", SDL_GetCurrentVideoDriver());
	num = SDL_GetNumVideoDisplays();
	cur = SDL_GetCurrentVideoDisplay();
	for (i=0;i<num;i++)
	{
		SDL_DisplayMode mode;
		int j;

		SDL_SelectVideoDisplay(i);
		mame_printf_verbose("\tDisplay #%d\n", i);
		if (SDL_GetDesktopDisplayMode(&mode));
			mame_printf_verbose("\t\tDesktop Mode:         %dx%d-%d@%d\n", mode.w, mode.h, SDL_BITSPERPIXEL(mode.format), mode.refresh_rate);
		if (SDL_GetCurrentDisplayMode(&mode));
			mame_printf_verbose("\t\tCurrent Display Mode: %dx%d-%d@%d\n", mode.w, mode.h, SDL_BITSPERPIXEL(mode.format), mode.refresh_rate);
		mame_printf_verbose("\t\tRenderdrivers:\n");
		for (j=0; j<SDL_GetNumRenderDrivers(); j++)
		{
			SDL_RendererInfo info;
			SDL_GetRenderDriverInfo(j, &info);
			mame_printf_verbose("\t\t\t%10s (%dx%d)\n", info.name, info.max_texture_width, info.max_texture_height);
		}
	}
	SDL_SelectVideoDisplay(cur);

	mame_printf_verbose("Available audio drivers: \n");
	num = SDL_GetNumAudioDrivers();
	for (i=0;i<num;i++)
	{
		mame_printf_verbose("\t%-20s\n", SDL_GetAudioDriver(i));
	}
#endif
}


//============================================================
//  osd_init
//============================================================

void osd_init(running_machine *machine)
{
	const char *stemp;

	// Some driver options - must be before audio init!
	stemp = options_get_string(mame_options(), SDLOPTION_AUDIODRIVER);
	if (stemp != NULL && strcmp(stemp, SDLOPTVAL_AUTO) != 0)
	{
		mame_printf_verbose("Setting SDL audiodriver '%s' ...\n", stemp);
		osd_setenv(SDLENV_AUDIODRIVER, stemp, 1);
	}

	stemp = options_get_string(mame_options(), SDLOPTION_VIDEODRIVER);
	if (stemp != NULL && strcmp(stemp, SDLOPTVAL_AUTO) != 0)
	{
		mame_printf_verbose("Setting SDL videodriver '%s' ...\n", stemp);
		osd_setenv(SDLENV_VIDEODRIVER, stemp, 1);
	}

	if (SDL_VERSION_ATLEAST(1,3,0))
	{
		stemp = options_get_string(mame_options(), SDLOPTION_RENDERDRIVER);
		if (stemp != NULL && strcmp(stemp, SDLOPTVAL_AUTO) != 0)
		{
			mame_printf_verbose("Setting SDL renderdriver '%s' ...\n", stemp);
			osd_setenv(SDLENV_RENDERDRIVER, stemp, 1);
		}
	}

	/* Set the SDL environment variable for drivers wanting to load the
     * lib at startup.
     */
	/* FIXME: move lib loading code from drawogl.c here */

	stemp = options_get_string(mame_options(), SDLOPTION_GL_LIB);
	if (stemp != NULL && strcmp(stemp, SDLOPTVAL_AUTO) != 0)
	{
		osd_setenv("SDL_VIDEO_GL_DRIVER", stemp, 1);
		mame_printf_verbose("Setting SDL_VIDEO_GL_DRIVER = '%s' ...\n", stemp);
	}

	/* get number of processors */
	stemp = options_get_string(mame_options(), SDLOPTION_NUMPROCESSORS);

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
	add_exit_callback(machine, osd_exit);

	defines_verbose();

	if (!SDLMAME_HAS_DEBUGGER)
		if (options_get_bool(mame_options(), OPTION_DEBUG))
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

	if (options_get_bool(mame_options(), SDLOPTION_OSLOG))
		add_logerror_callback(machine, output_oslog);

#if (SDL_VERSION_ATLEAST(1,3,0))
	SDL_EventState(SDL_TEXTINPUT, SDL_TRUE);
#else
	SDL_EnableUNICODE(SDL_TRUE);
#endif
}

#ifdef MESS
char *osd_get_startup_cwd(void)
{
	return cwd;
}
#endif
