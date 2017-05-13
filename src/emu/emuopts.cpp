// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    emuopts.cpp

    Options file and command line management.

***************************************************************************/

#include "emu.h"
#include "emuopts.h"


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
//  EMU OPTIONS
//**************************************************************************

//-------------------------------------------------
//  emu_options - constructor
//-------------------------------------------------

emu_options::emu_options()
: core_options()
, m_coin_impulse(0)
, m_joystick_contradictory(false)
, m_sleep(true)
, m_refresh_speed(false)
, m_ui(UI_CABINET)
{
	add_entries(emu_options::s_option_entries);
}


//-------------------------------------------------
//  value_changed - to prevent tagmap
//    lookups keep copies of frequently requested
//    options in member variables.
//-------------------------------------------------

void emu_options::value_changed(const std::string &name, const std::string &value)
{
	if (name == OPTION_COIN_IMPULSE)
	{
		m_coin_impulse = int_value(OPTION_COIN_IMPULSE);
	}
	else if (name == OPTION_JOYSTICK_CONTRADICTORY)
	{
		m_joystick_contradictory = bool_value(OPTION_JOYSTICK_CONTRADICTORY);
	}
	else if (name == OPTION_SLEEP)
	{
		m_sleep = bool_value(OPTION_SLEEP);
	}
	else if (name == OPTION_REFRESHSPEED)
	{
		m_refresh_speed = bool_value(OPTION_REFRESHSPEED);
	}
	else if (name == OPTION_UI)
	{
		if (value == "simple")
			m_ui = UI_SIMPLE;
		else
			m_ui = UI_CABINET;
	}
}


//-------------------------------------------------
//  override_get_value - when saving to an INI, we
//  need to hook into that process so we can write
//  out image/slot options
//-------------------------------------------------

core_options::override_get_value_result emu_options::override_get_value(const char *name, std::string &value) const
{
	if (name)
	{
		auto slotiter = m_slot_options.find(name);
		if (slotiter != m_slot_options.end())
		{
			value = slotiter->second.specified_value();
			return slotiter->second.specified()
				? override_get_value_result::OVERRIDE
				: override_get_value_result::SKIP;
		}

		auto imageiter = m_image_options.find(name);
		if (imageiter != m_image_options.end())
		{
			value = imageiter->second;
			return override_get_value_result::OVERRIDE;
		}
	}

	return override_get_value_result::NONE;
}


//-------------------------------------------------
//  override_set_value - when parsing an INI, we
//  need to hook into into it so we can do the same
//  crazy slot logic done in mameopt
//-------------------------------------------------

bool emu_options::override_set_value(const char *name, const std::string &value)
{
	auto slotiter = m_slot_options.find(name);
	if (slotiter != m_slot_options.end())
	{
		slotiter->second.specify(std::string(value));
		return true;
	}

	auto imageiter = m_image_options.find(name);
	if (imageiter != m_image_options.end())
	{
		// We've found a potential image slot for this value.  However, we're only going to specify it
		// if the current image option is empty.  This is because if there is an image option already
		// present, it is almost certain that this was because something was specified at the command
		// line and we're parsing an INI.  Because INIs have less priority than the command line, this
		// should be ignored
		//
		// Obviously, this ignores that INIs themselves have their own prioritization, so this should be
		// considered to be a hack.  Instead of having image options being just a straight map of std::string
		// it should really be a structure where the priority can be recorded
		if (imageiter->second.empty())
			imageiter->second = value;
		return true;
	}

	return false;
}


//-------------------------------------------------
//  slot_option ctor
//-------------------------------------------------

slot_option::slot_option(const char *default_value)
	: m_specified(false)
	, m_default_value(default_value ? default_value : "")
{
}


//-------------------------------------------------
//  slot_option::value
//-------------------------------------------------

const std::string &slot_option::value() const
{
	// There are a number of ways that the value can be determined; there
	// is a specific order of precedence:
	//
	//  1.  Highest priority is whatever may have been specified by the user (whether it
	//      was specified at the command line, an INI file, or in the UI).  We keep track
	//      of whether these values were specified this way
	//
	//      Take note that slots have a notion of being "selectable".  Slots that are not
	//      marked as selectable cannot be specified with this technique
	//
	//  2.  Next highest is what is returned from get_default_card_software()
	//
	//  3.  Last in priority is what was specified as the slot default.  This comes from
	//      device setup
	if (m_specified)
		return m_specified_value;
	else if (!m_default_card_software.empty())
		return m_default_card_software;
	else
		return m_default_value;
}


//-------------------------------------------------
//  slot_option::specified_value
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
//  slot_option::specify
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
//  slot_option::set_bios
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
