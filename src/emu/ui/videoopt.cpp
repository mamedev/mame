// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Nathan Woods
/*********************************************************************

    ui/videoopt.c

    Internal menus for video options

*********************************************************************/

#include "emu.h"
#include "rendutil.h"

#include "ui/menu.h"
#include "ui/videoopt.h"

/*-------------------------------------------------
    menu_video_targets - handle the video targets
    menu
-------------------------------------------------*/

void ui_menu_video_targets::handle()
{
	/* process the menu */
	const ui_menu_event *menu_event = process(0);
	if (menu_event != nullptr && menu_event->iptkey == IPT_UI_SELECT)
		ui_menu::stack_push(global_alloc_clear<ui_menu_video_options>(machine(), container, static_cast<render_target *>(menu_event->itemref)));
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
		if (target == nullptr)
			break;

		/* add a menu item */
		sprintf(buffer, "Screen #%d", targetnum);
		item_append(buffer, nullptr, 0, target);
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
	if (menu_event != nullptr && menu_event->itemref != nullptr)
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
	std::string tempstring;
	int viewnum;
	int enabled;

	/* add items for each view */
	for (viewnum = 0; ; viewnum++)
	{
		const char *name = target->view_name(viewnum);
		if (name == nullptr)
			break;

		/* create a string for the item, replacing underscores with spaces */
		tempstring.assign(name);
		strreplace(tempstring, "_", " ");
		item_append(tempstring.c_str(), nullptr, 0, (void *)(FPTR)(VIDEO_ITEM_VIEW + viewnum));
	}

	/* add a separator */
	item_append(MENU_SEPARATOR_ITEM, nullptr, 0, nullptr);

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
