// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Nathan Woods
/*********************************************************************

    miscmenu.c

    Internal MAME menus for the user interface.

*********************************************************************/

#include "emu.h"
#include "osdnet.h"

#include "uiinput.h"
#include "ui/ui.h"
#include "ui/menu.h"
#include "ui/miscmenu.h"


/***************************************************************************
    MENU HANDLERS
***************************************************************************/

/*-------------------------------------------------
    ui_menu_keyboard_mode - menu that
-------------------------------------------------*/

ui_menu_keyboard_mode::ui_menu_keyboard_mode(running_machine &machine, render_container *container) : ui_menu(machine, container)
{
}

void ui_menu_keyboard_mode::populate()
{
	bool natural = machine().ui().use_natural_keyboard();
	item_append("Keyboard Mode:", natural ? "Natural" : "Emulated", natural ? MENU_FLAG_LEFT_ARROW : MENU_FLAG_RIGHT_ARROW, nullptr);
}

ui_menu_keyboard_mode::~ui_menu_keyboard_mode()
{
}

void ui_menu_keyboard_mode::handle()
{
	bool natural = machine().ui().use_natural_keyboard();

	/* process the menu */
	const ui_menu_event *menu_event = process(0);

	if (menu_event != nullptr)
	{
		if (menu_event->iptkey == IPT_UI_LEFT || menu_event->iptkey == IPT_UI_RIGHT)
		{
			machine().ui().set_use_natural_keyboard(natural ^ true);
			reset(UI_MENU_RESET_REMEMBER_REF);
		}
	}
}


/*-------------------------------------------------
    ui_menu_bios_selection - populates the main
    bios selection menu
-------------------------------------------------*/

ui_menu_bios_selection::ui_menu_bios_selection(running_machine &machine, render_container *container) : ui_menu(machine, container)
{
}

void ui_menu_bios_selection::populate()
{
	/* cycle through all devices for this system */
	device_iterator deviter(machine().root_device());
	for (device_t *device = deviter.first(); device != nullptr; device = deviter.next())
	{
		if (device->rom_region()) {
			const char *val = "default";
			for (const rom_entry *rom = device->rom_region(); !ROMENTRY_ISEND(rom); rom++)
			{
				if (ROMENTRY_ISSYSTEM_BIOS(rom) && ROM_GETBIOSFLAGS(rom)==device->system_bios())
				{
					val = ROM_GETHASHDATA(rom);
				}
			}
			item_append(device->tag()==":" ? "driver" : device->tag().c_str()+1, val, MENU_FLAG_LEFT_ARROW | MENU_FLAG_RIGHT_ARROW, (void *)device);
		}
	}

	item_append(MENU_SEPARATOR_ITEM, nullptr, 0, nullptr);
	item_append("Reset",  nullptr, 0, (void *)1);
}

ui_menu_bios_selection::~ui_menu_bios_selection()
{
}

/*-------------------------------------------------
    ui_menu_bios_selection - menu that
-------------------------------------------------*/

void ui_menu_bios_selection::handle()
{
	/* process the menu */
	const ui_menu_event *menu_event = process(0);

	if (menu_event != nullptr && menu_event->itemref != nullptr)
	{
		if ((FPTR)menu_event->itemref == 1 && menu_event->iptkey == IPT_UI_SELECT)
			machine().schedule_hard_reset();
		else if (menu_event->iptkey == IPT_UI_LEFT || menu_event->iptkey == IPT_UI_RIGHT)
		{
			device_t *dev = (device_t *)menu_event->itemref;
			int cnt = 0;
			for (const rom_entry *rom = dev->rom_region(); !ROMENTRY_ISEND(rom); rom++)
			{
				if (ROMENTRY_ISSYSTEM_BIOS(rom)) cnt ++;
			}
			int val = dev->system_bios() + ((menu_event->iptkey == IPT_UI_LEFT) ? -1 : +1);
			if (val<1) val=cnt;
			if (val>cnt) val=1;
			dev->set_system_bios(val);
			if (dev->tag()!=":") {
				std::string error;
				machine().options().set_value("bios", val-1, OPTION_PRIORITY_CMDLINE, error);
				assert(error.empty());
			} else {
				std::string error;
				std::string value = machine().options().main_value(dev->owner()->tag().c_str()+1);
				strcatprintf(value,",bios=%d",val-1);
				machine().options().set_value(dev->owner()->tag().c_str()+1, value.c_str(), OPTION_PRIORITY_CMDLINE, error);
				assert(error.empty());
			}
			reset(UI_MENU_RESET_REMEMBER_REF);
		}
	}
}



