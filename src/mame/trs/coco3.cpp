// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    coco3.c

    TRS-80 Radio Shack Color Computer 3

    Mathis Rosenhauer (original driver)
    Nate Woods (current maintainer)
    Tim Lindner (VHD and other work)

***************************************************************************/

#include "emu.h"

#include "coco3.h"

#include "cpu/m6809/m6809.h"
#include "cpu/m6809/hd6309.h"

#include "softlist_dev.h"

#include "formats/coco_cas.h"



//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

//-------------------------------------------------
//  ADDRESS_MAP( coco3_mem )
//-------------------------------------------------

void coco3_state::coco3_mem(address_map &map)
{
	map(0x0000, 0x1FFF).bankr("rbank0").bankw("wbank0");
	map(0x2000, 0x3FFF).bankr("rbank1").bankw("wbank1");
	map(0x4000, 0x5FFF).bankr("rbank2").bankw("wbank2");
	map(0x6000, 0x7FFF).bankr("rbank3").bankw("wbank3");
	map(0x8000, 0x9FFF).bankr("rbank4").bankw("wbank4");
	map(0xA000, 0xBFFF).bankr("rbank5").bankw("wbank5");
	map(0xC000, 0xDFFF).bankr("rbank6").bankw("wbank6");
	map(0xE000, 0xFDFF).bankr("rbank7").bankw("wbank7");
	map(0xFE00, 0xFEFF).bankr("rbank8").bankw("wbank8");
	map(0xFF00, 0xFF0F).rw(m_pia_0, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0xFF20, 0xFF2F).r(m_pia_1, FUNC(pia6821_device::read)).w(FUNC(coco3_state::ff20_write));
	map(0xFF40, 0xFF5F).rw(FUNC(coco3_state::ff40_read), FUNC(coco3_state::ff40_write));
	map(0xFF60, 0xFF8F).rw(FUNC(coco3_state::ff60_read), FUNC(coco3_state::ff60_write));
	map(0xFF90, 0xFFDF).rw(m_gime, FUNC(gime_device::read), FUNC(gime_device::write));

	// While Tepolt and other sources say that the interrupt vectors are mapped to
	// the same memory accessed at $BFFx, William Astle offered evidence that this
	// memory on a CoCo 3 is not the same.
	//
	// http://lost.l-w.ca/0x05/coco3-and-interrupt-vectors/
	map(0xFFE0, 0xFFFF).rom().region(MAINCPU_TAG, 0x7FE0);
}



//**************************************************************************
//  INPUT PORTS
//**************************************************************************

//-------------------------------------------------
//  INPUT_PORTS( coco3_keyboard )
//-------------------------------------------------
//
//  CoCo 3 keyboard
//
//         PB0 PB1 PB2 PB3 PB4 PB5 PB6 PB7
//    PA6: Ent Clr Brk Alt Ctr F1  F2 Shift
//    PA5: 8   9   :   ;   ,   -   .   /
//    PA4: 0   1   2   3   4   5   6   7
//    PA3: X   Y   Z   Up  Dwn Lft Rgt Space
//    PA2: P   Q   R   S   T   U   V   W
//    PA1: H   I   J   K   L   M   N   O
//    PA0: @   A   B   C   D   E   F   G
//-------------------------------------------------

