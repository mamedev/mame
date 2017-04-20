// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    emuopts.cpp

    Options file and command line management.

***************************************************************************/

#include "emu.h"
#include "emuopts.h"
#include "drivenum.h"
#include "softlist_dev.h"

#include <stack>


//**************************************************************************
//  CORE EMULATOR OPTIONS
//**************************************************************************

const options_entry emu_options::s_option_entries[] =
{
	// unadorned options - only a single one supported at the moment
	{ OPTION_SYSTEMNAME,                                 nullptr,     OPTION_STRING,     nullptr },
	{ OPTION_SOFTWARENAME,                               nullptr,     OPTION_STRING,     nullptr },

	// config options
	{ nullptr,                                           nullptr,     OPTION_HEADER,     "CORE CONFIGURATION OPTIONS" },
	{ OPTION_READCONFIG ";rc",                           "1",         OPTION_BOOLEAN,    "enable loading of configuration files" },
	{ OPTION_WRITECONFIG ";wc",                          "0",         OPTION_BOOLEAN,    "writes configuration to (driver).ini on exit" },

	// search path options
	{ nullptr,                                           nullptr,     OPTION_HEADER,     "CORE SEARCH PATH OPTIONS" },
	{ OPTION_HOMEPATH,                                   ".",         OPTION_STRING,     "path to home directory (read/write) location" },
	{ OPTION_MEDIAPATH ";rp;biospath;bp",                "roms",      OPTION_STRING,     "path to ROMsets and hard disk images" },
	{ OPTION_HASHPATH ";hash_directory;hash",            "hash",      OPTION_STRING,     "path to hash files" },
	{ OPTION_SAMPLEPATH ";sp",                           "samples",   OPTION_STRING,     "path to samplesets" },
	{ OPTION_ARTPATH,                                    "artwork",   OPTION_STRING,     "path to artwork files" },
	{ OPTION_CTRLRPATH,                                  "ctrlr",     OPTION_STRING,     "path to controller definitions" },
	{ OPTION_INIPATH,                                    ".;ini;ini/presets",     OPTION_STRING,     "path to ini files" },
	{ OPTION_FONTPATH,                                   ".",         OPTION_STRING,     "path to font files" },
	{ OPTION_CHEATPATH,                                  "cheat",     OPTION_STRING,     "path to cheat files" },
	{ OPTION_CROSSHAIRPATH,                              "crosshair", OPTION_STRING,     "path to crosshair files" },
	{ OPTION_PLUGINSPATH,                                "plugins",   OPTION_STRING,     "path to plugin files" },
	{ OPTION_LANGUAGEPATH,                               "language",  OPTION_STRING,     "path to language files" },
	{ OPTION_SWPATH,                                     "software",  OPTION_STRING,     "path to loose software" },

	// output directory options
	{ nullptr,                                           nullptr,     OPTION_HEADER,     "CORE OUTPUT DIRECTORY OPTIONS" },
	{ OPTION_CFG_DIRECTORY,                              "cfg",       OPTION_STRING,     "directory to save configurations" },
	{ OPTION_NVRAM_DIRECTORY,                            "nvram",     OPTION_STRING,     "directory to save nvram contents" },
	{ OPTION_INPUT_DIRECTORY,                            "inp",       OPTION_STRING,     "directory to save input device logs" },
	{ OPTION_STATE_DIRECTORY,                            "sta",       OPTION_STRING,     "directory to save states" },
	{ OPTION_SNAPSHOT_DIRECTORY,                         "snap",      OPTION_STRING,     "directory to save/load screenshots" },
	{ OPTION_DIFF_DIRECTORY,                             "diff",      OPTION_STRING,     "directory to save hard drive image difference files" },
	{ OPTION_COMMENT_DIRECTORY,                          "comments",  OPTION_STRING,     "directory to save debugger comments" },

	// state/playback options
	{ nullptr,                                           nullptr,     OPTION_HEADER,     "CORE STATE/PLAYBACK OPTIONS" },
	{ OPTION_STATE,                                      nullptr,     OPTION_STRING,     "saved state to load" },
	{ OPTION_AUTOSAVE,                                   "0",         OPTION_BOOLEAN,    "enable automatic restore at startup, and automatic save at exit time" },
	{ OPTION_PLAYBACK ";pb",                             nullptr,     OPTION_STRING,     "playback an input file" },
	{ OPTION_RECORD ";rec",                              nullptr,     OPTION_STRING,     "record an input file" },
	{ OPTION_RECORD_TIMECODE,                            "0",         OPTION_BOOLEAN,    "record an input timecode file (requires -record option)" },
	{ OPTION_EXIT_AFTER_PLAYBACK,                        "0",         OPTION_BOOLEAN,    "close the program at the end of playback" },

	{ OPTION_MNGWRITE,                                   nullptr,     OPTION_STRING,     "optional filename to write a MNG movie of the current session" },
	{ OPTION_AVIWRITE,                                   nullptr,     OPTION_STRING,     "optional filename to write an AVI movie of the current session" },
	{ OPTION_WAVWRITE,                                   nullptr,     OPTION_STRING,     "optional filename to write a WAV file of the current session" },
	{ OPTION_SNAPNAME,                                   "%g/%i",     OPTION_STRING,     "override of the default snapshot/movie naming; %g == gamename, %i == index" },
	{ OPTION_SNAPSIZE,                                   "auto",      OPTION_STRING,     "specify snapshot/movie resolution (<width>x<height>) or 'auto' to use minimal size " },
	{ OPTION_SNAPVIEW,                                   "internal",  OPTION_STRING,     "specify snapshot/movie view or 'internal' to use internal pixel-aspect views" },
	{ OPTION_SNAPBILINEAR,                               "1",         OPTION_BOOLEAN,    "specify if the snapshot/movie should have bilinear filtering applied" },
	{ OPTION_STATENAME,                                  "%g",        OPTION_STRING,     "override of the default state subfolder naming; %g == gamename" },
	{ OPTION_BURNIN,                                     "0",         OPTION_BOOLEAN,    "create burn-in snapshots for each screen" },

	// performance options
	{ nullptr,                                           nullptr,     OPTION_HEADER,     "CORE PERFORMANCE OPTIONS" },
	{ OPTION_AUTOFRAMESKIP ";afs",                       "0",         OPTION_BOOLEAN,    "enable automatic frameskip selection" },
	{ OPTION_FRAMESKIP ";fs(0-10)",                      "0",         OPTION_INTEGER,    "set frameskip to fixed value, 0-10 (autoframeskip must be disabled)" },
	{ OPTION_SECONDS_TO_RUN ";str",                      "0",         OPTION_INTEGER,    "number of emulated seconds to run before automatically exiting" },
	{ OPTION_THROTTLE,                                   "1",         OPTION_BOOLEAN,    "enable throttling to keep game running in sync with real time" },
	{ OPTION_SLEEP,                                      "1",         OPTION_BOOLEAN,    "enable sleeping, which gives time back to other applications when idle" },
	{ OPTION_SPEED "(0.01-100)",                         "1.0",       OPTION_FLOAT,      "controls the speed of gameplay, relative to realtime; smaller numbers are slower" },
	{ OPTION_REFRESHSPEED ";rs",                         "0",         OPTION_BOOLEAN,    "automatically adjusts the speed of gameplay to keep the refresh rate lower than the screen" },

	// render options
	{ nullptr,                                           nullptr,     OPTION_HEADER,     "CORE RENDER OPTIONS" },
	{ OPTION_KEEPASPECT ";ka",                           "1",         OPTION_BOOLEAN,    "constrain to the proper aspect ratio" },
	{ OPTION_UNEVENSTRETCH ";ues",                       "1",         OPTION_BOOLEAN,    "allow non-integer stretch factors" },
	{ OPTION_UNEVENSTRETCHX ";uesx",                     "0",         OPTION_BOOLEAN,    "allow non-integer stretch factors only on horizontal axis"},
	{ OPTION_UNEVENSTRETCHY ";uesy",                     "0",         OPTION_BOOLEAN,    "allow non-integer stretch factors only on vertical axis"},
	{ OPTION_AUTOSTRETCHXY ";asxy",                      "0",         OPTION_BOOLEAN,    "automatically apply -unevenstretchx/y based on source native orientation"},
	{ OPTION_INTOVERSCAN ";ios",                         "0",         OPTION_BOOLEAN,    "allow overscan on integer scaled targets"},
	{ OPTION_INTSCALEX ";sx",                            "0",         OPTION_INTEGER,    "set horizontal integer scale factor."},
	{ OPTION_INTSCALEY ";sy",                            "0",         OPTION_INTEGER,    "set vertical integer scale."},

	// rotation options
	{ nullptr,                                           nullptr,     OPTION_HEADER,     "CORE ROTATION OPTIONS" },
	{ OPTION_ROTATE,                                     "1",         OPTION_BOOLEAN,    "rotate the game screen according to the game's orientation needs it" },
	{ OPTION_ROR,                                        "0",         OPTION_BOOLEAN,    "rotate screen clockwise 90 degrees" },
	{ OPTION_ROL,                                        "0",         OPTION_BOOLEAN,    "rotate screen counterclockwise 90 degrees" },
	{ OPTION_AUTOROR,                                    "0",         OPTION_BOOLEAN,    "automatically rotate screen clockwise 90 degrees if vertical" },
	{ OPTION_AUTOROL,                                    "0",         OPTION_BOOLEAN,    "automatically rotate screen counterclockwise 90 degrees if vertical" },
	{ OPTION_FLIPX,                                      "0",         OPTION_BOOLEAN,    "flip screen left-right" },
	{ OPTION_FLIPY,                                      "0",         OPTION_BOOLEAN,    "flip screen upside-down" },

	// artwork options
	{ nullptr,                                           nullptr,     OPTION_HEADER,     "CORE ARTWORK OPTIONS" },
	{ OPTION_ARTWORK_CROP ";artcrop",                    "0",         OPTION_BOOLEAN,    "crop artwork to game screen size" },
	{ OPTION_USE_BACKDROPS ";backdrop",                  "1",         OPTION_BOOLEAN,    "enable backdrops if artwork is enabled and available" },
	{ OPTION_USE_OVERLAYS ";overlay",                    "1",         OPTION_BOOLEAN,    "enable overlays if artwork is enabled and available" },
	{ OPTION_USE_BEZELS ";bezel",                        "1",         OPTION_BOOLEAN,    "enable bezels if artwork is enabled and available" },
	{ OPTION_USE_CPANELS ";cpanel",                      "1",         OPTION_BOOLEAN,    "enable cpanels if artwork is enabled and available" },
	{ OPTION_USE_MARQUEES ";marquee",                    "1",         OPTION_BOOLEAN,    "enable marquees if artwork is enabled and available" },

	// screen options
	{ nullptr,                                           nullptr,     OPTION_HEADER,     "CORE SCREEN OPTIONS" },
	{ OPTION_BRIGHTNESS "(0.1-2.0)",                     "1.0",       OPTION_FLOAT,      "default game screen brightness correction" },
	{ OPTION_CONTRAST "(0.1-2.0)",                       "1.0",       OPTION_FLOAT,      "default game screen contrast correction" },
	{ OPTION_GAMMA "(0.1-3.0)",                          "1.0",       OPTION_FLOAT,      "default game screen gamma correction" },
	{ OPTION_PAUSE_BRIGHTNESS "(0.0-1.0)",               "0.65",      OPTION_FLOAT,      "amount to scale the screen brightness when paused" },
	{ OPTION_EFFECT,                                     "none",      OPTION_STRING,     "name of a PNG file to use for visual effects, or 'none'" },

	// vector options
	{ nullptr,                                           nullptr,     OPTION_HEADER,     "CORE VECTOR OPTIONS" },
	{ OPTION_BEAM_WIDTH_MIN,                             "1.0",       OPTION_FLOAT,      "set vector beam width minimum" },
	{ OPTION_BEAM_WIDTH_MAX,                             "1.0",       OPTION_FLOAT,      "set vector beam width maximum" },
	{ OPTION_BEAM_INTENSITY_WEIGHT,                      "0",         OPTION_FLOAT,      "set vector beam intensity weight " },
	{ OPTION_FLICKER,                                    "0",         OPTION_FLOAT,      "set vector flicker effect" },

	// sound options
	{ nullptr,                                           nullptr,     OPTION_HEADER,     "CORE SOUND OPTIONS" },
	{ OPTION_SAMPLERATE ";sr(1000-1000000)",             "48000",     OPTION_INTEGER,    "set sound output sample rate" },
	{ OPTION_SAMPLES,                                    "1",         OPTION_BOOLEAN,    "enable the use of external samples if available" },
	{ OPTION_VOLUME ";vol",                              "0",         OPTION_INTEGER,    "sound volume in decibels (-32 min, 0 max)" },

	// input options
	{ nullptr,                                           nullptr,     OPTION_HEADER,     "CORE INPUT OPTIONS" },
	{ OPTION_COIN_LOCKOUT ";coinlock",                   "1",         OPTION_BOOLEAN,    "enable coin lockouts to actually lock out coins" },
	{ OPTION_CTRLR,                                      nullptr,     OPTION_STRING,     "preconfigure for specified controller" },
	{ OPTION_MOUSE,                                      "0",         OPTION_BOOLEAN,    "enable mouse input" },
	{ OPTION_JOYSTICK ";joy",                            "1",         OPTION_BOOLEAN,    "enable joystick input" },
	{ OPTION_LIGHTGUN ";gun",                            "0",         OPTION_BOOLEAN,    "enable lightgun input" },
	{ OPTION_MULTIKEYBOARD ";multikey",                  "0",         OPTION_BOOLEAN,    "enable separate input from each keyboard device (if present)" },
	{ OPTION_MULTIMOUSE,                                 "0",         OPTION_BOOLEAN,    "enable separate input from each mouse device (if present)" },
	{ OPTION_STEADYKEY ";steady",                        "0",         OPTION_BOOLEAN,    "enable steadykey support" },
	{ OPTION_UI_ACTIVE,                                  "0",         OPTION_BOOLEAN,    "enable user interface on top of emulated keyboard (if present)" },
	{ OPTION_OFFSCREEN_RELOAD ";reload",                 "0",         OPTION_BOOLEAN,    "convert lightgun button 2 into offscreen reload" },
	{ OPTION_JOYSTICK_MAP ";joymap",                     "auto",      OPTION_STRING,     "explicit joystick map, or auto to auto-select" },
	{ OPTION_JOYSTICK_DEADZONE ";joy_deadzone;jdz(0.00-1)",      "0.3",       OPTION_FLOAT,      "center deadzone range for joystick where change is ignored (0.0 center, 1.0 end)" },
	{ OPTION_JOYSTICK_SATURATION ";joy_saturation;jsat(0.00-1)", "0.85",      OPTION_FLOAT,      "end of axis saturation range for joystick where change is ignored (0.0 center, 1.0 end)" },
	{ OPTION_NATURAL_KEYBOARD ";nat",                    "0",         OPTION_BOOLEAN,    "specifies whether to use a natural keyboard or not" },
	{ OPTION_JOYSTICK_CONTRADICTORY ";joy_contradictory","0",         OPTION_BOOLEAN,    "enable contradictory direction digital joystick input at the same time" },
	{ OPTION_COIN_IMPULSE,                               "0",         OPTION_INTEGER,    "set coin impulse time (n<0 disable impulse, n==0 obey driver, 0<n set time n)" },

	// input autoenable options
	{ nullptr,                                           nullptr,     OPTION_HEADER,     "CORE INPUT AUTOMATIC ENABLE OPTIONS" },
	{ OPTION_PADDLE_DEVICE ";paddle",                    "keyboard",  OPTION_STRING,     "enable (none|keyboard|mouse|lightgun|joystick) if a paddle control is present" },
	{ OPTION_ADSTICK_DEVICE ";adstick",                  "keyboard",  OPTION_STRING,     "enable (none|keyboard|mouse|lightgun|joystick) if an analog joystick control is present" },
	{ OPTION_PEDAL_DEVICE ";pedal",                      "keyboard",  OPTION_STRING,     "enable (none|keyboard|mouse|lightgun|joystick) if a pedal control is present" },
	{ OPTION_DIAL_DEVICE ";dial",                        "keyboard",  OPTION_STRING,     "enable (none|keyboard|mouse|lightgun|joystick) if a dial control is present" },
	{ OPTION_TRACKBALL_DEVICE ";trackball",              "keyboard",  OPTION_STRING,     "enable (none|keyboard|mouse|lightgun|joystick) if a trackball control is present" },
	{ OPTION_LIGHTGUN_DEVICE,                            "keyboard",  OPTION_STRING,     "enable (none|keyboard|mouse|lightgun|joystick) if a lightgun control is present" },
	{ OPTION_POSITIONAL_DEVICE,                          "keyboard",  OPTION_STRING,     "enable (none|keyboard|mouse|lightgun|joystick) if a positional control is present" },
	{ OPTION_MOUSE_DEVICE,                               "mouse",     OPTION_STRING,     "enable (none|keyboard|mouse|lightgun|joystick) if a mouse control is present" },

	// debugging options
	{ nullptr,                                           nullptr,     OPTION_HEADER,     "CORE DEBUGGING OPTIONS" },
	{ OPTION_VERBOSE ";v",                               "0",         OPTION_BOOLEAN,    "display additional diagnostic information" },
	{ OPTION_LOG,                                        "0",         OPTION_BOOLEAN,    "generate an error.log file" },
	{ OPTION_OSLOG,                                      "0",         OPTION_BOOLEAN,    "output error.log data to the system debugger" },
	{ OPTION_DEBUG ";d",                                 "0",         OPTION_BOOLEAN,    "enable/disable debugger" },
	{ OPTION_UPDATEINPAUSE,                              "0",         OPTION_BOOLEAN,    "keep calling video updates while in pause" },
	{ OPTION_DEBUGSCRIPT,                                nullptr,     OPTION_STRING,     "script for debugger" },

	// comm options
	{ nullptr,                                           nullptr,     OPTION_HEADER,     "CORE COMM OPTIONS" },
	{ OPTION_COMM_LOCAL_HOST,                            "0.0.0.0",   OPTION_STRING,     "local address to bind to" },
	{ OPTION_COMM_LOCAL_PORT,                            "15112",     OPTION_STRING,     "local port to bind to" },
	{ OPTION_COMM_REMOTE_HOST,                           "127.0.0.1", OPTION_STRING,     "remote address to connect to" },
	{ OPTION_COMM_REMOTE_PORT,                           "15112",     OPTION_STRING,     "remote port to connect to" },

	// misc options
	{ nullptr,                                           nullptr,     OPTION_HEADER,     "CORE MISC OPTIONS" },
	{ OPTION_DRC,                                        "1",         OPTION_BOOLEAN,    "enable DRC cpu core if available" },
	{ OPTION_DRC_USE_C,                                  "0",         OPTION_BOOLEAN,    "force DRC use C backend" },
	{ OPTION_DRC_LOG_UML,                                "0",         OPTION_BOOLEAN,    "write DRC UML disassembly log" },
	{ OPTION_DRC_LOG_NATIVE,                             "0",         OPTION_BOOLEAN,    "write DRC native disassembly log" },
	{ OPTION_BIOS,                                       nullptr,     OPTION_STRING,     "select the system BIOS to use" },
	{ OPTION_CHEAT ";c",                                 "0",         OPTION_BOOLEAN,    "enable cheat subsystem" },
	{ OPTION_SKIP_GAMEINFO,                              "0",         OPTION_BOOLEAN,    "skip displaying the information screen at startup" },
	{ OPTION_UI_FONT,                                    "default",   OPTION_STRING,     "specify a font to use" },
	{ OPTION_UI,                                         "cabinet",   OPTION_STRING,     "type of UI (simple|cabinet)" },
	{ OPTION_RAMSIZE ";ram",                             nullptr,     OPTION_STRING,     "size of RAM (if supported by driver)" },
	{ OPTION_CONFIRM_QUIT,                               "0",         OPTION_BOOLEAN,    "display confirm quit screen on exit" },
	{ OPTION_UI_MOUSE,                                   "1",         OPTION_BOOLEAN,    "display ui mouse cursor" },
	{ OPTION_AUTOBOOT_COMMAND ";ab",                     nullptr,     OPTION_STRING,     "command to execute after machine boot" },
	{ OPTION_AUTOBOOT_DELAY,                             "0",         OPTION_INTEGER,    "timer delay in sec to trigger command execution on autoboot" },
	{ OPTION_AUTOBOOT_SCRIPT ";script",                  nullptr,     OPTION_STRING,     "lua script to execute after machine boot" },
	{ OPTION_CONSOLE,                                    "0",         OPTION_BOOLEAN,    "enable emulator LUA console" },
	{ OPTION_PLUGINS,                                    "1",         OPTION_BOOLEAN,    "enable LUA plugin support" },
	{ OPTION_PLUGIN,                                     nullptr,     OPTION_STRING,     "list of plugins to enable" },
	{ OPTION_NO_PLUGIN,                                  nullptr,     OPTION_STRING,     "list of plugins to disable" },
	{ OPTION_LANGUAGE ";lang",                           "English",   OPTION_STRING,     "display language" },

	{ nullptr,                                           nullptr,     OPTION_HEADER,     "HTTP SERVER OPTIONS" },
	{ OPTION_HTTP,                                       "0",         OPTION_BOOLEAN,    "HTTP server enable" },
	{ OPTION_HTTP_PORT,                                  "8080",      OPTION_INTEGER,    "HTTP server port" },
	{ OPTION_HTTP_ROOT,                                  "web",       OPTION_STRING,     "HTTP server document root" },

	{ nullptr }
};



