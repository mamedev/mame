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
#include "screen.h"

#include "util/unicode.h"

#include <locale>
#include <sstream>


namespace ui {

/*-------------------------------------------------
 device_config - handle the game information
 menu
 -------------------------------------------------*/

menu_device_config::menu_device_config(
		mame_ui_manager &mui,
		render_container &container,
		device_slot_interface *slot,
		device_slot_interface::slot_option const *option)
	: menu_textbox(mui, container)
	, m_option(option)
	, m_mounted(machine().root_device().subdevice(slot->device().subtag(option->name())) != nullptr)
{
	set_process_flags(PROCESS_CUSTOM_NAV);
}

menu_device_config::~menu_device_config()
{
}

void menu_device_config::populate_text(std::optional<text_layout> &layout, float &width, int &lines)
{
	if (!layout || (layout->width() != width))
	{
		rgb_t const color = ui().colors().text_color();
		layout.emplace(create_layout(width));

		machine_config &mconfig(const_cast<machine_config &>(machine().config()));
		machine_config::token const tok(mconfig.begin_configuration(mconfig.root_device()));
		device_t *const dev = mconfig.device_add(m_option->name(), m_option->devtype(), 0);
		for (device_t &d : device_enumerator(*dev))
			if (!d.configured())
				d.config_complete();

		// get decimal separator
		std::string point;
		{
			wchar_t const s(std::use_facet<std::numpunct<wchar_t> >(std::locale()).decimal_point());
			point = utf8_from_wstring(std::wstring_view(&s, 1));
		}

		layout->add_text(
				util::string_format(
					m_mounted
						? _("[This option is currently mounted in the running system]\n\nOption: %1$s\nDevice: %2$s\n\nThe selected option enables the following items:\n")
						: _("[This option is NOT currently mounted in the running system]\n\nOption: %1$s\nDevice: %2$s\n\nIf you select this option, the following items will be enabled:\n"),
					m_option->name(),
					dev->name()),
				color);

		// loop over all CPUs
		execute_interface_enumerator execiter(*dev);
		if (execiter.count() > 0)
		{
			layout->add_text(_("* CPU:\n"), color);
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
				layout->add_text(
						util::string_format(
							(count > 1)
								? ((clock != 0) ? u8"  %1$d×%2$s %3$s\u00a0%4$s\n" : u8"  %1$d×%2$s\n")
								: ((clock != 0) ? u8"  %2$s %3$s\u00a0%4$s\n" : "  %2$s\n"),
							count, name, hz,
							(d == 9) ? _("GHz") : (d == 6) ? _("MHz") : (d == 3) ? _("kHz") : _("Hz")),
						color);
			}
		}

		// display screen information
		screen_device_enumerator scriter(*dev);
		if (scriter.count() > 0)
		{
			layout->add_text(_("* Video:\n"), color);
			for (screen_device &screen : scriter)
			{
				if (screen.screen_type() == SCREEN_TYPE_VECTOR)
				{
					layout->add_text(util::string_format(_("  Screen '%1$s': Vector\n"), screen.tag()), color);
				}
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
					layout->add_text(
							util::string_format(
								(screen.orientation() & ORIENTATION_SWAP_XY)
									? _(u8"  Screen '%1$s': %2$d × %3$d (V) %4$s\u00a0Hz\n")
									: _(u8"  Screen '%1$s': %2$d × %3$d (H) %4$s\u00a0Hz\n"),
								screen.tag(),
								visarea.width(),
								visarea.height(),
								hz),
							color);
				}
			}
		}

		// loop over all sound chips
		sound_interface_enumerator snditer(*dev);
		if (snditer.count() > 0)
		{
			layout->add_text(_("* Sound:\n"), color);
			std::unordered_set<std::string> soundtags;
			for (device_sound_interface &sound : snditer)
			{
				if (!soundtags.insert(sound.device().tag()).second)
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
					hz.insert(dpos, point);
					size_t last = hz.find_last_not_of('0');
					hz = hz.substr(0, last + (last != dpos ? 1 : 0));
				}

				// if more than one, prepend a #x in front of the name and display clock
				layout->add_text(
						util::string_format(
							(count > 1)
								? ((clock != 0) ? u8"  %1$d×%2$s %3$s\u00a0%4$s\n" : u8"  %1$d×%2$s\n")
								: ((clock != 0) ? u8"  %2$s %3$s\u00a0%4$s\n" : "  %2$s\n"),
							count, sound.device().name(), hz,
							(d == 9) ? _("GHz") : (d == 6) ? _("MHz") : (d == 3) ? _("kHz") : _("Hz")),
						color);
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
			{
				layout->add_text(
						util::string_format(
							_("* BIOS settings:\n  %1$d options    [default: %2$s]\n"),
							bios,
							bios_desc ? bios_desc : bios_str ? bios_str : ""),
						color);
			}
		}