ui_menu_network_devices::ui_menu_network_devices(running_machine &machine, render_container *container) : ui_menu(machine, container)
{
}

ui_menu_network_devices::~ui_menu_network_devices()
{
}

/*-------------------------------------------------
    menu_network_devices_populate - populates the main
    network device menu
-------------------------------------------------*/

void ui_menu_network_devices::populate()
{
	/* cycle through all devices for this system */
	network_interface_iterator iter(machine().root_device());
	for (device_network_interface *network = iter.first(); network != nullptr; network = iter.next())
	{
		int curr = network->get_interface();
		const char *title = nullptr;
		const osd_netdev::entry_t *entry = netdev_first();
		while(entry) {
			if(entry->id==curr) {
				title = entry->description;
				break;
			}
			entry = entry->m_next;
		}

		item_append(network->device().tag().c_str(),  (title) ? title : "------", MENU_FLAG_LEFT_ARROW | MENU_FLAG_RIGHT_ARROW, (void *)network);
	}
}

/*-------------------------------------------------
    ui_menu_network_devices - menu that
-------------------------------------------------*/

void ui_menu_network_devices::handle()
{
	/* process the menu */
	const ui_menu_event *menu_event = process(0);

	if (menu_event != nullptr && menu_event->itemref != nullptr)
	{
		if (menu_event->iptkey == IPT_UI_LEFT || menu_event->iptkey == IPT_UI_RIGHT) {
			device_network_interface *network = (device_network_interface *)menu_event->itemref;
			int curr = network->get_interface();
			if (menu_event->iptkey == IPT_UI_LEFT) curr--; else curr++;
			if (curr==-2) curr = netdev_count() - 1;
			network->set_interface(curr);
			reset(UI_MENU_RESET_REMEMBER_REF);
		}
	}
}


/*-------------------------------------------------
    menu_bookkeeping - handle the bookkeeping
    information menu
-------------------------------------------------*/

void ui_menu_bookkeeping::handle()
{
	attotime curtime;

	/* if the time has rolled over another second, regenerate */
	curtime = machine().time();
	if (prevtime.seconds() != curtime.seconds())
	{
		reset(UI_MENU_RESET_SELECT_FIRST);
		prevtime = curtime;
		populate();
	}

	/* process the menu */
	process(0);
}


/*-------------------------------------------------
    menu_bookkeeping - handle the bookkeeping
    information menu
-------------------------------------------------*/
ui_menu_bookkeeping::ui_menu_bookkeeping(running_machine &machine, render_container *container) : ui_menu(machine, container)
{
}

ui_menu_bookkeeping::~ui_menu_bookkeeping()
{
}

void ui_menu_bookkeeping::populate()
{
	int tickets = machine().bookkeeping().get_dispensed_tickets();
	std::string tempstring;
	int ctrnum;

	/* show total time first */
	if (prevtime.seconds() >= 60 * 60)
		strcatprintf(tempstring, "Uptime: %d:%02d:%02d\n\n", prevtime.seconds() / (60 * 60), (prevtime.seconds() / 60) % 60, prevtime.seconds() % 60);
	else
		strcatprintf(tempstring,"Uptime: %d:%02d\n\n", (prevtime.seconds() / 60) % 60, prevtime.seconds() % 60);

	/* show tickets at the top */
	if (tickets > 0)
		strcatprintf(tempstring,"Tickets dispensed: %d\n\n", tickets);

	/* loop over coin counters */
	for (ctrnum = 0; ctrnum < COIN_COUNTERS; ctrnum++)
	{
		int count = machine().bookkeeping().coin_counter_get_count(ctrnum);

		/* display the coin counter number */
		strcatprintf(tempstring,"Coin %c: ", ctrnum + 'A');

		/* display how many coins */
		if (count == 0)
			tempstring.append("NA");
		else
			strcatprintf(tempstring, "%d", count);

		/* display whether or not we are locked out */
		if (machine().bookkeeping().coin_lockout_get_state(ctrnum))
			tempstring.append(" (locked)");
		tempstring.append("\n");
	}

	/* append the single item */
	item_append(tempstring.c_str(), nullptr, MENU_FLAG_MULTILINE, nullptr);
}

