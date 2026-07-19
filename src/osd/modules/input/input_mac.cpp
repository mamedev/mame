// license:BSD-3-Clause
// copyright-holders:R. Belmont
//============================================================
//
//  input_mac.cpp - Mac input
//
//  Mac OSD by R. Belmont
//
//============================================================

#include "input_module.h"
#include "modules/osdmodule.h"

#if defined(OSD_MAC)

// System headers
#include <cctype>
#include <cstddef>
#include <mutex>
#include <memory>
#include <algorithm>

// MAME headers
#include "emu.h"
#include "osdepend.h"
#include "render.h"
#include "inpttype.h"
#include "ui/uimain.h"
#include "uiinput.h"
#include "window.h"
#include "strconv.h"

#include "../../mac/osdmac.h"
#include "input_common.h"

#include "util/language.h"

// implemented in windowcontroller.mm
extern int MacPointerInWindow();


namespace osd {

namespace {

//============================================================
//  event records queued from the Cocoa side
//============================================================

struct mac_key_event
{
	int         vk;         // Mac virtual keycode
	bool        down;
};

struct mac_mouse_event
{
	enum class kind { BUTTON, MOTION, WHEEL };

	kind        type;
	int         button;     // BUTTON: 0 = left, 1 = right, 2 = middle, ...
	bool        down;       // BUTTON only
	float       dx, dy;     // MOTION: relative pixels; WHEEL: vertical/horizontal detents
};


//============================================================
//  Mac virtual keycode to MAME item ID table (ANSI layout)
//============================================================

struct mac_key_lookup
{
	input_item_id   itemid;
	int             vk;
	const char *    name;
};

constexpr unsigned KEY_TOTAL_ENTRIES = 128;

const mac_key_lookup mac_key_table[] =
{
	{ ITEM_ID_A,            0x00,   "A" },
	{ ITEM_ID_S,            0x01,   "S" },
	{ ITEM_ID_D,            0x02,   "D" },
	{ ITEM_ID_F,            0x03,   "F" },
	{ ITEM_ID_H,            0x04,   "H" },
	{ ITEM_ID_G,            0x05,   "G" },
	{ ITEM_ID_Z,            0x06,   "Z" },
	{ ITEM_ID_X,            0x07,   "X" },
	{ ITEM_ID_C,            0x08,   "C" },
	{ ITEM_ID_V,            0x09,   "V" },
	{ ITEM_ID_BACKSLASH2,   0x0a,   "Section" },    // ISO keyboards only
	{ ITEM_ID_B,            0x0b,   "B" },
	{ ITEM_ID_Q,            0x0c,   "Q" },
	{ ITEM_ID_W,            0x0d,   "W" },
	{ ITEM_ID_E,            0x0e,   "E" },
	{ ITEM_ID_R,            0x0f,   "R" },
	{ ITEM_ID_Y,            0x10,   "Y" },
	{ ITEM_ID_T,            0x11,   "T" },
	{ ITEM_ID_1,            0x12,   "1" },
	{ ITEM_ID_2,            0x13,   "2" },
	{ ITEM_ID_3,            0x14,   "3" },
	{ ITEM_ID_4,            0x15,   "4" },
	{ ITEM_ID_6,            0x16,   "6" },
	{ ITEM_ID_5,            0x17,   "5" },
	{ ITEM_ID_EQUALS,       0x18,   "=" },
	{ ITEM_ID_9,            0x19,   "9" },
	{ ITEM_ID_7,            0x1a,   "7" },
	{ ITEM_ID_MINUS,        0x1b,   "-" },
	{ ITEM_ID_8,            0x1c,   "8" },
	{ ITEM_ID_0,            0x1d,   "0" },
	{ ITEM_ID_CLOSEBRACE,   0x1e,   "]" },
	{ ITEM_ID_O,            0x1f,   "O" },
	{ ITEM_ID_U,            0x20,   "U" },
	{ ITEM_ID_OPENBRACE,    0x21,   "[" },
	{ ITEM_ID_I,            0x22,   "I" },
	{ ITEM_ID_P,            0x23,   "P" },
	{ ITEM_ID_ENTER,        0x24,   "Return" },
	{ ITEM_ID_L,            0x25,   "L" },
	{ ITEM_ID_J,            0x26,   "J" },
	{ ITEM_ID_QUOTE,        0x27,   "'" },
	{ ITEM_ID_K,            0x28,   "K" },
	{ ITEM_ID_COLON,        0x29,   ";" },
	{ ITEM_ID_BACKSLASH,    0x2a,   "\\" },
	{ ITEM_ID_COMMA,        0x2b,   "," },
	{ ITEM_ID_SLASH,        0x2c,   "/" },
	{ ITEM_ID_N,            0x2d,   "N" },
	{ ITEM_ID_M,            0x2e,   "M" },
	{ ITEM_ID_STOP,         0x2f,   "." },
	{ ITEM_ID_TAB,          0x30,   "Tab" },
	{ ITEM_ID_SPACE,        0x31,   "Space" },
	{ ITEM_ID_TILDE,        0x32,   "`" },
	{ ITEM_ID_BACKSPACE,    0x33,   "Delete" },
	{ ITEM_ID_ESC,          0x35,   "Escape" },
	{ ITEM_ID_RWIN,         0x36,   "Right Command" },
	{ ITEM_ID_LWIN,         0x37,   "Command" },
	{ ITEM_ID_LSHIFT,       0x38,   "Shift" },
	{ ITEM_ID_CAPSLOCK,     0x39,   "Caps Lock" },
	{ ITEM_ID_LALT,         0x3a,   "Option" },
	{ ITEM_ID_LCONTROL,     0x3b,   "Control" },
	{ ITEM_ID_RSHIFT,       0x3c,   "Right Shift" },
	{ ITEM_ID_RALT,         0x3d,   "Right Option" },
	{ ITEM_ID_RCONTROL,     0x3e,   "Right Control" },
	{ ITEM_ID_F17,          0x40,   "F17" },
	{ ITEM_ID_DEL_PAD,      0x41,   "Keypad ." },
	{ ITEM_ID_ASTERISK,     0x43,   "Keypad *" },
	{ ITEM_ID_PLUS_PAD,     0x45,   "Keypad +" },
	{ ITEM_ID_NUMLOCK,      0x47,   "Keypad Clear" },
	{ ITEM_ID_SLASH_PAD,    0x4b,   "Keypad /" },
	{ ITEM_ID_ENTER_PAD,    0x4c,   "Keypad Enter" },
	{ ITEM_ID_MINUS_PAD,    0x4e,   "Keypad -" },
	{ ITEM_ID_F18,          0x4f,   "F18" },
	{ ITEM_ID_F19,          0x50,   "F19" },
	{ ITEM_ID_EQUALS_PAD,   0x51,   "Keypad =" },
	{ ITEM_ID_0_PAD,        0x52,   "Keypad 0" },
	{ ITEM_ID_1_PAD,        0x53,   "Keypad 1" },
	{ ITEM_ID_2_PAD,        0x54,   "Keypad 2" },
	{ ITEM_ID_3_PAD,        0x55,   "Keypad 3" },
	{ ITEM_ID_4_PAD,        0x56,   "Keypad 4" },
	{ ITEM_ID_5_PAD,        0x57,   "Keypad 5" },
	{ ITEM_ID_6_PAD,        0x58,   "Keypad 6" },
	{ ITEM_ID_7_PAD,        0x59,   "Keypad 7" },
	{ ITEM_ID_F20,          0x5a,   "F20" },
	{ ITEM_ID_8_PAD,        0x5b,   "Keypad 8" },
	{ ITEM_ID_9_PAD,        0x5c,   "Keypad 9" },
	{ ITEM_ID_F5,           0x60,   "F5" },
	{ ITEM_ID_F6,           0x61,   "F6" },
	{ ITEM_ID_F7,           0x62,   "F7" },
	{ ITEM_ID_F3,           0x63,   "F3" },
	{ ITEM_ID_F8,           0x64,   "F8" },
	{ ITEM_ID_F9,           0x65,   "F9" },
	{ ITEM_ID_F11,          0x67,   "F11" },
	{ ITEM_ID_F13,          0x69,   "F13" },
	{ ITEM_ID_F16,          0x6a,   "F16" },
	{ ITEM_ID_F14,          0x6b,   "F14" },
	{ ITEM_ID_F10,          0x6d,   "F10" },
	{ ITEM_ID_MENU,         0x6e,   "Menu" },
	{ ITEM_ID_F12,          0x6f,   "F12" },
	{ ITEM_ID_F15,          0x71,   "F15" },
	{ ITEM_ID_INSERT,       0x72,   "Help" },
	{ ITEM_ID_HOME,         0x73,   "Home" },
	{ ITEM_ID_PGUP,         0x74,   "Page Up" },
	{ ITEM_ID_DEL,          0x75,   "Forward Delete" },
	{ ITEM_ID_F4,           0x76,   "F4" },
	{ ITEM_ID_END,          0x77,   "End" },
	{ ITEM_ID_F2,           0x78,   "F2" },
	{ ITEM_ID_PGDN,         0x79,   "Page Down" },
	{ ITEM_ID_F1,           0x7a,   "F1" },
	{ ITEM_ID_LEFT,         0x7b,   "Left" },
	{ ITEM_ID_RIGHT,        0x7c,   "Right" },
	{ ITEM_ID_DOWN,         0x7d,   "Down" },
	{ ITEM_ID_UP,           0x7e,   "Up" }
};


//============================================================
//  mac_keyboard_device
//============================================================

class mac_keyboard_device : public event_based_device<mac_key_event>
{
public:
	mac_keyboard_device(std::string &&name, std::string &&id, input_module &module) :
		event_based_device(std::move(name), std::move(id), module),
		m_state{ 0 }
	{
	}

