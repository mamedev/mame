/***************************************************************************

    emuopts.c

    Options file and command line management.

    Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#include "mamecore.h"
#include "mame.h"
#include "fileio.h"
#include "emuopts.h"

#include <ctype.h>



/***************************************************************************
    BUILT-IN (CORE) OPTIONS
***************************************************************************/

const options_entry mame_core_options[] =
{
	/* unadorned options - only a single one supported at the moment */
	{ "<UNADORNED0>",                NULL,        0,                 NULL },

	/* config options */
	{ NULL,                          NULL,        OPTION_HEADER,     "CORE CONFIGURATION OPTIONS" },
	{ "readconfig;rc",               "1",         OPTION_BOOLEAN,    "enable loading of configuration files" },

	/* seach path options */
	{ NULL,                          NULL,        OPTION_HEADER,     "CORE SEARCH PATH OPTIONS" },
	{ "rompath;rp;biospath;bp",      "roms",      0,                 "path to ROMsets and hard disk images" },
#ifdef MESS
	{ "hashpath;hash_directory;hash","hash",      0,                 "path to hash files" },
#endif /* MESS */
	{ "samplepath;sp",               "samples",   0,                 "path to samplesets" },
	{ "artpath;artwork_directory",   "artwork",   0,                 "path to artwork files" },
	{ "ctrlrpath;ctrlr_directory",   "ctrlr",     0,                 "path to controller definitions" },
	{ "inipath",                     ".;ini",     0,                 "path to ini files" },
	{ "fontpath",                    ".",         0,                 "path to font files" },

	/* output directory options */
	{ NULL,                          NULL,        OPTION_HEADER,     "CORE OUTPUT DIRECTORY OPTIONS" },
	{ "cfg_directory",               "cfg",       0,                 "directory to save configurations" },
	{ "nvram_directory",             "nvram",     0,                 "directory to save nvram contents" },
	{ "memcard_directory",           "memcard",   0,                 "directory to save memory card contents" },
	{ "input_directory",             "inp",       0,                 "directory to save input device logs" },
	{ "state_directory",             "sta",       0,                 "directory to save states" },
	{ "snapshot_directory",          "snap",      0,                 "directory to save screenshots" },
	{ "diff_directory",              "diff",      0,                 "directory to save hard drive image difference files" },
	{ "comment_directory",           "comments",  0,                 "directory to save debugger comments" },

	/* filename options */
	{ NULL,                          NULL,        OPTION_HEADER,     "CORE FILENAME OPTIONS" },
	{ "cheat_file",                  "cheat.dat", 0,                 "cheat filename" },

	/* state/playback options */
	{ NULL,                          NULL,        OPTION_HEADER,     "CORE STATE/PLAYBACK OPTIONS" },
	{ "state",                       NULL,        0,                 "saved state to load" },
	{ "autosave",                    "0",         OPTION_BOOLEAN,    "enable automatic restore at startup, and automatic save at exit time" },
	{ "playback;pb",                 NULL,        0,                 "playback an input file" },
	{ "record;rec",                  NULL,        0,                 "record an input file" },
	{ "mngwrite",                    NULL,        0,                 "optional filename to write a MNG movie of the current session" },
	{ "wavwrite",                    NULL,        0,                 "optional filename to write a WAV file of the current session" },

	/* performance options */
	{ NULL,                          NULL,        OPTION_HEADER,     "CORE PERFORMANCE OPTIONS" },
	{ "autoframeskip;afs",           "0",         OPTION_BOOLEAN,    "enable automatic frameskip selection" },
	{ "frameskip;fs(0-10)",          "0",         0,                 "set frameskip to fixed value, 0-12 (autoframeskip must be disabled)" },
	{ "seconds_to_run;str",          "0",         0,                 "number of emulated seconds to run before automatically exiting" },
	{ "throttle",                    "1",         OPTION_BOOLEAN,    "enable throttling to keep game running in sync with real time" },
	{ "sleep",                       "1",         OPTION_BOOLEAN,    "enable sleeping, which gives time back to other applications when idle" },
	{ "speed(0.01-100)",             "1.0",       0,                 "controls the speed of gameplay, relative to realtime; smaller numbers are slower" },
	{ "refreshspeed;rs",             "0",         OPTION_BOOLEAN,    "automatically adjusts the speed of gameplay to keep the refresh rate lower than the screen" },

	/* rotation options */
	{ NULL,                          NULL,        OPTION_HEADER,     "CORE ROTATION OPTIONS" },
	{ "rotate",                      "1",         OPTION_BOOLEAN,    "rotate the game screen according to the game's orientation needs it" },
	{ "ror",                         "0",         OPTION_BOOLEAN,    "rotate screen clockwise 90 degrees" },
	{ "rol",                         "0",         OPTION_BOOLEAN,    "rotate screen counterclockwise 90 degrees" },
	{ "autoror",                     "0",         OPTION_BOOLEAN,    "automatically rotate screen clockwise 90 degrees if vertical" },
	{ "autorol",                     "0",         OPTION_BOOLEAN,    "automatically rotate screen counterclockwise 90 degrees if vertical" },
	{ "flipx",                       "0",         OPTION_BOOLEAN,    "flip screen left-right" },
	{ "flipy",                       "0",         OPTION_BOOLEAN,    "flip screen upside-down" },

	/* artwork options */
	{ NULL,                          NULL,        OPTION_HEADER,     "CORE ARTWORK OPTIONS" },
	{ "artwork_crop;artcrop",        "0",         OPTION_BOOLEAN,    "crop artwork to game screen size" },
	{ "use_backdrops;backdrop",      "1",         OPTION_BOOLEAN,    "enable backdrops if artwork is enabled and available" },
	{ "use_overlays;overlay",        "1",         OPTION_BOOLEAN,    "enable overlays if artwork is enabled and available" },
	{ "use_bezels;bezel",            "1",         OPTION_BOOLEAN,    "enable bezels if artwork is enabled and available" },

	/* screen options */
	{ NULL,                          NULL,        OPTION_HEADER,     "CORE SCREEN OPTIONS" },
	{ "brightness(0.1-2.0)",         "1.0",       0,                 "default game screen brightness correction" },
	{ "contrast(0.1-2.0)",           "1.0",       0,                 "default game screen contrast correction" },
	{ "gamma(0.1-3.0)",              "1.0",       0,                 "default game screen gamma correction" },
	{ "pause_brightness(0.0-1.0)",   "0.65",      0,                 "amount to scale the screen brightness when paused" },

	/* vector options */
	{ NULL,                          NULL,        OPTION_HEADER,     "CORE VECTOR OPTIONS" },
	{ "antialias;aa",                "1",         OPTION_BOOLEAN,    "use antialiasing when drawing vectors" },
	{ "beam",                        "1.0",       0,                 "set vector beam width" },
	{ "flicker",                     "0",         0,                 "set vector flicker effect" },

	/* sound options */
	{ NULL,                          NULL,        OPTION_HEADER,     "CORE SOUND OPTIONS" },
	{ "sound",                       "1",         OPTION_BOOLEAN,    "enable sound output" },
	{ "samplerate;sr(1000-1000000)", "48000",     0,                 "set sound output sample rate" },
	{ "samples",                     "1",         OPTION_BOOLEAN,    "enable the use of external samples if available" },
	{ "volume;vol",                  "0",         0,                 "sound volume in decibels (-32 min, 0 max)" },

	/* input options */
	{ NULL,                          NULL,        OPTION_HEADER,     "CORE INPUT OPTIONS" },
	{ "ctrlr",                       NULL,        0,                 "preconfigure for specified controller" },
	{ "mouse",                       "0",         OPTION_BOOLEAN,    "enable mouse input" },
	{ "joystick;joy",                "0",         OPTION_BOOLEAN,    "enable joystick input" },
	{ "lightgun;gun",                "0",         OPTION_BOOLEAN,    "enable lightgun input" },
	{ "multikeyboard;multikey",      "0",         OPTION_BOOLEAN,    "enable separate input from each keyboard device (if present)" },
	{ "multimouse",                  "0",         OPTION_BOOLEAN,    "enable separate input from each mouse device (if present)" },
	{ "steadykey;steady",            "0",         OPTION_BOOLEAN,    "enable steadykey support" },
	{ "offscreen_reload;reload",     "0",         OPTION_BOOLEAN,    "convert lightgun button 2 into offscreen reload" },
	{ "joystick_map;joymap",         "auto",      0,                 "explicit joystick map, or auto to auto-select" },
	{ "joystick_deadzone;joy_deadzone;jdz",      "0.3",  0,          "center deadzone range for joystick where change is ignored (0.0 center, 1.0 end)" },
	{ "joystick_saturation;joy_saturation;jsat", "0.85", 0,          "end of axis saturation range for joystick where change is ignored (0.0 center, 1.0 end)" },

	/* input autoenable options */
	{ NULL,                          NULL,        OPTION_HEADER,     "CORE INPUT AUTOMATIC ENABLE OPTIONS" },
	{ "paddle_device;paddle",        "keyboard",  0,                 "enable (keyboard|mouse|joystick) if a paddle control is present" },
	{ "adstick_device;adstick",      "keyboard",  0,                 "enable (keyboard|mouse|joystick) if an analog joystick control is present" },
	{ "pedal_device;pedal",          "keyboard",  0,                 "enable (keyboard|mouse|joystick) if a pedal control is present" },
	{ "dial_device;dial",            "keyboard",  0,                 "enable (keyboard|mouse|joystick) if a dial control is present" },
	{ "trackball_device;trackball",  "keyboard",  0,                 "enable (keyboard|mouse|joystick) if a trackball control is present" },
	{ "lightgun_device",             "keyboard",  0,                 "enable (keyboard|mouse|joystick) if a lightgun control is present" },
	{ "positional_device",           "keyboard",  0,                 "enable (keyboard|mouse|joystick) if a positional control is present" },
	{ "mouse_device",                "mouse",     0,                 "enable (keyboard|mouse|joystick) if a mouse control is present" },

	/* debugging options */
	{ NULL,                          NULL,        OPTION_HEADER,     "CORE DEBUGGING OPTIONS" },
	{ "log",                         "0",         OPTION_BOOLEAN,    "generate an error.log file" },
	{ "verbose;v",                   "0",         OPTION_BOOLEAN,    "display additional diagnostic information" },
#ifdef MAME_DEBUG
	{ "debug;d",                     "1",         OPTION_BOOLEAN,    "enable/disable debugger" },
	{ "debugscript",                 NULL,        0,                 "script for debugger" },
#else
	{ "debug;d",                     "1",         OPTION_DEPRECATED, "(debugger-only command)" },
	{ "debugscript",                 NULL,        OPTION_DEPRECATED, "(debugger-only command)" },
#endif

	/* misc options */
	{ NULL,                          NULL,        OPTION_HEADER,     "CORE MISC OPTIONS" },
	{ "bios",                        "default",   0,                 "select the system BIOS to use" },
	{ "cheat;c",                     "0",         OPTION_BOOLEAN,    "enable cheat subsystem" },
	{ "skip_gameinfo",               "0",         OPTION_BOOLEAN,    "skip displaying the information screen at startup" },

	{ NULL }
};



