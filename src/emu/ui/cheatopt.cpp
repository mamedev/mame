// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Nathan Woods
/*********************************************************************

    ui/cheatopt.cpp

    Internal menu for the cheat interface.

*********************************************************************/

#include "emu.h"
#include "cheat.h"

#include "ui/ui.h"
#include "ui/menu.h"
#include "ui/cheatopt.h"

/*-------------------------------------------------
    menu_cheat - handle the cheat menu
-------------------------------------------------*/

void ui_menu_cheat::handle()
{
	/* process the menu */
	const ui_menu_event *menu_event = process(UI_MENU_PROCESS_LR_REPEAT);


	/* handle events */
	if (menu_event != nullptr && menu_event->itemref != nullptr)
	{
		bool changed = false;

		/* clear cheat comment on any movement or keypress */
		machine().popmessage();

		/* handle reset all + reset all cheats for reload all option */
		if ((menu_event->itemref == ITEMREF_CHEATS_RESET_ALL || menu_event->itemref == ITEMREF_CHEATS_RELOAD_ALL) && menu_event->iptkey == IPT_UI_SELECT)
		{
			for (cheat_entry &curcheat : machine().cheat().entries())
				if (curcheat.select_default_state())
					changed = true;
		}

		/* handle individual cheats */
		else if (menu_event->itemref >= ITEMREF_CHEATS_FIRST_ITEM)
		{
			cheat_entry *curcheat = reinterpret_cast<cheat_entry *>(menu_event->itemref);
			const char *string;
			switch (menu_event->iptkey)
			{
				/* if selected, activate a oneshot */
				case IPT_UI_SELECT:
					changed = curcheat->activate();
					break;

				/* if cleared, reset to default value */
				case IPT_UI_CLEAR:
					changed = curcheat->select_default_state();
					break;

				/* left decrements */
				case IPT_UI_LEFT:
					changed = curcheat->select_previous_state();
					break;

				/* right increments */
				case IPT_UI_RIGHT:
					changed = curcheat->select_next_state();
					break;

				/* bring up display comment if one exists */
				case IPT_UI_DISPLAY_COMMENT:
				case IPT_UI_UP:
				case IPT_UI_DOWN:
					string = curcheat->comment();
					if (string != nullptr && string[0] != 0)
						machine().popmessage(_("Cheat Comment:\n%s"), string);
					break;
			}
		}

		/* handle reload all  */
		if (menu_event->itemref == ITEMREF_CHEATS_RELOAD_ALL && menu_event->iptkey == IPT_UI_SELECT)
		{
			/* re-init cheat engine and thus reload cheats/cheats have already been turned off by here */
			machine().cheat().reload();

			/* display the reloaded cheats */
			reset(UI_MENU_RESET_REMEMBER_REF);
			machine().popmessage(_("All cheats reloaded"));
		}

		/* handle autofire menu */
		if (menu_event->itemref == ITEMREF_CHEATS_AUTOFIRE_SETTINGS && menu_event->iptkey == IPT_UI_SELECT)
		{
			ui_menu::stack_push(global_alloc_clear<ui_menu_autofire>(machine(), container));
		}

		/* if things changed, update */
		if (changed)
			reset(UI_MENU_RESET_REMEMBER_REF);
	}
}


/*-------------------------------------------------
    menu_cheat_populate - populate the cheat menu
-------------------------------------------------*/

ui_menu_cheat::ui_menu_cheat(running_machine &machine, render_container *container) : ui_menu(machine, container)
{
}

void ui_menu_cheat::populate()
{
	/* iterate over cheats */
	std::string text;
	std::string subtext;

	// add the autofire menu
	item_append(_("Autofire Settings"), nullptr, 0, (void *)ITEMREF_CHEATS_AUTOFIRE_SETTINGS);

	/* add a separator */
	item_append(MENU_SEPARATOR_ITEM, nullptr, 0, nullptr);

	// add other cheats
	if (!machine().cheat().entries().empty()) {
		for (cheat_entry &curcheat : machine().cheat().entries())
		{
			UINT32 flags;
			curcheat.menu_text(text, subtext, flags);
			item_append(text.c_str(), subtext.c_str(), flags, &curcheat);
		}

		/* add a separator */
		item_append(MENU_SEPARATOR_ITEM, nullptr, 0, nullptr);

		/* add a reset all option */
		item_append(_("Reset All"), nullptr, 0, (void *)ITEMREF_CHEATS_RESET_ALL);

		/* add a reload all cheats option */
		item_append(_("Reload All"), nullptr, 0, (void *)ITEMREF_CHEATS_RELOAD_ALL);
	}
}

ui_menu_cheat::~ui_menu_cheat()
{
}





/*-------------------------------------------------
    menu_autofire - handle the autofire settings
    menu
-------------------------------------------------*/

ui_menu_autofire::ui_menu_autofire(running_machine &machine, render_container *container) : ui_menu(machine, container)
{
	screen_device_iterator iter(machine.root_device());
	const screen_device *screen = iter.first();

	if (screen == nullptr)
	{
		refresh = 60.0;
	}
	else
	{
		refresh = ATTOSECONDS_TO_HZ(screen->refresh_attoseconds());
	}
}

