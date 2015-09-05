// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    emuopts.c

    Options file and command line management.

***************************************************************************/

#include "emu.h"
#include "emuopts.h"
#include "drivenum.h"

#include <ctype.h>



//**************************************************************************
//  CORE EMULATOR OPTIONS
//**************************************************************************

const options_entry emu_options::s_option_entries[] =
{
	// unadorned options - only a single one supported at the moment
	{ OPTION_SYSTEMNAME,                                 NULL,        OPTION_STRING,     NULL },
	{ OPTION_SOFTWARENAME,                               NULL,        OPTION_STRING,     NULL },

	// config options
	{ NULL,                                              NULL,        OPTION_HEADER,     "CORE CONFIGURATION OPTIONS" },
	{ OPTION_READCONFIG ";rc",                           "1",         OPTION_BOOLEAN,    "enable loading of configuration files" },
	{ OPTION_WRITECONFIG ";wc",                          "0",         OPTION_BOOLEAN,    "writes configuration to (driver).ini on exit" },

	// seach path options
	{ NULL,                                              NULL,        OPTION_HEADER,     "CORE SEARCH PATH OPTIONS" },
	{ OPTION_MEDIAPATH ";rp;biospath;bp",                "roms",      OPTION_STRING,     "path to ROMsets and hard disk images" },
	{ OPTION_HASHPATH ";hash_directory;hash",            "hash",      OPTION_STRING,     "path to hash files" },
	{ OPTION_SAMPLEPATH ";sp",                           "samples",   OPTION_STRING,     "path to samplesets" },
	{ OPTION_ARTPATH,                                    "artwork",   OPTION_STRING,     "path to artwork files" },
	{ OPTION_CTRLRPATH,                                  "ctrlr",     OPTION_STRING,     "path to controller definitions" },
	{ OPTION_INIPATH,                                    ".;ini",     OPTION_STRING,     "path to ini files" },
	{ OPTION_FONTPATH,                                   ".",         OPTION_STRING,     "path to font files" },
	{ OPTION_CHEATPATH,                                  "cheat",     OPTION_STRING,     "path to cheat files" },
	{ OPTION_CROSSHAIRPATH,                              "crosshair", OPTION_STRING,     "path to crosshair files" },

	// output directory options
	{ NULL,                                              NULL,        OPTION_HEADER,     "CORE OUTPUT DIRECTORY OPTIONS" },
	{ OPTION_CFG_DIRECTORY,                              "cfg",       OPTION_STRING,     "directory to save configurations" },
	{ OPTION_NVRAM_DIRECTORY,                            "nvram",     OPTION_STRING,     "directory to save nvram contents" },
	{ OPTION_INPUT_DIRECTORY,                            "inp",       OPTION_STRING,     "directory to save input device logs" },
	{ OPTION_STATE_DIRECTORY,                            "sta",       OPTION_STRING,     "directory to save states" },
	{ OPTION_SNAPSHOT_DIRECTORY,                         "snap",      OPTION_STRING,     "directory to save/load screenshots" },
	{ OPTION_DIFF_DIRECTORY,                             "diff",      OPTION_STRING,     "directory to save hard drive image difference files" },
	{ OPTION_COMMENT_DIRECTORY,                          "comments",  OPTION_STRING,     "directory to save debugger comments" },

	// state/playback options
	{ NULL,                                              NULL,        OPTION_HEADER,     "CORE STATE/PLAYBACK OPTIONS" },
	{ OPTION_STATE,                                      NULL,        OPTION_STRING,     "saved state to load" },
	{ OPTION_AUTOSAVE,                                   "0",         OPTION_BOOLEAN,    "enable automatic restore at startup, and automatic save at exit time" },
	{ OPTION_PLAYBACK ";pb",                             NULL,        OPTION_STRING,     "playback an input file" },
	{ OPTION_RECORD ";rec",                              NULL,        OPTION_STRING,     "record an input file" },
	{ OPTION_MNGWRITE,                                   NULL,        OPTION_STRING,     "optional filename to write a MNG movie of the current session" },
	{ OPTION_AVIWRITE,                                   NULL,        OPTION_STRING,     "optional filename to write an AVI movie of the current session" },
#ifdef MAME_DEBUG
	{ OPTION_DUMMYWRITE,                                 "0",         OPTION_BOOLEAN,    "indicates if a snapshot should be created if each frame" },
#endif
	{ OPTION_WAVWRITE,                                   NULL,        OPTION_STRING,     "optional filename to write a WAV file of the current session" },
	{ OPTION_SNAPNAME,                                   "%g/%i",     OPTION_STRING,     "override of the default snapshot/movie naming; %g == gamename, %i == index" },
	{ OPTION_SNAPSIZE,                                   "auto",      OPTION_STRING,     "specify snapshot/movie resolution (<width>x<height>) or 'auto' to use minimal size " },
	{ OPTION_SNAPVIEW,                                   "internal",  OPTION_STRING,     "specify snapshot/movie view or 'internal' to use internal pixel-aspect views" },
	{ OPTION_SNAPBILINEAR,                               "1",         OPTION_BOOLEAN,    "specify if the snapshot/movie should have bilinear filtering applied" },
	{ OPTION_STATENAME,                                  "%g",        OPTION_STRING,     "override of the default state subfolder naming; %g == gamename" },
	{ OPTION_BURNIN,                                     "0",         OPTION_BOOLEAN,    "create burn-in snapshots for each screen" },

	// performance options
	{ NULL,                                              NULL,        OPTION_HEADER,     "CORE PERFORMANCE OPTIONS" },
	{ OPTION_AUTOFRAMESKIP ";afs",                       "0",         OPTION_BOOLEAN,    "enable automatic frameskip selection" },
	{ OPTION_FRAMESKIP ";fs(0-10)",                      "0",         OPTION_INTEGER,    "set frameskip to fixed value, 0-10 (autoframeskip must be disabled)" },
	{ OPTION_SECONDS_TO_RUN ";str",                      "0",         OPTION_INTEGER,    "number of emulated seconds to run before automatically exiting" },
	{ OPTION_THROTTLE,                                   "1",         OPTION_BOOLEAN,    "enable throttling to keep game running in sync with real time" },
	{ OPTION_SLEEP,                                      "1",         OPTION_BOOLEAN,    "enable sleeping, which gives time back to other applications when idle" },
	{ OPTION_SPEED "(0.01-100)",                         "1.0",       OPTION_FLOAT,      "controls the speed of gameplay, relative to realtime; smaller numbers are slower" },
	{ OPTION_REFRESHSPEED ";rs",                         "0",         OPTION_BOOLEAN,    "automatically adjusts the speed of gameplay to keep the refresh rate lower than the screen" },

	// rotation options
	{ NULL,                                              NULL,        OPTION_HEADER,     "CORE ROTATION OPTIONS" },
	{ OPTION_ROTATE,                                     "1",         OPTION_BOOLEAN,    "rotate the game screen according to the game's orientation needs it" },
	{ OPTION_ROR,                                        "0",         OPTION_BOOLEAN,    "rotate screen clockwise 90 degrees" },
	{ OPTION_ROL,                                        "0",         OPTION_BOOLEAN,    "rotate screen counterclockwise 90 degrees" },
	{ OPTION_AUTOROR,                                    "0",         OPTION_BOOLEAN,    "automatically rotate screen clockwise 90 degrees if vertical" },
	{ OPTION_AUTOROL,                                    "0",         OPTION_BOOLEAN,    "automatically rotate screen counterclockwise 90 degrees if vertical" },
	{ OPTION_FLIPX,                                      "0",         OPTION_BOOLEAN,    "flip screen left-right" },
	{ OPTION_FLIPY,                                      "0",         OPTION_BOOLEAN,    "flip screen upside-down" },

	// artwork options
	{ NULL,                                              NULL,        OPTION_HEADER,     "CORE ARTWORK OPTIONS" },
	{ OPTION_ARTWORK_CROP ";artcrop",                    "0",         OPTION_BOOLEAN,    "crop artwork to game screen size" },
	{ OPTION_USE_BACKDROPS ";backdrop",                  "1",         OPTION_BOOLEAN,    "enable backdrops if artwork is enabled and available" },
	{ OPTION_USE_OVERLAYS ";overlay",                    "1",         OPTION_BOOLEAN,    "enable overlays if artwork is enabled and available" },
	{ OPTION_USE_BEZELS ";bezel",                        "1",         OPTION_BOOLEAN,    "enable bezels if artwork is enabled and available" },
	{ OPTION_USE_CPANELS ";cpanel",                      "1",         OPTION_BOOLEAN,    "enable cpanels if artwork is enabled and available" },
	{ OPTION_USE_MARQUEES ";marquee",                    "1",         OPTION_BOOLEAN,    "enable marquees if artwork is enabled and available" },

	// screen options
	{ NULL,                                              NULL,        OPTION_HEADER,     "CORE SCREEN OPTIONS" },
	{ OPTION_BRIGHTNESS "(0.1-2.0)",                     "1.0",       OPTION_FLOAT,      "default game screen brightness correction" },
	{ OPTION_CONTRAST "(0.1-2.0)",                       "1.0",       OPTION_FLOAT,      "default game screen contrast correction" },
	{ OPTION_GAMMA "(0.1-3.0)",                          "1.0",       OPTION_FLOAT,      "default game screen gamma correction" },
	{ OPTION_PAUSE_BRIGHTNESS "(0.0-1.0)",               "0.65",      OPTION_FLOAT,      "amount to scale the screen brightness when paused" },
	{ OPTION_EFFECT,                                     "none",      OPTION_STRING,     "name of a PNG file to use for visual effects, or 'none'" },

	// vector options
	{ NULL,                                              NULL,        OPTION_HEADER,     "CORE VECTOR OPTIONS" },
	{ OPTION_ANTIALIAS ";aa",                            "1",         OPTION_BOOLEAN,    "use antialiasing when drawing vectors" },
	{ OPTION_BEAM,                                       "1.0",       OPTION_FLOAT,      "set vector beam width" },
	{ OPTION_FLICKER,                                    "0",         OPTION_FLOAT,      "set vector flicker effect" },

	// sound options
	{ NULL,                                              NULL,        OPTION_HEADER,     "CORE SOUND OPTIONS" },
	{ OPTION_SAMPLERATE ";sr(1000-1000000)",             "48000",     OPTION_INTEGER,    "set sound output sample rate" },
	{ OPTION_SAMPLES,                                    "1",         OPTION_BOOLEAN,    "enable the use of external samples if available" },
	{ OPTION_VOLUME ";vol",                              "0",         OPTION_INTEGER,    "sound volume in decibels (-32 min, 0 max)" },

	// input options
	{ NULL,                                              NULL,        OPTION_HEADER,     "CORE INPUT OPTIONS" },
	{ OPTION_COIN_LOCKOUT ";coinlock",                   "1",         OPTION_BOOLEAN,    "enable coin lockouts to actually lock out coins" },
	{ OPTION_CTRLR,                                      NULL,        OPTION_STRING,     "preconfigure for specified controller" },
	{ OPTION_MOUSE,                                      "0",         OPTION_BOOLEAN,    "enable mouse input" },
	{ OPTION_JOYSTICK ";joy",                            "1",         OPTION_BOOLEAN,    "enable joystick input" },
	{ OPTION_LIGHTGUN ";gun",                            "0",         OPTION_BOOLEAN,    "enable lightgun input" },
	{ OPTION_MULTIKEYBOARD ";multikey",                  "0",         OPTION_BOOLEAN,    "enable separate input from each keyboard device (if present)" },
	{ OPTION_MULTIMOUSE,                                 "0",         OPTION_BOOLEAN,    "enable separate input from each mouse device (if present)" },
	{ OPTION_STEADYKEY ";steady",                        "0",         OPTION_BOOLEAN,    "enable steadykey support" },
	{ OPTION_UI_ACTIVE,                                  "0",         OPTION_BOOLEAN,    "enable user interface on top of emulated keyboard (if present)" },
	{ OPTION_OFFSCREEN_RELOAD ";reload",                 "0",         OPTION_BOOLEAN,    "convert lightgun button 2 into offscreen reload" },
	{ OPTION_JOYSTICK_MAP ";joymap",                     "auto",      OPTION_STRING,     "explicit joystick map, or auto to auto-select" },
	{ OPTION_JOYSTICK_DEADZONE ";joy_deadzone;jdz",      "0.3",       OPTION_FLOAT,      "center deadzone range for joystick where change is ignored (0.0 center, 1.0 end)" },
	{ OPTION_JOYSTICK_SATURATION ";joy_saturation;jsat", "0.85",      OPTION_FLOAT,      "end of axis saturation range for joystick where change is ignored (0.0 center, 1.0 end)" },
	{ OPTION_NATURAL_KEYBOARD ";nat",                    "0",         OPTION_BOOLEAN,    "specifies whether to use a natural keyboard or not" },
	{ OPTION_JOYSTICK_CONTRADICTORY ";joy_contradictory","0",         OPTION_BOOLEAN,    "enable contradictory direction digital joystick input at the same time" },
	{ OPTION_COIN_IMPULSE,                               "0",         OPTION_INTEGER,    "set coin impulse time (n<0 disable impulse, n==0 obey driver, 0<n set time n)" },

	// input autoenable options
	{ NULL,                                              NULL,        OPTION_HEADER,     "CORE INPUT AUTOMATIC ENABLE OPTIONS" },
	{ OPTION_PADDLE_DEVICE ";paddle",                    "keyboard",  OPTION_STRING,     "enable (none|keyboard|mouse|lightgun|joystick) if a paddle control is present" },
	{ OPTION_ADSTICK_DEVICE ";adstick",                  "keyboard",  OPTION_STRING,     "enable (none|keyboard|mouse|lightgun|joystick) if an analog joystick control is present" },
	{ OPTION_PEDAL_DEVICE ";pedal",                      "keyboard",  OPTION_STRING,     "enable (none|keyboard|mouse|lightgun|joystick) if a pedal control is present" },
	{ OPTION_DIAL_DEVICE ";dial",                        "keyboard",  OPTION_STRING,     "enable (none|keyboard|mouse|lightgun|joystick) if a dial control is present" },
	{ OPTION_TRACKBALL_DEVICE ";trackball",              "keyboard",  OPTION_STRING,     "enable (none|keyboard|mouse|lightgun|joystick) if a trackball control is present" },
	{ OPTION_LIGHTGUN_DEVICE,                            "keyboard",  OPTION_STRING,     "enable (none|keyboard|mouse|lightgun|joystick) if a lightgun control is present" },
	{ OPTION_POSITIONAL_DEVICE,                          "keyboard",  OPTION_STRING,     "enable (none|keyboard|mouse|lightgun|joystick) if a positional control is present" },
	{ OPTION_MOUSE_DEVICE,                               "mouse",     OPTION_STRING,     "enable (none|keyboard|mouse|lightgun|joystick) if a mouse control is present" },

	// debugging options
	{ NULL,                                              NULL,        OPTION_HEADER,     "CORE DEBUGGING OPTIONS" },
	{ OPTION_VERBOSE ";v",                               "0",         OPTION_BOOLEAN,    "display additional diagnostic information" },
	{ OPTION_LOG,                                        "0",         OPTION_BOOLEAN,    "generate an error.log file" },
	{ OPTION_OSLOG,                                      "0",         OPTION_BOOLEAN,    "output error.log data to the system debugger" },
	{ OPTION_DEBUG ";d",                                 "0",         OPTION_BOOLEAN,    "enable/disable debugger" },
	{ OPTION_UPDATEINPAUSE,                              "0",         OPTION_BOOLEAN,    "keep calling video updates while in pause" },
	{ OPTION_DEBUGSCRIPT,                                NULL,        OPTION_STRING,     "script for debugger" },

	// comm options
	{ NULL,                                              NULL,        OPTION_HEADER,     "CORE COMM OPTIONS" },
	{ OPTION_COMM_LOCAL_HOST,                            "0.0.0.0",   OPTION_STRING,     "local address to bind to" },
	{ OPTION_COMM_LOCAL_PORT,                            "15112",     OPTION_STRING,     "local port to bind to" },
	{ OPTION_COMM_REMOTE_HOST,                           "127.0.0.1", OPTION_STRING,     "remote address to connect to" },
	{ OPTION_COMM_REMOTE_PORT,                           "15112",     OPTION_STRING,     "remote port to connect to" },

	// misc options
	{ NULL,                                              NULL,        OPTION_HEADER,     "CORE MISC OPTIONS" },
	{ OPTION_DRC,                                        "1",         OPTION_BOOLEAN,    "enable DRC cpu core if available" },
	{ OPTION_DRC_USE_C,                                  "0",         OPTION_BOOLEAN,    "force DRC use C backend" },
	{ OPTION_DRC_LOG_UML,                                "0",         OPTION_BOOLEAN,    "write DRC UML disassembly log" },
	{ OPTION_DRC_LOG_NATIVE,                             "0",         OPTION_BOOLEAN,    "write DRC native disassembly log" },
	{ OPTION_BIOS,                                       NULL,        OPTION_STRING,     "select the system BIOS to use" },
	{ OPTION_CHEAT ";c",                                 "0",         OPTION_BOOLEAN,    "enable cheat subsystem" },
	{ OPTION_SKIP_GAMEINFO,                              "0",         OPTION_BOOLEAN,    "skip displaying the information screen at startup" },
	{ OPTION_UI_FONT,                                    "default",   OPTION_STRING,     "specify a font to use" },
	{ OPTION_RAMSIZE ";ram",                             NULL,        OPTION_STRING,     "size of RAM (if supported by driver)" },
	{ OPTION_CONFIRM_QUIT,                               "1",         OPTION_BOOLEAN,    "display confirm quit screen on exit" },
	{ OPTION_UI_MOUSE,                                   "1",         OPTION_BOOLEAN,    "display ui mouse cursor" },
	{ OPTION_AUTOBOOT_COMMAND ";ab",                     NULL,        OPTION_STRING,     "command to execute after machine boot" },
	{ OPTION_AUTOBOOT_DELAY,                             "2",         OPTION_INTEGER,    "timer delay in sec to trigger command execution on autoboot" },
	{ OPTION_AUTOBOOT_SCRIPT ";script",                  NULL,        OPTION_STRING,     "lua script to execute after machine boot" },
	{ OPTION_HTTP,                                       "0",         OPTION_BOOLEAN,    "enable local http server" },
	{ OPTION_HTTP_PORT,                                  "8080",      OPTION_STRING,     "http server listener port" },
	{ OPTION_HTTP_PATH,                                  "web",       OPTION_STRING,     "path to web files" },
	{ OPTION_CONSOLE,                                    "0",         OPTION_BOOLEAN,    "enable emulator LUA console" },
	{ NULL }
};



