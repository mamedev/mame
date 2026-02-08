// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont, Brad Hughes, Vas Crabb
//============================================================
//
//  input_sdl3.cpp - SDL 3 implementation of MAME input routines
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================

#include "input_module.h"

#include "modules/osdmodule.h"

#if defined(OSD_SDL) && defined(SDLMAME_SDL3)

#include "assignmenthelper.h"
#include "input_common.h"

#include "interface/inputseq.h"
#include "modules/lib/osdobj_common.h"
#include "osdsdl.h"
// emu
#include "inpttype.h"

// standard SDL header
#include <SDL3/SDL.h>

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstring>
#include <initializer_list>
#include <iterator>
#include <list>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>

// winnt.h defines this
#ifdef DELETE
#undef DELETE
#endif


namespace osd {

namespace {

char const *const CONTROLLER_AXIS_XBOX[]{
		"LSX",
		"LSY",
		"RSX",
		"RSY",
		"LT",
		"RT" };

char const *const CONTROLLER_AXIS_PS[]{
		"LSX",
		"LSY",
		"RSX",
		"RSY",
		"L2",
		"R2" };

char const *const CONTROLLER_AXIS_SWITCH[]{
		"LSX",
		"LSY",
		"RSX",
		"RSY",
		"ZL",
		"ZR" };

char const *const CONTROLLER_BUTTON_XBOX360[]{
		"A",
		"B",
		"X",
		"Y",
		"Back",
		"Guide",
		"Start",
		"LSB",
		"RSB",
		"LB",
		"RB",
		"D-pad Up",
		"D-pad Down",
		"D-pad Left",
		"D-pad Right",
		"Share",
		"P1",
		"P2",
		"P3",
		"P4",
		"Touchpad" };

char const *const CONTROLLER_BUTTON_XBOXONE[]{
		"A",
		"B",
		"X",
		"Y",
		"View",
		"Logo",
		"Menu",
		"LSB",
		"RSB",
		"LB",
		"RB",
		"D-pad Up",
		"D-pad Down",
		"D-pad Left",
		"D-pad Right",
		"Share",
		"P1",
		"P2",
		"P3",
		"P4",
		"Touchpad" };

char const *const CONTROLLER_BUTTON_PS3[]{
		"Cross",
		"Circle",
		"Square",
		"Triangle",
		"Select",
		"PS",
		"Start",
		"L3",
		"R3",
		"L1",
		"R1",
		"D-pad Up",
		"D-pad Down",
		"D-pad Left",
		"D-pad Right",
		"Mute",
		"P1",
		"P2",
		"P3",
		"P4",
		"Touchpad" };

char const *const CONTROLLER_BUTTON_PS4[]{
		"Cross",
		"Circle",
		"Square",
		"Triangle",
		"Share",
		"PS",
		"Options",
		"L3",
		"R3",
		"L1",
		"R1",
		"D-pad Up",
		"D-pad Down",
		"D-pad Left",
		"D-pad Right",
		"Mute",
		"P1",
		"P2",
		"P3",
		"P4",
		"Touchpad" };

char const *const CONTROLLER_BUTTON_PS5[]{
		"Cross",
		"Circle",
		"Square",
		"Triangle",
		"Create",
		"PS",
		"Options",
		"L3",
		"R3",
		"L1",
		"R1",
		"D-pad Up",
		"D-pad Down",
		"D-pad Left",
		"D-pad Right",
		"Mute",
		"P1",
		"P2",
		"P3",
		"P4",
		"Touchpad" };

char const *const CONTROLLER_BUTTON_SWITCH[]{
		"A",
		"B",
		"X",
		"Y",
		"-",
		"Home",
		"+",
		"LSB",
		"RSB",
		"L",
		"R",
		"D-pad Up",
		"D-pad Down",
		"D-pad Left",
		"D-pad Right",
		"Capture",
		"RSR",
		"LSL",
		"RSL",
		"LSR",
		"Touchpad" };

[[maybe_unused]] char const *const CONTROLLER_BUTTON_STADIA[]{
		"A",
		"B",
		"X",
		"Y",
		"Options",
		"Logo",
		"Menu",
		"L3",
		"R3",
		"L1",
		"R1",
		"D-pad Up",
		"D-pad Down",
		"D-pad Left",
		"D-pad Right",
		"Capture",
		"P1",
		"P2",
		"P3",
		"P4",
		"Touchpad" };

[[maybe_unused]] char const *const CONTROLLER_BUTTON_SHIELD[]{
		"A",
		"B",
		"X",
		"Y",
		"Back",
		"Logo",
		"Start",
		"LSB",
		"RSB",
		"LB",
		"RB",
		"D-pad Up",
		"D-pad Down",
		"D-pad Left",
		"D-pad Right",
		"Share",
		"P1",
		"P2",
		"P3",
		"P4",
		"Touchpad" };

struct key_lookup_table
{
	int code;
	const char *name;
};

#define KE(x) { SDL_SCANCODE_ ## x, "SDL_SCANCODE_" #x },

key_lookup_table const sdl_lookup_table[] =
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
	KE(MEDIA_NEXT_TRACK)
	KE(MEDIA_PREVIOUS_TRACK)
	KE(MEDIA_STOP)
	KE(MEDIA_PLAY)
	KE(MUTE)
	KE(MEDIA_SELECT)
	KE(AC_SEARCH)
	KE(AC_HOME)
	KE(AC_BACK)
	KE(AC_FORWARD)
	KE(AC_STOP)
	KE(AC_REFRESH)
	KE(AC_BOOKMARKS)

	KE(MEDIA_EJECT)
	KE(SLEEP)
};


//============================================================
//  lookup_sdl_code
//============================================================

int lookup_sdl_code(std::string_view scode)
{
	auto const found = std::find_if(
			std::begin(sdl_lookup_table),
			std::end(sdl_lookup_table),
			[&scode] (auto const &key) { return scode == key.name; });
	return (std::end(sdl_lookup_table) != found) ? found->code : -1;
}


//============================================================
//  sdl_device
//============================================================

using sdl_device = event_based_device<SDL_Event>;


//============================================================
//  sdl_keyboard_device
//============================================================

class sdl_keyboard_device : public sdl_device
{
public:
	sdl_keyboard_device(
			std::string &&name,
			std::string &&id,
			input_module &module,
			SDL_KeyboardID &kbdid,
			keyboard_trans_table const &trans_table) :
		sdl_device(std::move(name), std::move(id), module),
		m_keyboard_id(kbdid),
		m_trans_table(trans_table),
		m_keyboard({{0}}),
		m_capslock_pressed(std::chrono::steady_clock::time_point::min())
	{
	}

	virtual void poll(bool relative_reset) override
	{
		sdl_device::poll(relative_reset);

#ifdef SDL_PLATFORM_APPLE
		if (m_keyboard.state[SDL_SCANCODE_CAPSLOCK] && (std::chrono::steady_clock::now() > (m_capslock_pressed + std::chrono::milliseconds(30))))
			m_keyboard.state[SDL_SCANCODE_CAPSLOCK] = 0x00;
#endif
	}

	virtual void process_event(SDL_Event const &event) override
	{
		switch (event.type)
		{
		case SDL_EVENT_KEY_DOWN:
			// TODO: when we add proper multi-keyboard support.  Also below for the KEY_UP case.
			if (1) // event.key.which == m_keyboard_id)
			{
				if (event.key.scancode == SDL_SCANCODE_CAPSLOCK)
					m_capslock_pressed = std::chrono::steady_clock::now();

				m_keyboard.state[event.key.scancode] = 0x80;
			}
			break;

		case SDL_EVENT_KEY_UP:
			if (1) // event.key.which == m_keyboard_id)
			{
#ifdef SDL_PLATFORM_APPLE
				if (event.key.scancode == SDL_SCANCODE_CAPSLOCK)
					break;
#endif

				m_keyboard.state[event.key.scancode] = 0x00;
			}
			break;
		}
	}

	virtual void reset() override
	{
		sdl_device::reset();
		memset(&m_keyboard.state, 0, sizeof(m_keyboard.state));
		m_capslock_pressed = std::chrono::steady_clock::time_point::min();
	}

	virtual void configure(input_device &device) override
	{
		// populate it
		for (int keynum = 0; m_trans_table[keynum].mame_key != ITEM_ID_INVALID; keynum++)
		{
			input_item_id itemid = m_trans_table[keynum].mame_key;
			device.add_item(
					m_trans_table[keynum].ui_name,
					std::string_view(),
					itemid,
					generic_button_get_state<s32>,
					&m_keyboard.state[m_trans_table[keynum].sdl_scancode]);
		}
	}

private:
	// state information for a keyboard
	struct keyboard_state
	{
		s32 state[0x3ff];         // must be s32!
		s8  oldkey[MAX_KEYS];
		s8  currkey[MAX_KEYS];
	};

	SDL_KeyboardID m_keyboard_id;
	keyboard_trans_table const &m_trans_table;
	keyboard_state m_keyboard;
	std::chrono::steady_clock::time_point m_capslock_pressed;
};


//============================================================
//  sdl_mouse_device_base
//============================================================

class sdl_mouse_device_base : public sdl_device
{
public:
	virtual void poll(bool relative_reset) override
	{
		sdl_device::poll(relative_reset);

		if (relative_reset)
		{
			m_mouse.lV = std::exchange(m_v, 0);
			m_mouse.lH = std::exchange(m_h, 0);
		}
	}

	virtual void reset() override
	{
		sdl_device::reset();
		memset(&m_mouse, 0, sizeof(m_mouse));
		m_v = m_h = 0;
	}

protected:
	// state information for a mouse
	struct mouse_state
	{
		s32 lX, lY, lV, lH;
		s32 buttons[MAX_BUTTONS];
	};

	sdl_mouse_device_base(std::string &&name, std::string &&id, input_module &module) :
		sdl_device(std::move(name), std::move(id), module),
		m_mouse({0}),
		m_v(0),
		m_h(0)
	{
	}

	void add_common_items(input_device &device, unsigned buttons)
	{
		// add horizontal and vertical axes - relative for a mouse or absolute for a gun
		device.add_item(
				"X",
				std::string_view(),
				ITEM_ID_XAXIS,
				generic_axis_get_state<s32>,
				&m_mouse.lX);
		device.add_item(
				"Y",
				std::string_view(),
				ITEM_ID_YAXIS,
				generic_axis_get_state<s32>,
				&m_mouse.lY);

		// add buttons
		for (int button = 0; button < buttons; button++)
		{
			input_item_id itemid = (input_item_id)(ITEM_ID_BUTTON1 + button);
			int const offset = button ^ (((1 == button) || (2 == button)) ? 3 : 0);
			device.add_item(
					default_button_name(button),
					std::string_view(),
					itemid,
					generic_button_get_state<s32>,
					&m_mouse.buttons[offset]);
		}
	}

	mouse_state m_mouse;
	s32 m_v, m_h;
};


//============================================================
//  sdl_mouse_device
//============================================================

class sdl_mouse_device : public sdl_mouse_device_base
{
public:
	sdl_mouse_device(std::string &&name, std::string &&id, input_module &module) :
		sdl_mouse_device_base(std::move(name), std::move(id), module),
		m_x(0),
		m_y(0)
	{
	}

	virtual void poll(bool relative_reset) override
	{
		sdl_mouse_device_base::poll(relative_reset);

		if (relative_reset)
		{
			m_mouse.lX = std::exchange(m_x, 0);
			m_mouse.lY = std::exchange(m_y, 0);
		}
	}

	virtual void reset() override
	{
		sdl_mouse_device_base::reset();
		m_x = m_y = 0;
	}

	virtual void configure(input_device &device) override
	{
		add_common_items(device, 5);

		// add scroll axes
		device.add_item(
				"Scroll V",
				std::string_view(),
				ITEM_ID_ZAXIS,
				generic_axis_get_state<s32>,
				&m_mouse.lV);
		device.add_item(
				"Scroll H",
				std::string_view(),
				ITEM_ID_RZAXIS,
				generic_axis_get_state<s32>,
				&m_mouse.lH);
	}