ui_menu_autofire::~ui_menu_autofire()
{
}

void ui_menu_autofire::handle()
{
	ioport_field *field;
	bool changed = false;

	/* process the menu */
	const ui_menu_event *menu_event = process(0);

	/* handle events */
	if (menu_event != nullptr && menu_event->itemref != nullptr)
	{
		// menu item is changed using left/right keys only
		if (menu_event->iptkey == IPT_UI_LEFT || menu_event->iptkey == IPT_UI_RIGHT)
		{
			if (menu_event->itemref == ITEMREF_AUTOFIRE_STATUS)
			{
				// toggle autofire status
				bool autofire_toggle = machine().ioport().get_autofire_toggle();    // (menu_event->iptkey == IPT_UI_LEFT);
				machine().ioport().set_autofire_toggle(!autofire_toggle);
				changed = true;
			}
			else if (menu_event->itemref == ITEMREF_AUTOFIRE_DELAY)
			{
				// change autofire frequency
				int autofire_delay = machine().ioport().get_autofire_delay();
				if (menu_event->iptkey == IPT_UI_LEFT)
				{
					autofire_delay--;
					if (autofire_delay < 1)
						autofire_delay = 1;
				}
				else
				{
					autofire_delay++;
					if (autofire_delay > 30)
						autofire_delay = 30;
				}
				machine().ioport().set_autofire_delay(autofire_delay);
				changed = true;
			}
			else
			{
				// enable autofire on specific button
				field = (ioport_field *)menu_event->itemref;
				ioport_field::user_settings settings;
				field->get_user_settings(settings);
				settings.autofire = (menu_event->iptkey == IPT_UI_RIGHT);
				field->set_user_settings(settings);
				changed = true;
			}
		}
	}

	// if toggle settings changed, redraw menu to reflect new options
	if (!changed)
	{
		changed = (last_toggle != machine().ioport().get_autofire_toggle());
	}

	/* if something changed, rebuild the menu */
	if (changed)
	{
		reset(UI_MENU_RESET_REMEMBER_REF);
	}
}


/*-------------------------------------------------
    menu_autofire_populate - populate the autofire
    menu
-------------------------------------------------*/

void ui_menu_autofire::populate()
{
	char temp_text[64];

	/* add autofire toggle item */
	bool autofire_toggle = machine().ioport().get_autofire_toggle();
	item_append(_("Autofire Status"), (autofire_toggle ? _("Disabled") : _("Enabled")),
			(autofire_toggle ? MENU_FLAG_RIGHT_ARROW : MENU_FLAG_LEFT_ARROW), (void *)ITEMREF_AUTOFIRE_STATUS);

	/* iterate over the input ports and add autofire toggle items */
	int menu_items = 0;
	for (ioport_port &port : machine().ioport().ports())
	{
		bool is_first_button = true;
		for (ioport_field &field : port.fields())
		{
			if ((field.name()) && ((field.type() >= IPT_BUTTON1 && field.type() <= IPT_BUTTON16))) // IPT_BUTTON1 + 15)))
			{
				menu_items++;
				ioport_field::user_settings settings;
				field.get_user_settings(settings);

				if (is_first_button)
				{
					/* add a separator for each player */
					item_append(MENU_SEPARATOR_ITEM, nullptr, 0, nullptr);
					is_first_button = false;
				}
				/* add an autofire item */
				if (!autofire_toggle)
				{
					// item is enabled and can be switched to values on/off
					item_append(field.name(), (settings.autofire ? _("On") : _("Off")),
							(settings.autofire ? MENU_FLAG_LEFT_ARROW : MENU_FLAG_RIGHT_ARROW), (void *)&field);
				}
				else
				{
					// item is disabled
					item_append(field.name(), (settings.autofire ? _("On") : _("Off")),
							MENU_FLAG_DISABLE | MENU_FLAG_INVERT, nullptr);
				}
			}
		}
	}

	/* add text item if no buttons found */
	if (menu_items==0)
	{
		item_append(MENU_SEPARATOR_ITEM, nullptr, 0, nullptr);
		item_append(_("No buttons found on this machine!"), nullptr, MENU_FLAG_DISABLE, nullptr);
	}

	/* add a separator */
	item_append(MENU_SEPARATOR_ITEM, nullptr, 0, nullptr);

	/* add autofire delay item */
	int value = machine().ioport().get_autofire_delay();
	snprintf(temp_text, ARRAY_LENGTH(temp_text), "%d = %.2f Hz", value, (float)refresh/value);
	if (!autofire_toggle)
	{
		item_append(_("Autofire Delay"), temp_text, MENU_FLAG_LEFT_ARROW | MENU_FLAG_RIGHT_ARROW, (void *)ITEMREF_AUTOFIRE_DELAY);
	}
	else
	{
		item_append(_("Autofire Delay"), temp_text, MENU_FLAG_DISABLE | MENU_FLAG_INVERT, nullptr);
	}

	/* add a separator */
	item_append(MENU_SEPARATOR_ITEM, nullptr, 0, nullptr);

	last_toggle = autofire_toggle;
}
