/***************************************************************************

    mame.c

    Controls execution of the core MAME system.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

****************************************************************************

    Since there has been confusion in the past over the order of
    initialization and other such things, here it is, all spelled out
    as of January, 2008:

    main()
        - does platform-specific init
        - calls mame_execute() [mame.c]

        mame_execute() [mame.c]
            - calls mame_validitychecks() [validity.c] to perform validity checks on all compiled drivers
            - begins resource tracking (level 1)
            - calls create_machine [mame.c] to initialize the running_machine structure
            - calls init_machine() [mame.c]

            init_machine() [mame.c]
                - calls fileio_init() [fileio.c] to initialize file I/O info
                - calls config_init() [config.c] to initialize configuration system
                - calls input_init() [input.c] to initialize the input system
                - calls output_init() [output.c] to initialize the output system
                - calls state_init() [state.c] to initialize save state system
                - calls state_save_allow_registration() [state.c] to allow registrations
                - calls palette_init() [palette.c] to initialize palette system
                - calls render_init() [render.c] to initialize the rendering system
                - calls ui_init() [ui.c] to initialize the user interface
                - calls generic_machine_init() [machine/generic.c] to initialize generic machine structures
                - calls generic_video_init() [video/generic.c] to initialize generic video structures
                - calls generic_sound_init() [audio/generic.c] to initialize generic sound structures
                - calls timer_init() [timer.c] to reset the timer system
                - calls osd_init() [osdepend.h] to do platform-specific initialization
                - calls input_port_init() [inptport.c] to set up the input ports
                - calls rom_init() [romload.c] to load the game's ROMs
                - calls memory_init() [memory.c] to process the game's memory maps
                - calls watchdog_init() [watchdog.c] to initialize the watchdog system
                - calls the driver's DRIVER_INIT callback
                - calls device_list_start() [devintrf.c] to start any devices
                - calls video_init() [video.c] to start the video system
                - calls tilemap_init() [tilemap.c] to start the tilemap system
                - calls crosshair_init() [crsshair.c] to configure the crosshairs
                - calls sound_init() [sound.c] to start the audio system
                - calls debugger_init() [debugger.c] to set up the debugger
                - calls the driver's MACHINE_START, SOUND_START, and VIDEO_START callbacks
                - calls cheat_init() [cheat.c] to initialize the cheat system
                - calls image_init() [image.c] to initialize the image system

            - calls config_load_settings() [config.c] to load the configuration file
            - calls nvram_load [machine/generic.c] to load NVRAM
            - calls ui_display_startup_screens() [ui.c] to display the the startup screens
            - begins resource tracking (level 2)
            - calls soft_reset() [mame.c] to reset all systems

                -------------------( at this point, we're up and running )----------------------

            - calls scheduler->timeslice() [schedule.c] over and over until we exit
            - ends resource tracking (level 2), freeing all auto_mallocs and timers
            - calls the nvram_save() [machine/generic.c] to save NVRAM
            - calls config_save_settings() [config.c] to save the game's configuration
            - calls all registered exit routines [mame.c]
            - ends resource tracking (level 1), freeing all auto_mallocs and timers

        - exits the program

***************************************************************************/

#include "emu.h"
#include "emuopts.h"
#include "osdepend.h"
#include "config.h"
#include "debugger.h"
#include "image.h"
#include "profiler.h"
#include "render.h"
#include "cheat.h"
#include "ui.h"
#include "uimenu.h"
#include "uiinput.h"
#include "streams.h"
#include "crsshair.h"
#include "validity.h"
#include "debug/debugcon.h"

#include <time.h>



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

/* the current options */
static core_options *mame_opts;

/* started empty? */
static bool started_empty;

static running_machine *global_machine;

/* output channels */
static output_callback_func output_cb[OUTPUT_CHANNEL_COUNT];
static void *output_cb_param[OUTPUT_CHANNEL_COUNT];

/* the "disclaimer" that should be printed when run with no parameters */
const char mame_disclaimer[] =
	"MAME is an emulator: it reproduces, more or less faithfully, the behaviour of\n"
	"several arcade machines. But hardware is useless without software, so an image\n"
	"of the ROMs which run on that hardware is required. Such ROMs, like any other\n"
	"commercial software, are copyrighted material and it is therefore illegal to\n"
	"use them if you don't own the original arcade machine. Needless to say, ROMs\n"
	"are not distributed together with MAME. Distribution of MAME together with ROM\n"
	"images is a violation of copyright law and should be promptly reported to the\n"
	"authors so that appropriate legal action can be taken.\n";



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static int parse_ini_file(core_options *options, const char *name, int priority);



