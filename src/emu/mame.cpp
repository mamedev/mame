// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles
/***************************************************************************

    mame.c

    Controls execution of the core MAME system.
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
            - calls ui_display_startup_screens() [ui.c] to display the startup screens
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
#include "validity.h"
#include "luaengine.h"
#include <time.h>

//**************************************************************************
//  MACHINE MANAGER
//**************************************************************************

machine_manager* machine_manager::m_manager = nullptr;

osd_interface &machine_manager::osd() const
{
	return m_osd;
}


machine_manager* machine_manager::instance(emu_options &options,osd_interface &osd)
{
	if(!m_manager)
	{
		m_manager = global_alloc(machine_manager(options,osd));
	}
	return m_manager;
}

machine_manager* machine_manager::instance()
{
	return m_manager;
}

//-------------------------------------------------
//  machine_manager - constructor
//-------------------------------------------------

machine_manager::machine_manager(emu_options &options,osd_interface &osd)
		: m_osd(osd),
		m_options(options),
		m_lua(global_alloc(lua_engine)),
		m_new_driver_pending(nullptr),
		m_machine(nullptr)
{
}


//-------------------------------------------------
//  ~machine_manager - destructor
//-------------------------------------------------

machine_manager::~machine_manager()
{
	global_free(m_lua);
	m_manager = nullptr;
}


/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

//-------------------------------------------------
//  mame_schedule_new_driver - schedule a new game to
//  be loaded
//-------------------------------------------------

void machine_manager::schedule_new_driver(const game_driver &driver)
{
	m_new_driver_pending = &driver;
}


/***************************************************************************
    CORE IMPLEMENTATION
***************************************************************************/
void machine_manager::update_machine()
{
	m_lua->set_machine(m_machine);
}

/*-------------------------------------------------
    execute - run the core emulation
-------------------------------------------------*/

int machine_manager::execute()
{
	bool started_empty = false;

	bool firstgame = true;
	bool firstrun = true;

	// loop across multiple hard resets
	bool exit_pending = false;
	int error = MAMERR_NONE;

	m_lua->initialize();
	if (m_options.console()) {
		m_lua->start_console();
	}
	while (error == MAMERR_NONE && !exit_pending)
	{
		m_new_driver_pending = nullptr;

		// if no driver, use the internal empty driver
		const game_driver *system = m_options.system();
		if (system == nullptr)
		{
			system = &GAME_NAME(___empty);
			if (firstgame)
				started_empty = true;
		}

		firstgame = false;

		// parse any INI files as the first thing
		if (m_options.read_config())
		{
			m_options.revert(OPTION_PRIORITY_INI);
			std::string errors;
			m_options.parse_standard_inis(errors);
		}

		// otherwise, perform validity checks before anything else
		if (system != nullptr)
		{
			validity_checker valid(m_options);
			valid.set_verbose(false);
			valid.check_shared_source(*system);
		}

		// create the machine configuration
		machine_config config(*system, m_options);

		// create the machine structure and driver
		running_machine machine(config, *this);

		set_machine(&machine);

		// run the machine
		error = machine.run(firstrun);
		firstrun = false;

		// check the state of the machine
		if (m_new_driver_pending)
		{
			std::string old_system_name(m_options.system_name());
			bool new_system = (old_system_name.compare(m_new_driver_pending->name)!=0);
			// first: if we scheduled a new system, remove device options of the old system
			// notice that, if we relaunch the same system, there is no effect on the emulation
			if (new_system)
				m_options.remove_device_options();
			// second: set up new system name (and the related device options)
			m_options.set_system_name(m_new_driver_pending->name);
			// third: if we scheduled a new system, take also care of ramsize options
			if (new_system)
			{
				std::string error_string;
				m_options.set_value(OPTION_RAMSIZE, "", OPTION_PRIORITY_CMDLINE, error_string);
			}
			firstrun = true;
			if (m_options.software_name())
			{
				std::string sw_load(m_options.software_name());
				std::string sw_list, sw_name, sw_part, sw_instance, option_errors, error_string;
				int left = sw_load.find_first_of(':');
				int middle = sw_load.find_first_of(':', left + 1);
				int right = sw_load.find_last_of(':');

				sw_list = sw_load.substr(0, left - 1);
				sw_name = sw_load.substr(left + 1, middle - left - 1);
				sw_part = sw_load.substr(middle + 1, right - middle - 1);
				sw_instance = sw_load.substr(right + 1);
				sw_load.assign(sw_load.substr(0, right));

				char arg[] = "ume";
				char *argv = &arg[0];
				m_options.set_value(OPTION_SOFTWARENAME, sw_name.c_str(), OPTION_PRIORITY_CMDLINE, error_string);
				m_options.parse_slot_devices(1, &argv, option_errors, sw_instance.c_str(), sw_load.c_str());
			}
		}
		else
		{
			if (machine.exit_pending()) m_options.set_system_name("");
		}

		if (machine.exit_pending() && (!started_empty || (system == &GAME_NAME(___empty))))
			exit_pending = true;

		// machine will go away when we exit scope
		set_machine(nullptr);
	}
	// return an error
	return error;
}
