// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    coco12.cpp

    TRS-80 Radio Shack Color Computer 1/2

    Mathis Rosenhauer (original driver)
    Nate Woods (current maintainer)
    Tim Lindner (VHD and other work)

    Version information:

    Basic   Extended Basic  Disk Extended
    coco    1.0 -           -
    cocoe   1.1 1.0         1.0
    coco2   1.2 1.1         1.1
    coco2b  1.3 1.1         1.1
    coco3   1.2 2.0         1.1
    coco3p  1.2 2.0         1.1
    coco3h  1.2 2.0         1.1

***************************************************************************/

#include "emu.h"
#include "includes/coco12.h"

#include "bus/coco/coco_dcmodem.h"
#include "bus/coco/coco_dwsock.h"
#include "bus/coco/coco_fdc.h"
#include "bus/coco/coco_gmc.h"
#include "bus/coco/coco_multi.h"
#include "bus/coco/coco_orch90.h"
#include "bus/coco/coco_pak.h"
#include "bus/coco/coco_rs232.h"
#include "bus/coco/coco_ssc.h"
#include "bus/coco/coco_t4426.h"

#include "cpu/m6809/m6809.h"
#include "imagedev/cassette.h"
#include "sound/volt_reg.h"

#include "softlist.h"
#include "speaker.h"

#include "formats/coco_cas.h"

//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

//-------------------------------------------------
//  ADDRESS_MAP( coco_mem )
//-------------------------------------------------

ADDRESS_MAP_START(coco_state::coco_mem)
ADDRESS_MAP_END



//**************************************************************************
//  INPUT PORTS
//**************************************************************************

//--------------------------------------------------------------------------
//  INPUT_PORTS( coco_analog_control )
//
//  Other CoCo controllers were compatible with the Dragon, even if no
//  software is known to make use of them We add them here, based on this
//  compatibility, in case anyone wants to use them in homebrew software
//--------------------------------------------------------------------------

INPUT_PORTS_START( coco_analog_control )
	PORT_START(CTRL_SEL_TAG)  /* Select Controller Type */
	PORT_CONFNAME( 0x0f, 0x01, "Right Controller Port (P1)")            PORT_CHANGED_MEMBER(DEVICE_SELF, coco_state,  joystick_mode_changed, 0 )
	PORT_CONFSETTING(  0x00, "Unconnected" )
	PORT_CONFSETTING(  0x01, "Joystick" )
	PORT_CONFSETTING(  0x02, "The Rat Graphics Mouse" )                 PORT_CONDITION(CTRL_SEL_TAG, 0xf0, NOTEQUALS, 0x20)
	PORT_CONFSETTING(  0x03, "Diecom Light Gun Adaptor" )               PORT_CONDITION(CTRL_SEL_TAG, 0xf0, NOTEQUALS, 0x30)
	PORT_CONFNAME( 0xf0, 0x10, "Left Controller Port (P2)")             PORT_CHANGED_MEMBER(DEVICE_SELF, coco_state,  joystick_mode_changed, 0 )
	PORT_CONFSETTING(  0x00, "Unconnected" )
	PORT_CONFSETTING(  0x10, "Joystick" )
	PORT_CONFSETTING(  0x20, "The Rat Graphics Mouse" )                 PORT_CONDITION(CTRL_SEL_TAG, 0x0f, NOTEQUALS, 0x02)
	PORT_CONFSETTING(  0x30, "Diecom Light Gun Adaptor" )               PORT_CONDITION(CTRL_SEL_TAG, 0x0f, NOTEQUALS, 0x03)

	PORT_START(HIRES_INTF_TAG)
	PORT_CONFNAME( 0x07, 0x00, "Hi-Res Joystick Interfaces" )
	PORT_CONFSETTING(    0x00, "None" )
	PORT_CONFSETTING(    0x01, "Hi-Res in Right Port" )                 PORT_CONDITION(CTRL_SEL_TAG, 0x0f, EQUALS, 0x01)
	PORT_CONFSETTING(    0x02, "Hi-Res CoCoMax 3 Style in Right Port" ) PORT_CONDITION(CTRL_SEL_TAG, 0x0f, EQUALS, 0x01)
	PORT_CONFSETTING(    0x03, "Hi-Res in Left Port" )                  PORT_CONDITION(CTRL_SEL_TAG, 0xf0, EQUALS, 0x10)
	PORT_CONFSETTING(    0x04, "Hi-Res CoCoMax 3 Style in Left Port" )  PORT_CONDITION(CTRL_SEL_TAG, 0xf0, EQUALS, 0x10)
