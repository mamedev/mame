/***************************************************************************

    machine.c

    Controls execution of the core MAME system.

****************************************************************************

    Copyright Aaron Giles
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name 'MAME' nor the names of its contributors may be
          used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

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
#include "uimain.h"
#include "uiinput.h"
#include "crsshair.h"
#include "validity.h"
#include "unzip.h"
#include "debug/debugcon.h"

#include <time.h>



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// a giant string buffer for temporary strings
static char giant_string_buffer[65536] = { 0 };



//**************************************************************************
//  RUNNING MACHINE
//**************************************************************************

//-------------------------------------------------
//  running_machine - constructor
//-------------------------------------------------

running_machine::running_machine(const machine_config &_config, osd_interface &osd, bool exit_to_game_select)
	: firstcpu(NULL),
	  primary_screen(NULL),
	  palette(NULL),
	  pens(NULL),
	  colortable(NULL),
	  shadow_table(NULL),
	  debug_flags(0),
	  memory_data(NULL),
	  palette_data(NULL),
	  romload_data(NULL),
	  input_port_data(NULL),
	  ui_input_data(NULL),
	  debugcpu_data(NULL),
	  generic_machine_data(NULL),
	  generic_video_data(NULL),
	  generic_audio_data(NULL),

	  m_config(_config),
	  m_system(_config.gamedrv()),
	  m_osd(osd),
	  m_regionlist(m_respool),
	  m_save(*this),
	  m_scheduler(*this),
	  m_cheat(NULL),
	  m_render(NULL),
	  m_input(NULL),
	  m_sound(NULL),
	  m_video(NULL),
	  m_tilemap(NULL),
	  m_debug_view(NULL),
	  m_current_phase(MACHINE_PHASE_PREINIT),
	  m_paused(false),
	  m_hard_reset_pending(false),
	  m_exit_pending(false),
	  m_exit_to_game_select(exit_to_game_select),
	  m_new_driver_pending(NULL),
	  m_soft_reset_timer(NULL),
	  m_rand_seed(0x9d14abd7),
      m_ui_active(false),
	  m_basename(_config.gamedrv().name),
	  m_sample_rate(_config.options().sample_rate()),
	  m_logfile(NULL),
	  m_saveload_schedule(SLS_NONE),
	  m_saveload_schedule_time(attotime::zero),
	  m_saveload_searchpath(NULL),
	  m_logerror_list(m_respool)
{
	memset(gfx, 0, sizeof(gfx));
	memset(&m_base_time, 0, sizeof(m_base_time));

	// set the machine on all devices
	device_iterator iter(root_device());
	for (device_t *device = iter.first(); device != NULL; device = iter.next())
		device->set_machine(*this);

	// find devices
	for (device_t *device = iter.first(); device != NULL; device = iter.next())
		if (dynamic_cast<cpu_device *>(device) != NULL)
		{
			firstcpu = downcast<cpu_device *>(device);
			break;
		}
	screen_device_iterator screeniter(root_device());
	primary_screen = screeniter.first();

	// fetch core options
	if (options().debug())
		debug_flags = (DEBUG_FLAG_ENABLED | DEBUG_FLAG_CALL_HOOK) | (options().debug_internal() ? 0 : DEBUG_FLAG_OSD_ENABLED);
}


//-------------------------------------------------
//  ~running_machine - destructor
//-------------------------------------------------

running_machine::~running_machine()
{
}


//-------------------------------------------------
//  describe_context - return a string describing
//  which device is currently executing and its
//  PC
//-------------------------------------------------

const char *running_machine::describe_context()
{
	device_execute_interface *executing = m_scheduler.currently_executing();
	if (executing != NULL)
	{
		cpu_device *cpu = downcast<cpu_device *>(&executing->device());
		if (cpu != NULL)
			m_context.printf("'%s' (%s)", cpu->tag(), core_i64_format(cpu->pc(), cpu->space(AS_PROGRAM)->logaddrchars(), cpu->is_octal()));
	}
	else
		m_context.cpy("(no context)");

	return m_context;
}


//-------------------------------------------------
//  start - initialize the emulated machine
//-------------------------------------------------

void running_machine::start()
{
	// initialize basic can't-fail systems here
	config_init(*this);
	m_input = auto_alloc(*this, input_manager(*this));
	output_init(*this);
	palette_init(*this);
	m_render = auto_alloc(*this, render_manager(*this));
	generic_machine_init(*this);
	generic_sound_init(*this);
	generic_video_init(*this);

	// allocate a soft_reset timer
	m_soft_reset_timer = m_scheduler.timer_alloc(timer_expired_delegate(FUNC(running_machine::soft_reset), this));

	// init the osd layer
	m_osd.init(*this);

	// create the video manager
	m_video = auto_alloc(*this, video_manager(*this));
	ui_init(*this);

	// initialize the base time (needed for doing record/playback)
	::time(&m_base_time);

	// initialize the input system and input ports for the game
	// this must be done before memory_init in order to allow specifying
	// callbacks based on input port tags
	time_t newbase = input_port_init(*this);
	if (newbase != 0)
		m_base_time = newbase;

	// intialize UI input
	ui_input_init(*this);

	// initialize the streams engine before the sound devices start
	m_sound = auto_alloc(*this, sound_manager(*this));

	// first load ROMs, then populate memory, and finally initialize CPUs
	// these operations must proceed in this order
	rom_init(*this);
	memory_init(*this);
	watchdog_init(*this);

	// allocate the gfx elements prior to device initialization
	gfx_init(*this);

	// initialize natural keyboard support
	inputx_init(*this);

	// initialize image devices
	image_init(*this);
	m_tilemap = auto_alloc(*this, tilemap_manager(*this));
	crosshair_init(*this);
	network_init(*this);

	// initialize the debugger
	if ((debug_flags & DEBUG_FLAG_ENABLED) != 0)
		debugger_init(*this);

	// call the game driver's init function
	// this is where decryption is done and memory maps are altered
	// so this location in the init order is important
	ui_set_startup_text(*this, "Initializing...", true);

	// register callbacks for the devices, then start them
	add_notifier(MACHINE_NOTIFY_RESET, machine_notify_delegate(FUNC(running_machine::reset_all_devices), this));
	add_notifier(MACHINE_NOTIFY_EXIT, machine_notify_delegate(FUNC(running_machine::stop_all_devices), this));
	save().register_presave(save_prepost_delegate(FUNC(running_machine::presave_all_devices), this));
	save().register_postload(save_prepost_delegate(FUNC(running_machine::postload_all_devices), this));
	start_all_devices();

	// if we're coming in with a savegame request, process it now
	const char *savegame = options().state();
	if (savegame[0] != 0)
		schedule_load(savegame);

	// if we're in autosave mode, schedule a load
	else if (options().autosave() && (m_system.flags & GAME_SUPPORTS_SAVE) != 0)
		schedule_load("auto");

	// set up the cheat engine
	m_cheat = auto_alloc(*this, cheat_manager(*this));

	// disallow save state registrations starting here
	m_save.allow_registration(false);
}


//-------------------------------------------------
//  add_dynamic_device - dynamically add a device
//-------------------------------------------------

device_t &running_machine::add_dynamic_device(device_t &owner, device_type type, const char *tag, UINT32 clock)
{
	// add the device in a standard manner
	device_t *device = const_cast<machine_config &>(m_config).device_add(&owner, tag, type, clock);

	// notify this device and all its subdevices that they are now configured
	device_iterator iter(root_device());
	for (device_t *device = iter.first(); device != NULL; device = iter.next())
		if (!device->configured())
			device->config_complete();
	return *device;
}


//-------------------------------------------------
//  run - execute the machine
//-------------------------------------------------

int running_machine::run(bool firstrun)
{
	int error = MAMERR_NONE;

	// use try/catch for deep error recovery
	try
	{
		// move to the init phase
		m_current_phase = MACHINE_PHASE_INIT;

		// if we have a logfile, set up the callback
		if (options().log())
		{
			m_logfile = auto_alloc(*this, emu_file(OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS));
			file_error filerr = m_logfile->open("error.log");
			assert_always(filerr == FILERR_NONE, "unable to open log file");
			add_logerror_callback(logfile_callback);
		}

		// then finish setting up our local machine
		start();

		// load the configuration settings and NVRAM
		bool settingsloaded = config_load_settings(*this);
		nvram_load(*this);
		sound().ui_mute(false);

		// display the startup screens
		ui_display_startup_screens(*this, firstrun, !settingsloaded);

		// perform a soft reset -- this takes us to the running phase
		soft_reset();

		// run the CPUs until a reset or exit
		m_hard_reset_pending = false;
		while ((!m_hard_reset_pending && !m_exit_pending) || m_saveload_schedule != SLS_NONE)
		{
			g_profiler.start(PROFILER_EXTRA);

			// execute CPUs if not paused
			if (!m_paused)
				m_scheduler.timeslice();

			// otherwise, just pump video updates through
			else
				m_video->frame_update();

			// handle save/load
			if (m_saveload_schedule != SLS_NONE)
				handle_saveload();

			g_profiler.stop();
		}

		// and out via the exit phase
		m_current_phase = MACHINE_PHASE_EXIT;

		// save the NVRAM and configuration
		sound().ui_mute(true);
		nvram_save(*this);
		config_save_settings(*this);
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

	// make sure our phase is set properly before cleaning up,
	// in case we got here via exception
	m_current_phase = MACHINE_PHASE_EXIT;

	// call all exit callbacks registered
	call_notifiers(MACHINE_NOTIFY_EXIT);
	zip_file_cache_clear();

	// close the logfile
	auto_free(*this, m_logfile);
	return error;
}


//-------------------------------------------------
//  schedule_exit - schedule a clean exit
//-------------------------------------------------

void running_machine::schedule_exit()
{
	// if we are in-game but we started with the select game menu, return to that instead
	if (m_exit_to_game_select && options().system_name()[0] != 0)
	{
		options().set_system_name("");
		ui_menu_force_game_select(*this, &render().ui_container());
	}

	// otherwise, exit for real
	else
		m_exit_pending = true;

	// if we're executing, abort out immediately
	m_scheduler.eat_all_cycles();

	// if we're autosaving on exit, schedule a save as well
	if (options().autosave() && (m_system.flags & GAME_SUPPORTS_SAVE) && this->time() > attotime::zero)
		schedule_save("auto");
}


//-------------------------------------------------
//  schedule_hard_reset - schedule a hard-reset of
//  the machine
//-------------------------------------------------

void running_machine::schedule_hard_reset()
{
	m_hard_reset_pending = true;

	// if we're executing, abort out immediately
	m_scheduler.eat_all_cycles();
}


//-------------------------------------------------
//  schedule_soft_reset - schedule a soft-reset of
//  the system
//-------------------------------------------------

void running_machine::schedule_soft_reset()
{
	m_soft_reset_timer->adjust(attotime::zero);

	// we can't be paused since the timer needs to fire
	resume();

	// if we're executing, abort out immediately
	m_scheduler.eat_all_cycles();
}


//-------------------------------------------------
//  schedule_new_driver - schedule a new game to
//  be loaded
//-------------------------------------------------

void running_machine::schedule_new_driver(const game_driver &driver)
{
	m_hard_reset_pending = true;
	m_new_driver_pending = &driver;

	// if we're executing, abort out immediately
	m_scheduler.eat_all_cycles();
}


//-------------------------------------------------
//  set_saveload_filename - specifies the filename
//  for state loading/saving
//-------------------------------------------------

void running_machine::set_saveload_filename(const char *filename)
{
	// free any existing request and allocate a copy of the requested name
	if (osd_is_absolute_path(filename))
	{
		m_saveload_searchpath = NULL;
		m_saveload_pending_file.cpy(filename);
	}
	else
	{
		m_saveload_searchpath = options().state_directory();
		m_saveload_pending_file.cpy(basename()).cat(PATH_SEPARATOR).cat(filename).cat(".sta");
	}
}


//-------------------------------------------------
//  schedule_save - schedule a save to occur as
//  soon as possible
//-------------------------------------------------

void running_machine::schedule_save(const char *filename)
{
	// specify the filename to save or load
	set_saveload_filename(filename);

	// note the start time and set a timer for the next timeslice to actually schedule it
	m_saveload_schedule = SLS_SAVE;
	m_saveload_schedule_time = this->time();

	// we can't be paused since we need to clear out anonymous timers
	resume();
}


//-------------------------------------------------
//  schedule_load - schedule a load to occur as
//  soon as possible
//-------------------------------------------------

void running_machine::schedule_load(const char *filename)
{
	// specify the filename to save or load
	set_saveload_filename(filename);

	// note the start time and set a timer for the next timeslice to actually schedule it
	m_saveload_schedule = SLS_LOAD;
	m_saveload_schedule_time = this->time();

	// we can't be paused since we need to clear out anonymous timers
	resume();
}


//-------------------------------------------------
//  pause - pause the system
//-------------------------------------------------

void running_machine::pause()
{
	// ignore if nothing has changed
	if (m_paused)
		return;
	m_paused = true;

	// call the callbacks
	call_notifiers(MACHINE_NOTIFY_PAUSE);
}


//-------------------------------------------------
//  resume - resume the system
//-------------------------------------------------

void running_machine::resume()
{
	// ignore if nothing has changed
	if (!m_paused)
		return;
	m_paused = false;

	// call the callbacks
	call_notifiers(MACHINE_NOTIFY_RESUME);
}


//-------------------------------------------------
//  region_alloc - allocates memory for a region
//-------------------------------------------------

memory_region *running_machine::region_alloc(const char *name, UINT32 length, UINT8 width, endianness_t endian)
{
mame_printf_verbose("Region '%s' created\n", name);
    // make sure we don't have a region of the same name; also find the end of the list
    memory_region *info = m_regionlist.find(name);
    if (info != NULL)
		fatalerror("region_alloc called with duplicate region name \"%s\"\n", name);

	// allocate the region
	return &m_regionlist.append(name, *auto_alloc(*this, memory_region(*this, name, length, width, endian)));
}


//-------------------------------------------------
//  region_free - releases memory for a region
//-------------------------------------------------

void running_machine::region_free(const char *name)
{
	m_regionlist.remove(name);
}


//-------------------------------------------------
//  add_notifier - add a notifier of the
//  given type
//-------------------------------------------------

void running_machine::add_notifier(machine_notification event, machine_notify_delegate callback)
{
	assert_always(m_current_phase == MACHINE_PHASE_INIT, "Can only call add_notifier at init time!");

	// exit notifiers are added to the head, and executed in reverse order
	if (event == MACHINE_NOTIFY_EXIT)
		m_notifier_list[event].prepend(*global_alloc(notifier_callback_item(callback)));

	// all other notifiers are added to the tail, and executed in the order registered
	else
		m_notifier_list[event].append(*global_alloc(notifier_callback_item(callback)));
}


//-------------------------------------------------
//  add_logerror_callback - adds a callback to be
//  called on logerror()
//-------------------------------------------------

void running_machine::add_logerror_callback(logerror_callback callback)
{
	assert_always(m_current_phase == MACHINE_PHASE_INIT, "Can only call add_logerror_callback at init time!");
	m_logerror_list.append(*auto_alloc(*this, logerror_callback_item(callback)));
}


//-------------------------------------------------
//  logerror - printf-style error logging
//-------------------------------------------------

void CLIB_DECL running_machine::logerror(const char *format, ...)
{
	// process only if there is a target
	if (m_logerror_list.first() != NULL)
	{
		va_list arg;
		va_start(arg, format);
		vlogerror(format, arg);
		va_end(arg);
	}
}


//-------------------------------------------------
//  vlogerror - vprintf-style error logging
//-------------------------------------------------

void CLIB_DECL running_machine::vlogerror(const char *format, va_list args)
{
	// process only if there is a target
	if (m_logerror_list.first() != NULL)
	{
		g_profiler.start(PROFILER_LOGERROR);

		// dump to the buffer
		vsnprintf(giant_string_buffer, ARRAY_LENGTH(giant_string_buffer), format, args);

		// log to all callbacks
		for (logerror_callback_item *cb = m_logerror_list.first(); cb != NULL; cb = cb->next())
			(*cb->m_func)(*this, giant_string_buffer);

		g_profiler.stop();
	}
}


//-------------------------------------------------
//  base_datetime - retrieve the time of the host
//  system; useful for RTC implementations
//-------------------------------------------------

void running_machine::base_datetime(system_time &systime)
{
	systime.set(m_base_time);
}


//-------------------------------------------------
//  current_datetime - retrieve the current time
//  (offset by the base); useful for RTC
//  implementations
//-------------------------------------------------

void running_machine::current_datetime(system_time &systime)
{
	systime.set(m_base_time + this->time().seconds);
}


//-------------------------------------------------
//  rand - standardized random numbers
//-------------------------------------------------

UINT32 running_machine::rand()
{
	m_rand_seed = 1664525 * m_rand_seed + 1013904223;

	// return rotated by 16 bits; the low bits have a short period
    // and are frequently used
	return (m_rand_seed >> 16) | (m_rand_seed << 16);
}


//-------------------------------------------------
//  call_notifiers - call notifiers of the given
//  type
//-------------------------------------------------

void running_machine::call_notifiers(machine_notification which)
{
	for (notifier_callback_item *cb = m_notifier_list[which].first(); cb != NULL; cb = cb->next())
		cb->m_func();
}


//-------------------------------------------------
//  handle_saveload - attempt to perform a save
//  or load
//-------------------------------------------------

void running_machine::handle_saveload()
{
	UINT32 openflags = (m_saveload_schedule == SLS_LOAD) ? OPEN_FLAG_READ : (OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
	const char *opnamed = (m_saveload_schedule == SLS_LOAD) ? "loaded" : "saved";
	const char *opname = (m_saveload_schedule == SLS_LOAD) ? "load" : "save";
	file_error filerr = FILERR_NONE;

	// if no name, bail
	emu_file file(m_saveload_searchpath, openflags);
	if (!m_saveload_pending_file)
		goto cancel;

	// if there are anonymous timers, we can't save just yet, and we can't load yet either
	// because the timers might overwrite data we have loaded
	if (!m_scheduler.can_save())
	{
		// if more than a second has passed, we're probably screwed
		if ((this->time() - m_saveload_schedule_time) > attotime::from_seconds(1))
		{
			popmessage("Unable to %s due to pending anonymous timers. See error.log for details.", opname);
			goto cancel;
		}
		return;
	}

	// open the file
	filerr = file.open(m_saveload_pending_file);
	if (filerr == FILERR_NONE)
	{
		// read/write the save state
		save_error saverr = (m_saveload_schedule == SLS_LOAD) ? m_save.read_file(file) : m_save.write_file(file);

		// handle the result
		switch (saverr)
		{
			case STATERR_ILLEGAL_REGISTRATIONS:
				popmessage("Error: Unable to %s state due to illegal registrations. See error.log for details.", opname);
				break;

			case STATERR_INVALID_HEADER:
				popmessage("Error: Unable to %s state due to an invalid header. Make sure the save state is correct for this game.", opname);
				break;

			case STATERR_READ_ERROR:
				popmessage("Error: Unable to %s state due to a read error (file is likely corrupt).", opname);
				break;

			case STATERR_WRITE_ERROR:
				popmessage("Error: Unable to %s state due to a write error. Verify there is enough disk space.", opname);
				break;

			case STATERR_NONE:
				if (!(m_system.flags & GAME_SUPPORTS_SAVE))
					popmessage("State successfully %s.\nWarning: Save states are not officially supported for this game.", opnamed);
				else
					popmessage("State successfully %s.", opnamed);
				break;

			default:
				popmessage("Error: Unknown error during state %s.", opnamed);
				break;
		}

		// close and perhaps delete the file
		if (saverr != STATERR_NONE && m_saveload_schedule == SLS_SAVE)
			file.remove_on_close();
	}
	else
		popmessage("Error: Failed to open file for %s operation.", opname);

	// unschedule the operation
cancel:
	m_saveload_pending_file.reset();
	m_saveload_searchpath = NULL;
	m_saveload_schedule = SLS_NONE;
}


//-------------------------------------------------
//  soft_reset - actually perform a soft-reset
//  of the system
//-------------------------------------------------

void running_machine::soft_reset(void *ptr, INT32 param)
{
	logerror("Soft reset\n");

	// temporarily in the reset phase
	m_current_phase = MACHINE_PHASE_RESET;

	// call all registered reset callbacks
	call_notifiers(MACHINE_NOTIFY_RESET);

	// now we're running
	m_current_phase = MACHINE_PHASE_RUNNING;
}


//-------------------------------------------------
//  logfile_callback - callback for logging to
//  logfile
//-------------------------------------------------

void running_machine::logfile_callback(running_machine &machine, const char *buffer)
{
	if (machine.m_logfile != NULL)
		machine.m_logfile->puts(buffer);
}


//-------------------------------------------------
//  start_all_devices - start any unstarted devices
//-------------------------------------------------

void running_machine::start_all_devices()
{
	// iterate through the devices
	int last_failed_starts = -1;
	while (last_failed_starts != 0)
	{
		// iterate over all devices
		int failed_starts = 0;
		device_iterator iter(root_device());
		for (device_t *device = iter.first(); device != NULL; device = iter.next())
			if (!device->started())
			{
				// attempt to start the device, catching any expected exceptions
				try
				{
					// if the device doesn't have a machine yet, set it first
					if (device->m_machine == NULL)
						device->set_machine(*this);

					// now start the device
					mame_printf_verbose("Starting %s '%s'\n", device->name(), device->tag());
					device->start();
				}

				// handle missing dependencies by moving the device to the end
				catch (device_missing_dependencies &)
				{
					// if we're the end, fail
					mame_printf_verbose("  (missing dependencies; rescheduling)\n");
					failed_starts++;
				}
			}

		// each iteration should reduce the number of failed starts; error if
		// this doesn't happen
		if (failed_starts == last_failed_starts)
			throw emu_fatalerror("Circular dependency in device startup!");
		last_failed_starts = failed_starts;
	}
}


//-------------------------------------------------
//  reset_all_devices - reset all devices in the
//  hierarchy
//-------------------------------------------------

void running_machine::reset_all_devices()
{
	// reset the root and it will reset children
	root_device().reset();
}


//-------------------------------------------------
//  stop_all_devices - stop all the devices in the
//  hierarchy
//-------------------------------------------------

void running_machine::stop_all_devices()
{
	// first let the debugger save comments
	if ((debug_flags & DEBUG_FLAG_ENABLED) != 0)
		debug_comment_save(*this);

	// iterate over devices and stop them
	device_iterator iter(root_device());
	for (device_t *device = iter.first(); device != NULL; device = iter.next())
		device->stop();

	// then nuke the device tree
//  global_free(m_root_device);
}


//-------------------------------------------------
//  presave_all_devices - tell all the devices we
//  are about to save
//-------------------------------------------------

void running_machine::presave_all_devices()
{
	device_iterator iter(root_device());
	for (device_t *device = iter.first(); device != NULL; device = iter.next())
		device->pre_save();
}


//-------------------------------------------------
//  postload_all_devices - tell all the devices we
//  just completed a load
//-------------------------------------------------

void running_machine::postload_all_devices()
{
	device_iterator iter(root_device());
	for (device_t *device = iter.first(); device != NULL; device = iter.next())
		device->post_load();
}



/***************************************************************************
    MEMORY REGIONS
***************************************************************************/