static INPUT_PORTS_START( coco3_keyboard )
	PORT_START("row0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco3_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('@')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco3_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco3_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco3_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco3_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco3_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco3_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco3_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')

	PORT_START("row1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco3_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco3_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco3_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco3_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco3_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco3_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco3_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco3_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')

	PORT_START("row2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco3_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco3_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco3_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco3_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco3_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco3_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco3_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco3_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')

	PORT_START("row3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco3_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco3_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco3_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco3_state, coco_state::keyboard_changed, 0) PORT_NAME("UP") PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco3_state, coco_state::keyboard_changed, 0) PORT_NAME("DOWN") PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN), 10)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco3_state, coco_state::keyboard_changed, 0) PORT_NAME("LEFT") PORT_CODE(KEYCODE_LEFT) PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(UCHAR_MAMEKEY(LEFT), 8)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco3_state, coco_state::keyboard_changed, 0) PORT_NAME("RIGHT") PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT), 9)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco3_state, coco_state::keyboard_changed, 0) PORT_NAME("SPACE") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')

	PORT_START("row4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco3_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco3_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!') PORT_CHAR('|')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco3_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('\"')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco3_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#') PORT_CHAR('~')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco3_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco3_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco3_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco3_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'') PORT_CHAR('^')

	PORT_START("row5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco3_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(') PORT_CHAR('[')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco3_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')') PORT_CHAR(']')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco3_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_MINUS) PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco3_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco3_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<') PORT_CHAR('{')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco3_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('-') PORT_CHAR('=') PORT_CHAR('_')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco3_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>') PORT_CHAR('}')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco3_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?') PORT_CHAR('\\')

	PORT_START("row6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco3_state, coco_state::keyboard_changed, 0) PORT_NAME("ENTER") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco3_state, coco_state::keyboard_changed, 0) PORT_NAME("CLEAR") PORT_CODE(KEYCODE_HOME) PORT_CHAR(12)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco3_state, coco_state::keyboard_changed, 0) PORT_NAME("BREAK") PORT_CODE(KEYCODE_END) PORT_CODE(KEYCODE_ESC) PORT_CHAR(27)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco3_state, coco_state::keyboard_changed, 0) PORT_NAME("ALT") PORT_CODE(KEYCODE_LALT) PORT_CHAR(UCHAR_MAMEKEY(LALT))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco3_state, coco_state::keyboard_changed, 0) PORT_NAME("CTRL") PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_MAMEKEY(LCONTROL), UCHAR_SHIFT_2)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco3_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_F1) PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco3_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_F2) PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco3_state, coco_state::keyboard_changed, 0) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
INPUT_PORTS_END



//-------------------------------------------------
//  INPUT_PORTS( coco3_joystick )
//-------------------------------------------------

static INPUT_PORTS_START( coco3_joystick )
	PORT_START(JOYSTICK_RX_TAG)
	PORT_BIT( 0x3ff, 0x200,  IPT_AD_STICK_X) PORT_SENSITIVITY(JOYSTICK_SENSITIVITY) PORT_KEYDELTA(JOYSTICK_DELTA) PORT_MINMAX(0, 1023) PORT_CODE_DEC(KEYCODE_4_PAD) PORT_CODE_INC(KEYCODE_6_PAD) PORT_CODE_DEC(JOYCODE_X_LEFT_SWITCH) PORT_CODE_INC(JOYCODE_X_RIGHT_SWITCH) PORT_PLAYER(1) PORT_CONDITION(CTRL_SEL_TAG, 0x0f, EQUALS, 0x01)
	PORT_START(JOYSTICK_RY_TAG)
	PORT_BIT( 0x3ff, 0x200,  IPT_AD_STICK_Y) PORT_SENSITIVITY(JOYSTICK_SENSITIVITY) PORT_KEYDELTA(JOYSTICK_DELTA) PORT_MINMAX(0, 1023) PORT_CODE_DEC(KEYCODE_8_PAD) PORT_CODE_INC(KEYCODE_2_PAD) PORT_CODE_DEC(JOYCODE_Y_UP_SWITCH)   PORT_CODE_INC(JOYCODE_Y_DOWN_SWITCH)  PORT_PLAYER(1) PORT_CONDITION(CTRL_SEL_TAG, 0x0f, EQUALS, 0x01)
	PORT_START(JOYSTICK_LX_TAG)
	PORT_BIT( 0x3ff, 0x200,  IPT_AD_STICK_X) PORT_SENSITIVITY(JOYSTICK_SENSITIVITY) PORT_KEYDELTA(JOYSTICK_DELTA) PORT_MINMAX(0, 1023) PORT_CODE_DEC(KEYCODE_4_PAD) PORT_CODE_INC(KEYCODE_6_PAD) PORT_CODE_DEC(JOYCODE_X_LEFT_SWITCH) PORT_CODE_INC(JOYCODE_X_RIGHT_SWITCH) PORT_PLAYER(2) PORT_CONDITION(CTRL_SEL_TAG, 0xf0, EQUALS, 0x10)
	PORT_START(JOYSTICK_LY_TAG)
	PORT_BIT( 0x3ff, 0x200,  IPT_AD_STICK_Y) PORT_SENSITIVITY(JOYSTICK_SENSITIVITY) PORT_KEYDELTA(JOYSTICK_DELTA) PORT_MINMAX(0, 1023) PORT_CODE_DEC(KEYCODE_8_PAD) PORT_CODE_INC(KEYCODE_2_PAD) PORT_CODE_DEC(JOYCODE_Y_UP_SWITCH)   PORT_CODE_INC(JOYCODE_Y_DOWN_SWITCH)  PORT_PLAYER(2) PORT_CONDITION(CTRL_SEL_TAG, 0xf0, EQUALS, 0x10)
	PORT_START(JOYSTICK_BUTTONS_TAG)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Right Button 1") PORT_CHANGED_MEMBER(DEVICE_SELF, coco3_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_0_PAD) PORT_CODE(JOYCODE_BUTTON1) PORT_CODE(MOUSECODE_BUTTON1) PORT_PLAYER(1) PORT_CONDITION(CTRL_SEL_TAG, 0x0f, EQUALS, 0x01)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Right Button 2") PORT_CHANGED_MEMBER(DEVICE_SELF, coco3_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_DEL_PAD) PORT_CODE(JOYCODE_BUTTON2) PORT_CODE(MOUSECODE_BUTTON2) PORT_PLAYER(1) PORT_CONDITION(CTRL_SEL_TAG, 0x0f, EQUALS, 0x01)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Left Button 1")  PORT_CHANGED_MEMBER(DEVICE_SELF, coco3_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_0_PAD) PORT_CODE(JOYCODE_BUTTON1) PORT_CODE(MOUSECODE_BUTTON1) PORT_PLAYER(2) PORT_CONDITION(CTRL_SEL_TAG, 0xf0, EQUALS, 0x10)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Left Button 2")  PORT_CHANGED_MEMBER(DEVICE_SELF, coco3_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_DEL_PAD) PORT_CODE(JOYCODE_BUTTON2) PORT_CODE(MOUSECODE_BUTTON2) PORT_PLAYER(2) PORT_CONDITION(CTRL_SEL_TAG, 0xf0, EQUALS, 0x10)
INPUT_PORTS_END



//-------------------------------------------------
//  INPUT_PORTS( coco_rat_mouse )
//-------------------------------------------------

static INPUT_PORTS_START( coco_rat_mouse )
	PORT_START(RAT_MOUSE_RX_TAG)
	PORT_BIT( 0x03, 0x00,  IPT_MOUSE_X) PORT_NAME("Rat Mouse X (Right Port)") PORT_SENSITIVITY(100) PORT_KEYDELTA(1) PORT_CODE_DEC(KEYCODE_LEFT) PORT_CODE_INC(KEYCODE_RIGHT) PORT_CODE_DEC(JOYCODE_X_LEFT_SWITCH) PORT_CODE_INC(JOYCODE_X_RIGHT_SWITCH) PORT_PLAYER(1)  PORT_CONDITION(CTRL_SEL_TAG, 0x0f, EQUALS, 0x02)
	PORT_START(RAT_MOUSE_RY_TAG)
	PORT_BIT( 0x03, 0x00,  IPT_MOUSE_Y) PORT_NAME("Rat Mouse Y (Right Port)") PORT_SENSITIVITY(100) PORT_KEYDELTA(1) PORT_CODE_DEC(KEYCODE_UP) PORT_CODE_INC(KEYCODE_DOWN) PORT_CODE_DEC(JOYCODE_Y_UP_SWITCH) PORT_CODE_INC(JOYCODE_Y_DOWN_SWITCH) PORT_PLAYER(1) PORT_CONDITION(CTRL_SEL_TAG, 0x0f, EQUALS, 0x02)
	PORT_START(RAT_MOUSE_LX_TAG)
	PORT_BIT( 0x03, 0x00,  IPT_MOUSE_X) PORT_NAME("Rat Mouse X (Left Port)") PORT_SENSITIVITY(100) PORT_KEYDELTA(1) PORT_CODE_DEC(KEYCODE_LEFT) PORT_CODE_INC(KEYCODE_RIGHT) PORT_CODE_DEC(JOYCODE_X_LEFT_SWITCH) PORT_CODE_INC(JOYCODE_X_RIGHT_SWITCH) PORT_PLAYER(2)  PORT_CONDITION(CTRL_SEL_TAG, 0xf0, EQUALS, 0x20)
	PORT_START(RAT_MOUSE_LY_TAG)
	PORT_BIT( 0x03, 0x00,  IPT_MOUSE_Y) PORT_NAME("Rat Mouse Y (Left Port)") PORT_SENSITIVITY(100) PORT_KEYDELTA(1) PORT_CODE_DEC(KEYCODE_UP) PORT_CODE_INC(KEYCODE_DOWN) PORT_CODE_DEC(JOYCODE_Y_UP_SWITCH) PORT_CODE_INC(JOYCODE_Y_DOWN_SWITCH) PORT_PLAYER(2) PORT_CONDITION(CTRL_SEL_TAG, 0xf0, EQUALS, 0x20)
	PORT_START(RAT_MOUSE_BUTTONS_TAG)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Rat Mouse Button 1 (Right Port)") PORT_CHANGED_MEMBER(DEVICE_SELF, coco3_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_0_PAD) PORT_CODE(JOYCODE_BUTTON1) PORT_CODE(MOUSECODE_BUTTON1) PORT_PLAYER(1) PORT_CONDITION(CTRL_SEL_TAG, 0x0f, EQUALS, 0x02)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Rat Mouse Button 2 (Right Port)") PORT_CHANGED_MEMBER(DEVICE_SELF, coco3_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_DEL_PAD) PORT_CODE(JOYCODE_BUTTON2) PORT_CODE(MOUSECODE_BUTTON2) PORT_PLAYER(1) PORT_CONDITION(CTRL_SEL_TAG, 0x0f, EQUALS, 0x02)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Rat Mouse Button 1 (Left Port)")  PORT_CHANGED_MEMBER(DEVICE_SELF, coco3_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_0_PAD) PORT_CODE(JOYCODE_BUTTON1) PORT_CODE(MOUSECODE_BUTTON1) PORT_PLAYER(2) PORT_CONDITION(CTRL_SEL_TAG, 0xf0, EQUALS, 0x20)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Rat Mouse Button 2 (Left Port)")  PORT_CHANGED_MEMBER(DEVICE_SELF, coco3_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_DEL_PAD) PORT_CODE(JOYCODE_BUTTON2) PORT_CODE(MOUSECODE_BUTTON2) PORT_PLAYER(2) PORT_CONDITION(CTRL_SEL_TAG, 0xf0, EQUALS, 0x20)
INPUT_PORTS_END



//-------------------------------------------------
//  INPUT_PORTS( coco_lightgun )
//-------------------------------------------------

static INPUT_PORTS_START( coco_lightgun )
	PORT_START(DIECOM_LIGHTGUN_RX_TAG)
	PORT_BIT( 0xfff, 266, IPT_LIGHTGUN_X ) PORT_NAME("Lightgun X (Right Port)") PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_MINMAX(105,420) PORT_SENSITIVITY(100) PORT_KEYDELTA(1) PORT_PLAYER(1) PORT_CONDITION(CTRL_SEL_TAG, 0x0f, EQUALS, 0x03)
	PORT_START(DIECOM_LIGHTGUN_RY_TAG)
	PORT_BIT( 0xff, 121, IPT_LIGHTGUN_Y ) PORT_NAME("Lightgun Y (Right Port)") PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_MINMAX(0,242) PORT_SENSITIVITY(100) PORT_KEYDELTA(1) PORT_PLAYER(1) PORT_CONDITION(CTRL_SEL_TAG, 0x0f, EQUALS, 0x03)
	PORT_START(DIECOM_LIGHTGUN_LX_TAG)
	PORT_BIT( 0xfff, 266, IPT_LIGHTGUN_X ) PORT_NAME("Lightgun X (Left Port)") PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_MINMAX(105,420) PORT_SENSITIVITY(100) PORT_KEYDELTA(1) PORT_PLAYER(1) PORT_CONDITION(CTRL_SEL_TAG, 0xf0, EQUALS, 0x30)
	PORT_START(DIECOM_LIGHTGUN_LY_TAG)
	PORT_BIT( 0xff, 121, IPT_LIGHTGUN_Y ) PORT_NAME("Lightgun Y (Left Port)") PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_MINMAX(0,242) PORT_SENSITIVITY(100) PORT_KEYDELTA(1) PORT_PLAYER(1) PORT_CONDITION(CTRL_SEL_TAG, 0xf0, EQUALS, 0x30)
	PORT_START(DIECOM_LIGHTGUN_BUTTONS_TAG)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Lightgun Trigger (Right Port)") PORT_CHANGED_MEMBER(DEVICE_SELF, coco3_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_0_PAD) PORT_CODE(JOYCODE_BUTTON1) PORT_CODE(MOUSECODE_BUTTON1) PORT_PLAYER(1) PORT_CONDITION(CTRL_SEL_TAG, 0x0f, EQUALS, 0x03)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Lightgun Trigger (Left Port)")  PORT_CHANGED_MEMBER(DEVICE_SELF, coco3_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_0_PAD) PORT_CODE(JOYCODE_BUTTON1) PORT_CODE(MOUSECODE_BUTTON1) PORT_PLAYER(2) PORT_CONDITION(CTRL_SEL_TAG, 0xf0, EQUALS, 0x30)
INPUT_PORTS_END



//-------------------------------------------------
//  INPUT_PORTS( coco3 )
//-------------------------------------------------

static INPUT_PORTS_START( screen_config )
	PORT_START("screen_config")
	PORT_CONFNAME( 0x01, 0x00, "Monitor Type" )
	PORT_CONFSETTING(    0x00, "Composite" )
	PORT_CONFSETTING(    0x01, "RGB" )
INPUT_PORTS_END

static INPUT_PORTS_START( coco3 )
	PORT_INCLUDE( coco3_keyboard )
	PORT_INCLUDE( coco3_joystick )
	PORT_INCLUDE( coco_analog_control )
	PORT_INCLUDE( coco_rat_mouse )
	PORT_INCLUDE( coco_lightgun )
	PORT_INCLUDE( coco_beckerport )
	PORT_INCLUDE( screen_config )
INPUT_PORTS_END

static DEVICE_INPUT_DEFAULTS_START( rs_printer )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_600 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_8 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_NONE )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
DEVICE_INPUT_DEFAULTS_END


//**************************************************************************
//  MACHINE CONFIGURATION
//**************************************************************************

void coco3_state::coco3(machine_config &config)
{
	this->set_clock(XTAL(28'636'363) / 32);

	// basic machine hardware
	MC6809E(config, m_maincpu, DERIVED_CLOCK(1, 1));
	m_maincpu->set_addrmap(AS_PROGRAM, &coco3_state::coco3_mem);
	m_maincpu->set_dasm_override(FUNC(coco_state::dasm_override));

	// devices
	INPUT_MERGER_ANY_HIGH(config, m_irqs).output_handler().set_inputline(m_maincpu, M6809_IRQ_LINE);
	INPUT_MERGER_ANY_HIGH(config, m_firqs).output_handler().set_inputline(m_maincpu, M6809_FIRQ_LINE);

	PIA6821(config, m_pia_0);
	m_pia_0->writepa_handler().set(FUNC(coco_state::pia0_pa_w));
	m_pia_0->writepb_handler().set(FUNC(coco_state::pia0_pb_w));
	m_pia_0->tspb_handler().set_constant(0xff);
	m_pia_0->ca2_handler().set(FUNC(coco_state::pia0_ca2_w));
	m_pia_0->cb2_handler().set(FUNC(coco_state::pia0_cb2_w));
	m_pia_0->irqa_handler().set(m_irqs, FUNC(input_merger_device::in_w<0>));
	m_pia_0->irqb_handler().set(m_irqs, FUNC(input_merger_device::in_w<1>));

	PIA6821(config, m_pia_1);
	m_pia_1->readpa_handler().set(FUNC(coco_state::pia1_pa_r));
	m_pia_1->readpb_handler().set(FUNC(coco_state::pia1_pb_r));
	m_pia_1->writepa_handler().set(FUNC(coco_state::pia1_pa_w));
	m_pia_1->writepb_handler().set(FUNC(coco_state::pia1_pb_w));
	m_pia_1->ca2_handler().set(FUNC(coco_state::pia1_ca2_w));
	m_pia_1->cb2_handler().set(FUNC(coco_state::pia1_cb2_w));
	m_pia_1->irqa_handler().set(m_firqs, FUNC(input_merger_device::in_w<0>));
	m_pia_1->irqb_handler().set(m_firqs, FUNC(input_merger_device::in_w<1>));

	// Becker Port device
	COCO_DWSOCK(config, m_beckerport, 0);

	// sound hardware
	coco_sound(config);

	CASSETTE(config, m_cassette);
	m_cassette->set_formats(coco_cassette_formats);
	m_cassette->set_default_state(CASSETTE_PLAY | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_ENABLED);

	rs232_port_device &rs232(RS232_PORT(config, RS232_TAG, default_rs232_devices, "rs_printer"));
	rs232.dcd_handler().set(m_pia_1, FUNC(pia6821_device::ca1_w));
	rs232.set_option_device_input_defaults("rs_printer", DEVICE_INPUT_DEFAULTS_NAME(rs_printer));

	COCO_VHD(config, m_vhd_0, 0, m_maincpu);
	COCO_VHD(config, m_vhd_1, 0, m_maincpu);

	// video hardware
	GIME_NTSC(config, m_gime, XTAL(28'636'363), MAINCPU_TAG, RAM_TAG, m_cococart, MAINCPU_TAG);
	m_gime->set_screen("screen");
	m_gime->hsync_wr_callback().set(m_pia_0, FUNC(pia6821_device::ca1_w));
	m_gime->fsync_wr_callback().set(m_pia_0, FUNC(pia6821_device::cb1_w));
	m_gime->irq_wr_callback().set(m_irqs, FUNC(input_merger_device::in_w<2>));
	m_gime->firq_wr_callback().set(m_firqs, FUNC(input_merger_device::in_w<2>));
	m_gime->floating_bus_rd_callback().set(FUNC(coco3_state::floating_bus_r));

	// monitor
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(28.636363_MHz_XTAL/2, 912, 0, 640-1, 262, 1, 241-1);
	m_screen->set_screen_update(FUNC(coco3_state::screen_update));

	// internal ram
	RAM(config, RAM_TAG).set_default_size("512K").set_extra_options("128K,2M,8M");

	// floating space
	coco_floating(config);

	// cartridge
	COCOCART_SLOT(config, m_cococart, DERIVED_CLOCK(1, 1), coco_cart, "fdc");
	m_cococart->cart_callback().set([this] (int state) { cart_w(state != 0); }); // lambda because name is overloaded
	m_cococart->nmi_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);
	m_cococart->halt_callback().set_inputline(m_maincpu, INPUT_LINE_HALT);

	// software lists
	SOFTWARE_LIST(config, "cart_list").set_original("coco_cart").set_filter("COCO3");
	SOFTWARE_LIST(config, "cass_list").set_original("coco_cass").set_filter("COCO3");
	SOFTWARE_LIST(config, "flop_list").set_original("coco_flop").set_filter("COCO3");
}