INPUT_PORTS_END



//-------------------------------------------------
//  INPUT_PORTS( coco_joystick )
//-------------------------------------------------

INPUT_PORTS_START( coco_joystick )
	PORT_START(JOYSTICK_RX_TAG)
	PORT_BIT( 0xff, 0x80,  IPT_AD_STICK_X) PORT_SENSITIVITY(JOYSTICK_SENSITIVITY) PORT_KEYDELTA(JOYSTICK_DELTA) PORT_MINMAX(0x00,0xFF) PORT_CODE_DEC(KEYCODE_4_PAD) PORT_CODE_INC(KEYCODE_6_PAD) PORT_CODE_DEC(JOYCODE_X_LEFT_SWITCH) PORT_CODE_INC(JOYCODE_X_RIGHT_SWITCH) PORT_PLAYER(1) PORT_CONDITION(CTRL_SEL_TAG, 0x0f, EQUALS, 0x01)
	PORT_START(JOYSTICK_RY_TAG)
	PORT_BIT( 0xff, 0x80,  IPT_AD_STICK_Y) PORT_SENSITIVITY(JOYSTICK_SENSITIVITY) PORT_KEYDELTA(JOYSTICK_DELTA) PORT_MINMAX(0x00,0xFF) PORT_CODE_DEC(KEYCODE_8_PAD) PORT_CODE_INC(KEYCODE_2_PAD) PORT_CODE_DEC(JOYCODE_Y_UP_SWITCH)   PORT_CODE_INC(JOYCODE_Y_DOWN_SWITCH)  PORT_PLAYER(1) PORT_CONDITION(CTRL_SEL_TAG, 0x0f, EQUALS, 0x01)
	PORT_START(JOYSTICK_LX_TAG)
	PORT_BIT( 0xff, 0x80,  IPT_AD_STICK_X) PORT_SENSITIVITY(JOYSTICK_SENSITIVITY) PORT_KEYDELTA(JOYSTICK_DELTA) PORT_MINMAX(0x00,0xFF) PORT_CODE_DEC(KEYCODE_4_PAD) PORT_CODE_INC(KEYCODE_6_PAD) PORT_CODE_DEC(JOYCODE_X_LEFT_SWITCH) PORT_CODE_INC(JOYCODE_X_RIGHT_SWITCH) PORT_PLAYER(2) PORT_CONDITION(CTRL_SEL_TAG, 0xf0, EQUALS, 0x10)
	PORT_START(JOYSTICK_LY_TAG)
	PORT_BIT( 0xff, 0x80,  IPT_AD_STICK_Y) PORT_SENSITIVITY(JOYSTICK_SENSITIVITY) PORT_KEYDELTA(JOYSTICK_DELTA) PORT_MINMAX(0x00,0xFF) PORT_CODE_DEC(KEYCODE_8_PAD) PORT_CODE_INC(KEYCODE_2_PAD) PORT_CODE_DEC(JOYCODE_Y_UP_SWITCH)   PORT_CODE_INC(JOYCODE_Y_DOWN_SWITCH)  PORT_PLAYER(2) PORT_CONDITION(CTRL_SEL_TAG, 0xf0, EQUALS, 0x10)
	PORT_START(JOYSTICK_BUTTONS_TAG)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Right Button") PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_CODE(KEYCODE_0_PAD) PORT_CODE(JOYCODE_BUTTON1) PORT_CODE(MOUSECODE_BUTTON1) PORT_PLAYER(1) PORT_CONDITION(CTRL_SEL_TAG, 0x0f, EQUALS, 0x01)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Left Button")  PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_CODE(KEYCODE_0_PAD) PORT_CODE(JOYCODE_BUTTON1) PORT_CODE(MOUSECODE_BUTTON1) PORT_PLAYER(2) PORT_CONDITION(CTRL_SEL_TAG, 0xf0, EQUALS, 0x10)
INPUT_PORTS_END



//-------------------------------------------------
//  INPUT_PORTS( coco_beckerport )
//-------------------------------------------------

