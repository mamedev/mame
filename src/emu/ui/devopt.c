/*********************************************************************

    ui/devopt.c

    Internal menu for the device configuration.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

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
	astring tmp_tag;
	tmp_tag.cpy(slot->device().tag()).cat(":").cat(option->name());
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
	astring string;
	device_t *dev;

	string.printf("[This option is%s currently mounted in the running system]\n\n", m_mounted ? "" : " NOT");
	string.catprintf("Option: %s\n", m_option->name());

	dev = const_cast<machine_config &>(machine().config()).device_add(&machine().config().root_device(), m_option->name(), m_option->devtype(), 0);

	string.catprintf("Device: %s\n", dev->name());
	if (!m_mounted)
		string.cat("\nIf you select this option, the following items will be enabled:\n");
	else
		string.cat("\nThe selected option enables the following items:\n");

	// loop over all CPUs
	execute_interface_iterator execiter(*dev);
	if (execiter.count() > 0)
	{
		string.cat("* CPU:\n");
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
				string.catprintf("  %d" UTF8_MULTIPLY, count);
			else
				string.cat("  ");
			string.cat(name);

			// display clock in kHz or MHz
			if (clock >= 1000000)
				string.catprintf(" %d.%06d" UTF8_NBSP "MHz\n", clock / 1000000, clock % 1000000);
			else
				string.catprintf(" %d.%03d" UTF8_NBSP "kHz\n", clock / 1000, clock % 1000);
		}
	}

	// display screen information
	screen_device_iterator scriter(*dev);
	if (scriter.count() > 0)
	{
		string.cat("* Video:\n");
		for (screen_device *screen = scriter.first(); screen != NULL; screen = scriter.next())
		{
			string.catprintf("  Screen '%s': ", screen->tag());

			if (screen->screen_type() == SCREEN_TYPE_VECTOR)
				string.cat("Vector\n");
			else
			{
				const rectangle &visarea = screen->visible_area();

				string.catprintf("%d " UTF8_MULTIPLY " %d (%s) %f" UTF8_NBSP "Hz\n",
									visarea.width(), visarea.height(),
									(machine().system().flags & ORIENTATION_SWAP_XY) ? "V" : "H",
									ATTOSECONDS_TO_HZ(screen->frame_period().attoseconds));
			}
		}
	}

	// loop over all sound chips
	sound_interface_iterator snditer(*dev);
	if (snditer.count() > 0)
	{
		string.cat("* Sound:\n");
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
				string.catprintf("  %d" UTF8_MULTIPLY, count);
			else
				string.cat("  ");
			string.cat(sound->device().name());

			// display clock in kHz or MHz
			int clock = sound->device().clock();
			if (clock >= 1000000)
				string.catprintf(" %d.%06d" UTF8_NBSP "MHz\n", clock / 1000000, clock % 1000000);
			else if (clock != 0)
				string.catprintf(" %d.%03d" UTF8_NBSP "kHz\n", clock / 1000, clock % 1000);
			else
				string.cat("\n");
		}
	}

	// scan for BIOS settings
	int bios = 0;
	if (dev->rom_region())
	{
		astring bios_str;
		// first loop through roms in search of default bios (shortname)
		for (const rom_entry *rom = dev->rom_region(); !ROMENTRY_ISEND(rom); rom++)
			if (ROMENTRY_ISDEFAULT_BIOS(rom))
				bios_str.cpy(ROM_GETNAME(rom));

		// then loop again to count bios options and to get the default bios complete name
		for (const rom_entry *rom = dev->rom_region(); !ROMENTRY_ISEND(rom); rom++)
		{
			if (ROMENTRY_ISSYSTEM_BIOS(rom))
			{
				bios++;
				if (bios_str == ROM_GETNAME(rom))
					bios_str.cpy(ROM_GETHASHDATA(rom));
			}
		}

		if (bios)
			string.catprintf("* BIOS settings:\n  %d options    [default: %s]\n", bios, bios_str.c_str());
	}

	int input = 0, input_mj = 0, input_hana = 0, input_gamble = 0, input_analog = 0, input_adjust = 0;
	int input_keypad = 0, input_keyboard = 0, dips = 0, confs = 0;
	astring errors, dips_opt, confs_opt;
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
				dips_opt.cat("  ").cat(field->name());
				for (ioport_setting *setting = field->first_setting(); setting != NULL; setting = setting->next())
				{
					if (setting->value() == field->defvalue())
					{
						dips_opt.catprintf("    [default: %s]\n", setting->name());
						break;
					}
				}
			}
			else if (field->type() == IPT_CONFIG)
			{
				confs++;
				confs_opt.cat("  ").cat(field->name());
				for (ioport_setting *setting = field->first_setting(); setting != NULL; setting = setting->next())
				{
					if (setting->value() == field->defvalue())
					{
						confs_opt.catprintf("    [default: %s]\n", setting->name());
						break;
					}
				}
			}
		}

	if (dips)
		string.cat("* Dispwitch settings:\n").cat(dips_opt);
	if (confs)
		string.cat("* Configuration settings:\n").cat(confs_opt);
	if (input + input_mj + input_hana + input_gamble + input_analog + input_adjust + input_keypad + input_keyboard)
		string.cat("* Input device(s):\n");
	if (input)
		string.catprintf("  User inputs    [%d inputs]\n", input);
	if (input_mj)
		string.catprintf("  Mahjong inputs    [%d inputs]\n", input_mj);
	if (input_hana)
		string.catprintf("  Hanafuda inputs    [%d inputs]\n", input_hana);
	if (input_gamble)
		string.catprintf("  Gambling inputs    [%d inputs]\n", input_gamble);
	if (input_analog)
		string.catprintf("  Analog inputs    [%d inputs]\n", input_analog);
	if (input_adjust)
		string.catprintf("  Adjuster inputs    [%d inputs]\n", input_adjust);
	if (input_keypad)
		string.catprintf("  Keypad inputs    [%d inputs]\n", input_keypad);
	if (input_keyboard)
		string.catprintf("  Keyboard inputs    [%d inputs]\n", input_keyboard);

	image_interface_iterator imgiter(*dev);
	if (imgiter.count() > 0)
	{
		string.cat("* Media Options:\n");
		for (const device_image_interface *imagedev = imgiter.first(); imagedev != NULL; imagedev = imgiter.next())
			string.catprintf("  %s    [tag: %s]\n", imagedev->image_type_name(), imagedev->device().tag());
	}

	slot_interface_iterator slotiter(*dev);
	if (slotiter.count() > 0)
	{
		string.cat("* Slot Options:\n");
		for (const device_slot_interface *slot = slotiter.first(); slot != NULL; slot = slotiter.next())
			string.catprintf("  %s    [default: %s]\n", slot->device().tag(), slot->default_option() ? slot->default_option() : "----");
	}

	if ((execiter.count() + scriter.count() + snditer.count() + imgiter.count() + slotiter.count() + bios + dips + confs
			+ input + input_mj + input_hana + input_gamble + input_analog + input_adjust + input_keypad + input_keyboard) == 0)
		string.cat("[None]\n");

	const_cast<machine_config &>(machine().config()).device_remove(&machine().config().root_device(), m_option->name());
	item_append(string, NULL, MENU_FLAG_MULTILINE, NULL);
}

void ui_menu_device_config::handle()
{
	/* process the menu */
	process(0);
}

ui_menu_device_config::~ui_menu_device_config()
{
}