		int input = 0, input_mj = 0, input_hana = 0, input_gamble = 0, input_analog = 0, input_adjust = 0;
		int input_keypad = 0, input_keyboard = 0, dips = 0, confs = 0;
		std::ostringstream dips_opt, confs_opt;
		ioport_list portlist;
		{
			std::ostringstream errors;
			for (device_t &iptdev : device_enumerator(*dev))
				portlist.append(iptdev, errors);
		}

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
							util::stream_format(dips_opt, _("  %1$s    [default: %2$s]\n"), field.specific_name(), setting.name());
							break;
						}
					}
					if (!def)
						util::stream_format(dips_opt, _("  %1$s\n"), field.specific_name());
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
							util::stream_format(confs_opt, _("  %1$s    [default: %2$s]\n"), field.specific_name(), setting.name());
							break;
						}
					}
					if (!def)
						util::stream_format(confs_opt, _("  %1$s\n"), field.specific_name());
				}
			}

		if (dips)
		{
			layout->add_text(_("* DIP switch settings:\n"), color);
			layout->add_text(std::move(dips_opt).str(), color);
		}
		if (confs)
		{
			layout->add_text(_("* Configuration settings:\n"), color);
			layout->add_text(std::move(confs_opt).str(), color);
		}
		if (input || input_mj || input_hana || input_gamble || input_analog || input_adjust || input_keypad || input_keyboard)
			layout->add_text(_("* Input device(s):\n"), color);
		if (input)
			layout->add_text(util::string_format(_("  User inputs    [%1$d inputs]\n"), input), color);
		if (input_mj)
			layout->add_text(util::string_format(_("  Mahjong inputs    [%1$d inputs]\n"), input_mj), color);
		if (input_hana)
			layout->add_text(util::string_format(_("  Hanafuda inputs    [%1$d inputs]\n"), input_hana), color);
		if (input_gamble)
			layout->add_text(util::string_format(_("  Gambling inputs    [%1$d inputs]\n"), input_gamble), color);
		if (input_analog)
			layout->add_text(util::string_format(_("  Analog inputs    [%1$d inputs]\n"), input_analog), color);
		if (input_adjust)
			layout->add_text(util::string_format(_("  Adjuster inputs    [%1$d inputs]\n"), input_adjust), color);
		if (input_keypad)
			layout->add_text(util::string_format(_("  Keypad inputs    [%1$d inputs]\n"), input_keypad), color);
		if (input_keyboard)
			layout->add_text(util::string_format(_("  Keyboard inputs    [%1$d inputs]\n"), input_keyboard), color);

		image_interface_enumerator imgiter(*dev);
		if (imgiter.count() > 0)
		{
			layout->add_text(_("* Media Options:\n"), color);
			for (const device_image_interface &imagedev : imgiter)
			{
				layout->add_text(
						util::string_format(
							_("  %1$s    [tag: %2$s]\n"),
							imagedev.image_type_name(),
							imagedev.device().tag()),
						color);
			}
		}

		slot_interface_enumerator slotiter(*dev);
		if (slotiter.count() > 0)
		{
			layout->add_text(_("* Slot Options:\n"), color);
			for (const device_slot_interface &slot : slotiter)
			{
				layout->add_text(
						util::string_format(
							_("  %1$s    [default: %2$s]\n"),
							slot.device().tag(),
							slot.default_option() ? slot.default_option() : "----"),
						color);
			}
		}

		if ((execiter.count() + scriter.count() + snditer.count() + imgiter.count() + slotiter.count() + bios + dips + confs
				+ input + input_mj + input_hana + input_gamble + input_analog + input_adjust + input_keypad + input_keyboard) == 0)
			layout->add_text(_("[None]\n"), color);

		mconfig.device_remove(m_option->name());
		lines = layout->lines();
	}
	width = layout->actual_width();
}

void menu_device_config::populate()
{
}

} // namespace ui