void coco3_state::coco3p(machine_config &config)
{
	coco3(config);
	this->set_clock(XTAL(28'475'000) / 32);

	// An additional 4.433618 MHz XTAL is required for PAL color encoding
	GIME_PAL(config.replace(), m_gime, XTAL(28'475'000), MAINCPU_TAG, RAM_TAG, m_cococart, MAINCPU_TAG);
	m_gime->set_screen("screen");
	m_gime->hsync_wr_callback().set(m_pia_0, FUNC(pia6821_device::ca1_w));
	m_gime->fsync_wr_callback().set(m_pia_0, FUNC(pia6821_device::cb1_w));
	m_gime->irq_wr_callback().set(m_irqs, FUNC(input_merger_device::in_w<2>));
	m_gime->firq_wr_callback().set(m_firqs, FUNC(input_merger_device::in_w<2>));
	m_gime->floating_bus_rd_callback().set(FUNC(coco3_state::floating_bus_r));
}

void coco3_state::coco3h(machine_config &config)
{
	coco3(config);
	HD6309E(config.replace(), m_maincpu, DERIVED_CLOCK(1, 1));
	m_maincpu->set_addrmap(AS_PROGRAM, &coco3_state::coco3_mem);
}

//**************************************************************************
//  ROMS
//**************************************************************************

