// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Sega Saturn Keyboard emulation

**********************************************************************/

#include "emu.h"
#include "keybd.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(SATURN_KEYBD, saturn_keybd_device, "saturn_keybd", "Sega Saturn Keyboard")


static INPUT_PORTS_START( saturn_keybd )
	// TODO: there's no info about the keycode used on Saturn keyboard, the following is trial & error with Game Basic software
	PORT_START("KEY.0") // 0x00 - 0x07
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_UNUSED)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F1") /*PORT_CODE(KEYCODE_F1)*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x01)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("0-3") /*PORT_CODE(KEYCODE_2) PORT_CHAR('2')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x02)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F2") /*PORT_CODE(KEYCODE_F2)*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x03)  // RUN
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F3") /*PORT_CODE(KEYCODE_F3)*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x04)  // LIST
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F4") /*PORT_CODE(KEYCODE_F4)*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x05)  // EDIT
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F5") /*PORT_CODE(KEYCODE_F5)*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x06)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("CLR SCR") PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x07)

	PORT_START("KEY.1") // 0x08 - 0x0f
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("1-1") /*PORT_CODE(KEYCODE_1) PORT_CHAR('1')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x08)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F6") /*PORT_CODE(KEYCODE_F6)*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x09)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F7") /*PORT_CODE(KEYCODE_F7)*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x0a)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F8") /*PORT_CODE(KEYCODE_F8)*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x0b)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F9") /*PORT_CODE(KEYCODE_F9)*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x0c)  // LIST again
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("1-6") /*PORT_CODE(KEYCODE_5) PORT_CHAR('5')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x0d)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("1-7") /*PORT_CODE(KEYCODE_6) PORT_CHAR('6')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x0e)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("1-8") /*PORT_CODE(KEYCODE_7) PORT_CHAR('7')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x0f)

	PORT_START("KEY.2") // 0x10 - 0x17
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("2-1") /*PORT_CODE(KEYCODE_1) PORT_CHAR('1')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x10)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("2-2") /*PORT_CODE(KEYCODE_1) PORT_CHAR('1')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x11)
/* TODO: break codes! */
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("SHIFT") /*PORT_CODE(KEYCODE_2) PORT_CHAR('2')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x12)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("KANA SHIFT") /*PORT_CODE(KEYCODE_3) PORT_CHAR('3')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x13)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("(special keys)") /*PORT_CODE(KEYCODE_4) PORT_CHAR('4')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x14)

	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q) PORT_CHAR('Q') PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x15)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x16)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("2-8") /*PORT_CODE(KEYCODE_7) PORT_CHAR('7')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x17)

	PORT_START("KEY.3") // 0x18 - 0x1f
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("3-1") /*PORT_CODE(KEYCODE_1) PORT_CHAR('1')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x18)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("3-2") /*PORT_CODE(KEYCODE_1) PORT_CHAR('1')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x19)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z) PORT_CHAR('Z') PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x1a)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S) PORT_CHAR('S') PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x1b)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A) PORT_CHAR('A') PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x1c)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W) PORT_CHAR('W') PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x1d)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x1e)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("3-8") /*PORT_CODE(KEYCODE_7) PORT_CHAR('7')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x1f)

	PORT_START("KEY.4") // 0x20 - 0x27
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("4-1") /*PORT_CODE(KEYCODE_1) PORT_CHAR('1')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x20)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C) PORT_CHAR('C') PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x21)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X) PORT_CHAR('X') PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x22)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D) PORT_CHAR('D') PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x23)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E) PORT_CHAR('E') PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x24)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x25)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x26)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("4-8") /*PORT_CODE(KEYCODE_7) PORT_CHAR('7')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x27)

	PORT_START("KEY.5") // 0x28 - 0x2f
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("5-1") /*PORT_CODE(KEYCODE_F) PORT_CHAR('F')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x28)  // another F?
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("SPACE") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ') PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x29)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V) PORT_CHAR('V') PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x2a)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F) PORT_CHAR('F') PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x2b)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T) PORT_CHAR('T') PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x2c)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R) PORT_CHAR('R') PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x2d)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x2e)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("5-8") /*PORT_CODE(KEYCODE_7) PORT_CHAR('7')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x2f)

	PORT_START("KEY.6") // 0x30 - 0x37
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("6-1") /*PORT_CODE(KEYCODE_1) PORT_CHAR('1')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x30)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N) PORT_CHAR('N') PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x31)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B) PORT_CHAR('B') PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x32)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H) PORT_CHAR('H') PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x33)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G) PORT_CHAR('G') PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x34)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y) PORT_CHAR('Y') PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x35)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x36)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("6-8") /*PORT_CODE(KEYCODE_7) PORT_CHAR('7')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x37)

	PORT_START("KEY.7") // 0x38 - 0x3f
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("7-1") /*PORT_CODE(KEYCODE_1) PORT_CHAR('1')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x38)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("7-2") /*PORT_CODE(KEYCODE_1) PORT_CHAR('1')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x39)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M) PORT_CHAR('M') PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x3a)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J) PORT_CHAR('J') PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x3b)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U) PORT_CHAR('U') PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x3c)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x3d)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x3e)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("7-8") /*PORT_CODE(KEYCODE_7) PORT_CHAR('7')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x3f)

	PORT_START("KEY.8") // 0x40 - 0x47
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("8-1") /*PORT_CODE(KEYCODE_1) PORT_CHAR('1')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x40)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(",") /*PORT_CODE(KEYCODE_1) PORT_CHAR('1')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x41)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K) PORT_CHAR('K') PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x42)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I) PORT_CHAR('I') PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x43)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O) PORT_CHAR('O') PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x44)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x45)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x46)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("8-8") /*PORT_CODE(KEYCODE_7) PORT_CHAR('7')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x47)

	PORT_START("KEY.9") // 0x48 - 0x4f
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("9-1") /*PORT_CODE(KEYCODE_1) PORT_CHAR('1')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x48)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(".") /*PORT_CODE(KEYCODE_1) PORT_CHAR('1')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x49)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("/") /*PORT_CODE(KEYCODE_2) PORT_CHAR('2')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x4a)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L) PORT_CHAR('L') PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x4b)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(";") /*PORT_CODE(KEYCODE_4) PORT_CHAR('4')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x4c)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P) PORT_CHAR('P') PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x4d)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("- / =") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x4e)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("9-8") /*PORT_CODE(KEYCODE_7) PORT_CHAR('7')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x4f)

	PORT_START("KEY.10") // 0x50 - 0x57
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("A-1") /*PORT_CODE(KEYCODE_1) PORT_CHAR('1')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x50)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("\xC2\xA5") /*PORT_CODE(KEYCODE_1) PORT_CHAR('1')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x51)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(":") /*PORT_CODE(KEYCODE_2) PORT_CHAR('2')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x52)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("A-4") /*PORT_CODE(KEYCODE_3) PORT_CHAR('3')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x53)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("@") /*PORT_CODE(KEYCODE_4) PORT_CHAR('4')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x54)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("^") /*PORT_CODE(KEYCODE_5) PORT_CHAR(0xd)*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x55)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("A-7") /*PORT_CODE(KEYCODE_6) PORT_CHAR('6')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x56)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("A-8") /*PORT_CODE(KEYCODE_7) PORT_CHAR('7')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x57)

	PORT_START("KEY.11") // 0x58 - 0x5f
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("B-1") /*PORT_CODE(KEYCODE_1) PORT_CHAR('1')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x58)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("B-2") /*PORT_CODE(KEYCODE_1) PORT_CHAR('1')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x59)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("ENTER") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(0x0d) PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x5a)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("[") /*PORT_CODE(KEYCODE_3) PORT_CHAR('3')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x5b)  // {
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("B-5") /*PORT_CODE(KEYCODE_4) PORT_CHAR('4')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x5c)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("]") /*PORT_CODE(KEYCODE_5) PORT_CHAR(0xd)*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x5d)  // }
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("B-7") /*PORT_CODE(KEYCODE_6) PORT_CHAR('6')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x5e)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("B-8") /*PORT_CODE(KEYCODE_7) PORT_CHAR('7')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x5f)

	PORT_START("KEY.12") // 0x60 - 0x67
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("C-1") /*PORT_CODE(KEYCODE_1) PORT_CHAR('1')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x60)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("C-2") /*PORT_CODE(KEYCODE_1) PORT_CHAR('1')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x61)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("C-3") /*PORT_CODE(KEYCODE_2) PORT_CHAR('2')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x62)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("C-4") /*PORT_CODE(KEYCODE_3) PORT_CHAR('3')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x63)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("C-5") /*PORT_CODE(KEYCODE_4) PORT_CHAR('4')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x64)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("C-6") /*PORT_CODE(KEYCODE_5) PORT_CHAR(0xd)*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x65)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("BACKSPACE") PORT_CODE(KEYCODE_BACKSPACE) /* PORT_CHAR('6')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x66)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("C-8") /*PORT_CODE(KEYCODE_7) PORT_CHAR('7')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x67)

	PORT_START("KEY.13") // 0x68 - 0x6f
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("D-1") /*PORT_CODE(KEYCODE_1) PORT_CHAR('1')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x68)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("D-2") /*PORT_CODE(KEYCODE_1) PORT_CHAR('1')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x69)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("D-3") /*PORT_CODE(KEYCODE_2) PORT_CHAR('2')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x6a)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("D-4") /*PORT_CODE(KEYCODE_3) PORT_CHAR('3')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x6b)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("D-5") /*PORT_CODE(KEYCODE_4) PORT_CHAR('4')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x6c)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("D-6") /*PORT_CODE(KEYCODE_5) PORT_CHAR(0xd)*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x6d)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("D-7") /*PORT_CODE(KEYCODE_6) PORT_CHAR('6')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x6e)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("D-8") /*PORT_CODE(KEYCODE_7) PORT_CHAR('7')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x6f)

	PORT_START("KEY.14") // 0x70 - 0x77
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("E-1") /*PORT_CODE(KEYCODE_1) PORT_CHAR('1')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x70)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("E-2") /*PORT_CODE(KEYCODE_1) PORT_CHAR('1')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x71)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("E-3") /*PORT_CODE(KEYCODE_2) PORT_CHAR('2')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x72)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("E-4") /*PORT_CODE(KEYCODE_3) PORT_CHAR('3')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x73)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("E-5") /*PORT_CODE(KEYCODE_4) PORT_CHAR('4')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x74)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("E-6") /*PORT_CODE(KEYCODE_5) PORT_CHAR(0xd)*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x75)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("E-7") /*PORT_CODE(KEYCODE_6) PORT_CHAR('6')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x76)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("E-8") /*PORT_CODE(KEYCODE_7) PORT_CHAR('7')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x77)

	PORT_START("KEY.15") // 0x78 - 0x7f
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F-1") /*PORT_CODE(KEYCODE_1) PORT_CHAR('1')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x78)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F-2") /*PORT_CODE(KEYCODE_1) PORT_CHAR('1')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x79)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F-3") /*PORT_CODE(KEYCODE_2) PORT_CHAR('2')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x7a)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F-4") /*PORT_CODE(KEYCODE_3) PORT_CHAR('3')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x7b)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F-5") /*PORT_CODE(KEYCODE_4) PORT_CHAR('4')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x7c)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F-6") /*PORT_CODE(KEYCODE_5) PORT_CHAR(0xd)*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x7d)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F-7") /*PORT_CODE(KEYCODE_6) PORT_CHAR('6')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x7e)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("ESC") /*PORT_CODE(KEYCODE_7) PORT_CHAR('7')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x7f)  //SYSTEM CONFIGURATION

	PORT_START("KEYS_1") // special keys
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("UP") PORT_CODE(KEYCODE_UP) /*PORT_CHAR('1')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x78)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("DOWN") PORT_CODE(KEYCODE_DOWN) /*PORT_CHAR('1')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x79)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("LEFT") PORT_CODE(KEYCODE_LEFT) /*PORT_CHAR('2')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x7a)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("RIGHT") PORT_CODE(KEYCODE_RIGHT) /*PORT_CHAR('3')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_keybd_device, key_stroke, 0x7b)
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor saturn_keybd_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( saturn_keybd );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  saturn_keybd_device - constructor
//-------------------------------------------------

saturn_keybd_device::saturn_keybd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, SATURN_KEYBD, tag, owner, clock),
	device_saturn_control_port_interface(mconfig, *this),
	m_key(*this, "KEY.%u", 0),
	m_key_s1(*this, "KEYS_1")
{
	m_ctrl_id = 0x34;
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void saturn_keybd_device::device_start()
{
	save_item(NAME(m_status));
	save_item(NAME(m_data));
	save_item(NAME(m_prev_data));
	save_item(NAME(m_repeat_count));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void saturn_keybd_device::device_reset()
{
	m_status = 0;
	m_data = 0;
	m_prev_data = 0;
	m_repeat_count = 0;
}


//-------------------------------------------------
//  read_ctrl
//-------------------------------------------------

/* TODO: needs a proper keycode table */
INPUT_CHANGED_MEMBER(saturn_keybd_device::key_stroke)
{
	if (newval && !oldval)
	{
		m_data = (uint8_t(param) & 0xff);
		m_status |= 8;
	}

	if(oldval && !newval)
	{
		//m_status &= ~8;
		m_data = 0;
	}
}

