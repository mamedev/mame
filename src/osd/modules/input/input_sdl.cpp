// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont, Brad Hughes, Vas Crabb
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

#if defined(OSD_SDL)

#include "input_common.h"

#include "interface/inputseq.h"
#include "modules/lib/osdobj_common.h"
#include "sdl/osdsdl.h"

// emu
#include "emu.h"

// standard SDL header
#include <SDL2/SDL.h>

#include <algorithm>
#include <cctype>
#include <chrono>
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

[[maybe_unused]] char const *const CONTROLLER_AXIS_PS[]{
		"LSX",
		"LSY",
		"RSX",
		"RSY",
		"L2",
		"R2" };

[[maybe_unused]] char const *const CONTROLLER_AXIS_SWITCH[]{
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

[[maybe_unused]] char const *const CONTROLLER_BUTTON_XBOXONE[]{
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

[[maybe_unused]] char const *const CONTROLLER_BUTTON_PS3[]{
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

[[maybe_unused]] char const *const CONTROLLER_BUTTON_PS4[]{
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

[[maybe_unused]] char const *const CONTROLLER_BUTTON_PS5[]{
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

[[maybe_unused]] char const *const CONTROLLER_BUTTON_SWITCH[]{
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
			keyboard_trans_table const &trans_table) :
		sdl_device(std::move(name), std::move(id), module),
		m_trans_table(trans_table),
		m_keyboard({{0}}),
		m_capslock_pressed(std::chrono::steady_clock::time_point::min())
	{
	}

	virtual void poll() override
	{
		sdl_device::poll();

#ifdef __APPLE__
		if (m_keyboard.state[SDL_SCANCODE_CAPSLOCK] && (std::chrono::steady_clock::now() > (m_capslock_pressed + std::chrono::milliseconds(30))))
			m_keyboard.state[SDL_SCANCODE_CAPSLOCK] = 0x00;
#endif
	}

	virtual void process_event(SDL_Event const &event) override
	{
		switch (event.type)
		{
		case SDL_KEYDOWN:
			if (event.key.keysym.scancode == SDL_SCANCODE_CAPSLOCK)
				m_capslock_pressed = std::chrono::steady_clock::now();

			m_keyboard.state[event.key.keysym.scancode] = 0x80;
			break;

		case SDL_KEYUP:
#ifdef __APPLE__
			if (event.key.keysym.scancode == SDL_SCANCODE_CAPSLOCK)
				break;
#endif

			m_keyboard.state[event.key.keysym.scancode] = 0x00;
			break;
		}
	}

	virtual void reset() override
	{
		memset(&m_keyboard.state, 0, sizeof(m_keyboard.state));
		m_capslock_pressed = std::chrono::steady_clock::time_point::min();
	}

	virtual void configure(input_device &device) override
	{
		// populate it
		for (int keynum = 0; m_trans_table[keynum].mame_key != ITEM_ID_INVALID; keynum++)
		{
			input_item_id itemid = m_trans_table[keynum].mame_key;

			// generate the default / modified name
			char defname[20];
			snprintf(defname, sizeof(defname) - 1, "%s", m_trans_table[keynum].ui_name);

			device.add_item(
					defname,
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

	keyboard_trans_table const &m_trans_table;
	keyboard_state m_keyboard;
	std::chrono::steady_clock::time_point m_capslock_pressed;
};


//============================================================
//  sdl_mouse_device
//============================================================

class sdl_mouse_device : public sdl_device
{
public:
	sdl_mouse_device(std::string &&name, std::string &&id, input_module &module) :
		sdl_device(std::move(name), std::move(id), module),
		m_mouse({0})
	{
	}

	virtual void poll() override
	{
		m_mouse.lX = 0;
		m_mouse.lY = 0;
		m_mouse.lV = 0;
		m_mouse.lH = 0;
		sdl_device::poll();
	}

	virtual void reset() override
	{
		memset(&m_mouse, 0, sizeof(m_mouse));
	}

	virtual void configure(input_device &device) override
	{
		// add the axes
		device.add_item(
				"X",
				ITEM_ID_XAXIS,
				generic_axis_get_state<s32>,
				&m_mouse.lX);
		device.add_item(
				"Y",
				ITEM_ID_YAXIS,
				generic_axis_get_state<s32>,
				&m_mouse.lY);
		device.add_item(
				"Scroll V",
				ITEM_ID_ZAXIS,
				generic_axis_get_state<s32>,
				&m_mouse.lV);
		device.add_item(
				"Scroll H",
				ITEM_ID_RZAXIS,
				generic_axis_get_state<s32>,
				&m_mouse.lH);

		// add the buttons
		for (int button = 0; button < 4; button++)
		{
			input_item_id itemid = (input_item_id)(ITEM_ID_BUTTON1 + button);
			int const offset = button ^ (((1 == button) || (2 == button)) ? 3 : 0);
			device.add_item(
					default_button_name(button),
					itemid,
					generic_button_get_state<s32>,
					&m_mouse.buttons[offset]);
		}
	}

	virtual void process_event(SDL_Event const &event) override
	{
		switch (event.type)
		{
		case SDL_MOUSEMOTION:
			m_mouse.lX += event.motion.xrel * INPUT_RELATIVE_PER_PIXEL;
			m_mouse.lY += event.motion.yrel * INPUT_RELATIVE_PER_PIXEL;
			break;

		case SDL_MOUSEBUTTONDOWN:
			m_mouse.buttons[event.button.button - 1] = 0x80;
			break;

		case SDL_MOUSEBUTTONUP:
			m_mouse.buttons[event.button.button - 1] = 0;
			break;

		case SDL_MOUSEWHEEL:
#if SDL_VERSION_ATLEAST(2, 0, 18)
			m_mouse.lV += event.wheel.preciseY * INPUT_RELATIVE_PER_PIXEL;
			m_mouse.lH += event.wheel.preciseX * INPUT_RELATIVE_PER_PIXEL;
#else
			m_mouse.lV += event.wheel.y * INPUT_RELATIVE_PER_PIXEL;
			m_mouse.lH += event.wheel.x * INPUT_RELATIVE_PER_PIXEL;
#endif
			break;
		}
	}

private:
	// state information for a mouse
	struct mouse_state
	{
		s32 lX, lY, lV, lH;
		s32 buttons[MAX_BUTTONS];
	};

	mouse_state m_mouse;
};


//============================================================
//  sdl_joystick_device_base
//============================================================

class sdl_joystick_device_base : public sdl_device
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
		m_hapdevice(SDL_HapticOpenFromJoystick(joy))
	{
		set_instance(SDL_JoystickInstanceID(joy));
	}

	virtual void configure(input_device &device) override
	{
		char tempname[32];

		int const axiscount = SDL_JoystickNumAxes(m_joydevice);
		int const buttoncount = SDL_JoystickNumButtons(m_joydevice);
		int const hatcount = SDL_JoystickNumHats(m_joydevice);
		int const ballcount = SDL_JoystickNumBalls(m_joydevice);

		// loop over all axes
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
			device.add_item(
					tempname,
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

			device.add_item(
					default_button_name(button),
					itemid,
					generic_button_get_state<s32>,
					&m_joystick.buttons[button]);
		}

		// loop over all hats
		for (int hat = 0; (hat < MAX_HATS) && (hat < hatcount); hat++)
		{
			input_item_id itemid;

			snprintf(tempname, sizeof(tempname), "Hat %d Up", hat + 1);
			itemid = input_item_id((hat < INPUT_MAX_HATS) ? ITEM_ID_HAT1UP + (4 * hat) : ITEM_ID_OTHER_SWITCH);
			device.add_item(
					tempname,
					itemid,
					generic_button_get_state<s32>,
					&m_joystick.hatsU[hat]);

			snprintf(tempname, sizeof(tempname), "Hat %d Down", hat + 1);
			itemid = input_item_id((hat < INPUT_MAX_HATS) ? ITEM_ID_HAT1DOWN + (4 * hat) : ITEM_ID_OTHER_SWITCH);
			device.add_item(
					tempname,
					itemid,
					generic_button_get_state<s32>,
					&m_joystick.hatsD[hat]);

			snprintf(tempname, sizeof(tempname), "Hat %d Left", hat + 1);
			itemid = input_item_id((hat < INPUT_MAX_HATS) ? ITEM_ID_HAT1LEFT + (4 * hat) : ITEM_ID_OTHER_SWITCH);
			device.add_item(
					tempname,
					itemid,
					generic_button_get_state<s32>,
					&m_joystick.hatsL[hat]);

			snprintf(tempname, sizeof(tempname), "Hat %d Right", hat + 1);
			itemid = input_item_id((hat < INPUT_MAX_HATS) ? ITEM_ID_HAT1RIGHT + (4 * hat) : ITEM_ID_OTHER_SWITCH);
			device.add_item(
					tempname,
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
			device.add_item(
					tempname,
					input_item_id(itemid),
					generic_axis_get_state<s32>,
					&m_joystick.balls[ball * 2]);

			snprintf(tempname, sizeof(tempname), "R%d Y", ball + 1);
			device.add_item(
					tempname,
					input_item_id(itemid + 1),
					generic_axis_get_state<s32>,
					&m_joystick.balls[ball * 2 + 1]);
		}
	}

	~sdl_joystick_device()
	{
		if (m_joydevice)
		{
			if (m_hapdevice)
			{
				SDL_HapticClose(m_hapdevice);
				m_hapdevice = nullptr;
			}
			SDL_JoystickClose(m_joydevice);
			m_joydevice = nullptr;
		}
	}

	virtual void reset() override
	{
		memset(&m_joystick, 0, sizeof(m_joystick));
	}

	virtual void process_event(SDL_Event const &event) override
	{
		switch (event.type)
		{
		case SDL_JOYAXISMOTION:
			if (event.jaxis.axis < MAX_AXES)
				m_joystick.axes[event.jaxis.axis] = (event.jaxis.value * 2);
			break;

		case SDL_JOYBALLMOTION:
			//printf("Ball %d %d\n", event.jball.xrel, event.jball.yrel);
			if (event.jball.ball < (MAX_AXES / 2))
			{
				m_joystick.balls[event.jball.ball * 2] = event.jball.xrel * INPUT_RELATIVE_PER_PIXEL;
				m_joystick.balls[event.jball.ball * 2 + 1] = event.jball.yrel * INPUT_RELATIVE_PER_PIXEL;
			}
			break;

		case SDL_JOYHATMOTION:
			if (event.jhat.hat < MAX_HATS)
			{
				m_joystick.hatsU[event.jhat.hat] = (event.jhat.value & SDL_HAT_UP) ? 0x80 : 0;
				m_joystick.hatsD[event.jhat.hat] = (event.jhat.value & SDL_HAT_DOWN) ? 0x80 : 0;
				m_joystick.hatsL[event.jhat.hat] = (event.jhat.value & SDL_HAT_LEFT) ? 0x80 : 0;
				m_joystick.hatsR[event.jhat.hat] = (event.jhat.value & SDL_HAT_RIGHT) ? 0x80 : 0;
			}
			break;

		case SDL_JOYBUTTONDOWN:
		case SDL_JOYBUTTONUP:
			if (event.jbutton.button < MAX_BUTTONS)
				m_joystick.buttons[event.jbutton.button] = (event.jbutton.state == SDL_PRESSED) ? 0x80 : 0;
			break;

		case SDL_JOYDEVICEREMOVED:
			osd_printf_verbose("Joystick: %s [ID %s] disconnected\n", name(), id());
			clear_instance();
			reset();
			if (m_joydevice)
			{
				if (m_hapdevice)
				{
					SDL_HapticClose(m_hapdevice);
					m_hapdevice = nullptr;
				}
				SDL_JoystickClose(m_joydevice);
				m_joydevice = nullptr;
			}
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

		set_instance(SDL_JoystickInstanceID(joy));
		m_joydevice = joy;
		m_hapdevice = SDL_HapticOpenFromJoystick(joy);

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
		case SDL_JOYAXISMOTION:
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
			SDL_GameController *ctrl,
			char const *serial) :
		sdl_joystick_device_base(
				std::move(name),
				std::move(id),
				module,
				serial),
		m_controller({{0}}),
		m_ctrldevice(ctrl)
	{
		set_instance(SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(ctrl)));
	}

	~sdl_game_controller_device()
	{
		if (m_ctrldevice)
		{
			SDL_GameControllerClose(m_ctrldevice);
			m_ctrldevice = nullptr;
		}
	}

	virtual void configure(input_device &device) override
	{
		char const *const *axisnames = CONTROLLER_AXIS_XBOX;
		char const *const *buttonnames = CONTROLLER_BUTTON_XBOX360;
		bool digitaltriggers = false;
#if SDL_VERSION_ATLEAST(2, 0, 12)
		auto const ctrltype = SDL_GameControllerGetType(m_ctrldevice);
		switch (ctrltype)
		{
		case SDL_CONTROLLER_TYPE_UNKNOWN:
			osd_printf_verbose("Game Controller:   ...  unknown type\n", int(ctrltype));
			break;
		case SDL_CONTROLLER_TYPE_XBOX360:
			osd_printf_verbose("Game Controller:   ...  Xbox 360 type\n");
			axisnames = CONTROLLER_AXIS_XBOX;
			buttonnames = CONTROLLER_BUTTON_XBOX360;
			break;
		case SDL_CONTROLLER_TYPE_XBOXONE:
			osd_printf_verbose("Game Controller:   ...  Xbox One type\n");
			axisnames = CONTROLLER_AXIS_XBOX;
			buttonnames = CONTROLLER_BUTTON_XBOXONE;
			break;
		case SDL_CONTROLLER_TYPE_PS3:
			osd_printf_verbose("Game Controller:   ...  PlayStation 3 type\n");
			axisnames = CONTROLLER_AXIS_PS;
			buttonnames = CONTROLLER_BUTTON_PS3;
			break;
		case SDL_CONTROLLER_TYPE_PS4:
			osd_printf_verbose("Game Controller:   ...  PlayStation 4 type\n");
			axisnames = CONTROLLER_AXIS_PS;
			buttonnames = CONTROLLER_BUTTON_PS4;
			break;
		case SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_PRO:
			osd_printf_verbose("Game Controller:   ...  Switch Pro Controller type\n");
			axisnames = CONTROLLER_AXIS_SWITCH;
			buttonnames = CONTROLLER_BUTTON_SWITCH;
			digitaltriggers = true;
			break;
#if SDL_VERSION_ATLEAST(2, 0, 14)
		//case SDL_CONTROLLER_TYPE_VIRTUAL:
		case SDL_CONTROLLER_TYPE_PS5:
			osd_printf_verbose("Game Controller:   ...  PlayStation 5 type\n");
			axisnames = CONTROLLER_AXIS_PS;
			buttonnames = CONTROLLER_BUTTON_PS5;
			break;
#endif
#if SDL_VERSION_ATLEAST(2, 0, 16)
		//case SDL_CONTROLLER_TYPE_AMAZON_LUNA:
		case SDL_CONTROLLER_TYPE_GOOGLE_STADIA:
			osd_printf_verbose("Game Controller:   ...  Google Stadia type\n");
			axisnames = CONTROLLER_AXIS_PS;
			buttonnames = CONTROLLER_BUTTON_STADIA;
			break;
#endif
#if SDL_VERSION_ATLEAST(2, 24, 0)
		case SDL_CONTROLLER_TYPE_NVIDIA_SHIELD:
			osd_printf_verbose("Game Controller:   ...  NVIDIA Shield type\n");
			axisnames = CONTROLLER_AXIS_XBOX;
			buttonnames = CONTROLLER_BUTTON_SHIELD;
			break;
		//case SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_JOYCON_LEFT:
		//case SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_JOYCON_RIGHT:
		case SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_JOYCON_PAIR:
			osd_printf_verbose("Game Controller:   ...  Joy-Con pair type\n");
			axisnames = CONTROLLER_AXIS_SWITCH;
			buttonnames = CONTROLLER_BUTTON_SWITCH;
			digitaltriggers = true;
			break;
#endif
		default: // default to Xbox 360 names
			osd_printf_verbose("Game Controller:   ...  unrecognized type (%d)\n", int(ctrltype));
			break;
		}
#endif

		// add axes
		std::tuple<SDL_GameControllerAxis, input_item_id, bool> const axes[]{
				{ SDL_CONTROLLER_AXIS_LEFTX,        ITEM_ID_XAXIS,   false },
				{ SDL_CONTROLLER_AXIS_LEFTY,        ITEM_ID_YAXIS,   false },
				{ SDL_CONTROLLER_AXIS_RIGHTX,       ITEM_ID_ZAXIS,   false },
				{ SDL_CONTROLLER_AXIS_RIGHTY,       ITEM_ID_RZAXIS,  false },
				{ SDL_CONTROLLER_AXIS_TRIGGERLEFT,  ITEM_ID_SLIDER1, true },
				{ SDL_CONTROLLER_AXIS_TRIGGERRIGHT, ITEM_ID_SLIDER2, true } };
		for (auto [axis, item, buttontest] : axes)
		{
			bool avail = !buttontest || !digitaltriggers;
#if SDL_VERSION_ATLEAST(2, 0, 14)
			avail = avail && SDL_GameControllerHasAxis(m_ctrldevice, axis);
#endif
			if (avail)
			{
				auto const binding = SDL_GameControllerGetBindForAxis(m_ctrldevice, axis);
				switch (binding.bindType)
				{
				case SDL_CONTROLLER_BINDTYPE_NONE:
					avail = false;
					break;
				case SDL_CONTROLLER_BINDTYPE_BUTTON:
					if (buttontest)
						avail = false;
					break;
				default:
					break;
				}
			}
			if (avail)
			{
				device.add_item(
						axisnames[axis],
						item,
						generic_axis_get_state<s32>,
						&m_controller.axes[axis]);
			}
		}

		// add automatically numbered buttons
		std::tuple<SDL_GameControllerButton, SDL_GameControllerAxis> const generalbuttons[]{
				{ SDL_CONTROLLER_BUTTON_A,             SDL_CONTROLLER_AXIS_INVALID },
				{ SDL_CONTROLLER_BUTTON_B,             SDL_CONTROLLER_AXIS_INVALID },
				{ SDL_CONTROLLER_BUTTON_X,             SDL_CONTROLLER_AXIS_INVALID },
				{ SDL_CONTROLLER_BUTTON_Y,             SDL_CONTROLLER_AXIS_INVALID },
				{ SDL_CONTROLLER_BUTTON_LEFTSHOULDER,  SDL_CONTROLLER_AXIS_INVALID },
				{ SDL_CONTROLLER_BUTTON_RIGHTSHOULDER, SDL_CONTROLLER_AXIS_INVALID },
				{ SDL_CONTROLLER_BUTTON_INVALID,       SDL_CONTROLLER_AXIS_TRIGGERLEFT },
				{ SDL_CONTROLLER_BUTTON_INVALID,       SDL_CONTROLLER_AXIS_TRIGGERRIGHT },
				{ SDL_CONTROLLER_BUTTON_LEFTSTICK,     SDL_CONTROLLER_AXIS_INVALID },
				{ SDL_CONTROLLER_BUTTON_RIGHTSTICK,    SDL_CONTROLLER_AXIS_INVALID },
#if SDL_VERSION_ATLEAST(2, 0, 14)
				{ SDL_CONTROLLER_BUTTON_PADDLE1,       SDL_CONTROLLER_AXIS_INVALID },
				{ SDL_CONTROLLER_BUTTON_PADDLE2,       SDL_CONTROLLER_AXIS_INVALID },
				{ SDL_CONTROLLER_BUTTON_PADDLE3,       SDL_CONTROLLER_AXIS_INVALID },
				{ SDL_CONTROLLER_BUTTON_PADDLE4,       SDL_CONTROLLER_AXIS_INVALID },
#endif
				{ SDL_CONTROLLER_BUTTON_GUIDE,         SDL_CONTROLLER_AXIS_INVALID },
#if SDL_VERSION_ATLEAST(2, 0, 14)
				{ SDL_CONTROLLER_BUTTON_MISC1,         SDL_CONTROLLER_AXIS_INVALID },
				{ SDL_CONTROLLER_BUTTON_TOUCHPAD,      SDL_CONTROLLER_AXIS_INVALID },
#endif
				};
		input_item_id button_item = ITEM_ID_BUTTON1;
		for (auto [button, axis] : generalbuttons)
		{
			bool avail = true;
			if (SDL_CONTROLLER_BUTTON_INVALID != button)
			{
#if SDL_VERSION_ATLEAST(2, 0, 14)
				avail = SDL_GameControllerHasButton(m_ctrldevice, button);
#endif
				if (avail)
				{
					auto const binding = SDL_GameControllerGetBindForButton(m_ctrldevice, button);
					switch (binding.bindType)
					{
					case SDL_CONTROLLER_BINDTYPE_NONE:
						avail = false;
						break;
					default:
						break;
					}
				}
				if (avail)
				{
					device.add_item(
							buttonnames[button],
							button_item++,
							generic_button_get_state<s32>,
							&m_controller.buttons[button]);
				}
			}
			else
			{
#if SDL_VERSION_ATLEAST(2, 0, 14)
				avail = SDL_GameControllerHasAxis(m_ctrldevice, axis);
#endif
				if (avail)
				{
					auto const binding = SDL_GameControllerGetBindForAxis(m_ctrldevice, axis);
					switch (binding.bindType)
					{
					case SDL_CONTROLLER_BINDTYPE_NONE:
						avail = false;
						break;
					case SDL_CONTROLLER_BINDTYPE_BUTTON:
						break;
					default:
						avail = digitaltriggers;
					}
				}
				if (avail)
				{
					device.add_item(
							axisnames[axis],
							button_item++,
							[] (void *device_internal, void *item_internal) -> int
							{
								static_cast<device_info *>(device_internal)->module().poll_if_necessary();
								return (*reinterpret_cast<s32 const *>(item_internal) <= -16'384) ? 1 : 0;
							},
							&m_controller.axes[axis]);
				}
			}
		}

		// add buttons with fixed item IDs
		std::pair<SDL_GameControllerButton, input_item_id> const fixedbuttons[]{
				{ SDL_CONTROLLER_BUTTON_BACK,       ITEM_ID_SELECT },
				{ SDL_CONTROLLER_BUTTON_START,      ITEM_ID_START },
				{ SDL_CONTROLLER_BUTTON_DPAD_UP,    ITEM_ID_HAT1UP },
				{ SDL_CONTROLLER_BUTTON_DPAD_DOWN,  ITEM_ID_HAT1DOWN },
				{ SDL_CONTROLLER_BUTTON_DPAD_LEFT,  ITEM_ID_HAT1LEFT },
				{ SDL_CONTROLLER_BUTTON_DPAD_RIGHT, ITEM_ID_HAT1RIGHT } };
		for (auto [button, item] : fixedbuttons)
		{
			bool avail = true;
#if SDL_VERSION_ATLEAST(2, 0, 14)
			avail = SDL_GameControllerHasButton(m_ctrldevice, button);
#endif
			if (avail)
			{
				auto const binding = SDL_GameControllerGetBindForButton(m_ctrldevice, button);
				switch (binding.bindType)
				{
				case SDL_CONTROLLER_BINDTYPE_NONE:
					avail = false;
					break;
				default:
					break;
				}
			}
			if (avail)
			{
				device.add_item(
						buttonnames[button],
						item,
						generic_button_get_state<s32>,
						&m_controller.buttons[button]);
			}
		}
	}

	virtual void reset() override
	{
		memset(&m_controller, 0, sizeof(m_controller));
	}

	virtual void process_event(SDL_Event const &event) override
	{
		switch (event.type)
		{
		case SDL_CONTROLLERAXISMOTION:
			if (event.caxis.axis < SDL_CONTROLLER_AXIS_MAX)
			{
				switch (event.caxis.axis)
				{
				case SDL_CONTROLLER_AXIS_TRIGGERLEFT: // MAME wants negative values for triggers
				case SDL_CONTROLLER_AXIS_TRIGGERRIGHT:
					m_controller.axes[event.caxis.axis] = -normalize_absolute_axis(event.caxis.value, -32'767, 32'767);
					break;
				default:
					m_controller.axes[event.caxis.axis] = normalize_absolute_axis(event.caxis.value, -32'767, 32'767);
				}
			}
			break;

		case SDL_CONTROLLERBUTTONDOWN:
		case SDL_CONTROLLERBUTTONUP:
			if (event.cbutton.button < SDL_CONTROLLER_BUTTON_MAX)
				m_controller.buttons[event.cbutton.button] = (event.cbutton.state == SDL_PRESSED) ? 0x80 : 0x00;
			break;

		case SDL_CONTROLLERDEVICEREMOVED:
			osd_printf_verbose("Game Controller: %s [ID %s] disconnected\n", name(), id());
			clear_instance();
			reset();
			if (m_ctrldevice)
			{
				SDL_GameControllerClose(m_ctrldevice);
				m_ctrldevice = nullptr;
			}
			break;
		}
	}

	void attach_device(SDL_GameController *ctrl)
	{
		assert(ctrl);
		assert(!m_ctrldevice);

		set_instance(SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(ctrl)));
		m_ctrldevice = ctrl;

		osd_printf_verbose("Game Controller: %s [ID %s] reconnected\n", name(), id());
	}

private:
	// state information for a game controller
	struct sdl_controller_state
	{
		s32 axes[SDL_CONTROLLER_AXIS_MAX];
		s32 buttons[SDL_CONTROLLER_BUTTON_MAX];
	};

	sdl_controller_state m_controller;
	SDL_GameController *m_ctrldevice;
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
		sdl_input_module<sdl_keyboard_device>(OSD_KEYBOARDINPUT_PROVIDER, "sdl"),
		m_key_trans_table(nullptr)
	{
	}

	virtual void input_init(running_machine &machine) override
	{
		sdl_input_module<sdl_keyboard_device>::input_init(machine);

		static int const event_types[] = {
				int(SDL_KEYDOWN),
				int(SDL_KEYUP) };

		subscribe(osd(), event_types);

		// Read our keymap and store a pointer to our table
		m_key_trans_table = sdlinput_read_keymap();

		osd_printf_verbose("Keyboard: Start initialization\n");

		// SDL only has 1 keyboard add it now
		auto &devinfo = create_device<sdl_keyboard_device>(
				DEVICE_CLASS_KEYBOARD,
				"System keyboard",
				"System keyboard",
				*m_key_trans_table);

		osd_printf_verbose("Keyboard: Registered %s\n", devinfo.name());
		osd_printf_verbose("Keyboard: End initialization\n");
	}

private:
	keyboard_trans_table *sdlinput_read_keymap()
	{
		keyboard_trans_table &default_table = keyboard_trans_table::instance();

		if (!options()->bool_value(SDLOPTION_KEYMAP))
			return &default_table;

		const char *const keymap_filename = dynamic_cast<sdl_options const &>(*options()).keymap_file();
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

	keyboard_trans_table *m_key_trans_table;
	std::unique_ptr<keyboard_trans_table> m_custom_table;
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

		static int const event_types[] = {
				int(SDL_MOUSEMOTION),
				int(SDL_MOUSEBUTTONDOWN),
				int(SDL_MOUSEBUTTONUP),
				int(SDL_MOUSEWHEEL) };

		subscribe(osd(), event_types);

		osd_printf_verbose("Mouse: Start initialization\n");

		// SDL currently only supports one mouse
		auto &devinfo = create_device<sdl_mouse_device>(
				DEVICE_CLASS_MOUSE,
				"System mouse",
				"System mouse");

		osd_printf_verbose("Mouse: Registered %s\n", devinfo.name());
		osd_printf_verbose("Mouse: End initialization\n");
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

		m_initialized_joystick = !SDL_InitSubSystem(SDL_INIT_JOYSTICK);
		if (!m_initialized_joystick)
		{
			osd_printf_error("Could not initialize SDL Joystick subsystem: %s.\n", SDL_GetError());
			return;
		}

		m_initialized_haptic = !SDL_InitSubSystem(SDL_INIT_HAPTIC);
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

	sdl_joystick_device *create_joystick_device(int index, bool sixaxis)
	{
		// open the joystick device
		SDL_Joystick *const joy = SDL_JoystickOpen(index);
		if (!joy)
		{
			osd_printf_error("Joystick: Could not open SDL joystick %d: %s.\n", index, SDL_GetError());
			return nullptr;
		}

		// get basic info
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

		// print some diagnostic info
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
		if (SDL_JoystickNumButtons(joy) > MAX_BUTTONS)
			osd_printf_verbose("Joystick:   ...  Has %d buttons which exceeds supported %d buttons\n", SDL_JoystickNumButtons(joy), MAX_BUTTONS);

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

	device_info *find_reconnect_match(SDL_JoystickGUID const &guid, char const *serial)
	{
		char guid_str[256];
		guid_str[0] = '\0';
		SDL_JoystickGetGUIDString(guid, guid_str, sizeof(guid_str) - 1);
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
		SDL_SetHint(SDL_HINT_ACCELEROMETER_AS_JOYSTICK, "0");

		init_joystick();
		if (!have_joystick())
			return;

		sdl_joystick_module_base::input_init(machine);

		osd_printf_verbose("Joystick: Start initialization\n");
		for (int physical_stick = 0; physical_stick < SDL_NumJoysticks(); physical_stick++)
			create_joystick_device(physical_stick, sixaxis_mode);

		static int const event_types[] = {
				int(SDL_JOYAXISMOTION),
				int(SDL_JOYBALLMOTION),
				int(SDL_JOYHATMOTION),
				int(SDL_JOYBUTTONDOWN),
				int(SDL_JOYBUTTONUP),
				int(SDL_JOYDEVICEADDED),
				int(SDL_JOYDEVICEREMOVED) };
		subscribe(osd(), event_types);

		osd_printf_verbose("Joystick: End initialization\n");
	}

	virtual void handle_event(SDL_Event const &event) override
	{
		if (SDL_JOYDEVICEADDED == event.type)
		{
			SDL_Joystick *const joy = SDL_JoystickOpen(event.jdevice.which);
			if (!joy)
			{
				osd_printf_error("Joystick: Could not open SDL joystick %d: %s.\n", event.jdevice.which, SDL_GetError());
			}
			else
			{
				SDL_JoystickGUID guid = SDL_JoystickGetGUID(joy);
				char const *serial = nullptr;
#if SDL_VERSION_ATLEAST(2, 0, 14)
				serial = SDL_JoystickGetSerial(joy);
#endif
				auto *const target_device = find_reconnect_match(guid, serial);
				if (target_device)
				{
					auto &devinfo = dynamic_cast<sdl_joystick_device &>(*target_device);
					devinfo.attach_device(joy);
				}
				else
				{
					SDL_JoystickClose(joy);
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
			SDL_QuitSubSystem(SDL_INIT_GAMECONTROLLER);

		quit_joystick();
	}

	virtual void input_init(running_machine &machine) override
	{
		auto &sdlopts = dynamic_cast<sdl_options const &>(*options());
		bool const sixaxis_mode = sdlopts.sixaxis();

		if (!sdlopts.debug() && sdlopts.background_input())
			SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");
		SDL_SetHint(SDL_HINT_ACCELEROMETER_AS_JOYSTICK, "0");

		init_joystick();
		if (!have_joystick())
			return;

		m_initialized_game_controller = !SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER);
		if (m_initialized_game_controller)
		{
			char const *const mapfile = sdlopts.controller_mapping_file();
			if (mapfile && *mapfile && std::strcmp(mapfile, OSDOPTVAL_NONE))
			{
				auto const count = SDL_GameControllerAddMappingsFromFile(mapfile);
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
		for (int physical_stick = 0; physical_stick < SDL_NumJoysticks(); physical_stick++)
		{
			// try to open as a game controller
			SDL_GameController *ctrl = nullptr;
			if (m_initialized_game_controller && SDL_IsGameController(physical_stick))
			{
				ctrl = SDL_GameControllerOpen(physical_stick);
				if (!ctrl)
					osd_printf_warning("Game Controller: Could not open SDL game controller %d: %s.\n", physical_stick, SDL_GetError());
			}

			// fall back to joystick API if necessary
			if (!ctrl)
				create_joystick_device(physical_stick, sixaxis_mode);
			else
				create_game_controller_device(physical_stick, ctrl);
		}

		static int const joy_event_types[] = {
				int(SDL_JOYAXISMOTION),
				int(SDL_JOYBALLMOTION),
				int(SDL_JOYHATMOTION),
				int(SDL_JOYBUTTONDOWN),
				int(SDL_JOYBUTTONUP),
				int(SDL_JOYDEVICEADDED),
				int(SDL_JOYDEVICEREMOVED) };
		static int const event_types[] = {
				int(SDL_JOYAXISMOTION),
				int(SDL_JOYBALLMOTION),
				int(SDL_JOYHATMOTION),
				int(SDL_JOYBUTTONDOWN),
				int(SDL_JOYBUTTONUP),
				int(SDL_JOYDEVICEADDED),
				int(SDL_JOYDEVICEREMOVED),
				int(SDL_CONTROLLERAXISMOTION),
				int(SDL_CONTROLLERBUTTONDOWN),
				int(SDL_CONTROLLERBUTTONUP),
				int(SDL_CONTROLLERDEVICEADDED),
				int(SDL_CONTROLLERDEVICEREMOVED) };
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
		case SDL_JOYDEVICEADDED:
			{
				// make sure this isn't an event for a reconnected game controller
				auto const controller = find_joystick(SDL_JoystickGetDeviceInstanceID(event.jdevice.which));
				if (find_joystick(SDL_JoystickGetDeviceInstanceID(event.jdevice.which)))
				{
					osd_printf_verbose(
							"Game Controller: Got SDL joystick added event for reconnected game controller %s [ID %s]\n",
							controller->name(),
							controller->id());
					break;
				}

				SDL_Joystick *const joy = SDL_JoystickOpen(event.jdevice.which);
				if (!joy)
				{
					osd_printf_error("Joystick: Could not open SDL joystick %d: %s.\n", event.jdevice.which, SDL_GetError());
					break;
				}

				SDL_JoystickGUID guid = SDL_JoystickGetGUID(joy);
				char const *serial = nullptr;
#if SDL_VERSION_ATLEAST(2, 0, 14)
				serial = SDL_JoystickGetSerial(joy);
#endif
				auto *const target_device = find_reconnect_match(guid, serial);
				if (target_device)
				{
					// if this downcast fails, opening as a game controller worked initially but failed on reconnection
					auto *const devinfo = dynamic_cast<sdl_joystick_device *>(target_device);
					if (devinfo)
						devinfo->attach_device(joy);
					else
						SDL_JoystickClose(joy);
				}
				else
				{
					SDL_JoystickClose(joy);
				}
			}
			break;

		// for devices supported by the game controller API, this is received before the corresponding SDL_JOYDEVICEADDED
		case SDL_CONTROLLERDEVICEADDED:
			if (m_initialized_game_controller)
			{
				SDL_GameController *const ctrl = SDL_GameControllerOpen(event.cdevice.which);
				if (!ctrl)
				{
					osd_printf_error("Game Controller: Could not open SDL game controller %d: %s.\n", event.cdevice.which, SDL_GetError());
					break;
				}

				SDL_JoystickGUID guid = SDL_JoystickGetDeviceGUID(event.cdevice.which);
				char const *serial = nullptr;
#if SDL_VERSION_ATLEAST(2, 0, 14)
				serial = SDL_GameControllerGetSerial(ctrl);
#endif
				auto *const target_device = find_reconnect_match(guid, serial);
				if (target_device)
				{
					// downcast can fail if there was an error opening the device as a game controller the first time
					auto *const devinfo = dynamic_cast<sdl_game_controller_device *>(target_device);
					if (devinfo)
						devinfo->attach_device(ctrl);
					else
						SDL_GameControllerClose(ctrl);
				}
				else
				{
					SDL_GameControllerClose(ctrl);
				}
			}
			break;

		default:
			dispatch_joystick_event(event);
		}
	}

private:
	sdl_game_controller_device *create_game_controller_device(int index, SDL_GameController *ctrl)
	{
		// get basic info
		char const *const name = SDL_GameControllerName(ctrl);
		SDL_JoystickGUID guid = SDL_JoystickGetDeviceGUID(index);
		char guid_str[256];
		guid_str[0] = '\0';
		SDL_JoystickGetGUIDString(guid, guid_str, sizeof(guid_str) - 1);
		char const *serial = nullptr;
#if SDL_VERSION_ATLEAST(2, 0, 14)
		serial = SDL_GameControllerGetSerial(ctrl);
#endif
		std::string id(guid_str);
		if (serial)
			id.append(1, '-').append(serial);

		// print some diagnostic info
		osd_printf_verbose("Game Controller: %s [GUID %s] Vendor ID %04X, Product ID %04X, Revision %04X, Serial %s\n",
				name ? name : "<nullptr>",
				guid_str,
				SDL_GameControllerGetVendor(ctrl),
				SDL_GameControllerGetProduct(ctrl),
				SDL_GameControllerGetProductVersion(ctrl),
				serial ? serial : "<nullptr>");
		char *const mapping = SDL_GameControllerMapping(ctrl);
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


#else // defined(SDLMAME_SDL2)

namespace osd {

namespace {

MODULE_NOT_SUPPORTED(sdl_keyboard_module, OSD_KEYBOARDINPUT_PROVIDER, "sdl")
MODULE_NOT_SUPPORTED(sdl_mouse_module, OSD_MOUSEINPUT_PROVIDER, "sdl")
MODULE_NOT_SUPPORTED(sdl_joystick_module, OSD_JOYSTICKINPUT_PROVIDER, "sdljoy")
MODULE_NOT_SUPPORTED(sdl_game_controller_module, OSD_JOYSTICKINPUT_PROVIDER, "sdlgame")

} // anonymous namespace

} // namespace osd

#endif // defined(SDLMAME_SDL2)


MODULE_DEFINITION(KEYBOARDINPUT_SDL, osd::sdl_keyboard_module)
MODULE_DEFINITION(MOUSEINPUT_SDL, osd::sdl_mouse_module)
MODULE_DEFINITION(JOYSTICKINPUT_SDLJOY, osd::sdl_joystick_module)
MODULE_DEFINITION(JOYSTICKINPUT_SDLGAME, osd::sdl_game_controller_module)