INPUT_PORTS_START( coco_beckerport )
	PORT_START(BECKERPORT_TAG)
	PORT_CONFNAME( 0x01, 0x01, "Becker Port" )
	PORT_CONFSETTING(    0x00, DEF_STR( Off ))
	PORT_CONFSETTING(    0x01, DEF_STR( On ))
INPUT_PORTS_END

//-------------------------------------------------
//  INPUT_PORTS( coco_rtc )
//-------------------------------------------------

INPUT_PORTS_START( coco_rtc )
	PORT_START("real_time_clock")
	PORT_CONFNAME( 0x03, 0x00, "Real Time Clock" )
	PORT_CONFSETTING(    0x00, "Disto" )
	PORT_CONFSETTING(    0x01, "Cloud-9" )
INPUT_PORTS_END



//-------------------------------------------------
//  INPUT_PORTS( coco_keyboard )
//-------------------------------------------------
//
//  CoCo keyboard
//
//         PB0 PB1 PB2 PB3 PB4 PB5 PB6 PB7
//    PA6: Ent Clr Brk N/c N/c N/c N/c Shift
//    PA5: 8   9   :   ;   ,   -   .   /
//    PA4: 0   1   2   3   4   5   6   7
//    PA3: X   Y   Z   Up  Dwn Lft Rgt Space
//    PA2: P   Q   R   S   T   U   V   W
//    PA1: H   I   J   K   L   M   N   O
//    PA0: @   A   B   C   D   E   F   G
//-------------------------------------------------