//**************************************************************************
//  EMU OPTIONS
//**************************************************************************

//-------------------------------------------------
//  emu_options - constructor
//-------------------------------------------------

emu_options::emu_options()
: mewui_options()
, m_coin_impulse(0)
, m_joystick_contradictory(false)
, m_sleep(true)
, m_refresh_speed(false)
{
	add_entries(emu_options::s_option_entries);
}


//-------------------------------------------------
//  add_slot_options - add all of the slot
//  options for the configured system
//-------------------------------------------------

bool emu_options::add_slot_options(bool isfirstpass)
{
	// look up the system configured by name; if no match, do nothing
	const game_driver *cursystem = system();
	if (cursystem == NULL)
		return false;
	machine_config config(*cursystem, *this);

	// iterate through all slot devices
	bool first = true;

	// create the configuration
	int starting_count = options_count();
	slot_interface_iterator iter(config.root_device());
	for (const device_slot_interface *slot = iter.first(); slot != NULL; slot = iter.next())
	{
		// skip fixed slots
		if (slot->fixed())
			continue;

		// first device? add the header as to be pretty
		if (isfirstpass && first)
			add_entry(NULL, "SLOT DEVICES", OPTION_HEADER | OPTION_FLAG_DEVICE);
		first = false;

		// retrieve info about the device instance
		const char *name = slot->device().tag() + 1;
		if (!exists(name))
		{
			// add the option
			UINT32 flags = OPTION_STRING | OPTION_FLAG_DEVICE;
			const char *defvalue = slot->default_option();
			if (defvalue != NULL)
			{
				const device_slot_option *option = slot->option(defvalue);
				if (option != NULL && !option->selectable())
					flags |= OPTION_FLAG_INTERNAL;
			}
			add_entry(name, NULL, flags, defvalue, true);
		}
	}
	return (options_count() != starting_count);
}