	virtual void process_event(SDL_Event const &event) override
	{
		switch (event.type)
		{
		case SDL_EVENT_MOUSE_MOTION:
			m_x += event.motion.xrel * input_device::RELATIVE_PER_PIXEL;
			m_y += event.motion.yrel * input_device::RELATIVE_PER_PIXEL;
			break;

		case SDL_EVENT_MOUSE_BUTTON_DOWN:
			m_mouse.buttons[event.button.button - 1] = 0x80;
			break;

		case SDL_EVENT_MOUSE_BUTTON_UP:
			m_mouse.buttons[event.button.button - 1] = 0;
			break;

		case SDL_EVENT_MOUSE_WHEEL:
			// adjust SDL 1-per-click to match Win32 120-per-click
			m_v += std::lround(event.wheel.integer_y * 120 * input_device::RELATIVE_PER_PIXEL);
			m_h += std::lround(event.wheel.integer_x * 120 * input_device::RELATIVE_PER_PIXEL);
			break;
		}
	}

private:
	s32 m_x, m_y;
};


//============================================================
//  sdl_lightgun_device
//============================================================

class sdl_lightgun_device : public sdl_mouse_device_base
{
public:
	sdl_lightgun_device(std::string &&name, std::string &&id, input_module &module) :
		sdl_mouse_device_base(std::move(name), std::move(id), module),
		m_x(0),
		m_y(0),
		m_window(0)
	{
	}

	virtual void poll(bool relative_reset) override
	{
		sdl_mouse_device_base::poll(relative_reset);

		SDL_Window *const win(m_window ? SDL_GetWindowFromID(m_window) : nullptr);
		if (win)
		{
			int w, h;
			SDL_GetWindowSize(win, &w, &h);
			m_mouse.lX = normalize_absolute_axis(m_x, 0, w - 1);
			m_mouse.lY = normalize_absolute_axis(m_y, 0, h - 1);
		}
		else
		{
			m_mouse.lX = 0;
			m_mouse.lY = 0;
		}
	}

	virtual void reset() override
	{
		sdl_mouse_device_base::reset();
		m_x = m_y = 0;
		m_window = 0;
	}

	virtual void configure(input_device &device) override
	{
		add_common_items(device, 5);

		// add scroll axes
		device.add_item(
				"Scroll V",
				std::string_view(),
				ITEM_ID_ADD_RELATIVE1,
				generic_axis_get_state<s32>,
				&m_mouse.lV);
		device.add_item(
				"Scroll H",
				std::string_view(),
				ITEM_ID_ADD_RELATIVE2,
				generic_axis_get_state<s32>,
				&m_mouse.lH);
	}

	virtual void process_event(SDL_Event const &event) override
	{
		switch (event.type)
		{
		case SDL_EVENT_MOUSE_MOTION:
			m_x = event.motion.x;
			m_y = event.motion.y;
			m_window = event.motion.windowID;
			break;

		case SDL_EVENT_MOUSE_BUTTON_DOWN:
			m_mouse.buttons[event.button.button - 1] = 0x80;
			m_x = event.button.x;
			m_y = event.button.y;
			m_window = event.button.windowID;
			break;

		case SDL_EVENT_MOUSE_BUTTON_UP:
			m_mouse.buttons[event.button.button - 1] = 0;
			m_x = event.button.x;
			m_y = event.button.y;
			m_window = event.button.windowID;
			break;

		case SDL_EVENT_MOUSE_WHEEL:
			// adjust SDL 1-per-click to match Win32 120-per-click
			m_v += std::lround(event.wheel.integer_y * 120 * input_device::RELATIVE_PER_PIXEL);
			m_h += std::lround(event.wheel.integer_x * 120 * input_device::RELATIVE_PER_PIXEL);
			break;

		case SDL_EVENT_WINDOW_MOUSE_LEAVE:
			if (event.window.windowID == m_window)
				m_window = 0;
			break;
		}
	}

private:
	s32 m_x, m_y;
	u32 m_window;
};


//============================================================
//  sdl_dual_lightgun_device
//============================================================

class sdl_dual_lightgun_device : public sdl_mouse_device_base
{
public:
	sdl_dual_lightgun_device(std::string &&name, std::string &&id, input_module &module, u8 index) :
		sdl_mouse_device_base(std::move(name), std::move(id), module),
		m_index(index)
	{
	}

	virtual void configure(input_device &device) override
	{
		add_common_items(device, 2);
	}

	virtual void process_event(SDL_Event const &event) override
	{
		switch (event.type)
		{
		case SDL_EVENT_MOUSE_BUTTON_DOWN:
			{
				SDL_Window *const win(SDL_GetWindowFromID(event.button.windowID));
				u8 const button = translate_button(event);
				if (win && ((button / 2) == m_index))
				{
					int w, h;
					SDL_GetWindowSize(win, &w, &h);
					m_mouse.buttons[(button & 1) << 1] = 0x80;
					m_mouse.lX = normalize_absolute_axis(event.button.x, 0, w - 1);
					m_mouse.lY = normalize_absolute_axis(event.button.y, 0, h - 1);
				}
			}
			break;

		case SDL_EVENT_MOUSE_BUTTON_UP:
			{
				u8 const button = translate_button(event);
				if ((button / 2) == m_index)
					m_mouse.buttons[(button & 1) << 1] = 0;
			}
			break;
		}
	}

private:
	static u8 translate_button(SDL_Event const &event)
	{
		u8 const index(event.button.button - 1);
		return index ^ (((1 == index) || (2 == index)) ? 3 : 0);
	}

	u8 const m_index;
};


//============================================================
//  sdl_joystick_device_base
//============================================================

class sdl_joystick_device_base : public sdl_device, protected joystick_assignment_helper
{
public:
	std::optional<std::string> const &serial() const { return m_serial; }
	SDL_JoystickID instance() const { return m_instance; }

	bool is_instance(SDL_JoystickID instance) const { return m_instance == instance; }

	bool reconnect_match(std::string_view g, char const *s) const
	{
		return
				(0 > m_instance) &&
				(id() == g) &&
				((s && serial() && (*serial() == s)) || (!s && !serial()));
	}

protected:
	sdl_joystick_device_base(
			std::string &&name,
			std::string &&id,
			input_module &module,
			char const *serial) :
		sdl_device(std::move(name), std::move(id), module),
		m_instance(-1)
	{
		if (serial)
			m_serial = serial;
	}

	void set_instance(SDL_JoystickID instance)
	{
		assert(0 > m_instance);
		assert(0 <= instance);

		m_instance = instance;
	}

	void clear_instance()
	{
		m_instance = -1;
	}

private:
	std::optional<std::string> m_serial;
	SDL_JoystickID m_instance;
};


//============================================================
//  sdl_joystick_device
//============================================================

class sdl_joystick_device : public sdl_joystick_device_base
{
public:
	sdl_joystick_device(
			std::string &&name,
			std::string &&id,
			input_module &module,
			SDL_Joystick *joy,
			char const *serial) :
		sdl_joystick_device_base(
				std::move(name),
				std::move(id),
				module,
				serial),
		m_joystick({{0}}),
		m_joydevice(joy),
		m_hapdevice(SDL_OpenHapticFromJoystick(joy))
	{
		set_instance(SDL_GetJoystickID(joy));
	}

	virtual void configure(input_device &device) override
	{
		input_device::assignment_vector assignments;
		char tempname[32];

		int const axiscount = SDL_GetNumJoystickAxes(m_joydevice);
		int const buttoncount = SDL_GetNumJoystickButtons(m_joydevice);
		int const hatcount = SDL_GetNumJoystickHats(m_joydevice);
		int const ballcount = SDL_GetNumJoystickBalls(m_joydevice);

		// loop over all axes
		input_item_id axisactual[MAX_AXES];
		for (int axis = 0; (axis < MAX_AXES) && (axis < axiscount); axis++)
		{
			input_item_id itemid;

			if (axis < INPUT_MAX_AXIS)
				itemid = input_item_id(ITEM_ID_XAXIS + axis);
			else if (axis < (INPUT_MAX_AXIS + INPUT_MAX_ADD_ABSOLUTE))
				itemid = input_item_id(ITEM_ID_ADD_ABSOLUTE1 + axis - INPUT_MAX_AXIS);
			else
				itemid = ITEM_ID_OTHER_AXIS_ABSOLUTE;

			snprintf(tempname, sizeof(tempname), "A%d", axis + 1);
			axisactual[axis] = device.add_item(
					tempname,
					std::string_view(),
					itemid,
					generic_axis_get_state<s32>,
					&m_joystick.axes[axis]);
		}

		// loop over all buttons
		for (int button = 0; (button < MAX_BUTTONS) && (button < buttoncount); button++)
		{
			input_item_id itemid;

			m_joystick.buttons[button] = 0;

			if (button < INPUT_MAX_BUTTONS)
				itemid = input_item_id(ITEM_ID_BUTTON1 + button);
			else if (button < INPUT_MAX_BUTTONS + INPUT_MAX_ADD_SWITCH)
				itemid = input_item_id(ITEM_ID_ADD_SWITCH1 + button - INPUT_MAX_BUTTONS);
			else
				itemid = ITEM_ID_OTHER_SWITCH;

			input_item_id const actual = device.add_item(
					default_button_name(button),
					std::string_view(),
					itemid,
					generic_button_get_state<s32>,
					&m_joystick.buttons[button]);

			// there are sixteen action button types
			if (button < 16)
			{
				input_seq const seq(make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, actual));
				assignments.emplace_back(ioport_type(IPT_BUTTON1 + button), SEQ_TYPE_STANDARD, seq);

				// assign the first few buttons to UI actions and pedals
				switch (button)
				{
				case 0:
					assignments.emplace_back(IPT_PEDAL, SEQ_TYPE_INCREMENT, seq);
					assignments.emplace_back(IPT_UI_SELECT, SEQ_TYPE_STANDARD, seq);
					break;
				case 1:
					assignments.emplace_back(IPT_PEDAL2, SEQ_TYPE_INCREMENT, seq);
					assignments.emplace_back((3 > buttoncount) ? IPT_UI_CLEAR : IPT_UI_BACK, SEQ_TYPE_STANDARD, seq);
					break;
				case 2:
					assignments.emplace_back(IPT_PEDAL3, SEQ_TYPE_INCREMENT, seq);
					assignments.emplace_back(IPT_UI_CLEAR, SEQ_TYPE_STANDARD, seq);
					break;
				case 3:
					assignments.emplace_back(IPT_UI_HELP, SEQ_TYPE_STANDARD, seq);
					break;
				}
			}
		}

		// loop over all hats
		input_item_id hatactual[MAX_HATS][4];
		for (int hat = 0; (hat < MAX_HATS) && (hat < hatcount); hat++)
		{
			input_item_id itemid;

			snprintf(tempname, sizeof(tempname), "Hat %d Up", hat + 1);
			itemid = input_item_id((hat < INPUT_MAX_HATS) ? ITEM_ID_HAT1UP + (4 * hat) : ITEM_ID_OTHER_SWITCH);
			hatactual[hat][0] = device.add_item(
					tempname,
					std::string_view(),
					itemid,
					generic_button_get_state<s32>,
					&m_joystick.hatsU[hat]);

			snprintf(tempname, sizeof(tempname), "Hat %d Down", hat + 1);
			itemid = input_item_id((hat < INPUT_MAX_HATS) ? ITEM_ID_HAT1DOWN + (4 * hat) : ITEM_ID_OTHER_SWITCH);
			hatactual[hat][1] = device.add_item(
					tempname,
					std::string_view(),
					itemid,
					generic_button_get_state<s32>,
					&m_joystick.hatsD[hat]);

			snprintf(tempname, sizeof(tempname), "Hat %d Left", hat + 1);
			itemid = input_item_id((hat < INPUT_MAX_HATS) ? ITEM_ID_HAT1LEFT + (4 * hat) : ITEM_ID_OTHER_SWITCH);
			hatactual[hat][2] = device.add_item(
					tempname,
					std::string_view(),
					itemid,
					generic_button_get_state<s32>,
					&m_joystick.hatsL[hat]);

			snprintf(tempname, sizeof(tempname), "Hat %d Right", hat + 1);
			itemid = input_item_id((hat < INPUT_MAX_HATS) ? ITEM_ID_HAT1RIGHT + (4 * hat) : ITEM_ID_OTHER_SWITCH);
			hatactual[hat][3] = device.add_item(
					tempname,
					std::string_view(),
					itemid,
					generic_button_get_state<s32>,
					&m_joystick.hatsR[hat]);
		}

