/*********************************************************************

    miscmenu.c

    Internal MAME menus for the user interface.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

*********************************************************************/

#include <ctype.h>

#include "emu.h"
#include "emuopts.h"

#include "cheat.h"
#include "osdnet.h"
#include "rendutil.h"

#include "uiinput.h"
#include "ui/ui.h"
#include "ui/miscmenu.h"
#include "ui/filemngr.h"


/*-------------------------------------------------
    ui_slider_ui_handler - pushes the slider
    menu on the stack and hands off to the
    standard menu handler
-------------------------------------------------*/

UINT32 ui_menu_sliders::ui_handler(running_machine &machine, render_container *container, UINT32 state)
{
	UINT32 result;

	/* if this is the first call, push the sliders menu */
	if (state)
		ui_menu::stack_push(auto_alloc_clear(machine, ui_menu_sliders(machine, container, true)));

	/* handle standard menus */
	result = ui_menu::ui_handler(machine, container, state);

	/* if we are cancelled, pop the sliders menu */
	if (result == UI_HANDLER_CANCEL)
		ui_menu::stack_pop(machine);

	ui_menu_sliders *uim = dynamic_cast<ui_menu_sliders *>(menu_stack);
	return uim && uim->menuless_mode ? 0 : UI_HANDLER_CANCEL;
}


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
	item_append("Keyboard Mode:", natural ? "Natural" : "Emulated", natural ? MENU_FLAG_LEFT_ARROW : MENU_FLAG_RIGHT_ARROW, NULL);
}

ui_menu_keyboard_mode::~ui_menu_keyboard_mode()
{
}

void ui_menu_keyboard_mode::handle()
{
	bool natural = machine().ui().use_natural_keyboard();

	/* process the menu */
	const ui_menu_event *menu_event = process(0);

	if (menu_event != NULL)
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
	for (device_t *device = deviter.first(); device != NULL; device = deviter.next())
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
			item_append(strcmp(device->tag(),":")==0 ? "driver" : device->tag()+1, val, MENU_FLAG_LEFT_ARROW | MENU_FLAG_RIGHT_ARROW, (void *)device);
		}
	}

	item_append(MENU_SEPARATOR_ITEM, NULL, 0, NULL);
	item_append("Reset",  NULL, 0, (void *)1);
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

	if (menu_event != NULL && menu_event->itemref != NULL)
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
			if (strcmp(dev->tag(),":")==0) {
				astring error;
				machine().options().set_value("bios", val-1, OPTION_PRIORITY_CMDLINE, error);
				assert(!error);
			} else {
				astring error;
				astring value;
				astring temp;
				value.printf("%s,bios=%d",machine().options().main_value(temp,dev->owner()->tag()+1),val-1);
				machine().options().set_value(dev->owner()->tag()+1, value.cstr(), OPTION_PRIORITY_CMDLINE, error);
				assert(!error);
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
	for (device_network_interface *network = iter.first(); network != NULL; network = iter.next())
	{
		int curr = network->get_interface();
		const char *title = NULL;
		const netdev_entry_t *entry = netdev_first();
		while(entry) {
			if(entry->id==curr) {
				title = entry->description;
				break;
			}
			entry = entry->m_next;
		}

		item_append(network->device().tag(),  (title) ? title : "------", MENU_FLAG_LEFT_ARROW | MENU_FLAG_RIGHT_ARROW, (void *)network);
	}
}

/*-------------------------------------------------
    ui_menu_network_devices - menu that
-------------------------------------------------*/