	virtual void reset() override
	{
		event_based_device::reset();
		memset(m_state, 0, sizeof(m_state));
	}

	virtual void configure(input_device &device) override
	{
		for (const mac_key_lookup &entry : mac_key_table)
		{
			device.add_item(
					entry.name,
					util::string_format("VK%02X", entry.vk),
					entry.itemid,
					generic_button_get_state<std::uint8_t>,
					&m_state[entry.vk]);
		}
	}

protected:
	virtual void process_event(const mac_key_event &event) override
	{
		m_state[event.vk & (KEY_TOTAL_ENTRIES - 1)] = event.down ? 0x80 : 0x00;
	}

private:
	uint8_t m_state[KEY_TOTAL_ENTRIES];
};


//============================================================
//  mac_mouse_device
//============================================================

class mac_mouse_device : public event_based_device<mac_mouse_event>
{
public:
	static constexpr unsigned MAX_BUTTONS = 5;

	mac_mouse_device(std::string &&name, std::string &&id, input_module &module) :
		event_based_device(std::move(name), std::move(id), module),
		m_mouse({ 0 }),
		m_x(0),
		m_y(0),
		m_v(0),
		m_h(0)
	{
	}

	virtual void poll(bool relative_reset) override
	{
		event_based_device::poll(relative_reset);

		if (relative_reset)
		{
			m_mouse.lX = std::exchange(m_x, 0);
			m_mouse.lY = std::exchange(m_y, 0);
			m_mouse.lV = std::exchange(m_v, 0);
			m_mouse.lH = std::exchange(m_h, 0);
		}
	}

