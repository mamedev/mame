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
#include "render.h"
#include "cheat.h"
#include "ui.h"
#include "uiinput.h"
#include "crsshair.h"
#include "validity.h"
#include "debug/debugcon.h"

#include <time.h>



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

/* started empty? */
static bool started_empty;

static bool print_verbose = false;

static running_machine *global_machine;

/* output channels */
static output_delegate output_cb[OUTPUT_CHANNEL_COUNT] =
{
	output_delegate(FUNC(mame_file_output_callback), stderr),	// OUTPUT_CHANNEL_ERROR
	output_delegate(FUNC(mame_file_output_callback), stderr),	// OUTPUT_CHANNEL_WARNING
	output_delegate(FUNC(mame_file_output_callback), stdout),	// OUTPUT_CHANNEL_INFO
#ifdef MAME_DEBUG
	output_delegate(FUNC(mame_file_output_callback), stdout),	// OUTPUT_CHANNEL_DEBUG
#else
	output_delegate(FUNC(mame_null_output_callback), stdout),	// OUTPUT_CHANNEL_DEBUG
#endif
	output_delegate(FUNC(mame_file_output_callback), stdout),	// OUTPUT_CHANNEL_VERBOSE
	output_delegate(FUNC(mame_file_output_callback), stdout)	// OUTPUT_CHANNEL_LOG
};



/***************************************************************************
    CORE IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    mame_is_valid_machine - return true if the
    given machine is valid
-------------------------------------------------*/

int mame_is_valid_machine(running_machine &machine)
{
	return (&machine == global_machine);
}


/*-------------------------------------------------
    mame_execute - run the core emulation
-------------------------------------------------*/

int mame_execute(emu_options &options, osd_interface &osd)
{
	bool firstgame = true;
	bool firstrun = true;

	// extract the verbose printing option
	if (options.verbose())
		print_verbose = true;

	// loop across multiple hard resets
	bool exit_pending = false;
	int error = MAMERR_NONE;
	while (error == MAMERR_NONE && !exit_pending)
	{
		// if no driver, use the internal empty driver
		const game_driver *system = options.system();
		if (system == NULL)
		{
			system = &GAME_NAME(___empty);
			if (firstgame)
				started_empty = true;
		}

		firstgame = false;

		// parse any INI files as the first thing
		if (options.read_config())
		{
			options.revert(OPTION_PRIORITY_INI);
			astring errors;
			options.parse_standard_inis(errors);
		}
		// otherwise, perform validity checks before anything else
		if (system != NULL)
		{
			validity_checker valid(options);
			valid.check_shared_source(*system);
		}

		ui_show_mouse(false);

		// create the machine configuration
		machine_config config(*system, options);

		// create the machine structure and driver
		running_machine machine(config, osd, started_empty);

		// looooong term: remove this
		global_machine = &machine;

		// run the machine
		error = machine.run(firstrun);
		firstrun = false;

		// check the state of the machine
		if (machine.new_driver_pending())
		{
			options.set_system_name(machine.new_driver_name());
			firstrun = true;
		}
		if (machine.exit_pending())
			exit_pending = true;

		// machine will go away when we exit scope
		global_machine = NULL;
	}

	// return an error
	return error;
}


/***************************************************************************
    OUTPUT MANAGEMENT
***************************************************************************/

/*-------------------------------------------------
    mame_set_output_channel - configure an output
    channel
-------------------------------------------------*/

output_delegate mame_set_output_channel(output_channel channel, output_delegate callback)
{
	assert(channel < OUTPUT_CHANNEL_COUNT);
	assert(!callback.isnull());

	/* return the originals if requested */
	output_delegate prevcb = output_cb[channel];

	/* set the new ones */
	output_cb[channel] = callback;
	return prevcb;
}


/*-------------------------------------------------
    mame_file_output_callback - default callback
    for file output
-------------------------------------------------*/

void mame_file_output_callback(FILE *param, const char *format, va_list argptr)
{
	vfprintf(param, format, argptr);
}


/*-------------------------------------------------
    mame_null_output_callback - default callback
    for no output
-------------------------------------------------*/

void mame_null_output_callback(FILE *param, const char *format, va_list argptr)
{
}


/*-------------------------------------------------
    mame_printf_error - output an error to the
    appropriate callback
-------------------------------------------------*/

void mame_printf_error(const char *format, ...)
{
	va_list argptr;

	/* do the output */
	va_start(argptr, format);
	output_cb[OUTPUT_CHANNEL_ERROR](format, argptr);
	va_end(argptr);
}


/*-------------------------------------------------
    mame_printf_warning - output a warning to the
    appropriate callback
-------------------------------------------------*/

void mame_printf_warning(const char *format, ...)
{
	va_list argptr;

	/* do the output */
	va_start(argptr, format);
	output_cb[OUTPUT_CHANNEL_WARNING](format, argptr);
	va_end(argptr);
}


/*-------------------------------------------------
    mame_printf_info - output info text to the
    appropriate callback
-------------------------------------------------*/

void mame_printf_info(const char *format, ...)
{
	va_list argptr;

	/* do the output */
	va_start(argptr, format);
	output_cb[OUTPUT_CHANNEL_INFO](format, argptr);
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
	if (!print_verbose)
		return;

	/* do the output */
	va_start(argptr, format);
	output_cb[OUTPUT_CHANNEL_VERBOSE](format, argptr);
	va_end(argptr);
}


/*-------------------------------------------------
    mame_printf_debug - output debug text to the
    appropriate callback
-------------------------------------------------*/

void mame_printf_debug(const char *format, ...)
{
	va_list argptr;

	/* do the output */
	va_start(argptr, format);
	output_cb[OUTPUT_CHANNEL_DEBUG](format, argptr);
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

	/* do the output */
	va_start(argptr, format);
	output_cb[OUTPUT_CHANNEL_LOG])(format, argptr);
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
	va_list arg;
	va_start(arg, format);
	vlogerror(format, arg);
	va_end(arg);
}


/*-------------------------------------------------
    vlogerror - log to the debugger and any other
    OSD-defined output streams
-------------------------------------------------*/

void CLIB_DECL vlogerror(const char *format, va_list arg)
{
	if (global_machine != NULL)
		global_machine->vlogerror(format, arg);
}