//-------------------------------------------------
//  update_slot_options - update slot values
//  depending of image mounted
//-------------------------------------------------

void emu_options::update_slot_options()
{
	// look up the system configured by name; if no match, do nothing
	const game_driver *cursystem = system();
	if (cursystem == NULL)
		return;
	machine_config config(*cursystem, *this);

	// iterate through all slot devices
	slot_interface_iterator iter(config.root_device());
	for (device_slot_interface *slot = iter.first(); slot != NULL; slot = iter.next())
	{
		// retrieve info about the device instance
		const char *name = slot->device().tag() + 1;
		if (exists(name) && slot->first_option() != NULL)
		{
			std::string defvalue;
			slot->get_default_card_software(defvalue);
			if (defvalue.length() > 0)
			{
				set_default_value(name, defvalue.c_str());
				const device_slot_option *option = slot->option(defvalue.c_str());
				set_flag(name, ~OPTION_FLAG_INTERNAL, (option != NULL && !option->selectable()) ? OPTION_FLAG_INTERNAL : 0);
			}
		}
	}
	while (add_slot_options(false));
	add_device_options(false);
}


//-------------------------------------------------
//  add_device_options - add all of the device
//  options for the configured system
//-------------------------------------------------

void emu_options::add_device_options(bool isfirstpass)
{
	// look up the system configured by name; if no match, do nothing
	const game_driver *cursystem = system();
	if (cursystem == NULL)
		return;
	machine_config config(*cursystem, *this);

	// iterate through all image devices
	bool first = true;
	image_interface_iterator iter(config.root_device());
	for (const device_image_interface *image = iter.first(); image != NULL; image = iter.next())
	{
		// first device? add the header as to be pretty
		if (first && isfirstpass)
			add_entry(NULL, "IMAGE DEVICES", OPTION_HEADER | OPTION_FLAG_DEVICE);
		first = false;

		// retrieve info about the device instance
		std::string option_name;
		strprintf(option_name, "%s;%s", image->instance_name(), image->brief_instance_name());
		if (strcmp(image->device_typename(image->image_type()), image->instance_name()) == 0)
			strcatprintf(option_name, ";%s1;%s1", image->instance_name(), image->brief_instance_name());

		// add the option
		if (!exists(image->instance_name()))
			add_entry(option_name.c_str(), NULL, OPTION_STRING | OPTION_FLAG_DEVICE, NULL, true);
	}
}


