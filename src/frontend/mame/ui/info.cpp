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
#include "softlist.h"

namespace ui {


//-------------------------------------------------
//  machine_info - constructor
//-------------------------------------------------

machine_info::machine_info(running_machine &machine)
	: m_machine(machine)
{
	// calculate "has..." values
	m_has_configs = false;
	m_has_analog = false;
	m_has_dips = false;
	m_has_bioses = false;

	// scan the input port array to see what options we need to enable
	for (auto &port : machine.ioport().ports())
		for (ioport_field &field : port.second->fields())
		{
			if (field.type() == IPT_DIPSWITCH)
				m_has_dips = true;
			if (field.type() == IPT_CONFIG)
				m_has_configs = true;
			if (field.is_analog())
				m_has_analog = true;
		}

	for (device_t &device : device_iterator(machine.root_device()))
		for (const rom_entry &rom : device.rom_region_vector())
			if (ROMENTRY_ISSYSTEM_BIOS(&rom)) { m_has_bioses = true; break; }
}


/***************************************************************************
    TEXT GENERATORS
***************************************************************************/

//-------------------------------------------------
//  warnings_string - print the warning flags
//  text to the given buffer
//-------------------------------------------------

std::string machine_info::warnings_string()
{
	constexpr UINT32 warning_flags = ( MACHINE_NOT_WORKING |
						MACHINE_UNEMULATED_PROTECTION |
						MACHINE_MECHANICAL |
						MACHINE_WRONG_COLORS |
						MACHINE_IMPERFECT_COLORS |
						MACHINE_REQUIRES_ARTWORK |
						MACHINE_NO_SOUND |
						MACHINE_IMPERFECT_SOUND |
						MACHINE_IMPERFECT_GRAPHICS |
						MACHINE_IMPERFECT_KEYBOARD |
						MACHINE_NO_COCKTAIL |
						MACHINE_IS_INCOMPLETE |
						MACHINE_NO_SOUND_HW );

	// if no warnings, nothing to return
	if (m_machine.rom_load().warnings() == 0 && m_machine.rom_load().knownbad() == 0 && !(m_machine.system().flags & warning_flags) && m_machine.rom_load().software_load_warnings_message().length() == 0)
		return std::string();

	std::ostringstream buf;

	// add a warning if any ROMs were loaded with warnings
	if (m_machine.rom_load().warnings() > 0)
	{
		buf << _("One or more ROMs/CHDs for this machine are incorrect. The machine may not run correctly.\n");
		if (m_machine.system().flags & warning_flags)
			buf << "\n";
	}

	if (m_machine.rom_load().software_load_warnings_message().length()>0) {
		buf << m_machine.rom_load().software_load_warnings_message();
		if (m_machine.system().flags & warning_flags)
			buf << "\n";
	}
	// if we have at least one warning flag, print the general header
	if ((m_machine.system().flags & warning_flags) || m_machine.rom_load().knownbad() > 0)
	{
		buf << _("There are known problems with this machine\n\n");

		// add a warning if any ROMs are flagged BAD_DUMP/NO_DUMP
		if (m_machine.rom_load().knownbad() > 0) {
			buf << _("One or more ROMs/CHDs for this machine have not been correctly dumped.\n");
		}
		// add one line per warning flag
		if (m_machine.system().flags & MACHINE_IMPERFECT_KEYBOARD)
			buf << _("The keyboard emulation may not be 100% accurate.\n");
		if (m_machine.system().flags & MACHINE_IMPERFECT_COLORS)
			buf << _("The colors aren't 100% accurate.\n");
		if (m_machine.system().flags & MACHINE_WRONG_COLORS)
			buf << _("The colors are completely wrong.\n");
		if (m_machine.system().flags & MACHINE_IMPERFECT_GRAPHICS)
			buf << _("The video emulation isn't 100% accurate.\n");
		if (m_machine.system().flags & MACHINE_IMPERFECT_SOUND)
			buf << _("The sound emulation isn't 100% accurate.\n");
		if (m_machine.system().flags & MACHINE_NO_SOUND) {
			buf << _("The machine lacks sound.\n");
		}
		if (m_machine.system().flags & MACHINE_NO_COCKTAIL)
			buf << _("Screen flipping in cocktail mode is not supported.\n");

		// check if external artwork is present before displaying this warning?
		if (m_machine.system().flags & MACHINE_REQUIRES_ARTWORK) {
			buf << _("The machine requires external artwork files\n");
		}

		if (m_machine.system().flags & MACHINE_IS_INCOMPLETE )
		{
			buf << _("This machine was never completed. It may exhibit strange behavior or missing elements that are not bugs in the emulation.\n");
		}

		if (m_machine.system().flags & MACHINE_NO_SOUND_HW )
		{
			buf << _("This machine has no sound hardware, MAME will produce no sounds, this is expected behaviour.\n");
		}

		// if there's a NOT WORKING, UNEMULATED PROTECTION or GAME MECHANICAL warning, make it stronger
		if (m_machine.system().flags & (MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION | MACHINE_MECHANICAL))
		{
			// add the strings for these warnings
			if (m_machine.system().flags & MACHINE_UNEMULATED_PROTECTION) {
				buf << _("The machine has protection which isn't fully emulated.\n");
			}
			if (m_machine.system().flags & MACHINE_NOT_WORKING) {
				buf << _("\nTHIS MACHINE DOESN'T WORK. The emulation for this machine is not yet complete. "
						"There is nothing you can do to fix this problem except wait for the developers to improve the emulation.\n");
			}
			if (m_machine.system().flags & MACHINE_MECHANICAL) {
				buf << _("\nCertain elements of this machine cannot be emulated as it requires actual physical interaction or consists of mechanical devices. "
						"It is not possible to fully play this machine.\n");
			}

			// find the parent of this driver
			driver_enumerator drivlist(m_machine.options());
			int maindrv = drivlist.find(m_machine.system());
			int clone_of = drivlist.non_bios_clone(maindrv);
			if (clone_of != -1)
				maindrv = clone_of;

			// scan the driver list for any working clones and add them
			bool foundworking = false;
			while (drivlist.next())
				if (drivlist.current() == maindrv || drivlist.clone() == maindrv)
					if ((drivlist.driver().flags & (MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION | MACHINE_MECHANICAL)) == 0)
					{
						// this one works, add a header and display the name of the clone
						if (!foundworking) {
							buf << _("\n\nThere are working clones of this machine: ");
						}
						else
							buf << ", ";
						buf << drivlist.driver().name;
						foundworking = true;
					}

			if (foundworking)
				buf << "\n";
		}
	}

	// add the 'press OK' string
	buf << _("\n\nPress any key to continue");
	return buf.str();
}


//-------------------------------------------------
//  game_info_string - return the game info text
//-------------------------------------------------

std::string machine_info::game_info_string()
{
	std::ostringstream buf;

	// print description, manufacturer, and CPU:
	util::stream_format(buf, _("%1$s\n%2$s %3$s\nDriver: %4$s\n\nCPU:\n"),
			m_machine.system().description,
			m_machine.system().year,
			m_machine.system().manufacturer,
			core_filename_extract_base(m_machine.system().source_file));

	// loop over all CPUs
	execute_interface_iterator execiter(m_machine.root_device());
	std::unordered_set<std::string> exectags;
	for (device_execute_interface &exec : execiter)
	{
		if (!exectags.insert(exec.device().tag()).second)
			continue;
		// get cpu specific clock that takes internal multiplier/dividers into account
		int clock = exec.device().clock();

		// count how many identical CPUs we have
		int count = 1;
		const char *name = exec.device().name();
		for (device_execute_interface &scan : execiter)
		{
			if (exec.device().type() == scan.device().type() && strcmp(name, scan.device().name()) == 0 && exec.device().clock() == scan.device().clock())
				if (exectags.insert(scan.device().tag()).second)
					count++;
		}

		// if more than one, prepend a #x in front of the CPU name
		// display clock in kHz or MHz
		util::stream_format(buf,
				(count > 1) ? "%1$d" UTF8_MULTIPLY "%2$s %3$d.%4$0*5$d%6$s\n" : "%2$s %3$d.%4$0*5$d%6$s\n",
				count,
				name,
				(clock >= 1000000) ? (clock / 1000000) : (clock / 1000),
				(clock >= 1000000) ? (clock % 1000000) : (clock % 1000),
				(clock >= 1000000) ? 6 : 3,
				(clock >= 1000000) ? _("MHz") : _("kHz"));
	}

	// loop over all sound chips
	sound_interface_iterator snditer(m_machine.root_device());
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

		// if more than one, prepend a #x in front of the CPU name
		// display clock in kHz or MHz
		int clock = sound.device().clock();
		util::stream_format(buf,
				(count > 1)
					? ((clock != 0) ? "%1$d" UTF8_MULTIPLY "%2$s %3$d.%4$0*5$d%6$s\n" : "%1$d" UTF8_MULTIPLY "%2$s\n")
					: ((clock != 0) ? "%2$s %3$d.%4$0*5$d%6$s\n" : "%2$s\n"),
				count,
				sound.device().name(),
				(clock >= 1000000) ? (clock / 1000000) : (clock / 1000),
				(clock >= 1000000) ? (clock % 1000000) : (clock % 1000),
				(clock >= 1000000) ? 6 : 3,
				(clock >= 1000000) ? _("MHz") : _("kHz"));
	}