/***************************************************************************
    CORE IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    memory_error - report a memory error
-------------------------------------------------*/

static void memory_error(const char *message)
{
	fatalerror("%s", message);
}



/*-------------------------------------------------
    mame_puts_info
    mame_puts_warning
    mame_puts_error
-------------------------------------------------*/

static void mame_puts_info(const char *s)
{
	mame_printf_info("%s", s);
}

static void mame_puts_warning(const char *s)
{
	mame_printf_warning("%s", s);
}

static void mame_puts_error(const char *s)
{
	mame_printf_error("%s", s);
}



/*-------------------------------------------------
    mame_options_init - create core MAME options
-------------------------------------------------*/

core_options *mame_options_init(const options_entry *entries)
{
	/* create MAME core options */
	core_options *opts = options_create(memory_error);

	/* set up output callbacks */
	options_set_output_callback(opts, OPTMSG_INFO, mame_puts_info);
	options_set_output_callback(opts, OPTMSG_WARNING, mame_puts_warning);
	options_set_output_callback(opts, OPTMSG_ERROR, mame_puts_error);

	options_add_entries(opts, mame_core_options);
	if (entries != NULL)
		options_add_entries(opts, entries);

#ifdef MESS
	mess_options_init(opts);
#endif /* MESS */

	return opts;
}