void ui_menu_network_devices::handle()
{
	/* process the menu */
	const ui_menu_event *menu_event = process(0);

	if (menu_event != NULL && menu_event->itemref != NULL)
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
	if (prevtime.seconds != curtime.seconds)
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
	int tickets = get_dispensed_tickets(machine());
	astring tempstring;
	int ctrnum;

	/* show total time first */
	if (prevtime.seconds >= 60 * 60)
		tempstring.catprintf("Uptime: %d:%02d:%02d\n\n", prevtime.seconds / (60*60), (prevtime.seconds / 60) % 60, prevtime.seconds % 60);
	else
		tempstring.catprintf("Uptime: %d:%02d\n\n", (prevtime.seconds / 60) % 60, prevtime.seconds % 60);

	/* show tickets at the top */
	if (tickets > 0)
		tempstring.catprintf("Tickets dispensed: %d\n\n", tickets);

	/* loop over coin counters */
	for (ctrnum = 0; ctrnum < COIN_COUNTERS; ctrnum++)
	{
		int count = coin_counter_get_count(machine(), ctrnum);

		/* display the coin counter number */
		tempstring.catprintf("Coin %c: ", ctrnum + 'A');

		/* display how many coins */
		if (count == 0)
			tempstring.cat("NA");
		else
			tempstring.catprintf("%d", count);

		/* display whether or not we are locked out */
		if (coin_lockout_get_state(machine(), ctrnum))
			tempstring.cat(" (locked)");
		tempstring.cat("\n");
	}

	/* append the single item */
	item_append(tempstring, NULL, MENU_FLAG_MULTILINE, NULL);
}


/*-------------------------------------------------
    menu_game_info - handle the game information
    menu
-------------------------------------------------*/

ui_menu_game_info::ui_menu_game_info(running_machine &machine, render_container *container) : ui_menu(machine, container)
{
}

void ui_menu_game_info::populate()
{
	astring tempstring;
	item_append(machine().ui().game_info_astring(tempstring), NULL, MENU_FLAG_MULTILINE, NULL);
}

void ui_menu_game_info::handle()
{
	/* process the menu */
	process(0);
}

ui_menu_game_info::~ui_menu_game_info()
{
}

/*-------------------------------------------------
    menu_cheat - handle the cheat menu
-------------------------------------------------*/

void ui_menu_cheat::handle()
{
	/* process the menu */
	const ui_menu_event *menu_event = process(UI_MENU_PROCESS_LR_REPEAT);

	/* handle events */
	if (menu_event != NULL && menu_event->itemref != NULL)
	{
		bool changed = false;

		/* clear cheat comment on any movement or keypress */
		popmessage(NULL);

		/* handle reset all + reset all cheats for reload all option */
		if ((FPTR)menu_event->itemref < 3 && menu_event->iptkey == IPT_UI_SELECT)
		{
			for (cheat_entry *curcheat = machine().cheat().first(); curcheat != NULL; curcheat = curcheat->next())
				if (curcheat->select_default_state())
					changed = true;
		}


		/* handle individual cheats */
		else if ((FPTR)menu_event->itemref > 2)
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
					if (string != NULL && string[0] != 0)
						popmessage("Cheat Comment:\n%s", string);
					break;
			}
		}

		/* handle reload all  */
		if ((FPTR)menu_event->itemref == 2 && menu_event->iptkey == IPT_UI_SELECT)
		{
			/* re-init cheat engine and thus reload cheats/cheats have already been turned off by here */
			machine().cheat().reload();

			/* display the reloaded cheats */
			reset(UI_MENU_RESET_REMEMBER_REF);
			popmessage("All cheats reloaded");
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
	astring text;
	astring subtext;
	for (cheat_entry *curcheat = machine().cheat().first(); curcheat != NULL; curcheat = curcheat->next())
	{
		UINT32 flags;
		curcheat->menu_text(text, subtext, flags);
		item_append(text, subtext, flags, curcheat);
	}

	/* add a separator */
	item_append(MENU_SEPARATOR_ITEM, NULL, 0, NULL);

	/* add a reset all option */
	item_append("Reset All", NULL, 0, (void *)1);

	/* add a reload all cheats option */
	item_append("Reload All", NULL, 0, (void *)2);
}

ui_menu_cheat::~ui_menu_cheat()
{
}

/*-------------------------------------------------
    menu_sliders - handle the sliders menu
-------------------------------------------------*/

