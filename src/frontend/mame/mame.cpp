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
#include "mame.h"
#include "emuopts.h"
#include "mameopts.h"
#include "osdepend.h"
#include "validity.h"
#include "clifront.h"
#include "luaengine.h"
#include <time.h>
#include "ui/selgame.h"
#include "ui/simpleselgame.h"
#include "cheat.h"
#include "ui/datfile.h"
#include "ui/inifile.h"

//**************************************************************************
//  MACHINE MANAGER
//**************************************************************************

mame_machine_manager* mame_machine_manager::m_manager = nullptr;

mame_machine_manager* mame_machine_manager::instance(emu_options &options,osd_interface &osd)
{
	if(!m_manager)
	{
		m_manager = global_alloc(mame_machine_manager(options,osd));
	}
	return m_manager;
}

mame_machine_manager* mame_machine_manager::instance()
{
	return m_manager;
}

//-------------------------------------------------
//  mame_machine_manager - constructor
//-------------------------------------------------

mame_machine_manager::mame_machine_manager(emu_options &options,osd_interface &osd)
		: machine_manager(options, osd),
		m_plugins(std::make_unique<plugin_options>()),
		m_lua(global_alloc(lua_engine)),
		m_new_driver_pending(nullptr), 
	    m_autoboot_timer(nullptr)
	{
}


//-------------------------------------------------
//  ~mame_machine_manager - destructor
//-------------------------------------------------

mame_machine_manager::~mame_machine_manager()
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

void mame_machine_manager::schedule_new_driver(const game_driver &driver)
{
	m_new_driver_pending = &driver;
}


/***************************************************************************
    CORE IMPLEMENTATION
***************************************************************************/
void mame_machine_manager::update_machine()
{
	m_lua->set_machine(m_machine);
	m_lua->attach_notifiers();
}


std::vector<std::string> split(const std::string &text, char sep)
{
	std::vector<std::string> tokens;
	std::size_t start = 0, end = 0;
	while ((end = text.find(sep, start)) != std::string::npos) {
		std::string temp = text.substr(start, end - start);
		if (temp != "") tokens.push_back(temp);
		start = end + 1;
	}
	std::string temp = text.substr(start);
	if (temp != "") tokens.push_back(temp);
	return tokens;
}

void mame_machine_manager::start_luaengine()
{
	if (options().plugins())
	{
		path_iterator iter(options().plugins_path());
		std::string pluginpath;
		while (iter.next(pluginpath))
		{
			m_plugins->parse_json(pluginpath);
		}
		std::vector<std::string> include = split(options().plugin(),',');
		std::vector<std::string> exclude = split(options().no_plugin(),',');
		{
			// parse the file
			std::string error;
			// attempt to open the output file
			emu_file file(options().ini_path(), OPEN_FLAG_READ);
			if (file.open("plugin.ini") == osd_file::error::NONE)
			{
				bool result = m_plugins->parse_ini_file((util::core_file&)file, OPTION_PRIORITY_MAME_INI, OPTION_PRIORITY_DRIVER_INI, error);
				if (!result)
					osd_printf_error("**Error loading plugin.ini**");
			}
		}
		for (auto &curentry : *m_plugins)
		{
			if (!curentry.is_header())
			{
				if (std::find(include.begin(), include.end(), curentry.name()) != include.end())
				{
					std::string error_string;
					m_plugins->set_value(curentry.name(), "1", OPTION_PRIORITY_CMDLINE, error_string);
				}
				if (std::find(exclude.begin(), exclude.end(), curentry.name()) != exclude.end())
				{
					std::string error_string;
					m_plugins->set_value(curentry.name(), "0", OPTION_PRIORITY_CMDLINE, error_string);
				}
			}
		}
	}
	m_lua->initialize();

	{
		emu_file file(options().plugins_path(), OPEN_FLAG_READ);
		osd_file::error filerr = file.open("boot.lua");
		if (filerr == osd_file::error::NONE)
		{
			m_lua->load_script(file.fullpath());
		}
	}
}