/***************************************************************************
    CORE IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    mame_is_valid_machine - return true if the
    given machine is valid
-------------------------------------------------*/

int mame_is_valid_machine(running_machine *machine)
{
	return (machine != NULL && machine == global_machine);
}


/*-------------------------------------------------
    mame_execute - run the core emulation
-------------------------------------------------*/

int mame_execute(core_options *options)
{
	bool firstgame = true;
	bool firstrun = true;

	// loop across multiple hard resets
	bool exit_pending = false;
	int error = MAMERR_NONE;
	while (error == MAMERR_NONE && !exit_pending)
	{
		// specify the global mame_options
		mame_opts = options;

		// convert the specified gamename to a driver
		astring gamename;
		core_filename_extract_base(&gamename, options_get_string(options, OPTION_GAMENAME), true);
		const game_driver *driver = driver_get_name(gamename);

		// if no driver, use the internal empty driver
		if (driver == NULL)
		{
			driver = &GAME_NAME(empty);
			if (firstgame)
				started_empty = true;
		}

		// otherwise, perform validity checks before anything else
		else if (mame_validitychecks(driver) != 0)
			return MAMERR_FAILED_VALIDITY;

		firstgame = false;

		// parse any INI files as the first thing
		if (options_get_bool(options, OPTION_READCONFIG))
		{
			options_revert(options, OPTION_PRIORITY_INI);
			mame_parse_ini_files(options, driver);
		}

		// create the machine configuration
		const machine_config *config = global_alloc(machine_config(driver->machine_config));

		// create the machine structure and driver
		running_machine *machine = global_alloc(running_machine(*driver, *config, *options, started_empty));

		// looooong term: remove this
		global_machine = machine;

		// run the machine
		error = machine->run(firstrun);
		firstrun = false;

		// check the state of the machine
		if (machine->new_driver_pending())
		{
			options_set_string(options, OPTION_GAMENAME, machine->new_driver_name(), OPTION_PRIORITY_CMDLINE);
			firstrun = true;
		}
		if (machine->exit_pending())
			exit_pending = true;

		// destroy the machine and the config
		global_free(machine);
		global_free(config);
		global_machine = NULL;

		// reset the options
		mame_opts = NULL;
	}

	// return an error
	return error;
}


/*-------------------------------------------------
    mame_options - accesses the options for the
    currently running emulation
-------------------------------------------------*/

core_options *mame_options(void)
{
	assert(mame_opts != NULL);
	return mame_opts;
}



/***************************************************************************
    OUTPUT MANAGEMENT
***************************************************************************/

/*-------------------------------------------------
    mame_set_output_channel - configure an output
    channel
-------------------------------------------------*/

void mame_set_output_channel(output_channel channel, output_callback_func callback, void *param, output_callback_func *prevcb, void **prevparam)
{
	assert(channel < OUTPUT_CHANNEL_COUNT);
	assert(callback != NULL);

	/* return the originals if requested */
	if (prevcb != NULL)
		*prevcb = output_cb[channel];
	if (prevparam != NULL)
		*prevparam = output_cb_param[channel];

	/* set the new ones */
	output_cb[channel] = callback;
	output_cb_param[channel] = param;
}


/*-------------------------------------------------
    mame_file_output_callback - default callback
    for file output
-------------------------------------------------*/

void mame_file_output_callback(void *param, const char *format, va_list argptr)
{
	vfprintf((FILE *)param, format, argptr);
}


/*-------------------------------------------------
    mame_null_output_callback - default callback
    for no output
-------------------------------------------------*/

void mame_null_output_callback(void *param, const char *format, va_list argptr)
{
}


/*-------------------------------------------------
    mame_printf_error - output an error to the
    appropriate callback
-------------------------------------------------*/

void mame_printf_error(const char *format, ...)
{
	va_list argptr;

	/* by default, we go to stderr */
	if (output_cb[OUTPUT_CHANNEL_ERROR] == NULL)
	{
		output_cb[OUTPUT_CHANNEL_ERROR] = mame_file_output_callback;
		output_cb_param[OUTPUT_CHANNEL_ERROR] = stderr;
	}

	/* do the output */
	va_start(argptr, format);
	(*output_cb[OUTPUT_CHANNEL_ERROR])(output_cb_param[OUTPUT_CHANNEL_ERROR], format, argptr);
	va_end(argptr);
}


/*-------------------------------------------------
    mame_printf_warning - output a warning to the
    appropriate callback
-------------------------------------------------*/