	virtual void reset() override
	{
		event_based_device::reset();
		memset(&m_mouse, 0, sizeof(m_mouse));
		m_x = m_y = m_v = m_h = 0;
	}

	virtual void configure(input_device &device) override
	{
		// axes
		device.add_item(
				"X",
				std::string_view(),
				ITEM_ID_XAXIS,
				generic_axis_get_state<int32_t>,
				&m_mouse.lX);
		device.add_item(
				"Y",
				std::string_view(),
				ITEM_ID_YAXIS,
				generic_axis_get_state<int32_t>,
				&m_mouse.lY);
		device.add_item(
				"Scroll V",
				std::string_view(),
				ITEM_ID_ZAXIS,
				generic_axis_get_state<int32_t>,
				&m_mouse.lV);
		device.add_item(
				"Scroll H",
				std::string_view(),
				ITEM_ID_RZAXIS,
				generic_axis_get_state<int32_t>,
				&m_mouse.lH);

		// buttons - NSEvent numbering is already left, right, middle
		for (unsigned button = 0; button < MAX_BUTTONS; button++)
		{
			device.add_item(
					default_button_name(button),
					std::string_view(),
					input_item_id(ITEM_ID_BUTTON1 + button),
					generic_button_get_state<std::uint8_t>,
					&m_mouse.buttons[button]);
		}
	}

protected:
	virtual void process_event(const mac_mouse_event &event) override
	{
		switch (event.type)
		{
		case mac_mouse_event::kind::BUTTON:
			if (event.button >= 0 && event.button < int(MAX_BUTTONS))
			{
				m_mouse.buttons[event.button] = event.down ? 0x80 : 0x00;
			}
			break;

		case mac_mouse_event::kind::MOTION:
			m_x += int32_t(event.dx * input_device::RELATIVE_PER_PIXEL);
			m_y += int32_t(event.dy * input_device::RELATIVE_PER_PIXEL);
			break;

		case mac_mouse_event::kind::WHEEL:
			// scale to the Win32 120-per-detent convention
			m_v += int32_t(event.dy * 120 * input_device::RELATIVE_PER_PIXEL);
			m_h += int32_t(event.dx * 120 * input_device::RELATIVE_PER_PIXEL);
			break;
		}
	}

private:
	struct mouse_state
	{
		int32_t lX, lY, lV, lH;
		uint8_t buttons[MAX_BUTTONS];
	};

