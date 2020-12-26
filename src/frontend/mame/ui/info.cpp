// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Nathan Woods
/***************************************************************************

    ui/info.cpp

    System and image info screens

***************************************************************************/

#include "emu.h"

#include "ui/info.h"
#include "ui/ui.h"

#include "drivenum.h"
#include "romload.h"
#include "softlist.h"
#include "emuopts.h"


namespace ui {

namespace {

constexpr machine_flags::type MACHINE_ERRORS    = machine_flags::NOT_WORKING | machine_flags::MECHANICAL;
constexpr machine_flags::type MACHINE_WARNINGS  = machine_flags::NO_COCKTAIL | machine_flags::REQUIRES_ARTWORK;
constexpr machine_flags::type MACHINE_BTANB     = machine_flags::NO_SOUND_HW | machine_flags::IS_INCOMPLETE;

constexpr std::pair<device_t::feature_type, char const *> FEATURE_NAMES[] = {
		{ device_t::feature::PROTECTION,    __("protection")            },
		{ device_t::feature::TIMING,        __("timing")                },
		{ device_t::feature::GRAPHICS,      __("graphics")              },
		{ device_t::feature::PALETTE,       __("color palette")         },
		{ device_t::feature::SOUND,         __("sound")                 },
		{ device_t::feature::CAPTURE,       __("capture hardware")      },
		{ device_t::feature::CAMERA,        __("camera")                },
		{ device_t::feature::MICROPHONE,    __("microphone")            },
		{ device_t::feature::CONTROLS,      __("controls")              },
		{ device_t::feature::KEYBOARD,      __("keyboard")              },
		{ device_t::feature::MOUSE,         __("mouse")                 },
		{ device_t::feature::MEDIA,         __("media")                 },
		{ device_t::feature::DISK,          __("disk")                  },
		{ device_t::feature::PRINTER,       __("printer")               },
		{ device_t::feature::TAPE,          __("magnetic tape")         },
		{ device_t::feature::PUNCH,         __("punch tape")            },
		{ device_t::feature::DRUM,          __("magnetic drum")         },
		{ device_t::feature::ROM,           __("solid state storage")   },
		{ device_t::feature::COMMS,         __("communications")        },
		{ device_t::feature::LAN,           __("LAN")                   },
		{ device_t::feature::WAN,           __("WAN")                   } };

} // anonymous namespace



//-------------------------------------------------
//  machine_static_info - constructors
//-------------------------------------------------

machine_static_info::machine_static_info(const ui_options &options, machine_config const &config)
	: machine_static_info(options, config, nullptr)
{
}

machine_static_info::machine_static_info(const ui_options &options, machine_config const &config, ioport_list const &ports)
	: machine_static_info(options, config, &ports)
{
}

machine_static_info::machine_static_info(const ui_options &options, machine_config const &config, ioport_list const *ports)
	: m_options(options)
	, m_flags(config.gamedrv().flags)
	, m_unemulated_features(config.gamedrv().type.unemulated_features())
	, m_imperfect_features(config.gamedrv().type.imperfect_features())
	, m_has_bioses(false)
	, m_has_dips(false)
	, m_has_configs(false)
	, m_has_keyboard(false)
	, m_has_test_switch(false)
	, m_has_analog(false)
{
	ioport_list local_ports;
	std::string sink;
	for (device_t &device : device_enumerator(config.root_device()))
	{
		// the "no sound hardware" warning doesn't make sense when you plug in a sound card
		if (dynamic_cast<device_sound_interface *>(&device))
			m_flags &= ~::machine_flags::NO_SOUND_HW;

		// build overall emulation status
		m_unemulated_features |= device.type().unemulated_features();
		m_imperfect_features |= device.type().imperfect_features();

		// look for BIOS options
		device_t const *const parent(device.owner());
		device_slot_interface const *const slot(dynamic_cast<device_slot_interface const *>(parent));
		if (!parent || (slot && (slot->get_card_device() == &device)))
		{
			for (tiny_rom_entry const *rom = device.rom_region(); !m_has_bioses && rom && !ROMENTRY_ISEND(rom); ++rom)
			{
				if (ROMENTRY_ISSYSTEM_BIOS(rom))
					m_has_bioses = true;
			}
		}

		// if we don't have ports passed in, build here
		if (!ports)
			local_ports.append(device, sink);
	}

	// suppress "requires external artwork" warning when external artwork was loaded
	if (config.root_device().has_running_machine())
	{
		for (render_target *target = config.root_device().machine().render().first_target(); target != nullptr; target = target->next())
			if (!target->hidden() && target->external_artwork())
			{
				m_flags &= ~::machine_flags::REQUIRES_ARTWORK;
				break;
			}
	}

	// unemulated trumps imperfect when aggregating (always be pessimistic)
	m_imperfect_features &= ~m_unemulated_features;

	// scan the input port array to see what options we need to enable
	for (ioport_list::value_type const &port : (ports ? *ports : local_ports))
	{
		for (ioport_field const &field : port.second->fields())
		{
			switch (field.type())
			{
			case IPT_DIPSWITCH: m_has_dips = true;          break;
			case IPT_CONFIG:    m_has_configs = true;       break;
			case IPT_KEYBOARD:  m_has_keyboard = true;      break;
			case IPT_SERVICE:   m_has_test_switch = true;   break;
			default: break;
			}
			if (field.is_analog())
				m_has_analog = true;
		}
	}
}


//-------------------------------------------------
//  has_warnings - returns true if the system has
//  issues that warrant a yellow/red message
//-------------------------------------------------

bool machine_static_info::has_warnings() const
{
	return (machine_flags() & (MACHINE_ERRORS | MACHINE_WARNINGS)) || unemulated_features() || imperfect_features();
}


//-------------------------------------------------
//  has_severe_warnings - returns true if the
//  system has issues that warrant a red message
//-------------------------------------------------

bool machine_static_info::has_severe_warnings() const
{
	return
			(machine_flags() & MACHINE_ERRORS) ||
			(unemulated_features() & (device_t::feature::PROTECTION | device_t::feature::GRAPHICS | device_t::feature::SOUND)) ||
			(imperfect_features() & device_t::feature::PROTECTION);
}


//-------------------------------------------------
//  status_color - returns suitable colour for
//  driver status box
//-------------------------------------------------

rgb_t machine_static_info::status_color() const
{
	if (has_severe_warnings())
		return UI_RED_COLOR;
	else if ((machine_flags() & MACHINE_WARNINGS & ~::machine_flags::REQUIRES_ARTWORK) || unemulated_features() || imperfect_features())
		return UI_YELLOW_COLOR;
	else
		return UI_GREEN_COLOR;
}


//-------------------------------------------------
//  warnings_color - returns suitable colour for
//  warning message based on severity
//-------------------------------------------------

rgb_t machine_static_info::warnings_color() const
{
	if (has_severe_warnings())
		return UI_RED_COLOR;
	else if ((machine_flags() & MACHINE_WARNINGS) || unemulated_features() || imperfect_features())
		return UI_YELLOW_COLOR;
	else
		return m_options.background_color();
}



//-------------------------------------------------
//  machine_info - constructor
//-------------------------------------------------

machine_info::machine_info(running_machine &machine)
	: machine_static_info(dynamic_cast<mame_ui_manager *>(&machine.ui())->options(), machine.config(), machine.ioport().ports())
	, m_machine(machine)
{
}


/***************************************************************************
    TEXT GENERATORS
***************************************************************************/

//-------------------------------------------------
//  warnings_string - print the warning flags
//  text to the given buffer
//-------------------------------------------------

std::string machine_info::warnings_string() const
{
	std::ostringstream buf;

	// add a warning if any ROMs were loaded with warnings
	if (m_machine.rom_load().warnings() > 0)
		buf << _("One or more ROMs/CHDs for this machine are incorrect. The machine may not run correctly.\n");

	if (!m_machine.rom_load().software_load_warnings_message().empty())
		buf << m_machine.rom_load().software_load_warnings_message();

	// if we have at least one warning flag, print the general header
	if ((m_machine.rom_load().knownbad() > 0) || (machine_flags() & (MACHINE_ERRORS | MACHINE_WARNINGS | MACHINE_BTANB)) || unemulated_features() || imperfect_features())
	{
		if (!buf.str().empty())
			buf << '\n';
		buf << _("There are known problems with this machine\n\n");
	}

	// add a warning if any ROMs are flagged BAD_DUMP/NO_DUMP
	if (m_machine.rom_load().knownbad() > 0)
		buf << _("One or more ROMs/CHDs for this machine have not been correctly dumped.\n");

	// add line for unemulated features
	if (unemulated_features())
	{
		buf << _("Completely unemulated features: ");
		bool first = true;
		for (auto const &feature : FEATURE_NAMES)
		{
			if (unemulated_features() & feature.first)
			{
				util::stream_format(buf, first ? _("%s") : _(", %s"), _(feature.second));
				first = false;
			}
		}
		buf << '\n';
	}

	// add line for imperfect features
	if (imperfect_features())
	{
		buf << _("Imperfectly emulated features: ");
		bool first = true;
		for (auto const &feature : FEATURE_NAMES)
		{
			if (imperfect_features() & feature.first)
			{
				util::stream_format(buf, first ? _("%s") : _(", %s"), _(feature.second));
				first = false;
			}
		}
		buf << '\n';
	}

	// add one line per machine warning flag
	if (machine_flags() & ::machine_flags::NO_COCKTAIL)
		buf << _("Screen flipping in cocktail mode is not supported.\n");
	if (machine_flags() & ::machine_flags::REQUIRES_ARTWORK)
		buf << _("This machine requires external artwork files.\n");
	if (machine_flags() & ::machine_flags::IS_INCOMPLETE)
		buf << _("This machine was never completed. It may exhibit strange behavior or missing elements that are not bugs in the emulation.\n");
	if (machine_flags() & ::machine_flags::NO_SOUND_HW)
		buf << _("This machine has no sound hardware, MAME will produce no sounds, this is expected behaviour.\n");

	// these are more severe warnings
	if (machine_flags() & ::machine_flags::NOT_WORKING)
		buf << _("\nTHIS MACHINE DOESN'T WORK. The emulation for this machine is not yet complete. There is nothing you can do to fix this problem except wait for the developers to improve the emulation.\n");
	if (machine_flags() & ::machine_flags::MECHANICAL)
		buf << _("\nElements of this machine cannot be emulated as they require physical interaction or consist of mechanical devices. It is not possible to fully experience this machine.\n");

	if ((machine_flags() & MACHINE_ERRORS) || ((m_machine.system().type.unemulated_features() | m_machine.system().type.imperfect_features()) & device_t::feature::PROTECTION))
	{
		// find the parent of this driver
		driver_enumerator drivlist(m_machine.options());
		int maindrv = drivlist.find(m_machine.system());
		int clone_of = drivlist.non_bios_clone(maindrv);
		if (clone_of != -1)
			maindrv = clone_of;

		// scan the driver list for any working clones and add them
		bool foundworking = false;
		while (drivlist.next())
		{
			if (drivlist.current() == maindrv || drivlist.clone() == maindrv)
			{
				game_driver const &driver(drivlist.driver());
				if (!(driver.flags & MACHINE_ERRORS) && !((driver.type.unemulated_features() | driver.type.imperfect_features()) & device_t::feature::PROTECTION))
				{
					// this one works, add a header and display the name of the clone
					if (!foundworking)
						util::stream_format(buf, _("\n\nThere are working clones of this machine: %s"), driver.name);
					else
						util::stream_format(buf, _(", %s"), driver.name);
					foundworking = true;
				}
			}
		}
		if (foundworking)
			buf << '\n';
	}

	return buf.str();
}


//-------------------------------------------------
//  game_info_string - return the game info text
//-------------------------------------------------

std::string machine_info::game_info_string() const
{
	std::ostringstream buf;

	// print description, manufacturer, and CPU:
	util::stream_format(buf, _("%1$s\n%2$s %3$s\nDriver: %4$s\n\nCPU:\n"),
			m_machine.system().type.fullname(),
			m_machine.system().year,
			m_machine.system().manufacturer,
			core_filename_extract_base(m_machine.system().type.source()));

	// loop over all CPUs
	execute_interface_enumerator execiter(m_machine.root_device());
	std::unordered_set<std::string> exectags;
	for (device_execute_interface &exec : execiter)
	{
		if (!exectags.insert(exec.device().tag()).second)
			continue;
		// get cpu specific clock that takes internal multiplier/dividers into account
		u32 clock = exec.device().clock();

		// count how many identical CPUs we have
		int count = 1;
		const char *name = exec.device().name();
		for (device_execute_interface &scan : execiter)
		{
			if (exec.device().type() == scan.device().type() && strcmp(name, scan.device().name()) == 0 && exec.device().clock() == scan.device().clock())
				if (exectags.insert(scan.device().tag()).second)
					count++;
		}

		std::string hz(std::to_string(clock));
		int d = (clock >= 1'000'000'000) ? 9 : (clock >= 1'000'000) ? 6 : (clock >= 1000) ? 3 : 0;
		if (d > 0)
		{
			size_t dpos = hz.length() - d;
			hz.insert(dpos, ".");
			size_t last = hz.find_last_not_of('0');
			hz = hz.substr(0, last + (last != dpos ? 1 : 0));
		}

		// if more than one, prepend a #x in front of the CPU name and display clock
		util::stream_format(buf,
				(count > 1)
					? ((clock != 0) ? "%1$d" UTF8_MULTIPLY "%2$s %3$s" UTF8_NBSP "%4$s\n" : "%1$d" UTF8_MULTIPLY "%2$s\n")
					: ((clock != 0) ? "%2$s %3$s" UTF8_NBSP "%4$s\n" : "%2$s\n"),
				count, name, hz,
				(d == 9) ? _("GHz") : (d == 6) ? _("MHz") : (d == 3) ? _("kHz") : _("Hz"));
	}

	// loop over all sound chips
	sound_interface_enumerator snditer(m_machine.root_device());
	std::unordered_set<std::string> soundtags;
	bool found_sound = false;
	for (device_sound_interface &sound : snditer)
	{
		if (!sound.issound() || !soundtags.insert(sound.device().tag()).second)
			continue;

		// append the Sound: string
		if (!found_sound)
			buf << _("\nSound:\n");
		found_sound = true;

		// count how many identical sound chips we have
		int count = 1;
		for (device_sound_interface &scan : snditer)
		{
			if (sound.device().type() == scan.device().type() && sound.device().clock() == scan.device().clock())
				if (soundtags.insert(scan.device().tag()).second)
					count++;
		}

		const u32 clock = sound.device().clock();
		std::string hz(std::to_string(clock));
		int d = (clock >= 1'000'000'000) ? 9 : (clock >= 1'000'000) ? 6 : (clock >= 1000) ? 3 : 0;
		if (d > 0)
		{
			size_t dpos = hz.length() - d;
			hz.insert(dpos, ".");
			size_t last = hz.find_last_not_of('0');
			hz = hz.substr(0, last + (last != dpos ? 1 : 0));
		}

		// if more than one, prepend a #x in front of the soundchip name and display clock
		util::stream_format(buf,
				(count > 1)
					? ((clock != 0) ? "%1$d" UTF8_MULTIPLY "%2$s %3$s" UTF8_NBSP "%4$s\n" : "%1$d" UTF8_MULTIPLY "%2$s\n")
					: ((clock != 0) ? "%2$s %3$s" UTF8_NBSP "%4$s\n" : "%2$s\n"),
				count, sound.device().name(), hz,
				(d == 9) ? _("GHz") : (d == 6) ? _("MHz") : (d == 3) ? _("kHz") : _("Hz"));
	}

	// display screen information
	buf << _("\nVideo:\n");
	screen_device_enumerator scriter(m_machine.root_device());
	int scrcount = scriter.count();
	if (scrcount == 0)
		buf << _("None\n");
	else
	{
		for (screen_device &screen : scriter)
		{
			std::string detail;
			if (screen.screen_type() == SCREEN_TYPE_VECTOR)
				detail = _("Vector");
			else
			{
				std::string hz(std::to_string(float(screen.frame_period().as_hz())));
				size_t last = hz.find_last_not_of('0');
				size_t dpos = hz.find_last_of('.');
				hz = hz.substr(0, last + (last != dpos ? 1 : 0));

				const rectangle &visarea = screen.visible_area();
				detail = string_format("%d " UTF8_MULTIPLY " %d (%s) %s" UTF8_NBSP "Hz",
						visarea.width(), visarea.height(),
						(screen.orientation() & ORIENTATION_SWAP_XY) ? "V" : "H",
						hz);
			}

			util::stream_format(buf,
					(scrcount > 1) ? _("%1$s: %2$s\n") : _("%2$s\n"),
					get_screen_desc(screen), detail);
		}
	}

	return buf.str();
}


//-------------------------------------------------
//  get_screen_desc - returns the description for
//  a given screen
//-------------------------------------------------

std::string machine_info::get_screen_desc(screen_device &screen) const
{
	if (screen_device_enumerator(m_machine.root_device()).count() > 1)
		return string_format(_("Screen '%1$s'"), screen.tag());
	else
		return _("Screen");
}



/*-------------------------------------------------
  menu_game_info - handle the game information menu
-------------------------------------------------*/

menu_game_info::menu_game_info(mame_ui_manager &mui, render_container &container) : menu(mui, container)
{
}

menu_game_info::~menu_game_info()
{
}

void menu_game_info::populate(float &customtop, float &custombottom)
{
	item_append(ui().machine_info().game_info_string(), FLAG_MULTILINE, nullptr);
}

void menu_game_info::handle()
{
	// process the menu
	process(0);
}


/*-------------------------------------------------
  menu_warn_info - handle the emulation warnings menu
-------------------------------------------------*/

menu_warn_info::menu_warn_info(mame_ui_manager &mui, render_container &container) : menu(mui, container)
{
}

menu_warn_info::~menu_warn_info()
{
}

void menu_warn_info::populate(float &customtop, float &custombottom)
{
	item_append(ui().machine_info().warnings_string(), FLAG_MULTILINE, nullptr);
}

void menu_warn_info::handle()
{
	// process the menu
	process(0);
}


/*-------------------------------------------------
  menu_image_info - handle the image information menu
-------------------------------------------------*/

menu_image_info::menu_image_info(mame_ui_manager &mui, render_container &container) : menu(mui, container)
{
}

menu_image_info::~menu_image_info()
{
}

void menu_image_info::populate(float &customtop, float &custombottom)
{
	item_append(machine().system().type.fullname(), FLAG_DISABLE, nullptr);
	item_append(std::string(), FLAG_DISABLE, nullptr);

	for (device_image_interface &image : image_interface_enumerator(machine().root_device()))
		image_info(&image);
}

void menu_image_info::handle()
{
	// process the menu
	process(0);
}


/*-------------------------------------------------
  image_info - display image info for a specific
  image interface device
-------------------------------------------------*/

void menu_image_info::image_info(device_image_interface *image)
{
	if (image->exists())
	{
		// display device type and filename
		item_append(image->brief_instance_name(), image->basename(), 0, nullptr);

		// if image has been loaded through softlist, let's add some more info
		if (image->loaded_through_softlist())
		{
			software_info const &swinfo(*image->software_entry());

			// display full name, publisher and year
			item_append(swinfo.longname(), FLAG_DISABLE, nullptr);
			item_append(string_format("%1$s, %2$s", swinfo.publisher(), swinfo.year()), FLAG_DISABLE, nullptr);

			// display supported information, if available
			switch (swinfo.supported())
			{
			case SOFTWARE_SUPPORTED_NO:
				item_append(_("Not supported"), FLAG_DISABLE, nullptr);
				break;
			case SOFTWARE_SUPPORTED_PARTIAL:
				item_append(_("Partially supported"), FLAG_DISABLE, nullptr);
				break;
			default:
				break;
			}
		}
	}
	else
	{
		item_append(image->brief_instance_name(), _("[empty]"), 0, nullptr);
	}
	item_append(std::string(), FLAG_DISABLE, nullptr);
}

} // namespace ui