void ui_menu_sliders::handle()
{
	const ui_menu_event *menu_event;

	/* process the menu */
	menu_event = process(UI_MENU_PROCESS_LR_REPEAT | (hidden ? UI_MENU_PROCESS_CUSTOM_ONLY : 0));
	if (menu_event != NULL)
	{
		/* handle keys if there is a valid item selected */
		if (menu_event->itemref != NULL)
		{
			const slider_state *slider = (const slider_state *)menu_event->itemref;
			INT32 curvalue = (*slider->update)(machine(), slider->arg, NULL, SLIDER_NOCHANGE);
			INT32 increment = 0;

			switch (menu_event->iptkey)
			{
				/* toggle visibility */
				case IPT_UI_ON_SCREEN_DISPLAY:
					if (menuless_mode)
						ui_menu::stack_pop(machine());
					else
						hidden = !hidden;
					break;

				/* decrease value */
				case IPT_UI_LEFT:
					if (machine().input().code_pressed(KEYCODE_LALT) || machine().input().code_pressed(KEYCODE_RALT))
						increment = -1;
					else if (machine().input().code_pressed(KEYCODE_LSHIFT) || machine().input().code_pressed(KEYCODE_RSHIFT))
						increment = (slider->incval > 10) ? -(slider->incval / 10) : -1;
					else if (machine().input().code_pressed(KEYCODE_LCONTROL) || machine().input().code_pressed(KEYCODE_RCONTROL))
						increment = -slider->incval * 10;
					else
						increment = -slider->incval;
					break;

				/* increase value */
				case IPT_UI_RIGHT:
					if (machine().input().code_pressed(KEYCODE_LALT) || machine().input().code_pressed(KEYCODE_RALT))
						increment = 1;
					else if (machine().input().code_pressed(KEYCODE_LSHIFT) || machine().input().code_pressed(KEYCODE_RSHIFT))
						increment = (slider->incval > 10) ? (slider->incval / 10) : 1;
					else if (machine().input().code_pressed(KEYCODE_LCONTROL) || machine().input().code_pressed(KEYCODE_RCONTROL))
						increment = slider->incval * 10;
					else
						increment = slider->incval;
					break;

				/* restore default */
				case IPT_UI_SELECT:
					increment = slider->defval - curvalue;
					break;
			}

			/* handle any changes */
			if (increment != 0)
			{
				INT32 newvalue = curvalue + increment;

				/* clamp within bounds */
				if (newvalue < slider->minval)
					newvalue = slider->minval;
				if (newvalue > slider->maxval)
					newvalue = slider->maxval;

				/* update the slider and recompute the menu */
				(*slider->update)(machine(), slider->arg, NULL, newvalue);
				reset(UI_MENU_RESET_REMEMBER_REF);
			}
		}

		/* if we are selecting an invalid item and we are hidden, skip to the next one */
		else if (hidden)
		{
			/* if we got here via up or page up, select the previous item */
			if (menu_event->iptkey == IPT_UI_UP || menu_event->iptkey == IPT_UI_PAGE_UP)
			{
				selected = (selected + numitems - 1) % numitems;
				validate_selection(-1);
			}

			/* otherwise select the next item */
			else if (menu_event->iptkey == IPT_UI_DOWN || menu_event->iptkey == IPT_UI_PAGE_DOWN)
			{
				selected = (selected + 1) % numitems;
				validate_selection(1);
			}
		}
	}
}


/*-------------------------------------------------
    menu_sliders_populate - populate the sliders
    menu
-------------------------------------------------*/

ui_menu_sliders::ui_menu_sliders(running_machine &machine, render_container *container, bool _menuless_mode) : ui_menu(machine, container)
{
	menuless_mode = hidden = _menuless_mode;
}

void ui_menu_sliders::populate()
{
	const slider_state *curslider;
	astring tempstring;

	/* add all sliders */
	for (curslider = machine().ui().get_slider_list(); curslider != NULL; curslider = curslider->next)
	{
		INT32 curval = (*curslider->update)(machine(), curslider->arg, &tempstring, SLIDER_NOCHANGE);
		UINT32 flags = 0;
		if (curval > curslider->minval)
			flags |= MENU_FLAG_LEFT_ARROW;
		if (curval < curslider->maxval)
			flags |= MENU_FLAG_RIGHT_ARROW;
		item_append(curslider->description, tempstring, flags, (void *)curslider);

		if (menuless_mode)
			break;
	}

	/* add all sliders */
	for (curslider = (slider_state*)machine().osd().get_slider_list(); curslider != NULL; curslider = curslider->next)
	{
		INT32 curval = (*curslider->update)(machine(), curslider->arg, &tempstring, SLIDER_NOCHANGE);
		UINT32 flags = 0;
		if (curval > curslider->minval)
			flags |= MENU_FLAG_LEFT_ARROW;
		if (curval < curslider->maxval)
			flags |= MENU_FLAG_RIGHT_ARROW;
		item_append(curslider->description, tempstring, flags, (void *)curslider);
	}

	custombottom = 2.0f * machine().ui().get_line_height() + 2.0f * UI_BOX_TB_BORDER;
}