	mouse_state m_mouse;
	int32_t m_x, m_y, m_v, m_h;
};


//============================================================
//  keyboard_input_mac - Mac keyboard input module
//============================================================

class keyboard_input_mac : public input_module_impl<mac_keyboard_device, mac_osd_interface>
{
public:
	keyboard_input_mac() : input_module_impl<mac_keyboard_device, mac_osd_interface>(OSD_KEYBOARDINPUT_PROVIDER, "mac")
	{
	}

	virtual void input_init(running_machine &machine) override;
	virtual void exit() override;

	void queue_key_event(const mac_key_event &event)
	{
		devicelist().for_each_device([&event] (auto &device) { device.queue_event(event); });
	}
};


//============================================================
//  mouse_input_mac - Mac mouse input module
//============================================================

class mouse_input_mac : public input_module_impl<mac_mouse_device, mac_osd_interface>
{
public:
	mouse_input_mac() : input_module_impl<mac_mouse_device, mac_osd_interface>(OSD_MOUSEINPUT_PROVIDER, "mac")
	{
	}

	virtual void input_init(running_machine &machine) override;
	virtual void exit() override;

	void queue_mouse_event(const mac_mouse_event &event)
	{
		devicelist().for_each_device([&event] (auto &device) { device.queue_event(event); });
	}
};


// the modules the Cocoa event pump delivers to
keyboard_input_mac *g_keyboard_input = nullptr;
mouse_input_mac *g_mouse_input = nullptr;


void keyboard_input_mac::input_init(running_machine &machine)
{
	input_module_impl<mac_keyboard_device, mac_osd_interface>::input_init(machine);

	create_device<mac_keyboard_device>(DEVICE_CLASS_KEYBOARD, "Mac Keyboard 1", "Mac Keyboard 1");

	g_keyboard_input = this;
}

void keyboard_input_mac::exit()
{
	g_keyboard_input = nullptr;

	input_module_impl<mac_keyboard_device, mac_osd_interface>::exit();
}

void mouse_input_mac::input_init(running_machine &machine)
{
	input_module_impl<mac_mouse_device, mac_osd_interface>::input_init(machine);

	create_device<mac_mouse_device>(DEVICE_CLASS_MOUSE, "Mac Mouse 1", "Mac Mouse 1");

	g_mouse_input = this;
}

void mouse_input_mac::exit()
{
	g_mouse_input = nullptr;

	input_module_impl<mac_mouse_device, mac_osd_interface>::exit();
}

} // anonymous namespace

} // namespace osd