static INPUT_PORTS_START( coco_keyboard )
	PORT_START("row0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('@')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')

	PORT_START("row1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')

	PORT_START("row2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')

	PORT_START("row3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_NAME("UP") PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP), '^')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_NAME("DOWN") PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN), 10)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_NAME("LEFT") PORT_CODE(KEYCODE_LEFT) PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(UCHAR_MAMEKEY(LEFT), 8)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_NAME("RIGHT") PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT), 9)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_NAME("SPACE") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')

	PORT_START("row4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('\"')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')

	PORT_START("row5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_CODE(KEYCODE_MINUS) PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')

	PORT_START("row6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_NAME("ENTER") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_NAME("CLEAR") PORT_CODE(KEYCODE_HOME) PORT_CHAR(12)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_NAME("BREAK") PORT_CODE(KEYCODE_END) PORT_CODE(KEYCODE_ESC) PORT_CHAR(27)
	PORT_BIT(0x78, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
INPUT_PORTS_END



//-------------------------------------------------
//  INPUT_PORTS( coco )
//-------------------------------------------------

static INPUT_PORTS_START( coco )
	PORT_INCLUDE( coco_keyboard )
	PORT_INCLUDE( coco_joystick )
	PORT_INCLUDE( coco_analog_control )
	PORT_INCLUDE( coco_rtc )
	PORT_INCLUDE( coco_beckerport )
INPUT_PORTS_END


//-------------------------------------------------
//  INPUT_PORTS( cp400c2_keyboard )
//-------------------------------------------------
//
//  CP400 Color II keyboard
//
//         PB0   PB1   PB2   PB3   PB4   PB5   PB6   PB7
//    PA6: Enter Clear Break Ctrl  PA1   PA2   PA3   Shift
//    PA5: 8     9     :     ;     ,     -     .     /
//    PA4: 0     1     2     3     4     5     6     7
//    PA3: X     Y     Z     Up    Down  Left  Right Space
//    PA2: P     Q     R     S     T     U     V     W
//    PA1: H     I     J     K     L     M     N     O
//    PA0: @     A     B     C     D     E     F     G
//-------------------------------------------------

static INPUT_PORTS_START( cp400c2_keyboard )
	PORT_START("row0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('@')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')

	PORT_START("row1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')

	PORT_START("row2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')

	PORT_START("row3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_NAME("UP") PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP), '^')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_NAME("DOWN") PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN), 10)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_NAME("LEFT") PORT_CODE(KEYCODE_LEFT) PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(UCHAR_MAMEKEY(LEFT), 8)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_NAME("RIGHT") PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT), 9)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_NAME("SPACE") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')

	PORT_START("row4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('\"')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')

	PORT_START("row5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_CODE(KEYCODE_MINUS) PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')

	PORT_START("row6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_NAME("ENTER") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_NAME("CLEAR") PORT_CODE(KEYCODE_HOME) PORT_CHAR(12)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_NAME("BREAK") PORT_CODE(KEYCODE_END) PORT_CODE(KEYCODE_ESC) PORT_CHAR(27)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_NAME("CTRL") PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_NAME("PA1") PORT_CODE(KEYCODE_LALT)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_NAME("PA2") PORT_CODE(KEYCODE_RALT)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_NAME("PA3") PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, nullptr) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
INPUT_PORTS_END


//-------------------------------------------------
//  INPUT_PORTS( cp400c2 )
//-------------------------------------------------

static INPUT_PORTS_START( cp400c2 )
	PORT_INCLUDE( cp400c2_keyboard )
	PORT_INCLUDE( coco_joystick )
	PORT_INCLUDE( coco_analog_control )
	PORT_INCLUDE( coco_rtc )
	PORT_INCLUDE( coco_beckerport )
INPUT_PORTS_END


//**************************************************************************
//  MACHINE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  SLOT_INTERFACE_START(coco_cart)
//-------------------------------------------------

SLOT_INTERFACE_START( coco_cart )
	SLOT_INTERFACE("fdc", COCO_FDC)
	SLOT_INTERFACE("fdcv11", COCO_FDC_V11)
	SLOT_INTERFACE("cc3hdb1", COCO3_HDB1)
	SLOT_INTERFACE("cp450_fdc", CP450_FDC)
	SLOT_INTERFACE("cd6809_fdc", CD6809_FDC)
	SLOT_INTERFACE("rs232", COCO_RS232)
	SLOT_INTERFACE("dcmodem", COCO_DCMODEM)
	SLOT_INTERFACE("orch90", COCO_ORCH90)
	SLOT_INTERFACE("ssc", COCO_SSC)                 MCFG_SLOT_OPTION_CLOCK("ssc", DERIVED_CLOCK(1, 1))
	SLOT_INTERFACE("games_master", COCO_PAK_GMC)
	SLOT_INTERFACE("banked_16k", COCO_PAK_BANKED)
	SLOT_INTERFACE("pak", COCO_PAK)
	SLOT_INTERFACE("multi", COCO_MULTIPAK)          MCFG_SLOT_OPTION_CLOCK("multi", DERIVED_CLOCK(1, 1))
SLOT_INTERFACE_END

//-------------------------------------------------
//  SLOT_INTERFACE_START(t4426_cart)
//-------------------------------------------------

SLOT_INTERFACE_START( t4426_cart )
	SLOT_INTERFACE("t4426", COCO_T4426)
SLOT_INTERFACE_END

//-------------------------------------------------
//  MACHINE_CONFIG_START( coco_sound )
//-------------------------------------------------

MACHINE_CONFIG_START(coco_state::coco_sound)
	MCFG_SPEAKER_STANDARD_MONO("speaker")

	// 6-bit D/A: R10-15 = 10K, 20K, 40.2K, 80.6K, 162K, 324K (according to parts list); output also controls joysticks
	MCFG_SOUND_ADD("dac", DAC_6BIT_BINARY_WEIGHTED, 0) MCFG_SOUND_ROUTE(ALL_OUTPUTS, "speaker", 0.125)
	MCFG_DEVICE_ADD("vref", VOLTAGE_REGULATOR, 0) MCFG_VOLTAGE_REGULATOR_OUTPUT(5.0)
	MCFG_SOUND_ROUTE_EX(0, "dac", 1.0, DAC_VREF_POS_INPUT) MCFG_SOUND_ROUTE_EX(0, "dac", -1.0, DAC_VREF_NEG_INPUT)

	// Single-bit sound: R22 = 10K
	MCFG_SOUND_ADD("sbs", DAC_1BIT, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "speaker", 0.125)

	MCFG_SOUND_WAVE_ADD(WAVE_TAG, "cassette")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "speaker", 0.25)
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG ( coco_floating )
//-------------------------------------------------

ADDRESS_MAP_START(coco_state::coco_floating_map)
	AM_RANGE(0x0000, 0xFFFF) AM_READ(floating_bus_read)
ADDRESS_MAP_END


MACHINE_CONFIG_START(coco_state::coco_floating)
	MCFG_DEVICE_ADD(FLOATING_TAG, ADDRESS_MAP_BANK, 0)
	MCFG_DEVICE_PROGRAM_MAP(coco_floating_map)
	MCFG_ADDRESS_MAP_BANK_ENDIANNESS(ENDIANNESS_BIG)
	MCFG_ADDRESS_MAP_BANK_DATA_WIDTH(8)
	MCFG_ADDRESS_MAP_BANK_ADDR_WIDTH(16)
MACHINE_CONFIG_END


//-------------------------------------------------
//  DEVICE_INPUT_DEFAULTS_START( printer )
//-------------------------------------------------

static DEVICE_INPUT_DEFAULTS_START( printer )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_600 )
	DEVICE_INPUT_DEFAULTS( "RS232_STARTBITS", 0xff, RS232_STARTBITS_1 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_8 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_NONE )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
DEVICE_INPUT_DEFAULTS_END

//-------------------------------------------------
//  MACHINE_CONFIG
//-------------------------------------------------

MACHINE_CONFIG_START(coco12_state::coco)
	MCFG_DEVICE_MODIFY(":")
	MCFG_DEVICE_CLOCK(XTAL(14'318'181) / 16)

	// basic machine hardware
	MCFG_CPU_ADD(MAINCPU_TAG, MC6809E, DERIVED_CLOCK(1, 1))
	MCFG_CPU_PROGRAM_MAP(coco_mem)
	MCFG_CPU_DISASSEMBLE_OVERRIDE(coco_state, dasm_override)

	// devices
	MCFG_DEVICE_ADD(PIA0_TAG, PIA6821, 0)
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(coco_state, pia0_pa_w))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(coco_state, pia0_pb_w))
	MCFG_PIA_CA2_HANDLER(WRITELINE(coco_state, pia0_ca2_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(coco_state, pia0_cb2_w))
	MCFG_PIA_IRQA_HANDLER(WRITELINE(coco_state, pia0_irq_a))
	MCFG_PIA_IRQB_HANDLER(WRITELINE(coco_state, pia0_irq_b))

	MCFG_DEVICE_ADD(PIA1_TAG, PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(READ8(coco_state, pia1_pa_r))
	MCFG_PIA_READPB_HANDLER(READ8(coco_state, pia1_pb_r))
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(coco_state, pia1_pa_w))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(coco_state, pia1_pb_w))
	MCFG_PIA_CA2_HANDLER(WRITELINE(coco_state, pia1_ca2_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(coco_state, pia1_cb2_w))
	MCFG_PIA_IRQA_HANDLER(WRITELINE(coco_state, pia1_firq_a))
	MCFG_PIA_IRQB_HANDLER(WRITELINE(coco_state, pia1_firq_b))

	MCFG_SAM6883_ADD(SAM_TAG, XTAL(14'318'181), MAINCPU_TAG, AS_PROGRAM)
	MCFG_SAM6883_RES_CALLBACK(READ8(coco12_state, sam_read))

	// Becker Port device
	MCFG_DEVICE_ADD(DWSOCK_TAG, COCO_DWSOCK, 0)

	MCFG_CASSETTE_ADD("cassette")
	MCFG_CASSETTE_FORMATS(coco_cassette_formats)
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_PLAY | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_MUTED)

	MCFG_RS232_PORT_ADD(RS232_TAG, default_rs232_devices, "printer")
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE(PIA1_TAG, pia6821_device, ca1_w))
	MCFG_DEVICE_CARD_DEVICE_INPUT_DEFAULTS("printer", printer)

	MCFG_COCO_CARTRIDGE_ADD(CARTRIDGE_TAG, coco_cart, "pak")
	MCFG_COCO_CARTRIDGE_CART_CB(WRITELINE(coco_state, cart_w))
	MCFG_COCO_CARTRIDGE_NMI_CB(INPUTLINE(MAINCPU_TAG, INPUT_LINE_NMI))
	MCFG_COCO_CARTRIDGE_HALT_CB(INPUTLINE(MAINCPU_TAG, INPUT_LINE_HALT))

	// video hardware
	MCFG_SCREEN_MC6847_NTSC_ADD(SCREEN_TAG, VDG_TAG)

	MCFG_DEVICE_ADD(VDG_TAG, MC6847_NTSC, XTAL(14'318'181) / 4) // VClk output from MC6883
	MCFG_MC6847_HSYNC_CALLBACK(WRITELINE(coco12_state, horizontal_sync))
	MCFG_MC6847_FSYNC_CALLBACK(WRITELINE(coco12_state, field_sync))
	MCFG_MC6847_INPUT_CALLBACK(DEVREAD8(SAM_TAG, sam6883_device, display_read))

	// sound hardware
	coco_sound(config);

	// internal ram
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("64K")
	MCFG_RAM_EXTRA_OPTIONS("4K,16K,32K")

	// floating space
	coco_floating(config);

	// software lists
	MCFG_SOFTWARE_LIST_ADD("coco_cart_list", "coco_cart")
	MCFG_SOFTWARE_LIST_FILTER("coco_cart_list", "COCO")
	MCFG_SOFTWARE_LIST_COMPATIBLE_ADD("dragon_cart_list", "dragon_cart")
MACHINE_CONFIG_END

MACHINE_CONFIG_START(coco12_state::cocoe)
	coco(config);
	MCFG_COCO_CARTRIDGE_REMOVE(CARTRIDGE_TAG)
	MCFG_COCO_CARTRIDGE_ADD(CARTRIDGE_TAG, coco_cart, "fdc")
	MCFG_COCO_CARTRIDGE_CART_CB(WRITELINE(coco_state, cart_w))
	MCFG_COCO_CARTRIDGE_NMI_CB(INPUTLINE(MAINCPU_TAG, INPUT_LINE_NMI))
	MCFG_COCO_CARTRIDGE_HALT_CB(INPUTLINE(MAINCPU_TAG, INPUT_LINE_HALT))
	MCFG_COCO_VHD_ADD(VHD0_TAG)
	MCFG_COCO_VHD_ADD(VHD1_TAG)
MACHINE_CONFIG_END

MACHINE_CONFIG_START(coco12_state::coco2)
	coco(config);
	MCFG_COCO_CARTRIDGE_REMOVE(CARTRIDGE_TAG)
	MCFG_COCO_CARTRIDGE_ADD(CARTRIDGE_TAG, coco_cart, "fdcv11")
	MCFG_COCO_CARTRIDGE_CART_CB(WRITELINE(coco_state, cart_w))
	MCFG_COCO_CARTRIDGE_NMI_CB(INPUTLINE(MAINCPU_TAG, INPUT_LINE_NMI))
	MCFG_COCO_CARTRIDGE_HALT_CB(INPUTLINE(MAINCPU_TAG, INPUT_LINE_HALT))
	MCFG_COCO_VHD_ADD(VHD0_TAG)
	MCFG_COCO_VHD_ADD(VHD1_TAG)
MACHINE_CONFIG_END

MACHINE_CONFIG_START(coco12_state::coco2b)
	coco2(config);
	MCFG_DEVICE_REMOVE(VDG_TAG)
	MCFG_DEVICE_ADD(VDG_TAG, MC6847T1_NTSC, XTAL(14'318'181) / 4)
	MCFG_MC6847_HSYNC_CALLBACK(WRITELINE(coco12_state, horizontal_sync))
	MCFG_MC6847_FSYNC_CALLBACK(WRITELINE(coco12_state, field_sync))
	MCFG_MC6847_INPUT_CALLBACK(DEVREAD8(SAM_TAG, sam6883_device, display_read))
MACHINE_CONFIG_END

MACHINE_CONFIG_START(coco12_state::cp400)
	coco(config);
	MCFG_COCO_CARTRIDGE_REMOVE(CARTRIDGE_TAG)
	MCFG_COCO_CARTRIDGE_ADD(CARTRIDGE_TAG, coco_cart, "cp450_fdc")
	MCFG_COCO_CARTRIDGE_CART_CB(WRITELINE(coco_state, cart_w))
	MCFG_COCO_CARTRIDGE_NMI_CB(INPUTLINE(MAINCPU_TAG, INPUT_LINE_NMI))
	MCFG_COCO_CARTRIDGE_HALT_CB(INPUTLINE(MAINCPU_TAG, INPUT_LINE_HALT))
MACHINE_CONFIG_END

MACHINE_CONFIG_START(coco12_state::t4426)
	coco2(config);
	MCFG_COCO_CARTRIDGE_REMOVE(CARTRIDGE_TAG)
	MCFG_COCO_CARTRIDGE_ADD(CARTRIDGE_TAG, t4426_cart, "t4426")
	MCFG_COCO_CARTRIDGE_CART_CB(WRITELINE(coco_state, cart_w))
	MCFG_COCO_CARTRIDGE_NMI_CB(INPUTLINE(MAINCPU_TAG, INPUT_LINE_NMI))
	MCFG_COCO_CARTRIDGE_HALT_CB(INPUTLINE(MAINCPU_TAG, INPUT_LINE_HALT))
	MCFG_SLOT_FIXED(true) // This cart is fixed so no way to change it
MACHINE_CONFIG_END

MACHINE_CONFIG_START(coco12_state::cd6809)
	coco(config);
	MCFG_COCO_CARTRIDGE_REMOVE(CARTRIDGE_TAG)
	MCFG_COCO_CARTRIDGE_ADD(CARTRIDGE_TAG, coco_cart, "cd6809_fdc")
	MCFG_COCO_CARTRIDGE_CART_CB(WRITELINE(coco_state, cart_w))
	MCFG_COCO_CARTRIDGE_NMI_CB(INPUTLINE(MAINCPU_TAG, INPUT_LINE_NMI))
	MCFG_COCO_CARTRIDGE_HALT_CB(INPUTLINE(MAINCPU_TAG, INPUT_LINE_HALT))
MACHINE_CONFIG_END

//**************************************************************************
//  ROMS
//**************************************************************************

ROM_START(coco)
	ROM_REGION(0x8000,MAINCPU_TAG,0)
	ROM_LOAD("bas10.rom",   0x2000, 0x2000, CRC(00b50aaa) SHA1(1f08455cd48ce6a06132aea15c4778f264e19539))
ROM_END

ROM_START(cocoe)
	ROM_REGION(0x8000,MAINCPU_TAG,0)
	ROM_LOAD("bas11.rom",   0x2000, 0x2000, CRC(6270955a) SHA1(cecb7c24ff1e0ab5836e4a7a8eb1b8e01f1fded3))
	ROM_LOAD("extbas10.rom",    0x0000, 0x2000, CRC(6111a086) SHA1(8aa58f2eb3e8bcfd5470e3e35e2b359e9a72848e))
ROM_END

ROM_START(coco2)
	ROM_REGION(0x8000,MAINCPU_TAG,0)
	ROM_LOAD("bas12.rom",   0x2000, 0x2000, CRC(54368805) SHA1(0f14dc46c647510eb0b7bd3f53e33da07907d04f))
	ROM_LOAD("extbas11.rom",    0x0000, 0x2000, CRC(a82a6254) SHA1(ad927fb4f30746d820cb8b860ebb585e7f095dea))
ROM_END

ROM_START(coco2b)
	ROM_REGION(0x8000,MAINCPU_TAG,0)
	ROM_LOAD("bas13.rom",   0x2000, 0x2000, CRC(d8f4d15e) SHA1(28b92bebe35fa4f026a084416d6ea3b1552b63d3))
	ROM_LOAD("extbas11.rom",    0x0000, 0x2000, CRC(a82a6254) SHA1(ad927fb4f30746d820cb8b860ebb585e7f095dea))
ROM_END

ROM_START(cp400)
	ROM_REGION(0x8000,MAINCPU_TAG,0)
	ROM_LOAD("cp400bas.rom",  0x0000, 0x4000, CRC(878396a5) SHA1(292c545da3c77978e043b00a3dbc317201d18c3b))
ROM_END

ROM_START(cp400c2)
	ROM_REGION(0x8000,MAINCPU_TAG,0)
	ROM_LOAD("cp400bas.rom",  0x0000, 0x4000, CRC(878396a5) SHA1(292c545da3c77978e043b00a3dbc317201d18c3b))
ROM_END

ROM_START(mx1600)
	ROM_REGION(0x8000,MAINCPU_TAG,0)
	ROM_LOAD("mx1600bas.rom",    0x2000, 0x2000, CRC(d918156e) SHA1(70a464edf3a654ed4ffe687e6dee4f0d2acc758b))
	ROM_LOAD("mx1600extbas.rom", 0x0000, 0x2000, CRC(322a3d58) SHA1(9079a477c3f22e46cebb1e68b61df5bd607c71a4))
ROM_END

ROM_START(t4426)
	ROM_REGION(0x8000,MAINCPU_TAG,0)
	ROM_LOAD("SOFT4426-U13-1.2.bin", 0x2000, 0x2000, CRC(3c1af94a) SHA1(1dc57b3e4a6ef6a743ca21d8f111a74b1ea9d54e))
	ROM_LOAD("SOFT4426-U14-1.2.bin", 0x0000, 0x2000, CRC(e031d076) SHA1(7275f1e3f165ff6a4657e4e5e24cb8b817239f54))
ROM_END

ROM_START(lzcolor64)
	ROM_REGION(0x8000,MAINCPU_TAG,0)
	ROM_LOAD("color_basic.ci24", 0x2000, 0x2000, CRC(b0717d71) SHA1(ad1beef9d6f095ada69f91d0b8ad75985172d86f))
	ROM_LOAD("extendido.ci23",   0x0000, 0x2000, CRC(d1b1560d) SHA1(7252de9df405ade453282e992eb1f1910adc8e50))
ROM_END

ROM_START(cd6809)
	ROM_REGION(0x8000,MAINCPU_TAG,0)
		ROM_DEFAULT_BIOS("84")

		ROM_SYSTEM_BIOS( 0, "83", "1983" )
	ROMX_LOAD("cd6809bas83.rom",    0x2000, 0x2000, CRC(f8e64142) SHA1(c0fd689119e2619ec226a2d67aeeb32070c14e38), ROM_BIOS(1))
	ROMX_LOAD("cd6809extbas83.rom", 0x0000, 0x2000, CRC(e5d5aa15) SHA1(0cd4a3d9e4af1d0176964e35e3d15a9fa0e68ac4), ROM_BIOS(1))

		ROM_SYSTEM_BIOS( 1, "84", "1984" )
	ROMX_LOAD("cd6809bas84.rom",    0x2000, 0x2000, CRC(8a9971da) SHA1(5cb5f1ffc983a85ba92af68b1d571b270f6db559), ROM_BIOS(2))
	ROMX_LOAD("cd6809extbas84.rom", 0x0000, 0x2000, CRC(8dc853e2) SHA1(d572ce4497c115af53d2b0feeb52d3c7a7fec175), ROM_BIOS(2))
ROM_END

//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//     YEAR     NAME        PARENT  COMPAT  MACHINE  INPUT     STATE         INIT  COMPANY                         FULLNAME                               FLAGS
COMP(  1980,    coco,       0,      0,      coco,    coco,     coco12_state, 0,    "Tandy Radio Shack",            "Color Computer",                      0 )
COMP(  1981,    cocoe,      coco,   0,      cocoe,   coco,     coco12_state, 0,    "Tandy Radio Shack",            "Color Computer (Extended BASIC 1.0)", 0 )
COMP(  1983,    coco2,      coco,   0,      coco2,   coco,     coco12_state, 0,    "Tandy Radio Shack",            "Color Computer 2",                    0 )
COMP(  1985?,   coco2b,     coco,   0,      coco2b,  coco,     coco12_state, 0,    "Tandy Radio Shack",            "Color Computer 2B",                   0 )
COMP(  1983,    cp400,      coco,   0,      cp400,   coco,     coco12_state, 0,    "Prológica",                    "CP400",                               0 )
COMP(  1985,    cp400c2,    coco,   0,      cp400,   cp400c2,  coco12_state, 0,    "Prológica",                    "CP400 Color II",                      0 )
COMP(  1983,    lzcolor64,  coco,   0,      coco,    coco,     coco12_state, 0,    "Novo Tempo / LZ Equipamentos", "Color64",                             0 )
COMP(  1983,    cd6809,     coco,   0,      cd6809,  coco,     coco12_state, 0,    "Codimex",                      "CD-6809",              0 )
COMP(  1984,    mx1600,     coco,   0,      coco,    coco,     coco12_state, 0,    "Dynacom",                      "MX-1600",                             0 )
COMP(  1986,    t4426,      coco,   0,      t4426,   coco,     coco12_state, 0,    "Terco AB",                     "Terco 4426 CNC Programming station",  0 )