void mame_printf_warning(const char *format, ...)
{
	va_list argptr;

	/* by default, we go to stderr */
	if (output_cb[OUTPUT_CHANNEL_WARNING] == NULL)
	{
		output_cb[OUTPUT_CHANNEL_WARNING] = mame_file_output_callback;
		output_cb_param[OUTPUT_CHANNEL_WARNING] = stderr;
	}

	/* do the output */
	va_start(argptr, format);
	(*output_cb[OUTPUT_CHANNEL_WARNING])(output_cb_param[OUTPUT_CHANNEL_WARNING], format, argptr);
	va_end(argptr);
}


/*-------------------------------------------------
    mame_printf_info - output info text to the
    appropriate callback
-------------------------------------------------*/

void mame_printf_info(const char *format, ...)
{
	va_list argptr;

	/* by default, we go to stdout */
	if (output_cb[OUTPUT_CHANNEL_INFO] == NULL)
	{
		output_cb[OUTPUT_CHANNEL_INFO] = mame_file_output_callback;
		output_cb_param[OUTPUT_CHANNEL_INFO] = stdout;
	}

	/* do the output */
	va_start(argptr, format);
	(*output_cb[OUTPUT_CHANNEL_INFO])(output_cb_param[OUTPUT_CHANNEL_INFO], format, argptr);
	va_end(argptr);
}


/*-------------------------------------------------
    mame_printf_verbose - output verbose text to
    the appropriate callback
-------------------------------------------------*/

void mame_printf_verbose(const char *format, ...)
{
	va_list argptr;

	/* if we're not verbose, skip it */
	if (mame_opts == NULL || !options_get_bool(mame_options(), OPTION_VERBOSE))
		return;

	/* by default, we go to stdout */
	if (output_cb[OUTPUT_CHANNEL_VERBOSE] == NULL)
	{
		output_cb[OUTPUT_CHANNEL_VERBOSE] = mame_file_output_callback;
		output_cb_param[OUTPUT_CHANNEL_VERBOSE] = stdout;
	}

	/* do the output */
	va_start(argptr, format);
	(*output_cb[OUTPUT_CHANNEL_VERBOSE])(output_cb_param[OUTPUT_CHANNEL_VERBOSE], format, argptr);
	va_end(argptr);
}


/*-------------------------------------------------
    mame_printf_debug - output debug text to the
    appropriate callback
-------------------------------------------------*/

void mame_printf_debug(const char *format, ...)
{
	va_list argptr;

	/* by default, we go to stderr */
	if (output_cb[OUTPUT_CHANNEL_DEBUG] == NULL)
	{
#ifdef MAME_DEBUG
		output_cb[OUTPUT_CHANNEL_DEBUG] = mame_file_output_callback;
		output_cb_param[OUTPUT_CHANNEL_DEBUG] = stdout;
#else
		output_cb[OUTPUT_CHANNEL_DEBUG] = mame_null_output_callback;
		output_cb_param[OUTPUT_CHANNEL_DEBUG] = NULL;
#endif
	}

	/* do the output */
	va_start(argptr, format);
	(*output_cb[OUTPUT_CHANNEL_DEBUG])(output_cb_param[OUTPUT_CHANNEL_DEBUG], format, argptr);
	va_end(argptr);
}


/*-------------------------------------------------
    mame_printf_log - output log text to the
    appropriate callback
-------------------------------------------------*/

#ifdef UNUSED_FUNCTION
void mame_printf_log(const char *format, ...)
{
	va_list argptr;

	/* by default, we go to stderr */
	if (output_cb[OUTPUT_CHANNEL_LOG] == NULL)
	{
		output_cb[OUTPUT_CHANNEL_LOG] = mame_file_output_callback;
		output_cb_param[OUTPUT_CHANNEL_LOG] = stderr;
	}

	/* do the output */
	va_start(argptr, format);
	(*output_cb[OUTPUT_CHANNEL_LOG])(output_cb_param[OUTPUT_CHANNEL_LOG], format, argptr);
	va_end(argptr);
}
#endif


/***************************************************************************
    MISCELLANEOUS
***************************************************************************/

/*-------------------------------------------------
    popmessage - pop up a user-visible message
-------------------------------------------------*/