		// loop over all (track)balls
		for (int ball = 0; (ball < (MAX_AXES / 2)) && (ball < ballcount); ball++)
		{
			int itemid;

			if (ball * 2 < INPUT_MAX_ADD_RELATIVE)
				itemid = ITEM_ID_ADD_RELATIVE1 + ball * 2;
			else
				itemid = ITEM_ID_OTHER_AXIS_RELATIVE;

			snprintf(tempname, sizeof(tempname), "R%d X", ball + 1);
			input_item_id const xactual = device.add_item(
					tempname,
					std::string_view(),
					input_item_id(itemid),
					generic_axis_get_state<s32>,
					&m_joystick.balls[ball * 2]);

			snprintf(tempname, sizeof(tempname), "R%d Y", ball + 1);
			input_item_id const yactual = device.add_item(
					tempname,
					std::string_view(),
					input_item_id(itemid + 1),
					generic_axis_get_state<s32>,
					&m_joystick.balls[ball * 2 + 1]);

			if (0 == ball)
			{
				// assign the first trackball to dial, trackball, mouse and lightgun inputs
				input_seq const xseq(make_code(ITEM_CLASS_RELATIVE, ITEM_MODIFIER_NONE, xactual));
				input_seq const yseq(make_code(ITEM_CLASS_RELATIVE, ITEM_MODIFIER_NONE, yactual));
				assignments.emplace_back(IPT_DIAL,        SEQ_TYPE_STANDARD, xseq);
				assignments.emplace_back(IPT_DIAL_V,      SEQ_TYPE_STANDARD, yseq);
				assignments.emplace_back(IPT_TRACKBALL_X, SEQ_TYPE_STANDARD, xseq);
				assignments.emplace_back(IPT_TRACKBALL_Y, SEQ_TYPE_STANDARD, yseq);
				assignments.emplace_back(IPT_LIGHTGUN_X,  SEQ_TYPE_STANDARD, xseq);
				assignments.emplace_back(IPT_LIGHTGUN_Y,  SEQ_TYPE_STANDARD, yseq);
				assignments.emplace_back(IPT_MOUSE_X,     SEQ_TYPE_STANDARD, xseq);
				assignments.emplace_back(IPT_MOUSE_Y,     SEQ_TYPE_STANDARD, yseq);
				if (2 > axiscount)
				{
					// use it for joystick inputs if axes are limited
					assignments.emplace_back(IPT_AD_STICK_X, SEQ_TYPE_STANDARD, xseq);
					assignments.emplace_back(IPT_AD_STICK_Y, SEQ_TYPE_STANDARD, yseq);
				}
				else
				{
					// use for non-centring throttle control
					assignments.emplace_back(IPT_AD_STICK_Z, SEQ_TYPE_STANDARD, yseq);
				}
			}
			else if ((1 == ball) && (2 > axiscount))
			{
				// provide a non-centring throttle control
				input_seq const yseq(make_code(ITEM_CLASS_RELATIVE, ITEM_MODIFIER_NONE, yactual));
				assignments.emplace_back(IPT_AD_STICK_Z, SEQ_TYPE_STANDARD, yseq);
			}
		}

		// set up default assignments for axes and hats
		add_directional_assignments(
				assignments,
				(1 <= axiscount) ? axisactual[0] : ITEM_ID_INVALID, // assume first axis is X
				(2 <= axiscount) ? axisactual[1] : ITEM_ID_INVALID, // assume second axis is Y
				(1 <= hatcount) ? hatactual[0][2] : ITEM_ID_INVALID,
				(1 <= hatcount) ? hatactual[0][3] : ITEM_ID_INVALID,
				(1 <= hatcount) ? hatactual[0][0] : ITEM_ID_INVALID,
				(1 <= hatcount) ? hatactual[0][1] : ITEM_ID_INVALID);
		if (2 <= axiscount)
		{
			// put pedals on the last of the second, third or fourth axis
			input_item_id const pedalitem = axisactual[(std::min)(axiscount, 4) - 1];
			assignments.emplace_back(
					IPT_PEDAL,
					SEQ_TYPE_STANDARD,
					input_seq(make_code(ITEM_CLASS_ABSOLUTE, ITEM_MODIFIER_NEG, pedalitem)));
			assignments.emplace_back(
					IPT_PEDAL2,
					SEQ_TYPE_STANDARD,
					input_seq(make_code(ITEM_CLASS_ABSOLUTE, ITEM_MODIFIER_POS, pedalitem)));
		}
		if (3 <= axiscount)
		{
			// assign X/Y to one of the twin sticks
			assignments.emplace_back(
					(4 <= axiscount) ? IPT_JOYSTICKLEFT_LEFT : IPT_JOYSTICKRIGHT_LEFT,
					SEQ_TYPE_STANDARD,
					input_seq(make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_LEFT, axisactual[0])));
			assignments.emplace_back(
					(4 <= axiscount) ? IPT_JOYSTICKLEFT_RIGHT : IPT_JOYSTICKRIGHT_RIGHT,
					SEQ_TYPE_STANDARD,
					input_seq(make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_RIGHT, axisactual[0])));
			assignments.emplace_back(
					(4 <= axiscount) ? IPT_JOYSTICKLEFT_UP : IPT_JOYSTICKRIGHT_UP,
					SEQ_TYPE_STANDARD,
					input_seq(make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_UP, axisactual[1])));
			assignments.emplace_back(
					(4 <= axiscount) ? IPT_JOYSTICKLEFT_DOWN : IPT_JOYSTICKRIGHT_DOWN,
					SEQ_TYPE_STANDARD,
					input_seq(make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_DOWN, axisactual[1])));

			// use third or fourth axis for Z
			input_seq const seq(make_code(ITEM_CLASS_ABSOLUTE, ITEM_MODIFIER_NONE, axisactual[(std::min)(axiscount, 4) - 1]));
			assignments.emplace_back(IPT_AD_STICK_Z, SEQ_TYPE_STANDARD, seq);

			// use this for focus next/previous to make system selection menu practical to navigate
			input_seq const upseq(make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_NEG, axisactual[2]));
			input_seq const downseq(make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_POS, axisactual[2]));
			assignments.emplace_back(IPT_UI_FOCUS_PREV, SEQ_TYPE_STANDARD, upseq);
			assignments.emplace_back(IPT_UI_FOCUS_NEXT, SEQ_TYPE_STANDARD, downseq);
			if (4 <= axiscount)
			{
				// use for zoom as well if there's another axis to use for previous/next group
				assignments.emplace_back(IPT_UI_ZOOM_IN, SEQ_TYPE_STANDARD, downseq);
				assignments.emplace_back(IPT_UI_ZOOM_OUT, SEQ_TYPE_STANDARD, upseq);
			}

			// use this for twin sticks, too
			assignments.emplace_back((4 <= axiscount) ? IPT_JOYSTICKRIGHT_LEFT : IPT_JOYSTICKLEFT_UP, SEQ_TYPE_STANDARD, upseq);
			assignments.emplace_back((4 <= axiscount) ? IPT_JOYSTICKRIGHT_RIGHT : IPT_JOYSTICKLEFT_DOWN, SEQ_TYPE_STANDARD, downseq);

			// put previous/next group on the last of the third or fourth axis
			input_item_id const groupitem = axisactual[(std::min)(axiscount, 4) - 1];
			assignments.emplace_back(
					IPT_UI_PREV_GROUP,
					SEQ_TYPE_STANDARD,
					input_seq(make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_NEG, groupitem)));
			assignments.emplace_back(
					IPT_UI_NEXT_GROUP,
					SEQ_TYPE_STANDARD,
					input_seq(make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_POS, groupitem)));
		}
		if (4 <= axiscount)
		{
			// use this for twin sticks
			input_seq const upseq(make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_NEG, axisactual[3]));
			input_seq const downseq(make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_POS, axisactual[3]));
			assignments.emplace_back(IPT_JOYSTICKRIGHT_UP, SEQ_TYPE_STANDARD, upseq);
			assignments.emplace_back(IPT_JOYSTICKRIGHT_DOWN, SEQ_TYPE_STANDARD, downseq);
		}

		// set default assignments
		device.set_default_assignments(std::move(assignments));
	}

	~sdl_joystick_device()
	{
		close_device();
	}

	virtual void reset() override
	{
		sdl_joystick_device_base::reset();
		clear_buffer();
	}

	virtual void process_event(SDL_Event const &event) override
	{
		if (!m_joydevice)
			return;

		switch (event.type)
		{
		case SDL_EVENT_JOYSTICK_AXIS_MOTION:
			if (event.jaxis.axis < MAX_AXES)
				m_joystick.axes[event.jaxis.axis] = (event.jaxis.value * 2);
			break;

		case SDL_EVENT_JOYSTICK_BALL_MOTION:
			//printf("Ball %d %d\n", event.jball.xrel, event.jball.yrel);
			if (event.jball.ball < (MAX_AXES / 2))
			{
				m_joystick.balls[event.jball.ball * 2] = event.jball.xrel * input_device::RELATIVE_PER_PIXEL;
				m_joystick.balls[event.jball.ball * 2 + 1] = event.jball.yrel * input_device::RELATIVE_PER_PIXEL;
			}
			break;

		case SDL_EVENT_JOYSTICK_HAT_MOTION:
			if (event.jhat.hat < MAX_HATS)
			{
				m_joystick.hatsU[event.jhat.hat] = (event.jhat.value & SDL_HAT_UP) ? 0x80 : 0;
				m_joystick.hatsD[event.jhat.hat] = (event.jhat.value & SDL_HAT_DOWN) ? 0x80 : 0;
				m_joystick.hatsL[event.jhat.hat] = (event.jhat.value & SDL_HAT_LEFT) ? 0x80 : 0;
				m_joystick.hatsR[event.jhat.hat] = (event.jhat.value & SDL_HAT_RIGHT) ? 0x80 : 0;
			}
			break;

		case SDL_EVENT_JOYSTICK_BUTTON_DOWN:
		case SDL_EVENT_JOYSTICK_BUTTON_UP:
			if (event.jbutton.button < MAX_BUTTONS)
				m_joystick.buttons[event.jbutton.button] = (event.jbutton.down) ? 0x80 : 0;
			break;

		case SDL_EVENT_JOYSTICK_REMOVED:
			osd_printf_verbose("Joystick: %s [ID %s] disconnected\n", name(), id());
			clear_instance();
			clear_buffer();
			close_device();
			break;
		}
	}

	bool has_haptic() const
	{
		return m_hapdevice != nullptr;
	}

	void attach_device(SDL_Joystick *joy)
	{
		assert(joy);
		assert(!m_joydevice);

		set_instance(SDL_GetJoystickID(joy));
		m_joydevice = joy;
		m_hapdevice = SDL_OpenHapticFromJoystick(joy);

		osd_printf_verbose("Joystick: %s [ID %s] reconnected\n", name(), id());
	}

protected:
	// state information for a joystick
	struct sdl_joystick_state
	{
		s32 axes[MAX_AXES];
		s32 buttons[MAX_BUTTONS];
		s32 hatsU[MAX_HATS], hatsD[MAX_HATS], hatsL[MAX_HATS], hatsR[MAX_HATS];
		s32 balls[MAX_AXES];
	};

	sdl_joystick_state m_joystick;

private:
	SDL_Joystick *m_joydevice;
	SDL_Haptic *m_hapdevice;

	void clear_buffer()
	{
		memset(&m_joystick, 0, sizeof(m_joystick));
	}

	void close_device()
	{
		if (m_joydevice)
		{
			if (m_hapdevice)
			{
				SDL_CloseHaptic(m_hapdevice);
				m_hapdevice = nullptr;
			}
			SDL_CloseJoystick(m_joydevice);
			m_joydevice = nullptr;
		}
	}
};


//============================================================
//  sdl_sixaxis_joystick_device
//============================================================

class sdl_sixaxis_joystick_device : public sdl_joystick_device
{
public:
	using sdl_joystick_device::sdl_joystick_device;