//-------------------------------------------------
//  remove_device_options - remove device options
//-------------------------------------------------

void emu_options::remove_device_options()
{
	// iterate through options and remove interesting ones
	entry *nextentry;
	for (entry *curentry = first(); curentry != NULL; curentry = nextentry)
	{
		// pre-fetch the next entry in case we delete this one
		nextentry = curentry->next();

		// if this is a device option, nuke it
		if ((curentry->flags() & OPTION_FLAG_DEVICE) != 0)
			remove_entry(*curentry);
	}
}


//-------------------------------------------------
//  parse_slot_devices - parse the command line
//  and update slot and image devices
//-------------------------------------------------

bool emu_options::parse_slot_devices(int argc, char *argv[], std::string &error_string, const char *name, const char *value)
{
	// an initial parse to capture the initial set of values
	bool result = core_options::parse_command_line(argc, argv, OPTION_PRIORITY_CMDLINE, error_string);

	// keep adding slot options until we stop seeing new stuff
	bool isfirstpass = true;
	while (add_slot_options(isfirstpass))
	{
		result = core_options::parse_command_line(argc, argv, OPTION_PRIORITY_CMDLINE, error_string);
		isfirstpass = false;
	}

	// add device options and reparse
	add_device_options(true);
	if (name != NULL && exists(name))
		set_value(name, value, OPTION_PRIORITY_CMDLINE, error_string);
	result = core_options::parse_command_line(argc, argv, OPTION_PRIORITY_CMDLINE, error_string);

	int num = 0;
	do {
		num = options_count();
		update_slot_options();
		result = core_options::parse_command_line(argc, argv, OPTION_PRIORITY_CMDLINE, error_string);
	} while (num != options_count());

	update_cached_options();

	return result;
}