/*-------------------------------------------------
    menu_crosshair - handle the crosshair settings
    menu
-------------------------------------------------*/

void ui_menu_crosshair::handle()
{
	/* process the menu */
	const ui_menu_event *menu_event = process(UI_MENU_PROCESS_LR_REPEAT);

	/* handle events */
	if (menu_event != nullptr && menu_event->itemref != nullptr)
	{
		crosshair_user_settings settings;
		crosshair_item_data *data = (crosshair_item_data *)menu_event->itemref;
		bool changed = false;
		//int set_def = false;
		int newval = data->cur;

		/* retreive the user settings */
		machine().crosshair().get_user_settings(data->player, &settings);

		switch (menu_event->iptkey)
		{
			/* if selected, reset to default value */
			case IPT_UI_SELECT:
				newval = data->defvalue;
				//set_def = true;
				break;

			/* left decrements */
			case IPT_UI_LEFT:
				newval -= machine().input().code_pressed(KEYCODE_LSHIFT) ? 10 : 1;
				break;

			/* right increments */
			case IPT_UI_RIGHT:
				newval += machine().input().code_pressed(KEYCODE_LSHIFT) ? 10 : 1;
				break;
		}

		/* clamp to range */
		if (newval < data->min)
			newval = data->min;
		if (newval > data->max)
			newval = data->max;

		/* if things changed, update */
		if (newval != data->cur)
		{
			switch (data->type)
			{
				/* visibility state */
				case CROSSHAIR_ITEM_VIS:
					settings.mode = newval;
					changed = true;
					break;

				/* auto time */
				case CROSSHAIR_ITEM_AUTO_TIME:
					settings.auto_time = newval;
					changed = true;
					break;
			}
		}

		/* crosshair graphic name */
		if (data->type == CROSSHAIR_ITEM_PIC)
		{
			switch (menu_event->iptkey)
			{
				case IPT_UI_SELECT:
					/* clear the name string to reset to default crosshair */
					settings.name[0] = 0;
					changed = true;
					break;

				case IPT_UI_LEFT:
					strcpy(settings.name, data->last_name);
					changed = true;
					break;

				case IPT_UI_RIGHT:
					strcpy(settings.name, data->next_name);
					changed = true;
					break;
			}
		}

		if (changed)
		{
			/* save the user settings */
			machine().crosshair().set_user_settings(data->player, &settings);

			/* rebuild the menu */
			reset(UI_MENU_RESET_REMEMBER_POSITION);
		}
	}
}


/*-------------------------------------------------
    menu_crosshair_populate - populate the
    crosshair settings menu
-------------------------------------------------*/

ui_menu_crosshair::ui_menu_crosshair(running_machine &machine, render_container *container) : ui_menu(machine, container)
{
}