//-------------------------------------------------
//  memory_region - constructor
//-------------------------------------------------

memory_region::memory_region(running_machine &machine, const char *name, UINT32 length, UINT8 width, endianness_t endian)
	: m_machine(machine),
	  m_next(NULL),
	  m_name(name),
	  m_length(length),
	  m_width(width),
	  m_endianness(endian)
{
	assert(width == 1 || width == 2 || width == 4 || width == 8);
	m_base.u8 = auto_alloc_array(machine, UINT8, length);
}


//-------------------------------------------------
//  ~memory_region - destructor
//-------------------------------------------------

memory_region::~memory_region()
{
	auto_free(machine(), m_base.v);
}



//**************************************************************************
//  CALLBACK ITEMS
//**************************************************************************

//-------------------------------------------------
//  notifier_callback_item - constructor
//-------------------------------------------------

running_machine::notifier_callback_item::notifier_callback_item(machine_notify_delegate func)
	: m_next(NULL),
	  m_func(func)
{
}


//-------------------------------------------------
//  logerror_callback_item - constructor
//-------------------------------------------------

running_machine::logerror_callback_item::logerror_callback_item(logerror_callback func)
	: m_next(NULL),
	  m_func(func)
{
}



//**************************************************************************
//  DRIVER DEVICE
//**************************************************************************