//**************************************************************************
//  CUSTOM OPTION ENTRIES AND SUPPORT CLASSES
//**************************************************************************

namespace
{
	// custom option entry for the system name
	class system_name_option_entry : public core_options::entry
	{
	public:
		system_name_option_entry(emu_options &host)
			: entry(OPTION_SYSTEMNAME)
			, m_host(host)
		{
		}

		virtual const char *value() const override
		{
			return m_host.system() ? m_host.system()->name : "";
		}

	protected:
		virtual void internal_set_value(std::string &&newvalue) override
		{
			m_host.set_system_name(newvalue);
		}

	private:
		emu_options &m_host;
	};

	// custom option entry for the software name
	class software_name_option_entry : public core_options::entry
	{
	public:
		software_name_option_entry(emu_options &host)
			: entry(OPTION_SOFTWARENAME)
			, m_host(host)
		{
		}

	protected:
		virtual void internal_set_value(std::string &&newvalue) override
		{
			m_host.set_software(newvalue);
		}

	private:
		emu_options &m_host;
	};

	// custom option entry for slots
	class slot_option_entry : public core_options::entry
	{
	public:
		slot_option_entry(const char *name, slot_option &host)
			: entry(name)
			, m_host(host)
		{
		}

		virtual const char *value() const override
		{
			return m_host.specified_value().c_str();
		}