void ui_menu_crosshair::populate()
{
	crosshair_user_settings settings;
	crosshair_item_data *data;
	char temp_text[16];
	int player;
	UINT8 use_auto = false;
	UINT32 flags = 0;

	/* loop over player and add the manual items */
	for (player = 0; player < MAX_PLAYERS; player++)
	{
		/* get the user settings */
		machine().crosshair().get_user_settings(player, &settings);

		/* add menu items for usable crosshairs */
		if (settings.used)
		{
			/* Make sure to keep these matched to the CROSSHAIR_VISIBILITY_xxx types */
			static const char *const vis_text[] = { "Off", "On", "Auto" };

			/* track if we need the auto time menu */
			if (settings.mode == CROSSHAIR_VISIBILITY_AUTO) use_auto = true;

			/* CROSSHAIR_ITEM_VIS - allocate a data item and fill it */
			data = (crosshair_item_data *)m_pool_alloc(sizeof(*data));
			data->type = CROSSHAIR_ITEM_VIS;
			data->player = player;
			data->min = CROSSHAIR_VISIBILITY_OFF;
			data->max = CROSSHAIR_VISIBILITY_AUTO;
			data->defvalue = CROSSHAIR_VISIBILITY_DEFAULT;
			data->cur = settings.mode;

			/* put on arrows */
			if (data->cur > data->min)
				flags |= MENU_FLAG_LEFT_ARROW;
			if (data->cur < data->max)
				flags |= MENU_FLAG_RIGHT_ARROW;

			/* add CROSSHAIR_ITEM_VIS menu */
			sprintf(temp_text, "P%d Visibility", player + 1);
			item_append(temp_text, vis_text[settings.mode], flags, data);

			/* CROSSHAIR_ITEM_PIC - allocate a data item and fill it */
			data = (crosshair_item_data *)m_pool_alloc(sizeof(*data));
			data->type = CROSSHAIR_ITEM_PIC;
			data->player = player;
			data->last_name[0] = 0;
			/* other data item not used by this menu */

			/* search for crosshair graphics */

			/* open a path to the crosshairs */
			file_enumerator path(machine().options().crosshair_path());
			const osd_directory_entry *dir;
			/* reset search flags */
			int using_default = false;
			int finished = false;
			int found = false;

			/* if we are using the default, then we just need to find the first in the list */
			if (*(settings.name) == 0)
				using_default = true;

			/* look for the current name, then remember the name before */
			/* and find the next name */
			while (((dir = path.next()) != nullptr) && !finished)
			{
				int length = strlen(dir->name);

				/* look for files ending in .png with a name not larger then 9 chars*/
				if ((length > 4) && (length <= CROSSHAIR_PIC_NAME_LENGTH + 4) &&
					dir->name[length - 4] == '.' &&
					tolower((UINT8)dir->name[length - 3]) == 'p' &&
					tolower((UINT8)dir->name[length - 2]) == 'n' &&
					tolower((UINT8)dir->name[length - 1]) == 'g')

				{
					/* remove .png from length */
					length -= 4;

					if (found || using_default)
					{
						/* get the next name */
						strncpy(data->next_name, dir->name, length);
						data->next_name[length] = 0;
						finished = true;
					}
					else if (!strncmp(dir->name, settings.name, length))
					{
						/* we found the current name */
						/* so loop once more to find the next name */
						found = true;
					}
					else
						/* remember last name */
						/* we will do it here in case files get added to the directory */
					{
						strncpy(data->last_name, dir->name, length);
						data->last_name[length] = 0;
					}
				}
			}
			/* if name not found then next item is DEFAULT */
			if (!found && !using_default)
			{
				data->next_name[0] = 0;
				finished = true;
			}
			/* setup the selection flags */
			flags = 0;
			if (finished)
				flags |= MENU_FLAG_RIGHT_ARROW;
			if (found)
				flags |= MENU_FLAG_LEFT_ARROW;

			/* add CROSSHAIR_ITEM_PIC menu */
			sprintf(temp_text, "P%d Crosshair", player + 1);
			item_append(temp_text, using_default ? "DEFAULT" : settings.name, flags, data);
		}
	}
	if (use_auto)
	{
		/* any player can be used to get the autotime */
		machine().crosshair().get_user_settings(0, &settings);

		/* CROSSHAIR_ITEM_AUTO_TIME - allocate a data item and fill it */
		data = (crosshair_item_data *)m_pool_alloc(sizeof(*data));
		data->type = CROSSHAIR_ITEM_AUTO_TIME;
		data->min = CROSSHAIR_VISIBILITY_AUTOTIME_MIN;
		data->max = CROSSHAIR_VISIBILITY_AUTOTIME_MAX;
		data->defvalue = CROSSHAIR_VISIBILITY_AUTOTIME_DEFAULT;
		data->cur = settings.auto_time;

		/* put on arrows in visible menu */
		if (data->cur > data->min)
			flags |= MENU_FLAG_LEFT_ARROW;
		if (data->cur < data->max)
			flags |= MENU_FLAG_RIGHT_ARROW;

		/* add CROSSHAIR_ITEM_AUTO_TIME menu */
		sprintf(temp_text, "%d", settings.auto_time);
		item_append("Visible Delay", temp_text, flags, data);
	}
//  else
//      /* leave a blank filler line when not in auto time so size does not rescale */
//      item_append("", "", NULL, NULL);
}

ui_menu_crosshair::~ui_menu_crosshair()
{
}

/*-------------------------------------------------
    menu_quit_game - handle the "menu" for
    quitting the game
-------------------------------------------------*/

ui_menu_quit_game::ui_menu_quit_game(running_machine &machine, render_container *container) : ui_menu(machine, container)
{
}

ui_menu_quit_game::~ui_menu_quit_game()
{
}

void ui_menu_quit_game::populate()
{
}

void ui_menu_quit_game::handle()
{
	/* request a reset */
	machine().schedule_exit();

	/* reset the menu stack */
	ui_menu::stack_reset(machine());
}