//============================================================
//  event intake from the Cocoa side (windowcontroller.mm)
//============================================================

extern void *GetOSWindow(void *wincontroller);
extern void *GetOSStandardWindow(void *wincontroller);

static render_target *mac_ui_target()
{
	if (osd_common_t::window_list().empty())
	{
		return nullptr;
	}

	return osd_common_t::window_list().front()->target();
}

// find the render target belonging to a native window
static render_target *mac_target_for_window(void *nswindow)
{
	if (nswindow == nullptr)
	{
		return nullptr;
	}

	for (const auto &w : osd_common_t::window_list())
	{
		auto &win = static_cast<mac_window_info &>(*w);
		if (win.platform_window() != nullptr)
		{
			// match either the current (possibly fullscreen) or the standard window
			if ((GetOSWindow(win.platform_window()) == nswindow) || (GetOSStandardWindow(win.platform_window()) == nswindow))
			{
				return win.target();
			}
		}
	}
	return nullptr;
}

extern "C" void MacKeyboardEvent(int vk, int down)
{
	if (osd::g_keyboard_input != nullptr)
	{
		osd::g_keyboard_input->queue_key_event(osd::mac_key_event{ vk, down != 0 });
	}
}

extern "C" void MacMouseButtonEvent(int button, int down)
{
	if (osd::g_mouse_input != nullptr)
	{
		osd::mac_mouse_event event{};
		event.type = osd::mac_mouse_event::kind::BUTTON;
		event.button = button;
		event.down = down != 0;
		osd::g_mouse_input->queue_mouse_event(event);
	}
}

extern "C" void MacMouseMotionEvent(float dx, float dy)
{
	if (osd::g_mouse_input != nullptr)
	{
		osd::mac_mouse_event event{};
		event.type = osd::mac_mouse_event::kind::MOTION;
		event.dx = dx;
		event.dy = dy;
		osd::g_mouse_input->queue_mouse_event(event);
	}
}

extern "C" void MacMouseWheelEvent(float dv, float dh)
{
	if (osd::g_mouse_input != nullptr)
	{
		osd::mac_mouse_event event{};
		event.type = osd::mac_mouse_event::kind::WHEEL;
		event.dx = dh;
		event.dy = dv;
		osd::g_mouse_input->queue_mouse_event(event);
	}
}

extern "C" void MacCharEvent(unsigned int ch)
{
	render_target *const target = mac_ui_target();
	if (target != nullptr)
	{
		target->push_char_event(char32_t(ch));
	}
}

extern "C" void MacPointerUpdate(void *nswindow, int x, int y, unsigned int buttons, unsigned int pressed, unsigned int released, int clicks)
{
	render_target *const target = mac_target_for_window(nswindow);
	if (target != nullptr)
	{
		target->push_pointer_update(osd::ui_event_handler::pointer::MOUSE, 0, 0, x, y, buttons, pressed, released, clicks);
	}
}

extern "C" void MacPointerLeave(void *nswindow, int x, int y, unsigned int released, int clicks)
{
	render_target *const target = mac_target_for_window(nswindow);
	if (target != nullptr)
	{
		target->push_pointer_leave(osd::ui_event_handler::pointer::MOUSE, 0, 0, x, y, released, clicks);
	}
}