	// display screen information
	buf << _("\nVideo:\n");
	screen_device_iterator scriter(m_machine.root_device());
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
				const rectangle &visarea = screen.visible_area();
				detail = string_format("%d " UTF8_MULTIPLY " %d (%s) %f" UTF8_NBSP "Hz",
						visarea.width(), visarea.height(),
						(m_machine.system().flags & ORIENTATION_SWAP_XY) ? "V" : "H",
						ATTOSECONDS_TO_HZ(screen.frame_period().attoseconds()));
			}

			util::stream_format(buf,
					(scrcount > 1) ? _("%1$s: %2$s\n") : _("%2$s\n"),
					get_screen_desc(screen), detail);
		}
	}

	return buf.str();
}


//-------------------------------------------------
//  mandatory_images - search for devices which
//  need an image to be loaded
//-------------------------------------------------

std::string machine_info::mandatory_images()
{
	std::ostringstream buf;

	// make sure that any required image has a mounted file
	for (device_image_interface &image : image_interface_iterator(m_machine.root_device()))
	{
		if (image.filename() == nullptr && image.must_be_loaded())
			buf << "\"" << image.instance_name() << "\", ";
	}
	return buf.str();
}


//-------------------------------------------------
//  get_screen_desc - returns the description for
//  a given screen
//-------------------------------------------------

