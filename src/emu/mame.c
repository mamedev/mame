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
            - calls setjmp to prepare for deep error handling
            - begins resource tracking (level 1)
            - calls create_machine [mame.c] to initialize the Machine structure
            - calls init_machine() [mame.c]

            init_machine() [mame.c]
                - calls cpuintrf_init() [cpuintrf.c] to determine which CPUs are available
                - calls sndintrf_init() [sndintrf.c] to determine which sound chips are available
                - calls fileio_init() [fileio.c] to initialize file I/O info
                - calls config_init() [config.c] to initialize configuration system
                - calls input_init() [input.c] to initialize the input system
                - calls output_init() [output.c] to initialize the output system
                - calls state_init() [state.c] to initialize save state system
                - calls state_save_allow_registration() [state.c] to allow registrations
                - calls drawgfx_init() [drawgfx.c] to initialize rendering globals
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
                - calls cpuint_init() [cpuint.c] to initialize the CPU interrupts
                - calls the driver's DRIVER_INIT callback
                - calls device_list_start() [devintrf.c] to start any devices
                - calls video_init() [video.c] to start the video system
                - calls tilemap_init() [tilemap.c] to start the tilemap system
                - calls crosshair_init() [crsshair.c] to configure the crosshairs
                - calls sound_init() [sound.c] to start the audio system
                - calls debugger_init() [debugger.c] to set up the debugger
                - calls the driver's MACHINE_START, SOUND_START, and VIDEO_START callbacks
                - disposes of regions marked as disposable
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

#include "osdepend.h"
#include "driver.h"
#include "config.h"
#include "cheat.h"
#include "debugger.h"
#include "profiler.h"
#include "render.h"
#include "ui.h"
#include "uimenu.h"
#include "uiinput.h"
#include "deprecat.h"
#include "debug/debugcon.h"

#include <stdarg.h>
#include <setjmp.h>
#include <time.h>



/***************************************************************************
    CONSTANTS
***************************************************************************/

#define MAX_MEMORY_REGIONS		32



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _region_info region_info;
struct _region_info
{
	region_info *	next;
	astring *		name;
	UINT32			length;
	UINT32			flags;
	UINT8			padding[32 - 2 * sizeof(void *) - 2 * sizeof(UINT32)];
	UINT8			base[1];
};


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
	astring *		saveload_pending_file;
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
	void 			(*saveload_schedule_callback)(running_machine *);
	attotime		saveload_schedule_time;

	/* array of memory regions */
	region_info	*	regions[RGNCLASS_COUNT];

	/* error recovery and exiting */
	jmp_buf			fatal_error_jmpbuf;
	int				fatal_error_jmpbuf_valid;

	/* random number seed */
	UINT32			rand_seed;

	/* base time */
	time_t			base_time;
};



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

/* the active machine */
running_machine *Machine;

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



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

extern int mame_validitychecks(const game_driver *driver);

static int parse_ini_file(core_options *options, const char *name);

static running_machine *create_machine(const game_driver *driver);
static void prepare_machine(running_machine *machine);
static void destroy_machine(running_machine *machine);
static void init_machine(running_machine *machine);
static TIMER_CALLBACK( soft_reset );
static void free_callback_list(callback_item **cb);

static void saveload_init(running_machine *machine);
static void handle_save(running_machine *machine);
static void handle_load(running_machine *machine);