	virtual void process_event(SDL_Event const &event) override
	{
		switch (event.type)
		{
		case SDL_EVENT_JOYSTICK_AXIS_MOTION:
			{
				int const axis = event.jaxis.axis;
				if (axis <= 3)
				{
					m_joystick.axes[event.jaxis.axis] = (event.jaxis.value * 2);
				}
				else
				{
					int const magic = (event.jaxis.value / 2) + 16384;
					m_joystick.axes[event.jaxis.axis] = magic;
				}
			}
			break;

		default:
			// Call the base for other events
			sdl_joystick_device::process_event(event);
			break;
		}
	}
};


//============================================================
//  sdl_game_controller_device
//============================================================

class sdl_game_controller_device : public sdl_joystick_device_base
{
public:
	sdl_game_controller_device(
			std::string &&name,
			std::string &&id,
			input_module &module,
			SDL_Gamepad *ctrl,
			char const *serial) :
		sdl_joystick_device_base(
				std::move(name),
				std::move(id),
				module,
				serial),
		m_controller({{0}}),
		m_ctrldevice(ctrl)
	{
		set_instance(SDL_GetJoystickID(SDL_GetGamepadJoystick(ctrl)));
	}

	~sdl_game_controller_device()
	{
		close_device();
	}

	virtual void configure(input_device &device) override
	{
		input_device::assignment_vector assignments;
		char const *const *axisnames = CONTROLLER_AXIS_XBOX;
		char const *const *buttonnames = CONTROLLER_BUTTON_XBOX360;
		bool digitaltriggers = false;
		bool avoidpaddles = false;
		auto const ctrltype = SDL_GetGamepadType(m_ctrldevice);
		switch (ctrltype)
		{
		case SDL_GAMEPAD_TYPE_STANDARD:
			osd_printf_verbose("Game Controller:   ...  unknown type\n", int(ctrltype));
			break;
		case SDL_GAMEPAD_TYPE_XBOX360:
			osd_printf_verbose("Game Controller:   ...  Xbox 360 type\n");
			axisnames = CONTROLLER_AXIS_XBOX;
			buttonnames = CONTROLLER_BUTTON_XBOX360;
			break;
		case SDL_GAMEPAD_TYPE_XBOXONE:
			osd_printf_verbose("Game Controller:   ...  Xbox One type\n");
			axisnames = CONTROLLER_AXIS_XBOX;
			buttonnames = CONTROLLER_BUTTON_XBOXONE;
			break;
		case SDL_GAMEPAD_TYPE_PS3:
			osd_printf_verbose("Game Controller:   ...  PlayStation 3 type\n");
			axisnames = CONTROLLER_AXIS_PS;
			buttonnames = CONTROLLER_BUTTON_PS3;
			break;
		case SDL_GAMEPAD_TYPE_PS4:
			osd_printf_verbose("Game Controller:   ...  PlayStation 4 type\n");
			axisnames = CONTROLLER_AXIS_PS;
			buttonnames = CONTROLLER_BUTTON_PS4;
			break;
		case SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_PRO:
			osd_printf_verbose("Game Controller:   ...  Switch Pro Controller type\n");
			axisnames = CONTROLLER_AXIS_SWITCH;
			buttonnames = CONTROLLER_BUTTON_SWITCH;
			digitaltriggers = true;
			break;
		//case SDL_GAMEPAD_TYPE_VIRTUAL:
		case SDL_GAMEPAD_TYPE_PS5:
			osd_printf_verbose("Game Controller:   ...  PlayStation 5 type\n");
			axisnames = CONTROLLER_AXIS_PS;
			buttonnames = CONTROLLER_BUTTON_PS5;
			break;

		case SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_JOYCON_PAIR:
			osd_printf_verbose("Game Controller:   ...  Joy-Con pair type\n");
			axisnames = CONTROLLER_AXIS_SWITCH;
			buttonnames = CONTROLLER_BUTTON_SWITCH;
			digitaltriggers = true;
			avoidpaddles = true;
			break;

		default: // do some other checks and fall back to Xbox layout if still unrecognized
			{
				const auto joystick = SDL_GetJoystickFromID(SDL_GetGamepadID(m_ctrldevice));
				const auto vendor_id = SDL_GetJoystickVendor(joystick);
				const auto product_id = SDL_GetJoystickProduct(joystick);

				if (vendor_id == 0x18d1 && product_id == 0x9400)
				{
					osd_printf_verbose("Game Controller:   ...  Google Stadia type\n");
					axisnames = CONTROLLER_AXIS_PS;
					buttonnames = CONTROLLER_BUTTON_STADIA;
				}
				else if (vendor_id == 0x0955 && (product_id == 0x7210 || product_id == 0x7214))
				{
					osd_printf_verbose("Game Controller:   ...  NVIDIA Shield type\n");
					axisnames = CONTROLLER_AXIS_XBOX;
					buttonnames = CONTROLLER_BUTTON_SHIELD;
				}
				else
				{
					osd_printf_verbose("Game Controller:   ...  unrecognized type (%d)\n", int(ctrltype));
				}
			}
			break;
		}

		// keep track of item numbers as we add controls
		std::pair<input_item_id, input_item_id> axisitems[SDL_GAMEPAD_AXIS_COUNT];
		input_item_id buttonitems[SDL_GAMEPAD_BUTTON_COUNT];
		std::tuple<input_item_id, SDL_GamepadButton, SDL_GamepadAxis> numberedbuttons[16];
		std::fill(
				std::begin(axisitems),
				std::end(axisitems),
				std::make_pair(ITEM_ID_INVALID, ITEM_ID_INVALID));
		std::fill(
				std::begin(buttonitems),
				std::end(buttonitems),
				ITEM_ID_INVALID);
		std::fill(
				std::begin(numberedbuttons),
				std::end(numberedbuttons),
				std::make_tuple(ITEM_ID_INVALID, SDL_GAMEPAD_BUTTON_INVALID, SDL_GAMEPAD_AXIS_INVALID));

		// add axes
		std::tuple<SDL_GamepadAxis, input_item_id, bool> const axes[]{
				{ SDL_GAMEPAD_AXIS_LEFTX,        ITEM_ID_XAXIS,   false },
				{ SDL_GAMEPAD_AXIS_LEFTY,        ITEM_ID_YAXIS,   false },
				{ SDL_GAMEPAD_AXIS_RIGHTX,       ITEM_ID_ZAXIS,   false },
				{ SDL_GAMEPAD_AXIS_RIGHTY,       ITEM_ID_RZAXIS,  false },
				{ SDL_GAMEPAD_AXIS_LEFT_TRIGGER,  ITEM_ID_SLIDER1, true },
				{ SDL_GAMEPAD_AXIS_RIGHT_TRIGGER, ITEM_ID_SLIDER2, true } };
		for (auto [axis, item, buttontest] : axes)
		{
			bool avail = !buttontest || !digitaltriggers;
			avail = avail && SDL_GamepadHasAxis(m_ctrldevice, axis);
			if (avail)
			{
				int bind_count = 0;
				SDL_GamepadBinding **binding = SDL_GetGamepadBindings(m_ctrldevice, &bind_count);
				bool hasAxisBinding = false;
				for (int idx = 0; idx < bind_count; idx++)
				{
					if (binding[idx]->input.axis.axis == axis)
					{
						// SDL 3 returns both analog (button) and digital (axis) bindings for
						// analog triggers.  So just allow it if there's an analog binding.
						if (binding[idx]->input_type == SDL_GAMEPAD_BINDTYPE_AXIS)
						{
							hasAxisBinding = true;
							break;
						}
					}
				}

				if (!hasAxisBinding)
				{
					avail = false;
				}
			}
			if (avail)
			{
				axisitems[axis].first = device.add_item(
						axisnames[axis],
						std::string_view(),
						item,
						generic_axis_get_state<s32>,
						&m_controller.axes[axis]);
			}
		}

		// add automatically numbered buttons
		std::tuple<SDL_GamepadButton, SDL_GamepadAxis, bool> const generalbuttons[]{
				{ SDL_GAMEPAD_BUTTON_SOUTH,             SDL_GAMEPAD_AXIS_INVALID,      true },
				{ SDL_GAMEPAD_BUTTON_EAST,             SDL_GAMEPAD_AXIS_INVALID,      true },
				{ SDL_GAMEPAD_BUTTON_WEST,             SDL_GAMEPAD_AXIS_INVALID,      true },
				{ SDL_GAMEPAD_BUTTON_NORTH,             SDL_GAMEPAD_AXIS_INVALID,      true },
				{ SDL_GAMEPAD_BUTTON_LEFT_SHOULDER,  SDL_GAMEPAD_AXIS_INVALID,      true },
				{ SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER, SDL_GAMEPAD_AXIS_INVALID,      true },
				{ SDL_GAMEPAD_BUTTON_INVALID,       SDL_GAMEPAD_AXIS_LEFT_TRIGGER,  true },
				{ SDL_GAMEPAD_BUTTON_INVALID,       SDL_GAMEPAD_AXIS_RIGHT_TRIGGER, true },
				{ SDL_GAMEPAD_BUTTON_LEFT_STICK,     SDL_GAMEPAD_AXIS_INVALID,      true },
				{ SDL_GAMEPAD_BUTTON_RIGHT_STICK,    SDL_GAMEPAD_AXIS_INVALID,      true },
				{ SDL_GAMEPAD_BUTTON_RIGHT_PADDLE1,       SDL_GAMEPAD_AXIS_INVALID,      true },
				{ SDL_GAMEPAD_BUTTON_LEFT_PADDLE1,       SDL_GAMEPAD_AXIS_INVALID,      true },
				{ SDL_GAMEPAD_BUTTON_RIGHT_PADDLE2,       SDL_GAMEPAD_AXIS_INVALID,      true },
				{ SDL_GAMEPAD_BUTTON_LEFT_PADDLE2,       SDL_GAMEPAD_AXIS_INVALID,      true },
				{ SDL_GAMEPAD_BUTTON_GUIDE,         SDL_GAMEPAD_AXIS_INVALID,      false },
				{ SDL_GAMEPAD_BUTTON_MISC1,         SDL_GAMEPAD_AXIS_INVALID,      false },
				{ SDL_GAMEPAD_BUTTON_TOUCHPAD,      SDL_GAMEPAD_AXIS_INVALID,      false },
				};
		input_item_id button_item = ITEM_ID_BUTTON1;
		unsigned buttoncount = 0;
		for (auto [button, axis, field] : generalbuttons)
		{
			bool avail = true;
			input_item_id actual = ITEM_ID_INVALID;
			if (SDL_GAMEPAD_BUTTON_INVALID != button)
			{
				avail = SDL_GamepadHasButton(m_ctrldevice, button);
				if (avail)
				{
					int bind_count = 0;
					SDL_GamepadBinding **binding = SDL_GetGamepadBindings(m_ctrldevice, &bind_count);
					for (int idx = 0; idx < bind_count; idx++)
					{
						if (binding[idx]->input.button == button)
						{
							if (binding[idx]->input_type == SDL_GAMEPAD_BINDTYPE_NONE)
							{
								avail = false;
							}
						}
					}
				}
				if (avail)
				{
					actual = buttonitems[button] = device.add_item(
							buttonnames[button],
							std::string_view(),
							button_item++,
							generic_button_get_state<s32>,
							&m_controller.buttons[button]);
					if (field && (std::size(numberedbuttons) > buttoncount))
						std::get<1>(numberedbuttons[buttoncount]) = button;
				}
			}
			else
			{
				avail = SDL_GamepadHasAxis(m_ctrldevice, axis);
				if (avail)
				{
					int bind_count = 0;
					SDL_GamepadBinding **binding = SDL_GetGamepadBindings(m_ctrldevice, &bind_count);
					for (int idx = 0; idx < bind_count; idx++)
					{
						if (binding[idx]->input.axis.axis ==axis)
						{
							switch (binding[idx]->input_type)
							{
							case SDL_GAMEPAD_BINDTYPE_NONE:
								avail = false;
								break;
							case SDL_GAMEPAD_BINDTYPE_BUTTON:
								break;
							default:
								avail = digitaltriggers;
								break;
							}
						}
					}
				}
				if (avail)
				{
					actual = axisitems[axis].second = device.add_item(
							axisnames[axis],
							std::string_view(),
							button_item++,
							[] (void *device_internal, void *item_internal) -> int
							{
								return (*reinterpret_cast<s32 const *>(item_internal) <= -16'384) ? 1 : 0;
							},
							&m_controller.axes[axis]);
					if (field && (std::size(numberedbuttons) > buttoncount))
						std::get<2>(numberedbuttons[buttoncount]) = axis;
				}
			}

			// add default button assignments
			if (field && avail && (std::size(numberedbuttons) > buttoncount))
			{
				std::get<0>(numberedbuttons[buttoncount]) = actual;
				add_button_assignment(assignments, ioport_type(IPT_BUTTON1 + buttoncount++), { actual });
			}
		}

		// add buttons with fixed item IDs
		std::pair<SDL_GamepadButton, input_item_id> const fixedbuttons[]{
				{ SDL_GAMEPAD_BUTTON_BACK,       ITEM_ID_SELECT },
				{ SDL_GAMEPAD_BUTTON_START,      ITEM_ID_START },
				{ SDL_GAMEPAD_BUTTON_DPAD_UP,    ITEM_ID_HAT1UP },
				{ SDL_GAMEPAD_BUTTON_DPAD_DOWN,  ITEM_ID_HAT1DOWN },
				{ SDL_GAMEPAD_BUTTON_DPAD_LEFT,  ITEM_ID_HAT1LEFT },
				{ SDL_GAMEPAD_BUTTON_DPAD_RIGHT, ITEM_ID_HAT1RIGHT } };
		for (auto [button, item] : fixedbuttons)
		{
			bool avail = true;
			avail = SDL_GamepadHasButton(m_ctrldevice, button);
			if (avail)
			{
				int bind_count = 0;
				SDL_GamepadBinding **binding = SDL_GetGamepadBindings(m_ctrldevice, &bind_count);
				for (int idx = 0; idx < bind_count; idx++)
				{
					if (binding[idx]->input.button == button)
					{
						switch (binding[idx]->input_type)
						{
						case SDL_GAMEPAD_BINDTYPE_NONE:
							avail = false;
							break;
						default:
							break;
						}
					}
				}
			}
			if (avail)
			{
				buttonitems[button] = device.add_item(
						buttonnames[button],
						std::string_view(),
						item,
						generic_button_get_state<s32>,
						&m_controller.buttons[button]);
			}
		}

		// try to get a "complete" joystick for primary movement controls
		input_item_id diraxis[2][2];
		choose_primary_stick(
				diraxis,
				axisitems[SDL_GAMEPAD_AXIS_LEFTX].first,
				axisitems[SDL_GAMEPAD_AXIS_LEFTY].first,
				axisitems[SDL_GAMEPAD_AXIS_RIGHTX].first,
				axisitems[SDL_GAMEPAD_AXIS_RIGHTY].first);

		// now set up controls using the primary joystick
		add_directional_assignments(
				assignments,
				diraxis[0][0],
				diraxis[0][1],
				buttonitems[SDL_GAMEPAD_BUTTON_DPAD_LEFT],
				buttonitems[SDL_GAMEPAD_BUTTON_DPAD_RIGHT],
				buttonitems[SDL_GAMEPAD_BUTTON_DPAD_UP],
				buttonitems[SDL_GAMEPAD_BUTTON_DPAD_DOWN]);

		// assign a secondary stick axis to joystick Z if available
		bool const zaxis = add_assignment(
				assignments,
				IPT_AD_STICK_Z,
				SEQ_TYPE_STANDARD,
				ITEM_CLASS_ABSOLUTE,
				ITEM_MODIFIER_NONE,
				{ diraxis[1][1], diraxis[1][0] });
		if (!zaxis)
		{
			// if both triggers are present, combine them, or failing that, fall back to a pair of buttons
			if ((ITEM_ID_INVALID != axisitems[SDL_GAMEPAD_AXIS_LEFT_TRIGGER].first) && (ITEM_ID_INVALID != axisitems[SDL_GAMEPAD_AXIS_RIGHT_TRIGGER].first))
			{
				assignments.emplace_back(
						IPT_AD_STICK_Z,
						SEQ_TYPE_STANDARD,
						input_seq(
								make_code(ITEM_CLASS_ABSOLUTE, ITEM_MODIFIER_NONE, axisitems[SDL_GAMEPAD_AXIS_LEFT_TRIGGER].first),
								make_code(ITEM_CLASS_ABSOLUTE, ITEM_MODIFIER_REVERSE, axisitems[SDL_GAMEPAD_AXIS_RIGHT_TRIGGER].first)));
			}
			else if (add_axis_inc_dec_assignment(assignments, IPT_AD_STICK_Z, buttonitems[SDL_GAMEPAD_BUTTON_LEFT_SHOULDER], buttonitems[SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER]))
			{
				// took shoulder buttons
			}
			else if (add_axis_inc_dec_assignment(assignments, IPT_AD_STICK_Z, axisitems[SDL_GAMEPAD_AXIS_LEFT_TRIGGER].second, axisitems[SDL_GAMEPAD_AXIS_RIGHT_TRIGGER].second))
			{
				// took trigger buttons
			}
			else if (add_axis_inc_dec_assignment(assignments, IPT_AD_STICK_Z, buttonitems[SDL_GAMEPAD_BUTTON_RIGHT_PADDLE1], buttonitems[SDL_GAMEPAD_BUTTON_LEFT_PADDLE1]))
			{
				// took P1/P2
			}
			else if (add_axis_inc_dec_assignment(assignments, IPT_AD_STICK_Z, buttonitems[SDL_GAMEPAD_BUTTON_RIGHT_PADDLE2], buttonitems[SDL_GAMEPAD_BUTTON_LEFT_PADDLE2]))
			{
				// took P3/P4
			}
		}

		// prefer trigger axes for pedals, otherwise take half axes and buttons
		unsigned pedalbutton = 0;
		if (!add_assignment(assignments, IPT_PEDAL, SEQ_TYPE_STANDARD, ITEM_CLASS_ABSOLUTE, ITEM_MODIFIER_NEG, { axisitems[SDL_GAMEPAD_AXIS_RIGHT_TRIGGER].first }))
		{
			add_assignment(
					assignments,
					IPT_PEDAL,
					SEQ_TYPE_STANDARD,
					ITEM_CLASS_ABSOLUTE,
					ITEM_MODIFIER_NEG,
					{ diraxis[1][1], diraxis[0][1] });
			bool const incbutton = add_assignment(
					assignments,
					IPT_PEDAL,
					SEQ_TYPE_INCREMENT,
					ITEM_CLASS_SWITCH,
					ITEM_MODIFIER_NONE,
					{ axisitems[SDL_GAMEPAD_AXIS_RIGHT_TRIGGER].second, buttonitems[SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER] });
			if (!incbutton)
			{
				if (add_assignment(assignments, IPT_PEDAL, SEQ_TYPE_INCREMENT, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, { std::get<0>(numberedbuttons[pedalbutton]) }))
					++pedalbutton;
			}
		}
		if (!add_assignment(assignments, IPT_PEDAL2, SEQ_TYPE_STANDARD, ITEM_CLASS_ABSOLUTE, ITEM_MODIFIER_NEG, { axisitems[SDL_GAMEPAD_AXIS_LEFT_TRIGGER].first }))
		{
			add_assignment(
					assignments,
					IPT_PEDAL2,
					SEQ_TYPE_STANDARD,
					ITEM_CLASS_ABSOLUTE,
					ITEM_MODIFIER_POS,
					{ diraxis[1][1], diraxis[0][1] });
			bool const incbutton = add_assignment(
					assignments,
					IPT_PEDAL2,
					SEQ_TYPE_INCREMENT,
					ITEM_CLASS_SWITCH,
					ITEM_MODIFIER_NONE,
					{ axisitems[SDL_GAMEPAD_AXIS_LEFT_TRIGGER].second, buttonitems[SDL_GAMEPAD_BUTTON_LEFT_SHOULDER] });
			if (!incbutton)
			{
				if (add_assignment(assignments, IPT_PEDAL2, SEQ_TYPE_INCREMENT, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, { std::get<0>(numberedbuttons[pedalbutton]) }))
					++pedalbutton;
			}
		}
		add_assignment(assignments, IPT_PEDAL3, SEQ_TYPE_INCREMENT, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, { std::get<0>(numberedbuttons[pedalbutton]) });

		// potentially use thumb sticks and/or D-pad and A/B/X/Y diamond for twin sticks
		add_twin_stick_assignments(
				assignments,
				axisitems[SDL_GAMEPAD_AXIS_LEFTX].first,
				axisitems[SDL_GAMEPAD_AXIS_LEFTY].first,
				axisitems[SDL_GAMEPAD_AXIS_RIGHTX].first,
				axisitems[SDL_GAMEPAD_AXIS_RIGHTY].first,
				buttonitems[SDL_GAMEPAD_BUTTON_DPAD_LEFT],
				buttonitems[SDL_GAMEPAD_BUTTON_DPAD_RIGHT],
				buttonitems[SDL_GAMEPAD_BUTTON_DPAD_UP],
				buttonitems[SDL_GAMEPAD_BUTTON_DPAD_DOWN],
				buttonitems[SDL_GAMEPAD_BUTTON_WEST],
				buttonitems[SDL_GAMEPAD_BUTTON_EAST],
				buttonitems[SDL_GAMEPAD_BUTTON_NORTH],
				buttonitems[SDL_GAMEPAD_BUTTON_SOUTH]);

		// add assignments for buttons with fixed functions
		add_button_assignment(assignments, IPT_SELECT,  { buttonitems[SDL_GAMEPAD_BUTTON_BACK] });
		add_button_assignment(assignments, IPT_START,   { buttonitems[SDL_GAMEPAD_BUTTON_START] });
		add_button_assignment(assignments, IPT_UI_MENU, { buttonitems[SDL_GAMEPAD_BUTTON_GUIDE] });

		// the first button is always UI select
		if (add_button_assignment(assignments, IPT_UI_SELECT, { std::get<0>(numberedbuttons[0]) }))
		{
			if (SDL_GAMEPAD_BUTTON_INVALID != std::get<1>(numberedbuttons[0]))
				buttonitems[std::get<1>(numberedbuttons[0])] = ITEM_ID_INVALID;
			if (SDL_GAMEPAD_AXIS_INVALID != std::get<2>(numberedbuttons[0]))
				axisitems[std::get<2>(numberedbuttons[0])].second = ITEM_ID_INVALID;
		}

		// try to get a matching pair of buttons for previous/next group
		if (consume_button_pair(assignments, IPT_UI_PREV_GROUP, IPT_UI_NEXT_GROUP, axisitems[SDL_GAMEPAD_AXIS_LEFT_TRIGGER].second, axisitems[SDL_GAMEPAD_AXIS_RIGHT_TRIGGER].second))
		{
			// took digital triggers
		}
		else if (!avoidpaddles && consume_button_pair(assignments, IPT_UI_PREV_GROUP, IPT_UI_NEXT_GROUP, buttonitems[SDL_GAMEPAD_BUTTON_RIGHT_PADDLE1], buttonitems[SDL_GAMEPAD_BUTTON_LEFT_PADDLE1]))
		{
			// took upper paddles
		}
		else if (consume_trigger_pair(assignments, IPT_UI_PREV_GROUP, IPT_UI_NEXT_GROUP, axisitems[SDL_GAMEPAD_AXIS_LEFT_TRIGGER].first, axisitems[SDL_GAMEPAD_AXIS_RIGHT_TRIGGER].first))
		{
			// took analog triggers
		}
		else if (!avoidpaddles && consume_button_pair(assignments, IPT_UI_PREV_GROUP, IPT_UI_NEXT_GROUP, buttonitems[SDL_GAMEPAD_BUTTON_RIGHT_PADDLE2], buttonitems[SDL_GAMEPAD_BUTTON_LEFT_PADDLE2]))
		{
			// took lower paddles
		}
		else if (consume_axis_pair(assignments, IPT_UI_PREV_GROUP, IPT_UI_NEXT_GROUP, diraxis[1][1]))
		{
			// took secondary Y
		}
		else if (consume_axis_pair(assignments, IPT_UI_PREV_GROUP, IPT_UI_NEXT_GROUP, diraxis[1][0]))
		{
			// took secondary X
		}

		// try to get a matching pair of buttons for page up/down
		if (!avoidpaddles && consume_button_pair(assignments, IPT_UI_PAGE_UP, IPT_UI_PAGE_DOWN, buttonitems[SDL_GAMEPAD_BUTTON_RIGHT_PADDLE1], buttonitems[SDL_GAMEPAD_BUTTON_LEFT_PADDLE1]))
		{
			// took upper paddles
		}
		else if (!avoidpaddles && consume_button_pair(assignments, IPT_UI_PAGE_UP, IPT_UI_PAGE_DOWN, buttonitems[SDL_GAMEPAD_BUTTON_RIGHT_PADDLE2], buttonitems[SDL_GAMEPAD_BUTTON_LEFT_PADDLE2]))
		{
			// took lower paddles
		}
		else
		if (consume_trigger_pair(assignments, IPT_UI_PAGE_UP, IPT_UI_PAGE_DOWN, axisitems[SDL_GAMEPAD_AXIS_LEFT_TRIGGER].first, axisitems[SDL_GAMEPAD_AXIS_RIGHT_TRIGGER].first))
		{
			// took analog triggers
		}
		else if (consume_axis_pair(assignments, IPT_UI_PAGE_UP, IPT_UI_PAGE_DOWN, diraxis[1][1]))
		{
			// took secondary Y
		}

		// try to assign X button to UI clear
		if (add_button_assignment(assignments, IPT_UI_CLEAR, { buttonitems[SDL_GAMEPAD_BUTTON_WEST] }))
		{
			buttonitems[SDL_GAMEPAD_BUTTON_WEST] = ITEM_ID_INVALID;
		}
		else
		{
			// otherwise try to find an unassigned button
			for (auto [item, button, axis] : numberedbuttons)
			{
				if ((SDL_GAMEPAD_BUTTON_INVALID != button) && (ITEM_ID_INVALID != buttonitems[button]))
				{
					add_button_assignment(assignments, IPT_UI_CLEAR, { item });
					buttonitems[button] = ITEM_ID_INVALID;
					break;
				}
				else if ((SDL_GAMEPAD_AXIS_INVALID != axis) && (ITEM_ID_INVALID != axisitems[axis].second))
				{
					add_button_assignment(assignments, IPT_UI_CLEAR, { item });
					axisitems[axis].second = ITEM_ID_INVALID;
					break;
				}
			}
		}

		// try to assign B button to UI back
		if (add_button_assignment(assignments, IPT_UI_BACK, { buttonitems[SDL_GAMEPAD_BUTTON_EAST] }))
		{
			buttonitems[SDL_GAMEPAD_BUTTON_WEST] = ITEM_ID_INVALID;
		}
		else
		{
			// otherwise try to find an unassigned button
			for (auto [item, button, axis] : numberedbuttons)
			{
				if ((SDL_GAMEPAD_BUTTON_INVALID != button) && (ITEM_ID_INVALID != buttonitems[button]))
				{
					add_button_assignment(assignments, IPT_UI_CLEAR, { item });
					buttonitems[button] = ITEM_ID_INVALID;
					break;
				}
				else if ((SDL_GAMEPAD_AXIS_INVALID != axis) && (ITEM_ID_INVALID != axisitems[axis].second))
				{
					add_button_assignment(assignments, IPT_UI_CLEAR, { item });
					axisitems[axis].second = ITEM_ID_INVALID;
					break;
				}
			}
		}

		// try to assign Y button to UI help
		if (add_button_assignment(assignments, IPT_UI_HELP, { buttonitems[SDL_GAMEPAD_BUTTON_NORTH] }))
		{
			buttonitems[SDL_GAMEPAD_BUTTON_NORTH] = ITEM_ID_INVALID;
		}
		else
		{
			// otherwise try to find an unassigned button
			for (auto [item, button, axis] : numberedbuttons)
			{
				if ((SDL_GAMEPAD_BUTTON_INVALID != button) && (ITEM_ID_INVALID != buttonitems[button]))
				{
					add_button_assignment(assignments, IPT_UI_HELP, { item });
					buttonitems[button] = ITEM_ID_INVALID;
					break;
				}
				else if ((SDL_GAMEPAD_AXIS_INVALID != axis) && (ITEM_ID_INVALID != axisitems[axis].second))
				{
					add_button_assignment(assignments, IPT_UI_HELP, { item });
					axisitems[axis].second = ITEM_ID_INVALID;
					break;
				}
			}
		}

		// put focus previous/next on the shoulder buttons if available - this can be overloaded with zoom
		if (add_button_pair_assignment(assignments, IPT_UI_FOCUS_PREV, IPT_UI_FOCUS_NEXT, buttonitems[SDL_GAMEPAD_BUTTON_LEFT_SHOULDER], buttonitems[SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER]))
		{
			// took shoulder buttons
		}
		else if (add_axis_pair_assignment(assignments, IPT_UI_FOCUS_PREV, IPT_UI_FOCUS_NEXT, diraxis[1][0]))
		{
			// took secondary X
		}
		else if (add_axis_pair_assignment(assignments, IPT_UI_FOCUS_PREV, IPT_UI_FOCUS_NEXT, diraxis[1][1]))
		{
			// took secondary Y
		}

		// put zoom on the secondary stick if available, or fall back to shoulder buttons
		if (add_axis_pair_assignment(assignments, IPT_UI_ZOOM_OUT, IPT_UI_ZOOM_IN, diraxis[1][0]))
		{
			// took secondary X
			if (axisitems[SDL_GAMEPAD_AXIS_LEFTX].first == diraxis[1][0])
				add_button_assignment(assignments, IPT_UI_ZOOM_DEFAULT, { buttonitems[SDL_GAMEPAD_BUTTON_LEFT_STICK] });
			else if (axisitems[SDL_GAMEPAD_AXIS_RIGHTX].first == diraxis[1][0])
				add_button_assignment(assignments, IPT_UI_ZOOM_DEFAULT, { buttonitems[SDL_GAMEPAD_BUTTON_RIGHT_STICK] });
			diraxis[1][0] = ITEM_ID_INVALID;
		}
		else if (add_axis_pair_assignment(assignments, IPT_UI_ZOOM_IN, IPT_UI_ZOOM_OUT, diraxis[1][1]))
		{
			// took secondary Y
			if (axisitems[SDL_GAMEPAD_AXIS_LEFTY].first == diraxis[1][1])
				add_button_assignment(assignments, IPT_UI_ZOOM_DEFAULT, { buttonitems[SDL_GAMEPAD_BUTTON_LEFT_STICK] });
			else if (axisitems[SDL_GAMEPAD_AXIS_RIGHTY].first == diraxis[1][1])
				add_button_assignment(assignments, IPT_UI_ZOOM_DEFAULT, { buttonitems[SDL_GAMEPAD_BUTTON_RIGHT_STICK] });
			diraxis[1][1] = ITEM_ID_INVALID;
		}
		else if (consume_button_pair(assignments, IPT_UI_ZOOM_OUT, IPT_UI_ZOOM_IN, buttonitems[SDL_GAMEPAD_BUTTON_LEFT_SHOULDER], buttonitems[SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER]))
		{
			// took shoulder buttons
		}

		// set default assignments
		device.set_default_assignments(std::move(assignments));
	}

	virtual void reset() override
	{
		sdl_joystick_device_base::reset();
		clear_buffer();
	}

	virtual void process_event(SDL_Event const &event) override
	{
		if (!m_ctrldevice)
			return;

		switch (event.type)
		{
		case SDL_EVENT_GAMEPAD_AXIS_MOTION:
			if (event.gaxis.axis < SDL_GAMEPAD_AXIS_COUNT)
			{
				switch (event.gaxis.axis)
				{
				case SDL_GAMEPAD_AXIS_LEFT_TRIGGER: // MAME wants negative values for triggers
				case SDL_GAMEPAD_AXIS_RIGHT_TRIGGER:
					m_controller.axes[event.gaxis.axis] = -normalize_absolute_axis(event.gaxis.value, -32'767, 32'767);
					break;
				default:
					m_controller.axes[event.gaxis.axis] = normalize_absolute_axis(event.gaxis.value, -32'767, 32'767);
				}
			}
			break;

		case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
		case SDL_EVENT_GAMEPAD_BUTTON_UP:
			if (event.gbutton.button < SDL_GAMEPAD_BUTTON_COUNT)
				m_controller.buttons[event.gbutton.button] = (event.gbutton.down) ? 0x80 : 0x00;
			break;

		case SDL_EVENT_GAMEPAD_REMOVED:
			osd_printf_verbose("Game Controller: %s [ID %s] disconnected\n", name(), id());
			clear_instance();
			clear_buffer();
			close_device();
			break;
		}
	}

	void attach_device(SDL_Gamepad *ctrl)
	{
		assert(ctrl);
		assert(!m_ctrldevice);

		set_instance(SDL_GetJoystickID(SDL_GetGamepadJoystick(ctrl)));
		m_ctrldevice = ctrl;

		osd_printf_verbose("Game Controller: %s [ID %s] reconnected\n", name(), id());
	}

private:
	// state information for a game controller
	struct sdl_controller_state
	{
		s32 axes[SDL_GAMEPAD_AXIS_COUNT];
		s32 buttons[SDL_GAMEPAD_BUTTON_COUNT];
	};

	sdl_controller_state m_controller;
	SDL_Gamepad *m_ctrldevice;

	void clear_buffer()
	{
		memset(&m_controller, 0, sizeof(m_controller));
	}

	void close_device()
	{
		if (m_ctrldevice)
		{
			SDL_CloseGamepad(m_ctrldevice);
			m_ctrldevice = nullptr;
		}
	}
};


//============================================================
//  sdl_input_module
//============================================================

template <typename Info>
class sdl_input_module :
		public input_module_impl<Info, sdl_osd_interface>,
		protected sdl_event_manager::subscriber
{
public:
	sdl_input_module(char const *type, char const *name) :
		input_module_impl<Info, sdl_osd_interface>(type, name)
	{
	}

	virtual void exit() override
	{
		// unsubscribe for events
		unsubscribe();

		input_module_impl<Info, sdl_osd_interface>::exit();
	}

protected:
	virtual void handle_event(SDL_Event const &event) override
	{
		// dispatch event to every device by default
		this->devicelist().for_each_device(
				[&event] (auto &device) { device.queue_events(&event, 1); });
	}
};


//============================================================
//  sdl_keyboard_module
//============================================================

class sdl_keyboard_module : public sdl_input_module<sdl_keyboard_device>
{
public:
	sdl_keyboard_module() :
		sdl_input_module<sdl_keyboard_device>(OSD_KEYBOARDINPUT_PROVIDER, "sdl")
	{
	}