	protected:
		virtual void internal_set_value(std::string &&newvalue) override
		{
			m_host.specify(std::move(newvalue));
		}

	private:
		slot_option &m_host;
	};

	// existing option tracker class; used by slot/image calculus to identify existing
	// options for later purging
	template<typename T>
	class existing_option_tracker
	{
	public:
		existing_option_tracker(const std::unordered_map<std::string, T> &map)
		{
			m_vec.reserve(map.size());
			for (const auto &entry : map)
				m_vec.push_back(&entry.first);
		}

		template<typename TStr>
		void remove(TStr str)
		{
			auto iter = std::find_if(
				m_vec.begin(),
				m_vec.end(),
				[&str](const auto &x) { return *x == str; });
			if (iter != m_vec.end())
				m_vec.erase(iter);
		}

		std::vector<const std::string *>::iterator begin() { return m_vec.begin(); }
		std::vector<const std::string *>::iterator end() { return m_vec.end(); }

	private:
		std::vector<const std::string *> m_vec;
	};


	//-------------------------------------------------
	//  get_full_option_names
	//-------------------------------------------------

	std::vector<std::string> get_full_option_names(const device_image_interface &image)
	{
		std::vector<std::string> result;

		result.push_back(image.instance_name());
		result.push_back(image.brief_instance_name());

		if (strcmp(image.device_typename(image.image_type()), image.instance_name().c_str()) == 0)
		{
			result.push_back(image.instance_name() + "1");
			result.push_back(image.brief_instance_name() + "1");
		}
		return result;
	}
}