static void logfile_callback(running_machine *machine, const char *buffer);



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
		astring *gamename;

		/* specify the mame_options */
		mame_opts = options;

		/* convert the specified gamename to a driver */
		gamename = core_filename_extract_base(astring_alloc(), options_get_string(mame_options(), OPTION_GAMENAME), TRUE);
		driver = driver_get_name(astring_c(gamename));
		astring_free(gamename);

		/* if no driver, use the internal empty driver */
		if (driver == NULL)
		{
			driver = &driver_empty;
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
		machine = create_machine(driver);
		prepare_machine(machine);
		mame = machine->mame_data;

		/* start in the "pre-init phase" */
		mame->current_phase = MAME_PHASE_PREINIT;

		/* looooong term: remove this */
		Machine = machine;

		init_resource_tracking();

		/* use setjmp/longjmp for deep error recovery */
		mame->fatal_error_jmpbuf_valid = TRUE;
		error = setjmp(mame->fatal_error_jmpbuf);
		if (error == 0)
		{
			int settingsloaded;

			/* move to the init phase */
			mame->current_phase = MAME_PHASE_INIT;

			/* start tracking resources for real */
			begin_resource_tracking();

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

			/* display the startup screens */
			ui_display_startup_screens(machine, firstrun, !settingsloaded);
			firstrun = FALSE;

			/* start resource tracking; note that soft_reset assumes it can */
			/* call end_resource_tracking followed by begin_resource_tracking */
			/* to clear out resources allocated between resets */
			begin_resource_tracking();

			/* perform a soft reset -- this takes us to the running phase */
			soft_reset(machine, NULL, 0);

			/* run the CPUs until a reset or exit */
			mame->hard_reset_pending = FALSE;
			while ((!mame->hard_reset_pending && !mame->exit_pending) || mame->saveload_pending_file != NULL)
			{
				profiler_mark(PROFILER_EXTRA);

				/* execute CPUs if not paused */
				if (!mame->paused)
					cpuexec_timeslice(machine);

				/* otherwise, just pump video updates through */
				else
					video_frame_update(machine, FALSE);

				/* handle save/load */
				if (mame->saveload_schedule_callback)
					(*mame->saveload_schedule_callback)(machine);

				profiler_mark(PROFILER_END);
			}

			/* and out via the exit phase */
			mame->current_phase = MAME_PHASE_EXIT;

			/* stop tracking resources at this level */
			end_resource_tracking();

			/* save the NVRAM and configuration */
			nvram_save(machine);
			config_save_settings(machine);
		}
		mame->fatal_error_jmpbuf_valid = FALSE;

		/* call all exit callbacks registered */
		for (cb = mame->exit_callback_list; cb; cb = cb->next)
			(*cb->func.exit)(machine);

		/* close all inner resource tracking */
		exit_resource_tracking();

		/* close the logfile */
		if (mame->logfile != NULL)
			mame_fclose(mame->logfile);

		/* free our callback lists */
		free_callback_list(&mame->exit_callback_list);
		free_callback_list(&mame->pause_callback_list);
		free_callback_list(&mame->reset_callback_list);
		free_callback_list(&mame->frame_callback_list);

		/* grab data from the MAME structure before it goes away */
		if (mame->new_driver_pending != NULL)
		{
			options_set_string(mame_options(), OPTION_GAMENAME, mame->new_driver_pending->name, OPTION_PRIORITY_CMDLINE);
			firstrun = TRUE;
		}
		exit_pending = mame->exit_pending;

		/* destroy the machine */
		destroy_machine(machine);

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
	cb = malloc_or_die(sizeof(*cb));

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
	cb = malloc_or_die(sizeof(*cb));

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
	cb = malloc_or_die(sizeof(*cb));

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
	cb = malloc_or_die(sizeof(*cb));

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
		ui_menu_force_game_select();
	}

	/* otherwise, exit for real */
	else
		mame->exit_pending = TRUE;

	/* if we're executing, abort out immediately */
	if (cpu_getactivecpu() >= 0)
		activecpu_adjust_icount(-activecpu_get_icount() - 1);

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
	if (cpu_getactivecpu() >= 0)
		activecpu_adjust_icount(-activecpu_get_icount() - 1);
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
	if (cpu_getactivecpu() >= 0)
		activecpu_adjust_icount(-activecpu_get_icount() - 1);
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
	if (cpu_getactivecpu() >= 0)
		activecpu_adjust_icount(-activecpu_get_icount() - 1);
}