//-------------------------------------------------
//  parse_command_line - parse the command line
//  and update the devices
//-------------------------------------------------

bool emu_options::parse_command_line(int argc, char *argv[], std::string &error_string)
{
	// parse as normal
	core_options::parse_command_line(argc, argv, OPTION_PRIORITY_CMDLINE, error_string);
	bool result = parse_slot_devices(argc, argv, error_string, NULL, NULL);
	update_cached_options();
	return result;
}


//-------------------------------------------------
//  parse_standard_inis - parse the standard set
//  of INI files
//-------------------------------------------------

void emu_options::parse_standard_inis(std::string &error_string)
{
	// start with an empty string
	error_string.clear();

	// parse the INI file defined by the platform (e.g., "mame.ini")
	// we do this twice so that the first file can change the INI path
	parse_one_ini(emulator_info::get_configname(), OPTION_PRIORITY_MAME_INI);
	parse_one_ini(emulator_info::get_configname(), OPTION_PRIORITY_MAME_INI, &error_string);

	// debug mode: parse "debug.ini" as well
	if (debug())
		parse_one_ini("debug", OPTION_PRIORITY_DEBUG_INI, &error_string);

	// if we have a valid system driver, parse system-specific INI files
	const game_driver *cursystem = system();
	if (cursystem == NULL)
		return;

	// parse "vertical.ini" or "horizont.ini"
	if (cursystem->flags & ORIENTATION_SWAP_XY)
		parse_one_ini("vertical", OPTION_PRIORITY_ORIENTATION_INI, &error_string);
	else
		parse_one_ini("horizont", OPTION_PRIORITY_ORIENTATION_INI, &error_string);

	if (cursystem->flags & MACHINE_TYPE_ARCADE)
		parse_one_ini("arcade", OPTION_PRIORITY_SYSTYPE_INI, &error_string);
	else if (cursystem->flags & MACHINE_TYPE_CONSOLE)
		parse_one_ini("console", OPTION_PRIORITY_SYSTYPE_INI, &error_string);
	else if (cursystem->flags & MACHINE_TYPE_COMPUTER)
		parse_one_ini("computer", OPTION_PRIORITY_SYSTYPE_INI, &error_string);
	else if (cursystem->flags & MACHINE_TYPE_OTHER)
		parse_one_ini("othersys", OPTION_PRIORITY_SYSTYPE_INI, &error_string);

	// parse "vector.ini" for vector games
	{
		machine_config config(*cursystem, *this);
		screen_device_iterator iter(config.root_device());
		for (const screen_device *device = iter.first(); device != NULL; device = iter.next())
			if (device->screen_type() == SCREEN_TYPE_VECTOR)
			{
				parse_one_ini("vector", OPTION_PRIORITY_VECTOR_INI, &error_string);
				break;
			}
	}

	// next parse "source/<sourcefile>.ini"; if that doesn't exist, try <sourcefile>.ini
	std::string sourcename;
	core_filename_extract_base(sourcename, cursystem->source_file, true).insert(0, "source" PATH_SEPARATOR);
	if (!parse_one_ini(sourcename.c_str(), OPTION_PRIORITY_SOURCE_INI, &error_string))
	{
		core_filename_extract_base(sourcename, cursystem->source_file, true);
		parse_one_ini(sourcename.c_str(), OPTION_PRIORITY_SOURCE_INI, &error_string);
	}

	// then parse the grandparent, parent, and system-specific INIs
	int parent = driver_list::clone(*cursystem);
	int gparent = (parent != -1) ? driver_list::clone(parent) : -1;
	if (gparent != -1)
		parse_one_ini(driver_list::driver(gparent).name, OPTION_PRIORITY_GPARENT_INI, &error_string);
	if (parent != -1)
		parse_one_ini(driver_list::driver(parent).name, OPTION_PRIORITY_PARENT_INI, &error_string);
	parse_one_ini(cursystem->name, OPTION_PRIORITY_DRIVER_INI, &error_string);

	// Re-evaluate slot options after loading ini files
	update_slot_options();

	update_cached_options();
}