	virtual void input_init(running_machine &machine) override
	{
		sdl_input_module<sdl_keyboard_device>::input_init(machine);

		constexpr int event_types[] = {
				int(SDL_EVENT_KEY_DOWN),
				int(SDL_EVENT_KEY_UP) };

		subscribe(osd(), event_types);

		// Read our keymap and store a pointer to our table
		sdlinput_read_keymap();

		osd_printf_verbose("Keyboard: Start initialization\n");

		int count = 0;
		const auto keyboards = SDL_GetKeyboards(&count);

		osd_printf_verbose("Keyboard: Using SDL 3.2+, found %d keyboard%c\n", count, count > 1 ? 's' : ' ');

		// TODO: Multiple keyboard/mouse support is Windows-only in SDL 3.2; this will expand to Linux Wayland
		// in SDL 3.4.  X11 will return all of the available keyboards but only return keypresses for the system
		// composite device #0 unless SDL is specially compiled (which its not in distro packages).
		// On macOS only the system keyboard is supported.
		auto &devinfo = create_device<sdl_keyboard_device>(
			DEVICE_CLASS_KEYBOARD,
			"System keyboard",
			"System keyboard",
			keyboards[0],
			*m_key_trans_table);
		osd_printf_verbose("Keyboard: Registered %s\n", devinfo.name());
		SDL_free(keyboards);

		osd_printf_verbose("Keyboard: End initialization\n");
	}

private:
	void sdlinput_read_keymap()
	{
		keyboard_trans_table &default_table = keyboard_trans_table::instance();

		// Allocate a block of translation entries big enough to hold what's in the default table
		auto key_trans_entries = std::make_unique<key_trans_entry []>(default_table.size());

		// copy the elements from the default table and ask SDL for key names
		for (int i = 0; i < default_table.size(); i++)
		{
			key_trans_entries[i] = default_table[i];
			char const *const name = SDL_GetScancodeName(SDL_Scancode(default_table[i].sdl_scancode));
			if (name && *name)
				key_trans_entries[i].ui_name = name;
		}

		// Allocate the trans table to be associated with the machine so we don't have to free it
		m_key_trans_table = std::make_unique<keyboard_trans_table>(std::move(key_trans_entries), default_table.size());

		if (!options()->bool_value(SDLOPTION_KEYMAP))
			return;

		const char *const keymap_filename = dynamic_cast<sdl_options const &>(*options()).keymap_file();
		osd_printf_verbose("Keymap: Start reading keymap_file %s\n", keymap_filename);

		FILE *const keymap_file = fopen(keymap_filename, "r");
		if (!keymap_file)
		{
			osd_printf_warning("Keymap: Unable to open keymap %s, using default\n", keymap_filename);
			return;
		}

		int line = 1;
		int sdl3section = 0;
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
				if (strncmp(buf, "[SDL3]", 6) == 0)
				{
					sdl3section = 1;
				}
				else if (sdl3section == 1)
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
						key_trans_entry &entry = (*m_key_trans_table)[index];
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
	}

