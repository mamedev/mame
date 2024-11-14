// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Brad Hughes
//============================================================
//
//  input_windows.cpp - Common code used by Windows input modules
//
//============================================================

#include "input_module.h"

#if defined(OSD_WINDOWS)

// MAME headers
#include "emu.h"

#include "input_windows.h"

#include "window.h"
#include "winmain.h"

#include "util/language.h"

#include "osdepend.h"


bool windows_osd_interface::should_hide_mouse() const
{
	if (!winwindow_has_focus())
		return false;

	if (machine().paused())
		return false;

	// track if mouse/lightgun is enabled, for mouse hiding purposes
	bool const mouse_enabled = machine().input().class_enabled(DEVICE_CLASS_MOUSE);
	bool const lightgun_enabled = machine().input().class_enabled(DEVICE_CLASS_LIGHTGUN);
	if (!mouse_enabled && !lightgun_enabled)
		return false;

	return true;
}

bool windows_osd_interface::handle_input_event(input_event eventid, void const *eventdata) const
{
	bool handled = false;

	wininput_event_handler *mod;

	mod = dynamic_cast<wininput_event_handler *>(m_keyboard_input);
	if (mod)
		handled |= mod->handle_input_event(eventid, eventdata);

	mod = dynamic_cast<wininput_event_handler *>(m_mouse_input);
	if (mod)
		handled |= mod->handle_input_event(eventid, eventdata);

	mod = dynamic_cast<wininput_event_handler *>(m_lightgun_input);
	if (mod)
		handled |= mod->handle_input_event(eventid, eventdata);

	mod = dynamic_cast<wininput_event_handler *>(m_joystick_input);
	if (mod)
		handled |= mod->handle_input_event(eventid, eventdata);

	return handled;
}

//============================================================
//  customize_input_type_list
//============================================================

void windows_osd_interface::customize_input_type_list(std::vector<input_type_entry> &typelist)
{
	// loop over the defaults
	for (input_type_entry &entry : typelist)
		switch (entry.type())
		{
			// disable the config menu if the ALT key is down
			// (allows ALT-TAB to switch between windows apps)
			case IPT_UI_MENU:
				entry.defseq(SEQ_TYPE_STANDARD).set(KEYCODE_TAB, input_seq::not_code, KEYCODE_LALT, input_seq::not_code, KEYCODE_RALT);
				break;

			// configurable UI mode switch
			case IPT_UI_TOGGLE_UI:
				{
					char const *const uimode = options().ui_mode_key();
					if (uimode && *uimode && strcmp(uimode, "auto"))
					{
						std::string fullmode("ITEM_ID_");
						fullmode.append(uimode);
						input_item_id const mameid_code = keyboard_trans_table::instance().lookup_mame_code(fullmode.c_str());
						if (ITEM_ID_INVALID != mameid_code)
						{
							input_code const ui_code = input_code(DEVICE_CLASS_KEYBOARD, 0, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, input_item_id(mameid_code));
							entry.defseq(SEQ_TYPE_STANDARD).set(ui_code);
						}
					}
				}
				break;

			// alt-enter for fullscreen
			case IPT_OSD_1:
				entry.configure_osd("TOGGLE_FULLSCREEN", N_p("input-name", "Toggle Fullscreen"));
				entry.defseq(SEQ_TYPE_STANDARD).set(KEYCODE_ENTER, KEYCODE_LALT, input_seq::or_code, KEYCODE_ENTER, KEYCODE_RALT);
				break;

			// lalt-F12 for fullscreen snap (HLSL)
			case IPT_OSD_2:
				entry.configure_osd("RENDER_SNAP", N_p("input-name", "Take Rendered Snapshot"));
				entry.defseq(SEQ_TYPE_STANDARD).set(KEYCODE_F12, KEYCODE_LALT, input_seq::not_code, KEYCODE_LSHIFT);
				break;

			// add a NOT-lalt to our default F12
			case IPT_UI_SNAPSHOT: // emu/input.c: input_seq(KEYCODE_F12, input_seq::not_code, KEYCODE_LSHIFT)
				entry.defseq(SEQ_TYPE_STANDARD).set(KEYCODE_F12, input_seq::not_code, KEYCODE_LSHIFT, input_seq::not_code, KEYCODE_LALT);
				break;

			// lshift-lalt-F12 for fullscreen video (HLSL, BGFX)
			case IPT_OSD_3:
				entry.configure_osd("RENDER_AVI", N_p("input-name", "Record Rendered Video"));
				entry.defseq(SEQ_TYPE_STANDARD).set(KEYCODE_F12, KEYCODE_LSHIFT, KEYCODE_LALT);
				break;

			// add a NOT-lalt to our default shift-F12
			case IPT_UI_RECORD_MNG: // emu/input.c: input_seq(KEYCODE_F12, KEYCODE_LSHIFT, input_seq::not_code, KEYCODE_LCONTROL)
				entry.defseq(SEQ_TYPE_STANDARD).set(KEYCODE_F12, KEYCODE_LSHIFT, input_seq::not_code, KEYCODE_LCONTROL, input_seq::not_code, KEYCODE_LALT);
				break;

			// add a NOT-lalt to our default shift-ctrl-F12
			case IPT_UI_RECORD_AVI: // emu/input.c: input_seq(KEYCODE_F12, KEYCODE_LSHIFT, KEYCODE_LCONTROL)
				entry.defseq(SEQ_TYPE_STANDARD).set(KEYCODE_F12, KEYCODE_LSHIFT, KEYCODE_LCONTROL, input_seq::not_code, KEYCODE_LALT);
				break;

			// lalt-F10 to toggle post-processing
			case IPT_OSD_4:
				entry.configure_osd("POST_PROCESS", N_p("input-name", "Toggle Post-Processing"));
				entry.defseq(SEQ_TYPE_STANDARD).set(KEYCODE_F10, KEYCODE_LALT);
				break;

			// add a Not LALT condition to the throttle key
			case IPT_UI_THROTTLE:
				entry.defseq(SEQ_TYPE_STANDARD).set(KEYCODE_F10, input_seq::not_code, KEYCODE_LALT);
				break;

			// leave everything else alone
			default:
				break;
		}
}

#endif // defined(OSD_WINDOWS)