//**************************************************************************
//  EMU OPTIONS
//**************************************************************************

//-------------------------------------------------
//  emu_options - constructor
//-------------------------------------------------

emu_options::emu_options(bool general_only)
	: m_system(nullptr)
	, m_coin_impulse(0)
	, m_joystick_contradictory(false)
	, m_sleep(true)
	, m_refresh_speed(false)
	, m_ui(UI_CABINET)
{
	// add entries
	if (!general_only)
	{
		add_entry(std::make_unique<system_name_option_entry>(*this));
		add_entry(std::make_unique<software_name_option_entry>(*this));
	}
	add_entries(emu_options::s_option_entries);

	// adding handlers to keep copies of frequently requested options in member variables
	set_value_changed_handler(OPTION_COIN_IMPULSE,				[this](const char *value) { m_coin_impulse = int_value(OPTION_COIN_IMPULSE); });
	set_value_changed_handler(OPTION_JOYSTICK_CONTRADICTORY,	[this](const char *value) { m_joystick_contradictory = bool_value(OPTION_JOYSTICK_CONTRADICTORY); });
	set_value_changed_handler(OPTION_SLEEP,						[this](const char *value) { m_sleep = bool_value(OPTION_SLEEP); });
	set_value_changed_handler(OPTION_REFRESHSPEED,				[this](const char *value) { m_sleep = bool_value(OPTION_REFRESHSPEED); });
	set_value_changed_handler(OPTION_UI, [this](const std::string &value)
	{
		if (value == "simple")
			m_ui = UI_SIMPLE;
		else
			m_ui = UI_CABINET;
	});
}