/*-------------------------------------------------
    set_saveload_filename - specifies the filename
    for state loading/saving
-------------------------------------------------*/

static void set_saveload_filename(running_machine *machine, const char *filename)
{
	mame_private *mame = machine->mame_data;

	/* free any existing request and allocate a copy of the requested name */
	if (mame->saveload_pending_file != NULL)
		astring_free(mame->saveload_pending_file);

	if (osd_is_absolute_path(filename))
	{
		mame->saveload_searchpath = NULL;
		mame->saveload_pending_file = astring_dupc(filename);
	}
	else
	{
		mame->saveload_searchpath = SEARCHPATH_STATE;
		mame->saveload_pending_file = astring_assemble_4(astring_alloc(), machine->basename, PATH_SEPARATOR, filename, ".sta");
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
	mame->saveload_schedule_time = timer_get_time();

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
	mame->saveload_schedule_time = timer_get_time();

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
	return (mame->saveload_pending_file != NULL);
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
    memory_region_alloc - allocates memory for a
    region
-------------------------------------------------*/

UINT8 *memory_region_alloc(running_machine *machine, int rgnclass, const char *name, UINT32 length, UINT32 flags)
{
	mame_private *mame = machine->mame_data;
	region_info *info;
    
    assert(rgnclass < RGNCLASS_COUNT);

    /* make sure we don't have a region of the same name */
    for (info = mame->regions[rgnclass]; info != NULL; info = info->next)
    	if (astring_cmpc(info->name, name) == 0)
    		fatalerror("memory_region_alloc called with duplicate region type %d name \"%s\"\n", rgnclass, name);

	/* allocate the region */
	info = malloc_or_die(sizeof(*info) + length);
	info->next = mame->regions[rgnclass];
	info->name = astring_dupc(name);
	info->length = length;
	info->flags = flags;
	
	/* hook us into the list */
	mame->regions[rgnclass] = info;
	return info->base;
}


/*-------------------------------------------------
    memory_region_free - releases memory for a
    region
-------------------------------------------------*/

void memory_region_free(running_machine *machine, int rgnclass, const char *name)
{
	mame_private *mame = machine->mame_data;
	region_info **infoptr;
	
    assert(rgnclass < RGNCLASS_COUNT);

	/* find the region */
	for (infoptr = &mame->regions[rgnclass]; *infoptr != NULL; infoptr = &(*infoptr)->next)
		if (astring_cmpc((*infoptr)->name, name) == 0)
		{
			region_info *deleteme = *infoptr;
			
			/* remove us from the list */
			*infoptr = deleteme->next;
			
			/* free the region */
			astring_free(deleteme->name);
			free(deleteme);
			break;
		}
}


/*-------------------------------------------------
    memory_region - returns pointer to a memory
    region
-------------------------------------------------*/

UINT8 *memory_region(running_machine *machine, int rgnclass, const char *name)
{
	mame_private *mame = machine->mame_data;
	region_info *info;
    
    assert(rgnclass < RGNCLASS_COUNT);
    
    /* NULL tag always fails */
    if (name == NULL)
    	return NULL;

    /* make sure we don't have a region of the same name */
    for (info = mame->regions[rgnclass]; info != NULL; info = info->next)
    	if (astring_cmpc(info->name, name) == 0)
    		return info->base;
    
    return NULL;
}


/*-------------------------------------------------
    memory_region_length - returns length of a
    memory region
-------------------------------------------------*/

UINT32 memory_region_length(running_machine *machine, int rgnclass, const char *name)
{
	mame_private *mame = machine->mame_data;
	region_info *info;
    
    assert(rgnclass < RGNCLASS_COUNT);

    /* NULL tag always fails */
    if (name == NULL)
    	return 0;

    /* make sure we don't have a region of the same name */
    for (info = mame->regions[rgnclass]; info != NULL; info = info->next)
    	if (astring_cmpc(info->name, name) == 0)
    		return info->length;
    
    return 0;
}


/*-------------------------------------------------
    memory_region_flags - returns flags for a
    memory region
-------------------------------------------------*/

UINT32 memory_region_flags(running_machine *machine, int rgnclass, const char *name)
{
	mame_private *mame = machine->mame_data;
	region_info *info;
    
    assert(rgnclass < RGNCLASS_COUNT);

    /* NULL tag always fails */
    if (name == NULL)
    	return 0;

    /* make sure we don't have a region of the same name */
    for (info = mame->regions[rgnclass]; info != NULL; info = info->next)
    	if (astring_cmpc(info->name, name) == 0)
    		return info->flags;
    
    return 0;
}


/*-------------------------------------------------
    memory_region_next - the name of the next 
    memory region of the same class (or the first 
    if name == NULL)
-------------------------------------------------*/

const char *memory_region_next(running_machine *machine, int rgnclass, const char *name)
{
	mame_private *mame = machine->mame_data;
	region_info *info;
    
    assert(rgnclass < RGNCLASS_COUNT);

	/* if there's nothing in this class, fail immediately */
    info = mame->regions[rgnclass];
	if (info == NULL)
		return NULL;

	/* NULL means return the first */
    if (name == NULL)
    	return astring_c(info->name);

    /* make sure we don't have a region of the same name */
    for ( ; info != NULL; info = info->next)
    	if (astring_cmpc(info->name, name) == 0)
    		return (info->next != NULL) ? astring_c(info->next->name) : NULL;
    
    return NULL;
}


/*-------------------------------------------------
    memory_region_class_name - return the name of
    the given memory region class
-------------------------------------------------*/

const char *memory_region_class_name(int rgnclass, int lowercase)
{
	switch (rgnclass)
	{
		default:				return lowercase ? "unknown" : "Unknown";
		case RGNCLASS_CPU:		return lowercase ? "cpu" : "CPU";
		case RGNCLASS_GFX:		return lowercase ? "gfx" : "Gfx";
		case RGNCLASS_SOUND:	return lowercase ? "sound" : "Sound";
		case RGNCLASS_USER:		return lowercase ? "user" : "User";
		case RGNCLASS_DISKS:	return lowercase ? "disk" : "Disk";
		case RGNCLASS_PROMS:	return lowercase ? "prom" : "PROM";
		case RGNCLASS_PLDS:		return lowercase ? "pld" : "PLD";
	}
	return NULL;
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
    fatalerror - print a message and escape back
    to the OSD layer
-------------------------------------------------*/

DECL_NORETURN static void fatalerror_common(running_machine *machine, int exitcode, const char *buffer) ATTR_NORETURN;

static void fatalerror_common(running_machine *machine, int exitcode, const char *buffer)
{
	/* output and return */
	mame_printf_error("%s\n", giant_string_buffer);

	/* break into the debugger if attached */
	osd_break_into_debugger(giant_string_buffer);

	/* longjmp back if we can; otherwise, exit */
	if (machine != NULL && machine->mame_data != NULL && machine->mame_data->fatal_error_jmpbuf_valid)
  		longjmp(machine->mame_data->fatal_error_jmpbuf, exitcode);
	else
		exit(exitcode);
}


void CLIB_DECL fatalerror(const char *text, ...)
{
	running_machine *machine = Machine;
	va_list arg;

	/* dump to the buffer; assume no one writes >2k lines this way */
	va_start(arg, text);
	vsnprintf(giant_string_buffer, GIANT_STRING_BUFFER_SIZE, text, arg);
	va_end(arg);

	fatalerror_common(machine, MAMERR_FATALERROR, giant_string_buffer);
}


void CLIB_DECL fatalerror_exitcode(int exitcode, const char *text, ...)
{
	running_machine *machine = Machine;
	va_list arg;

	/* dump to the buffer; assume no one writes >2k lines this way */
	va_start(arg, text);
	vsnprintf(giant_string_buffer, GIANT_STRING_BUFFER_SIZE, text, arg);
	va_end(arg);

	fatalerror_common(machine, exitcode, giant_string_buffer);
}


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
		vsnprintf(giant_string_buffer, GIANT_STRING_BUFFER_SIZE, format, arg);
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
	running_machine *machine = Machine;

	/* currently, we need a machine to do this */
	if (machine != NULL)
	{
		mame_private *mame = machine->mame_data;
		callback_item *cb;

		/* process only if there is a target */
		if (mame->logerror_callback_list != NULL)
		{
			va_list arg;

			profiler_mark(PROFILER_LOGERROR);

			/* dump to the buffer */
			va_start(arg, format);
			vsnprintf(giant_string_buffer, GIANT_STRING_BUFFER_SIZE, format, arg);
			va_end(arg);

			/* log to all callbacks */
			for (cb = mame->logerror_callback_list; cb; cb = cb->next)
				(*cb->func.log)(machine, giant_string_buffer);

			profiler_mark(PROFILER_END);
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

	cb = auto_malloc(sizeof(*cb));
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
    mame_find_cpu_index - return the index of the
    given CPU, or -1 if not found
-------------------------------------------------*/

int mame_find_cpu_index(running_machine *machine, const char *tag)
{
	int cpunum;

	for (cpunum = 0; cpunum < MAX_CPU; cpunum++)
		if (machine->config->cpu[cpunum].tag && strcmp(machine->config->cpu[cpunum].tag, tag) == 0)
			return cpunum;

	return -1;
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
	parse_ini_file(options, CONFIGNAME);
	parse_ini_file(options, CONFIGNAME);

	/* debug mode: parse "debug.ini" as well */
	if (options_get_bool(options, OPTION_DEBUG))
		parse_ini_file(options, "debug");

	/* if we have a valid game driver, parse game-specific INI files */
	if (driver != NULL)
	{
		const game_driver *parent = driver_get_clone(driver);
		const game_driver *gparent = (parent != NULL) ? driver_get_clone(parent) : NULL;
		const device_config *device;
		machine_config *config;
		astring *sourcename;

		/* parse "vector.ini" for vector games */
		config = machine_config_alloc(driver->machine_config);
		for (device = video_screen_first(config); device != NULL; device = video_screen_next(device))
		{
			const screen_config *scrconfig = device->inline_config;
			if (scrconfig->type == SCREEN_TYPE_VECTOR)
			{
				parse_ini_file(options, "vector");
				break;
			}
		}
		machine_config_free(config);

		/* next parse "source/<sourcefile>.ini"; if that doesn't exist, try <sourcefile>.ini */
		sourcename = core_filename_extract_base(astring_alloc(), driver->source_file, TRUE);
		astring_insc(sourcename, 0, "source" PATH_SEPARATOR);
		if (!parse_ini_file(options, astring_c(sourcename)))
		{
			core_filename_extract_base(sourcename, driver->source_file, TRUE);
			parse_ini_file(options, astring_c(sourcename));
		}
		astring_free(sourcename);

		/* then parent the grandparent, parent, and game-specific INIs */
		if (gparent != NULL)
			parse_ini_file(options, gparent->name);
		if (parent != NULL)
			parse_ini_file(options, parent->name);
		parse_ini_file(options, driver->name);
	}
}


/*-------------------------------------------------
    parse_ini_file - parse a single INI file
-------------------------------------------------*/

static int parse_ini_file(core_options *options, const char *name)
{
	file_error filerr;
	mame_file *file;
	astring *fname;

	/* don't parse if it has been disabled */
	if (!options_get_bool(options, OPTION_READCONFIG))
		return FALSE;

	/* open the file; if we fail, that's ok */
	fname = astring_assemble_2(astring_alloc(), name, ".ini");
	filerr = mame_fopen_options(options, SEARCHPATH_INI, astring_c(fname), OPEN_FLAG_READ, &file);
	astring_free(fname);
	if (filerr != FILERR_NONE)
		return FALSE;

	/* parse the file and close it */
	mame_printf_verbose("Parsing %s.ini\n", name);
	options_parse_ini_file(options, mame_core_file(file), OPTION_PRIORITY_INI);
	mame_fclose(file);
	return TRUE;
}


/*-------------------------------------------------
    create_machine - create the running machine
    object and initialize it based on options
-------------------------------------------------*/

static running_machine *create_machine(const game_driver *driver)
{
	running_machine *machine;

	/* allocate memory for the machine */
	machine = malloc(sizeof(*machine));
	if (machine == NULL)
		goto error;
	memset(machine, 0, sizeof(*machine));

	/* allocate memory for the internal mame_data */
	machine->mame_data = malloc(sizeof(*machine->mame_data));
	if (machine->mame_data == NULL)
		goto error;
	memset(machine->mame_data, 0, sizeof(*machine->mame_data));

	/* initialize the driver-related variables in the machine */
	machine->gamedrv = driver;
	machine->basename = mame_strdup(driver->name);
	machine->config = machine_config_alloc(driver->machine_config);

	/* allocate the driver data */
	if (machine->config->driver_data_size != 0)
	{
		machine->driver_data = malloc(machine->config->driver_data_size);
		if (machine->driver_data == NULL)
			goto error;
	}
	return machine;

error:
	if (machine->driver_data != NULL)
		free(machine->driver_data);
	if (machine->config != NULL)
		machine_config_free((machine_config *)machine->config);
	if (machine->mame_data != NULL)
		free(machine->mame_data);
	if (machine != NULL)
		free(machine);
	return NULL;
}


/*-------------------------------------------------
    prepare_machine - reset the state of the
    machine object
-------------------------------------------------*/

static void prepare_machine(running_machine *machine)
{
	/* reset most portions of the machine */

	/* graphics layout */
	memset(machine->gfx, 0, sizeof(machine->gfx));

	/* palette-related information */
	machine->pens = NULL;
	machine->shadow_table = NULL;

	/* audio-related information */
	machine->sample_rate = options_get_int(mame_options(), OPTION_SAMPLERATE);

	/* input-related information */
	machine->portconfig = NULL;

	/* debugger-related information */
	machine->debug_flags = options_get_bool(mame_options(), OPTION_DEBUG) ? (DEBUG_FLAG_ENABLED | DEBUG_FLAG_CALL_HOOK) : 0;

	/* reset the global MAME data and clear the other privates */
	memset(machine->mame_data, 0, sizeof(*machine->mame_data));
	machine->palette_data = NULL;
	machine->streams_data = NULL;

	/* reset the driver data */
	if (machine->config->driver_data_size != 0)
		memset(machine->driver_data, 0, machine->config->driver_data_size);
}



/*-------------------------------------------------
    destroy_machine - free the machine data
-------------------------------------------------*/

static void destroy_machine(running_machine *machine)
{
	assert(machine == Machine);
	if (machine->driver_data != NULL)
		free(machine->driver_data);
	if (machine->config != NULL)
		machine_config_free((machine_config *)machine->config);
	if (machine->mame_data != NULL)
		free(machine->mame_data);
	if (machine->basename != NULL)
		free((void *)machine->basename);
	free(machine);
	Machine = NULL;
}


/*-------------------------------------------------
    init_machine - initialize the emulated machine
-------------------------------------------------*/

static void init_machine(running_machine *machine)
{
	mame_private *mame = machine->mame_data;
	const char *rgntag, *nextrgntag;
	time_t newbase;
	int rgnclass;

	/* initialize basic can't-fail systems here */
	cpuintrf_init(machine);
	sndintrf_init(machine);
	fileio_init(machine);
	config_init(machine);
	input_init(machine);
	output_init(machine);
	state_init(machine);
	state_save_allow_registration(TRUE);
	drawgfx_init(machine);
	palette_init(machine);
	render_init(machine);
	ui_init(machine);
#ifdef MESS
	ui_mess_init(machine);
#endif /* MESS */
	generic_machine_init(machine);
	generic_video_init(machine);
	generic_sound_init();
	mame->rand_seed = 0x9d14abd7;

	/* initialize the timers and allocate a soft_reset timer */
	/* this must be done before cpu_init so that CPU's can allocate timers */
	timer_init(machine);
	mame->soft_reset_timer = timer_alloc(soft_reset, NULL);

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

	/* first load ROMs, then populate memory, and finally initialize CPUs */
	/* these operations must proceed in this order */
	rom_init(machine, machine->gamedrv->rom);
	memory_init(machine);
	cpuexec_init(machine);
	watchdog_init(machine);
	cpuint_init(machine);

#ifdef MESS
	/* first MESS initialization */
	mess_predevice_init(machine);
#endif /* MESS */

	/* call the game driver's init function */
	/* this is where decryption is done and memory maps are altered */
	/* so this location in the init order is important */
	ui_set_startup_text(machine, "Initializing...", TRUE);
	if (machine->gamedrv->driver_init != NULL)
		(*machine->gamedrv->driver_init)(machine);

	/* start up the devices */
	device_list_start(machine);

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

	/* free memory regions allocated with REGIONFLAG_DISPOSE (typically gfx roms) */
	for (rgnclass = 0; rgnclass < RGNCLASS_COUNT; rgnclass++)
		for (rgntag = memory_region_next(machine, rgnclass, NULL); rgntag != NULL; rgntag = nextrgntag)
		{
			nextrgntag = memory_region_next(machine, rgnclass, rgntag);
			if (memory_region_flags(machine, rgnclass, rgntag) & ROMREGION_DISPOSE)
				memory_region_free(machine, rgnclass, rgntag);
		}

	/* initialize miscellaneous systems */
	saveload_init(machine);
//  if (options_get_bool(mame_options(), OPTION_CHEAT))
//      cheat_init(machine);
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

	/* a bit gross -- back off of the resource tracking, and put it back at the end */
	assert(get_resource_tag() == 2);
	end_resource_tracking();
	begin_resource_tracking();

	/* allow save state registrations during the reset */
	state_save_allow_registration(TRUE);

	/* unfortunately, we can't rely on callbacks to reset the interrupt */
	/* structures, as these need to happen before we call the reset */
	/* functions registered by the drivers */
	cpuint_reset();

	/* run the driver's reset callbacks */
	if (machine->config->machine_reset != NULL)
		(*machine->config->machine_reset)(machine);
	if (machine->config->sound_reset != NULL)
		(*machine->config->sound_reset)(machine);
	if (machine->config->video_reset != NULL)
		(*machine->config->video_reset)(machine);

	/* call all registered reset callbacks */
	for (cb = machine->mame_data->reset_callback_list; cb; cb = cb->next)
		(*cb->func.reset)(machine);

	/* disallow save state registrations starting here */
	state_save_allow_registration(FALSE);

	/* now we're running */
	mame->current_phase = MAME_PHASE_RUNNING;

	/* set the global time to the current time */
	/* this allows 0-time queued callbacks to run before any CPUs execute */
	timer_set_global_time(timer_get_time());
}


/*-------------------------------------------------
    free_callback_list - free a list of callbacks
-------------------------------------------------*/

static void free_callback_list(callback_item **cb)
{
	while (*cb)
	{
		callback_item *temp = *cb;
		*cb = (*cb)->next;
		free(temp);
	}
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
	if (mame->saveload_pending_file == NULL)
	{
		mame->saveload_schedule_callback = NULL;
		return;
	}

	/* if there are anonymous timers, we can't save just yet */
	if (timer_count_anonymous() > 0)
	{
		/* if more than a second has passed, we're probably screwed */
		if (attotime_sub(timer_get_time(), mame->saveload_schedule_time).seconds > 0)
		{
			popmessage("Unable to save due to pending anonymous timers. See error.log for details.");
			goto cancel;
		}
		return;
	}

	/* open the file */
	filerr = mame_fopen(mame->saveload_searchpath, astring_c(mame->saveload_pending_file), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS, &file);
	if (filerr == FILERR_NONE)
	{
		int cpunum;

		/* write the save state */
		if (state_save_save_begin(file) != 0)
		{
			popmessage("Error: Unable to save state due to illegal registrations. See error.log for details.");
			mame_fclose(file);
			goto cancel;
		}

		/* write the default tag */
		state_save_push_tag(0);
		state_save_save_continue();
		state_save_pop_tag();

		/* loop over CPUs */
		for (cpunum = 0; cpunum < cpu_gettotalcpu(); cpunum++)
		{
			cpuintrf_push_context(cpunum);

			/* make sure banking is set */
			activecpu_reset_banking();

			/* save the CPU data */
			state_save_push_tag(cpunum + 1);
			state_save_save_continue();
			state_save_pop_tag();

			cpuintrf_pop_context();
		}

		/* finish and close */
		state_save_save_finish();
		mame_fclose(file);

		/* pop a warning if the game doesn't support saves */
		if (!(machine->gamedrv->flags & GAME_SUPPORTS_SAVE))
			popmessage("State successfully saved.\nWarning: Save states are not officially supported for this game.");
		else
			popmessage("State successfully saved.");
	}
	else
		popmessage("Error: Failed to save state");

cancel:
	/* unschedule the save */
	astring_free(mame->saveload_pending_file);
	mame->saveload_searchpath = NULL;
	mame->saveload_pending_file = NULL;
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
	if (mame->saveload_pending_file == NULL)
	{
		mame->saveload_schedule_callback = NULL;
		return;
	}

	/* if there are anonymous timers, we can't load just yet because the timers might */
	/* overwrite data we have loaded */
	if (timer_count_anonymous() > 0)
	{
		/* if more than a second has passed, we're probably screwed */
		if (attotime_sub(timer_get_time(), mame->saveload_schedule_time).seconds > 0)
		{
			popmessage("Unable to load due to pending anonymous timers. See error.log for details.");
			goto cancel;
		}
		return;
	}

	/* open the file */
	filerr = mame_fopen(mame->saveload_searchpath, astring_c(mame->saveload_pending_file), OPEN_FLAG_READ, &file);
	if (filerr == FILERR_NONE)
	{
		/* start loading */
		if (state_save_load_begin(file) == 0)
		{
			int cpunum;

			/* read tag 0 */
			state_save_push_tag(0);
			state_save_load_continue();
			state_save_pop_tag();

			/* loop over CPUs */
			for (cpunum = 0; cpunum < cpu_gettotalcpu(); cpunum++)
			{
				cpuintrf_push_context(cpunum);

				/* make sure banking is set */
				activecpu_reset_banking();

				/* load the CPU data */
				state_save_push_tag(cpunum + 1);
				state_save_load_continue();
				state_save_pop_tag();

				/* make sure banking is set */
				activecpu_reset_banking();

				cpuintrf_pop_context();
			}

			/* finish and close */
			state_save_load_finish();
			popmessage("State successfully loaded.");
		}
		else
			popmessage("Error: Failed to load state");
		mame_fclose(file);
	}
	else
		popmessage("Error: Failed to load state");

cancel:
	/* unschedule the load */
	astring_free(mame->saveload_pending_file);
	mame->saveload_pending_file = NULL;
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
	fill_systime(systime, mame->base_time + timer_get_time().seconds);
}
