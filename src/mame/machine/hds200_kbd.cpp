// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    HDS200 Keyboard (HLE)

    TODO:
    - Add missing keycodes, verify existing
    - LEDs
    - Keyclick

    Notes:
    - No clear picture of the keyboard available, everything guessed

***************************************************************************/

#include "emu.h"
#include "hds200_kbd.h"
#include "machine/keyboard.ipp"
#include "speaker.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(HDS200_KBD_HLE, hds200_kbd_hle_device, "hds200_kbd_hle", "HDS200 Keyboard (HLE)")

namespace {

uint8_t const TRANSLATION_TABLE[4][7][22] =
{
	// unshift
	{ //  main                                                                                         control              number pad
		{ 0x00, 0xb0, 0x00, 0xf0, 0xa8, 0xa9, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,    0xb1, 0x00, 0x00,    0x00, 0x00, 0x00, 0x00 }, // 0
		{ 0x1b, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x2d, 0x3d, 0x60, 0x00,    0x00, 0x00, 0x00,    0x00, 0x00, 0x00, 0x00 }, // 1
		{ 0x09, 0x71, 0x77, 0x65, 0x72, 0x74, 0x79, 0x75, 0x69, 0x6f, 0x70, 0x5b, 0x5d, 0x0d, 0x08,    0x00, 0x00, 0x00,    0xa5, 0xa6, 0xa7, 0x9c }, // 2
		{ 0x00, 0x61, 0x73, 0x64, 0x66, 0x67, 0x68, 0x6a, 0x6b, 0x6c, 0x3b, 0x27, 0x00, 0x00, 0x5c,    0x00, 0x00, 0x00,    0xa2, 0xa3, 0xa4, 0x9b }, // 3
		{ 0x00, 0x00, 0x7a, 0x78, 0x63, 0x76, 0x62, 0x6e, 0x6d, 0x2c, 0x2e, 0x2f, 0x00, 0x00, 0x0a,    0xab, 0xad, 0xaa,    0x9f, 0xa0, 0xa1, 0x0d }, // 4
		{ 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,    0x00, 0x00, 0x00,    0x9e, 0x00, 0x9d, 0x00 }  // 5
	},
	// shift
	{ //  main                                                                                         control              number pad
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,    0x00, 0x00, 0x00,    0x00, 0x00, 0x00, 0x00 }, // 0
		{ 0x00, 0x21, 0x40, 0x23, 0x24, 0x25, 0x5e, 0x26, 0x2a, 0x28, 0x29, 0x5f, 0x2b, 0x7e, 0x00,    0x00, 0x00, 0x00,    0x00, 0x00, 0x00, 0x00 }, // 1
		{ 0x00, 0x51, 0x57, 0x45, 0x52, 0x54, 0x59, 0x55, 0x49, 0x4f, 0x50, 0x7b, 0x7d, 0x00, 0x00,    0x00, 0x00, 0x00,    0x00, 0x00, 0x00, 0x00 }, // 2
		{ 0x00, 0x41, 0x53, 0x44, 0x46, 0x47, 0x48, 0x4a, 0x4b, 0x4c, 0x3a, 0x22, 0x00, 0x00, 0x7c,    0x00, 0x00, 0x00,    0x00, 0x00, 0x00, 0x00 }, // 3
		{ 0x00, 0x00, 0x5a, 0x58, 0x43, 0x56, 0x42, 0x4e, 0x4d, 0x3c, 0x3e, 0x3f, 0x00, 0x00, 0x00,    0x00, 0x00, 0x00,    0x00, 0x00, 0x00, 0x00 }, // 4
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,    0x00, 0x00, 0x00,    0x00, 0x00, 0x00, 0x00 }  // 5
	},
	// unshift-control
	{ //  main                                                                                         control              number pad
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,    0x00, 0x00, 0x00,    0x00, 0x00, 0x00, 0x00 }, // 0
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,    0x00, 0x00, 0x00,    0x00, 0x00, 0x00, 0x00 }, // 1
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,    0x00, 0x00, 0x00,    0x00, 0x00, 0x00, 0x00 }, // 2
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,    0x00, 0x00, 0x00,    0x00, 0x00, 0x00, 0x00 }, // 3
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,    0x00, 0x00, 0x00,    0x00, 0x00, 0x00, 0x00 }, // 4
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,    0x00, 0x00, 0x00,    0x00, 0x00, 0x00, 0x00 }  // 5
	},
	// shift-control
	{ //  main                                                                                         control              number pad
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,    0x00, 0x00, 0x00,    0x00, 0x00, 0x00, 0x00 }, // 0
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,    0x00, 0x00, 0x00,    0x00, 0x00, 0x00, 0x00 }, // 1
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,    0x00, 0x00, 0x00,    0x00, 0x00, 0x00, 0x00 }, // 2
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,    0x00, 0x00, 0x00,    0x00, 0x00, 0x00, 0x00 }, // 3
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,    0x00, 0x00, 0x00,    0x00, 0x00, 0x00, 0x00 }, // 4
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,    0x00, 0x00, 0x00,    0x00, 0x00, 0x00, 0x00 }  // 5
	}
};

} // anonymous namespace

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