//-------------------------------------------------
//  system - return a pointer to the specified
//  system driver, or NULL if no match
//-------------------------------------------------

const game_driver *emu_options::system() const
{
	std::string tempstr;
	int index = driver_list::find(core_filename_extract_base(tempstr, system_name(), true).c_str());
	return (index != -1) ? &driver_list::driver(index) : NULL;
}


//-------------------------------------------------
//  set_system_name - set a new system name
//-------------------------------------------------

void emu_options::set_system_name(const char *name)
{
	// remember the original system name
	std::string old_system_name(system_name());

	// if the system name changed, fix up the device options
	if (old_system_name.compare(name)!=0)
	{
		// first set the new name
		std::string error;
		set_value(OPTION_SYSTEMNAME, name, OPTION_PRIORITY_CMDLINE, error);
		assert(error.empty());

		// remove any existing device options and then add them afresh
		remove_device_options();
		if (add_slot_options(true))
			while (add_slot_options(false)) { }

		// then add the options
		add_device_options(true);
		int num = 0;
		do {
			num = options_count();
			update_slot_options();
		} while(num != options_count());
	}
}

//-------------------------------------------------
//  parse_one_ini - parse a single INI file
//-------------------------------------------------

bool emu_options::parse_one_ini(const char *basename, int priority, std::string *error_string)
{
	// don't parse if it has been disabled
	if (!read_config())
		return false;

	// open the file; if we fail, that's ok
	emu_file file(ini_path(), OPEN_FLAG_READ);
	file_error filerr = file.open(basename, ".ini");
	if (filerr != FILERR_NONE)
		return false;

	// parse the file
	osd_printf_verbose("Parsing %s.ini\n", basename);
	std::string error;
	bool result = parse_ini_file(file, priority, OPTION_PRIORITY_DRIVER_INI, error);

	// append errors if requested
	if (!error.empty() && error_string != NULL)
		strcatprintf(*error_string, "While parsing %s:\n%s\n", file.fullpath(), error.c_str());

	return result;
}


