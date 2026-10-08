// license:BSD-3-Clause
// copyright-holders:Carl,Vas Crabb

#include "emu.h"
#include "m20_kbd.h"

#include "machine/keyboard.ipp"
#include "speaker.h"


namespace {
/*
    TODO: dump 8048 mcu

    There seem to be a lot of guesses in this code - actual scan rate,
    FIFO length, command processing, etc. don't seem to be evidence-
    based.  The modifier handling seems very odd as it has aliasing and
    overflow all over the place.

    Layout is selected with jumpers 26-29 on the keyboard MCU (Olivetti
    M20 Service Manual, April 1983, pp. 2-36/2-37, fig. 2-17).  Closed
    jumpers encode zero bits: all closed is Italy, opening 26 is Germany,
    and opening 28 is US ASCII (country 4).
    The response to command 03 has the country in bits 7-4.  BIOS 1.0
    at 0978-09ae and BIOS 2.0f at 093e-096c check the low nibble, then
    shift the response right by four to obtain the country.

    Italian, German and US ASCII key legends are implemented.  US ASCII
    matches PCOS 1.3 and PCOS 2.0h kb.all country 4.  The original German
    legends match PCOS 1.3 kb.all country 1 and PCOS 2.0h kb.all country 15;
    the revised legends match PCOS 2.0h country 1.  This is a keycap choice,
    not another jumper setting: both German versions report country 1.
    PCOS still performs all national scan-code translation.

    TODO: add input layouts for the other national keyboards.

    Available layouts:
    Italian
    German
    French
    British
    USA ASCII
    Spanish
    Portuguese
    Swedish - Finnish
    Danish
    Katakana
    Yugoslavian
    Norwegian
    Greek
    Swiss - French
    Swiss - German

    The keyboard MCU drives a buzzer.  We should work out what the bell
    on/off commands are and emulate it.

    There are apparently no break codes.
    The scancodes are arranged to read in alphabetical order with the QWERTY layout.
    Modifiers apparently modify the make codes.
    We are using a matrix here that corresponds to the logical scan codes - the physical matrix apparently doesn't match this.

    00  1c  1d  1e  1f  20  21  22  23  24  25  26  27   c3     cd  ce  cf  d0
     ??   12  18  06  13  15  1a  16  0a  10  11  28  29  c2    ca  cb  cc  d1
       ??  02  14  05  07  08  09  0b  0c  0d  2a  2b  2c  c1   c7  c8  c9  d2
    ??   01  1b  19  04  17  03  0f  0e  2d  2e  2f   ??        c4  c5  c6  d3
                               c0

    Ths is the 72-key version of the keyboard.
    The katakana kayout has 75 keys, but we don't have a diagram for it.
    ?? are modifiers, which are read directly, not through the 8x9 scan matrix.
*/
// Italian (QZERTY), German (QWERTZ) and US ASCII (QWERTY) layouts
// Natural keyboard character levels are unmodified, Shift, then Ctrl.
static INPUT_PORTS_START( m20_keyboard )
	PORT_START("LAYOUT")
	PORT_DIPNAME(0x0f, 0x00, "Keyboard Language") PORT_DIPLOCATION("J:26,27,28,29")
	PORT_DIPSETTING(0x00, "Italian")
	PORT_DIPSETTING(0x01, "German")
	PORT_DIPSETTING(0x04, "US ASCII")
	PORT_CONFNAME(0x10, 0x00, "German Key Legends") PORT_CONDITION("LAYOUT", 0x0f, EQUALS, 0x01)
	PORT_CONFSETTING(0x00, "Original (PCOS 1.x)")
	PORT_CONFSETTING(0x10, "Revised (PCOS 2.x)")