extern "C" void MacMouseWheelUI(void *nswindow, int x, int y, int delta)
{
	render_target *const target = mac_target_for_window(nswindow);
	if (target != nullptr)
	{
		target->push_mouse_wheel_event(x, y, delta, 3);
	}
}


//============================================================
//  mac_osd_interface input hooks
//============================================================

void mac_osd_interface::customize_input_type_list(std::vector<input_type_entry> &typelist)
{
	// loop over the defaults
	for (input_type_entry &entry : typelist)
	{
		switch (entry.type())
		{
		// configurable UI mode switch
		case IPT_UI_TOGGLE_UI:
			{
				const char *const uimode = options().ui_mode_key();
				input_item_id mameid_code = ITEM_ID_INVALID;
				if (!uimode || !*uimode || !strcmp(uimode, "auto"))
				{
					mameid_code = keyboard_trans_table::instance().lookup_mame_code("ITEM_ID_INSERT");
				}
				else
				{
					std::string fullmode("ITEM_ID_");
					fullmode.append(uimode);
					mameid_code = keyboard_trans_table::instance().lookup_mame_code(fullmode.c_str());
				}
				if (ITEM_ID_INVALID != mameid_code)
				{
					const input_code ui_code = input_code(DEVICE_CLASS_KEYBOARD, 0, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, input_item_id(mameid_code));
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

		// LALT-F10 to toggle OpenGL filtering
		case IPT_OSD_5:
			entry.configure_osd("TOGGLE_FILTER", N_p("input-name", "Toggle Filter"));
			entry.defseq(SEQ_TYPE_STANDARD).set(KEYCODE_F10, KEYCODE_LALT);
			break;

		// add a Not LALT condition to the throttle key
		case IPT_UI_THROTTLE:
			entry.defseq(SEQ_TYPE_STANDARD).set(KEYCODE_F10, input_seq::not_code, KEYCODE_LALT);
			break;

		// LALT-F8 to decrease OpenGL prescaling
		case IPT_OSD_6:
			entry.configure_osd("DECREASE_PRESCALE", N_p("input-name", "Decrease Prescaling"));
			entry.defseq(SEQ_TYPE_STANDARD).set(KEYCODE_F8, KEYCODE_LALT);
			break;

		// LALT-F9 to increase OpenGL prescaling
		case IPT_OSD_7:
			entry.configure_osd("INCREASE_PRESCALE", N_p("input-name", "Increase Prescaling"));
			entry.defseq(SEQ_TYPE_STANDARD).set(KEYCODE_F9, KEYCODE_LALT);
			break;

		default:
			break;
		}
	}
}

void mac_osd_interface::release_keys()
{
	if (osd::g_keyboard_input != nullptr)
	{
		osd::g_keyboard_input->reset_devices();
	}
}

bool mac_osd_interface::should_hide_mouse()
{
	// if we are paused, no
	if (machine().paused())
	{
		return false;
	}

	// if neither mice nor lightguns are enabled in the core, then no
	if (!options().mouse() && !options().lightgun())
	{
		return false;
	}

	// if the pointer isn't over the window, then no
	if (!MacPointerInWindow())
	{
		return false;
	}

	// otherwise, yes
	return true;
}

void mac_osd_interface::process_events_buf()
{
}

#else // defined(OSD_MAC)

namespace osd {

namespace {

MODULE_NOT_SUPPORTED(keyboard_input_mac, OSD_KEYBOARDINPUT_PROVIDER, "mac")
MODULE_NOT_SUPPORTED(mouse_input_mac, OSD_MOUSEINPUT_PROVIDER, "mac")

} // anonymous namespace

} // namespace osd

#endif // defined(OSD_MAC)

MODULE_DEFINITION(KEYBOARDINPUT_MAC, osd::keyboard_input_mac)
MODULE_DEFINITION(MOUSEINPUT_MAC, osd::mouse_input_mac)