ui_menu_sliders::~ui_menu_sliders()
{
}

/*-------------------------------------------------
    menu_sliders_custom_render - perform our special
    rendering
-------------------------------------------------*/

void ui_menu_sliders::custom_render(void *selectedref, float top, float bottom, float x1, float y1, float x2, float y2)
{
	const slider_state *curslider = (const slider_state *)selectedref;
	if (curslider != NULL)
	{
		float bar_left, bar_area_top, bar_width, bar_area_height, bar_top, bar_bottom, default_x, current_x;
		float line_height = machine().ui().get_line_height();
		float percentage, default_percentage;
		astring tempstring;
		float text_height;
		INT32 curval;

		/* determine the current value and text */
		curval = (*curslider->update)(machine(), curslider->arg, &tempstring, SLIDER_NOCHANGE);

		/* compute the current and default percentages */
		percentage = (float)(curval - curslider->minval) / (float)(curslider->maxval - curslider->minval);
		default_percentage = (float)(curslider->defval - curslider->minval) / (float)(curslider->maxval - curslider->minval);

		/* assemble the text */
		tempstring.ins(0, " ").ins(0, curslider->description);

		/* move us to the bottom of the screen, and expand to full width */
		y2 = 1.0f - UI_BOX_TB_BORDER;
		y1 = y2 - bottom;
		x1 = UI_BOX_LR_BORDER;
		x2 = 1.0f - UI_BOX_LR_BORDER;

		/* draw extra menu area */
		machine().ui().draw_outlined_box(container, x1, y1, x2, y2, UI_BACKGROUND_COLOR);
		y1 += UI_BOX_TB_BORDER;

		/* determine the text height */
		machine().ui().draw_text_full(container, tempstring, 0, 0, x2 - x1 - 2.0f * UI_BOX_LR_BORDER,
					JUSTIFY_CENTER, WRAP_TRUNCATE, DRAW_NONE, ARGB_WHITE, ARGB_BLACK, NULL, &text_height);

		/* draw the thermometer */
		bar_left = x1 + UI_BOX_LR_BORDER;
		bar_area_top = y1;
		bar_width = x2 - x1 - 2.0f * UI_BOX_LR_BORDER;
		bar_area_height = line_height;

		/* compute positions */
		bar_top = bar_area_top + 0.125f * bar_area_height;
		bar_bottom = bar_area_top + 0.875f * bar_area_height;
		default_x = bar_left + bar_width * default_percentage;
		current_x = bar_left + bar_width * percentage;

		/* fill in the percentage */
		container->add_rect(bar_left, bar_top, current_x, bar_bottom, UI_SLIDER_COLOR, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));

		/* draw the top and bottom lines */
		container->add_line(bar_left, bar_top, bar_left + bar_width, bar_top, UI_LINE_WIDTH, UI_BORDER_COLOR, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
		container->add_line(bar_left, bar_bottom, bar_left + bar_width, bar_bottom, UI_LINE_WIDTH, UI_BORDER_COLOR, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));

		/* draw default marker */
		container->add_line(default_x, bar_area_top, default_x, bar_top, UI_LINE_WIDTH, UI_BORDER_COLOR, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
		container->add_line(default_x, bar_bottom, default_x, bar_area_top + bar_area_height, UI_LINE_WIDTH, UI_BORDER_COLOR, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));

		/* draw the actual text */
		machine().ui().draw_text_full(container, tempstring, x1 + UI_BOX_LR_BORDER, y1 + line_height, x2 - x1 - 2.0f * UI_BOX_LR_BORDER,
					JUSTIFY_CENTER, WRAP_WORD, DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, NULL, &text_height);
	}
}


/*-------------------------------------------------
    menu_video_targets - handle the video targets
    menu
-------------------------------------------------*/