	PORT_START("LINE0")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("RESET")    PORT_CODE(KEYCODE_ESC)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_LALT)        PORT_CHAR('<') PORT_CHAR('>') PORT_CONDITION("LAYOUT", 0x0f, EQUALS, 0x00)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_BACKSLASH2)  PORT_CHAR('<') PORT_CHAR('>') PORT_CONDITION("LAYOUT", 0x0f, EQUALS, 0x01)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_LALT) PORT_CODE(KEYCODE_BACKSLASH2) PORT_CHAR('\\') PORT_CHAR('|') PORT_CONDITION("LAYOUT", 0x0f, EQUALS, 0x04)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_A)           PORT_CHAR('a') PORT_CHAR('A') PORT_CHAR(0x01U)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_B)           PORT_CHAR('b') PORT_CHAR('B') PORT_CHAR(0x02U)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_C)           PORT_CHAR('c') PORT_CHAR('C') PORT_CHAR(0x03U)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_D)           PORT_CHAR('d') PORT_CHAR('D') PORT_CHAR(0x04U)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_E)           PORT_CHAR('e') PORT_CHAR('E') PORT_CHAR(0x05U)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_F)           PORT_CHAR('f') PORT_CHAR('F') PORT_CHAR(0x06U)

	PORT_START("LINE1")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_G)           PORT_CHAR('g') PORT_CHAR('G') PORT_CHAR(0x07U)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_H)           PORT_CHAR('h') PORT_CHAR('H') PORT_CHAR(0x08U)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_I)           PORT_CHAR('i') PORT_CHAR('I') PORT_CHAR(0x09U)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_J)           PORT_CHAR('j') PORT_CHAR('J') PORT_CHAR(0x0aU)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_K)           PORT_CHAR('k') PORT_CHAR('K') PORT_CHAR(0x0bU)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_L)           PORT_CHAR('l') PORT_CHAR('L') PORT_CHAR(0x0cU)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_M)           PORT_CHAR(',') PORT_CHAR('?') PORT_CONDITION("LAYOUT", 0x0f, EQUALS, 0x00)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_M)           PORT_CHAR('m') PORT_CHAR('M') PORT_CHAR(0x0dU) PORT_CONDITION("LAYOUT", 0x0f, EQUALS, 0x01)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_M)           PORT_CHAR('m') PORT_CHAR('M') PORT_CHAR(0x0dU) PORT_CONDITION("LAYOUT", 0x0f, EQUALS, 0x04)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_N)           PORT_CHAR('n') PORT_CHAR('N') PORT_CHAR(0x0eU)

	PORT_START("LINE2")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_O)           PORT_CHAR('o') PORT_CHAR('O') PORT_CHAR(0x0fU)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_P)           PORT_CHAR('p') PORT_CHAR('P') PORT_CHAR(0x10U)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_Q)           PORT_CHAR('q') PORT_CHAR('Q') PORT_CHAR(0x11U)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_R)           PORT_CHAR('r') PORT_CHAR('R') PORT_CHAR(0x12U)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_S)           PORT_CHAR('s') PORT_CHAR('S') PORT_CHAR(0x13U)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_T)           PORT_CHAR('t') PORT_CHAR('T') PORT_CHAR(0x14U)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_U)           PORT_CHAR('u') PORT_CHAR('U') PORT_CHAR(0x15U)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_V)           PORT_CHAR('v') PORT_CHAR('V') PORT_CHAR(0x16U)

	PORT_START("LINE3")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_W)           PORT_CHAR('z') PORT_CHAR('Z') PORT_CHAR(0x1aU) PORT_CONDITION("LAYOUT", 0x0f, EQUALS, 0x00)
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_W)           PORT_CHAR('w') PORT_CHAR('W') PORT_CHAR(0x17U) PORT_CONDITION("LAYOUT", 0x0f, EQUALS, 0x01)
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_W)           PORT_CHAR('w') PORT_CHAR('W') PORT_CHAR(0x17U) PORT_CONDITION("LAYOUT", 0x0f, EQUALS, 0x04)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_X)           PORT_CHAR('x') PORT_CHAR('X') PORT_CHAR(0x18U)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_Y)           PORT_CHAR('y') PORT_CHAR('Y') PORT_CHAR(0x19U) PORT_CONDITION("LAYOUT", 0x0f, EQUALS, 0x00)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_Y)           PORT_CHAR('z') PORT_CHAR('Z') PORT_CHAR(0x1aU) PORT_CONDITION("LAYOUT", 0x0f, EQUALS, 0x01)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_Y)           PORT_CHAR('y') PORT_CHAR('Y') PORT_CHAR(0x19U) PORT_CONDITION("LAYOUT", 0x0f, EQUALS, 0x04)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_Z)           PORT_CHAR('w') PORT_CHAR('W') PORT_CHAR(0x17U) PORT_CONDITION("LAYOUT", 0x0f, EQUALS, 0x00)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_Z)           PORT_CHAR('y') PORT_CHAR('Y') PORT_CHAR(0x19U) PORT_CONDITION("LAYOUT", 0x0f, EQUALS, 0x01)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_Z)           PORT_CHAR('z') PORT_CHAR('Z') PORT_CHAR(0x1aU) PORT_CONDITION("LAYOUT", 0x0f, EQUALS, 0x04)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_0)           PORT_CHAR(0x00e0U) PORT_CHAR('0') PORT_CONDITION("LAYOUT", 0x0f, EQUALS, 0x00)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_0)           PORT_CHAR('0') PORT_CHAR('/') PORT_CONDITION("LAYOUT", 0x1f, EQUALS, 0x01)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_0)           PORT_CHAR('0') PORT_CHAR('=') PORT_CONDITION("LAYOUT", 0x1f, EQUALS, 0x11)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_0)           PORT_CHAR('0') PORT_CHAR('_') PORT_CONDITION("LAYOUT", 0x0f, EQUALS, 0x04)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_1)           PORT_CHAR(0x00a3U) PORT_CHAR('1') PORT_CONDITION("LAYOUT", 0x0f, EQUALS, 0x00)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_1)           PORT_CHAR('1') PORT_CHAR(';') PORT_CONDITION("LAYOUT", 0x1f, EQUALS, 0x01)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_1)           PORT_CHAR('1') PORT_CHAR('!') PORT_CONDITION("LAYOUT", 0x1f, EQUALS, 0x11)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_1)           PORT_CHAR('1') PORT_CHAR('!') PORT_CONDITION("LAYOUT", 0x0f, EQUALS, 0x04)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_2)           PORT_CHAR(0x00e9U) PORT_CHAR('2') PORT_CONDITION("LAYOUT", 0x0f, EQUALS, 0x00)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_2)           PORT_CHAR('2') PORT_CHAR('"') PORT_CONDITION("LAYOUT", 0x0f, EQUALS, 0x01)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_2)           PORT_CHAR('2') PORT_CHAR('"') PORT_CONDITION("LAYOUT", 0x0f, EQUALS, 0x04)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_3)           PORT_CHAR('"') PORT_CHAR('3') PORT_CONDITION("LAYOUT", 0x0f, EQUALS, 0x00)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_3)           PORT_CHAR('3') PORT_CHAR('=') PORT_CONDITION("LAYOUT", 0x1f, EQUALS, 0x01)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_3)           PORT_CHAR('3') PORT_CHAR(0x00a7U) PORT_CONDITION("LAYOUT", 0x1f, EQUALS, 0x11)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_3)           PORT_CHAR('3') PORT_CHAR('#') PORT_CONDITION("LAYOUT", 0x0f, EQUALS, 0x04)

	PORT_START("LINE4")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_4)           PORT_CHAR('\'') PORT_CHAR('4') PORT_CONDITION("LAYOUT", 0x0f, EQUALS, 0x00)
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_4)           PORT_CHAR('4') PORT_CHAR('%') PORT_CONDITION("LAYOUT", 0x1f, EQUALS, 0x01)
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_4)           PORT_CHAR('4') PORT_CHAR('$') PORT_CONDITION("LAYOUT", 0x1f, EQUALS, 0x11)
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_4)           PORT_CHAR('4') PORT_CHAR('$') PORT_CONDITION("LAYOUT", 0x0f, EQUALS, 0x04)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_5)           PORT_CHAR('(') PORT_CHAR('5') PORT_CONDITION("LAYOUT", 0x0f, EQUALS, 0x00)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_5)           PORT_CHAR('5') PORT_CHAR('&') PORT_CONDITION("LAYOUT", 0x1f, EQUALS, 0x01)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_5)           PORT_CHAR('5') PORT_CHAR('%') PORT_CONDITION("LAYOUT", 0x1f, EQUALS, 0x11)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_5)           PORT_CHAR('5') PORT_CHAR('%') PORT_CONDITION("LAYOUT", 0x0f, EQUALS, 0x04)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_6)           PORT_CHAR('_') PORT_CHAR('6') PORT_CONDITION("LAYOUT", 0x0f, EQUALS, 0x00)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_6)           PORT_CHAR('6') PORT_CHAR('(') PORT_CONDITION("LAYOUT", 0x1f, EQUALS, 0x01)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_6)           PORT_CHAR('6') PORT_CHAR('&') PORT_CONDITION("LAYOUT", 0x1f, EQUALS, 0x11)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_6)           PORT_CHAR('6') PORT_CHAR('&') PORT_CONDITION("LAYOUT", 0x0f, EQUALS, 0x04)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_7)           PORT_CHAR(0x00e8U) PORT_CHAR('7') PORT_CONDITION("LAYOUT", 0x0f, EQUALS, 0x00)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_7)           PORT_CHAR('7') PORT_CHAR(')') PORT_CONDITION("LAYOUT", 0x1f, EQUALS, 0x01)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_7)           PORT_CHAR('7') PORT_CHAR('/') PORT_CONDITION("LAYOUT", 0x1f, EQUALS, 0x11)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_7)           PORT_CHAR('7') PORT_CHAR('\'') PORT_CONDITION("LAYOUT", 0x0f, EQUALS, 0x04)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_8)           PORT_CHAR('^') PORT_CHAR('8') PORT_CONDITION("LAYOUT", 0x0f, EQUALS, 0x00)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_8)           PORT_CHAR('8') PORT_CHAR('_') PORT_CONDITION("LAYOUT", 0x1f, EQUALS, 0x01)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_8)           PORT_CHAR('8') PORT_CHAR('(') PORT_CONDITION("LAYOUT", 0x1f, EQUALS, 0x11)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_8)           PORT_CHAR('8') PORT_CHAR('(') PORT_CONDITION("LAYOUT", 0x0f, EQUALS, 0x04)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_9)           PORT_CHAR(0x00e7U) PORT_CHAR('9') PORT_CONDITION("LAYOUT", 0x0f, EQUALS, 0x00)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_9)           PORT_CHAR('9') PORT_CHAR(0x00a7U) PORT_CONDITION("LAYOUT", 0x1f, EQUALS, 0x01)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_9)           PORT_CHAR('9') PORT_CHAR(')') PORT_CONDITION("LAYOUT", 0x1f, EQUALS, 0x11)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_9)           PORT_CHAR('9') PORT_CHAR(')') PORT_CONDITION("LAYOUT", 0x0f, EQUALS, 0x04)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_MINUS)       PORT_CHAR(')') PORT_CHAR(0x00b0U) PORT_CONDITION("LAYOUT", 0x0f, EQUALS, 0x00)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_MINUS)       PORT_CHAR(0x00dfU) PORT_CHAR(':') PORT_CONDITION("LAYOUT", 0x1f, EQUALS, 0x01)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_MINUS)       PORT_CHAR(0x00dfU) PORT_CHAR('?') PORT_CONDITION("LAYOUT", 0x1f, EQUALS, 0x11)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_MINUS)       PORT_CHAR('-') PORT_CHAR('=') PORT_CONDITION("LAYOUT", 0x0f, EQUALS, 0x04)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_EQUALS)      PORT_CHAR('-') PORT_CHAR('+') PORT_CONDITION("LAYOUT", 0x0f, EQUALS, 0x00)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_EQUALS)      PORT_CHAR('^') PORT_CHAR('`') PORT_CONDITION("LAYOUT", 0x1f, EQUALS, 0x01)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_EQUALS)      PORT_CHAR('\'') PORT_CHAR('`') PORT_CONDITION("LAYOUT", 0x1f, EQUALS, 0x11)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_EQUALS)      PORT_CHAR('^') PORT_CHAR('~') PORT_CONDITION("LAYOUT", 0x0f, EQUALS, 0x04)

	PORT_START("LINE5")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_OPENBRACE)   PORT_CHAR(0x00ecU) PORT_CHAR('=') PORT_CONDITION("LAYOUT", 0x0f, EQUALS, 0x00)
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_OPENBRACE)   PORT_CHAR(0x00fcU) PORT_CHAR(0x00dcU) PORT_CONDITION("LAYOUT", 0x0f, EQUALS, 0x01)
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_OPENBRACE)   PORT_CHAR('@') PORT_CHAR('`') PORT_CONDITION("LAYOUT", 0x0f, EQUALS, 0x04)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_CLOSEBRACE)  PORT_CHAR('$') PORT_CHAR('&') PORT_CONDITION("LAYOUT", 0x0f, EQUALS, 0x00)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_CLOSEBRACE)  PORT_CHAR('+') PORT_CHAR('*') PORT_CONDITION("LAYOUT", 0x0f, EQUALS, 0x01)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_CLOSEBRACE)  PORT_CHAR('[') PORT_CHAR('{') PORT_CONDITION("LAYOUT", 0x0f, EQUALS, 0x04)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_COLON)       PORT_CHAR('m') PORT_CHAR('M') PORT_CHAR(0x0dU) PORT_CONDITION("LAYOUT", 0x0f, EQUALS, 0x00)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_COLON)       PORT_CHAR(0x00f6U) PORT_CHAR(0x00d6U) PORT_CONDITION("LAYOUT", 0x0f, EQUALS, 0x01)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_COLON)       PORT_CHAR(';') PORT_CHAR('+') PORT_CONDITION("LAYOUT", 0x0f, EQUALS, 0x04)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_QUOTE)       PORT_CHAR(0x00f9U) PORT_CHAR('%') PORT_CONDITION("LAYOUT", 0x0f, EQUALS, 0x00)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_QUOTE)       PORT_CHAR(0x00e4U) PORT_CHAR(0x00c4U) PORT_CONDITION("LAYOUT", 0x0f, EQUALS, 0x01)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_QUOTE)       PORT_CHAR(':') PORT_CHAR('*') PORT_CONDITION("LAYOUT", 0x0f, EQUALS, 0x04)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_TILDE)       PORT_CHAR('*') PORT_CHAR(0x00a7U) PORT_CONDITION("LAYOUT", 0x0f, EQUALS, 0x00)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_TILDE)       PORT_CHAR('$') PORT_CHAR('#') PORT_CONDITION("LAYOUT", 0x1f, EQUALS, 0x01)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_TILDE)       PORT_CHAR('#') PORT_CHAR('^') PORT_CONDITION("LAYOUT", 0x1f, EQUALS, 0x11)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_TILDE)       PORT_CHAR(']') PORT_CHAR('}') PORT_CONDITION("LAYOUT", 0x0f, EQUALS, 0x04)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_COMMA)       PORT_CHAR(';') PORT_CHAR('.') PORT_CONDITION("LAYOUT", 0x0f, EQUALS, 0x00)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_COMMA)       PORT_CHAR(',') PORT_CHAR('?') PORT_CONDITION("LAYOUT", 0x1f, EQUALS, 0x01)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_COMMA)       PORT_CHAR(',') PORT_CHAR(';') PORT_CONDITION("LAYOUT", 0x1f, EQUALS, 0x11)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_COMMA)       PORT_CHAR(',') PORT_CHAR('<') PORT_CONDITION("LAYOUT", 0x0f, EQUALS, 0x04)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_STOP)        PORT_CHAR(':') PORT_CHAR('/') PORT_CONDITION("LAYOUT", 0x0f, EQUALS, 0x00)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_STOP)        PORT_CHAR('.') PORT_CHAR('!') PORT_CONDITION("LAYOUT", 0x1f, EQUALS, 0x01)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_STOP)        PORT_CHAR('.') PORT_CHAR(':') PORT_CONDITION("LAYOUT", 0x1f, EQUALS, 0x11)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_STOP)        PORT_CHAR('.') PORT_CHAR('>') PORT_CONDITION("LAYOUT", 0x0f, EQUALS, 0x04)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_SLASH)       PORT_CHAR(0x00f2U) PORT_CHAR('!') PORT_CONDITION("LAYOUT", 0x0f, EQUALS, 0x00)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_SLASH)       PORT_CHAR('-') PORT_CHAR('\'') PORT_CONDITION("LAYOUT", 0x1f, EQUALS, 0x01)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_SLASH)       PORT_CHAR('-') PORT_CHAR('_') PORT_CONDITION("LAYOUT", 0x1f, EQUALS, 0x11)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_SLASH)       PORT_CHAR('/') PORT_CHAR('?') PORT_CONDITION("LAYOUT", 0x0f, EQUALS, 0x04)

	PORT_START("LINE6")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD)                        PORT_CODE(KEYCODE_SPACE)      PORT_CHAR(' ')
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD)                        PORT_CODE(KEYCODE_ENTER)      PORT_CHAR(0x000dU)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("S1")        PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("S2")        PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Keypad .")  PORT_CODE(KEYCODE_DEL_PAD)    PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD)                        PORT_CODE(KEYCODE_0_PAD)      PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD)                        PORT_CODE(KEYCODE_ENTER_PAD)  PORT_CHAR(UCHAR_MAMEKEY(00_PAD))
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD)                        PORT_CODE(KEYCODE_1_PAD)      PORT_CHAR(UCHAR_MAMEKEY(1_PAD))

	PORT_START("LINE7")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD)                        PORT_CODE(KEYCODE_2_PAD)      PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD)                        PORT_CODE(KEYCODE_3_PAD)      PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD)                        PORT_CODE(KEYCODE_4_PAD)      PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD)                        PORT_CODE(KEYCODE_5_PAD)      PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD)                        PORT_CODE(KEYCODE_6_PAD)      PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD)                        PORT_CODE(KEYCODE_7_PAD)      PORT_CHAR(UCHAR_MAMEKEY(7_PAD))
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD)                        PORT_CODE(KEYCODE_8_PAD)      PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD)                        PORT_CODE(KEYCODE_9_PAD)      PORT_CHAR(UCHAR_MAMEKEY(9_PAD))

	PORT_START("LINE8")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD)                        PORT_CODE(KEYCODE_PLUS_PAD)   PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD))
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD)                        PORT_CODE(KEYCODE_MINUS_PAD)  PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD))
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD)                        PORT_CODE(KEYCODE_ASTERISK)   PORT_CHAR(UCHAR_MAMEKEY(ASTERISK))
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD)                        PORT_CODE(KEYCODE_SLASH_PAD)  PORT_CHAR(UCHAR_MAMEKEY(SLASH_PAD))
	PORT_BIT(0xf0,IP_ACTIVE_HIGH,IPT_UNUSED)

	PORT_START("MODIFIERS")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("COMMAND")   PORT_CODE(KEYCODE_TAB)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("CTRL")      PORT_CODE(KEYCODE_LCONTROL)   PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("R SHIFT")   PORT_CODE(KEYCODE_RSHIFT)     PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("L SHIFT")   PORT_CODE(KEYCODE_LSHIFT)
	PORT_BIT(0xf0,IP_ACTIVE_HIGH,IPT_UNUSED)
