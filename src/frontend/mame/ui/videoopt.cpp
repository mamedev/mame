// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Nathan Woods
/*********************************************************************

    ui/videoopt.cpp

    Internal menus for video options

*********************************************************************/

#include "emu.h"
#include "ui/videoopt.h"

#include "rendfont.h"
#include "rendlay.h"
#include "rendutil.h"

#include <chrono>


namespace ui {

namespace {

constexpr uintptr_t ITEM_ROTATE         = 0x00000100;
constexpr uintptr_t ITEM_ZOOM           = 0x00000101;
constexpr uintptr_t ITEM_UNEVENSTRETCH  = 0x00000102;
constexpr uintptr_t ITEM_KEEPASPECT     = 0x00000103;
constexpr uintptr_t ITEM_POINTERTIMEOUT = 0x00000104;
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
	set_heading(_("Video Options"));
}

menu_video_targets::~menu_video_targets()
{
}

void menu_video_targets::populate()
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
	item_append(menu_item_type::SEPARATOR);
}

/*-------------------------------------------------
    menu_video_targets - handle the video targets
    menu
-------------------------------------------------*/

bool menu_video_targets::handle(event const *ev)
{
	if (ev && (ev->iptkey == IPT_UI_SELECT))
	{
		render_target *const target = reinterpret_cast<render_target *>(ev->itemref);
		menu::stack_push<menu_video_options>(
				ui(),
				container(),
				std::string(selected_item().text()),
				*target,
				&machine().video().snapshot_target() == target);
	}

	return false;
}


/*-------------------------------------------------
    menu_video_options_populate - populate the
    video options menu
-------------------------------------------------*/

menu_video_options::menu_video_options(
		mame_ui_manager &mui,
		render_container &container,
		std::string_view title,
		render_target &target,
		bool snapshot)
	: menu(mui, container)
	, m_target(target)
	, m_snapshot(snapshot)
{
	set_heading(util::string_format(_("Video Options: %1$s"), title));

	if (!m_snapshot || !machine().video().snap_native())
	{
		set_selected_index(target.view());
		reset(reset_options::REMEMBER_POSITION);
	}
}

menu_video_options::~menu_video_options()
{
}

void menu_video_options::populate()
{
	uintptr_t ref;

	// add items for each view
	if (!m_snapshot || !machine().video().snap_native())
	{
		for (char const *name = m_target.view_name(ref = 0); name; name = m_target.view_name(++ref))
			item_append(name, convert_command_glyph(ref == m_target.view() ? "_>" : "_<"), 0, reinterpret_cast<void *>(ITEM_VIEW_FIRST + ref));
		item_append(menu_item_type::SEPARATOR);
	}

	// add items for visibility toggles
	layout_view const &curview = m_target.current_view();
	auto const &toggles = curview.visibility_toggles();
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
	case ROT0:      subtext = "None";       break;
	case ROT90:     subtext = u8"CW 90°";   break;
	case ROT180:    subtext = u8"180°";     break;
	case ROT270:    subtext = u8"CCW 90°";  break;
	}
	item_append(_("Rotate"), subtext, FLAG_LEFT_ARROW | FLAG_RIGHT_ARROW, reinterpret_cast<void *>(ITEM_ROTATE));

	// cropping
	bool const canzoom(curview.has_art() && !curview.visible_screens().empty());
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

	// add pointer display options
	if (!m_target.hidden())
	{
		item_append(menu_item_type::SEPARATOR);

		// use millisecond precision for timeout display
		auto const timeout = std::chrono::duration_cast<std::chrono::milliseconds>(ui().pointer_activity_timeout(m_target.index()));
		bool const hide = ui().hide_inactive_pointers(m_target.index());
		if (hide)
		{
			if (timeout.count())
			{
				int const precision = (timeout.count() % 10) ? 3 : (timeout.count() % 100) ? 2 : 1;
				item_append(
						_("Hide Inactive Pointers After Delay"),
						util::string_format(_("%1$.*2$f s"), timeout.count() * 1e-3, precision),
						((timeout >= std::chrono::milliseconds(100)) ? FLAG_LEFT_ARROW : 0) | FLAG_RIGHT_ARROW,
						reinterpret_cast<void *>(ITEM_POINTERTIMEOUT));
			}
			else
				item_append(_("Hide Inactive Pointers After Delay"), _("Always"), FLAG_RIGHT_ARROW, reinterpret_cast<void *>(ITEM_POINTERTIMEOUT));
		}
		else
			item_append(_("Hide Inactive Pointers After Delay"), _("Never"), FLAG_LEFT_ARROW, reinterpret_cast<void *>(ITEM_POINTERTIMEOUT));
	}

	item_append(menu_item_type::SEPARATOR);
}


/*-------------------------------------------------
    menu_video_options - handle the video options
    menu
-------------------------------------------------*/