void CLIB_DECL popmessage(const char *format, ...)
{
	// if the format is NULL, it is a signal to clear the popmessage
	if (format == NULL)
		ui_popup_time(0, " ");

	// otherwise, generate the buffer and call the UI to display the message
	else
	{
		astring temp;
		va_list arg;

		// dump to the buffer
		va_start(arg, format);
		temp.vprintf(format, arg);
		va_end(arg);

		// pop it in the UI
		ui_popup_time(temp.len() / 40 + 2, "%s", temp.cstr());
	}
}


/*-------------------------------------------------
    logerror - log to the debugger and any other
    OSD-defined output streams
-------------------------------------------------*/

void CLIB_DECL logerror(const char *format, ...)
{
	if (global_machine != NULL)
	{
		va_list arg;
		va_start(arg, format);
		global_machine->vlogerror(format, arg);
		va_end(arg);
	}
}



/***************************************************************************
    INTERNAL INITIALIZATION LOGIC
***************************************************************************/

/*-------------------------------------------------
    mame_parse_ini_files - parse the relevant INI
    files and apply their options
-------------------------------------------------*/

void mame_parse_ini_files(core_options *options, const game_driver *driver)
{
	/* parse the INI file defined by the platform (e.g., "mame.ini") */
	/* we do this twice so that the first file can change the INI path */
	parse_ini_file(options, CONFIGNAME, OPTION_PRIORITY_MAME_INI);
	parse_ini_file(options, CONFIGNAME, OPTION_PRIORITY_MAME_INI);

	/* debug mode: parse "debug.ini" as well */
	if (options_get_bool(options, OPTION_DEBUG))
		parse_ini_file(options, "debug", OPTION_PRIORITY_DEBUG_INI);

	/* if we have a valid game driver, parse game-specific INI files */
	if (driver != NULL)
	{
#ifndef MESS
		const game_driver *parent = driver_get_clone(driver);
		const game_driver *gparent = (parent != NULL) ? driver_get_clone(parent) : NULL;
		machine_config *config;

		/* parse "vertical.ini" or "horizont.ini" */
		if (driver->flags & ORIENTATION_SWAP_XY)
			parse_ini_file(options, "vertical", OPTION_PRIORITY_ORIENTATION_INI);
		else
			parse_ini_file(options, "horizont", OPTION_PRIORITY_ORIENTATION_INI);

		/* parse "vector.ini" for vector games */
		config = global_alloc(machine_config(driver->machine_config));
		for (const screen_device_config *devconfig = screen_first(*config); devconfig != NULL; devconfig = screen_next(devconfig))
			if (devconfig->screen_type() == SCREEN_TYPE_VECTOR)
			{
				parse_ini_file(options, "vector", OPTION_PRIORITY_VECTOR_INI);
				break;
			}
		global_free(config);

		/* next parse "source/<sourcefile>.ini"; if that doesn't exist, try <sourcefile>.ini */
		astring sourcename;
		core_filename_extract_base(&sourcename, driver->source_file, TRUE)->ins(0, "source" PATH_SEPARATOR);
		if (!parse_ini_file(options, sourcename, OPTION_PRIORITY_SOURCE_INI))
		{
			core_filename_extract_base(&sourcename, driver->source_file, TRUE);
			parse_ini_file(options, sourcename, OPTION_PRIORITY_SOURCE_INI);
		}

		/* then parent the grandparent, parent, and game-specific INIs */
		if (gparent != NULL)
			parse_ini_file(options, gparent->name, OPTION_PRIORITY_GPARENT_INI);
		if (parent != NULL)
			parse_ini_file(options, parent->name, OPTION_PRIORITY_PARENT_INI);
#endif	/* MESS */
		parse_ini_file(options, driver->name, OPTION_PRIORITY_DRIVER_INI);
	}
}


/*-------------------------------------------------
    parse_ini_file - parse a single INI file
-------------------------------------------------*/

static int parse_ini_file(core_options *options, const char *name, int priority)
{
	file_error filerr;
	mame_file *file;

	/* don't parse if it has been disabled */
	if (!options_get_bool(options, OPTION_READCONFIG))
		return FALSE;

	/* open the file; if we fail, that's ok */
	astring fname(name, ".ini");
	filerr = mame_fopen_options(options, SEARCHPATH_INI, fname, OPEN_FLAG_READ, &file);
	if (filerr != FILERR_NONE)
		return FALSE;
	
	/* update game name so depending callback options could be added */
	if (priority==OPTION_PRIORITY_DRIVER_INI) {		
		options_force_option_callback(options, OPTION_GAMENAME, name, priority);
	}
	
	/* parse the file and close it */
	options_parse_ini_file(options, mame_core_file(file), priority);
	mame_fclose(file);
	return TRUE;
}
