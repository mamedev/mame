// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles
/***************************************************************************

    mame.c

    Controls execution of the core MAME system.

***************************************************************************/

#include "emu.h"
#include "mame.h"

#include "ui/inifile.h"
#include "ui/selgame.h"
#include "ui/simpleselgame.h"
#include "ui/ui.h"

#include "cheat.h"
#include "clifront.h"
#include "emuopts.h"
#include "fileio.h"
#include "luaengine.h"
#include "mameopts.h"
#include "pluginopts.h"
#include "rendlay.h"
#include "validity.h"

#include "corestr.h"
#include "xmlfile.h"

#include "osdepend.h"

#include <ctime>


//**************************************************************************
//  MACHINE MANAGER
//**************************************************************************

mame_machine_manager *mame_machine_manager::s_manager = nullptr;

mame_machine_manager* mame_machine_manager::instance(emu_options &options, osd_interface &osd)
{
	if (!s_manager)
		s_manager = new mame_machine_manager(options, osd);

	return s_manager;
}

mame_machine_manager* mame_machine_manager::instance()
{
	return s_manager;
}

//-------------------------------------------------
//  mame_machine_manager - constructor
//-------------------------------------------------

mame_machine_manager::mame_machine_manager(emu_options &options,osd_interface &osd) :
	machine_manager(options, osd),
	m_plugins(std::make_unique<plugin_options>()),
	m_lua(std::make_unique<lua_engine>()),
	m_new_driver_pending(nullptr),
	m_firstrun(true),
	m_autoboot_timer(nullptr)
{
}


//-------------------------------------------------
//  ~mame_machine_manager - destructor
//-------------------------------------------------