//-------------------------------------------------
//  emu_options - destructor
//-------------------------------------------------

emu_options::~emu_options()
{
}


//-------------------------------------------------
//	system_name
//-------------------------------------------------

const char *emu_options::system_name() const
{
	return m_system ? m_system->name : "";
}


//-------------------------------------------------
//  set_system_name - called to set the system
//	name; will adjust slot/image options as appropriate
//-------------------------------------------------

void emu_options::set_system_name(const std::string &new_system_name)
{
	const game_driver *new_system = nullptr;

	// was a system name specified?
	if (!new_system_name.empty())
	{
		// if so, find it (and error if it cannot be found)
		int index = driver_list::find(core_filename_extract_base(new_system_name, true).c_str());
		if (index < 0)
			throw options_exception("Unknown system '%s'", new_system_name);
		new_system = &driver_list::driver(index);
	}

	// did we change anything?
	if (new_system != m_system)
	{
		// if so, specify the new system and update
		m_system = new_system;
		update_slot_and_image_options();
	}
}


//-------------------------------------------------
//	update_slot_and_image_options
//-------------------------------------------------

void emu_options::update_slot_and_image_options()
{
	bool changed;
	do
	{
		changed = false;

		// first we add and remove slot options depending on what has been configured in the
		// device, bringing m_slot_options up to a state where it matches machine_config
		if (add_and_remove_slot_options())
			changed = true;

		// second, we perform an analgous operation with m_image_options
		if (add_and_remove_image_options())
			changed = true;
	} while (changed);
}


