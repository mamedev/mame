// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Nathan Woods
/*********************************************************************

    ui/cheatopt.cpp

    Internal menu for the cheat interface.

*********************************************************************/

#include "emu.h"
#include "ui/cheatopt.h"

#include "ui/ui.h"

#include "cheat.h"
#include "mame.h"


namespace ui {

// itemrefs for key menu items
#define ITEMREF_CHEATS_ENABLE               ((void *) 0x0001)
#define ITEMREF_CHEATS_RESET_ALL            ((void *) 0x0002)
#define ITEMREF_CHEATS_RELOAD_ALL           ((void *) 0x0003)
#define ITEMREF_CHEATS_FIRST_ITEM           ((void *) 0x0004)


/*-------------------------------------------------
    menu_cheat - handle the cheat menu
-------------------------------------------------*/

bool menu_cheat::handle(event const *ev)
{
	if (!ev || !ev->itemref)
		return false;

	bool changed = false;

	// clear cheat comment on any movement or keypress
	machine().popmessage();

	if (ev->itemref == ITEMREF_CHEATS_ENABLE)
	{
		if ((ev->iptkey == IPT_UI_LEFT) || (ev->iptkey == IPT_UI_RIGHT) || (ev->iptkey == IPT_UI_CLEAR))
		{
			// handle global enable toggle
			mame_machine_manager::instance()->cheat().set_enable(ev->iptkey == IPT_UI_RIGHT || (ev->iptkey == IPT_UI_CLEAR), false);
			changed = true;
		}
	}
	if ((ev->itemref == ITEMREF_CHEATS_RESET_ALL || ev->itemref == ITEMREF_CHEATS_RELOAD_ALL) && ev->iptkey == IPT_UI_SELECT)
	{
		// handle reset all + reset all cheats for reload all option
		for (auto &curcheat : mame_machine_manager::instance()->cheat().entries())
			if (curcheat->select_default_state())
				changed = true;
	}
	else if (ev->itemref >= ITEMREF_CHEATS_FIRST_ITEM)
	{
		// handle individual cheats
		cheat_entry *curcheat = reinterpret_cast<cheat_entry *>(ev->itemref);
		const char *string;
		switch (ev->iptkey)
		{
			// if selected, activate a oneshot
			case IPT_UI_SELECT:
				changed = curcheat->activate();
				break;

			// if cleared, reset to default value
			case IPT_UI_CLEAR:
				changed = curcheat->select_default_state();
				break;

			// left decrements
			case IPT_UI_LEFT:
				changed = curcheat->select_previous_state();
				break;

			// right increments
			case IPT_UI_RIGHT:
				changed = curcheat->select_next_state();
				break;

			// bring up display comment if one exists
			case IPT_UI_DISPLAY_COMMENT:
			case IPT_UI_UP:
			case IPT_UI_DOWN:
				string = curcheat->comment();
				if (string && *string)
					machine().popmessage(_("Cheat Comment:\n%s"), string);
				break;
		}
	}

	// handle reload all
	if (ev->itemref == ITEMREF_CHEATS_RELOAD_ALL && ev->iptkey == IPT_UI_SELECT)
	{
		// re-init cheat engine and thus reload cheats/cheats have already been turned off by here
		mame_machine_manager::instance()->cheat().reload();

		// display the reloaded cheats
		machine().popmessage(_("All cheats reloaded"));
		changed = true;
	}

	// if things changed, update
	if (changed)
		reset(reset_options::REMEMBER_REF);

	return false; // always triggers an item reset if the menu needs to be redrawn
}


/*-------------------------------------------------
    menu_cheat_populate - populate the cheat menu
-------------------------------------------------*/

menu_cheat::menu_cheat(mame_ui_manager &mui, render_container &container) : menu(mui, container)
{
	set_heading(_("Cheat Options"));
	set_process_flags(PROCESS_LR_REPEAT);
}

void menu_cheat::menu_activated()
{
	reset(reset_options::REMEMBER_REF);
}

void menu_cheat::populate()
{
	const bool empty = mame_machine_manager::instance()->cheat().entries().empty();

	// iterate over cheats
	if (!empty)
	{
		std::string text;
		std::string subtext;

		for (auto &curcheat : mame_machine_manager::instance()->cheat().entries())
		{
			uint32_t flags;
			curcheat->menu_text(text, subtext, flags);
			if (text == MENU_SEPARATOR_ITEM)
				item_append(menu_item_type::SEPARATOR, flags);
			else
				item_append(text, subtext, flags, curcheat.get());
		}
	}
	else
	{
		// indicate that none were found
		item_append(_("[no cheats found]"), FLAG_DISABLE, nullptr);
	}

	item_append(menu_item_type::SEPARATOR);

	if (!empty)
	{
		// add global enable toggle
		item_append_on_off(_("Enable Cheats"), mame_machine_manager::instance()->cheat().enabled(), 0, (void *)ITEMREF_CHEATS_ENABLE);
		item_append(menu_item_type::SEPARATOR);

		// add a reset all option
		item_append(_("Reset All"), 0, (void *)ITEMREF_CHEATS_RESET_ALL);
	}

	// add a reload all cheats option
	item_append(_("Reload All"), 0, (void *)ITEMREF_CHEATS_RELOAD_ALL);
}

menu_cheat::~menu_cheat()
{
}

} // namespace ui
