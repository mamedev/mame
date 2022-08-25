// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont, Brad Hughes
//============================================================
//
//  input_sdl.cpp - SDL 2.0 implementation of MAME input routines
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//  SixAxis info: left analog is axes 0 & 1, right analog is axes 2 & 3,
//                analog L2 is axis 12 and analog L3 is axis 13
//
//============================================================

#include "input_module.h"
#include "modules/osdmodule.h"

#if defined(SDLMAME_SDL2)

// standard sdl header
#include <SDL2/SDL.h>
#include <cctype>
// ReSharper disable once CppUnusedIncludeDirective
#include <algorithm>
#include <cstddef>
#include <iterator>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>

// MAME headers
#include "emu.h"
#include "uiinput.h"
#include "strconv.h"
#include "unicode.h"

// MAMEOS headers
#include "input_common.h"
#include "../lib/osdobj_common.h"
#include "input_sdlcommon.h"
#include "../../sdl/osdsdl.h"
#include "../../sdl/window.h"

// winnt.h defines this
#ifdef DELETE
#undef DELETE
#endif


namespace {

// FIXME: sdl does not properly report the window for certain OS.
#define GET_FOCUS_WINDOW(ev) focus_window()
//#define GET_FOCUS_WINDOW(ev) window_from_id((ev)->windowID)

struct key_lookup_table
{
	int code;
	const char *name;
};

#define KE(x) { SDL_SCANCODE_ ## x, "SDL_SCANCODE_" #x },
#define KE8(A, B, C, D, E, F, G, H) KE(A) KE(B) KE(C) KE(D) KE(E) KE(F) KE(G) KE(H)
#define KE7(A, B, C, D, E, F, G) KE(A) KE(B) KE(C) KE(D) KE(E) KE(F) KE(G)
#define KE5(A, B, C, D, E) KE(A) KE(B) KE(C) KE(D) KE(E)
#define KE3(A, B, C) KE(A) KE(B) KE(C)

key_lookup_table sdl_lookup_table[] =
{
	KE(UNKNOWN)

	KE(A)
	KE(B)
	KE(C)
	KE(D)
	KE(E)
	KE(F)
	KE(G)
	KE(H)
	KE(I)
	KE(J)
	KE(K)
	KE(L)
	KE(M)
	KE(N)
	KE(O)
	KE(P)
	KE(Q)
	KE(R)
	KE(S)
	KE(T)
	KE(U)
	KE(V)
	KE(W)
	KE(X)
	KE(Y)
	KE(Z)

	KE(1)
	KE(2)
	KE(3)
	KE(4)
	KE(5)
	KE(6)
	KE(7)
	KE(8)
	KE(9)
	KE(0)

	KE(RETURN)
	KE(ESCAPE)
	KE(BACKSPACE)
	KE(TAB)
	KE(SPACE)

	KE(MINUS)
	KE(EQUALS)
	KE(LEFTBRACKET)
	KE(RIGHTBRACKET)
	KE(BACKSLASH)
	KE(NONUSHASH)
	KE(SEMICOLON)
	KE(APOSTROPHE)
	KE(GRAVE)
	KE(COMMA)
	KE(PERIOD)
	KE(SLASH)

	KE(CAPSLOCK)

	KE(F1)
	KE(F2)
	KE(F3)
	KE(F4)
	KE(F5)
	KE(F6)
	KE(F7)
	KE(F8)
	KE(F9)
	KE(F10)
	KE(F11)
	KE(F12)

	KE(PRINTSCREEN)
	KE(SCROLLLOCK)
	KE(PAUSE)
	KE(INSERT)
	KE(HOME)
	KE(PAGEUP)
	KE(DELETE)
	KE(END)
	KE(PAGEDOWN)
	KE(RIGHT)
	KE(LEFT)
	KE(DOWN)
	KE(UP)

	KE(NUMLOCKCLEAR)
	KE(KP_DIVIDE)
	KE(KP_MULTIPLY)
	KE(KP_MINUS)
	KE(KP_PLUS)
	KE(KP_ENTER)
	KE(KP_1)
	KE(KP_2)
	KE(KP_3)
	KE(KP_4)
	KE(KP_5)
	KE(KP_6)
	KE(KP_7)
	KE(KP_8)
	KE(KP_9)
	KE(KP_0)
	KE(KP_PERIOD)

	KE(NONUSBACKSLASH)
	KE(APPLICATION)
	KE(POWER)
	KE(KP_EQUALS)
	KE(F13)
	KE(F14)
	KE(F15)
	KE(F16)
	KE(F17)
	KE(F18)
	KE(F19)
	KE(F20)
	KE(F21)
	KE(F22)
	KE(F23)
	KE(F24)
	KE(EXECUTE)
	KE(HELP)
	KE(MENU)
	KE(SELECT)
	KE(STOP)
	KE(AGAIN)
	KE(UNDO)
	KE(CUT)
	KE(COPY)
	KE(PASTE)
	KE(FIND)
	KE(MUTE)
	KE(VOLUMEUP)
	KE(VOLUMEDOWN)
	KE(KP_COMMA)
	KE(KP_EQUALSAS400)

	KE(INTERNATIONAL1)
	KE(INTERNATIONAL2)
	KE(INTERNATIONAL3)
	KE(INTERNATIONAL4)
	KE(INTERNATIONAL5)
	KE(INTERNATIONAL6)
	KE(INTERNATIONAL7)
	KE(INTERNATIONAL8)
	KE(INTERNATIONAL9)
	KE(LANG1)
	KE(LANG2)
	KE(LANG3)
	KE(LANG4)
	KE(LANG5)
	KE(LANG6)
	KE(LANG7)
	KE(LANG8)
	KE(LANG9)

	KE(ALTERASE)
	KE(SYSREQ)
	KE(CANCEL)
	KE(CLEAR)
	KE(PRIOR)
	KE(RETURN2)
	KE(SEPARATOR)
	KE(OUT)
	KE(OPER)
	KE(CLEARAGAIN)
	KE(CRSEL)
	KE(EXSEL)

	KE(KP_00)
	KE(KP_000)
	KE(THOUSANDSSEPARATOR)
	KE(DECIMALSEPARATOR)
	KE(CURRENCYUNIT)
	KE(CURRENCYSUBUNIT)
	KE(KP_LEFTPAREN)
	KE(KP_RIGHTPAREN)
	KE(KP_LEFTBRACE)
	KE(KP_RIGHTBRACE)
	KE(KP_TAB)
	KE(KP_BACKSPACE)
	KE(KP_A)
	KE(KP_B)
	KE(KP_C)
	KE(KP_D)
	KE(KP_E)
	KE(KP_F)
	KE(KP_XOR)
	KE(KP_POWER)
	KE(KP_PERCENT)
	KE(KP_LESS)
	KE(KP_GREATER)
	KE(KP_AMPERSAND)
	KE(KP_DBLAMPERSAND)
	KE(KP_VERTICALBAR)
	KE(KP_DBLVERTICALBAR)
	KE(KP_COLON)
	KE(KP_HASH)
	KE(KP_SPACE)
	KE(KP_AT)
	KE(KP_EXCLAM)
	KE(KP_MEMSTORE)
	KE(KP_MEMRECALL)
	KE(KP_MEMCLEAR)
	KE(KP_MEMADD)
	KE(KP_MEMSUBTRACT)
	KE(KP_MEMMULTIPLY)
	KE(KP_MEMDIVIDE)
	KE(KP_PLUSMINUS)
	KE(KP_CLEAR)
	KE(KP_CLEARENTRY)
	KE(KP_BINARY)
	KE(KP_OCTAL)
	KE(KP_DECIMAL)
	KE(KP_HEXADECIMAL)

	KE(LCTRL)
	KE(LSHIFT)
	KE(LALT)
	KE(LGUI)
	KE(RCTRL)
	KE(RSHIFT)
	KE(RALT)
	KE(RGUI)

	KE(MODE)
	KE(AUDIONEXT)
	KE(AUDIOPREV)
	KE(AUDIOSTOP)
	KE(AUDIOPLAY)
	KE(AUDIOMUTE)
	KE(MEDIASELECT)
	KE(WWW)
	KE(MAIL)
	KE(CALCULATOR)
	KE(COMPUTER)
	KE(AC_SEARCH)
	KE(AC_HOME)
	KE(AC_BACK)
	KE(AC_FORWARD)
	KE(AC_STOP)
	KE(AC_REFRESH)
	KE(AC_BOOKMARKS)

	KE(BRIGHTNESSDOWN)
	KE(BRIGHTNESSUP)
	KE(DISPLAYSWITCH)
	KE(KBDILLUMTOGGLE)
	KE(KBDILLUMDOWN)
	KE(KBDILLUMUP)
	KE(EJECT)
	KE(SLEEP)

	KE(APP1)
	KE(APP2)


{
	-1, ""
}
};

//============================================================
//  lookup_sdl_code
//============================================================

int lookup_sdl_code(const char *scode)
{
	int i = 0;

	while (sdl_lookup_table[i].code >= 0)
	{
		if (!strcmp(scode, sdl_lookup_table[i].name))
			return sdl_lookup_table[i].code;
		i++;
	}
	return -1;
}

//============================================================
//  sdl_device
//============================================================

class sdl_device : public event_based_device<SDL_Event>
{
public:
	sdl_device(running_machine &machine, std::string &&name, std::string &&id, input_device_class devclass, input_module &module) :
		event_based_device(machine, std::move(name), std::move(id), devclass, module)
	{
	}

protected:
	std::shared_ptr<sdl_window_info> focus_window()
	{
		return sdl_event_manager::instance().focus_window();
	}
};

//============================================================
//  sdl_keyboard_device
//============================================================

class sdl_keyboard_device : public sdl_device
{
public:
	// state information for a keyboard
	struct keyboard_state
	{
		int32_t   state[0x3ff];         // must be int32_t!
		int8_t    oldkey[MAX_KEYS];
		int8_t    currkey[MAX_KEYS];
	};

