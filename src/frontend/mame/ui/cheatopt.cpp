// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Nathan Woods
/*********************************************************************

    ui/cheatopt.cpp

    Internal menu for the cheat interface.

*********************************************************************/

#include "emu.h"
#include "cheat.h"
#include "mame.h"

#include "ui/ui.h"
#include "ui/menu.h"
#include "ui/cheatopt.h"


namespace ui {
// itemrefs for key menu items
#define ITEMREF_CHEATS_RESET_ALL            ((void *) 0x0001)
#define ITEMREF_CHEATS_RELOAD_ALL           ((void *) 0x0002)
#define ITEMREF_CHEATS_AUTOFIRE_SETTINGS    ((void *) 0x0003)
#define ITEMREF_CHEATS_FIRST_ITEM           ((void *) 0x0004)

// itemrefs for key menu items
#define ITEMREF_AUTOFIRE_STATUS       ((void *) 0x0001)
#define ITEMREF_AUTOFIRE_DELAY        ((void *) 0x0002)
#define ITEMREF_AUTOFIRE_FIRST_BUTTON ((void *) 0x0003)


/*-------------------------------------------------
    menu_cheat - handle the cheat menu
-------------------------------------------------*/

void menu_cheat::handle()
{
	/* process the menu */
	const event *menu_event = process(PROCESS_LR_REPEAT);


	/* handle events */
	if (menu_event != nullptr && menu_event->itemref != nullptr)
	{
		bool changed = false;

		/* clear cheat comment on any movement or keypress */
		machine().popmessage();

		/* handle reset all + reset all cheats for reload all option */
		if ((menu_event->itemref == ITEMREF_CHEATS_RESET_ALL || menu_event->itemref == ITEMREF_CHEATS_RELOAD_ALL) && menu_event->iptkey == IPT_UI_SELECT)
		{
			for (auto &curcheat : mame_machine_manager::instance()->cheat().entries())
				if (curcheat->select_default_state())
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
			mame_machine_manager::instance()->cheat().reload();

			/* display the reloaded cheats */
			reset(reset_options::REMEMBER_REF);
			machine().popmessage(_("All cheats reloaded"));
		}

		/* handle autofire menu */
		if (menu_event->itemref == ITEMREF_CHEATS_AUTOFIRE_SETTINGS && menu_event->iptkey == IPT_UI_SELECT)
		{
			menu::stack_push<menu_autofire>(ui(), container());
		}

		/* if things changed, update */
		if (changed)
			reset(reset_options::REMEMBER_REF);
	}
}


/*-------------------------------------------------
    menu_cheat_populate - populate the cheat menu
-------------------------------------------------*/

menu_cheat::menu_cheat(mame_ui_manager &mui, render_container &container) : menu(mui, container)
{
}

void menu_cheat::populate()
{
	/* iterate over cheats */
	std::string text;
	std::string subtext;

	// add the autofire menu
	item_append(_("Autofire Settings"), "", 0, (void *)ITEMREF_CHEATS_AUTOFIRE_SETTINGS);

	/* add a separator */
	item_append(menu_item_type::SEPARATOR);

	// add other cheats
	if (!mame_machine_manager::instance()->cheat().entries().empty()) {
		for (auto &curcheat : mame_machine_manager::instance()->cheat().entries())
		{
			UINT32 flags;
			curcheat->menu_text(text, subtext, flags);
			if (text == MENU_SEPARATOR_ITEM)
				item_append(menu_item_type::SEPARATOR, flags);
			else
				item_append(text, subtext, flags, curcheat.get());
		}

		/* add a separator */
		item_append(menu_item_type::SEPARATOR);

		/* add a reset all option */
		item_append(_("Reset All"), "", 0, (void *)ITEMREF_CHEATS_RESET_ALL);

		/* add a reload all cheats option */
		item_append(_("Reload All"), "", 0, (void *)ITEMREF_CHEATS_RELOAD_ALL);
	}
}

menu_cheat::~menu_cheat()
{
}





/*-------------------------------------------------
    menu_autofire - handle the autofire settings
    menu
-------------------------------------------------*/

menu_autofire::menu_autofire(mame_ui_manager &mui, render_container &container) : menu(mui, container), last_toggle(false)
{
	const screen_device *screen = mui.machine().first_screen();

	if (screen == nullptr)
	{
		refresh = 60.0;
	}
	else
	{
		refresh = ATTOSECONDS_TO_HZ(screen->refresh_attoseconds());
	}
}

menu_autofire::~menu_autofire()
{
}

void menu_autofire::handle()
{
	ioport_field *field;
	bool changed = false;

	/* process the menu */
	const event *menu_event = process(0);

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
		reset(reset_options::REMEMBER_REF);
	}
}


/*-------------------------------------------------
    menu_autofire_populate - populate the autofire
    menu
-------------------------------------------------*/

void menu_autofire::populate()
{
	char temp_text[64];

	/* add autofire toggle item */
	bool autofire_toggle = machine().ioport().get_autofire_toggle();
	item_append(_("Autofire Status"), (autofire_toggle ? _("Disabled") : _("Enabled")),
			(autofire_toggle ? FLAG_RIGHT_ARROW : FLAG_LEFT_ARROW), (void *)ITEMREF_AUTOFIRE_STATUS);

	/* iterate over the input ports and add autofire toggle items */
	int menu_items = 0;
	for (auto &port : machine().ioport().ports())
	{
		bool is_first_button = true;
		for (ioport_field &field : port.second->fields())
		{
			if (field.type() >= IPT_BUTTON1 && field.type() <= IPT_BUTTON16)
			{
				menu_items++;
				ioport_field::user_settings settings;
				field.get_user_settings(settings);

				if (is_first_button)
				{
					/* add a separator for each player */
					item_append(menu_item_type::SEPARATOR);
					is_first_button = false;
				}
				/* add an autofire item */
				if (!autofire_toggle)
				{
					// item is enabled and can be switched to values on/off
					item_append(field.name(), (settings.autofire ? _("On") : _("Off")),
							(settings.autofire ? FLAG_LEFT_ARROW : FLAG_RIGHT_ARROW), (void *)&field);
				}
				else
				{
					// item is disabled
					item_append(field.name(), (settings.autofire ? _("On") : _("Off")),
							FLAG_DISABLE | FLAG_INVERT, nullptr);
				}
			}
		}
	}

	/* add text item if no buttons found */
	if (menu_items==0)
	{
		item_append(menu_item_type::SEPARATOR);
		item_append(_("No buttons found on this machine!"), "", FLAG_DISABLE, nullptr);
	}

	/* add a separator */
	item_append(menu_item_type::SEPARATOR);

	/* add autofire delay item */
	int value = machine().ioport().get_autofire_delay();
	snprintf(temp_text, ARRAY_LENGTH(temp_text), "%d = %.2f Hz", value, (float)refresh/value);
	if (!autofire_toggle)
	{
		item_append(_("Autofire Delay"), temp_text, FLAG_LEFT_ARROW | FLAG_RIGHT_ARROW, (void *)ITEMREF_AUTOFIRE_DELAY);
	}
	else
	{
		item_append(_("Autofire Delay"), temp_text, FLAG_DISABLE | FLAG_INVERT, nullptr);
	}

	/* add a separator */
	item_append(menu_item_type::SEPARATOR);

	last_toggle = autofire_toggle;
}

} // namespace ui