void ui_menu_video_targets::handle()
{
	/* process the menu */
	const ui_menu_event *menu_event = process(0);
	if (menu_event != NULL && menu_event->iptkey == IPT_UI_SELECT)
		ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_video_options(machine(), container, static_cast<render_target *>(menu_event->itemref))));
}


/*-------------------------------------------------
    menu_video_targets_populate - populate the
    video targets menu
-------------------------------------------------*/

ui_menu_video_targets::ui_menu_video_targets(running_machine &machine, render_container *container) : ui_menu(machine, container)
{
}

void ui_menu_video_targets::populate()
{
	int targetnum;

	/* find the targets */
	for (targetnum = 0; ; targetnum++)
	{
		render_target *target = machine().render().target_by_index(targetnum);
		char buffer[40];

		/* stop when we run out */
		if (target == NULL)
			break;

		/* add a menu item */
		sprintf(buffer, "Screen #%d", targetnum);
		item_append(buffer, NULL, 0, target);
	}
}

ui_menu_video_targets::~ui_menu_video_targets()
{
}

/*-------------------------------------------------
    menu_video_options - handle the video options
    menu
-------------------------------------------------*/

void ui_menu_video_options::handle()
{
	bool changed = false;

	/* process the menu */
	const ui_menu_event *menu_event = process(0);
	if (menu_event != NULL && menu_event->itemref != NULL)
	{
		switch ((FPTR)menu_event->itemref)
		{
			/* rotate adds rotation depending on the direction */
			case VIDEO_ITEM_ROTATE:
				if (menu_event->iptkey == IPT_UI_LEFT || menu_event->iptkey == IPT_UI_RIGHT)
				{
					int delta = (menu_event->iptkey == IPT_UI_LEFT) ? ROT270 : ROT90;
					target->set_orientation(orientation_add(delta, target->orientation()));
					if (target->is_ui_target())
					{
						render_container::user_settings settings;
						container->get_user_settings(settings);
						settings.m_orientation = orientation_add(delta ^ ROT180, settings.m_orientation);
						container->set_user_settings(settings);
					}
					changed = true;
				}
				break;

			/* layer config bitmasks handle left/right keys the same (toggle) */
			case VIDEO_ITEM_BACKDROPS:
				if (menu_event->iptkey == IPT_UI_LEFT || menu_event->iptkey == IPT_UI_RIGHT)
				{
					target->set_backdrops_enabled(!target->backdrops_enabled());
					changed = true;
				}
				break;

			case VIDEO_ITEM_OVERLAYS:
				if (menu_event->iptkey == IPT_UI_LEFT || menu_event->iptkey == IPT_UI_RIGHT)
				{
					target->set_overlays_enabled(!target->overlays_enabled());
					changed = true;
				}
				break;

			case VIDEO_ITEM_BEZELS:
				if (menu_event->iptkey == IPT_UI_LEFT || menu_event->iptkey == IPT_UI_RIGHT)
				{
					target->set_bezels_enabled(!target->bezels_enabled());
					changed = true;
				}
				break;

			case VIDEO_ITEM_CPANELS:
				if (menu_event->iptkey == IPT_UI_LEFT || menu_event->iptkey == IPT_UI_RIGHT)
				{
					target->set_cpanels_enabled(!target->cpanels_enabled());
					changed = true;
				}
				break;

			case VIDEO_ITEM_MARQUEES:
				if (menu_event->iptkey == IPT_UI_LEFT || menu_event->iptkey == IPT_UI_RIGHT)
				{
					target->set_marquees_enabled(!target->marquees_enabled());
					changed = true;
				}
				break;

			case VIDEO_ITEM_ZOOM:
				if (menu_event->iptkey == IPT_UI_LEFT || menu_event->iptkey == IPT_UI_RIGHT)
				{
					target->set_zoom_to_screen(!target->zoom_to_screen());
					changed = true;
				}
				break;

			/* anything else is a view item */
			default:
				if (menu_event->iptkey == IPT_UI_SELECT && (int)(FPTR)menu_event->itemref >= VIDEO_ITEM_VIEW)
				{
					target->set_view((FPTR)menu_event->itemref - VIDEO_ITEM_VIEW);
					changed = true;
				}
				break;
		}
	}

	/* if something changed, rebuild the menu */
	if (changed)
		reset(UI_MENU_RESET_REMEMBER_REF);
}


