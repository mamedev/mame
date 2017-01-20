// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles
/***************************************************************************

    mame.c

    Controls execution of the core MAME system.

***************************************************************************/

#include "emu.h"
#include "mame.h"
#include "emuopts.h"
#include "mameopts.h"
#include "pluginopts.h"
#include "osdepend.h"
#include "validity.h"
#include "clifront.h"
#include "language.h"
#include "luaengine.h"
#include <time.h>
#include "ui/ui.h"
#include "ui/selgame.h"
#include "ui/simpleselgame.h"
#include "cheat.h"
#include "ui/inifile.h"
#include "xmlfile.h"

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
		m_firstrun(true),
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
	if (options().console()) {
		std::string error_string;
		m_plugins->set_value("console", "1", OPTION_PRIORITY_CMDLINE, error_string);
	}

	m_lua->initialize();

	{
		emu_file file(options().plugins_path(), OPEN_FLAG_READ);
		osd_file::error filerr = file.open("boot.lua");
		if (filerr == osd_file::error::NONE)
		{
			std::string exppath;
			osd_subst_env(exppath, std::string(file.fullpath()));
			m_lua->load_script(exppath.c_str());
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
			m_options.revert(OPTION_PRIORITY_INI);
			std::string errors;
			mame_options::parse_standard_inis(m_options,errors);
		}

		// otherwise, perform validity checks before anything else
		bool is_empty = (system == &GAME_NAME(___empty));
		if (!is_empty)
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
		error = machine.run(is_empty);
		m_firstrun = false;

		// check the state of the machine
		if (m_new_driver_pending)
		{
			// set up new system name and adjust device options accordingly
			mame_options::set_system_name(m_options,m_new_driver_pending->name);
			m_firstrun = true;
		}
		else
		{
			if (machine.exit_pending()) mame_options::set_system_name(m_options,"");
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

void mame_machine_manager::create_custom(running_machine& machine)
{
	name_input_types(machine.ioport().types());

	// start the inifile manager
	m_inifile = std::make_unique<inifile_manager>(machine, m_ui->options());

	// allocate autoboot timer
	m_autoboot_timer = machine.scheduler().timer_alloc(timer_expired_delegate(FUNC(mame_machine_manager::autoboot_callback), this));

	// start favorite manager
	m_favorite = std::make_unique<favorite_manager>(machine, m_ui->options());

	// set up the cheat engine
	m_cheat = std::make_unique<cheat_manager>(machine);
}

std::string mame_machine_manager::ioport_type_to_name(ioport_type type)
{
	if (type >= IPT_START2 && type <= IPT_START10)
		return string_format(_("%d Players Start"), type - IPT_START1 + 1);
	else if (type >= IPT_COIN1 && type <= IPT_COIN12)
		return string_format(_("Coin %d"), type - IPT_COIN1 + 1);
	else if (type >= IPT_SERVICE1 && type <= IPT_SERVICE4)
		return string_format(_("Service %d"), type - IPT_SERVICE1 + 1);
	else if (type >= IPT_TILT1 && type <= IPT_TILT4)
		return string_format(_("Tilt %d"), type - IPT_TILT1 + 1);
	else if (type >= IPT_BUTTON1 && type <= IPT_BUTTON16)
		return string_format(_("Button %d"), type - IPT_BUTTON1 + 1);
	else if (type >= IPT_MAHJONG_A && type <= IPT_MAHJONG_Q)
		return string_format(_("Mahjong %c"), type - IPT_MAHJONG_A + 'A');
	else if (type >= IPT_HANAFUDA_A && type <= IPT_HANAFUDA_H)
		return string_format(_("Hanafuda %c/%d"), type - IPT_HANAFUDA_A + 'A', type - IPT_HANAFUDA_A + 1);
	else if (type >= IPT_POKER_HOLD1 && type <= IPT_POKER_HOLD5)
		return string_format(_("Hold %d"), type - IPT_POKER_HOLD1 + 1);
	else if (type >= IPT_SLOT_STOP1 && type <= IPT_SLOT_STOP4)
		return string_format(_("Stop Reel %d"), type - IPT_SLOT_STOP1 + 1);
	else if (type >= IPT_PEDAL1 && type <= IPT_PEDAL3)
		return string_format(_("Pedal %d"), type - IPT_PEDAL1 + 1);
	else switch (type)
	{
		case IPT_JOYSTICK_UP:           return _("Up");
		case IPT_JOYSTICK_DOWN:         return _("Down");
		case IPT_JOYSTICK_LEFT:         return _("Left");
		case IPT_JOYSTICK_RIGHT:        return _("Right");
		case IPT_JOYSTICKRIGHT_UP:      return _("Right Stick/Up");
		case IPT_JOYSTICKRIGHT_DOWN:    return _("Right Stick/Down");
		case IPT_JOYSTICKRIGHT_LEFT:    return _("Right Stick/Left");
		case IPT_JOYSTICKRIGHT_RIGHT:   return _("Right Stick/Right");
		case IPT_JOYSTICKLEFT_UP:       return _("Left Stick/Up");
		case IPT_JOYSTICKLEFT_DOWN:     return _("Left Stick/Down");
		case IPT_JOYSTICKLEFT_LEFT:     return _("Left Stick/Left");
		case IPT_JOYSTICKLEFT_RIGHT:    return _("Left Stick/Right");
		case IPT_START:                 return _("Start");
		case IPT_SELECT:                return _("Select");
		case IPT_MAHJONG_KAN:           return _("Mahjong Kan");
		case IPT_MAHJONG_CHI:           return _("Mahjong Chi");
		case IPT_MAHJONG_PON:           return _("Mahjong Pon");
		case IPT_MAHJONG_REACH:         return _("Mahjong Reach");
		case IPT_MAHJONG_RON:           return _("Mahjong Ron");
		case IPT_MAHJONG_BET:           return _("Mahjong Bet");
		case IPT_MAHJONG_LAST_CHANCE:   return _("Mahjong Last Chance");
		case IPT_MAHJONG_SCORE:         return _("Mahjong Score");
		case IPT_MAHJONG_DOUBLE_UP:     return _("Mahjong Double Up");
		case IPT_MAHJONG_FLIP_FLOP:     return _("Mahjong Flip Flop");
		case IPT_MAHJONG_BIG:           return _("Mahjong Big");
		case IPT_MAHJONG_SMALL:         return _("Mahjong Small");
		case IPT_HANAFUDA_YES:          return _("Hanafuda Yes");
		case IPT_HANAFUDA_NO:           return _("Hanafuda No");
		case IPT_GAMBLE_KEYIN:          return _("Key In");
		case IPT_GAMBLE_KEYOUT:         return _("Key Out");
		case IPT_GAMBLE_SERVICE:        return _("Service");
		case IPT_GAMBLE_BOOK:           return _("Book-Keeping");
		case IPT_GAMBLE_DOOR:           return _("Door");
		case IPT_GAMBLE_HIGH:           return _("High");
		case IPT_GAMBLE_LOW:            return _("Low");
		case IPT_GAMBLE_HALF:           return _("Half Gamble");
		case IPT_GAMBLE_DEAL:           return _("Deal");
		case IPT_GAMBLE_D_UP:           return _("Double Up");
		case IPT_GAMBLE_TAKE:           return _("Take");
		case IPT_GAMBLE_STAND:          return _("Stand");
		case IPT_GAMBLE_BET:            return _("Bet");
		case IPT_GAMBLE_PAYOUT:         return _("Payout");
		case IPT_POKER_CANCEL:          return _("Cancel");
		case IPT_POKER_BET:             return _("Bet");
		case IPT_SLOT_STOP_ALL:         return _("Stop All Reels");
		case IPT_START1:                return _("1 Player Start");
		case IPT_BILL1:                 return _("Bill 1");
		case IPT_POWER_ON:              return _("Power On");
		case IPT_POWER_OFF:             return _("Power Off");
		case IPT_SERVICE:               return _("Service");
		case IPT_TILT:                  return _("Tilt");
		case IPT_INTERLOCK:             return _("Door Interlock");
		case IPT_MEMORY_RESET:          return _("Memory Reset");
		case IPT_VOLUME_DOWN:           return _("Volume Down");
		case IPT_VOLUME_UP:             return _("Volume Up");
		case IPT_KEYPAD:                return _("Keypad");
		case IPT_KEYBOARD:              return _("Keyboard");
		case IPT_PADDLE:                return _("Paddle");
		case IPT_PADDLE_V:              return _("Paddle V");
		case IPT_POSITIONAL:            return _("Positional");
		case IPT_POSITIONAL_V:          return _("Positional V");
		case IPT_DIAL:                  return _("Dial");
		case IPT_DIAL_V:                return _("Dial V");
		case IPT_TRACKBALL_X:           return _("Track X");
		case IPT_TRACKBALL_Y:           return _("Track Y");
		case IPT_AD_STICK_X:            return _("AD Stick X");
		case IPT_AD_STICK_Y:            return _("AD Stick Y");
		case IPT_AD_STICK_Z:            return _("AD Stick Z");
		case IPT_LIGHTGUN_X:            return _("Lightgun X");
		case IPT_LIGHTGUN_Y:            return _("Lightgun Y");
		case IPT_MOUSE_X:               return _("Mouse X");
		case IPT_MOUSE_Y:               return _("Mouse Y");
		case IPT_UI_ON_SCREEN_DISPLAY:  return _("On Screen Display");
		case IPT_UI_DEBUG_BREAK:        return _("Break in Debugger");
		case IPT_UI_CONFIGURE:          return _("Config Menu");
		case IPT_UI_PAUSE:              return _("Pause");
		case IPT_UI_PAUSE_SINGLE:       return _("Pause - Single Step");
		case IPT_UI_RESET_MACHINE:      return _("Reset Machine");
		case IPT_UI_SOFT_RESET:         return _("Soft Reset");
		case IPT_UI_SHOW_GFX:           return _("Show Gfx");
		case IPT_UI_FRAMESKIP_DEC:      return _("Frameskip Dec");
		case IPT_UI_FRAMESKIP_INC:      return _("Frameskip Inc");
		case IPT_UI_THROTTLE:           return _("Throttle");
		case IPT_UI_FAST_FORWARD:       return _("Fast Forward");
		case IPT_UI_SHOW_FPS:           return _("Show FPS");
		case IPT_UI_SNAPSHOT:           return _("Save Snapshot");
		case IPT_UI_TIMECODE:           return _("Write current timecode");
		case IPT_UI_RECORD_MOVIE:       return _("Record Movie");
		case IPT_UI_TOGGLE_CHEAT:       return _("Toggle Cheat");
		case IPT_UI_TOGGLE_AUTOFIRE:    return _("Toggle Autofire");
		case IPT_UI_UP:                 return _("UI Up");
		case IPT_UI_DOWN:               return _("UI Down");
		case IPT_UI_LEFT:               return _("UI Left");
		case IPT_UI_RIGHT:              return _("UI Right");
		case IPT_UI_HOME:               return _("UI Home");
		case IPT_UI_END:                return _("UI End");
		case IPT_UI_PAGE_UP:            return _("UI Page Up");
		case IPT_UI_PAGE_DOWN:          return _("UI Page Down");
		case IPT_UI_SELECT:             return _("UI Select");
		case IPT_UI_CANCEL:             return _("UI Cancel");
		case IPT_UI_DISPLAY_COMMENT:    return _("UI Display Comment");
		case IPT_UI_CLEAR:              return _("UI Clear");
		case IPT_UI_ZOOM_IN:            return _("UI Zoom In");
		case IPT_UI_ZOOM_OUT:           return _("UI Zoom Out");
		case IPT_UI_PREV_GROUP:         return _("UI Previous Group");
		case IPT_UI_NEXT_GROUP:         return _("UI Next Group");
		case IPT_UI_ROTATE:             return _("UI Rotate");
		case IPT_UI_SHOW_PROFILER:      return _("Show Profiler");
		case IPT_UI_TOGGLE_UI:          return _("UI Toggle");
		case IPT_UI_PASTE:              return _("UI Paste Text");
		case IPT_UI_TOGGLE_DEBUG:       return _("Toggle Debugger");
		case IPT_UI_SAVE_STATE:         return _("Save State");
		case IPT_UI_LOAD_STATE:         return _("Load State");
		case IPT_UI_TAPE_START:         return _("UI (First) Tape Start");
		case IPT_UI_TAPE_STOP:          return _("UI (First) Tape Stop");
		case IPT_UI_DATS:               return _("UI External DAT View");
		case IPT_UI_FAVORITES:          return _("UI Add/Remove favorites");
		//case IPT_UI_UP_FILTER:
		//case IPT_UI_DOWN_FILTER:
		//case IPT_UI_LEFT_PANEL:
		//case IPT_UI_RIGHT_PANEL:
		//case IPT_UI_UP_PANEL:
		//case IPT_UI_DOWN_PANEL:
		case IPT_UI_EXPORT:             return _("UI Export list");
		case IPT_UI_AUDIT_FAST:         return _("UI Audit Unavailable");
		case IPT_UI_AUDIT_ALL:          return _("UI Audit All");
		default: return "";
	}
}

const char *mame_machine_manager::s_ipt_name_format_analog[MAX_PLAYERS + 1]
{
	__("%s"),
	__("%s"),
	__("%s 2"),
	__("%s 3"),
	__("%s 4"),
	__("%s 5"),
	__("%s 6"),
	__("%s 7"),
	__("%s 8"),
	__("%s 9"),
	__("%s 10")
};

const char *mame_machine_manager::s_ipt_name_format_digital[MAX_PLAYERS + 1]
{
	__("%s"),
	__("P1 %s"),
	__("P2 %s"),
	__("P3 %s"),
	__("P4 %s"),
	__("P5 %s"),
	__("P6 %s"),
	__("P7 %s"),
	__("P8 %s"),
	__("P9 %s"),
	__("P10 %s")
};

void mame_machine_manager::name_input_types(const simple_list<input_type_entry> &types)
{
	for (auto &entry : types)
	{
		if (entry.group() != IPG_INVALID && !entry.is_osd())
		{
			std::string ipt_name = ioport_type_to_name(entry.type());
			int player = (entry.group() >= IPG_PLAYER1 && entry.group() <= IPG_PLAYER_LAST) ? entry.player() + 1 : 0;

			// suppress "P1" string for input types only provided for one player
			if (entry.type() > IPT_GAMBLING_FIRST && entry.type() < IPT_GAMBLING_LAST)
			{
				assert(entry.player() == 0);
				player = 0;
			}

			// use different format strings for (most) analog inputs
			if (entry.is_analog() && (entry.type() < IPT_PEDAL1 || entry.type() > IPT_PEDAL3))
				entry.set_name(string_format(_(s_ipt_name_format_analog[player]), ipt_name.c_str()));
			else
				entry.set_name(string_format(_(s_ipt_name_format_digital[player]), ipt_name.c_str()));
		}
	}
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

int emulator_info::start_frontend(emu_options &options, osd_interface &osd, int argc, char *argv[])
{
	cli_frontend frontend(options, osd);
	return frontend.execute(argc, argv);
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

void emulator_info::layout_file_cb(util::xml::data_node &layout)
{
	util::xml::data_node const *const mamelayout = layout.get_child("mamelayout");
	if (mamelayout)
	{
		util::xml::data_node const *const script = mamelayout->get_child("script");
		if(script)
			mame_machine_manager::instance()->lua()->call_plugin_set("layout", script->get_value());
	}
}

bool emulator_info::standalone() { return false; }
