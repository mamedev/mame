// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont

#include "osdsdl.h"

#include "modules/input/input_common.h"
#include "modules/lib/osdlib.h"
#include "window.h"

#include "util/language.h"
#include "util/unicode.h"

// TODO: reduce dependence on concrete emu classes
#include "emu.h"
#include "main.h"
#include "uiinput.h"

#include "ui/uimain.h"

#include <SDL2/SDL.h>

#include <cstdio>
#include <cstring>


namespace {

//============================================================
//  defines_verbose
//============================================================

#define MAC_EXPAND_STR(_m) #_m
#define MACRO_VERBOSE(_mac) \
	do { \
		if (strcmp(MAC_EXPAND_STR(_mac), #_mac) != 0) \
			osd_printf_verbose("%s=%s ", #_mac, MAC_EXPAND_STR(_mac)); \
	} while (0)

void defines_verbose()
{
	osd_printf_verbose("Build version:      %s\n", emulator_info::get_build_version());
	osd_printf_verbose("Build architecure:  ");
	MACRO_VERBOSE(SDLMAME_ARCH);
	osd_printf_verbose("\n");
	osd_printf_verbose("Build defines 1:    ");
	MACRO_VERBOSE(SDLMAME_UNIX);
	MACRO_VERBOSE(SDLMAME_X11);
	MACRO_VERBOSE(SDLMAME_WIN32);
	MACRO_VERBOSE(SDLMAME_MACOSX);
	MACRO_VERBOSE(SDLMAME_DARWIN);
	MACRO_VERBOSE(SDLMAME_LINUX);
	MACRO_VERBOSE(SDLMAME_SOLARIS);
	MACRO_VERBOSE(SDLMAME_IRIX);
	MACRO_VERBOSE(SDLMAME_BSD);
	osd_printf_verbose("\n");
	osd_printf_verbose("Build defines 1:    ");
	MACRO_VERBOSE(LSB_FIRST);
	MACRO_VERBOSE(PTR64);
	MACRO_VERBOSE(MAME_NOASM);
	MACRO_VERBOSE(MAME_DEBUG);
	MACRO_VERBOSE(BIGENDIAN);
	MACRO_VERBOSE(CPP_COMPILE);
	MACRO_VERBOSE(SYNC_IMPLEMENTATION);
	osd_printf_verbose("\n");
	osd_printf_verbose("SDL/OpenGL defines: ");
	osd_printf_verbose("SDL_COMPILEDVERSION=%d ", SDL_COMPILEDVERSION);
	MACRO_VERBOSE(USE_OPENGL);
	MACRO_VERBOSE(USE_DISPATCH_GL);
	osd_printf_verbose("\n");
	osd_printf_verbose("Compiler defines A: ");
	MACRO_VERBOSE(__GNUC__);
	MACRO_VERBOSE(__GNUC_MINOR__);
	MACRO_VERBOSE(__GNUC_PATCHLEVEL__);
	MACRO_VERBOSE(__VERSION__);
	osd_printf_verbose("\n");
	osd_printf_verbose("Compiler defines B: ");
	MACRO_VERBOSE(__amd64__);
	MACRO_VERBOSE(__x86_64__);
	MACRO_VERBOSE(__unix__);
	MACRO_VERBOSE(__i386__);
	MACRO_VERBOSE(__ppc__);
	MACRO_VERBOSE(__ppc64__);
	osd_printf_verbose("\n");
	osd_printf_verbose("Compiler defines C: ");
	MACRO_VERBOSE(_FORTIFY_SOURCE);
	MACRO_VERBOSE(__USE_FORTIFY_LEVEL);
	osd_printf_verbose("\n");
}


//============================================================
//  osd_sdl_info
//============================================================

void osd_sdl_info()
{
	int num = SDL_GetNumVideoDrivers();

	osd_printf_verbose("Available videodrivers: ");
	for (int i = 0; i < num; i++)
	{
		const char *name = SDL_GetVideoDriver(i);
		osd_printf_verbose("%s ", name);
	}
	osd_printf_verbose("\n");

	osd_printf_verbose("Current Videodriver: %s\n", SDL_GetCurrentVideoDriver());
	num = SDL_GetNumVideoDisplays();
	for (int i = 0; i < num; i++)
	{
		SDL_DisplayMode mode;

		osd_printf_verbose("\tDisplay #%d\n", i);
		if (SDL_GetDesktopDisplayMode(i, &mode) == 0)
			osd_printf_verbose("\t\tDesktop Mode:         %dx%d-%d@%d\n", mode.w, mode.h, SDL_BITSPERPIXEL(mode.format), mode.refresh_rate);
		if (SDL_GetCurrentDisplayMode(i, &mode) == 0)
			osd_printf_verbose("\t\tCurrent Display Mode: %dx%d-%d@%d\n", mode.w, mode.h, SDL_BITSPERPIXEL(mode.format), mode.refresh_rate);

		osd_printf_verbose("\t\tRenderdrivers:\n");
		for (int j = 0; j < SDL_GetNumRenderDrivers(); j++)
		{
			SDL_RendererInfo info;
			SDL_GetRenderDriverInfo(j, &info);
			osd_printf_verbose("\t\t\t%10s (%dx%d)\n", info.name, info.max_texture_width, info.max_texture_height);
		}
	}

	osd_printf_verbose("Available audio drivers: \n");
	num = SDL_GetNumAudioDrivers();
	for (int i = 0; i < num; i++)
	{
		osd_printf_verbose("\t%-20s\n", SDL_GetAudioDriver(i));
	}
}


sdl_window_info *window_from_id(Uint32 id)
{
	SDL_Window const *const sdl_window = SDL_GetWindowFromID(id);

	auto const window = std::find_if(
			osd_common_t::window_list().begin(),
			osd_common_t::window_list().end(),
			[sdl_window] (std::unique_ptr<osd_window> const &w)
			{
				return dynamic_cast<sdl_window_info &>(*w).platform_window() == sdl_window;
			});

	if (window == osd_common_t::window_list().end())
		return nullptr;

	return &static_cast<sdl_window_info &>(**window);
}

} // anonymous namespace



//============================================================
//  SDL OSD interface
//============================================================

sdl_osd_interface::sdl_osd_interface(sdl_options &options) :
	osd_common_t(options),
	m_options(options),
	m_focus_window(nullptr),
	m_mouse_over_window(0),
	m_modifier_keys(0),
	m_last_click_time(std::chrono::steady_clock::time_point::min()),
	m_last_click_x(0),
	m_last_click_y(0)
{
}


sdl_osd_interface::~sdl_osd_interface()
{
}


void sdl_osd_interface::init(running_machine &machine)
{
	// call our parent
	osd_common_t::init(machine);

	const char *stemp;

	// determine if we are benchmarking, and adjust options appropriately
	int bench = options().bench();
	if (bench > 0)
	{
		options().set_value(OPTION_SLEEP, false, OPTION_PRIORITY_MAXIMUM);
		options().set_value(OPTION_THROTTLE, false, OPTION_PRIORITY_MAXIMUM);
		options().set_value(OSDOPTION_SOUND, "none", OPTION_PRIORITY_MAXIMUM);
		options().set_value(OSDOPTION_VIDEO, "none", OPTION_PRIORITY_MAXIMUM);
		options().set_value(OPTION_SECONDS_TO_RUN, bench, OPTION_PRIORITY_MAXIMUM);
	}

	// Some driver options - must be before audio init!
	stemp = options().audio_driver();
	if (stemp != nullptr && strcmp(stemp, OSDOPTVAL_AUTO) != 0)
	{
		osd_printf_verbose("Setting SDL audiodriver '%s' ...\n", stemp);
		osd_setenv(SDLENV_AUDIODRIVER, stemp, 1);
	}

	stemp = options().video_driver();
	if (stemp != nullptr && strcmp(stemp, OSDOPTVAL_AUTO) != 0)
	{
		osd_printf_verbose("Setting SDL videodriver '%s' ...\n", stemp);
		osd_setenv(SDLENV_VIDEODRIVER, stemp, 1);
	}

	stemp = options().render_driver();
	if (stemp != nullptr)
	{
		if (strcmp(stemp, OSDOPTVAL_AUTO) != 0)
		{
			osd_printf_verbose("Setting SDL renderdriver '%s' ...\n", stemp);
			//osd_setenv(SDLENV_RENDERDRIVER, stemp, 1);
			SDL_SetHint(SDL_HINT_RENDER_DRIVER, stemp);
		}
		else
		{
#if defined(SDLMAME_WIN32)
			// OpenGL renderer has less issues with mode switching on windows
			osd_printf_verbose("Setting SDL renderdriver '%s' ...\n", "opengl");
			//osd_setenv(SDLENV_RENDERDRIVER, stemp, 1);
			SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");
#endif
		}
	}

	/* Set the SDL environment variable for drivers wanting to load the
	 * lib at startup.
	 */
#if USE_OPENGL
	/* FIXME: move lib loading code from drawogl.c here */

	stemp = options().gl_lib();
	if (stemp != nullptr && strcmp(stemp, OSDOPTVAL_AUTO) != 0)
	{
		osd_setenv("SDL_VIDEO_GL_DRIVER", stemp, 1);
		osd_printf_verbose("Setting SDL_VIDEO_GL_DRIVER = '%s' ...\n", stemp);
	}
#endif

	/* get number of processors */
	stemp = options().numprocessors();

	osd_num_processors = 0;

	if (strcmp(stemp, "auto") != 0)
	{
		osd_num_processors = atoi(stemp);
		if (osd_num_processors < 1)
		{
			osd_printf_warning("numprocessors < 1 doesn't make much sense. Assuming auto ...\n");
			osd_num_processors = 0;
		}
	}

#if defined(SDLMAME_ANDROID)
	SDL_SetHint(SDL_HINT_VIDEO_EXTERNAL_CONTEXT, "1");
#endif
	/* Initialize SDL */

	if (SDL_InitSubSystem(SDL_INIT_VIDEO))
	{
		osd_printf_error("Could not initialize SDL %s\n", SDL_GetError());
		exit(-1);
	}

	osd_sdl_info();

	defines_verbose();

	osd_common_t::init_subsystems();

	if (options().oslog())
	{
		using namespace std::placeholders;
		machine.add_logerror_callback(std::bind(&sdl_osd_interface::output_oslog, this, _1));
	}



#ifdef SDLMAME_EMSCRIPTEN
	SDL_EventState(SDL_TEXTINPUT, SDL_FALSE);
#else
	SDL_EventState(SDL_TEXTINPUT, SDL_TRUE);
#endif
}


void sdl_osd_interface::input_update(bool relative_reset)
{
	process_events_buf();
	poll_input_modules(relative_reset);
}


void sdl_osd_interface::customize_input_type_list(std::vector<input_type_entry> &typelist)
{
	// loop over the defaults
	for (input_type_entry &entry : typelist)
	{
		switch (entry.type())
		{
			// configurable UI mode switch
		case IPT_UI_TOGGLE_UI:
			{
				char const *const uimode = options().ui_mode_key();
				input_item_id mameid_code = ITEM_ID_INVALID;
				if (!uimode || !*uimode || !strcmp(uimode, "auto"))
				{
#if defined(__APPLE__) && defined(__MACH__)
					mameid_code = keyboard_trans_table::instance().lookup_mame_code("ITEM_ID_INSERT");
#endif
				}
				else
				{
					std::string fullmode("ITEM_ID_");
					fullmode.append(uimode);
					mameid_code = keyboard_trans_table::instance().lookup_mame_code(fullmode.c_str());
				}
				if (ITEM_ID_INVALID != mameid_code)
				{
					input_code const ui_code = input_code(DEVICE_CLASS_KEYBOARD, 0, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, input_item_id(mameid_code));
					entry.defseq(SEQ_TYPE_STANDARD).set(ui_code);
				}
			}
			break;

		// alt-enter for fullscreen
		case IPT_OSD_1:
			entry.configure_osd("TOGGLE_FULLSCREEN", N_p("input-name", "Toggle Fullscreen"));
			entry.defseq(SEQ_TYPE_STANDARD).set(KEYCODE_ENTER, KEYCODE_LALT);
			break;

		// page down for fastforward (must be OSD_3 as per src/emu/ui.c)
		case IPT_UI_FAST_FORWARD:
			entry.defseq(SEQ_TYPE_STANDARD).set(KEYCODE_PGDN);
			break;

		// OSD hotkeys use LCTRL and start at F3, they start at
		// F3 because F1-F2 are hardcoded into many drivers to
		// various dipswitches, and pressing them together with
		// LCTRL will still press/toggle these dipswitches.

		// LALT-F10 to toggle OpenGL filtering
		case IPT_OSD_5:
			entry.configure_osd("TOGGLE_FILTER", N_p("input-name", "Toggle Filter"));
			entry.defseq(SEQ_TYPE_STANDARD).set(KEYCODE_F10, KEYCODE_LALT);
			break;

		// add a Not LALT condition to the throttle key
		case IPT_UI_THROTTLE:
			entry.defseq(SEQ_TYPE_STANDARD).set(KEYCODE_F10, input_seq::not_code, KEYCODE_LALT);
			break;

		// LCTRL-F6 to decrease OpenGL prescaling
		case IPT_OSD_6:
			entry.configure_osd("DECREASE_PRESCALE", N_p("input-name", "Decrease Prescaling"));
			entry.defseq(SEQ_TYPE_STANDARD).set(KEYCODE_F6, KEYCODE_LCONTROL);
			break;

		// add a Not LCTRL condition to the save state key
		case IPT_UI_SAVE_STATE:
			entry.defseq(SEQ_TYPE_STANDARD).set(KEYCODE_F6, input_seq::not_code, KEYCODE_LCONTROL, input_seq::not_code, KEYCODE_LSHIFT, input_seq::not_code, KEYCODE_RSHIFT);
			break;

		// LCTRL-F7 to increase OpenGL prescaling
		case IPT_OSD_7:
			entry.configure_osd("INCREASE_PRESCALE", N_p("input-name", "Increase Prescaling"));
			entry.defseq(SEQ_TYPE_STANDARD).set(KEYCODE_F7, KEYCODE_LCONTROL);
			break;

		// add a Not LCTRL condition to the load state key
		case IPT_UI_LOAD_STATE:
			entry.defseq(SEQ_TYPE_STANDARD).set(KEYCODE_F7, input_seq::not_code, KEYCODE_LCONTROL, input_seq::not_code, KEYCODE_LSHIFT, input_seq::not_code, KEYCODE_RSHIFT);
			break;

		// LSHIFT-LALT-F12 for fullscreen video (BGFX)
		case IPT_OSD_8:
			entry.configure_osd("RENDER_AVI", N_p("input-name", "Record Rendered Video"));
			entry.defseq(SEQ_TYPE_STANDARD).set(KEYCODE_F12, KEYCODE_LSHIFT, KEYCODE_LALT);
			break;

		// disable the config menu if the ALT key is down
		// (allows ALT-TAB to switch between apps)
		case IPT_UI_MENU:
			entry.defseq(SEQ_TYPE_STANDARD).set(KEYCODE_TAB, input_seq::not_code, KEYCODE_LALT, input_seq::not_code, KEYCODE_RALT);
			break;

#if defined(__APPLE__) && defined(__MACH__)
		// 78-key Apple MacBook & Bluetooth keyboards have no right control key
		case IPT_MAHJONG_SCORE:
			if (entry.player() == 0)
				entry.defseq(SEQ_TYPE_STANDARD).set(KEYCODE_SLASH);
			break;
#endif

		// leave everything else alone
		default:
			break;
		}
	}
}


void sdl_osd_interface::release_keys()
{
	auto const keybd = dynamic_cast<input_module_base *>(m_keyboard_input);
	if (keybd)
		keybd->reset_devices();
}


bool sdl_osd_interface::should_hide_mouse()
{
	// if we are paused, no
	if (machine().paused())
		return false;

	// if neither mice nor lightguns are enabled in the core, then no
	if (!options().mouse() && !options().lightgun())
		return false;

	if (!mouse_over_window())
		return false;

	// otherwise, yes
	return true;
}


void sdl_osd_interface::process_events_buf()
{
	SDL_PumpEvents();
}


void sdl_osd_interface::process_events()
{
	std::lock_guard<std::mutex> lock(subscription_mutex());
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		// handle UI events
		switch (event.type)
		{
		case SDL_WINDOWEVENT:
			process_window_event(event);
			break;

		case SDL_KEYDOWN:
			if (event.key.keysym.scancode == SDL_SCANCODE_LCTRL)
				m_modifier_keys |= MODIFIER_KEY_LCTRL;
			else if (event.key.keysym.scancode == SDL_SCANCODE_RCTRL)
				m_modifier_keys |= MODIFIER_KEY_RCTRL;
			else if (event.key.keysym.scancode == SDL_SCANCODE_LSHIFT)
				m_modifier_keys |= MODIFIER_KEY_LSHIFT;
			else if (event.key.keysym.scancode == SDL_SCANCODE_RSHIFT)
				m_modifier_keys |= MODIFIER_KEY_RSHIFT;

			if (event.key.keysym.sym < 0x20)
			{
				// push control characters - they don't arrive as text input events
				machine().ui_input().push_char_event(osd_common_t::window_list().front()->target(), event.key.keysym.sym);
			}
			else if (m_modifier_keys & MODIFIER_KEY_CTRL)
			{
				// SDL filters out control characters for text input, so they are decoded here
				if (event.key.keysym.sym >= 0x40 && event.key.keysym.sym < 0x7f)
				{
					machine().ui_input().push_char_event(osd_common_t::window_list().front()->target(), event.key.keysym.sym & 0x1f);
				}
				else if (m_modifier_keys & MODIFIER_KEY_SHIFT)
				{
					if (event.key.keysym.sym == SDLK_6) // Ctrl-^ (RS)
						machine().ui_input().push_char_event(osd_common_t::window_list().front()->target(), 0x1e);
					else if (event.key.keysym.sym == SDLK_MINUS) // Ctrl-_ (US)
						machine().ui_input().push_char_event(osd_common_t::window_list().front()->target(), 0x1f);
				}
			}
			break;

		case SDL_KEYUP:
			if (event.key.keysym.scancode == SDL_SCANCODE_LCTRL)
				m_modifier_keys &= ~MODIFIER_KEY_LCTRL;
			else if (event.key.keysym.scancode == SDL_SCANCODE_RCTRL)
				m_modifier_keys &= ~MODIFIER_KEY_RCTRL;
			else if (event.key.keysym.scancode == SDL_SCANCODE_LSHIFT)
				m_modifier_keys &= ~MODIFIER_KEY_LSHIFT;
			else if (event.key.keysym.scancode == SDL_SCANCODE_RSHIFT)
				m_modifier_keys &= ~MODIFIER_KEY_RSHIFT;
			break;

		case SDL_TEXTINPUT:
			process_textinput_event(event);
			break;

		case SDL_MOUSEMOTION:
			{
				int cx, cy;
				auto const window = focus_window(event.motion);
				if (window && window->xy_to_render_target(event.motion.x, event.motion.y, &cx, &cy))
					machine().ui_input().push_mouse_move_event(window->target(), cx, cy);
			}
			break;

		case SDL_MOUSEBUTTONDOWN:
			//printf("But down %d %d %d %d %s\n", event.button.which, event.button.button, event.button.x, event.button.y, devinfo->name.c_str());
			if (event.button.button == 1)
			{
				int cx, cy;
				auto const window = focus_window(event.button);
				if (window && window->xy_to_render_target(event.button.x, event.button.y, &cx, &cy))
				{
					auto const double_click_speed = std::chrono::milliseconds(250);
					auto const click = std::chrono::steady_clock::now();
					machine().ui_input().push_mouse_down_event(window->target(), cx, cy);

					// avoid overflow with std::chrono::time_point::min() by adding rather than subtracting
					if (click < (m_last_click_time + double_click_speed)
						&& (cx >= (m_last_click_x - 4) && cx <= (m_last_click_x + 4))
						&& (cy >= (m_last_click_y - 4) && cy <= (m_last_click_y + 4)))
					{
						m_last_click_time = std::chrono::time_point<std::chrono::steady_clock>::min();
						machine().ui_input().push_mouse_double_click_event(window->target(), cx, cy);
					}
					else
					{
						m_last_click_time = click;
						m_last_click_x = cx;
						m_last_click_y = cy;
					}
				}
			}
			else if (event.button.button == 3)
			{
				int cx, cy;
				auto const window = focus_window(event.button);
				if (window != nullptr && window->xy_to_render_target(event.button.x, event.button.y, &cx, &cy))
					machine().ui_input().push_mouse_rdown_event(window->target(), cx, cy);
			}
			break;

		case SDL_MOUSEBUTTONUP:
			//printf("But up %d %d %d %d\n", event.button.which, event.button.button, event.button.x, event.button.y);
			if (event.button.button == 1)
			{
				int cx, cy;
				auto const window = focus_window(event.button);
				if (window && window->xy_to_render_target(event.button.x, event.button.y, &cx, &cy))
					machine().ui_input().push_mouse_up_event(window->target(), cx, cy);
			}
			else if (event.button.button == 3)
			{
				int cx, cy;
				auto window = focus_window(event.button);
				if (window && window->xy_to_render_target(event.button.x, event.button.y, &cx, &cy))
					machine().ui_input().push_mouse_rup_event(window->target(), cx, cy);
			}
			break;

		case SDL_MOUSEWHEEL:
			{
				auto const window = focus_window(event.wheel);
				if (window)
					machine().ui_input().push_mouse_wheel_event(window->target(), 0, 0, event.wheel.y, 3);
			}
			break;
		}

		// let input modules do their thing
		dispatch_event(event.type, event);
	}
}


void sdl_osd_interface::osd_exit()
{
	osd_common_t::osd_exit();

	SDL_QuitSubSystem(SDL_INIT_VIDEO);
}


void sdl_osd_interface::output_oslog(const char *buffer)
{
	fputs(buffer, stderr);
}


void sdl_osd_interface::process_window_event(SDL_Event const &event)
{
	auto const window = window_from_id(event.window.windowID);

	if (!window)
	{
		// This condition may occur when the fullscreen toggle is used
		osd_printf_verbose("Skipped window event due to missing window param from SDL\n");
		return;
	}

	switch (event.window.event)
	{
	case SDL_WINDOWEVENT_MOVED:
		window->notify_changed();
		m_focus_window = window;
		break;

	case SDL_WINDOWEVENT_RESIZED:
#ifdef SDLMAME_LINUX
		/* FIXME: SDL2 sends some spurious resize events on Ubuntu
		* while in fullscreen mode. Ignore them for now.
		*/
		if (!window->fullscreen())
#endif
		{
			//printf("event data1,data2 %d x %d %ld\n", event.window.data1, event.window.data2, sizeof(SDL_Event));
			window->resize(event.window.data1, event.window.data2);
		}
		break;

	case SDL_WINDOWEVENT_ENTER:
		m_mouse_over_window = 1;
		break;

	case SDL_WINDOWEVENT_LEAVE:
		machine().ui_input().push_mouse_leave_event(window->target());
		m_mouse_over_window = 0;
		break;

	case SDL_WINDOWEVENT_FOCUS_GAINED:
		m_focus_window = window;
		machine().ui_input().push_window_focus_event(window->target());
		break;

	case SDL_WINDOWEVENT_FOCUS_LOST:
		if (window == m_focus_window)
			m_focus_window = nullptr;
		machine().ui_input().push_window_defocus_event(window->target());
		break;

	case SDL_WINDOWEVENT_CLOSE:
		machine().schedule_exit();
		break;
	}
}

void sdl_osd_interface::process_textinput_event(SDL_Event const &event)
{
	if (*event.text.text)
	{
		auto const window = focus_window(event.text);
		//printf("Focus window is %p - wl %p\n", window, osd_common_t::window_list().front().get());
		if (window != nullptr)
		{
			auto ptr = event.text.text;
			auto len = std::strlen(event.text.text);
			while (len)
			{
				char32_t ch;
				auto chlen = uchar_from_utf8(&ch, ptr, len);
				if (0 > chlen)
				{
					ch = 0x0fffd;
					chlen = 1;
				}
				ptr += chlen;
				len -= chlen;
				machine().ui_input().push_char_event(window->target(), ch);
			}
		}
	}
}


void sdl_osd_interface::check_osd_inputs()
{
	// check for toggling fullscreen mode (don't do this in debug mode)
	if (machine().ui_input().pressed(IPT_OSD_1) && !(machine().debug_flags & DEBUG_FLAG_OSD_ENABLED))
	{
		// destroy the renderers first so that the render module can bounce if it depends on having a window handle
		for (auto it = osd_common_t::window_list().rbegin(); osd_common_t::window_list().rend() != it; ++it)
			(*it)->renderer_reset();
		for (auto const &curwin : osd_common_t::window_list())
			dynamic_cast<sdl_window_info &>(*curwin).toggle_full_screen();
	}

	auto const &window = osd_common_t::window_list().front();

	if (USE_OPENGL)
	{
		//FIXME: on a per window basis
		if (machine().ui_input().pressed(IPT_OSD_5))
		{
			video_config.filter = !video_config.filter;
			machine().ui().popup_time(1, "Filter %s", video_config.filter? "enabled" : "disabled");
		}
	}

	if (machine().ui_input().pressed(IPT_OSD_6))
		dynamic_cast<sdl_window_info &>(*window).modify_prescale(-1);

	if (machine().ui_input().pressed(IPT_OSD_7))
		dynamic_cast<sdl_window_info &>(*window).modify_prescale(1);

	if (machine().ui_input().pressed(IPT_OSD_8))
		window->renderer().record();
}


template <typename T>
sdl_window_info *sdl_osd_interface::focus_window(T const &event) const
{
	// FIXME: SDL does not properly report the window for certain OS.
	if (false)
		return window_from_id(event.windowID);
	else
		return m_focus_window;
}