//-------------------------------------------------
//	add_and_remove_slot_options - add any missing
//	and/or purge extraneous slot options
//-------------------------------------------------

bool emu_options::add_and_remove_slot_options()
{
	bool changed = false;

	// first, create a list of existing slot options; this is so we can purge
	// any stray slot options that are no longer pertinent when we're done
	existing_option_tracker<::slot_option> existing(m_slot_options);

	// it is perfectly legal for this to be called without a system; we
	// need to check for that condition!
	if (m_system)
	{
		// create the configuration
		machine_config config(*m_system, *this);

		for (const device_slot_interface &slot : slot_interface_iterator(config.root_device()))
		{
			// come up with the cannonical name of the slot
			const char *slot_option_name = slot.slot_name();

			// erase this option from existing (so we don't purge it later)
			existing.remove(slot_option_name);

			// do we need to add this option?
			if (!has_slot_option(slot_option_name))
			{
				// we do - add it to m_slot_options
				m_slot_options[slot_option_name] = ::slot_option(slot.default_option());
				changed = true;

				// for non-fixed slots, this slot needs representation in the options collection
				if (!slot.fixed())
				{
					// first device? add the header as to be pretty
					const char *header = "SLOT DEVICES";
					if (!header_exists(header))
						add_header(header);

					// create a new entry in the options
					auto new_entry = m_slot_options[slot_option_name].setup_option_entry(slot_option_name);

					// and add it
					add_entry(std::move(new_entry), header);
				}
			}

		}
	}

	// at this point we need to purge stray slot options that may no longer be pertinent
	for (auto &opt_name : existing)
	{
		auto iter = m_slot_options.find(*opt_name);
		assert(iter != m_slot_options.end());

		// if this is represented in core_options, remove it
		if (iter->second.option_entry())
			remove_entry(*iter->second.option_entry());

		// remove this option
		m_slot_options.erase(iter);
		changed = true;
	}

	return changed;
}


//-------------------------------------------------
//	add_and_remove_slot_options - add any missing
//	and/or purge extraneous slot options
//-------------------------------------------------