bool menu_video_options::handle(event const *ev)
{
	auto const lockout_popup(
			[this] ()
			{
				machine().popmessage(_("Cannot change options while recording!"));
				return true;
			});
	bool const snap_lockout(m_snapshot && machine().video().is_recording());
	bool changed(false);
	set_process_flags((reinterpret_cast<uintptr_t>(get_selection_ref()) == ITEM_POINTERTIMEOUT) ? PROCESS_LR_REPEAT : 0);

	// process the menu
	if (ev && uintptr_t(ev->itemref))
	{
		switch (reinterpret_cast<uintptr_t>(ev->itemref))
		{
		// rotate adds rotation depending on the direction
		case ITEM_ROTATE:
			if (ev->iptkey == IPT_UI_LEFT || ev->iptkey == IPT_UI_RIGHT)
			{
				if (snap_lockout)
					return lockout_popup();
				int const delta((ev->iptkey == IPT_UI_LEFT) ? ROT270 : ROT90);
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
			if ((ev->iptkey == IPT_UI_LEFT) || (ev->iptkey == IPT_UI_RIGHT))
			{
				if (snap_lockout)
					return lockout_popup();
				m_target.set_zoom_to_screen(ev->iptkey == IPT_UI_RIGHT);
				changed = true;
			}
			break;

		// non-integer scaling: rotate through options
		case ITEM_UNEVENSTRETCH:
			if (ev->iptkey == IPT_UI_LEFT)
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
			else if (ev->iptkey == IPT_UI_RIGHT)
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

		// keep aspect handles left/right keys identically (toggle)
		case ITEM_KEEPASPECT:
			if ((ev->iptkey == IPT_UI_LEFT) || (ev->iptkey == IPT_UI_RIGHT))
			{
				if (snap_lockout)
					return lockout_popup();
				m_target.set_keepaspect(ev->iptkey == IPT_UI_RIGHT);
				changed = true;
			}
			break;

		// pointer inactivity timeout
		case ITEM_POINTERTIMEOUT:
			switch (ev->iptkey)
			{
			// decrease value
			case IPT_UI_LEFT:
				if (!ui().hide_inactive_pointers(m_target.index()))
				{
					ui().set_hide_inactive_pointers(m_target.index(), true);
					ui().set_pointer_activity_timeout(m_target.index(), std::chrono::milliseconds(10'000));
					changed = true;
				}
				else
				{
					auto timeout = ui().pointer_activity_timeout(m_target.index());
					if (timeout >= std::chrono::milliseconds(100))
					{
						bool const shift_pressed = machine().input().code_pressed(KEYCODE_LSHIFT) || machine().input().code_pressed(KEYCODE_RSHIFT);
						std::chrono::milliseconds const increment(shift_pressed ? 100 : 1'000);
						auto const remainder = timeout % increment;
						timeout -= remainder.count() ? remainder : increment;
						ui().set_pointer_activity_timeout(m_target.index(), timeout);
						changed = true;

						if (!timeout.count())
							machine().popmessage(_("Clickable artwork is still active when pointer is hidden."));
					}
				}
				break;

			// increase value
			case IPT_UI_RIGHT:
				if (ui().hide_inactive_pointers(m_target.index()))
				{
					auto const timeout = ui().pointer_activity_timeout(m_target.index());
					if (std::chrono::milliseconds(10'000) <= timeout)
					{
						ui().set_hide_inactive_pointers(m_target.index(), false);
					}
					else
					{
						bool const shift_pressed = machine().input().code_pressed(KEYCODE_LSHIFT) || machine().input().code_pressed(KEYCODE_RSHIFT);
						int const increment(shift_pressed ? 100 : 1'000);
						ui().set_pointer_activity_timeout(
								m_target.index(),
								std::chrono::milliseconds((1 + (timeout / std::chrono::milliseconds(increment))) * increment));
					}
					changed = true;
				}
				break;

			// toggle hide after delay
			case IPT_UI_SELECT:
				ui().set_hide_inactive_pointers(m_target.index(), !ui().hide_inactive_pointers(m_target.index()));
				changed = true;
				break;

			// restore initial setting
			case IPT_UI_CLEAR:
				ui().restore_initial_pointer_options(m_target.index());
				changed = true;
				break;
			}
			break;

		// anything else is a view item
		default:
			if (reinterpret_cast<uintptr_t>(ev->itemref) >= ITEM_VIEW_FIRST)
			{
				if (snap_lockout)
					return lockout_popup();
				if (ev->iptkey == IPT_UI_SELECT)
				{
					m_target.set_view(reinterpret_cast<uintptr_t>(ev->itemref) - ITEM_VIEW_FIRST);
					changed = true;
				}
			}
			else if (reinterpret_cast<uintptr_t>(ev->itemref) >= ITEM_TOGGLE_FIRST)
			{
				if (snap_lockout)
					return lockout_popup();
				if ((ev->iptkey == IPT_UI_LEFT) || (ev->iptkey == IPT_UI_RIGHT))
				{
					m_target.set_visibility_toggle(reinterpret_cast<uintptr_t>(ev->itemref) - ITEM_TOGGLE_FIRST, ev->iptkey == IPT_UI_RIGHT);
					changed = true;
				}
			}
			break;
		}
	}

	// if something changed, rebuild the menu
	if (changed)
		reset(reset_options::REMEMBER_REF);
	return false;
}

} // namespace ui