std::string machine_info::get_screen_desc(screen_device &screen)
{
	if (screen_device_iterator(m_machine.root_device()).count() > 1)
		return string_format(_("Screen '%1$s'"), screen.tag());
	else
		return _("Screen");
}


/*-------------------------------------------------
  menu_game_info - handle the game information
  menu
 -------------------------------------------------*/

menu_game_info::menu_game_info(mame_ui_manager &mui, render_container &container) : menu(mui, container)
{
}

menu_game_info::~menu_game_info()
{
}

void menu_game_info::populate()
{
	std::string tempstring = ui().machine_info().game_info_string();
	item_append(std::move(tempstring), "", FLAG_MULTILINE, nullptr);
}

void menu_game_info::handle()
{
	// process the menu
	process(0);
}


/*-------------------------------------------------
  menu_image_info - handle the image information
  menu
 -------------------------------------------------*/

menu_image_info::menu_image_info(mame_ui_manager &mui, render_container &container) : menu(mui, container)
{
}

menu_image_info::~menu_image_info()
{
}

void menu_image_info::populate()
{
	item_append(machine().system().description, "", FLAG_DISABLE, nullptr);
	item_append("", "", FLAG_DISABLE, nullptr);

	for (device_image_interface &image : image_interface_iterator(machine().root_device()))
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
		if (image->software_entry())
		{
			// display long filename
			item_append(image->longname(), "", FLAG_DISABLE, nullptr);

			// display manufacturer and year
			item_append(string_format("%s, %s", image->manufacturer(), image->year()), "", FLAG_DISABLE, nullptr);

			// display supported information, if available
			switch (image->supported())
			{
				case SOFTWARE_SUPPORTED_NO:
					item_append(_("Not supported"), "", FLAG_DISABLE, nullptr);
					break;
				case SOFTWARE_SUPPORTED_PARTIAL:
					item_append(_("Partially supported"), "", FLAG_DISABLE, nullptr);
					break;
				default:
					break;
			}
		}
	}
	else
		item_append(image->brief_instance_name(), _("[empty]"), 0, nullptr);
	item_append("", "", FLAG_DISABLE, nullptr);
}

} // namespace ui