	std::unique_ptr<keyboard_trans_table> m_key_trans_table;
	std::list<std::string> m_ui_names;
};


//============================================================
//  sdl_mouse_module
//============================================================

class sdl_mouse_module : public sdl_input_module<sdl_mouse_device>
{
public:
	sdl_mouse_module() : sdl_input_module<sdl_mouse_device>(OSD_MOUSEINPUT_PROVIDER, "sdl")
	{
	}

	virtual void input_init(running_machine &machine) override
	{
		sdl_input_module::input_init(machine);

		constexpr int event_types[] = {
				int(SDL_EVENT_MOUSE_MOTION),
				int(SDL_EVENT_MOUSE_BUTTON_DOWN),
				int(SDL_EVENT_MOUSE_BUTTON_UP),
				int(SDL_EVENT_MOUSE_WHEEL) };

		subscribe(osd(), event_types);

		osd_printf_verbose("Mouse: Start initialization\n");

		int count = 0;
		const auto mice = SDL_GetMice(&count);

		osd_printf_verbose("Mouse: Using SDL 3.2+, found %d %s\n", count, (count == 1) ? "mouse" : "mice");

		// TODO: add mice other than the first one
		auto &devinfo = create_device<sdl_mouse_device>(
				DEVICE_CLASS_MOUSE,
				"System mouse",
				"System mouse");

		osd_printf_verbose("Mouse: Registered %s\n", devinfo.name());
		SDL_free(mice);

		osd_printf_verbose("Mouse: End initialization\n");
	}
};


//============================================================
//  sdl_lightgun_module
//============================================================

class sdl_lightgun_module : public sdl_input_module<sdl_mouse_device_base>
{
public:
	sdl_lightgun_module() : sdl_input_module<sdl_mouse_device_base>(OSD_LIGHTGUNINPUT_PROVIDER, "sdl")
	{
	}