const char *emu_options::main_value(std::string &buffer, const char *name) const
{
	buffer = value(name);
	int pos = buffer.find_first_of(',');
	if (pos != -1)
		buffer = buffer.substr(0, pos);
	return buffer.c_str();
}

const char *emu_options::sub_value(std::string &buffer, const char *name, const char *subname) const
{
	std::string tmp = std::string(",").append(subname).append("=");
	buffer = value(name);
	int pos = buffer.find(tmp.c_str());
	if (pos != -1)
	{
		int endpos = buffer.find_first_of(',', pos + 1);
		if (endpos == -1)
			endpos = buffer.length();
		buffer = buffer.substr(pos + tmp.length(), endpos - pos - tmp.length());
	}
	else
		buffer.clear();
	return buffer.c_str();
}


//-------------------------------------------------
//  update_cached_options - to prevent tagmap
//    lookups keep copies of frequently requested
//    options in member variables.
//-------------------------------------------------

void emu_options::update_cached_options()
{
	m_coin_impulse = int_value(OPTION_COIN_IMPULSE);
	m_joystick_contradictory = bool_value(OPTION_JOYSTICK_CONTRADICTORY);
	m_sleep = bool_value(OPTION_SLEEP);
	m_refresh_speed = bool_value(OPTION_REFRESHSPEED);
}
