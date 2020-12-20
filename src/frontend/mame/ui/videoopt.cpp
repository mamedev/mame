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

namespace {

constexpr uintptr_t ITEM_ROTATE         = 0x00000100;
constexpr uintptr_t ITEM_ZOOM           = 0x00000101;
constexpr uintptr_t ITEM_UNEVENSTRETCH  = 0x00000102;
constexpr uintptr_t ITEM_KEEPASPECT     = 0x00000103;
constexpr uintptr_t ITEM_TOGGLE_FIRST   = 0x00000200;
constexpr uintptr_t ITEM_VIEW_FIRST     = 0x00000300;

} // anonymous namespace


/*-------------------------------------------------
    menu_video_targets_populate - populate the
    video targets menu
-------------------------------------------------*/

menu_video_targets::menu_video_targets(mame_ui_manager &mui, render_container &container)
	: menu(mui, container)
{
}

menu_video_targets::~menu_video_targets()
{
}

void menu_video_targets::populate(float &customtop, float &custombottom)
{
	// find the targets
	for (unsigned targetnum = 0; ; targetnum++)
	{
		// stop when we run out
		render_target *const target = machine().render().target_by_index(targetnum);
		if (!target)
			break;

		// add a menu item
		item_append(util::string_format(_("Screen #%d"), targetnum), 0, target);
	}

	// add option for snapshot target
	item_append("Snapshot", 0, &machine().video().snapshot_target());
}

/*-------------------------------------------------
    menu_video_targets - handle the video targets
    menu
-------------------------------------------------*/

void menu_video_targets::handle()
{
	event const *const menu_event = process(0);
	if (menu_event && (menu_event->iptkey == IPT_UI_SELECT))
	{
		render_target *const target = reinterpret_cast<render_target *>(menu_event->itemref);
		menu::stack_push<menu_video_options>(
				ui(),
				container(),
				std::string(selected_item().text),
				*target,
				&machine().video().snapshot_target() == target);
	}
}


/*-------------------------------------------------
    menu_video_options_populate - populate the
    video options menu
-------------------------------------------------*/

menu_video_options::menu_video_options(
		mame_ui_manager &mui,
		render_container &container,
		render_target &target,
		bool snapshot)
	: menu(mui, container)
	, m_target(target)
	, m_title()
	, m_show_title(false)
	, m_snapshot(snapshot)
{
	if (!m_snapshot || !machine().video().snap_native())
	{
		set_selected_index(target.view());
		reset(reset_options::REMEMBER_POSITION);
	}
}

menu_video_options::menu_video_options(
		mame_ui_manager &mui,
		render_container &container,
		std::string &&title,
		render_target &target,
		bool snapshot)
	: menu(mui, container)
	, m_target(target)
	, m_title(std::move(title))
	, m_show_title(true)
	, m_snapshot(snapshot)
{
	if (!m_snapshot || !machine().video().snap_native())
	{
		set_selected_index(target.view() + 2);
		reset(reset_options::REMEMBER_POSITION);
	}
}

menu_video_options::~menu_video_options()
{
}

