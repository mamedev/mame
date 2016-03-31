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
#include "osdepend.h"

// MAMEOS headers
#include "winmain.h"

#include "input_common.h"
#include "input_windows.h"

bool windows_osd_interface::should_hide_mouse() const
{
	bool hidemouse = false;
	hidemouse |= downcast<wininput_module*>(m_keyboard_input)->should_hide_mouse();
	hidemouse |= downcast<wininput_module*>(m_mouse_input)->should_hide_mouse();
	hidemouse |= downcast<wininput_module*>(m_lightgun_input)->should_hide_mouse();
	hidemouse |= downcast<wininput_module*>(m_joystick_input)->should_hide_mouse();
	return hidemouse;
}

bool windows_osd_interface::handle_input_event(input_event eventid, void* eventdata) const
{
	bool handled = false;
	handled |= downcast<wininput_module*>(m_keyboard_input)->handle_input_event(eventid, eventdata);
	handled |= downcast<wininput_module*>(m_mouse_input)->handle_input_event(eventid, eventdata);
	handled |= downcast<wininput_module*>(m_lightgun_input)->handle_input_event(eventid, eventdata);
	handled |= downcast<wininput_module*>(m_joystick_input)->handle_input_event(eventid, eventdata);
	return handled;
}

void windows_osd_interface::poll_input(running_machine &machine) const
{
	m_keyboard_input->poll_if_necessary(machine);
	m_mouse_input->poll_if_necessary(machine);
	m_lightgun_input->poll_if_necessary(machine);
	m_joystick_input->poll_if_necessary(machine);
}

//============================================================
//  customize_input_type_list
//============================================================

void windows_osd_interface::customize_input_type_list(simple_list<input_type_entry> &typelist)
{
	const char* uimode;

	// loop over the defaults
	for (input_type_entry &entry : typelist)
		switch (entry.type())
		{
			// disable the config menu if the ALT key is down
			// (allows ALT-TAB to switch between windows apps)
			case IPT_UI_CONFIGURE:
				entry.defseq(SEQ_TYPE_STANDARD).set(KEYCODE_TAB, input_seq::not_code, KEYCODE_LALT, input_seq::not_code, KEYCODE_RALT);
				break;

			// configurable UI mode switch
			case IPT_UI_TOGGLE_UI:
				uimode = options().ui_mode_key();
				if (strcmp(uimode, "auto"))
				{
					std::string fullmode = "ITEM_ID_";
					fullmode += uimode;
					input_item_id const mameid_code = keyboard_trans_table::instance().lookup_mame_code(fullmode.c_str());
					if (ITEM_ID_INVALID != mameid_code)
					{
						input_code const ui_code = input_code(DEVICE_CLASS_KEYBOARD, 0, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, input_item_id(mameid_code));
						entry.defseq(SEQ_TYPE_STANDARD).set(ui_code);
					}
				}
				break;

			// alt-enter for fullscreen
			case IPT_OSD_1:
				entry.configure_osd("TOGGLE_FULLSCREEN", "Toggle Fullscreen");
				entry.defseq(SEQ_TYPE_STANDARD).set(KEYCODE_ENTER, KEYCODE_LALT, input_seq::or_code, KEYCODE_ENTER, KEYCODE_RALT);
				break;

			// lalt-F12 for fullscreen snap (HLSL)
			case IPT_OSD_2:
				entry.configure_osd("RENDER_SNAP", "Take Rendered Snapshot");
				entry.defseq(SEQ_TYPE_STANDARD).set(KEYCODE_F12, KEYCODE_LALT, input_seq::not_code, KEYCODE_LSHIFT);
				break;
			// add a NOT-lalt to our default F12
			case IPT_UI_SNAPSHOT: // emu/input.c: input_seq(KEYCODE_F12, input_seq::not_code, KEYCODE_LSHIFT)
				entry.defseq(SEQ_TYPE_STANDARD).set(KEYCODE_F12, input_seq::not_code, KEYCODE_LSHIFT, input_seq::not_code, KEYCODE_LALT);
				break;

			// lshift-lalt-F12 for fullscreen video (HLSL)
			case IPT_OSD_3:
				entry.configure_osd("RENDER_AVI", "Record Rendered Video");
				entry.defseq(SEQ_TYPE_STANDARD).set(KEYCODE_F12, KEYCODE_LSHIFT, KEYCODE_LALT);
				break;
			// add a NOT-lalt to our default shift-F12
			case IPT_UI_RECORD_MOVIE: // emu/input.c: input_seq(KEYCODE_F12, KEYCODE_LSHIFT)
				entry.defseq(SEQ_TYPE_STANDARD).set(KEYCODE_F12, KEYCODE_LSHIFT, input_seq::not_code, KEYCODE_LALT);
				break;

			// add a NOT-lalt to write timecode file
			case IPT_UI_TIMECODE: // emu/input.c: input_seq(KEYCODE_F12, input_seq::not_code, KEYCODE_LSHIFT)
				entry.defseq(SEQ_TYPE_STANDARD).set(KEYCODE_F12, input_seq::not_code, KEYCODE_LSHIFT, input_seq::not_code, KEYCODE_LALT);
				break;

			// lctrl-lalt-F5 to toggle post-processing
			case IPT_OSD_4:
				entry.configure_osd("POST_PROCESS", "Toggle Post-Processing");
				entry.defseq(SEQ_TYPE_STANDARD).set(KEYCODE_F5, KEYCODE_LALT, KEYCODE_LCONTROL);
				break;
			// add a NOT-lctrl-lalt to our default F5
			case IPT_UI_TOGGLE_DEBUG: // emu/input.c: input_seq(KEYCODE_F5)
				entry.defseq(SEQ_TYPE_STANDARD).set(KEYCODE_F5, input_seq::not_code, KEYCODE_LCONTROL, input_seq::not_code, KEYCODE_LALT);
				break;

			// leave everything else alone
			default:
				break;
		}
}

#endif
