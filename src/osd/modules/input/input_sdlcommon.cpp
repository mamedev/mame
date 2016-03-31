// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont, Brad Hughes
//============================================================
//
//  input_sdlcommon.cpp - SDL Common code shared by SDL modules
//
//    Note: this code is also used by the X11 input modules
//
//============================================================

#include "input_module.h"
#include "modules/osdmodule.h"

#if defined(OSD_SDL)

// standard sdl header
#include "sdlinc.h"
#include <ctype.h>
#include <stddef.h>
#include <mutex>
#include <memory>

// MAME headers
#include "emu.h"
#include "osdepend.h"
#include "ui/ui.h"
#include "uiinput.h"
#include "window.h"
#include "strconv.h"

#include "../../sdl/osdsdl.h"
#include "input_common.h"
#include "input_sdlcommon.h"

#define GET_WINDOW(ev) window_from_id((ev)->windowID)
//#define GET_WINDOW(ev) ((ev)->windowID)

static inline sdl_window_info * window_from_id(Uint32 windowID)
{
	sdl_window_info *w;
	SDL_Window *window = SDL_GetWindowFromID(windowID);

	for (w = sdl_window_list; w != NULL; w = w->m_next)
	{
		//printf("w->window_id: %d\n", w->window_id);
		if (w->sdl_window() == window)
		{
			return w;
		}
	}
	return NULL;
}

void sdl_event_manager::process_events(running_machine &machine)
{
	std::lock_guard<std::mutex> scope_lock(m_lock);
	SDL_Event sdlevent;
	while (SDL_PollEvent(&sdlevent))
	{
		// process window events if they come in
		if (sdlevent.type == SDL_WINDOWEVENT)
			process_window_event(machine, sdlevent);

		// Find all subscribers for the event type
		auto subscribers = m_subscription_index.equal_range(sdlevent.type);

		// Dispatch the events
		for (auto iter = subscribers.first; iter != subscribers.second; iter++)
			iter->second->handle_event(sdlevent);
	}
}

void sdl_event_manager::process_window_event(running_machine &machine, SDL_Event &sdlevent)
{
	sdl_window_info *window = GET_WINDOW(&sdlevent.window);

	if (window == NULL)
		return;

	switch (sdlevent.window.event)
	{
	case SDL_WINDOWEVENT_SHOWN:
		m_has_focus = true;
		break;

	case SDL_WINDOWEVENT_CLOSE:
		machine.schedule_exit();
		break;

	case SDL_WINDOWEVENT_LEAVE:
		machine.ui_input().push_mouse_leave_event(window->target());
		m_mouse_over_window = 0;
		break;

	case SDL_WINDOWEVENT_MOVED:
		window->notify_changed();
		m_focus_window = window;
		m_has_focus = true;
		break;

	case SDL_WINDOWEVENT_RESIZED:
#ifndef SDLMAME_WIN32
		/* FIXME: SDL2 sends some spurious resize events on Ubuntu
		* while in fullscreen mode. Ignore them for now.
		*/
		if (!window->fullscreen())
#endif
		{
			//printf("event data1,data2 %d x %d %ld\n", event.window.data1, event.window.data2, sizeof(SDL_Event));
			window->resize(sdlevent.window.data1, sdlevent.window.data2);
		}
		m_focus_window = window;
		m_has_focus = true;
		break;

	case SDL_WINDOWEVENT_ENTER:
		m_mouse_over_window = 1;
		/* fall through */
	case SDL_WINDOWEVENT_FOCUS_GAINED:
	case SDL_WINDOWEVENT_EXPOSED:
	case SDL_WINDOWEVENT_MAXIMIZED:
	case SDL_WINDOWEVENT_RESTORED:
		m_focus_window = window;
		m_has_focus = true;
		break;

	case SDL_WINDOWEVENT_MINIMIZED:
	case SDL_WINDOWEVENT_FOCUS_LOST:
		m_has_focus = false;
		break;
	}
}

//============================================================
//  customize_input_type_list
//============================================================