void menu_video_options::populate(float &customtop, float &custombottom)
{
	uintptr_t ref;

	// add title if requested
	if (m_show_title)
	{
		item_append(m_title, FLAG_DISABLE, nullptr);
		item_append(menu_item_type::SEPARATOR);
	}

	// add items for each view
	if (!m_snapshot || !machine().video().snap_native())
	{
		for (char const *name = m_target.view_name(ref = 0); name; name = m_target.view_name(++ref))
			item_append(name, 0, reinterpret_cast<void *>(ITEM_VIEW_FIRST + ref));
		item_append(menu_item_type::SEPARATOR);
	}

	// add items for visibility toggles
	auto const &toggles = m_target.visibility_toggles();
	if (!toggles.empty())
	{
		ref = 0U;
		auto const current_mask(m_target.visibility_mask());
		for (auto toggle = toggles.begin(); toggles.end() != toggle; ++toggle, ++ref)
		{
			auto const toggle_mask(toggle->mask());
			bool const enabled(BIT(current_mask, ref));
			bool eclipsed(false);
			for (auto it = toggles.begin(); !eclipsed && (toggle != it); ++it)
				eclipsed = ((current_mask & it->mask()) != it->mask()) && ((toggle_mask & it->mask()) == it->mask());
			item_append_on_off(toggle->name(), enabled, eclipsed ? (FLAG_INVERT | FLAG_DISABLE) : 0U, reinterpret_cast<void *>(ITEM_TOGGLE_FIRST + ref));
		}
		item_append(menu_item_type::SEPARATOR);
	}

	const char *subtext = "";

	// add a rotate item
	switch (m_target.orientation())
	{
	case ROT0:      subtext = "None";                   break;
	case ROT90:     subtext = "CW 90" UTF8_DEGREES;     break;
	case ROT180:    subtext = "180" UTF8_DEGREES;       break;
	case ROT270:    subtext = "CCW 90" UTF8_DEGREES;    break;
	}
	item_append(_("Rotate"), subtext, FLAG_LEFT_ARROW | FLAG_RIGHT_ARROW, reinterpret_cast<void *>(ITEM_ROTATE));

	// cropping
	bool const canzoom(m_target.current_view().has_art() && !m_target.current_view().visible_screens().empty());
	item_append_on_off(_("Zoom to Screen Area"), m_target.zoom_to_screen(), canzoom ? 0U : (FLAG_INVERT | FLAG_DISABLE), reinterpret_cast<void *>(ITEM_ZOOM));

	if (!m_snapshot)
	{
		// uneven stretch
		switch (m_target.scale_mode())
		{
		case SCALE_FRACTIONAL:
			subtext = _("On");
			break;

		case SCALE_FRACTIONAL_X:
			subtext = _("X Only");
			break;

		case SCALE_FRACTIONAL_Y:
			subtext = _("Y Only");
			break;

		case SCALE_FRACTIONAL_AUTO:
			subtext = _("X or Y (Auto)");
			break;

		case SCALE_INTEGER:
			subtext = _("Off");
			break;
		}
		item_append(_("Non-Integer Scaling"), subtext, FLAG_LEFT_ARROW | FLAG_RIGHT_ARROW, reinterpret_cast<void *>(ITEM_UNEVENSTRETCH));

		// keep aspect
		item_append_on_off(_("Maintain Aspect Ratio"), m_target.keepaspect(), 0, reinterpret_cast<void *>(ITEM_KEEPASPECT));
	}
}


/*-------------------------------------------------
    menu_video_options - handle the video options
    menu
-------------------------------------------------*/