/*-------------------------------------------------
    execute - run the core emulation
-------------------------------------------------*/

int mame_machine_manager::execute()
{
	bool started_empty = false;

	bool firstgame = true;
	bool firstrun = true;

	// loop across multiple hard resets
	bool exit_pending = false;
	int error = EMU_ERR_NONE;

	if (m_options.console()) {
		m_lua->start_console();
	}
	while (error == EMU_ERR_NONE && !exit_pending)
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
			mame_options::parse_standard_inis(m_options,errors);
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
			// set up new system name and adjust device options accordingly
			mame_options::set_system_name(m_options,m_new_driver_pending->name);
			firstrun = true;
		}
		else
		{
			if (machine.exit_pending()) mame_options::set_system_name(m_options,"");
		}

		if (machine.exit_pending() && (!started_empty || (system == &GAME_NAME(___empty))))
			exit_pending = true;

		// machine will go away when we exit scope
		set_machine(nullptr);
	}
	// return an error
	return error;
}

TIMER_CALLBACK_MEMBER(mame_machine_manager::autoboot_callback)
{
	if (strlen(options().autoboot_script())!=0) {
		mame_machine_manager::instance()->lua()->load_script(options().autoboot_script());
	}
	else if (strlen(options().autoboot_command())!=0) {
		std::string cmd = std::string(options().autoboot_command());
		strreplace(cmd, "'", "\\'");
		std::string val = std::string("emu.keypost('").append(cmd).append("')");
		mame_machine_manager::instance()->lua()->load_string(val.c_str());
	}
}

void mame_machine_manager::reset()
{
	// setup autoboot if needed
	m_autoboot_timer->adjust(attotime(options().autoboot_delay(),0),0);
}
	
ui_manager* mame_machine_manager::create_ui(running_machine& machine)
{
	m_ui = std::make_unique<mame_ui_manager>(machine);
	m_ui->init();

	machine.add_notifier(MACHINE_NOTIFY_RESET, machine_notify_delegate(FUNC(mame_machine_manager::reset), this));
	
	// start the inifile manager
	m_inifile = std::make_unique<inifile_manager>(machine);
	
	m_ui->set_startup_text("Initializing...", true);
	// set up the cheat engine
	m_cheat = std::make_unique<cheat_manager>(machine);

	// allocate autoboot timer
	m_autoboot_timer = machine.scheduler().timer_alloc(timer_expired_delegate(FUNC(mame_machine_manager::autoboot_callback), this));

	// start datfile manager
	m_datfile = std::make_unique<datfile_manager>(machine);

	// start favorite manager
	m_favorite = std::make_unique<favorite_manager>(machine);

	return m_ui.get();
}

void mame_machine_manager::ui_initialize(running_machine& machine,bool firstrun)
{
	m_ui->initialize(machine);

	// display the startup screens
	m_ui->display_startup_screens(firstrun);
}

const char * emulator_info::get_bare_build_version() { return bare_build_version; }
const char * emulator_info::get_build_version() { return build_version; }

void emulator_info::display_ui_chooser(running_machine& machine)
{
	// force the UI to show the game select screen
	if (strcmp(machine.options().ui(), "simple") == 0) {
		ui_simple_menu_select_game::force_game_select(machine, &machine.render().ui_container());
	}
	else {
		ui_menu_select_game::force_game_select(machine, &machine.render().ui_container());
	}
}

int emulator_info::start_frontend(emu_options &options, osd_interface &osd, int argc, char *argv[])
{
	cli_frontend frontend(options, osd);
	return frontend.execute(argc, argv);
}

void emulator_info::draw_user_interface(running_machine& machine)
{
	mame_machine_manager::instance()->ui().update_and_render(&machine.render().ui_container());
}

void emulator_info::periodic_check()
{
	mame_machine_manager::instance()->lua()->periodic_check();
}

bool emulator_info::frame_hook()
{
	return mame_machine_manager::instance()->lua()->frame_hook();
}