uint16_t saturn_keybd_device::get_game_key()
{
	uint16_t game_key = 0xffff;

	game_key ^= ((m_key_s1->read() & 0x80) << 8); // right
	game_key ^= ((m_key_s1->read() & 0x40) << 8); // left
	game_key ^= ((m_key_s1->read() & 0x20) << 8); // down
	game_key ^= ((m_key_s1->read() & 0x10) << 8); // up
	game_key ^= ((m_key[0xf]->read() & 0x80) << 4); // ESC -> START
	game_key ^= ((m_key[0x3]->read() & 0x04) << 8); // Z / A trigger
	game_key ^= ((m_key[0x4]->read() & 0x02) << 8); // C / C trigger
	game_key ^= ((m_key[0x6]->read() & 0x04) << 6); // X / B trigger
	game_key ^= ((m_key[0x2]->read() & 0x20) << 2); // Q / R trigger
	game_key ^= ((m_key[0x3]->read() & 0x10) << 2); // A / X trigger
	game_key ^= ((m_key[0x3]->read() & 0x08) << 2); // S / Y trigger
	game_key ^= ((m_key[0x4]->read() & 0x08) << 1); // D / Z trigger
	game_key ^= ((m_key[0x4]->read() & 0x10) >> 1); // E / L trigger

	return game_key;
}

uint8_t saturn_keybd_device::read_ctrl(uint8_t offset)
{
	uint8_t res = 0;

	/*
	 Keyboard Status hook-up
	 TODO: how shift key actually works? EGWord uses it in order to switch between hiragana and katakana modes.
	 x--- ---- 0
	 -x-- ---- caps lock
	 --x- ---- num lock
	 ---x ---- scroll lock
	 ---- x--- data ok
	 ---- -x-- 1
	 ---- --x- 1
	 ---- ---x Break key
	 */

	switch (offset)
	{
		case 0:
		default:
			res = get_game_key() >> 8;
			break;
		case 1:
			res = get_game_key() & 0xff;
			break;
		case 3:
			res = m_status | 6;
			break;
		case 4:
			if (m_prev_data != m_data)
			{
				res = m_data;
				m_repeat_count = 0;
				m_prev_data = m_data;
			}
			else
			{
				/* Very crude repeat support */
				m_repeat_count++;
				m_repeat_count = m_repeat_count > 32 ? 32 : m_repeat_count;
				res = (m_repeat_count == 32) ? m_data : 0;
			}
			break;
	}
	return res;
}