void menu_video_options::handle()
{
	auto const lockout_popup([this] () { machine().popmessage(_("Cannot change options while recording!")); });
	bool const snap_lockout(m_snapshot && machine().video().is_recording());
	bool changed(false);

	// process the menu
	event const *const menu_event(process(0));
	if (menu_event && uintptr_t(menu_event->itemref))
	{
		switch (reinterpret_cast<uintptr_t>(menu_event->itemref))
		{
		// rotate adds rotation depending on the direction
		case ITEM_ROTATE:
			if (menu_event->iptkey == IPT_UI_LEFT || menu_event->iptkey == IPT_UI_RIGHT)
			{
				if (snap_lockout)
					return lockout_popup();
				int const delta((menu_event->iptkey == IPT_UI_LEFT) ? ROT270 : ROT90);
				m_target.set_orientation(orientation_add(delta, m_target.orientation()));
				if (m_target.is_ui_target())
				{
					render_container::user_settings settings = container().get_user_settings();
					settings.m_orientation = orientation_add(delta ^ ROT180, settings.m_orientation);
					container().set_user_settings(settings);
				}
				changed = true;
			}
			break;

		// layer config bitmasks handle left/right keys the same (toggle)
		case ITEM_ZOOM:
			if ((menu_event->iptkey == IPT_UI_LEFT) || (menu_event->iptkey == IPT_UI_RIGHT))
			{
				if (snap_lockout)
					return lockout_popup();
				m_target.set_zoom_to_screen(menu_event->iptkey == IPT_UI_RIGHT);
				changed = true;
			}
			break;

		// non-integer scaling: rotate through options
		case ITEM_UNEVENSTRETCH:
			if (menu_event->iptkey == IPT_UI_LEFT)
			{
				if (snap_lockout)
					return lockout_popup();
				switch (m_target.scale_mode())
				{
				case SCALE_FRACTIONAL:
					m_target.set_scale_mode(SCALE_INTEGER);
					break;

				case SCALE_FRACTIONAL_X:
					m_target.set_scale_mode(SCALE_FRACTIONAL);
					break;

				case SCALE_FRACTIONAL_Y:
					m_target.set_scale_mode(SCALE_FRACTIONAL_X);
					break;

				case SCALE_FRACTIONAL_AUTO:
					m_target.set_scale_mode(SCALE_FRACTIONAL_Y);
					break;

				case SCALE_INTEGER:
					m_target.set_scale_mode(SCALE_FRACTIONAL_AUTO);
					break;
				}
				changed = true;
			}
			else if (menu_event->iptkey == IPT_UI_RIGHT)
			{
				if (snap_lockout)
					return lockout_popup();
				switch (m_target.scale_mode())
				{
				case SCALE_FRACTIONAL:
					m_target.set_scale_mode(SCALE_FRACTIONAL_X);
					break;

				case SCALE_FRACTIONAL_X:
					m_target.set_scale_mode(SCALE_FRACTIONAL_Y);
					break;

				case SCALE_FRACTIONAL_Y:
					m_target.set_scale_mode(SCALE_FRACTIONAL_AUTO);
					break;

				case SCALE_FRACTIONAL_AUTO:
					m_target.set_scale_mode(SCALE_INTEGER);
					break;

				case SCALE_INTEGER:
					m_target.set_scale_mode(SCALE_FRACTIONAL);
					break;
				}
				changed = true;
			}
			break;

		// keep aspect handles left/right keys the same (toggle)
		case ITEM_KEEPASPECT:
			if ((menu_event->iptkey == IPT_UI_LEFT) || (menu_event->iptkey == IPT_UI_RIGHT))
			{
				if (snap_lockout)
					return lockout_popup();
				m_target.set_keepaspect(menu_event->iptkey == IPT_UI_RIGHT);
				changed = true;
			}
			break;

		// anything else is a view item
		default:
			if (reinterpret_cast<uintptr_t>(menu_event->itemref) >= ITEM_VIEW_FIRST)
			{
				if (snap_lockout)
					return lockout_popup();
				if (menu_event->iptkey == IPT_UI_SELECT)
				{
					m_target.set_view(reinterpret_cast<uintptr_t>(menu_event->itemref) - ITEM_VIEW_FIRST);
					changed = true;
				}
			}
			else if (reinterpret_cast<uintptr_t>(menu_event->itemref) >= ITEM_TOGGLE_FIRST)
			{
				if (snap_lockout)
					return lockout_popup();
				if ((menu_event->iptkey == IPT_UI_LEFT) || (menu_event->iptkey == IPT_UI_RIGHT))
				{
					m_target.set_visibility_toggle(reinterpret_cast<uintptr_t>(menu_event->itemref) - ITEM_TOGGLE_FIRST, menu_event->iptkey == IPT_UI_RIGHT);
					changed = true;
				}
			}
			break;
		}
	}

	// if something changed, rebuild the menu
	if (changed)
		reset(reset_options::REMEMBER_REF);
}

} // namespace ui
