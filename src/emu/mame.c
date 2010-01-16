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
                - calls cpuexec_init() [cpuexec.c] to initialize the CPUs
                - calls watchdog_init() [watchdog.c] to initialize the watchdog system
                - calls the driver's DRIVER_INIT callback
                - calls device_list_start() [devintrf.c] to start any devices
                - calls video_init() [video.c] to start the video system
                - calls tilemap_init() [tilemap.c] to start the tilemap system
                - calls crosshair_init() [crsshair.c] to configure the crosshairs
                - calls sound_init() [sound.c] to start the audio system
                - calls debugger_init() [debugger.c] to set up the debugger
                - calls the driver's MACHINE_START, SOUND_START, and VIDEO_START callbacks
                - calls saveload_init() [mame.c] to set up for save/load
                - calls cheat_init() [cheat.c] to initialize the cheat system

            - calls config_load_settings() [config.c] to load the configuration file
            - calls nvram_load [machine/generic.c] to load NVRAM
            - calls ui_display_startup_screens() [ui.c] to display the the startup screens
            - begins resource tracking (level 2)
            - calls soft_reset() [mame.c] to reset all systems

                -------------------( at this point, we're up and running )----------------------

            - calls cpuexec_timeslice() [cpuexec.c] over and over until we exit
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
#include "cheat.h"
#include "debugger.h"
#include "profiler.h"
#include "render.h"
#include "ui.h"
#include "uimenu.h"
#include "uiinput.h"
#include "streams.h"
#include "crsshair.h"
#include "validity.h"
#include "debug/debugcon.h"

#include <time.h>



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _callback_item callback_item;
struct _callback_item
{
	callback_item *	next;
	union
	{
		void		(*exit)(running_machine *);
		void		(*reset)(running_machine *);
		void		(*frame)(running_machine *);
		void		(*pause)(running_machine *, int);
		void		(*log)(running_machine *, const char *);
	} func;
};


/* typedef struct _mame_private mame_private; */
struct _mame_private
{
	/* system state */
	int				current_phase;
	UINT8			paused;
	UINT8			hard_reset_pending;
	UINT8			exit_pending;
	const game_driver *new_driver_pending;
	astring			saveload_pending_file;
	const char *	saveload_searchpath;
	emu_timer *		soft_reset_timer;
	mame_file *		logfile;

	/* callbacks */
	callback_item *	frame_callback_list;
	callback_item *	reset_callback_list;
	callback_item *	pause_callback_list;
	callback_item *	exit_callback_list;
	callback_item *	logerror_callback_list;

	/* load/save */
	void			(*saveload_schedule_callback)(running_machine *);
	attotime		saveload_schedule_time;

	/* list of memory regions, and a map for lookups */
	region_info	*	regionlist;
	tagmap_t<region_info *> regionmap;

	/* random number seed */
	UINT32			rand_seed;

	/* base time */
	time_t			base_time;
};



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

/* the active machine */
static running_machine *global_machine;

/* the current options */
static core_options *mame_opts;

/* started empty? */
static UINT8 started_empty;

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

/* a giant string buffer for temporary strings */
static char giant_string_buffer[65536] = { 0 };



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static int parse_ini_file(core_options *options, const char *name, int priority);

static void init_machine(running_machine *machine);
static TIMER_CALLBACK( soft_reset );

static void saveload_init(running_machine *machine);
static void handle_save(running_machine *machine);
static void handle_load(running_machine *machine);

static void logfile_callback(running_machine *machine, const char *buffer);



/***************************************************************************
    CORE IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    eat_all_cpu_cycles - eat a ton of cycles on
    all CPUs to force a quick exit
-------------------------------------------------*/

INLINE void eat_all_cpu_cycles(running_machine *machine)
{
	const device_config *cpu;

    if(machine->cpuexec_data)
		for (cpu = machine->firstcpu; cpu != NULL; cpu = cpu_next(cpu))
			cpu_eat_cycles(cpu, 1000000000);
}



/***************************************************************************
    CORE IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    mame_execute - run the core emulation
-------------------------------------------------*/

int mame_execute(core_options *options)
{
	int exit_pending = FALSE;
	int error = MAMERR_NONE;
	int firstgame = TRUE;
	int firstrun = TRUE;

	/* loop across multiple hard resets */
	while (error == MAMERR_NONE && !exit_pending)
	{
		const game_driver *driver;
		running_machine *machine;
		mame_private *mame;
		callback_item *cb;
		astring gamename;

		/* specify the mame_options */
		mame_opts = options;

		/* convert the specified gamename to a driver */
		core_filename_extract_base(&gamename, options_get_string(mame_options(), OPTION_GAMENAME), TRUE);
		driver = driver_get_name(gamename);

		/* if no driver, use the internal empty driver */
		if (driver == NULL)
		{
			driver = &GAME_NAME(empty);
			if (firstgame)
				started_empty = TRUE;
		}

		/* otherwise, perform validity checks before anything else */
		else if (mame_validitychecks(driver) != 0)
			return MAMERR_FAILED_VALIDITY;
		firstgame = FALSE;

		/* parse any INI files as the first thing */
		options_revert(mame_options(), OPTION_PRIORITY_INI);
		mame_parse_ini_files(mame_options(), driver);

		/* create the machine structure and driver */
		machine = global_alloc(running_machine(driver));
		mame = machine->mame_data;

		/* start in the "pre-init phase" */
		mame->current_phase = MAME_PHASE_PREINIT;

		/* looooong term: remove this */
		global_machine = machine;

		/* use try/catch for deep error recovery */
		try
		{
			int settingsloaded;

			/* move to the init phase */
			mame->current_phase = MAME_PHASE_INIT;

			/* if we have a logfile, set up the callback */
			mame->logerror_callback_list = NULL;
			if (options_get_bool(mame_options(), OPTION_LOG))
			{
				file_error filerr = mame_fopen(SEARCHPATH_DEBUGLOG, "error.log", OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS, &mame->logfile);
				assert_always(filerr == FILERR_NONE, "unable to open log file");
				add_logerror_callback(machine, logfile_callback);
			}

			/* then finish setting up our local machine */
			init_machine(machine);

			/* load the configuration settings and NVRAM */
			settingsloaded = config_load_settings(machine);
			nvram_load(machine);
			sound_mute(machine, FALSE);

			/* display the startup screens */
			ui_display_startup_screens(machine, firstrun, !settingsloaded);
			firstrun = FALSE;

			/* perform a soft reset -- this takes us to the running phase */
			soft_reset(machine, NULL, 0);

			/* run the CPUs until a reset or exit */
			mame->hard_reset_pending = FALSE;
			while ((!mame->hard_reset_pending && !mame->exit_pending) || mame->saveload_pending_file.len() != 0)
			{
				profiler_mark_start(PROFILER_EXTRA);

				/* execute CPUs if not paused */
				if (!mame->paused)
					cpuexec_timeslice(machine);

				/* otherwise, just pump video updates through */
				else
					video_frame_update(machine, FALSE);

				/* handle save/load */
				if (mame->saveload_schedule_callback != NULL)
					(*mame->saveload_schedule_callback)(machine);

				profiler_mark_end();
			}

			/* and out via the exit phase */
			mame->current_phase = MAME_PHASE_EXIT;

			/* save the NVRAM and configuration */
			sound_mute(machine, TRUE);
			nvram_save(machine);
			config_save_settings(machine);
		}
		catch (emu_fatalerror &fatal)
		{
			mame_printf_error("%s\n", fatal.string());
			error = MAMERR_FATALERROR;
			if (fatal.exitcode() != 0)
				error = fatal.exitcode();
		}
		catch (emu_exception &)
		{
			mame_printf_error("Caught unhandled emulator exception\n");
			error = MAMERR_FATALERROR;
		}
		catch (std::bad_alloc &)
		{
			mame_printf_error("Out of memory!\n");
			error = MAMERR_FATALERROR;
		}

		/* call all exit callbacks registered */
		for (cb = mame->exit_callback_list; cb; cb = cb->next)
			(*cb->func.exit)(machine);

		/* close the logfile */
		if (mame->logfile != NULL)
			mame_fclose(mame->logfile);

		/* grab data from the MAME structure before it goes away */
		if (mame->new_driver_pending != NULL)
		{
			options_set_string(mame_options(), OPTION_GAMENAME, mame->new_driver_pending->name, OPTION_PRIORITY_CMDLINE);
			firstrun = TRUE;
		}
		exit_pending = mame->exit_pending;

		/* destroy the machine */
		global_free(machine);

		/* reset the options */
		mame_opts = NULL;
	}

	/* return an error */
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



/*-------------------------------------------------
    mame_get_phase - return the current program
    phase
-------------------------------------------------*/

int mame_get_phase(running_machine *machine)
{
	mame_private *mame = machine->mame_data;
	return mame->current_phase;
}


/*-------------------------------------------------
    add_frame_callback - request a callback on
    frame update
-------------------------------------------------*/

void add_frame_callback(running_machine *machine, void (*callback)(running_machine *))
{
	mame_private *mame = machine->mame_data;
	callback_item *cb, **cur;

	assert_always(mame_get_phase(machine) == MAME_PHASE_INIT, "Can only call add_frame_callback at init time!");

	/* allocate memory */
	cb = auto_alloc(machine, callback_item);

	/* add us to the end of the list */
	cb->func.frame = callback;
	cb->next = NULL;
	for (cur = &mame->frame_callback_list; *cur; cur = &(*cur)->next) ;
	*cur = cb;
}


/*-------------------------------------------------
    add_reset_callback - request a callback on
    reset
-------------------------------------------------*/

void add_reset_callback(running_machine *machine, void (*callback)(running_machine *))
{
	mame_private *mame = machine->mame_data;
	callback_item *cb, **cur;

	assert_always(mame_get_phase(machine) == MAME_PHASE_INIT, "Can only call add_reset_callback at init time!");

	/* allocate memory */
	cb = auto_alloc(machine, callback_item);

	/* add us to the end of the list */
	cb->func.reset = callback;
	cb->next = NULL;
	for (cur = &mame->reset_callback_list; *cur; cur = &(*cur)->next) ;
	*cur = cb;
}


/*-------------------------------------------------
    add_pause_callback - request a callback on
    pause
-------------------------------------------------*/

void add_pause_callback(running_machine *machine, void (*callback)(running_machine *, int))
{
	mame_private *mame = machine->mame_data;
	callback_item *cb, **cur;

	assert_always(mame_get_phase(machine) == MAME_PHASE_INIT, "Can only call add_pause_callback at init time!");

	/* allocate memory */
	cb = auto_alloc(machine, callback_item);

	/* add us to the end of the list */
	cb->func.pause = callback;
	cb->next = NULL;
	for (cur = &mame->pause_callback_list; *cur; cur = &(*cur)->next) ;
	*cur = cb;
}


/*-------------------------------------------------
    add_exit_callback - request a callback on
    termination
-------------------------------------------------*/

void add_exit_callback(running_machine *machine, void (*callback)(running_machine *))
{
	mame_private *mame = machine->mame_data;
	callback_item *cb;

	assert_always(mame_get_phase(machine) == MAME_PHASE_INIT, "Can only call add_exit_callback at init time!");

	/* allocate memory */
	cb = auto_alloc(machine, callback_item);

	/* add us to the head of the list */
	cb->func.exit = callback;
	cb->next = mame->exit_callback_list;
	mame->exit_callback_list = cb;
}


/*-------------------------------------------------
    mame_frame_update - handle update tasks for a
    frame boundary
-------------------------------------------------*/

void mame_frame_update(running_machine *machine)
{
	callback_item *cb;

	/* call all registered frame callbacks */
	for (cb = machine->mame_data->frame_callback_list; cb; cb = cb->next)
		(*cb->func.frame)(machine);
}


/*-------------------------------------------------
    mame_is_valid_machine - return true if the
    given machine is valid
-------------------------------------------------*/

int mame_is_valid_machine(running_machine *machine)
{
	return (machine != NULL && machine == global_machine);
}



/***************************************************************************
    GLOBAL SYSTEM STATES
***************************************************************************/

/*-------------------------------------------------
    mame_schedule_exit - schedule a clean exit
-------------------------------------------------*/

void mame_schedule_exit(running_machine *machine)
{
	mame_private *mame = machine->mame_data;

	/* if we are in-game but we started with the select game menu, return to that instead */
	if (started_empty && options_get_string(mame_options(), OPTION_GAMENAME)[0] != 0)
	{
		options_set_string(mame_options(), OPTION_GAMENAME, "", OPTION_PRIORITY_CMDLINE);
		ui_menu_force_game_select(machine);
	}

	/* otherwise, exit for real */
	else
		mame->exit_pending = TRUE;

	/* if we're executing, abort out immediately */
	eat_all_cpu_cycles(machine);

	/* if we're autosaving on exit, schedule a save as well */
	if (options_get_bool(mame_options(), OPTION_AUTOSAVE) && (machine->gamedrv->flags & GAME_SUPPORTS_SAVE))
		mame_schedule_save(machine, "auto");
}


/*-------------------------------------------------
    mame_schedule_hard_reset - schedule a hard-
    reset of the system
-------------------------------------------------*/

void mame_schedule_hard_reset(running_machine *machine)
{
	mame_private *mame = machine->mame_data;
	mame->hard_reset_pending = TRUE;

	/* if we're executing, abort out immediately */
	eat_all_cpu_cycles(machine);
}


/*-------------------------------------------------
    mame_schedule_soft_reset - schedule a soft-
    reset of the system
-------------------------------------------------*/

void mame_schedule_soft_reset(running_machine *machine)
{
	mame_private *mame = machine->mame_data;

	timer_adjust_oneshot(mame->soft_reset_timer, attotime_zero, 0);

	/* we can't be paused since the timer needs to fire */
	mame_pause(machine, FALSE);

	/* if we're executing, abort out immediately */
	eat_all_cpu_cycles(machine);
}


/*-------------------------------------------------
    mame_schedule_new_driver - schedule a new game
    to be loaded
-------------------------------------------------*/

void mame_schedule_new_driver(running_machine *machine, const game_driver *driver)
{
	mame_private *mame = machine->mame_data;
	mame->hard_reset_pending = TRUE;
	mame->new_driver_pending = driver;

	/* if we're executing, abort out immediately */
	eat_all_cpu_cycles(machine);
}


/*-------------------------------------------------
    set_saveload_filename - specifies the filename
    for state loading/saving
-------------------------------------------------*/

static void set_saveload_filename(running_machine *machine, const char *filename)
{
	mame_private *mame = machine->mame_data;

	/* free any existing request and allocate a copy of the requested name */
	if (osd_is_absolute_path(filename))
	{
		mame->saveload_searchpath = NULL;
		mame->saveload_pending_file.cpy(filename);
	}
	else
	{
		mame->saveload_searchpath = SEARCHPATH_STATE;
		mame->saveload_pending_file.cpy(machine->basename).cat(PATH_SEPARATOR).cat(filename).cat(".sta");
	}
}


/*-------------------------------------------------
    mame_schedule_save - schedule a save to
    occur as soon as possible
-------------------------------------------------*/

void mame_schedule_save(running_machine *machine, const char *filename)
{
	mame_private *mame = machine->mame_data;

	/* specify the filename to save or load */
	set_saveload_filename(machine, filename);

	/* note the start time and set a timer for the next timeslice to actually schedule it */
	mame->saveload_schedule_callback = handle_save;
	mame->saveload_schedule_time = timer_get_time(machine);

	/* we can't be paused since we need to clear out anonymous timers */
	mame_pause(machine, FALSE);
}


/*-------------------------------------------------
    mame_schedule_load - schedule a load to
    occur as soon as possible
-------------------------------------------------*/

void mame_schedule_load(running_machine *machine, const char *filename)
{
	mame_private *mame = machine->mame_data;

	/* specify the filename to save or load */
	set_saveload_filename(machine, filename);

	/* note the start time and set a timer for the next timeslice to actually schedule it */
	mame->saveload_schedule_callback = handle_load;
	mame->saveload_schedule_time = timer_get_time(machine);

	/* we can't be paused since we need to clear out anonymous timers */
	mame_pause(machine, FALSE);
}


/*-------------------------------------------------
    mame_is_save_or_load_pending - is a save or
    load pending?
-------------------------------------------------*/

int mame_is_save_or_load_pending(running_machine *machine)
{
	/* we can't check for saveload_pending_file here because it will bypass */
	/* required UI screens if a state is queued from the command line */
	mame_private *mame = machine->mame_data;
	return (mame->saveload_pending_file.len() != 0);
}


/*-------------------------------------------------
    mame_is_scheduled_event_pending - is a
    scheduled event pending?
-------------------------------------------------*/

int mame_is_scheduled_event_pending(running_machine *machine)
{
	/* we can't check for saveload_pending_file here because it will bypass */
	/* required UI screens if a state is queued from the command line */
	mame_private *mame = machine->mame_data;
	return mame->exit_pending || mame->hard_reset_pending;
}


/*-------------------------------------------------
    mame_pause - pause or resume the system
-------------------------------------------------*/

void mame_pause(running_machine *machine, int pause)
{
	mame_private *mame = machine->mame_data;
	callback_item *cb;

	/* ignore if nothing has changed */
	if (mame->paused == pause)
		return;
	mame->paused = pause;

	/* call all registered pause callbacks */
	for (cb = mame->pause_callback_list; cb; cb = cb->next)
		(*cb->func.pause)(machine, mame->paused);
}


/*-------------------------------------------------
    mame_is_paused - the system paused?
-------------------------------------------------*/

int mame_is_paused(running_machine *machine)
{
	mame_private *mame = machine->mame_data;
	return (mame->current_phase != MAME_PHASE_RUNNING) || mame->paused;
}



/***************************************************************************
    MEMORY REGIONS
***************************************************************************/

/*-------------------------------------------------
    region_info - constructor for a memory region
-------------------------------------------------*/

region_info::region_info(running_machine *_machine, const char *_name, UINT32 _length, UINT32 _flags)
	: machine(_machine),
	  next(NULL),
	  name(_name),
	  length(_length),
	  flags(_flags)
{
	base.u8 = auto_alloc_array(_machine, UINT8, _length);
}


/*-------------------------------------------------
    ~region_info - memory region destructor
-------------------------------------------------*/

region_info::~region_info()
{
	auto_free(machine, base.v);
}


/*-------------------------------------------------
    memory_region_alloc - allocates memory for a
    region
-------------------------------------------------*/

UINT8 *memory_region_alloc(running_machine *machine, const char *name, UINT32 length, UINT32 flags)
{
	mame_private *mame = machine->mame_data;
	region_info **infoptr, *info;
	tagmap_error tagerr;

    /* make sure we don't have a region of the same name; also find the end of the list */
    for (infoptr = &mame->regionlist; *infoptr != NULL; infoptr = &(*infoptr)->next)
    	if ((*infoptr)->name.cmp(name) == 0)
    		fatalerror("memory_region_alloc called with duplicate region name \"%s\"\n", name);

	/* allocate the region */
	info = auto_alloc(machine, region_info(machine, name, length, flags));

	/* attempt to put is in the hash table */
	tagerr = mame->regionmap.add_unique_hash(name, info, FALSE);
	if (tagerr == TMERR_DUPLICATE)
	{
		region_info *match = mame->regionmap.find_hash_only(name);
		fatalerror("Memory region '%s' has same hash as tag '%s'; please change one of them", name, match->name.cstr());
	}

	/* hook us into the list */
	*infoptr = info;
	return info->base.u8;
}


/*-------------------------------------------------
    memory_region_free - releases memory for a
    region
-------------------------------------------------*/

void memory_region_free(running_machine *machine, const char *name)
{
	mame_private *mame = machine->mame_data;
	region_info **infoptr;

	/* find the region */
	for (infoptr = &mame->regionlist; *infoptr != NULL; infoptr = &(*infoptr)->next)
		if ((*infoptr)->name.cmp(name) == 0)
		{
			region_info *info = *infoptr;

			/* remove us from the list and the map */
			*infoptr = info->next;
			mame->regionmap.remove(info->name);

			/* free the region */
			auto_free(machine, info);
			break;
		}
}


/*-------------------------------------------------
    memory_region_info - return a pointer to the
    information struct for a given memory region
-------------------------------------------------*/

region_info *memory_region_info(running_machine *machine, const char *name)
{
	mame_private *mame = machine->mame_data;

    /* NULL tag always fails */
    if (name == NULL)
    	return NULL;

    /* look up the region and return the base */
    return mame->regionmap.find_hash_only(name);
}


/*-------------------------------------------------
    memory_region - returns pointer to a memory
    region
-------------------------------------------------*/

UINT8 *memory_region(running_machine *machine, const char *name)
{
	const region_info *region = machine->region(name);
	return (region != NULL) ? region->base.u8 : NULL;
}


/*-------------------------------------------------
    memory_region_length - returns length of a
    memory region
-------------------------------------------------*/

UINT32 memory_region_length(running_machine *machine, const char *name)
{
	const region_info *region = machine->region(name);
	return (region != NULL) ? region->length : NULL;
}


/*-------------------------------------------------
    memory_region_flags - returns flags for a
    memory region
-------------------------------------------------*/

UINT32 memory_region_flags(running_machine *machine, const char *name)
{
	const region_info *region = machine->region(name);
	return (region != NULL) ? region->flags : NULL;
}


/*-------------------------------------------------
    memory_region_next - the name of the next
    memory region (or the first if name == NULL)
-------------------------------------------------*/

const char *memory_region_next(running_machine *machine, const char *name)
{
	if (name == NULL)
		return (machine->mame_data->regionlist != NULL) ? machine->mame_data->regionlist->name : NULL;
	const region_info *region = machine->region(name);
	return (region != NULL && region->next != NULL) ? region->next->name.cstr() : NULL;
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
	/* if the format is NULL, it is a signal to clear the popmessage */
	if (format == NULL)
		ui_popup_time(0, " ");

	/* otherwise, generate the buffer and call the UI to display the message */
	else
	{
		va_list arg;

		/* dump to the buffer */
		va_start(arg, format);
		vsnprintf(giant_string_buffer, ARRAY_LENGTH(giant_string_buffer), format, arg);
		va_end(arg);

		/* pop it in the UI */
		ui_popup_time((int)strlen(giant_string_buffer) / 40 + 2, "%s", giant_string_buffer);
	}
}


/*-------------------------------------------------
    logerror - log to the debugger and any other
    OSD-defined output streams
-------------------------------------------------*/

void CLIB_DECL logerror(const char *format, ...)
{
	running_machine *machine = global_machine;

	/* currently, we need a machine to do this */
	if (machine != NULL)
	{
		mame_private *mame = machine->mame_data;
		callback_item *cb;

		/* process only if there is a target */
		if (mame->logerror_callback_list != NULL)
		{
			va_list arg;

			profiler_mark_start(PROFILER_LOGERROR);

			/* dump to the buffer */
			va_start(arg, format);
			vsnprintf(giant_string_buffer, ARRAY_LENGTH(giant_string_buffer), format, arg);
			va_end(arg);

			/* log to all callbacks */
			for (cb = mame->logerror_callback_list; cb; cb = cb->next)
				(*cb->func.log)(machine, giant_string_buffer);

			profiler_mark_end();
		}
	}
}


/*-------------------------------------------------
    add_logerror_callback - adds a callback to be
    called on logerror()
-------------------------------------------------*/

void add_logerror_callback(running_machine *machine, void (*callback)(running_machine *, const char *))
{
	mame_private *mame = machine->mame_data;
	callback_item *cb, **cur;

	assert_always(mame_get_phase(machine) == MAME_PHASE_INIT, "Can only call add_logerror_callback at init time!");

	cb = auto_alloc(machine, callback_item);
	cb->func.log = callback;
	cb->next = NULL;

	for (cur = &mame->logerror_callback_list; *cur; cur = &(*cur)->next) ;
	*cur = cb;
}


/*-------------------------------------------------
    logfile_callback - callback for logging to
    logfile
-------------------------------------------------*/

static void logfile_callback(running_machine *machine, const char *buffer)
{
	mame_private *mame = machine->mame_data;
	if (mame->logfile != NULL)
		mame_fputs(mame->logfile, buffer);
}


/*-------------------------------------------------
    mame_rand - standardized random numbers
-------------------------------------------------*/

UINT32 mame_rand(running_machine *machine)
{
	mame_private *mame = machine->mame_data;
	mame->rand_seed = 1664525 * mame->rand_seed + 1013904223;

	/* return rotated by 16 bits; the low bits have a short period
       and are frequently used */
	return (mame->rand_seed >> 16) | (mame->rand_seed << 16);
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
		const device_config *device;
		machine_config *config;

		/* parse "vertical.ini" or "horizont.ini" */
		if (driver->flags & ORIENTATION_SWAP_XY)
			parse_ini_file(options, "vertical", OPTION_PRIORITY_ORIENTATION_INI);
		else
			parse_ini_file(options, "horizont", OPTION_PRIORITY_ORIENTATION_INI);

		/* parse "vector.ini" for vector games */
		config = machine_config_alloc(driver->machine_config);
		for (device = video_screen_first(config); device != NULL; device = video_screen_next(device))
		{
			const screen_config *scrconfig = (const screen_config *)device->inline_config;
			if (scrconfig->type == SCREEN_TYPE_VECTOR)
			{
				parse_ini_file(options, "vector", OPTION_PRIORITY_VECTOR_INI);
				break;
			}
		}
		machine_config_free(config);

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

	/* parse the file and close it */
	mame_printf_verbose("Parsing %s.ini\n", name);
	options_parse_ini_file(options, mame_core_file(file), priority);
	mame_fclose(file);
	return TRUE;
}


/*-------------------------------------------------
    running_machine - create the running machine
    object and initialize it based on options
-------------------------------------------------*/

running_machine::running_machine(const game_driver *driver)
	: config(NULL),
	  firstcpu(NULL),
	  gamedrv(driver),
	  basename(NULL),
	  primary_screen(NULL),
	  palette(NULL),
	  pens(NULL),
	  colortable(NULL),
	  shadow_table(NULL),
	  priority_bitmap(NULL),
	  sample_rate(0),
	  debug_flags(0),
	  mame_data(NULL),
	  cpuexec_data(NULL),
	  timer_data(NULL),
	  state_data(NULL),
	  memory_data(NULL),
	  palette_data(NULL),
	  tilemap_data(NULL),
	  streams_data(NULL),
	  devices_data(NULL),
	  romload_data(NULL),
	  sound_data(NULL),
	  input_data(NULL),
	  input_port_data(NULL),
	  ui_input_data(NULL),
	  cheat_data(NULL),
	  debugcpu_data(NULL),
	  debugvw_data(NULL),
	  generic_machine_data(NULL),
	  generic_video_data(NULL),
	  generic_audio_data(NULL),
#ifdef MESS
	  images_data(NULL),
	  ui_mess_data(NULL),
#endif /* MESS */
	  driver_data(NULL)
{
	try
	{
		memset(&portlist, 0, sizeof(portlist));
		memset(gfx, 0, sizeof(gfx));
		memset(&generic, 0, sizeof(generic));

		/* allocate memory for the internal mame_data */
		mame_data = auto_alloc_clear(this, mame_private);

		/* initialize the driver-related variables in the machine */
		basename = mame_strdup(driver->name);
		config = machine_config_alloc(driver->machine_config);

		/* allocate the driver data */
		if (config->driver_data_size != 0)
			driver_data = auto_alloc_array_clear(this, UINT8, config->driver_data_size);

		/* find devices */
		firstcpu = cpu_first(config);
		primary_screen = video_screen_first(config);

		/* attach this machine to all the devices in the configuration */
		device_list_attach_machine(this);

		/* fetch core options */
		sample_rate = options_get_int(mame_options(), OPTION_SAMPLERATE);
		debug_flags = options_get_bool(mame_options(), OPTION_DEBUG) ? (DEBUG_FLAG_ENABLED | DEBUG_FLAG_CALL_HOOK) : 0;
	}
	catch (std::bad_alloc &)
	{
		if (driver_data != NULL)
			auto_free(this, driver_data);
		if (config != NULL)
			machine_config_free((machine_config *)config);
		if (mame_data != NULL)
			auto_free(this, mame_data);
	}
}


/*-------------------------------------------------
    ~running_machine - free the machine data
-------------------------------------------------*/

running_machine::~running_machine()
{
	assert(this == global_machine);

	if (config != NULL)
		machine_config_free((machine_config *)config);

	global_machine = NULL;
}


/*-------------------------------------------------
    init_machine - initialize the emulated machine
-------------------------------------------------*/

static void init_machine(running_machine *machine)
{
	mame_private *mame = machine->mame_data;
	time_t newbase;

	/* initialize basic can't-fail systems here */
	fileio_init(machine);
	config_init(machine);
	input_init(machine);
	output_init(machine);
	state_init(machine);
	state_save_allow_registration(machine, TRUE);
	palette_init(machine);
	render_init(machine);
	ui_init(machine);
#ifdef MESS
	ui_mess_init(machine);
#endif /* MESS */
	generic_machine_init(machine);
	generic_video_init(machine);
	generic_sound_init(machine);
	mame->rand_seed = 0x9d14abd7;

	/* initialize the timers and allocate a soft_reset timer */
	/* this must be done before cpu_init so that CPU's can allocate timers */
	timer_init(machine);
	mame->soft_reset_timer = timer_alloc(machine, soft_reset, NULL);

	/* init the osd layer */
	osd_init(machine);

	/* initialize the base time (needed for doing record/playback) */
	time(&mame->base_time);

	/* initialize the input system and input ports for the game */
	/* this must be done before memory_init in order to allow specifying */
	/* callbacks based on input port tags */
	newbase = input_port_init(machine, machine->gamedrv->ipt);
	if (newbase != 0)
		mame->base_time = newbase;

	/* intialize UI input */
	ui_input_init(machine);

	/* initialize the streams engine before the sound devices start */
	streams_init(machine);

	/* first load ROMs, then populate memory, and finally initialize CPUs */
	/* these operations must proceed in this order */
	rom_init(machine);
	memory_init(machine);
	cpuexec_init(machine);
	watchdog_init(machine);

	/* allocate the gfx elements prior to device initialization */
	gfx_init(machine);

#ifdef MESS
	/* first MESS initialization */
	mess_predevice_init(machine);
#endif /* MESS */

	/* start up the devices */
	device_list_start(machine);

	/* call the game driver's init function */
	/* this is where decryption is done and memory maps are altered */
	/* so this location in the init order is important */
	ui_set_startup_text(machine, "Initializing...", TRUE);
	if (machine->gamedrv->driver_init != NULL)
		(*machine->gamedrv->driver_init)(machine);

#ifdef MESS
	/* second MESS initialization */
	mess_postdevice_init(machine);
#endif /* MESS */

	/* start the video and audio hardware */
	video_init(machine);
	tilemap_init(machine);
	crosshair_init(machine);

	sound_init(machine);

	/* initialize the debugger */
	if ((machine->debug_flags & DEBUG_FLAG_ENABLED) != 0)
		debugger_init(machine);

	/* call the driver's _START callbacks */
	if (machine->config->machine_start != NULL)
		(*machine->config->machine_start)(machine);
	if (machine->config->sound_start != NULL)
		(*machine->config->sound_start)(machine);
	if (machine->config->video_start != NULL)
		(*machine->config->video_start)(machine);

	/* initialize miscellaneous systems */
	saveload_init(machine);
	if (options_get_bool(mame_options(), OPTION_CHEAT))
		cheat_init(machine);

	/* disallow save state registrations starting here */
	state_save_allow_registration(machine, FALSE);
}


/*-------------------------------------------------
    soft_reset - actually perform a soft-reset
    of the system
-------------------------------------------------*/

static TIMER_CALLBACK( soft_reset )
{
	mame_private *mame = machine->mame_data;
	callback_item *cb;

	logerror("Soft reset\n");

	/* temporarily in the reset phase */
	mame->current_phase = MAME_PHASE_RESET;

	/* call all registered reset callbacks */
	for (cb = machine->mame_data->reset_callback_list; cb; cb = cb->next)
		(*cb->func.reset)(machine);

	/* run the driver's reset callbacks */
	if (machine->config->machine_reset != NULL)
		(*machine->config->machine_reset)(machine);
	if (machine->config->sound_reset != NULL)
		(*machine->config->sound_reset)(machine);
	if (machine->config->video_reset != NULL)
		(*machine->config->video_reset)(machine);

	/* now we're running */
	mame->current_phase = MAME_PHASE_RUNNING;

	/* allow 0-time queued callbacks to run before any CPUs execute */
	timer_execute_timers(machine);
}



/***************************************************************************
    SAVE/RESTORE
***************************************************************************/

/*-------------------------------------------------
    saveload_init - initialize the save/load logic
-------------------------------------------------*/

static void saveload_init(running_machine *machine)
{
	const char *savegame = options_get_string(mame_options(), OPTION_STATE);

	/* if we're coming in with a savegame request, process it now */
	if (savegame[0] != 0)
		mame_schedule_load(machine, savegame);

	/* if we're in autosave mode, schedule a load */
	else if (options_get_bool(mame_options(), OPTION_AUTOSAVE) && (machine->gamedrv->flags & GAME_SUPPORTS_SAVE))
		mame_schedule_load(machine, "auto");
}


/*-------------------------------------------------
    handle_save - attempt to perform a save
-------------------------------------------------*/

static void handle_save(running_machine *machine)
{
	mame_private *mame = machine->mame_data;
	file_error filerr;
	mame_file *file;

	/* if no name, bail */
	if (mame->saveload_pending_file.len() == 0)
	{
		mame->saveload_schedule_callback = NULL;
		return;
	}

	/* if there are anonymous timers, we can't save just yet */
	if (timer_count_anonymous(machine) > 0)
	{
		/* if more than a second has passed, we're probably screwed */
		if (attotime_sub(timer_get_time(machine), mame->saveload_schedule_time).seconds > 0)
		{
			popmessage("Unable to save due to pending anonymous timers. See error.log for details.");
			goto cancel;
		}
		return;
	}

	/* open the file */
	filerr = mame_fopen(mame->saveload_searchpath, mame->saveload_pending_file, OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS, &file);
	if (filerr == FILERR_NONE)
	{
		astring fullname(mame_file_full_name(file));
		state_save_error staterr;

		/* write the save state */
		staterr = state_save_write_file(machine, file);

		/* handle the result */
		switch (staterr)
		{
			case STATERR_ILLEGAL_REGISTRATIONS:
				popmessage("Error: Unable to save state due to illegal registrations. See error.log for details.");
				break;

			case STATERR_WRITE_ERROR:
				popmessage("Error: Unable to save state due to a write error. Verify there is enough disk space.");
				break;

			case STATERR_NONE:
				if (!(machine->gamedrv->flags & GAME_SUPPORTS_SAVE))
					popmessage("State successfully saved.\nWarning: Save states are not officially supported for this game.");
				else
					popmessage("State successfully saved.");
				break;

			default:
				popmessage("Error: Unknwon error during state save.");
				break;
		}

		/* close and perhaps delete the file */
		mame_fclose(file);
		if (staterr != STATERR_NONE)
			osd_rmfile(fullname);
	}
	else
		popmessage("Error: Failed to create save state file.");

	/* unschedule the save */
cancel:
	mame->saveload_pending_file.reset();
	mame->saveload_searchpath = NULL;
	mame->saveload_schedule_callback = NULL;
}


/*-------------------------------------------------
    handle_load - attempt to perform a load
-------------------------------------------------*/

static void handle_load(running_machine *machine)
{
	mame_private *mame = machine->mame_data;
	file_error filerr;
	mame_file *file;

	/* if no name, bail */
	if (mame->saveload_pending_file.len() == 0)
	{
		mame->saveload_schedule_callback = NULL;
		return;
	}

	/* if there are anonymous timers, we can't load just yet because the timers might */
	/* overwrite data we have loaded */
	if (timer_count_anonymous(machine) > 0)
	{
		/* if more than a second has passed, we're probably screwed */
		if (attotime_sub(timer_get_time(machine), mame->saveload_schedule_time).seconds > 0)
		{
			popmessage("Unable to load due to pending anonymous timers. See error.log for details.");
			goto cancel;
		}
		return;
	}

	/* open the file */
	filerr = mame_fopen(mame->saveload_searchpath, mame->saveload_pending_file, OPEN_FLAG_READ, &file);
	if (filerr == FILERR_NONE)
	{
		state_save_error staterr;

		/* write the save state */
		staterr = state_save_read_file(machine, file);

		/* handle the result */
		switch (staterr)
		{
			case STATERR_ILLEGAL_REGISTRATIONS:
				popmessage("Error: Unable to load state due to illegal registrations. See error.log for details.");
				break;

			case STATERR_INVALID_HEADER:
				popmessage("Error: Unable to load state due to an invalid header. Make sure the save state is correct for this game.");
				break;

			case STATERR_READ_ERROR:
				popmessage("Error: Unable to load state due to a read error (file is likely corrupt).");
				break;

			case STATERR_NONE:
				if (!(machine->gamedrv->flags & GAME_SUPPORTS_SAVE))
					popmessage("State successfully loaded.\nWarning: Save states are not officially supported for this game.");
				else
					popmessage("State successfully loaded.");
				break;

			default:
				popmessage("Error: Unknwon error during state load.");
				break;
		}

		/* close the file */
		mame_fclose(file);
	}
	else
		popmessage("Error: Failed to open save state file.");

	/* unschedule the load */
cancel:
	mame->saveload_pending_file.reset();
	mame->saveload_schedule_callback = NULL;
}



/***************************************************************************
    SYSTEM TIME
***************************************************************************/

/*-------------------------------------------------
    get_tm_time - converts a MAME
-------------------------------------------------*/

static void get_tm_time(struct tm *t, mame_system_tm *systm)
{
	systm->second	= t->tm_sec;
	systm->minute	= t->tm_min;
	systm->hour		= t->tm_hour;
	systm->mday		= t->tm_mday;
	systm->month	= t->tm_mon;
	systm->year		= t->tm_year + 1900;
	systm->weekday	= t->tm_wday;
	systm->day		= t->tm_yday;
	systm->is_dst	= t->tm_isdst;
}


/*-------------------------------------------------
    fill_systime - fills out a mame_system_time
    structure
-------------------------------------------------*/

static void fill_systime(mame_system_time *systime, time_t t)
{
	systime->time = t;
	get_tm_time(localtime(&t), &systime->local_time);
	get_tm_time(gmtime(&t), &systime->utc_time);
}


/*-------------------------------------------------
    mame_get_base_datetime - retrieve the time of
    the host system; useful for RTC implementations
-------------------------------------------------*/

void mame_get_base_datetime(running_machine *machine, mame_system_time *systime)
{
	mame_private *mame = machine->mame_data;
	fill_systime(systime, mame->base_time);
}


/*-------------------------------------------------
    mame_get_current_datetime - retrieve the current
    time (offsetted by the baes); useful for RTC
    implementations
-------------------------------------------------*/

void mame_get_current_datetime(running_machine *machine, mame_system_time *systime)
{
	mame_private *mame = machine->mame_data;
	fill_systime(systime, mame->base_time + timer_get_time(machine).seconds);
}