//-------------------------------------------------
//  driver_device - constructor
//-------------------------------------------------

driver_device::driver_device(const machine_config &mconfig, device_type type, const char *tag)
	: device_t(mconfig, type, "Driver Device", tag, NULL, 0),
	  m_system(NULL),
	  m_palette_init(NULL),
	  m_generic_paletteram_8(*this, "paletteram"),
	  m_generic_paletteram2_8(*this, "paletteram2"),
	  m_generic_paletteram_16(*this, "paletteram"),
	  m_generic_paletteram2_16(*this, "paletteram2"),
	  m_generic_paletteram_32(*this, "paletteram"),
	  m_generic_paletteram2_32(*this, "paletteram2")
{
	memset(m_callbacks, 0, sizeof(m_callbacks));
}


//-------------------------------------------------
//  driver_device - destructor
//-------------------------------------------------

driver_device::~driver_device()
{
}


//-------------------------------------------------
//  static_set_game - set the game in the device
//  configuration
//-------------------------------------------------

void driver_device::static_set_game(device_t &device, const game_driver &game)
{
	driver_device &driver = downcast<driver_device &>(device);

	// set the system
	driver.m_system = &game;

	// set the short name to the game's name
	driver.m_shortname = game.name;

	// set the full name to the game's description
	driver.m_name = game.description;

	// and set the search path to include all parents
	driver.m_searchpath = game.name;
	for (int parent = driver_list::clone(game); parent != -1; parent = driver_list::clone(parent))
		driver.m_searchpath.cat(";").cat(driver_list::driver(parent).name);
}