	keyboard_state keyboard;

	sdl_keyboard_device(running_machine &machine, std::string &&name, std::string &&id, input_module &module) :
		sdl_device(machine, std::move(name), std::move(id), DEVICE_CLASS_KEYBOARD, module),
		keyboard({{0}})
	{
		#ifdef __APPLE__
		m_capslock_hack = false;
		machine.add_notifier(MACHINE_NOTIFY_FRAME, machine_notify_delegate(&sdl_keyboard_device::frame_callback, this));
		#endif
	}

	#ifdef __APPLE__
	unsigned m_capslock_hack;
	void frame_callback()
	{
		if (m_capslock_hack)
			if (--m_capslock_hack == 0)
				keyboard.state[SDL_SCANCODE_CAPSLOCK] = 0x00;
	}
	#endif

	void process_event(SDL_Event &sdlevent) override
	{
		switch (sdlevent.type)
		{
		case SDL_KEYDOWN:

			#ifdef __APPLE__
			if (sdlevent.key.keysym.scancode == SDL_SCANCODE_CAPSLOCK)
				m_capslock_hack = 2;
			#endif

			keyboard.state[sdlevent.key.keysym.scancode] = 0x80;
			if (sdlevent.key.keysym.sym < 0x20)
				machine().ui_input().push_char_event(osd_common_t::s_window_list.front()->target(), sdlevent.key.keysym.sym);
			else if (keyboard.state[SDL_SCANCODE_LCTRL] == 0x80 || keyboard.state[SDL_SCANCODE_RCTRL] == 0x80)
			{
				// SDL filters out control characters for text input, so they are decoded here
				if (sdlevent.key.keysym.sym >= 0x40 && sdlevent.key.keysym.sym < 0x7f)
					machine().ui_input().push_char_event(osd_common_t::s_window_list.front()->target(), sdlevent.key.keysym.sym & 0x1f);
				else if (keyboard.state[SDL_SCANCODE_LSHIFT] == 0x80 || keyboard.state[SDL_SCANCODE_RSHIFT] == 0x80)
				{
					if (sdlevent.key.keysym.sym == SDLK_6) // Ctrl-^ (RS)
						machine().ui_input().push_char_event(osd_common_t::s_window_list.front()->target(), 0x1e);
					else if (sdlevent.key.keysym.sym == SDLK_MINUS) // Ctrl-_ (US)
						machine().ui_input().push_char_event(osd_common_t::s_window_list.front()->target(), 0x1f);
				}
			}
			break;

		case SDL_KEYUP:
			#ifdef __APPLE__
			if (sdlevent.key.keysym.scancode == SDL_SCANCODE_CAPSLOCK)
				break;
			#endif

			keyboard.state[sdlevent.key.keysym.scancode] = 0x00;
			break;

		case SDL_TEXTINPUT:
			if (*sdlevent.text.text)
			{
				auto window = GET_FOCUS_WINDOW(&event.text);
				//printf("Focus window is %p - wl %p\n", window, osd_common_t::s_window_list);
				if (window != nullptr)
				{
					auto ptr = sdlevent.text.text;
					auto len = std::strlen(sdlevent.text.text);
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
			break;
		}
	}

	void reset() override
	{
		memset(&keyboard.state, 0, sizeof(keyboard.state));
		#ifdef __APPLE__
		m_capslock_hack = 0;
		#endif
	}
};

//============================================================
//  sdl_mouse_device
//============================================================

class sdl_mouse_device : public sdl_device
{
private:
	const std::chrono::milliseconds double_click_speed = std::chrono::milliseconds(250);
	std::chrono::system_clock::time_point last_click;
	int last_x;
	int last_y;

public:
	// state information for a mouse
	struct mouse_state
	{
		int32_t lX, lY;
		int32_t buttons[MAX_BUTTONS];
	};

	mouse_state mouse;

	sdl_mouse_device(running_machine &machine, std::string &&name, std::string &&id, input_module &module) :
		sdl_device(machine, std::move(name), std::move(id), DEVICE_CLASS_MOUSE, module),
		last_x(0),
		last_y(0),
		mouse({0})
	{
	}

	void reset() override
	{
		memset(&mouse, 0, sizeof(mouse));
	}

	void poll() override
	{
		mouse.lX = 0;
		mouse.lY = 0;
		sdl_device::poll();
	}

	virtual void process_event(SDL_Event &sdlevent) override
	{
		switch (sdlevent.type)
		{
		case SDL_MOUSEMOTION:
			mouse.lX += sdlevent.motion.xrel * osd::INPUT_RELATIVE_PER_PIXEL;
			mouse.lY += sdlevent.motion.yrel * osd::INPUT_RELATIVE_PER_PIXEL;

			{
				int cx = -1, cy = -1;
				auto window = GET_FOCUS_WINDOW(&sdlevent.motion);

				if (window != nullptr && window->xy_to_render_target(sdlevent.motion.x, sdlevent.motion.y, &cx, &cy))
					machine().ui_input().push_mouse_move_event(window->target(), cx, cy);
			}
			break;

		case SDL_MOUSEBUTTONDOWN:
			mouse.buttons[sdlevent.button.button - 1] = 0x80;
			//printf("But down %d %d %d %d %s\n", event.button.which, event.button.button, event.button.x, event.button.y, devinfo->name.c_str());
			if (sdlevent.button.button == 1)
			{
				int cx, cy;
				auto click = std::chrono::system_clock::now();
				auto window = GET_FOCUS_WINDOW(&sdlevent.button);
				if (window != nullptr && window->xy_to_render_target(sdlevent.button.x, sdlevent.button.y, &cx, &cy))
				{
					machine().ui_input().push_mouse_down_event(window->target(), cx, cy);

					// avoid overflow with std::chrono::time_point::min() by adding rather than subtracting
					if (click < last_click + double_click_speed
						&& (cx >= last_x - 4 && cx <= last_x + 4)
						&& (cy >= last_y - 4 && cy <= last_y + 4))
					{
						last_click = std::chrono::time_point<std::chrono::system_clock>::min();
						machine().ui_input().push_mouse_double_click_event(window->target(), cx, cy);
					}
					else
					{
						last_click = click;
						last_x = cx;
						last_y = cy;
					}
				}
			}

			else if (sdlevent.button.button == 3)
			{
				int cx, cy;
				auto window = GET_FOCUS_WINDOW(&sdlevent.button);

				if (window != nullptr && window->xy_to_render_target(sdlevent.button.x, sdlevent.button.y, &cx, &cy))
				{
					machine().ui_input().push_mouse_rdown_event(window->target(), cx, cy);
				}
			}
			break;

		case SDL_MOUSEBUTTONUP:
			mouse.buttons[sdlevent.button.button - 1] = 0;
			//printf("But up %d %d %d %d\n", event.button.which, event.button.button, event.button.x, event.button.y);

			if (sdlevent.button.button == 1)
			{
				int cx, cy;
				auto window = GET_FOCUS_WINDOW(&sdlevent.button);

				if (window != nullptr && window->xy_to_render_target(sdlevent.button.x, sdlevent.button.y, &cx, &cy))
				{
					machine().ui_input().push_mouse_up_event(window->target(), cx, cy);
				}
			}
			else if (sdlevent.button.button == 3)
			{
				int cx, cy;
				auto window = GET_FOCUS_WINDOW(&sdlevent.button);

				if (window != nullptr && window->xy_to_render_target(sdlevent.button.x, sdlevent.button.y, &cx, &cy))
				{
					machine().ui_input().push_mouse_rup_event(window->target(), cx, cy);
				}
			}
			break;

		case SDL_MOUSEWHEEL:
			auto window = GET_FOCUS_WINDOW(&sdlevent.wheel);
			if (window != nullptr)
				machine().ui_input().push_mouse_wheel_event(window->target(), 0, 0, sdlevent.wheel.y, 3);
			break;
		}
	}
};

//============================================================
//  sdl_joystick_device
//============================================================

class sdl_joystick_device : public sdl_device
{
public:
	// state information for a joystick
	struct sdl_joystick_state
	{
		int32_t axes[MAX_AXES];
		int32_t buttons[MAX_BUTTONS];
		int32_t hatsU[MAX_HATS], hatsD[MAX_HATS], hatsL[MAX_HATS], hatsR[MAX_HATS];
		int32_t balls[MAX_AXES];
	};

	struct sdl_api_state
	{
		SDL_Joystick *device = nullptr;
		SDL_Haptic *hapdevice = nullptr;
		SDL_JoystickID joystick_id;
		std::optional<std::string> serial;
	};

	sdl_joystick_state    joystick;
	sdl_api_state         sdl_state;

	sdl_joystick_device(running_machine &machine, std::string &&name, std::string &&id, input_module &module) :
		sdl_device(machine, std::move(name), std::move(id), DEVICE_CLASS_JOYSTICK, module),
		joystick({{0}})
	{
	}

	~sdl_joystick_device()
	{
		if (sdl_state.device)
		{
			if (sdl_state.hapdevice)
			{
				SDL_HapticClose(sdl_state.hapdevice);
				sdl_state.hapdevice = nullptr;
			}
			SDL_JoystickClose(sdl_state.device);
			sdl_state.device = nullptr;
		}
	}

	void reset() override
	{
		memset(&joystick, 0, sizeof(joystick));
	}

	void process_event(SDL_Event &sdlevent) override
	{
		switch (sdlevent.type)
		{
		case SDL_JOYAXISMOTION:
			joystick.axes[sdlevent.jaxis.axis] = (sdlevent.jaxis.value * 2);
			break;

		case SDL_JOYBALLMOTION:
			//printf("Ball %d %d\n", sdlevent.jball.xrel, sdlevent.jball.yrel);
			joystick.balls[sdlevent.jball.ball * 2] = sdlevent.jball.xrel * osd::INPUT_RELATIVE_PER_PIXEL;
			joystick.balls[sdlevent.jball.ball * 2 + 1] = sdlevent.jball.yrel * osd::INPUT_RELATIVE_PER_PIXEL;
			break;

		case SDL_JOYHATMOTION:
			joystick.hatsU[sdlevent.jhat.hat] = (sdlevent.jhat.value & SDL_HAT_UP) ? 0x80 : 0;
			joystick.hatsD[sdlevent.jhat.hat] = (sdlevent.jhat.value & SDL_HAT_DOWN) ? 0x80 : 0;
			joystick.hatsL[sdlevent.jhat.hat] = (sdlevent.jhat.value & SDL_HAT_LEFT) ? 0x80 : 0;
			joystick.hatsR[sdlevent.jhat.hat] = (sdlevent.jhat.value & SDL_HAT_RIGHT) ? 0x80 : 0;
			break;

		case SDL_JOYBUTTONDOWN:
		case SDL_JOYBUTTONUP:
			if (sdlevent.jbutton.button < MAX_BUTTONS)
				joystick.buttons[sdlevent.jbutton.button] = (sdlevent.jbutton.state == SDL_PRESSED) ? 0x80 : 0;
			break;

		case SDL_JOYDEVICEREMOVED:
			osd_printf_verbose("Joystick: %s [GUID %s] disconnected\n", name(), id());
			reset();
			if (sdl_state.device)
			{
				if (sdl_state.hapdevice)
				{
					SDL_HapticClose(sdl_state.hapdevice);
					sdl_state.hapdevice = nullptr;
				}
				SDL_JoystickClose(sdl_state.device);
				sdl_state.device = nullptr;
			}
			break;
		}
	}
};

class sdl_sixaxis_joystick_device : public sdl_joystick_device
{
public:
	sdl_sixaxis_joystick_device(running_machine &machine, std::string &&name, std::string &&id, input_module &module) :
		sdl_joystick_device(machine, std::move(name), std::move(id), module)
	{
	}

	void process_event(SDL_Event &sdlevent) override
	{
		switch (sdlevent.type)
		{
		case SDL_JOYAXISMOTION:
			{
				int const axis = sdlevent.jaxis.axis;
				if (axis <= 3)
				{
					joystick.axes[sdlevent.jaxis.axis] = (sdlevent.jaxis.value * 2);
				}
				else
				{
					int const magic = (sdlevent.jaxis.value / 2) + 16384;
					joystick.axes[sdlevent.jaxis.axis] = magic;
				}
			}
			break;

		default:
			// Call the base for other events
			sdl_joystick_device::process_event(sdlevent);
			break;
		}
	}
};

//============================================================
//  sdl_input_module
//============================================================

class sdl_input_module : public input_module_base, public sdl_event_subscriber
{
public:
	sdl_input_module(const char *type) : input_module_base(type, "sdl")
	{
	}

	void input_init(running_machine &machine) override
	{
		if (machine.debug_flags & DEBUG_FLAG_OSD_ENABLED)
		{
			osd_printf_warning("Debug Build: Disabling input grab for -debug\n");
			set_mouse_enabled(false);
		}
	}

	void exit() override
	{
		// unsubscribe for events
		sdl_event_manager::instance().unsubscribe(this);

		input_module_base::exit();
	}

	void before_poll(running_machine& machine) override
	{
		// Tell the event manager to process events and push them to the devices
		sdl_event_manager::instance().process_events(machine);
	}

	bool should_poll_devices(running_machine& machine) override
	{
		return sdl_event_manager::instance().has_focus() && input_enabled();
	}

	void handle_event(SDL_Event &sdlevent) override
	{
		// By default dispatch event to every device
		devicelist().for_each_device(
				[&sdlevent](auto device) {
					downcast<sdl_device*>(device)->queue_events(&sdlevent, 1);
				});
	}
};

//============================================================
//  sdl_keyboard_module
//============================================================

class sdl_keyboard_module : public sdl_input_module
{
	keyboard_trans_table * m_key_trans_table;
public:
	sdl_keyboard_module() :
		sdl_input_module(OSD_KEYBOARDINPUT_PROVIDER),
		m_key_trans_table(nullptr)
	{
	}

	void input_init(running_machine &machine) override
	{
		sdl_input_module::input_init(machine);

		static int const event_types[] = {
				int(SDL_KEYDOWN),
				int(SDL_KEYUP),
				int(SDL_TEXTINPUT) };

		sdl_event_manager::instance().subscribe(event_types, this);

		// Read our keymap and store a pointer to our table
		m_key_trans_table = sdlinput_read_keymap(machine);

		keyboard_trans_table& local_table = *m_key_trans_table;

		osd_printf_verbose("Keyboard: Start initialization\n");

		// SDL only has 1 keyboard add it now
		auto &devinfo = devicelist().create_device<sdl_keyboard_device>(machine, "System keyboard", "System keyboard", *this);

		// populate it
		for (int keynum = 0; local_table[keynum].mame_key != ITEM_ID_INVALID; keynum++)
		{
			input_item_id itemid = local_table[keynum].mame_key;

			// generate the default / modified name
			char defname[20];
			snprintf(defname, sizeof(defname) - 1, "%s", local_table[keynum].ui_name);

			devinfo.device()->add_item(defname, itemid, generic_button_get_state<std::int32_t>, &devinfo.keyboard.state[local_table[keynum].sdl_scancode]);
		}

		osd_printf_verbose("Keyboard: Registered %s\n", devinfo.name());
		osd_printf_verbose("Keyboard: End initialization\n");
	}

private:
	keyboard_trans_table *sdlinput_read_keymap(running_machine &machine)
	{
		keyboard_trans_table &default_table = keyboard_trans_table::instance();

		if (!machine.options().bool_value(SDLOPTION_KEYMAP))
			return &default_table;

		const char *const keymap_filename = downcast<sdl_options &>(machine.options()).keymap_file();
		osd_printf_verbose("Keymap: Start reading keymap_file %s\n", keymap_filename);

		FILE *const keymap_file = fopen(keymap_filename, "r");
		if (!keymap_file)
		{
			osd_printf_warning("Keymap: Unable to open keymap %s, using default\n", keymap_filename);
			return &default_table;
		}

		// Allocate a block of translation entries big enough to hold what's in the default table
		auto key_trans_entries = std::make_unique<key_trans_entry[]>(default_table.size());

		// copy the elements from the default table
		for (int i = 0; i < default_table.size(); i++)
			key_trans_entries[i] = default_table[i];

		// Allocate the trans table to be associated with the machine so we don't have to free it
		m_custom_table = std::make_unique<keyboard_trans_table>(std::move(key_trans_entries), default_table.size());

		int line = 1;
		int sdl2section = 0;
		while (!feof(keymap_file))
		{
			char buf[256];

			char *ret = fgets(buf, 255, keymap_file);
			if (ret && buf[0] != '\n' && buf[0] != '#')
			{
				buf[255] = 0;
				int len = strlen(buf);
				if (len && buf[len - 1] == '\n')
					buf[len - 1] = 0;
				if (strncmp(buf, "[SDL2]", 6) == 0)
				{
					sdl2section = 1;
				}
				else if (sdl2section == 1)
				{
					char mks[41] = {0};
					char sks[41] = {0};
					char kns[41] = {0};

					int n = sscanf(buf, "%40s %40s %40c\n", mks, sks, kns);
					if (n != 3)
						osd_printf_error("Keymap: Error on line %d : Expected 3 parameters, got %d\n", line, n);

					int index = default_table.lookup_mame_index(mks);
					int sk = lookup_sdl_code(sks);

					if (sk >= 0 && index >= 0)
					{
						key_trans_entry &entry = (*m_custom_table)[index];
						entry.sdl_scancode = sk;
						entry.ui_name = const_cast<char *>(m_ui_names.emplace_back(kns).c_str());
						osd_printf_verbose("Keymap: Mapped <%s> to <%s> with ui-text <%s>\n", sks, mks, kns);
					}
					else
					{
						osd_printf_error("Keymap: Error on line %d - %s key not found: %s\n", line, (sk<0) ? "sdl" : "mame", buf);
					}
				}
			}
			line++;
		}
		fclose(keymap_file);
		osd_printf_verbose("Keymap: Processed %d lines\n", line);

		return m_custom_table.get();
	}

	std::unique_ptr<keyboard_trans_table> m_custom_table;
	std::list<std::string> m_ui_names;
};

//============================================================
//  sdl_mouse_module
//============================================================

class sdl_mouse_module : public sdl_input_module
{
public:
	sdl_mouse_module() : sdl_input_module(OSD_MOUSEINPUT_PROVIDER)
	{
	}

	virtual void input_init(running_machine &machine) override
	{
		sdl_input_module::input_init(machine);

		static int const event_types[] = {
				int(SDL_MOUSEMOTION),
				int(SDL_MOUSEBUTTONDOWN),
				int(SDL_MOUSEBUTTONUP),
				int(SDL_MOUSEWHEEL) };

		sdl_event_manager::instance().subscribe(event_types, this);

		osd_printf_verbose("Mouse: Start initialization\n");

		// SDL currently only supports one mouse
		auto &devinfo = devicelist().create_device<sdl_mouse_device>(machine, "System mouse", "System mouse", *this);

		// add the axes
		devinfo.device()->add_item("X", ITEM_ID_XAXIS, generic_axis_get_state<std::int32_t>, &devinfo.mouse.lX);
		devinfo.device()->add_item("Y", ITEM_ID_YAXIS, generic_axis_get_state<std::int32_t>, &devinfo.mouse.lY);

		for (int button = 0; button < 4; button++)
		{
			input_item_id itemid = (input_item_id)(ITEM_ID_BUTTON1 + button);
			devinfo.device()->add_item(default_button_name(button), itemid, generic_button_get_state<std::int32_t>, &devinfo.mouse.buttons[button]);
		}

		osd_printf_verbose("Mouse: Registered %s\n", devinfo.name());
		osd_printf_verbose("Mouse: End initialization\n");
	}
};


//============================================================
//  sdl_joystick_module
//============================================================

class sdl_joystick_module : public sdl_input_module
{
private:
	bool m_initialized_joystick;
	bool m_initialized_haptic;
	bool m_sixaxis_mode;
public:
	sdl_joystick_module() :
		sdl_input_module(OSD_JOYSTICKINPUT_PROVIDER),
		m_initialized_joystick(false),
		m_initialized_haptic(false),
		m_sixaxis_mode(false)
	{
	}

	virtual void exit() override
	{
		sdl_input_module::exit();

		if (m_initialized_joystick)
		{
			SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
		}

		if (m_initialized_haptic)
		{
			SDL_QuitSubSystem(SDL_INIT_HAPTIC);
		}
	}

	virtual void input_init(running_machine &machine) override
	{
		SDL_SetHint(SDL_HINT_ACCELEROMETER_AS_JOYSTICK, "0");

		m_initialized_joystick = !SDL_InitSubSystem(SDL_INIT_JOYSTICK);
		if (!m_initialized_joystick)
		{
			osd_printf_error("Could not initialize SDL Joystick: %s.\n", SDL_GetError());
			return;
		}

		m_initialized_haptic = !SDL_InitSubSystem(SDL_INIT_HAPTIC);
		if (!m_initialized_haptic)
		{
			osd_printf_verbose("Could not initialize SDL Haptic subsystem: %s.\n", SDL_GetError());
		}

		sdl_input_module::input_init(machine);

		char tempname[512];

		m_sixaxis_mode = downcast<const sdl_options *>(options())->sixaxis();

		osd_printf_verbose("Joystick: Start initialization\n");
		for (int physical_stick = 0; physical_stick < SDL_NumJoysticks(); physical_stick++)
		{
			SDL_Joystick *const joy = SDL_JoystickOpen(physical_stick);
			char const *const name = SDL_JoystickName(joy);
			SDL_JoystickGUID guid = SDL_JoystickGetGUID(joy);
			char guid_str[256];
			guid_str[0] = '\0';
			SDL_JoystickGetGUIDString(guid, guid_str, sizeof(guid_str) - 1);
			char const *serial = nullptr;
#if SDL_VERSION_ATLEAST(2, 0, 14)
			serial = SDL_JoystickGetSerial(joy);
#endif
			std::string id(guid_str);
			if (serial)
				id.append(1, '-').append(serial);

			sdl_joystick_device *const devinfo = m_sixaxis_mode
					? &devicelist().create_device<sdl_sixaxis_joystick_device>(machine, name ? name : guid_str, guid_str, *this)
					: &devicelist().create_device<sdl_joystick_device>(machine, name ? name : guid_str, guid_str, *this);

			if (!devinfo)
			{
				SDL_JoystickClose(joy);
				continue;
			}

			devinfo->sdl_state.device = joy;
			devinfo->sdl_state.joystick_id = SDL_JoystickInstanceID(joy);
			devinfo->sdl_state.hapdevice = SDL_HapticOpenFromJoystick(joy);
			if (serial)
				devinfo->sdl_state.serial = serial;

			osd_printf_verbose("Joystick: %s [GUID %s] Vendor ID %04X, Product ID %04X, Revision %04X, Serial %s\n",
					name ? name : "<nullptr>",
					guid_str,
					SDL_JoystickGetVendor(joy),
					SDL_JoystickGetProduct(joy),
					SDL_JoystickGetProductVersion(joy),
					serial ? serial : "<nullptr>");
			osd_printf_verbose("Joystick:   ...  %d axes, %d buttons %d hats %d balls\n",
					SDL_JoystickNumAxes(joy),
					SDL_JoystickNumButtons(joy),
					SDL_JoystickNumHats(joy),
					SDL_JoystickNumBalls(joy));
			if (devinfo->sdl_state.hapdevice)
				osd_printf_verbose("Joystick:   ...  Has haptic capability\n");
			else
				osd_printf_verbose("Joystick:   ...  Does not have haptic capability\n");

			// loop over all axes
			for (int axis = 0; axis < SDL_JoystickNumAxes(joy); axis++)
			{
				input_item_id itemid;

				if (axis < INPUT_MAX_AXIS)
					itemid = (input_item_id)(ITEM_ID_XAXIS + axis);
				else if (axis < INPUT_MAX_AXIS + INPUT_MAX_ADD_ABSOLUTE)
					itemid = (input_item_id)(ITEM_ID_ADD_ABSOLUTE1 - INPUT_MAX_AXIS + axis);
				else
					itemid = ITEM_ID_OTHER_AXIS_ABSOLUTE;

				snprintf(tempname, sizeof(tempname), "A%d", axis + 1);
				devinfo->device()->add_item(tempname, itemid, generic_axis_get_state<std::int32_t>, &devinfo->joystick.axes[axis]);
			}

			// loop over all buttons
			if (SDL_JoystickNumButtons(joy) > MAX_BUTTONS)
				osd_printf_verbose("Joystick:   ...  Has %d buttons which exceeds supported %d buttons\n", SDL_JoystickNumButtons(joy), MAX_BUTTONS);
			for (int button = 0; (button < MAX_BUTTONS) && (button < SDL_JoystickNumButtons(joy)); button++)
			{
				input_item_id itemid;

				devinfo->joystick.buttons[button] = 0;

				if (button < INPUT_MAX_BUTTONS)
					itemid = (input_item_id)(ITEM_ID_BUTTON1 + button);
				else if (button < INPUT_MAX_BUTTONS + INPUT_MAX_ADD_SWITCH)
					itemid = (input_item_id)(ITEM_ID_ADD_SWITCH1 - INPUT_MAX_BUTTONS + button);
				else
					itemid = ITEM_ID_OTHER_SWITCH;

				devinfo->device()->add_item(default_button_name(button), itemid, generic_button_get_state<std::int32_t>, &devinfo->joystick.buttons[button]);
			}

			// loop over all hats
			for (int hat = 0; hat < SDL_JoystickNumHats(joy); hat++)
			{
				input_item_id itemid;

				snprintf(tempname, sizeof(tempname), "Hat %d Up", hat + 1);
				itemid = (input_item_id)((hat < INPUT_MAX_HATS) ? ITEM_ID_HAT1UP + 4 * hat : ITEM_ID_OTHER_SWITCH);
				devinfo->device()->add_item(tempname, itemid, generic_button_get_state<std::int32_t>, &devinfo->joystick.hatsU[hat]);
				snprintf(tempname, sizeof(tempname), "Hat %d Down", hat + 1);
				itemid = (input_item_id)((hat < INPUT_MAX_HATS) ? ITEM_ID_HAT1DOWN + 4 * hat : ITEM_ID_OTHER_SWITCH);
				devinfo->device()->add_item(tempname, itemid, generic_button_get_state<std::int32_t>, &devinfo->joystick.hatsD[hat]);
				snprintf(tempname, sizeof(tempname), "Hat %d Left", hat + 1);
				itemid = (input_item_id)((hat < INPUT_MAX_HATS) ? ITEM_ID_HAT1LEFT + 4 * hat : ITEM_ID_OTHER_SWITCH);
				devinfo->device()->add_item(tempname, itemid, generic_button_get_state<std::int32_t>, &devinfo->joystick.hatsL[hat]);
				snprintf(tempname, sizeof(tempname), "Hat %d Right", hat + 1);
				itemid = (input_item_id)((hat < INPUT_MAX_HATS) ? ITEM_ID_HAT1RIGHT + 4 * hat : ITEM_ID_OTHER_SWITCH);
				devinfo->device()->add_item(tempname, itemid, generic_button_get_state<std::int32_t>, &devinfo->joystick.hatsR[hat]);
			}

			// loop over all (track)balls
			for (int ball = 0; ball < SDL_JoystickNumBalls(joy); ball++)
			{
				int itemid;

				if (ball * 2 < INPUT_MAX_ADD_RELATIVE)
					itemid = ITEM_ID_ADD_RELATIVE1 + ball * 2;
				else
					itemid = ITEM_ID_OTHER_AXIS_RELATIVE;

				snprintf(tempname, sizeof(tempname), "R%d X", ball + 1);
				devinfo->device()->add_item(tempname, (input_item_id)itemid, generic_axis_get_state<std::int32_t>, &devinfo->joystick.balls[ball * 2]);
				snprintf(tempname, sizeof(tempname), "R%d Y", ball + 1);
				devinfo->device()->add_item(tempname, (input_item_id)(itemid + 1), generic_axis_get_state<std::int32_t>, &devinfo->joystick.balls[ball * 2 + 1]);
			}
		}

		static int const event_types[] = {
				int(SDL_JOYAXISMOTION),
				int(SDL_JOYBALLMOTION),
				int(SDL_JOYHATMOTION),
				int(SDL_JOYBUTTONDOWN),
				int(SDL_JOYBUTTONUP),
				int(SDL_JOYDEVICEADDED),
				int(SDL_JOYDEVICEREMOVED) };

		sdl_event_manager::instance().subscribe(event_types, this);

		osd_printf_verbose("Joystick: End initialization\n");
	}

	virtual void handle_event(SDL_Event &sdlevent) override
	{
		if (SDL_JOYDEVICEADDED == sdlevent.type)
		{
			SDL_Joystick *const joy = SDL_JoystickOpen(sdlevent.jdevice.which);
			if (joy)
			{
				SDL_JoystickGUID guid = SDL_JoystickGetGUID(joy);
				char guid_str[256];
				guid_str[0] = '\0';
				SDL_JoystickGetGUIDString(guid, guid_str, sizeof(guid_str) - 1);
				char const *serial = nullptr;
#if SDL_VERSION_ATLEAST(2, 0, 14)
				serial = SDL_JoystickGetSerial(joy);
#endif
				auto target_device = std::find_if(
						devicelist().begin(),
						devicelist().end(),
						[&guid_str, &serial] (auto const &device)
						{
							auto &devinfo = downcast<sdl_joystick_device &>(*device);
							return
									!devinfo.sdl_state.device &&
									(devinfo.id() == guid_str) &&
									((serial && devinfo.sdl_state.serial && (*devinfo.sdl_state.serial == serial)) || (!serial && !devinfo.sdl_state.serial));
						});
				if (devicelist().end() != target_device)
				{
					auto &devinfo = downcast<sdl_joystick_device &>(**target_device);
					devinfo.sdl_state.device = joy;
					devinfo.sdl_state.joystick_id = SDL_JoystickInstanceID(joy);
					devinfo.sdl_state.hapdevice = SDL_HapticOpenFromJoystick(joy);
					osd_printf_verbose("Joystick: %s [GUID %s] reconnected\n", devinfo.name(), guid_str);
				}
				else
				{
					SDL_JoystickClose(joy);
				}
			}
		}
		else
		{
			// Figure out which joystick this event id destined for
			sdl_joystick_device *const target_device = find_joystick(sdlevent.jdevice.which); // FIXME: this depends on SDL_JoystickID being the same size as Sint32

			// If we find a matching joystick, dispatch the event to the joystick
			if (target_device)
				target_device->queue_events(&sdlevent, 1);
		}
	}

private:
	sdl_joystick_device *find_joystick(SDL_JoystickID instance)
	{
		for (auto &ptr : devicelist())
		{
			sdl_joystick_device *const device = downcast<sdl_joystick_device *>(ptr.get());
			if (device->sdl_state.device && (device->sdl_state.joystick_id == instance))
				return device;
		}
		return nullptr;
	}
};

} // anonymous namespace

#else // defined(SDLMAME_SDL2)

#include "input_module.h"

MODULE_NOT_SUPPORTED(sdl_keyboard_module, OSD_KEYBOARDINPUT_PROVIDER, "sdl")
MODULE_NOT_SUPPORTED(sdl_mouse_module, OSD_MOUSEINPUT_PROVIDER, "sdl")
MODULE_NOT_SUPPORTED(sdl_joystick_module, OSD_JOYSTICKINPUT_PROVIDER, "sdl")

#endif // defined(SDLMAME_SDL2)

MODULE_DEFINITION(KEYBOARDINPUT_SDL, sdl_keyboard_module)
MODULE_DEFINITION(MOUSEINPUT_SDL, sdl_mouse_module)
MODULE_DEFINITION(JOYSTICKINPUT_SDL, sdl_joystick_module)
