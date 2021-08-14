// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/*********************************************************************

    ui/devopt.cpp

    Internal menu for the device configuration.

*********************************************************************/

#include "emu.h"
#include "ui/devopt.h"

#include "ui/ui.h"
#include "romload.h"


namespace ui {

/*-------------------------------------------------
 device_config - handle the game information
 menu
 -------------------------------------------------*/

menu_device_config::menu_device_config(mame_ui_manager &mui, render_container &container, device_slot_interface *slot, device_slot_interface::slot_option const *option) : menu(mui, container)
{
	m_option = option;
	m_owner = slot;
	m_mounted = slot->device().subdevice(option->name()) != nullptr;
}

void menu_device_config::populate(float &customtop, float &custombottom)
{
	machine_config &mconfig(const_cast<machine_config &>(machine().config()));
	machine_config::token const tok(mconfig.begin_configuration(mconfig.root_device()));
	device_t *const dev = mconfig.device_add(m_option->name(), m_option->devtype(), 0);
	for (device_t &d : device_enumerator(*dev))
		if (!d.configured())
			d.config_complete();

	std::ostringstream str;
	util::stream_format(
			str,
			m_mounted
				? _("[This option is currently mounted in the running system]\n\nOption: %1$s\nDevice: %2$s\n\nThe selected option enables the following items:\n")
				: _("[This option is NOT currently mounted in the running system]\n\nOption: %1$s\nDevice: %2$s\n\nIf you select this option, the following items will be enabled:\n"),
			m_option->name(),
			dev->name());

	// loop over all CPUs
	execute_interface_enumerator execiter(*dev);
	if (execiter.count() > 0)
	{
		str << _("* CPU:\n");
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
			util::stream_format(str,
					(count > 1)
						? ((clock != 0) ? "  %1$d" UTF8_MULTIPLY "%2$s %3$s" UTF8_NBSP "%4$s\n" : "  %1$d" UTF8_MULTIPLY "%2$s\n")
						: ((clock != 0) ? "  %2$s %3$s" UTF8_NBSP "%4$s\n" : "  %2$s\n"),
					count, name, hz,
					(d == 9) ? _("GHz") : (d == 6) ? _("MHz") : (d == 3) ? _("kHz") : _("Hz"));
		}
	}

	// display screen information
	screen_device_enumerator scriter(*dev);
	if (scriter.count() > 0)
	{
		str << _("* Video:\n");
		for (screen_device &screen : scriter)
		{
			if (screen.screen_type() == SCREEN_TYPE_VECTOR)
			{
				util::stream_format(str, _("  Screen '%1$s': Vector\n"), screen.tag());
			}
			else
			{
				std::string hz(std::to_string(float(screen.frame_period().as_hz())));
				size_t last = hz.find_last_not_of('0');
				size_t dpos = hz.find_last_of('.');
				hz = hz.substr(0, last + (last != dpos ? 1 : 0));

				const rectangle &visarea = screen.visible_area();
				util::stream_format(
						str,
						(screen.orientation() & ORIENTATION_SWAP_XY)
							? _("  Screen '%1$s': %2$d \xC3\x97 %3$d (V) %4$s\xC2\xA0Hz\n")
							: _("  Screen '%1$s': %2$d \xC3\x97 %3$d (H) %4$s\xC2\xA0Hz\n"),
						screen.tag(),
						visarea.width(),
						visarea.height(),
						hz);
			}
		}
	}

	// loop over all sound chips
	sound_interface_enumerator snditer(*dev);
	if (snditer.count() > 0)
	{
		str << _("* Sound:\n");
		std::unordered_set<std::string> soundtags;
		for (device_sound_interface &sound : snditer)
		{
			if (!sound.issound() || !soundtags.insert(sound.device().tag()).second)
				continue;

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

			// if more than one, prepend a #x in front of the name and display clock
			util::stream_format(str,
					(count > 1)
						? ((clock != 0) ? "  %1$d" UTF8_MULTIPLY "%2$s %3$s" UTF8_NBSP "%4$s\n" : "  %1$d" UTF8_MULTIPLY "%2$s\n")
						: ((clock != 0) ? "  %2$s %3$s" UTF8_NBSP "%4$s\n" : "  %2$s\n"),
					count, sound.device().name(), hz,
					(d == 9) ? _("GHz") : (d == 6) ? _("MHz") : (d == 3) ? _("kHz") : _("Hz"));
		}
	}

	// scan for BIOS settings
	int bios = 0;
	if (dev->rom_region())
	{
		// first loop through roms in search of default bios (shortname)
		char const *bios_str(nullptr);
		for (const tiny_rom_entry *rom = dev->rom_region(); !ROMENTRY_ISEND(rom); ++rom)
		{
			if (ROMENTRY_ISDEFAULT_BIOS(rom))
				bios_str = rom->name;
		}

		// then loop again to count bios options and to get the default bios complete name
		char const *bios_desc(nullptr);
		for (romload::system_bios const &rom : romload::entries(dev->rom_region()).get_system_bioses())
		{
			bios++;
			if (bios_str && !std::strcmp(bios_str, rom.get_name()))
				bios_desc = rom.get_description();
		}

		if (bios)
			util::stream_format(str, _("* BIOS settings:\n  %1$d options    [default: %2$s]\n"), bios, bios_desc ? bios_desc : bios_str ? bios_str : "");
	}