mame_machine_manager::~mame_machine_manager()
{
	m_lua.reset();
	s_manager = nullptr;
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

//-------------------------------------------------
//  update_machine
//-------------------------------------------------

void mame_machine_manager::update_machine()
{
	m_lua->set_machine(m_machine);
	m_lua->attach_notifiers();
}


//-------------------------------------------------
//  split
//-------------------------------------------------

static std::vector<std::string> split(const std::string &text, char sep)
{
	std::vector<std::string> tokens;
	std::size_t start = 0, end = 0;
	while ((end = text.find(sep, start)) != std::string::npos)
	{
		std::string temp = text.substr(start, end - start);
		if (temp != "") tokens.push_back(temp);
		start = end + 1;
	}
	std::string temp = text.substr(start);
	if (temp != "") tokens.push_back(temp);
	return tokens;
}


//-------------------------------------------------
//  start_luaengine
//-------------------------------------------------

void mame_machine_manager::start_luaengine()
{
	if (options().plugins())
	{
		// scan all plugin directories
		path_iterator iter(options().plugins_path());
		std::string pluginpath;
		while (iter.next(pluginpath))
		{
			// user may specify environment variables; subsitute them
			osd_subst_env(pluginpath, pluginpath);

			// and then scan the directory recursively
			m_plugins->scan_directory(pluginpath, true);
		}

		{
			// parse the file
			// attempt to open the output file
			emu_file file(options().ini_path(), OPEN_FLAG_READ);
			if (!file.open("plugin.ini"))
			{
				try
				{
					m_plugins->parse_ini_file((util::core_file&)file);
				}
				catch (options_exception &)
				{
					osd_printf_error("**Error loading plugin.ini**\n");
				}
			}
		}

		// process includes
		for (const std::string &incl : split(options().plugin(), ','))
		{
			plugin_options::plugin *p = m_plugins->find(incl);
			if (!p)
				fatalerror("Fatal error: Could not load plugin: %s\n", incl);
			p->m_start = true;
		}

		// process excludes
		for (const std::string &excl : split(options().no_plugin(), ','))
		{
			plugin_options::plugin *p = m_plugins->find(excl);
			if (!p)
				fatalerror("Fatal error: Unknown plugin: %s\n", excl);
			p->m_start = false;
		}
	}

	// we have a special way to open the console plugin
	if (options().console())
	{
		plugin_options::plugin *p = m_plugins->find(OPTION_CONSOLE);
		if (!p)
			fatalerror("Fatal error: Console plugin not found.\n");

		p->m_start = true;
	}

	m_lua->initialize();

	{
		emu_file file(options().plugins_path(), OPEN_FLAG_READ);
		std::error_condition const filerr = file.open("boot.lua");
		if (!filerr)
		{
			std::string exppath;
			osd_subst_env(exppath, std::string(file.fullpath()));
			m_lua->load_script(exppath.c_str());
		}
	}
}


//-------------------------------------------------
//  execute - run the core emulation
//-------------------------------------------------

int mame_machine_manager::execute()
{
	bool started_empty = false;

	bool firstgame = true;

	// loop across multiple hard resets
	bool exit_pending = false;
	int error = EMU_ERR_NONE;

	while (error == EMU_ERR_NONE && !exit_pending)
	{
		m_new_driver_pending = nullptr;

		// if no driver, use the internal empty driver
		const game_driver *system = mame_options::system(m_options);
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
			// but first, revert out any potential game-specific INI settings from previous runs via the internal UI
			m_options.revert(OPTION_PRIORITY_INI);

			std::ostringstream errors;
			mame_options::parse_standard_inis(m_options, errors);
		}

		// otherwise, perform validity checks before anything else
		bool is_empty = (system == &GAME_NAME(___empty));
		if (!is_empty)
		{
			validity_checker valid(m_options, true);
			valid.set_verbose(false);
			valid.check_shared_source(*system);
		}

		// create the machine configuration
		machine_config config(*system, m_options);

		// create the machine structure and driver
		running_machine machine(config, *this);

		set_machine(&machine);

		// run the machine
		error = machine.run(is_empty);
		m_firstrun = false;

		// check the state of the machine
		if (m_new_driver_pending)
		{
			// set up new system name and adjust device options accordingly
			m_options.set_system_name(m_new_driver_pending->name);
			m_firstrun = true;
		}
		else
		{
			if (machine.exit_pending())
				m_options.set_system_name("");
		}

		if (machine.exit_pending() && (!started_empty || is_empty))
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

	machine.add_notifier(MACHINE_NOTIFY_RESET, machine_notify_delegate(&mame_machine_manager::reset, this));

	m_ui->set_startup_text("Initializing...", true);

	return m_ui.get();
}

void mame_machine_manager::ui_initialize(running_machine& machine)
{
	m_ui->initialize(machine);

	// display the startup screens
	m_ui->display_startup_screens(m_firstrun);
}

void mame_machine_manager::before_load_settings(running_machine& machine)
{
	m_lua->on_machine_before_load_settings();
}

void mame_machine_manager::create_custom(running_machine& machine)
{
	// start the inifile manager
	m_inifile = std::make_unique<inifile_manager>(m_ui->options());

	// allocate autoboot timer
	m_autoboot_timer = machine.scheduler().timer_alloc(timer_expired_delegate(FUNC(mame_machine_manager::autoboot_callback), this));

	// start favorite manager
	m_favorite = std::make_unique<favorite_manager>(m_ui->options());
}

void mame_machine_manager::load_cheatfiles(running_machine& machine)
{
	// set up the cheat engine
	m_cheat = std::make_unique<cheat_manager>(machine);
}

//-------------------------------------------------
//  missing_mandatory_images - search for devices
//  which need an image to be loaded
//-------------------------------------------------

std::vector<std::reference_wrapper<const std::string>> mame_machine_manager::missing_mandatory_images()
{
	std::vector<std::reference_wrapper<const std::string>> results;
	assert(m_machine);

	// make sure that any required image has a mounted file
	for (device_image_interface &image : image_interface_enumerator(m_machine->root_device()))
	{
		if (image.must_be_loaded())
		{
			if (m_machine->options().image_option(image.instance_name()).value().empty())
			{
				// this is a missing image; give LUA plugins a chance to handle it
				if (!lua()->on_missing_mandatory_image(image.instance_name()))
					results.push_back(std::reference_wrapper<const std::string>(image.instance_name()));
			}
		}
	}
	return results;
}

const char * emulator_info::get_bare_build_version() { return bare_build_version; }
const char * emulator_info::get_build_version() { return build_version; }

void emulator_info::display_ui_chooser(running_machine& machine)
{
	// force the UI to show the game select screen
	mame_ui_manager &mui = mame_machine_manager::instance()->ui();
	render_container &container = machine.render().ui_container();
	if (machine.options().ui() == emu_options::UI_SIMPLE)
		ui::simple_menu_select_game::force_game_select(mui, container);
	else
		ui::menu_select_game::force_game_select(mui, container);
}

int emulator_info::start_frontend(emu_options &options, osd_interface &osd, std::vector<std::string> &args)
{
	cli_frontend frontend(options, osd);
	return frontend.execute(args);
}

int emulator_info::start_frontend(emu_options &options, osd_interface &osd, int argc, char *argv[])
{
	std::vector<std::string> args(argv, argv + argc);
	return start_frontend(options, osd, args);
}

void emulator_info::draw_user_interface(running_machine& machine)
{
	mame_machine_manager::instance()->ui().update_and_render(machine.render().ui_container());
}

void emulator_info::periodic_check()
{
	return mame_machine_manager::instance()->lua()->on_periodic();
}

bool emulator_info::frame_hook()
{
	return mame_machine_manager::instance()->lua()->frame_hook();
}

void emulator_info::sound_hook()
{
	return mame_machine_manager::instance()->lua()->on_sound_update();
}

void emulator_info::layout_script_cb(layout_file &file, const char *script)
{
	// TODO: come up with a better way to pass multiple arguments to plugin
	//mame_machine_manager::instance()->lua()->call_plugin_set("layout", std::make_tuple(&file, script->get_value()));
	auto &lua(mame_machine_manager::instance()->lua()->sol());
	sol::object obj = lua.registry()["cb_layout"];
	if (obj.is<sol::protected_function>())
	{
		auto res = obj.as<sol::protected_function>()(sol::make_reference(lua, &file), sol::make_reference(lua, script));
		if (!res.valid())
		{
			sol::error err = res;
			osd_printf_error("[LUA ERROR] in call_plugin: %s\n", err.what());
		}
	}
}

bool emulator_info::standalone() { return false; }