static INPUT_PORTS_START( keyboard )
	PORT_START("mod")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Ctrl")        PORT_CODE(KEYCODE_LCONTROL)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Left Shift")  PORT_CODE(KEYCODE_LSHIFT)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Right Shift") PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Lock")        PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE

	PORT_START("row_0")
	PORT_BIT(0x000001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Hold Scrn")  PORT_CODE(KEYCODE_SCRLOCK)
	PORT_BIT(0x000002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Setup")      PORT_CODE(KEYCODE_F1) // ? (mapped to 'Setup' for now)
	PORT_BIT(0x000004, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x000008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Status")     PORT_CODE(KEYCODE_F2) // ? (mapped to 'Status' for now)
	PORT_BIT(0x000010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Next Line")  PORT_CODE(KEYCODE_F3) // ? (mapped to 'Next Line' for now)
	PORT_BIT(0x000020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Next Value") PORT_CODE(KEYCODE_F4) // ? (mapped to 'Next Value' for now)
	PORT_BIT(0x000040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F6)
	PORT_BIT(0x000080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F7)
	PORT_BIT(0x000100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F8)
	PORT_BIT(0x000200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F9)
	PORT_BIT(0x000400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F10)
	PORT_BIT(0x000800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F11)
	PORT_BIT(0x001000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F12)
	PORT_BIT(0x002000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F13)
	PORT_BIT(0x004000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F14)
	PORT_BIT(0x008000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Save") PORT_CODE(KEYCODE_F5) // ? (mapped to 'Save' [code ac or b1?] for now)
	PORT_BIT(0x010000, IP_ACTIVE_HIGH, IPT_KEYBOARD) // ?
	PORT_BIT(0x020000, IP_ACTIVE_HIGH, IPT_KEYBOARD) // ?
	PORT_BIT(0x040000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F17)
	PORT_BIT(0x080000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F18)
	PORT_BIT(0x100000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F19)
	PORT_BIT(0x200000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F20)
	PORT_BIT(0x400000, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x800000, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("row_1")
	PORT_BIT(0x000001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC)
	PORT_BIT(0x000002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)
	PORT_BIT(0x000004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)
	PORT_BIT(0x000008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)
	PORT_BIT(0x000010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)
	PORT_BIT(0x000020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)
	PORT_BIT(0x000040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)
	PORT_BIT(0x000080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)
	PORT_BIT(0x000100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)
	PORT_BIT(0x000200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)
	PORT_BIT(0x000400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)
	PORT_BIT(0x000800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)
	PORT_BIT(0x001000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT(0x002000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE)
	PORT_BIT(0x004000, IP_ACTIVE_HIGH, IPT_KEYBOARD) // "erase"?
	PORT_BIT(0x008000, IP_ACTIVE_HIGH, IPT_KEYBOARD) // ?
	PORT_BIT(0x010000, IP_ACTIVE_HIGH, IPT_KEYBOARD) // ?
	PORT_BIT(0x020000, IP_ACTIVE_HIGH, IPT_KEYBOARD) // ?
	PORT_BIT(0x040000, IP_ACTIVE_HIGH, IPT_KEYBOARD) // PF1?
	PORT_BIT(0x080000, IP_ACTIVE_HIGH, IPT_KEYBOARD) // PF2?
	PORT_BIT(0x100000, IP_ACTIVE_HIGH, IPT_KEYBOARD) // PF3?
	PORT_BIT(0x200000, IP_ACTIVE_HIGH, IPT_KEYBOARD) // PF4?
	PORT_BIT(0x400000, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x800000, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("row_2")
	PORT_BIT(0x000001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB)
	PORT_BIT(0x000002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x000004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)
	PORT_BIT(0x000008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)
	PORT_BIT(0x000010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)
	PORT_BIT(0x000020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)
	PORT_BIT(0x000040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x000080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)
	PORT_BIT(0x000100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)
	PORT_BIT(0x000200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)
	PORT_BIT(0x000400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)
	PORT_BIT(0x000800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT(0x001000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT(0x002000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER)
	PORT_BIT(0x004000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT(0x008000, IP_ACTIVE_HIGH, IPT_KEYBOARD) // ?
	PORT_BIT(0x010000, IP_ACTIVE_HIGH, IPT_KEYBOARD) // ?
	PORT_BIT(0x020000, IP_ACTIVE_HIGH, IPT_KEYBOARD) // ?
	PORT_BIT(0x040000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT(0x080000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT(0x100000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT(0x200000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS_PAD)
	PORT_BIT(0x400000, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x800000, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("row_3")
	PORT_BIT(0x000001, IP_ACTIVE_HIGH, IPT_UNUSED) // Ctrl (handled elsewhere)
	PORT_BIT(0x000002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)
	PORT_BIT(0x000004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)
	PORT_BIT(0x000008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)
	PORT_BIT(0x000010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)
	PORT_BIT(0x000020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)
	PORT_BIT(0x000040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)
	PORT_BIT(0x000080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)
	PORT_BIT(0x000100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)
	PORT_BIT(0x000200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)
	PORT_BIT(0x000400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)
	PORT_BIT(0x000800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT(0x001000, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x002000, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x004000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT(0x008000, IP_ACTIVE_HIGH, IPT_KEYBOARD)
	PORT_BIT(0x010000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)
	PORT_BIT(0x020000, IP_ACTIVE_HIGH, IPT_KEYBOARD)
	PORT_BIT(0x040000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT(0x080000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT(0x100000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT(0x200000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA_PAD)
	PORT_BIT(0x400000, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x800000, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("row_4")
	PORT_BIT(0x000001, IP_ACTIVE_HIGH, IPT_UNUSED) // Left Shift (handled elsewhere)
	PORT_BIT(0x000002, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x000004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)
	PORT_BIT(0x000008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)
	PORT_BIT(0x000010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)
	PORT_BIT(0x000020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)
	PORT_BIT(0x000040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)
	PORT_BIT(0x000080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)
	PORT_BIT(0x000100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)
	PORT_BIT(0x000200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)
	PORT_BIT(0x000400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)
	PORT_BIT(0x000800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)
	PORT_BIT(0x001000, IP_ACTIVE_HIGH, IPT_UNUSED) // Right Shift (handled elsewhere)
	PORT_BIT(0x002000, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x004000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Line Feed")
	PORT_BIT(0x008000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x010000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)
	PORT_BIT(0x020000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x040000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT(0x080000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x100000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT(0x200000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER_PAD)
	PORT_BIT(0x400000, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x800000, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("row_5")
	PORT_BIT(0x000001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Comp Char")
	PORT_BIT(0x000002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)
	PORT_BIT(0x000004, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x000008, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x000010, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x000020, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x000040, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x000080, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x000100, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x000200, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x000400, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x000800, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x001000, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x002000, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x004000, IP_ACTIVE_HIGH, IPT_UNUSED) // Lock
	PORT_BIT(0x008000, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x010000, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x020000, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x040000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT(0x080000, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x100000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL_PAD)
	PORT_BIT(0x200000, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x400000, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x800000, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END

ioport_constructor hds200_kbd_hle_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( keyboard );
}

void hds200_kbd_hle_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "mono").front_center();
	BEEP(config, m_bell, 786).add_route(ALL_OUTPUTS, "mono", 0.5); // unknown frequency
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  hds200_kbd_hle_device - constructor
//-------------------------------------------------

hds200_kbd_hle_device::hds200_kbd_hle_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, HDS200_KBD_HLE, tag, owner, clock),
	device_buffered_serial_interface(mconfig, *this),
	device_matrix_keyboard_interface(mconfig, *this, "row_0", "row_1", "row_2", "row_3", "row_4", "row_5"),
	m_bell(*this, "bell"),
	m_modifiers(*this, "mod"),
	m_tx_handler(*this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void hds200_kbd_hle_device::device_start()
{
	// resolve callbacks
	m_tx_handler.resolve_safe();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void hds200_kbd_hle_device::device_reset()
{
	clear_fifo();

	receive_register_reset();
	transmit_register_reset();

	set_data_frame(1, 8, PARITY_EVEN, STOP_BITS_2);
	set_rcv_rate(2400);
	set_tra_rate(2400);

	reset_key_state();
	start_processing(attotime::from_hz(2400));
	typematic_stop();
}

//-------------------------------------------------
//  tra_callback - send bit to host
//-------------------------------------------------

void hds200_kbd_hle_device::tra_callback()
{
	m_tx_handler(transmit_register_get_data_bit());
}

//-------------------------------------------------
//  received_byte - handle received byte
//-------------------------------------------------

void hds200_kbd_hle_device::received_byte(uint8_t byte)
{
	logerror("Received from host: %02x\n", byte);

	switch (byte & 0xc0)
	{
		// state
		case 0x00:

			// 76------  00
			// --5-----  caps lock led
			// ---43---  unknown
			// -----2--  insert mode
			// ------1-  led l1
			// -------0  bell

			m_bell->set_state(BIT(byte, 0));
			break;

		// volume
		case 0xc0:

			// 76------  11
			// --543---  bell volume
			// -----210  keyclick volume

			int bell_vol = (byte >> 3) & 0x07;
			m_bell->set_output_gain(0, bell_vol / 7.0);
			break;
	}
}

//-------------------------------------------------
//  key_make - handle a key being pressed
//-------------------------------------------------

void hds200_kbd_hle_device::key_make(uint8_t row, uint8_t column)
{
	uint8_t code = translate(row, column);

	// send the code
	if (code != 0x00)
	{
		transmit_byte(code);
		typematic_start(row, column, attotime::from_msec(750), attotime::from_msec(50));
	}
}

//-------------------------------------------------
//  key_break - handle a key being released
//-------------------------------------------------

void hds200_kbd_hle_device::key_break(uint8_t row, uint8_t column)
{
	if (typematic_is(row, column))
		typematic_stop();
}

//-------------------------------------------------
//  key_repeat - handle a key being repeated
//-------------------------------------------------

void hds200_kbd_hle_device::key_repeat(u8 row, u8 column)
{
	uint8_t code = translate(row, column);

	if (code != 0x00)
		transmit_byte(code);
}

//-------------------------------------------------
//  rx_w - receive bit from host
//-------------------------------------------------

void hds200_kbd_hle_device::rx_w(int state)
{
	device_buffered_serial_interface::rx_w(state);
}

//-------------------------------------------------
//  translate - row and column to key code
//-------------------------------------------------

uint8_t hds200_kbd_hle_device::translate(uint8_t row, uint8_t column)
{
	uint8_t const modifiers(m_modifiers->read());

	bool const ctrl(modifiers & 0x01);
	bool const shift(bool(modifiers & 0x02) || bool(modifiers & 0x04) || bool(modifiers & 0x08));
	bool const ctrl_shift(ctrl && shift);

	unsigned const map(ctrl_shift? 3 : ctrl ? 2 : shift ? 1 : 0);

	return TRANSLATION_TABLE[map][row][column];
}