	int input = 0, input_mj = 0, input_hana = 0, input_gamble = 0, input_analog = 0, input_adjust = 0;
	int input_keypad = 0, input_keyboard = 0, dips = 0, confs = 0;
	std::string errors;
	std::ostringstream dips_opt, confs_opt;
	ioport_list portlist;
	for (device_t &iptdev : device_enumerator(*dev))
		portlist.append(iptdev, errors);

	// check if the device adds inputs to the system
	for (auto &port : portlist)
		for (ioport_field &field : port.second->fields())
		{
			if (field.type() >= IPT_MAHJONG_FIRST && field.type() < IPT_MAHJONG_LAST)
				input_mj++;
			else if (field.type() >= IPT_HANAFUDA_FIRST && field.type() < IPT_HANAFUDA_LAST)
				input_hana++;
			else if (field.type() >= IPT_GAMBLING_FIRST && field.type() < IPT_GAMBLING_LAST)
				input_gamble++;
			else if (field.type() >= IPT_ANALOG_FIRST && field.type() < IPT_ANALOG_LAST)
				input_analog++;
			else if (field.type() == IPT_ADJUSTER)
				input_adjust++;
			else if (field.type() == IPT_KEYPAD)
				input_keypad++;
			else if (field.type() == IPT_KEYBOARD)
				input_keyboard++;
			else if (field.type() >= IPT_START1 && field.type() < IPT_UI_FIRST)
				input++;
			else if (field.type() == IPT_DIPSWITCH)
			{
				dips++;
				bool def(false);
				for (ioport_setting const &setting : field.settings())
				{
					if (setting.value() == field.defvalue())
					{
						def = true;
						util::stream_format(dips_opt, _("  %1$s    [default: %2$s]\n"), field.name(), setting.name());
						break;
					}
				}
				if (!def)
					util::stream_format(dips_opt, _("  %1$s\n"), field.name());
			}
			else if (field.type() == IPT_CONFIG)
			{
				confs++;
				bool def(false);
				for (ioport_setting const &setting : field.settings())
				{
					if (setting.value() == field.defvalue())
					{
						def = true;
						util::stream_format(confs_opt, _("  %1$s    [default: %2$s]\n"), field.name(), setting.name());
						break;
					}
				}
				if (!def)
					util::stream_format(confs_opt, _("  %1$s\n"), field.name());
			}
		}

	if (dips)
		str << _("* DIP switch settings:\n") << dips_opt.str();
	if (confs)
		str << _("* Configuration settings:\n") << confs_opt.str();
	if (input || input_mj || input_hana || input_gamble || input_analog || input_adjust || input_keypad || input_keyboard)
		str << _("* Input device(s):\n");
	if (input)
		util::stream_format(str, _("  User inputs    [%1$d inputs]\n"), input);
	if (input_mj)
		util::stream_format(str, _("  Mahjong inputs    [%1$d inputs]\n"), input_mj);
	if (input_hana)
		util::stream_format(str, _("  Hanafuda inputs    [%1$d inputs]\n"), input_hana);
	if (input_gamble)
		util::stream_format(str, _("  Gambling inputs    [%1$d inputs]\n"), input_gamble);
	if (input_analog)
		util::stream_format(str, _("  Analog inputs    [%1$d inputs]\n"), input_analog);
	if (input_adjust)
		util::stream_format(str, _("  Adjuster inputs    [%1$d inputs]\n"), input_adjust);
	if (input_keypad)
		util::stream_format(str, _("  Keypad inputs    [%1$d inputs]\n"), input_keypad);
	if (input_keyboard)
		util::stream_format(str, _("  Keyboard inputs    [%1$d inputs]\n"), input_keyboard);

	image_interface_enumerator imgiter(*dev);
	if (imgiter.count() > 0)
	{
		str << _("* Media Options:\n");
		for (const device_image_interface &imagedev : imgiter)
			util::stream_format(str, _("  %1$s    [tag: %2$s]\n"), imagedev.image_type_name(), imagedev.device().tag());
	}

	slot_interface_enumerator slotiter(*dev);
	if (slotiter.count() > 0)
	{
		str << _("* Slot Options:\n");
		for (const device_slot_interface &slot : slotiter)
			util::stream_format(str, _("  %1$s    [default: %2$s]\n"), slot.device().tag(), slot.default_option() ? slot.default_option() : "----");
	}

	if ((execiter.count() + scriter.count() + snditer.count() + imgiter.count() + slotiter.count() + bios + dips + confs
			+ input + input_mj + input_hana + input_gamble + input_analog + input_adjust + input_keypad + input_keyboard) == 0)
			str << _("[None]\n");

	mconfig.device_remove(m_option->name());
	item_append(str.str(), FLAG_MULTILINE, nullptr);
}

void menu_device_config::handle()
{
	/* process the menu */
	process(0);
}

menu_device_config::~menu_device_config()
{
}

} // namespace ui