//-------------------------------------------------
//  static_set_machine_start - set the legacy
//  machine start callback in the device
//  configuration
//-------------------------------------------------

void driver_device::static_set_callback(device_t &device, callback_type type, legacy_callback_func callback)
{
	downcast<driver_device &>(device).m_callbacks[type] = callback;
}


//-------------------------------------------------
//  static_set_palette_init - set the legacy
//  palette init callback in the device
//  configuration
//-------------------------------------------------

void driver_device::static_set_palette_init(device_t &device, palette_init_func callback)
{
	downcast<driver_device &>(device).m_palette_init = callback;
}


//-------------------------------------------------
//  driver_start - default implementation which
//  does nothing
//-------------------------------------------------

void driver_device::driver_start()
{
}


//-------------------------------------------------
//  machine_start - default implementation which
//  calls to the legacy machine_start function
//-------------------------------------------------

void driver_device::machine_start()
{
	if (m_callbacks[CB_MACHINE_START] != NULL)
		(*m_callbacks[CB_MACHINE_START])(machine());
}


//-------------------------------------------------
//  sound_start - default implementation which
//  calls to the legacy sound_start function
//-------------------------------------------------

void driver_device::sound_start()
{
	if (m_callbacks[CB_SOUND_START] != NULL)
		(*m_callbacks[CB_SOUND_START])(machine());
}