/*-------------------------------------------------
    menu_video_options_populate - populate the
    video options menu
-------------------------------------------------*/

ui_menu_video_options::ui_menu_video_options(running_machine &machine, render_container *container, render_target *_target) : ui_menu(machine, container)
{
	target = _target;
}

void ui_menu_video_options::populate()
{
	const char *subtext = "";
	astring tempstring;
	int viewnum;
	int enabled;

	/* add items for each view */
	for (viewnum = 0; ; viewnum++)
	{
		const char *name = target->view_name(viewnum);
		if (name == NULL)
			break;

		/* create a string for the item, replacing underscores with spaces */
		tempstring.cpy(name).replace(0, "_", " ");
		item_append(tempstring, NULL, 0, (void *)(FPTR)(VIDEO_ITEM_VIEW + viewnum));
	}

	/* add a separator */
	item_append(MENU_SEPARATOR_ITEM, NULL, 0, NULL);

	/* add a rotate item */
	switch (target->orientation())
	{
		case ROT0:      subtext = "None";                   break;
		case ROT90:     subtext = "CW 90" UTF8_DEGREES;     break;
		case ROT180:    subtext = "180" UTF8_DEGREES;       break;
		case ROT270:    subtext = "CCW 90" UTF8_DEGREES;    break;
	}
	item_append("Rotate", subtext, MENU_FLAG_LEFT_ARROW | MENU_FLAG_RIGHT_ARROW, (void *)VIDEO_ITEM_ROTATE);

	/* backdrop item */
	enabled = target->backdrops_enabled();
	item_append("Backdrops", enabled ? "Enabled" : "Disabled", enabled ? MENU_FLAG_LEFT_ARROW : MENU_FLAG_RIGHT_ARROW, (void *)VIDEO_ITEM_BACKDROPS);

	/* overlay item */
	enabled = target->overlays_enabled();
	item_append("Overlays", enabled ? "Enabled" : "Disabled", enabled ? MENU_FLAG_LEFT_ARROW : MENU_FLAG_RIGHT_ARROW, (void *)VIDEO_ITEM_OVERLAYS);

	/* bezel item */
	enabled = target->bezels_enabled();
	item_append("Bezels", enabled ? "Enabled" : "Disabled", enabled ? MENU_FLAG_LEFT_ARROW : MENU_FLAG_RIGHT_ARROW, (void *)VIDEO_ITEM_BEZELS);

	/* cpanel item */
	enabled = target->cpanels_enabled();
	item_append("CPanels", enabled ? "Enabled" : "Disabled", enabled ? MENU_FLAG_LEFT_ARROW : MENU_FLAG_RIGHT_ARROW, (void *)VIDEO_ITEM_CPANELS);

	/* marquee item */
	enabled = target->marquees_enabled();
	item_append("Marquees", enabled ? "Enabled" : "Disabled", enabled ? MENU_FLAG_LEFT_ARROW : MENU_FLAG_RIGHT_ARROW, (void *)VIDEO_ITEM_MARQUEES);

	/* cropping */
	enabled = target->zoom_to_screen();
	item_append("View", enabled ? "Cropped" : "Full", enabled ? MENU_FLAG_RIGHT_ARROW : MENU_FLAG_LEFT_ARROW, (void *)VIDEO_ITEM_ZOOM);
}

ui_menu_video_options::~ui_menu_video_options()
{
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
	if (menu_event != NULL && menu_event->itemref != NULL)
	{
		crosshair_user_settings settings;
		crosshair_item_data *data = (crosshair_item_data *)menu_event->itemref;
		bool changed = false;
		//int set_def = false;
		int newval = data->cur;

		/* retreive the user settings */
		crosshair_get_user_settings(machine(), data->player, &settings);

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
			crosshair_set_user_settings(machine(), data->player, &settings);

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
		crosshair_get_user_settings(machine(), player, &settings);

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
			while (((dir = path.next()) != NULL) && !finished)
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
		crosshair_get_user_settings(machine(), 0, &settings);

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