void sdl_osd_interface::customize_input_type_list(simple_list<input_type_entry> &typelist)
{
	input_item_id mameid_code;
	input_code ui_code;
	const char* uimode;
	char fullmode[64];

	// loop over the defaults
	for (input_type_entry &entry : typelist)
	{
		switch (entry.type())
		{
			// configurable UI mode switch
		case IPT_UI_TOGGLE_UI:
			uimode = options().ui_mode_key();
			if (!strcmp(uimode, "auto"))
			{
#if defined(__APPLE__) && defined(__MACH__)
				mameid_code = keyboard_trans_table::instance().lookup_mame_code("ITEM_ID_INSERT");
#else
				mameid_code = keyboard_trans_table::instance().lookup_mame_code("ITEM_ID_SCRLOCK");
#endif
			}
			else
			{
				snprintf(fullmode, 63, "ITEM_ID_%s", uimode);
				mameid_code = keyboard_trans_table::instance().lookup_mame_code(fullmode);
			}
			ui_code = input_code(DEVICE_CLASS_KEYBOARD, 0, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, input_item_id(mameid_code));
			entry.defseq(SEQ_TYPE_STANDARD).set(ui_code);
			break;
			// alt-enter for fullscreen
		case IPT_OSD_1:
			entry.configure_osd("TOGGLE_FULLSCREEN", "Toggle Fullscreen");
			entry.defseq(SEQ_TYPE_STANDARD).set(KEYCODE_ENTER, KEYCODE_LALT);
			break;

			// disable UI_SELECT when LALT is down, this stops selecting
			// things in the menu when toggling fullscreen with LALT+ENTER
			/*          case IPT_UI_SELECT:
			entry.defseq(SEQ_TYPE_STANDARD).set(KEYCODE_ENTER, input_seq::not_code, KEYCODE_LALT);
			break;*/

			// page down for fastforward (must be OSD_3 as per src/emu/ui.c)
		case IPT_UI_FAST_FORWARD:
			entry.defseq(SEQ_TYPE_STANDARD).set(KEYCODE_PGDN);
			break;

			// OSD hotkeys use LCTRL and start at F3, they start at
			// F3 because F1-F2 are hardcoded into many drivers to
			// various dipswitches, and pressing them together with
			// LCTRL will still press/toggle these dipswitches.

			// LCTRL-F3 to toggle fullstretch
		case IPT_OSD_2:
			entry.configure_osd("TOGGLE_FULLSTRETCH", "Toggle Uneven stretch");
			entry.defseq(SEQ_TYPE_STANDARD).set(KEYCODE_F3, KEYCODE_LCONTROL);
			break;
			// add a Not lcrtl condition to the reset key
		case IPT_UI_SOFT_RESET:
			entry.defseq(SEQ_TYPE_STANDARD).set(KEYCODE_F3, input_seq::not_code, KEYCODE_LCONTROL, input_seq::not_code, KEYCODE_LSHIFT);
			break;

			// LCTRL-F4 to toggle keep aspect
		case IPT_OSD_4:
			entry.configure_osd("TOGGLE_KEEP_ASPECT", "Toggle Keepaspect");
			entry.defseq(SEQ_TYPE_STANDARD).set(KEYCODE_F4, KEYCODE_LCONTROL);
			break;
			// add a Not lcrtl condition to the show gfx key
		case IPT_UI_SHOW_GFX:
			entry.defseq(SEQ_TYPE_STANDARD).set(KEYCODE_F4, input_seq::not_code, KEYCODE_LCONTROL);
			break;

			// LCTRL-F5 to toggle OpenGL filtering
		case IPT_OSD_5:
			entry.configure_osd("TOGGLE_FILTER", "Toggle Filter");
			entry.defseq(SEQ_TYPE_STANDARD).set(KEYCODE_F5, KEYCODE_LCONTROL);
			break;
			// add a Not lcrtl condition to the toggle debug key
		case IPT_UI_TOGGLE_DEBUG:
			entry.defseq(SEQ_TYPE_STANDARD).set(KEYCODE_F5, input_seq::not_code, KEYCODE_LCONTROL);
			break;

			// LCTRL-F6 to decrease OpenGL prescaling
		case IPT_OSD_6:
			entry.configure_osd("DECREASE_PRESCALE", "Decrease Prescaling");
			entry.defseq(SEQ_TYPE_STANDARD).set(KEYCODE_F6, KEYCODE_LCONTROL);
			break;
			// add a Not lcrtl condition to the toggle cheat key
		case IPT_UI_TOGGLE_CHEAT:
			entry.defseq(SEQ_TYPE_STANDARD).set(KEYCODE_F6, input_seq::not_code, KEYCODE_LCONTROL);
			break;

			// LCTRL-F7 to increase OpenGL prescaling
		case IPT_OSD_7:
			entry.configure_osd("INCREASE_PRESCALE", "Increase Prescaling");
			entry.defseq(SEQ_TYPE_STANDARD).set(KEYCODE_F7, KEYCODE_LCONTROL);
			break;
			// add a Not lcrtl condition to the load state key
		case IPT_UI_LOAD_STATE:
			entry.defseq(SEQ_TYPE_STANDARD).set(KEYCODE_F7, input_seq::not_code, KEYCODE_LCONTROL, input_seq::not_code, KEYCODE_LSHIFT);
			break;

			// add a Not lcrtl condition to the throttle key
		case IPT_UI_THROTTLE:
			entry.defseq(SEQ_TYPE_STANDARD).set(KEYCODE_F10, input_seq::not_code, KEYCODE_LCONTROL);
			break;

			// disable the config menu if the ALT key is down
			// (allows ALT-TAB to switch between apps)
		case IPT_UI_CONFIGURE:
			entry.defseq(SEQ_TYPE_STANDARD).set(KEYCODE_TAB, input_seq::not_code, KEYCODE_LALT, input_seq::not_code, KEYCODE_RALT);
			break;

			// leave everything else alone
		default:
			break;
		}
	}
}

void sdl_osd_interface::poll_inputs(running_machine &machine)
{
	m_keyboard_input->poll_if_necessary(machine);
	m_mouse_input->poll_if_necessary(machine);
	m_lightgun_input->poll_if_necessary(machine);
	m_joystick_input->poll_if_necessary(machine);
}

void sdl_osd_interface::release_keys()
{
	downcast<input_module_base*>(m_keyboard_input)->devicelist()->reset_devices();
}

bool sdl_osd_interface::should_hide_mouse()
{
	// if we are paused, no
	if (machine().paused())
		return false;

	// if neither mice nor lightguns enabled in the core, then no
	if (!options().mouse() && !options().lightgun())
		return false;

	if (!sdl_event_manager::instance().mouse_over_window())
		return false;

	// otherwise, yes
	return true;
}

void sdl_osd_interface::process_events_buf()
{
	SDL_PumpEvents();
}

#endif