//-------------------------------------------------
//  video_start - default implementation which
//  calls to the legacy video_start function
//-------------------------------------------------

void driver_device::video_start()
{
	if (m_callbacks[CB_VIDEO_START] != NULL)
		(*m_callbacks[CB_VIDEO_START])(machine());
}


//-------------------------------------------------
//  driver_reset - default implementation which
//  does nothing
//-------------------------------------------------

void driver_device::driver_reset()
{
}


//-------------------------------------------------
//  machine_reset - default implementation which
//  calls to the legacy machine_reset function
//-------------------------------------------------

void driver_device::machine_reset()
{
	if (m_callbacks[CB_MACHINE_RESET] != NULL)
		(*m_callbacks[CB_MACHINE_RESET])(machine());
}


//-------------------------------------------------
//  sound_reset - default implementation which
//  calls to the legacy sound_reset function
//-------------------------------------------------

void driver_device::sound_reset()
{
	if (m_callbacks[CB_SOUND_RESET] != NULL)
		(*m_callbacks[CB_SOUND_RESET])(machine());
}


//-------------------------------------------------
//  video_reset - default implementation which
//  calls to the legacy video_reset function
//-------------------------------------------------

void driver_device::video_reset()
{
	if (m_callbacks[CB_VIDEO_RESET] != NULL)
		(*m_callbacks[CB_VIDEO_RESET])(machine());
}


