// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/*********************************************************************

    ui/devopt.c

    Internal menu for the device configuration.

*********************************************************************/

#include "emu.h"
#include "ui/ui.h"
#include "ui/devopt.h"

/*-------------------------------------------------
 ui_device_config - handle the game information
 menu
 -------------------------------------------------*/

ui_menu_device_config::ui_menu_device_config(running_machine &machine, render_container *container, device_slot_interface *slot, device_slot_option *option) : ui_menu(machine, container)
{
	std::string tmp_tag;
	tmp_tag.assign(slot->device().tag()).append(":").append(option->name());
	m_option = option;
	m_owner = slot;
	m_mounted = false;

	device_iterator deviter(machine.config().root_device());
	for (device_t *device = deviter.first(); device != NULL; device = deviter.next())
	{
		if (strcmp(device->tag(), tmp_tag.c_str()) == 0)
		{
			m_mounted = true;
			break;
		}
	}
}

void ui_menu_device_config::populate()
{
	std::string str;
	device_t *dev;

	strprintf(str,"[This option is%s currently mounted in the running system]\n\n", m_mounted ? "" : " NOT");
	strcatprintf(str,"Option: %s\n", m_option->name());

	dev = const_cast<machine_config &>(machine().config()).device_add(&machine().config().root_device(), m_option->name(), m_option->devtype(), 0);

	strcatprintf(str,"Device: %s\n", dev->name());
	if (!m_mounted)
		str.append("\nIf you select this option, the following items will be enabled:\n");
	else
		str.append("\nThe selected option enables the following items:\n");

	// loop over all CPUs
	execute_interface_iterator execiter(*dev);
	if (execiter.count() > 0)
	{
		str.append("* CPU:\n");
		tagmap_t<UINT8> exectags;
		for (device_execute_interface *exec = execiter.first(); exec != NULL; exec = execiter.next())
		{
			if (exectags.add(exec->device().tag(), 1, FALSE) == TMERR_DUPLICATE)
				continue;

			// get cpu specific clock that takes internal multiplier/dividers into account
			int clock = exec->device().clock();

			// count how many identical CPUs we have
			int count = 1;
			const char *name = exec->device().name();
			execute_interface_iterator execinneriter(*dev);
			for (device_execute_interface *scan = execinneriter.first(); scan != NULL; scan = execinneriter.next())
			{
				if (exec->device().type() == scan->device().type() && strcmp(name, scan->device().name()) == 0 && exec->device().clock() == scan->device().clock())
					if (exectags.add(scan->device().tag(), 1, FALSE) != TMERR_DUPLICATE)
						count++;
			}

			// if more than one, prepend a #x in front of the CPU name
			if (count > 1)
				strcatprintf(str,"  %d" UTF8_MULTIPLY, count);
			else
				str.append("  ");
			str.append(name);

			// display clock in kHz or MHz
			if (clock >= 1000000)
				strcatprintf(str," %d.%06d" UTF8_NBSP "MHz\n", clock / 1000000, clock % 1000000);
			else
				strcatprintf(str," %d.%03d" UTF8_NBSP "kHz\n", clock / 1000, clock % 1000);
		}
	}

	// display screen information
	screen_device_iterator scriter(*dev);
	if (scriter.count() > 0)
	{
		str.append("* Video:\n");
		for (screen_device *screen = scriter.first(); screen != NULL; screen = scriter.next())
		{
			strcatprintf(str,"  Screen '%s': ", screen->tag());

			if (screen->screen_type() == SCREEN_TYPE_VECTOR)
				str.append("Vector\n");
			else
			{
				const rectangle &visarea = screen->visible_area();

				strcatprintf(str,"%d " UTF8_MULTIPLY " %d (%s) %f" UTF8_NBSP "Hz\n",
									visarea.width(), visarea.height(),
									(machine().system().flags & ORIENTATION_SWAP_XY) ? "V" : "H",
									ATTOSECONDS_TO_HZ(screen->frame_period().attoseconds()));
			}
		}
	}

	// loop over all sound chips
	sound_interface_iterator snditer(*dev);
	if (snditer.count() > 0)
	{
		str.append("* Sound:\n");
		tagmap_t<UINT8> soundtags;
		for (device_sound_interface *sound = snditer.first(); sound != NULL; sound = snditer.next())
		{
			if (soundtags.add(sound->device().tag(), 1, FALSE) == TMERR_DUPLICATE)
				continue;

			// count how many identical sound chips we have
			int count = 1;
			sound_interface_iterator sndinneriter(*dev);
			for (device_sound_interface *scan = sndinneriter.first(); scan != NULL; scan = sndinneriter.next())
			{
				if (sound->device().type() == scan->device().type() && sound->device().clock() == scan->device().clock())
					if (soundtags.add(scan->device().tag(), 1, FALSE) != TMERR_DUPLICATE)
						count++;
			}
			// if more than one, prepend a #x in front of the CPU name
			if (count > 1)
				strcatprintf(str,"  %d" UTF8_MULTIPLY, count);
			else
				str.append("  ");
			str.append(sound->device().name());

			// display clock in kHz or MHz
			int clock = sound->device().clock();
			if (clock >= 1000000)
				strcatprintf(str," %d.%06d" UTF8_NBSP "MHz\n", clock / 1000000, clock % 1000000);
			else if (clock != 0)
				strcatprintf(str," %d.%03d" UTF8_NBSP "kHz\n", clock / 1000, clock % 1000);
			else
				str.append("\n");
		}
	}

	// scan for BIOS settings
	int bios = 0;
	if (dev->rom_region())
	{
		std::string bios_str;
		// first loop through roms in search of default bios (shortname)
		for (const rom_entry *rom = dev->rom_region(); !ROMENTRY_ISEND(rom); rom++)
			if (ROMENTRY_ISDEFAULT_BIOS(rom))
				bios_str.assign(ROM_GETNAME(rom));

		// then loop again to count bios options and to get the default bios complete name
		for (const rom_entry *rom = dev->rom_region(); !ROMENTRY_ISEND(rom); rom++)
		{
			if (ROMENTRY_ISSYSTEM_BIOS(rom))
			{
				bios++;
				if (bios_str.compare(ROM_GETNAME(rom))==0)
					bios_str.assign(ROM_GETHASHDATA(rom));
			}
		}

		if (bios)
			strcatprintf(str,"* BIOS settings:\n  %d options    [default: %s]\n", bios, bios_str.c_str());
	}

	int input = 0, input_mj = 0, input_hana = 0, input_gamble = 0, input_analog = 0, input_adjust = 0;
	int input_keypad = 0, input_keyboard = 0, dips = 0, confs = 0;
	std::string errors, dips_opt, confs_opt;
	ioport_list portlist;
	device_iterator iptiter(*dev);
	for (device_t *iptdev = iptiter.first(); iptdev != NULL; iptdev = iptiter.next())
		portlist.append(*iptdev, errors);

	// check if the device adds inputs to the system
	for (ioport_port *port = portlist.first(); port != NULL; port = port->next())
		for (ioport_field *field = port->first_field(); field != NULL; field = field->next())
		{
			if (field->type() >= IPT_MAHJONG_FIRST && field->type() < IPT_MAHJONG_LAST)
				input_mj++;
			else if (field->type() >= IPT_HANAFUDA_FIRST && field->type() < IPT_HANAFUDA_LAST)
				input_hana++;
			else if (field->type() >= IPT_GAMBLING_FIRST && field->type() < IPT_GAMBLING_LAST)
				input_gamble++;
			else if (field->type() >= IPT_ANALOG_FIRST && field->type() < IPT_ANALOG_LAST)
				input_analog++;
			else if (field->type() == IPT_ADJUSTER)
				input_adjust++;
			else if (field->type() == IPT_KEYPAD)
				input_keypad++;
			else if (field->type() == IPT_KEYBOARD)
				input_keyboard++;
			else if (field->type() >= IPT_START1 && field->type() < IPT_UI_FIRST)
				input++;
			else if (field->type() == IPT_DIPSWITCH)
			{
				dips++;
				dips_opt.append("  ").append(field->name());
				for (ioport_setting *setting = field->first_setting(); setting != NULL; setting = setting->next())
				{
					if (setting->value() == field->defvalue())
					{
						strcatprintf(dips_opt, "    [default: %s]\n", setting->name());
						break;
					}
				}
			}
			else if (field->type() == IPT_CONFIG)
			{
				confs++;
				confs_opt.append("  ").append(field->name());
				for (ioport_setting *setting = field->first_setting(); setting != NULL; setting = setting->next())
				{
					if (setting->value() == field->defvalue())
					{
						strcatprintf(confs_opt,"    [default: %s]\n", setting->name());
						break;
					}
				}
			}
		}

	if (dips)
		str.append("* Dispwitch settings:\n").append(dips_opt);
	if (confs)
		str.append("* Configuration settings:\n").append(confs_opt);
	if (input + input_mj + input_hana + input_gamble + input_analog + input_adjust + input_keypad + input_keyboard)
		str.append("* Input device(s):\n");
	if (input)
		strcatprintf(str,"  User inputs    [%d inputs]\n", input);
	if (input_mj)
		strcatprintf(str,"  Mahjong inputs    [%d inputs]\n", input_mj);
	if (input_hana)
		strcatprintf(str,"  Hanafuda inputs    [%d inputs]\n", input_hana);
	if (input_gamble)
		strcatprintf(str,"  Gambling inputs    [%d inputs]\n", input_gamble);
	if (input_analog)
		strcatprintf(str,"  Analog inputs    [%d inputs]\n", input_analog);
	if (input_adjust)
		strcatprintf(str,"  Adjuster inputs    [%d inputs]\n", input_adjust);
	if (input_keypad)
		strcatprintf(str,"  Keypad inputs    [%d inputs]\n", input_keypad);
	if (input_keyboard)
		strcatprintf(str,"  Keyboard inputs    [%d inputs]\n", input_keyboard);

	image_interface_iterator imgiter(*dev);
	if (imgiter.count() > 0)
	{
		str.append("* Media Options:\n");
		for (const device_image_interface *imagedev = imgiter.first(); imagedev != NULL; imagedev = imgiter.next())
			strcatprintf(str,"  %s    [tag: %s]\n", imagedev->image_type_name(), imagedev->device().tag());
	}

	slot_interface_iterator slotiter(*dev);
	if (slotiter.count() > 0)
	{
		str.append("* Slot Options:\n");
		for (const device_slot_interface *slot = slotiter.first(); slot != NULL; slot = slotiter.next())
			strcatprintf(str,"  %s    [default: %s]\n", slot->device().tag(), slot->default_option() ? slot->default_option() : "----");
	}

	if ((execiter.count() + scriter.count() + snditer.count() + imgiter.count() + slotiter.count() + bios + dips + confs
			+ input + input_mj + input_hana + input_gamble + input_analog + input_adjust + input_keypad + input_keyboard) == 0)
			str.append("[None]\n");

	const_cast<machine_config &>(machine().config()).device_remove(&machine().config().root_device(), m_option->name());
	item_append(str.c_str(), NULL, MENU_FLAG_MULTILINE, NULL);
}

void ui_menu_device_config::handle()
{
	/* process the menu */
	process(0);
}

ui_menu_device_config::~ui_menu_device_config()
{
}
