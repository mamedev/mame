// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Nathan Woods
/*********************************************************************

    ui/videoopt.cpp

    Internal menus for video options

*********************************************************************/

#include "emu.h"

#include "ui/videoopt.h"

#include "rendutil.h"

namespace ui {
/*-------------------------------------------------
    menu_video_targets - handle the video targets
    menu
-------------------------------------------------*/

void menu_video_targets::handle()
{
	/* process the menu */
	const event *menu_event = process(0);
	if (menu_event != nullptr && menu_event->iptkey == IPT_UI_SELECT)
		menu::stack_push<menu_video_options>(ui(), container(), static_cast<render_target *>(menu_event->itemref));
}


/*-------------------------------------------------
    menu_video_targets_populate - populate the
    video targets menu
-------------------------------------------------*/

menu_video_targets::menu_video_targets(mame_ui_manager &mui, render_container &container) : menu(mui, container)
{
}

void menu_video_targets::populate(float &customtop, float &custombottom)
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
		sprintf(buffer, _("Screen #%d"), targetnum);
		item_append(buffer, "", 0, target);
	}
}

menu_video_targets::~menu_video_targets()
{
}

/*-------------------------------------------------
    menu_video_options - handle the video options
    menu
-------------------------------------------------*/

void menu_video_options::handle()
{
	bool changed = false;

	// process the menu
	const event *menu_event = process(0);
	if (menu_event != nullptr && menu_event->itemref != nullptr)
	{
		switch ((uintptr_t)menu_event->itemref)
		{
		// rotate adds rotation depending on the direction
		case VIDEO_ITEM_ROTATE:
			if (menu_event->iptkey == IPT_UI_LEFT || menu_event->iptkey == IPT_UI_RIGHT)
			{
				int delta = (menu_event->iptkey == IPT_UI_LEFT) ? ROT270 : ROT90;
				target->set_orientation(orientation_add(delta, target->orientation()));
				if (target->is_ui_target())
				{
					render_container::user_settings settings;
					container().get_user_settings(settings);
					settings.m_orientation = orientation_add(delta ^ ROT180, settings.m_orientation);
					container().set_user_settings(settings);
				}
				changed = true;
			}
			break;

		// layer config bitmasks handle left/right keys the same (toggle)
		case VIDEO_ITEM_ZOOM:
			if (menu_event->iptkey == IPT_UI_LEFT || menu_event->iptkey == IPT_UI_RIGHT)
			{
				target->set_zoom_to_screen(!target->zoom_to_screen());
				changed = true;
			}
			break;

		// anything else is a view item
		default:
			if (menu_event->iptkey == IPT_UI_SELECT && (int)(uintptr_t)menu_event->itemref >= VIDEO_ITEM_VIEW)
			{
				target->set_view((uintptr_t)menu_event->itemref - VIDEO_ITEM_VIEW);
				changed = true;
			}
			break;
		}
	}

	/* if something changed, rebuild the menu */
	if (changed)
		reset(reset_options::REMEMBER_REF);
}


/*-------------------------------------------------
    menu_video_options_populate - populate the
    video options menu
-------------------------------------------------*/

menu_video_options::menu_video_options(mame_ui_manager &mui, render_container &container, render_target *_target) : menu(mui, container)
{
	target = _target;
}

void menu_video_options::populate(float &customtop, float &custombottom)
{
	const char *subtext = "";
	std::string tempstring;
	int enabled;

	// add items for each view
	for (int viewnum = 0; ; viewnum++)
	{
		const char *name = target->view_name(viewnum);
		if (name == nullptr)
			break;

		// create a string for the item, replacing underscores with spaces
		tempstring.assign(name);
		strreplace(tempstring, "_", " ");
		item_append(tempstring, "", 0, (void *)(uintptr_t)(VIDEO_ITEM_VIEW + viewnum));
	}

	// add a separator
	item_append(menu_item_type::SEPARATOR);

	// add a rotate item
	switch (target->orientation())
	{
	case ROT0:      subtext = "None";                   break;
	case ROT90:     subtext = "CW 90" UTF8_DEGREES;     break;
	case ROT180:    subtext = "180" UTF8_DEGREES;       break;
	case ROT270:    subtext = "CCW 90" UTF8_DEGREES;    break;
	}
	item_append(_("Rotate"), subtext, FLAG_LEFT_ARROW | FLAG_RIGHT_ARROW, (void *)VIDEO_ITEM_ROTATE);

	// cropping
	enabled = target->zoom_to_screen();
	item_append(_("View"), enabled ? _("Cropped") : _("Full"), enabled ? FLAG_RIGHT_ARROW : FLAG_LEFT_ARROW, (void *)VIDEO_ITEM_ZOOM);
}

menu_video_options::~menu_video_options()
{
}

} // namespace ui
