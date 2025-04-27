// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Nathan Woods
/***************************************************************************

    ui/info.cpp

    System and image info screens

***************************************************************************/

#include "emu.h"
#include "ui/info.h"

#include "ui/systemlist.h"
#include "ui/ui.h"

#include "infoxml.h"

#include "drivenum.h"
#include "emuopts.h"
#include "romload.h"
#include "screen.h"
#include "softlist.h"
#include "speaker.h"

#include "util/unicode.h"

#include <locale>
#include <set>
#include <sstream>
#include <type_traits>
#include <utility>


namespace ui {

namespace {

constexpr machine_flags::type MACHINE_ERRORS    = machine_flags::NOT_WORKING | machine_flags::MECHANICAL;
constexpr machine_flags::type MACHINE_WARNINGS  = machine_flags::NO_COCKTAIL | machine_flags::REQUIRES_ARTWORK;
constexpr machine_flags::type MACHINE_BTANB     = machine_flags::NO_SOUND_HW | machine_flags::IS_INCOMPLETE;

constexpr std::pair<device_t::feature_type, char const *> FEATURE_NAMES[] = {
		{ device_t::feature::PROTECTION,    N_p("emulation-feature",    "protection")           },
		{ device_t::feature::TIMING,        N_p("emulation-feature",    "timing")               },
		{ device_t::feature::GRAPHICS,      N_p("emulation-feature",    "graphics")             },
		{ device_t::feature::PALETTE,       N_p("emulation-feature",    "color palette")        },
		{ device_t::feature::SOUND,         N_p("emulation-feature",    "sound")                },
		{ device_t::feature::CAPTURE,       N_p("emulation-feature",    "capture hardware")     },
		{ device_t::feature::CAMERA,        N_p("emulation-feature",    "camera")               },
		{ device_t::feature::MICROPHONE,    N_p("emulation-feature",    "microphone")           },
		{ device_t::feature::CONTROLS,      N_p("emulation-feature",    "controls")             },
		{ device_t::feature::KEYBOARD,      N_p("emulation-feature",    "keyboard")             },
		{ device_t::feature::MOUSE,         N_p("emulation-feature",    "mouse")                },
		{ device_t::feature::MEDIA,         N_p("emulation-feature",    "media")                },
		{ device_t::feature::DISK,          N_p("emulation-feature",    "disk")                 },
		{ device_t::feature::PRINTER,       N_p("emulation-feature",    "printer")              },
		{ device_t::feature::TAPE,          N_p("emulation-feature",    "magnetic tape")        },
		{ device_t::feature::PUNCH,         N_p("emulation-feature",    "punch tape")           },
		{ device_t::feature::DRUM,          N_p("emulation-feature",    "magnetic drum")        },
		{ device_t::feature::ROM,           N_p("emulation-feature",    "solid state storage")  },
		{ device_t::feature::COMMS,         N_p("emulation-feature",    "communications")       },
		{ device_t::feature::LAN,           N_p("emulation-feature",    "LAN")                  },
		{ device_t::feature::WAN,           N_p("emulation-feature",    "WAN")                  } };

void get_general_warnings(std::ostream &buf, running_machine &machine, machine_flags::type flags, device_t::feature_type unemulated, device_t::feature_type imperfect)
{
	// add a warning if any ROMs were loaded with warnings
	bool bad_roms(false);
	if (machine.rom_load().warnings() > 0)
	{
		bad_roms = true;
		buf << _("One or more ROMs/disk images for this system are incorrect. The system may not run correctly.\n");
	}
	if (!machine.rom_load().software_load_warnings_message().empty())
	{
		bad_roms = true;
		buf << machine.rom_load().software_load_warnings_message();
	}

	// if we have at least one warning flag, print the general header
	if ((machine.rom_load().knownbad() > 0) || (flags & (MACHINE_ERRORS | MACHINE_WARNINGS | MACHINE_BTANB)) || unemulated || imperfect)
	{
		if (bad_roms)
			buf << '\n';
		buf << _("There are known problems with this system:\n\n");
	}

	// add a warning if any ROMs are flagged BAD_DUMP/NO_DUMP
	if (machine.rom_load().knownbad() > 0)
		buf << _("One or more ROMs/disk images for this system have not been correctly dumped.\n");
}

void get_device_warnings(std::ostream &buf, device_t::feature_type unemulated, device_t::feature_type imperfect)
{
	// add line for unemulated features
	if (unemulated)
	{
		buf << _("Completely unemulated features: ");
		bool first = true;
		for (auto const &feature : FEATURE_NAMES)
		{
			if (unemulated & feature.first)
			{
				util::stream_format(buf, first ? _("%s") : _(", %s"), _("emulation-feature", feature.second));
				first = false;
			}
		}
		buf << '\n';
	}

	// add line for imperfect features
	if (imperfect)
	{
		buf << _("Imperfectly emulated features: ");
		bool first = true;
		for (auto const &feature : FEATURE_NAMES)
		{
			if (imperfect & feature.first)
			{
				util::stream_format(buf, first ? _("%s") : _(", %s"), _("emulation-feature", feature.second));
				first = false;
			}
		}
		buf << '\n';
	}
}

void get_system_warnings(std::ostream &buf, running_machine &machine, machine_flags::type flags, device_t::feature_type unemulated, device_t::feature_type imperfect)
{
	std::streampos start_position = buf.tellp();

	// start with the unemulated/imperfect features
	get_device_warnings(buf, unemulated, imperfect);

	// add one line per machine warning flag
	if (flags & ::machine_flags::NO_COCKTAIL)
		buf << _("Screen flipping in cocktail mode is not supported.\n");
	if (flags & ::machine_flags::REQUIRES_ARTWORK)
		buf << _("This system requires external artwork files.\n");

	// add the 'BTANB' warnings
	if (flags & ::machine_flags::IS_INCOMPLETE)
	{
		if (buf.tellp() > start_position)
			buf << '\n';
		buf << _("This system was never completed. It may exhibit strange behavior or missing elements that are not bugs in the emulation.\n");
	}
	if (flags & ::machine_flags::NO_SOUND_HW)
	{
		if (buf.tellp() > start_position)
			buf << '\n';
		buf << _("This system has no sound hardware, MAME will produce no sounds, this is expected behavior.\n");
	}

	// these are more severe warnings
	if (flags & ::machine_flags::MECHANICAL)
	{
		if (buf.tellp() > start_position)
			buf << '\n';
		buf << _("Elements of this system cannot be emulated accurately as they require physical interaction or consist of mechanical devices. It is not possible to fully experience this system.\n");
	}
	if (flags & ::machine_flags::NOT_WORKING)
	{
		if (buf.tellp() > start_position)
			buf << '\n';
		buf << _("THIS SYSTEM DOESN'T WORK. The emulation for this system is not yet complete. There is nothing you can do to fix this problem except wait for the developers to improve the emulation.\n");
	}

	if ((flags & MACHINE_ERRORS) || ((machine.system().type.unemulated_features() | machine.system().type.imperfect_features()) & device_t::feature::PROTECTION))
	{
		// find the parent of this driver
		driver_enumerator drivlist(machine.options());
		int maindrv = drivlist.find(machine.system());
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
						util::stream_format(buf, _("\n\nThere are working clones of this system: %s"), driver.name);
					else
						util::stream_format(buf, _(", %s"), driver.name);
					foundworking = true;
				}
			}
		}
		if (foundworking)
			buf << '\n';
	}
}

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
	std::ostringstream sink;
	for (device_t &device : device_enumerator(config.root_device()))
	{
		// the "no sound hardware" warning doesn't make sense when you plug in a sound card
		if (dynamic_cast<speaker_device *>(&device))
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
		for (render_target const &target : config.root_device().machine().render().targets())
			if (!target.hidden() && target.external_artwork())
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
	get_general_warnings(buf, m_machine, machine_flags(), unemulated_features(), imperfect_features());
	get_system_warnings(buf, m_machine, machine_flags(), unemulated_features(), imperfect_features());
	return buf.str();
}