INPUT_PORTS_END
} // anonymous namespace


m20_keyboard_device::m20_keyboard_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock)
	: buffered_rs232_device(mconfig, M20_KEYBOARD, tag, owner, 0)
	, device_matrix_keyboard_interface(mconfig, *this, "LINE0", "LINE1", "LINE2", "LINE3", "LINE4", "LINE5", "LINE6", "LINE7", "LINE8")
	, m_modifiers(*this, "MODIFIERS")
	, m_layout(*this, "LAYOUT")
	, m_beeper(*this, "beeper")
	, m_bell_timer(nullptr)
{
}


ioport_constructor m20_keyboard_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(m20_keyboard);
}


void m20_keyboard_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beeper, 2000); // TODO: unknown frequency
	m_beeper->add_route(ALL_OUTPUTS, "mono", 1.0);
}

void m20_keyboard_device::device_start()
{
	buffered_rs232_device::device_start();
	m_bell_timer = timer_alloc(FUNC(m20_keyboard_device::bell_off), this);
}

TIMER_CALLBACK_MEMBER(m20_keyboard_device::bell_off)
{
	m_beeper->set_state(0);
}

void m20_keyboard_device::device_reset()
{
	buffered_rs232_device::device_reset();

	m_beeper->set_state(0);

	reset_key_state();
	clear_fifo();

	set_data_frame(1, 8, PARITY_NONE, STOP_BITS_2);
	set_rate(1'200);
	receive_register_reset();
	transmit_register_reset();

	output_dcd(0);
	output_dsr(0);
	output_cts(0);
	output_rxd(1);

	start_processing(attotime::from_hz(1'200));
}


void m20_keyboard_device::key_make(uint8_t row, uint8_t column)
{
	uint8_t const row_code(((row < 6U) ? row : (0x18U | (row - 6U))) << 3);
	uint8_t const modifiers(m_modifiers->read());
	uint8_t mod_code(0U);
	switch (modifiers)
	{
	case 0x01U: // COMMAND
		mod_code = 0x90;
		break;
	case 0x02U: // CTRL
		mod_code = 0x60;
		break;
	case 0x04U: // RSHIFT
	case 0x08U: // LSHIFT
	case 0x0cU: // LSHIFT|RSHIFT
		mod_code = 0x30;
		break;
	}
	transmit_byte((row_code | column) + mod_code);
}


void m20_keyboard_device::received_byte(uint8_t byte)
{
	switch (byte)
	{
	case 0x03U: // keyboard self-test / nationality
		transmit_byte(((m_layout->read() & 0x0fU) << 4) | 0x0fU);
		break;
	case 0x0aU: // BEEP
		m_beeper->set_state(1);
		m_bell_timer->reset(attotime::from_msec(50));
		break;
	case 0x80U:
		transmit_byte(0x80U);
		break;
	default:
		logerror("received unknown command %02x", byte);
	}
}


DEFINE_DEVICE_TYPE(M20_KEYBOARD, m20_keyboard_device, "m20_kbd", "Olivetti M20 Keyboard")