	virtual void input_init(running_machine &machine) override
	{
		auto &sdlopts = dynamic_cast<sdl_options const &>(*options());
		sdl_input_module::input_init(machine);
		bool const dual(sdlopts.dual_lightgun());

		if (!dual)
		{
			constexpr int event_types[] = {
				int(SDL_EVENT_MOUSE_MOTION),
				int(SDL_EVENT_MOUSE_BUTTON_DOWN),
				int(SDL_EVENT_MOUSE_BUTTON_UP),
				int(SDL_EVENT_MOUSE_WHEEL),
				int(SDL_EVENT_WINDOW_SHOWN),
				int(SDL_EVENT_WINDOW_HIDDEN),
				int(SDL_EVENT_WINDOW_EXPOSED),
				int(SDL_EVENT_WINDOW_MOVED),
				int(SDL_EVENT_WINDOW_RESIZED),
				int(SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED),
				int(SDL_EVENT_WINDOW_METAL_VIEW_RESIZED),
				int(SDL_EVENT_WINDOW_MINIMIZED),
				int(SDL_EVENT_WINDOW_MAXIMIZED),
				int(SDL_EVENT_WINDOW_RESTORED),
				int(SDL_EVENT_WINDOW_MOUSE_ENTER),
				int(SDL_EVENT_WINDOW_MOUSE_LEAVE),
				int(SDL_EVENT_WINDOW_FOCUS_GAINED),
				int(SDL_EVENT_WINDOW_FOCUS_LOST),
				int(SDL_EVENT_WINDOW_CLOSE_REQUESTED),
				int(SDL_EVENT_WINDOW_HIT_TEST),
				int(SDL_EVENT_WINDOW_ICCPROF_CHANGED),
				int(SDL_EVENT_WINDOW_DISPLAY_CHANGED),
				int(SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED),
				int(SDL_EVENT_WINDOW_SAFE_AREA_CHANGED),
				int(SDL_EVENT_WINDOW_OCCLUDED),
				int(SDL_EVENT_WINDOW_ENTER_FULLSCREEN),
				int(SDL_EVENT_WINDOW_LEAVE_FULLSCREEN),
				int(SDL_EVENT_WINDOW_DESTROYED),
				int(SDL_EVENT_WINDOW_HDR_STATE_CHANGED) };
			subscribe(osd(), event_types);
		}
		else
		{
			constexpr int event_types[] = {
					int(SDL_EVENT_MOUSE_BUTTON_DOWN),
					int(SDL_EVENT_MOUSE_BUTTON_UP) };
			subscribe(osd(), event_types);
		}

		osd_printf_verbose("Lightgun: Start initialization\n");

		if (!dual)
		{
			auto &devinfo = create_device<sdl_lightgun_device>(
					DEVICE_CLASS_LIGHTGUN,
					"System pointer gun 1",
					"System pointer gun 1");
			osd_printf_verbose("Lightgun: Registered %s\n", devinfo.name());
		}
		else
		{
			auto &dev1info = create_device<sdl_dual_lightgun_device>(
					DEVICE_CLASS_LIGHTGUN,
					"System pointer gun 1",
					"System pointer gun 1",
					0);
			osd_printf_verbose("Lightgun: Registered %s\n", dev1info.name());

			auto &dev2info = create_device<sdl_dual_lightgun_device>(
					DEVICE_CLASS_LIGHTGUN,
					"System pointer gun 2",
					"System pointer gun 2",
					1);
			osd_printf_verbose("Lightgun: Registered %s\n", dev2info.name());
		}

		osd_printf_verbose("Lightgun: End initialization\n");
	}
};


//============================================================
//  sdl_joystick_module_base
//============================================================

class sdl_joystick_module_base : public sdl_input_module<sdl_joystick_device_base>
{
protected:
	sdl_joystick_module_base(char const *name) :
		sdl_input_module<sdl_joystick_device_base>(OSD_JOYSTICKINPUT_PROVIDER, name),
		m_initialized_joystick(false),
		m_initialized_haptic(false)
	{
	}

	virtual ~sdl_joystick_module_base()
	{
		assert(!m_initialized_joystick);
		assert(!m_initialized_haptic);
	}

	bool have_joystick() const { return m_initialized_joystick; }
	bool have_haptic() const { return m_initialized_haptic; }

	void init_joystick()
	{
		assert(!m_initialized_joystick);
		assert(!m_initialized_haptic);

		m_initialized_joystick = SDL_InitSubSystem(SDL_INIT_JOYSTICK);
		if (!m_initialized_joystick)
		{
			osd_printf_error("Could not initialize SDL Joystick subsystem: %s.\n", SDL_GetError());
			return;
		}

		m_initialized_haptic = SDL_InitSubSystem(SDL_INIT_HAPTIC);
		if (!m_initialized_haptic)
			osd_printf_verbose("Could not initialize SDL Haptic subsystem: %s.\n", SDL_GetError());
	}

	void quit_joystick()
	{
		if (m_initialized_joystick)
		{
			SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
			m_initialized_joystick = false;
		}

		if (m_initialized_haptic)
		{
			SDL_QuitSubSystem(SDL_INIT_HAPTIC);
			m_initialized_haptic = false;
		}
	}

	sdl_joystick_device *create_joystick_device(SDL_JoystickID sdl_id, bool sixaxis)
	{
		// open the joystick device
		SDL_Joystick *const joy = SDL_OpenJoystick(sdl_id);
		if (!joy)
		{
			osd_printf_error("Joystick: Could not open SDL joystick %d: %s.\n", (int)sdl_id, SDL_GetError());
			return nullptr;
		}

		// get basic info
		char const *const name = SDL_GetJoystickName(joy);
		SDL_GUID guid = SDL_GetJoystickGUID(joy);
		char guid_str[256];
		guid_str[0] = '\0';
		SDL_GUIDToString(guid, guid_str, sizeof(guid_str) - 1);
		char const *const serial = SDL_GetJoystickSerial(joy);
		std::string id(guid_str);
		if (serial)
			id.append(1, '-').append(serial);

		// print some diagnostic info
		osd_printf_verbose("Joystick: %s [GUID %s] Vendor ID %04X, Product ID %04X, Revision %04X, Serial %s\n",
				name ? name : "<nullptr>",
				guid_str,
				SDL_GetJoystickVendor(joy),
				SDL_GetJoystickProduct(joy),
				SDL_GetJoystickProductVersion(joy),
				serial ? serial : "<nullptr>");
		osd_printf_verbose("Joystick:   ...  %d axes, %d buttons %d hats %d balls\n",
				SDL_GetNumJoystickAxes(joy),
				SDL_GetNumJoystickButtons(joy),
				SDL_GetNumJoystickHats(joy),
				SDL_GetNumJoystickBalls(joy));
		if (SDL_GetNumJoystickButtons(joy) > MAX_BUTTONS)
			osd_printf_verbose("Joystick:   ...  Has %d buttons which exceeds supported %d buttons\n", SDL_GetNumJoystickButtons(joy), MAX_BUTTONS);

		// instantiate device
		sdl_joystick_device &devinfo = sixaxis
				? create_device<sdl_sixaxis_joystick_device>(DEVICE_CLASS_JOYSTICK, name ? name : guid_str, guid_str, joy, serial)
				: create_device<sdl_joystick_device>(DEVICE_CLASS_JOYSTICK, name ? name : guid_str, guid_str, joy, serial);

		if (devinfo.has_haptic())
			osd_printf_verbose("Joystick:   ...  Has haptic capability\n");
		else
			osd_printf_verbose("Joystick:   ...  Does not have haptic capability\n");

		return &devinfo;
	}

	void dispatch_joystick_event(SDL_Event const &event)
	{
		// figure out which joystick this event is destined for
		sdl_joystick_device_base *const target_device = find_joystick(event.jdevice.which); // FIXME: this depends on SDL_JoystickID being the same size as Sint32

		// if we find a matching joystick, dispatch the event to the joystick
		if (target_device)
			target_device->queue_events(&event, 1);
	}

	device_info *find_reconnect_match(SDL_GUID const &guid, char const *serial)
	{
		char guid_str[256];
		guid_str[0] = '\0';
		SDL_GUIDToString(guid, guid_str, sizeof(guid_str) - 1);
		auto target_device = std::find_if(
				devicelist().begin(),
				devicelist().end(),
				[&guid_str, &serial] (auto const &device)
				{
					return device->reconnect_match(guid_str, serial);
				});
		return (devicelist().end() != target_device) ? target_device->get() : nullptr;
	}

	sdl_joystick_device_base *find_joystick(SDL_JoystickID instance)
	{
		for (auto &device : devicelist())
		{
			if (device->is_instance(instance))
				return device.get();
		}
		return nullptr;
	}

private:
	bool m_initialized_joystick;
	bool m_initialized_haptic;
};


//============================================================
//  sdl_joystick_module
//============================================================

class sdl_joystick_module : public sdl_joystick_module_base
{
public:
	sdl_joystick_module() : sdl_joystick_module_base("sdljoy")
	{
	}

	virtual void exit() override
	{
		sdl_joystick_module_base::exit();

		quit_joystick();
	}