//-------------------------------------------------
//  game_info_string - return the game info text
//-------------------------------------------------

std::string machine_info::game_info_string() const
{
	std::ostringstream buf;

	// get decimal separator
	std::string point;
	{
		wchar_t const s(std::use_facet<std::numpunct<wchar_t> >(std::locale()).decimal_point());
		point = utf8_from_wstring(std::wstring_view(&s, 1));
	}

	// print description, manufacturer, and CPU:
	util::stream_format(buf, _("%1$s\n%2$s %3$s\nSource file: %4$s\n\nCPU:\n"),
			system_list::instance().systems()[driver_list::find(m_machine.system().name)].description,
			m_machine.system().year,
			m_machine.system().manufacturer,
			info_xml_creator::format_sourcefile(m_machine.system().type.source()));

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
			hz.insert(dpos, point);
			size_t last = hz.find_last_not_of('0');
			hz = hz.substr(0, last + (last != dpos ? 1 : 0));
		}

		// if more than one, prepend a #x in front of the CPU name and display clock
		util::stream_format(buf,
				(count > 1)
					? ((clock != 0) ? u8"%1$d×%2$s %3$s\u00a0%4$s\n" : u8"%1$d×%2$s\n")
					: ((clock != 0) ? u8"%2$s %3$s\u00a0%4$s\n" : "%2$s\n"),
				count, name, hz,
				(d == 9) ? _("GHz") : (d == 6) ? _("MHz") : (d == 3) ? _("kHz") : _("Hz"));
	}

	// loop over all sound chips
	sound_interface_enumerator snditer(m_machine.root_device());
	std::unordered_set<std::string> soundtags;
	bool found_sound = false;
	for (device_sound_interface &sound : snditer)
	{
		if (!soundtags.insert(sound.device().tag()).second)
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
			hz.insert(dpos, point);
			size_t last = hz.find_last_not_of('0');
			hz = hz.substr(0, last + (last != dpos ? 1 : 0));
		}

		// if more than one, prepend a #x in front of the soundchip name and display clock
		util::stream_format(buf,
				(count > 1)
					? ((clock != 0) ? u8"%1$d×%2$s %3$s\u00a0%4$s\n" : u8"%1$d×%2$s\n")
					: ((clock != 0) ? u8"%2$s %3$s\u00a0%4$s\n" : "%2$s\n"),
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
				const u32 rate = u32(screen.frame_period().as_hz() * 1'000'000 + 0.5);
				const bool valid = rate >= 1'000'000;
				std::string hz(valid ? std::to_string(rate) : "?");
				if (valid)
				{
					size_t dpos = hz.length() - 6;
					hz.insert(dpos, point);
					size_t last = hz.find_last_not_of('0');
					hz = hz.substr(0, last + (last != dpos ? 1 : 0));
				}

				const rectangle &visarea = screen.visible_area();
				detail = string_format(u8"%d × %d (%s) %s\u00a0Hz",
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

menu_game_info::menu_game_info(mame_ui_manager &mui, render_container &container) : menu_textbox(mui, container)
{
	set_process_flags(PROCESS_CUSTOM_NAV);
}

menu_game_info::~menu_game_info()
{
}

void menu_game_info::menu_activated()
{
	// screen modes can be reconfigured while the menu isn't displayed, etc.
	reset_layout();
}

void menu_game_info::populate_text(std::optional<text_layout> &layout, float &width, int &lines)
{
	if (!layout || (layout->width() != width))
	{
		rgb_t const color = ui().colors().text_color();
		layout.emplace(create_layout(width));
		layout->add_text(ui().machine_info().game_info_string(), color);
		lines = layout->lines();
	}
	width = layout->actual_width();
}

void menu_game_info::populate()
{
}


/*-------------------------------------------------
  menu_warn_info - handle the emulation warnings menu
-------------------------------------------------*/

menu_warn_info::menu_warn_info(mame_ui_manager &mui, render_container &container) : menu_textbox(mui, container)
{
	set_process_flags(PROCESS_CUSTOM_NAV);
}

menu_warn_info::~menu_warn_info()
{
}

void menu_warn_info::populate_text(std::optional<text_layout> &layout, float &width, int &lines)
{
	if (!layout || (layout->width() != width))
	{
		std::ostringstream buf;
		std::set<std::add_pointer_t<device_type> > seen;
		bool first(!machine().rom_load().knownbad());

		machine_info const &info(ui().machine_info());
		device_t &root(machine().root_device());
		get_general_warnings(buf, machine(), info.machine_flags(), info.unemulated_features(), info.imperfect_features());
		if ((info.machine_flags() & (MACHINE_ERRORS | MACHINE_WARNINGS | MACHINE_BTANB)) || root.type().unemulated_features() || root.type().imperfect_features())
		{
			seen.insert(&root.type());
			if (!first)
				buf << '\n';
			first = false;
			util::stream_format(buf, _("%1$s:\n"), root.name());
			get_system_warnings(buf, machine(), info.machine_flags(), root.type().unemulated_features(), root.type().imperfect_features());
		}

		for (device_t const &device : device_enumerator(root))
		{
			if ((device.type().unemulated_features() || device.type().imperfect_features()) && seen.insert(&device.type()).second)
			{
				if (!first)
					buf << '\n';
				first = false;
				util::stream_format(buf, _("%1$s:\n"), device.name());
				get_device_warnings(buf, device.type().unemulated_features(), device.type().imperfect_features());
			}
		}

		rgb_t const color(ui().colors().text_color());
		layout.emplace(create_layout(width));
		layout->add_text(std::move(buf).str(), color);
		lines = layout->lines();
	}
	width = layout->actual_width();
}

void menu_warn_info::populate()
{
}


/*-------------------------------------------------
  menu_image_info - handle the image information menu
-------------------------------------------------*/

menu_image_info::menu_image_info(mame_ui_manager &mui, render_container &container) : menu(mui, container)
{
	set_heading(_("Media Image Information"));
}

menu_image_info::~menu_image_info()
{
}

void menu_image_info::menu_activated()
{
	reset(reset_options::REMEMBER_POSITION);
}

void menu_image_info::populate()
{
	for (device_image_interface &image : image_interface_enumerator(machine().root_device()))
		image_info(image);
}

bool menu_image_info::handle(event const *ev)
{
	return false;
}


/*-------------------------------------------------
  image_info - display image info for a specific
  image interface device
-------------------------------------------------*/

void menu_image_info::image_info(device_image_interface &image)
{
	if (!image.user_loadable())
		return;

	m_notifiers.emplace_back(image.add_media_change_notifier(delegate(&menu_image_info::reload, this)));

	if (image.exists())
	{
		// display device type and filename
		item_append(image.brief_instance_name(), image.basename(), 0, &image);

		// if image has been loaded through softlist, let's add some more info
		if (image.loaded_through_softlist())
		{
			software_info const &swinfo(*image.software_entry());

			// display full name, publisher and year
			item_append(swinfo.longname(), FLAG_DISABLE, nullptr);
			item_append(string_format("%1$s, %2$s", swinfo.publisher(), swinfo.year()), FLAG_DISABLE, nullptr);

			// display supported information, if available
			switch (swinfo.supported())
			{
			case software_support::UNSUPPORTED:
				item_append(_("Not supported"), FLAG_DISABLE, nullptr);
				break;
			case software_support::PARTIALLY_SUPPORTED:
				item_append(_("Partially supported"), FLAG_DISABLE, nullptr);
				break;
			case software_support::SUPPORTED:
				break;
			}
		}
	}
	else
	{
		item_append(image.brief_instance_name(), _("[empty]"), 0, &image);
	}
	item_append(menu_item_type::SEPARATOR);
}


/*-------------------------------------------------
  reload - refresh the menu after a media change
-------------------------------------------------*/

void menu_image_info::reload(device_image_interface::media_change_event ev)
{
	m_notifiers.clear();
	reset(reset_options::REMEMBER_REF);
}

} // namespace ui