ROM_START(coco3)
	ROM_REGION(0x8000,MAINCPU_TAG,0)
	ROM_LOAD("coco3.rom",   0x0000, 0x8000, CRC(b4c88d6c) SHA1(e0d82953fb6fd03768604933df1ce8bc51fc427d))
ROM_END

ROM_START(coco3p)
	ROM_REGION(0x8000,MAINCPU_TAG,0)
	ROM_LOAD("coco3p.rom",  0x0000, 0x8000, CRC(ff050d80) SHA1(631e383068b1f52a8f419f4114b69501b21cf379))
ROM_END

ROM_START(msm3)
	ROM_REGION(0x8000,MAINCPU_TAG,0)
	ROM_LOAD("msm3.rom", 0x0000, 0x8000, CRC(26d67890) SHA1(271fd39b3eb3f521aa5484ae7ce3956ab9ef782c))
ROM_END

#define rom_coco3h  rom_coco3

//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME      PARENT COMPAT MACHINE   INPUT    CLASS        INIT        COMPANY              FULLNAME                            FLAGS
COMP( 1986, coco3,    coco,  0,     coco3,    coco3,   coco3_state, empty_init, "Tandy Radio Shack", "Color Computer 3 (NTSC)",          0 )
COMP( 1986, coco3p,   coco,  0,     coco3p,   coco3,   coco3_state, empty_init, "Tandy Radio Shack", "Color Computer 3 (PAL)",           0 )
COMP( 19??, coco3h,   coco,  0,     coco3h,   coco3,   coco3_state, empty_init, "Tandy Radio Shack", "Color Computer 3 (NTSC; HD6309)",  MACHINE_UNOFFICIAL )
COMP( 1987, msm3,     coco,  0,     coco3,    coco3,   coco3_state, empty_init, "ILCE / SEP",        "Micro-Sep Model 3",                0 )