	virtual void input_init(running_machine &machine) override
	{
		auto &sdlopts = dynamic_cast<sdl_options const &>(*options());
		bool const sixaxis_mode = sdlopts.sixaxis();

		if (!sdlopts.debug() && sdlopts.background_input())
			SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");

		init_joystick();
		if (!have_joystick())
			return;

		sdl_joystick_module_base::input_init(machine);

		osd_printf_verbose("Joystick: Start initialization\n");
		int stick_count = 0;
		const auto joysticks = SDL_GetJoysticks(&stick_count);
		for (int physical_stick = 0; physical_stick < stick_count; physical_stick++)
		{
			create_joystick_device(joysticks[physical_stick], sixaxis_mode);
		}
		SDL_free(joysticks);

		constexpr int event_types[] = {
				int(SDL_EVENT_JOYSTICK_AXIS_MOTION),
				int(SDL_EVENT_JOYSTICK_BALL_MOTION),
				int(SDL_EVENT_JOYSTICK_HAT_MOTION),
				int(SDL_EVENT_JOYSTICK_BUTTON_DOWN),
				int(SDL_EVENT_JOYSTICK_BUTTON_UP),
				int(SDL_EVENT_JOYSTICK_ADDED),
				int(SDL_EVENT_JOYSTICK_REMOVED) };
		subscribe(osd(), event_types);

		osd_printf_verbose("Joystick: End initialization\n");
	}

	virtual void handle_event(SDL_Event const &event) override
	{
		if (SDL_EVENT_JOYSTICK_ADDED == event.type)
		{
			SDL_Joystick *const joy = SDL_OpenJoystick(event.jdevice.which);
			if (!joy)
			{
				osd_printf_error("Joystick: Could not open SDL joystick %d: %s.\n", event.jdevice.which, SDL_GetError());
			}
			else
			{
				SDL_GUID guid = SDL_GetJoystickGUID(joy);
				char const *const serial = SDL_GetJoystickSerial(joy);
				auto *const target_device = find_reconnect_match(guid, serial);
				if (target_device)
				{
					auto &devinfo = dynamic_cast<sdl_joystick_device &>(*target_device);
					devinfo.attach_device(joy);
				}
				else
				{
					SDL_CloseJoystick(joy);
				}
			}
		}
		else
		{
			dispatch_joystick_event(event);
		}
	}
};


//============================================================
//  sdl_game_controller_module
//============================================================

class sdl_game_controller_module : public sdl_joystick_module_base
{
public:
	sdl_game_controller_module() :
		sdl_joystick_module_base("sdlgame"),
		m_initialized_game_controller(false)
	{
	}

	virtual void exit() override
	{
		sdl_joystick_module_base::exit();

		if (m_initialized_game_controller)
			SDL_QuitSubSystem(SDL_INIT_GAMEPAD);

		quit_joystick();
	}

	virtual void input_init(running_machine &machine) override
	{
		auto &sdlopts = dynamic_cast<sdl_options const &>(*options());
		bool const sixaxis_mode = sdlopts.sixaxis();

		if (!sdlopts.debug() && sdlopts.background_input())
			SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");

		init_joystick();
		if (!have_joystick())
			return;

		m_initialized_game_controller = SDL_InitSubSystem(SDL_INIT_GAMEPAD);
		if (m_initialized_game_controller)
		{
			char const *const mapfile = sdlopts.controller_mapping_file();
			if (mapfile && *mapfile && std::strcmp(mapfile, OSDOPTVAL_NONE))
			{
				auto const count = SDL_AddGamepadMappingsFromFile(mapfile);
				if (0 <= count)
					osd_printf_verbose("Game Controller: %d controller mapping(s) added from file [%s].\n", count, mapfile);
				else
					osd_printf_error("Game Controller: Error adding mappings from file [%s]: %s.\n", mapfile, SDL_GetError());
			}
		}
		else
		{
			osd_printf_warning("Could not initialize SDL Game Controller: %s.\n", SDL_GetError());
		}

		sdl_joystick_module_base::input_init(machine);

		osd_printf_verbose("Game Controller: Start initialization\n");
		int stick_count = 0;
		const auto joysticks = SDL_GetJoysticks(&stick_count);
		for (int physical_stick = 0; physical_stick < stick_count; physical_stick++)
		{
			// try to open as a game controller
			SDL_Gamepad *ctrl = nullptr;
			if (m_initialized_game_controller && SDL_IsGamepad(joysticks[physical_stick]))
			{
				ctrl = SDL_OpenGamepad(joysticks[physical_stick]);
				if (!ctrl)
					osd_printf_warning("Game Controller: Could not open SDL game controller %d: %s.\n", joysticks[physical_stick], SDL_GetError());
			}

			// fall back to joystick API if necessary
			if (!ctrl)
				create_joystick_device(joysticks[physical_stick], sixaxis_mode);
			else
				create_game_controller_device(joysticks[physical_stick], ctrl);
		}
		SDL_free(joysticks);

		constexpr int joy_event_types[] = {
				int(SDL_EVENT_JOYSTICK_AXIS_MOTION),
				int(SDL_EVENT_JOYSTICK_BALL_MOTION),
				int(SDL_EVENT_JOYSTICK_HAT_MOTION),
				int(SDL_EVENT_JOYSTICK_BUTTON_DOWN),
				int(SDL_EVENT_JOYSTICK_BUTTON_UP),
				int(SDL_EVENT_JOYSTICK_ADDED),
				int(SDL_EVENT_JOYSTICK_REMOVED) };
		constexpr int event_types[] = {
				int(SDL_EVENT_JOYSTICK_AXIS_MOTION),
				int(SDL_EVENT_JOYSTICK_BALL_MOTION),
				int(SDL_EVENT_JOYSTICK_HAT_MOTION),
				int(SDL_EVENT_JOYSTICK_BUTTON_DOWN),
				int(SDL_EVENT_JOYSTICK_BUTTON_UP),
				int(SDL_EVENT_JOYSTICK_ADDED),
				int(SDL_EVENT_JOYSTICK_REMOVED),
				int(SDL_EVENT_GAMEPAD_AXIS_MOTION),
				int(SDL_EVENT_GAMEPAD_BUTTON_DOWN),
				int(SDL_EVENT_GAMEPAD_BUTTON_UP),
				int(SDL_EVENT_GAMEPAD_ADDED),
				int(SDL_EVENT_GAMEPAD_REMOVED) };
		if (m_initialized_game_controller)
			subscribe(osd(), event_types);
		else
			subscribe(osd(), joy_event_types);

		osd_printf_verbose("Game Controller: End initialization\n");
	}

	virtual void handle_event(SDL_Event const &event) override
	{
		switch (event.type)
		{
		case SDL_EVENT_JOYSTICK_ADDED:
			{
				// make sure this isn't an event for a reconnected game controller
				auto const controller = find_joystick(event.jdevice.which);
				if (find_joystick(event.jdevice.which))
				{
					osd_printf_verbose(
							"Game Controller: Got SDL joystick added event for reconnected game controller %s [ID %s]\n",
							controller->name(),
							controller->id());
					break;
				}

				SDL_Joystick *const joy = SDL_OpenJoystick(event.jdevice.which);
				if (!joy)
				{
					osd_printf_error("Joystick: Could not open SDL joystick %d: %s.\n", event.jdevice.which, SDL_GetError());
					break;
				}

				SDL_GUID guid = SDL_GetJoystickGUID(joy);
				char const *const serial = SDL_GetJoystickSerial(joy);
				auto *const target_device = find_reconnect_match(guid, serial);
				if (target_device)
				{
					// if this downcast fails, opening as a game controller worked initially but failed on reconnection
					auto *const devinfo = dynamic_cast<sdl_joystick_device *>(target_device);
					if (devinfo)
						devinfo->attach_device(joy);
					else
						SDL_CloseJoystick(joy);
				}
				else
				{
					SDL_CloseJoystick(joy);
				}
			}
			break;

		// for devices supported by the game controller API, this is received before the corresponding SDL_EVENT_JOYSTICK_ADDED
		case SDL_EVENT_GAMEPAD_ADDED:
			if (m_initialized_game_controller)
			{
				SDL_Gamepad *const ctrl = SDL_OpenGamepad(event.cdevice.which);
				if (!ctrl)
				{
					osd_printf_error("Game Controller: Could not open SDL game controller %d: %s.\n", event.cdevice.which, SDL_GetError());
					break;
				}

				SDL_GUID guid = SDL_GetJoystickGUIDForID(event.cdevice.which);
				char const *const serial = SDL_GetGamepadSerial(ctrl);
				auto *const target_device = find_reconnect_match(guid, serial);
				if (target_device)
				{
					// downcast can fail if there was an error opening the device as a game controller the first time
					auto *const devinfo = dynamic_cast<sdl_game_controller_device *>(target_device);
					if (devinfo)
						devinfo->attach_device(ctrl);
					else
						SDL_CloseGamepad(ctrl);
				}
				else
				{
					SDL_CloseGamepad(ctrl);
				}
			}
			break;

		default:
			dispatch_joystick_event(event);
		}
	}

private:
	sdl_game_controller_device *create_game_controller_device(SDL_JoystickID sdl_id, SDL_Gamepad *ctrl)
	{
		// get basic info
		char const *const name = SDL_GetGamepadName(ctrl);
		SDL_GUID guid = SDL_GetJoystickGUIDForID(sdl_id);
		char guid_str[256];
		guid_str[0] = '\0';
		SDL_GUIDToString(guid, guid_str, sizeof(guid_str) - 1);
		char const *const serial = SDL_GetGamepadSerial(ctrl);
		std::string id(guid_str);
		if (serial)
			id.append(1, '-').append(serial);

		// print some diagnostic info
		osd_printf_verbose("Game Controller: %s [GUID %s] Vendor ID %04X, Product ID %04X, Revision %04X, Serial %s\n",
				name ? name : "<nullptr>",
				guid_str,
				SDL_GetGamepadVendor(ctrl),
				SDL_GetGamepadProduct(ctrl),
				SDL_GetGamepadProductVersion(ctrl),
				serial ? serial : "<nullptr>");
		char *const mapping = SDL_GetGamepadMapping(ctrl);
		if (mapping)
		{
			osd_printf_verbose("Game Controller:   ...  mapping [%s]\n", mapping);
			SDL_free(mapping);
		}
		else
		{
			osd_printf_verbose("Game Controller:   ...  no mapping\n");
		}

		// instantiate device
		sdl_game_controller_device &devinfo = create_device<sdl_game_controller_device>(
				DEVICE_CLASS_JOYSTICK,
				name ? name : guid_str,
				guid_str,
				ctrl,
				serial);
		return &devinfo;
	}

	bool m_initialized_game_controller;
};

} // anonymous namespace

} // namespace osd


#else // defined(SDLMAME_SDL3)

namespace osd {

namespace {
MODULE_NOT_SUPPORTED(sdl_keyboard_module, OSD_KEYBOARDINPUT_PROVIDER, "sdl")
MODULE_NOT_SUPPORTED(sdl_mouse_module, OSD_MOUSEINPUT_PROVIDER, "sdl")
MODULE_NOT_SUPPORTED(sdl_lightgun_module, OSD_LIGHTGUNINPUT_PROVIDER, "sdl")
MODULE_NOT_SUPPORTED(sdl_joystick_module, OSD_JOYSTICKINPUT_PROVIDER, "sdljoy")
MODULE_NOT_SUPPORTED(sdl_game_controller_module, OSD_JOYSTICKINPUT_PROVIDER, "sdlgame")
} // anonymous namespace

} // namespace osd

#endif // defined(SDLMAME_SDL3)

#ifdef SDLMAME_SDL3
MODULE_DEFINITION(KEYBOARDINPUT_SDL, osd::sdl_keyboard_module)
MODULE_DEFINITION(MOUSEINPUT_SDL, osd::sdl_mouse_module)
MODULE_DEFINITION(LIGHTGUNINPUT_SDL, osd::sdl_lightgun_module)
MODULE_DEFINITION(JOYSTICKINPUT_SDLJOY, osd::sdl_joystick_module)
MODULE_DEFINITION(JOYSTICKINPUT_SDLGAME, osd::sdl_game_controller_module)
#endif