//-------------------------------------------------
//  device_rom_region - return a pointer to the
//  game's ROMs
//-------------------------------------------------

const rom_entry *driver_device::device_rom_region() const
{
	return m_system->rom;
}


//-------------------------------------------------
//  device_input_ports - return a pointer to the
//  game's input ports
//-------------------------------------------------

ioport_constructor driver_device::device_input_ports() const
{
	return m_system->ipt;
}


//-------------------------------------------------
//  device_start - device override which calls
//  the various helpers
//-------------------------------------------------

void driver_device::device_start()
{
	// reschedule ourselves to be last
	device_iterator iter(*this);
	for (device_t *test = iter.first(); test != NULL; test = iter.next())
		if (test != this && !test->started())
			throw device_missing_dependencies();

	// call the game-specific init
	if (m_system->driver_init != NULL)
		(*m_system->driver_init)(machine());

	// finish image devices init process
	image_postdevice_init(machine());

	// call palette_init if present
	if (m_palette_init != NULL)
		(*m_palette_init)(machine(), machine().region("proms")->base());

	// start the various pieces
	driver_start();
	machine_start();
	sound_start();
	video_start();
}


//-------------------------------------------------
//  device_reset_after_children - device override
//  which calls the various helpers; must happen
//  after all child devices are reset
//-------------------------------------------------

void driver_device::device_reset_after_children()
{
	// reset each piece
	driver_reset();
	machine_reset();
	sound_reset();
	video_reset();
}



//**************************************************************************
//  SYSTEM TIME
//**************************************************************************

//-------------------------------------------------
//  system_time - constructor
//-------------------------------------------------

system_time::system_time()
{
	set(0);
}


//-------------------------------------------------
//  set - fills out a system_time structure
//-------------------------------------------------

void system_time::set(time_t t)
{
	time = t;
	local_time.set(*localtime(&t));
	utc_time.set(*gmtime(&t));
}


//-------------------------------------------------
//  get_tm_time - converts a tm struction to a
//  MAME mame_system_tm structure
//-------------------------------------------------

void system_time::full_time::set(struct tm &t)
{
	second	= t.tm_sec;
	minute	= t.tm_min;
	hour	= t.tm_hour;
	mday	= t.tm_mday;
	month	= t.tm_mon;
	year	= t.tm_year + 1900;
	weekday	= t.tm_wday;
	day		= t.tm_yday;
	is_dst	= t.tm_isdst;
}