bool emu_options::add_and_remove_image_options()
{
	// The logic for image options is superficially similar to the logic for slot options, but
	// there is one larger piece of complexity.  The image instance names (returned by the
	// image_instance() call and surfaced in the UI) may change simply because we've added more
	// devices.  This is because the instance_name() for a singular cartridge device might be
	// "cartridge" starting out, but become "cartridge1" when another cartridge device is added.
	//
	// To get around this behavior, our internal data structures work in terms of what is
	// returned by cannonical_instance_name(), which will be something like "cartridge1" both
	// for a singular cartridge device and the first cartridge in a multi cartridge system.
	//
	// The need for this behavior was identified by Tafoid when the following command line
	// regressed:
	//		
	//		mame snes bsxsore -cart2 bszelda
	//
	// Before we were accounting for this behavior, 'bsxsore' got stored in "cartridge" and
	// the association got lost when the second cartridge was added.

	bool changed = false;

	// first, create a list of existing image options; this is so we can purge
	// any stray slot options that are no longer pertinent when we're done; we
	// have to do this for both "flavors" of name
	existing_option_tracker<::image_option> existing(m_image_options_cannonical);

	// wipe the non-cannonical image options; we're going to rebuild it
	m_image_options.clear();

	// it is perfectly legal for this to be called without a system; we
	// need to check for that condition!
	if (m_system)
	{
		// create the configuration
		machine_config config(*m_system, *this);

		// iterate through all image devices
		for (device_image_interface &image : image_interface_iterator(config.root_device()))
		{
			const std::string &name(image.instance_name());
			const std::string &cannonical_name(image.cannonical_instance_name());

			// erase this option from existing (so we don't purge it later)
			existing.remove(cannonical_name);

			// do we need to add this option?
			auto iter = m_image_options_cannonical.find(cannonical_name);
			::image_option *this_option = iter != m_image_options_cannonical.end() ? &iter->second : nullptr;
			if (!this_option)
			{
				// we do - add it to both m_image_options_cannonical and m_image_options
				m_image_options_cannonical[cannonical_name] = ::image_option(image.cannonical_instance_name());
				this_option = &m_image_options_cannonical[cannonical_name];
				changed = true;

				// if this image is user loadable, we have to surface it in the core_options
				if (image.user_loadable())
				{
					// first device? add the header as to be pretty
					const char *header = "IMAGE DEVICES";
					if (!header_exists(header))
						add_header(header);

					// name this options
					auto names = get_full_option_names(image);

					// create a new entry in the options
					auto new_entry = this_option->setup_option_entry(std::move(names));

					// and add it
					add_entry(std::move(new_entry), header);
				}
			}

			// whether we added it or we didn't, we have to add it to the m_image_option map
			m_image_options[image.instance_name()] = this_option;
		}
	}

	// at this point we need to purge stray image options that may no longer be pertinent
	for (auto &opt_name : existing)
	{
		auto iter = m_image_options_cannonical.find(*opt_name);
		assert(iter != m_image_options_cannonical.end());

		// if this is represented in core_options, remove it
		if (iter->second.option_entry())
			remove_entry(*iter->second.option_entry());

		// remove this option
		m_image_options_cannonical.erase(iter);
		changed = true;
	}

	return changed;
}


//-------------------------------------------------
//  set_software - called to load "unqualified"
//	software out of a software list (e.g. - "mame nes 'zelda'")
//-------------------------------------------------

void emu_options::set_software(const std::string &new_software)
{
	// identify any options as a result of softlists
	auto softlist_opts = evaluate_initial_softlist_options(new_software);

	// NYI
	throw false;
}


//-------------------------------------------------
//  evaluate_initial_softlist_options
//-------------------------------------------------

std::map<std::string, std::string> emu_options::evaluate_initial_softlist_options(const std::string &software_identifier)
{
	std::map<std::string, std::string> results;

	// load software specified at the command line (if any of course)
	if (!software_identifier.empty())
	{
		// we have software; first identify the proper game_driver
		if (!m_system)
			throw options_exception("Cannot specify software without specifying system");

		// and set up a configuration
		machine_config config(*m_system, *this);
		software_list_device_iterator iter(config.root_device());
		if (iter.count() == 0)
			throw emu_fatalerror(EMU_ERR_FATALERROR, "Error: unknown option: %s\n", software_identifier.c_str());

		// and finally set up the stack
		std::stack<std::string> software_identifier_stack;
		software_identifier_stack.push(software_identifier);

		// we need to keep evaluating softlist identifiers until the stack is empty
		while (!software_identifier_stack.empty())
		{
			// pop the identifier
			std::string current_software_identifier = std::move(software_identifier_stack.top());
			software_identifier_stack.pop();

			// and parse it
			std::string list_name, software_name;
			auto colon_pos = current_software_identifier.find_first_of(':');
			if (colon_pos != std::string::npos)
			{
				list_name = current_software_identifier.substr(0, colon_pos);
				software_name = current_software_identifier.substr(colon_pos + 1);
			}
			else
			{
				software_name = current_software_identifier;
			}

			// loop through all softlist devices, and try to find one capable of handling the requested software
			bool found = false;
			bool compatible = false;
			for (software_list_device &swlistdev : iter)
			{
				if (list_name.empty() || (list_name == swlistdev.list_name()))
				{
					const software_info *swinfo = swlistdev.find(software_name);
					if (swinfo != nullptr)
					{
						// loop through all parts
						for (const software_part &swpart : swinfo->parts())
						{
							// only load compatible software this way
							if (swlistdev.is_compatible(swpart) == SOFTWARE_IS_COMPATIBLE)
							{
								// we need to find a mountable image slot, but we need to ensure it is a slot
								// for which we have not already distributed a part to
								device_image_interface *image = software_list_device::find_mountable_image(
									config,
									swpart,
									[&results](const device_image_interface &candidate) { return results.count(candidate.instance_name()) == 0; });

								// did we find a slot to put this part into?
								if (image != nullptr)
								{
									// we've resolved this software
									results[image->instance_name()] = string_format("%s:%s:%s", swlistdev.list_name(), software_name, swpart.name());

									// does this software part have a requirement on another part?
									const char *requirement = swpart.feature("requirement");
									if (requirement)
										software_identifier_stack.push(requirement);
								}
								compatible = true;
							}
							found = true;
						}

						// identify other shared features specified as '<<slot name>>_default'
						//
						// example from SMS:
						//
						//  <software name = "alexbmx">
						//      ...
						//      <sharedfeat name = "ctrl1_default" value = "paddle" />
						//  </software>
						for (const feature_list_item &fi : swinfo->shared_info())
						{
							const std::string default_suffix = "_default";
							if (fi.name().size() > default_suffix.size()
								&& fi.name().compare(fi.name().size() - default_suffix.size(), default_suffix.size(), default_suffix) == 0)
							{
								std::string slot_name = fi.name().substr(0, fi.name().size() - default_suffix.size());
								results[slot_name] = fi.value();
							}
						}
					}
				}
				if (compatible)
					break;
			}

			if (!compatible)
			{
				software_list_device::display_matches(config, nullptr, software_name);
				if (!found)
					throw options_exception("");
				else
					throw options_exception("Software '%s' is incompatible with system '%s'\n", software_name, m_system->name);
			}
		}
	}
	return results;
}


//-------------------------------------------------
//	slot_option
//-------------------------------------------------

const slot_option &emu_options::slot_option(const std::string &device_name) const
{
	auto iter = m_slot_options.find(device_name);
	assert(iter != m_slot_options.end() && "Attempt to access non-existent slot option");
	return iter->second;
}

slot_option &emu_options::slot_option(const std::string &device_name)
{
	auto iter = m_slot_options.find(device_name);
	assert(iter != m_slot_options.end() && "Attempt to access non-existent slot option");
	return iter->second;
}


//-------------------------------------------------
//	has_slot_option
//-------------------------------------------------

bool emu_options::has_slot_option(const std::string &device_name) const
{
	return m_slot_options.count(device_name) > 0;
}


//-------------------------------------------------
//	image_option
//-------------------------------------------------

const image_option &emu_options::image_option(const std::string &device_name) const
{
	auto iter = m_image_options.find(device_name);
	assert(iter != m_image_options.end() && "Attempt to access non-existent image option");
	return *iter->second;
}

image_option &emu_options::image_option(const std::string &device_name)
{
	auto iter = m_image_options.find(device_name);
	assert(iter != m_image_options.end() && "Attempt to access non-existent image option");
	return *iter->second;
}


//**************************************************************************
//  SLOT OPTIONS
//**************************************************************************

//-------------------------------------------------
//	slot_option ctor
//-------------------------------------------------

slot_option::slot_option(const char *default_value)
	: m_specified(false)
	, m_default_value(default_value ? default_value : "")
	, m_entry(nullptr)
{
}


//-------------------------------------------------
//	slot_option::value
//-------------------------------------------------

const std::string &slot_option::value() const
{
	// There are a number of ways that the value can be determined; there
	// is a specific order of precedence:
	//
	//	1.  Highest priority is whatever may have been specified by the user (whether it
	//		was specified at the command line, an INI file, or in the UI).  We keep track
	//		of whether these values were specified this way
	//
	//		Take note that slots have a notion of being "selectable".  Slots that are not
	//		marked as selectable cannot be specified with this technique
	//
	//	2.  Next highest is what is returned from get_default_card_software()
	//
	//	3.	Last in priority is what was specified as the slot default.  This comes from
	//		device setup
	if (m_specified)
		return m_specified_value;
	else if (!m_default_card_software.empty())
		return m_default_card_software;
	else
		return m_default_value;
}


//-------------------------------------------------
//	slot_option::specified_value
//-------------------------------------------------

std::string slot_option::specified_value() const
{
	std::string result;
	if (m_specified)
	{
		result = m_specified_bios.empty()
			? m_specified_value
			: util::string_format("%s,bios=%s", m_specified_value, m_specified_bios);
	}
	return result;
}


//-------------------------------------------------
//	slot_option::specify
//-------------------------------------------------

void slot_option::specify(std::string &&text)
{
	// we need to do some elementary parsing here
	const char *bios_arg = ",bios=";

	size_t pos = text.find(bios_arg);
	if (pos != std::string::npos)
	{
		m_specified = true;
		m_specified_value = text.substr(0, pos);
		m_specified_bios = text.substr(pos + strlen(bios_arg));
	}
	else
	{
		m_specified = true;
		m_specified_value = std::move(text);
		m_specified_bios = "";
	}
}


//-------------------------------------------------
//	slot_option::set_bios
//-------------------------------------------------

void slot_option::set_bios(std::string &&text)
{
	if (!m_specified)
	{
		m_specified = true;
		m_specified_value = value();
	}
	m_specified_bios = std::move(text);
}


//-------------------------------------------------
//	slot_option::setup_option_entry
//-------------------------------------------------

core_options::entry::ptr slot_option::setup_option_entry(const char *name)
{
	// this should only be called once
	assert(!m_entry);

	// create the entry and return it
	m_entry = new slot_option_entry(name, *this);
	return std::unique_ptr<core_options::entry>(m_entry);
}


//**************************************************************************
//  IMAGE OPTIONS
//**************************************************************************

//-------------------------------------------------
//	image_option ctor
//-------------------------------------------------

image_option::image_option(const std::string &cannonical_instance_name)
	: m_cannonical_instance_name(cannonical_instance_name)
{
}


//-------------------------------------------------
//	image_option::setup_option_entry
//-------------------------------------------------

core_options::entry::ptr image_option::setup_option_entry(std::vector<std::string> &&names)
{
	throw false;
}

