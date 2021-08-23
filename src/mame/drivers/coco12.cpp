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

#include "bus/coco/cococart.h"
#include "bus/coco/coco_t4426.h"

#include "cpu/m6809/m6809.h"
#include "cpu/m6809/hd6309.h"
#include "imagedev/cassette.h"

#include "softlist.h"
#include "speaker.h"

#include "formats/coco_cas.h"

//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void coco12_state::coco_mem(address_map &map)
{
	map(0x0000, 0xffff).rw(m_sam, FUNC(sam6883_device::read), FUNC(sam6883_device::write));
}


void coco12_state::coco_ram(address_map &map)
{
	// mapped by configure_sam
}

void coco12_state::coco_rom0(address_map &map)
{
	// $8000-$9FFF
	map(0x0000, 0x1fff).rom().region(MAINCPU_TAG, 0x0000).nopw();
}

void coco12_state::coco_rom1(address_map &map)
{
	// $A000-$BFFF
	map(0x0000, 0x1fff).rom().region(MAINCPU_TAG, 0x2000).nopw();
}

void coco12_state::coco_rom2(address_map &map)
{
	// $C000-$FEFF
	map(0x0000, 0x3eff).rw(m_cococart, FUNC(cococart_slot_device::cts_read), FUNC(cococart_slot_device::cts_write));
}

void coco12_state::coco_io0(address_map &map)
{
	// $FF00-$FF1F
	map(0x00, 0x03).mirror(0x1c).rw(PIA0_TAG, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
}

void coco12_state::coco_io1(address_map &map)
{
	// $FF20-$FF3F
	map(0x00, 0x03).mirror(0x1c).r(PIA1_TAG, FUNC(pia6821_device::read)).w(FUNC(coco12_state::ff20_write));
}

void coco12_state::coco_io2(address_map &map)
{
	// $FF40-$FF5F
	map(0x00, 0x1f).rw(FUNC(coco12_state::ff40_read), FUNC(coco12_state::ff40_write));
}

void coco12_state::coco_ff60(address_map &map)
{
	// $FF60-$FFDF
	map(0x00, 0x7f).rw(FUNC(coco12_state::ff60_read), FUNC(coco12_state::ff60_write));
}


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
	PORT_BIT( 0x3ff, 0x140,  IPT_AD_STICK_X) PORT_NAME("Right Joystick X") PORT_SENSITIVITY(JOYSTICK_SENSITIVITY) PORT_KEYDELTA(JOYSTICK_DELTA) PORT_MINMAX(0x00,0x280) PORT_CODE_DEC(KEYCODE_4_PAD) PORT_CODE_INC(KEYCODE_6_PAD) PORT_CODE_DEC(JOYCODE_X_LEFT_SWITCH) PORT_CODE_INC(JOYCODE_X_RIGHT_SWITCH) PORT_PLAYER(1) PORT_CONDITION(CTRL_SEL_TAG, 0x0f, EQUALS, 0x01)
	PORT_START(JOYSTICK_RY_TAG)
	PORT_BIT( 0x3ff, 0x140,  IPT_AD_STICK_Y) PORT_NAME("Right Joystick Y") PORT_SENSITIVITY(JOYSTICK_SENSITIVITY) PORT_KEYDELTA(JOYSTICK_DELTA) PORT_MINMAX(0x00,0x280) PORT_CODE_DEC(KEYCODE_8_PAD) PORT_CODE_INC(KEYCODE_2_PAD) PORT_CODE_DEC(JOYCODE_Y_UP_SWITCH)   PORT_CODE_INC(JOYCODE_Y_DOWN_SWITCH)  PORT_PLAYER(1) PORT_CONDITION(CTRL_SEL_TAG, 0x0f, EQUALS, 0x01)
	PORT_START(JOYSTICK_LX_TAG)
	PORT_BIT( 0x3ff, 0x140,  IPT_AD_STICK_X) PORT_NAME("Left Joystick X") PORT_SENSITIVITY(JOYSTICK_SENSITIVITY) PORT_KEYDELTA(JOYSTICK_DELTA) PORT_MINMAX(0x00,0x280) PORT_CODE_DEC(KEYCODE_4_PAD) PORT_CODE_INC(KEYCODE_6_PAD) PORT_CODE_DEC(JOYCODE_X_LEFT_SWITCH) PORT_CODE_INC(JOYCODE_X_RIGHT_SWITCH) PORT_PLAYER(2) PORT_CONDITION(CTRL_SEL_TAG, 0xf0, EQUALS, 0x10)
	PORT_START(JOYSTICK_LY_TAG)
	PORT_BIT( 0x3ff, 0x140,  IPT_AD_STICK_Y) PORT_NAME("Left Joystick Y") PORT_SENSITIVITY(JOYSTICK_SENSITIVITY) PORT_KEYDELTA(JOYSTICK_DELTA) PORT_MINMAX(0x00,0x280) PORT_CODE_DEC(KEYCODE_8_PAD) PORT_CODE_INC(KEYCODE_2_PAD) PORT_CODE_DEC(JOYCODE_Y_UP_SWITCH)   PORT_CODE_INC(JOYCODE_Y_DOWN_SWITCH)  PORT_PLAYER(2) PORT_CONDITION(CTRL_SEL_TAG, 0xf0, EQUALS, 0x10)
	PORT_START(JOYSTICK_BUTTONS_TAG)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Right Button") PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_0_PAD) PORT_CODE(JOYCODE_BUTTON1) PORT_CODE(MOUSECODE_BUTTON1) PORT_PLAYER(1) PORT_CONDITION(CTRL_SEL_TAG, 0x0f, EQUALS, 0x01)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Left Button")  PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_0_PAD) PORT_CODE(JOYCODE_BUTTON1) PORT_CODE(MOUSECODE_BUTTON1) PORT_PLAYER(2) PORT_CONDITION(CTRL_SEL_TAG, 0xf0, EQUALS, 0x10)
INPUT_PORTS_END



//-------------------------------------------------
//  INPUT_PORTS( coco_beckerport )
//-------------------------------------------------

INPUT_PORTS_START( coco_beckerport )
	PORT_START(BECKERPORT_TAG)
	PORT_CONFNAME( 0x01, 0x00, "Becker Port" )
	PORT_CONFSETTING(    0x00, DEF_STR( Off ))
	PORT_CONFSETTING(    0x01, DEF_STR( On ))
INPUT_PORTS_END

INPUT_PORTS_START( coco_beckerport_dw )
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
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('@')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')

	PORT_START("row1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')

	PORT_START("row2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')

	PORT_START("row3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_NAME("UP") PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_NAME("DOWN") PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN), 10)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_NAME("LEFT") PORT_CODE(KEYCODE_LEFT) PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(UCHAR_MAMEKEY(LEFT), 8)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_NAME("RIGHT") PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT), 9)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_NAME("SPACE") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')

	PORT_START("row4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!') PORT_CHAR('|')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('\"')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#') PORT_CHAR('~')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'') PORT_CHAR('^')

	PORT_START("row5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(') PORT_CHAR('[')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')') PORT_CHAR(']')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_MINUS) PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<') PORT_CHAR('{')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('-') PORT_CHAR('=') PORT_CHAR('_')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>') PORT_CHAR('}')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?') PORT_CHAR('\\')

	PORT_START("row6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_NAME("ENTER") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_NAME("CLEAR") PORT_CODE(KEYCODE_HOME) PORT_CHAR(12, UCHAR_SHIFT_2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_NAME("BREAK") PORT_CODE(KEYCODE_END) PORT_CODE(KEYCODE_ESC) PORT_CHAR(27)
	PORT_BIT(0x78, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
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
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('@')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')

	PORT_START("row1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')

	PORT_START("row2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')

	PORT_START("row3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_NAME("UP") PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP), '^')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_NAME("DOWN") PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN), 10)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_NAME("LEFT") PORT_CODE(KEYCODE_LEFT) PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(UCHAR_MAMEKEY(LEFT), 8)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_NAME("RIGHT") PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT), 9)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_NAME("SPACE") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')

	PORT_START("row4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('\"')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')

	PORT_START("row5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_MINUS) PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')

	PORT_START("row6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_NAME("ENTER") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_NAME("CLEAR") PORT_CODE(KEYCODE_HOME) PORT_CHAR(12)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_NAME("BREAK") PORT_CODE(KEYCODE_END) PORT_CODE(KEYCODE_ESC) PORT_CHAR(27)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_NAME("CTRL") PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_NAME("PA1") PORT_CODE(KEYCODE_LALT)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_NAME("PA2") PORT_CODE(KEYCODE_RALT)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_NAME("PA3") PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
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
//  coco_cart
//-------------------------------------------------

void coco_cart(device_slot_interface &device)
{
	coco_cart_add_basic_devices(device);
	coco_cart_add_fdcs(device);
	coco_cart_add_multi_pak(device);
}


//-------------------------------------------------
//  SLOT_INTERFACE_START(t4426_cart)
//-------------------------------------------------

void t4426_cart(device_slot_interface &device)
{
	device.option_add("t4426", COCO_T4426);
}

//-------------------------------------------------
//  machine_config( coco_sound )
//-------------------------------------------------

void coco_state::coco_sound(machine_config &config)
{
	SPEAKER(config, "speaker").front_center();

	// 6-bit D/A: R10-15 = 10K, 20K, 40.2K, 80.6K, 162K, 324K (according to parts list); output also controls joysticks
	DAC_6BIT_BINARY_WEIGHTED(config, m_dac, 0).add_route(ALL_OUTPUTS, "speaker", 0.125);

	// Single-bit sound: R22 = 10K
	DAC_1BIT(config, "sbs", 0).set_output_range(-1, 1).add_route(ALL_OUTPUTS, "speaker", 0.125);
}


//-------------------------------------------------
//  machine_config ( coco_floating )
//-------------------------------------------------

void coco_state::coco_floating_map(address_map &map)
{
	map(0x0000, 0xFFFF).r(FUNC(coco_state::floating_bus_r));
	map(0x0000, 0xFFFF).nopw(); /* suppress log warnings */
}


void coco_state::coco_floating(machine_config &config)
{
	ADDRESS_MAP_BANK(config, FLOATING_TAG).set_map(&coco_state::coco_floating_map).set_options(ENDIANNESS_BIG, 8, 16);
}


//-------------------------------------------------
//  DEVICE_INPUT_DEFAULTS_START( printer )
//-------------------------------------------------

static DEVICE_INPUT_DEFAULTS_START( rs_printer )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_600 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_8 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_NONE )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
DEVICE_INPUT_DEFAULTS_END

//-------------------------------------------------
//  machine_config
//-------------------------------------------------

void coco12_state::coco(machine_config &config)
{
	this->set_clock(XTAL(14'318'181) / 16);

	// basic machine hardware
	MC6809E(config, m_maincpu, DERIVED_CLOCK(1, 1));
	m_maincpu->set_addrmap(AS_PROGRAM, &coco12_state::coco_mem);
	m_maincpu->set_dasm_override(FUNC(coco_state::dasm_override));

	// devices
	INPUT_MERGER_ANY_HIGH(config, m_irqs).output_handler().set_inputline(m_maincpu, M6809_IRQ_LINE);
	INPUT_MERGER_ANY_HIGH(config, m_firqs).output_handler().set_inputline(m_maincpu, M6809_FIRQ_LINE);

	pia6821_device &pia0(PIA6821(config, PIA0_TAG, 0));
	pia0.writepa_handler().set(FUNC(coco_state::pia0_pa_w));
	pia0.writepb_handler().set(FUNC(coco_state::pia0_pb_w));
	pia0.tspb_handler().set_constant(0xff);
	pia0.ca2_handler().set(FUNC(coco_state::pia0_ca2_w));
	pia0.cb2_handler().set(FUNC(coco_state::pia0_cb2_w));
	pia0.irqa_handler().set(m_irqs, FUNC(input_merger_device::in_w<0>));
	pia0.irqb_handler().set(m_irqs, FUNC(input_merger_device::in_w<1>));

	pia6821_device &pia1(PIA6821(config, PIA1_TAG, 0));
	pia1.readpa_handler().set(FUNC(coco_state::pia1_pa_r));
	pia1.readpb_handler().set(FUNC(coco_state::pia1_pb_r));
	pia1.writepa_handler().set(FUNC(coco_state::pia1_pa_w));
	pia1.writepb_handler().set(FUNC(coco_state::pia1_pb_w));
	pia1.ca2_handler().set(FUNC(coco_state::pia1_ca2_w));
	pia1.cb2_handler().set(FUNC(coco_state::pia1_cb2_w));
	pia1.irqa_handler().set(m_firqs, FUNC(input_merger_device::in_w<0>));
	pia1.irqb_handler().set(m_firqs, FUNC(input_merger_device::in_w<1>));

	SAM6883(config, m_sam, XTAL(14'318'181), m_maincpu);
	m_sam->set_addrmap(0, &coco12_state::coco_ram);
	m_sam->set_addrmap(1, &coco12_state::coco_rom0);
	m_sam->set_addrmap(2, &coco12_state::coco_rom1);
	m_sam->set_addrmap(3, &coco12_state::coco_rom2);
	m_sam->set_addrmap(4, &coco12_state::coco_io0);
	m_sam->set_addrmap(5, &coco12_state::coco_io1);
	m_sam->set_addrmap(6, &coco12_state::coco_io2);
	m_sam->set_addrmap(7, &coco12_state::coco_ff60);

	// Becker Port device
	COCO_DWSOCK(config, DWSOCK_TAG, 0);

	// sound hardware
	coco_sound(config);

	CASSETTE(config, m_cassette);
	m_cassette->set_formats(coco_cassette_formats);
	m_cassette->set_default_state(CASSETTE_PLAY | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_ENABLED);

	rs232_port_device &rs232(RS232_PORT(config, RS232_TAG, default_rs232_devices, "rs_printer"));
	rs232.dcd_handler().set(PIA1_TAG, FUNC(pia6821_device::ca1_w));
	rs232.set_option_device_input_defaults("rs_printer", DEVICE_INPUT_DEFAULTS_NAME(rs_printer));

	cococart_slot_device &cartslot(COCOCART_SLOT(config, CARTRIDGE_TAG, DERIVED_CLOCK(1, 1), coco_cart, "pak"));
	cartslot.cart_callback().set([this] (int state) { cart_w(state != 0); }); // lambda because name is overloaded
	cartslot.nmi_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);
	cartslot.halt_callback().set_inputline(m_maincpu, INPUT_LINE_HALT);

	// video hardware
	SCREEN(config, "screen", SCREEN_TYPE_RASTER);

	MC6847_NTSC(config, m_vdg, XTAL(14'318'181) / 4); // VClk output from MC6883
	m_vdg->set_screen("screen");
	m_vdg->hsync_wr_callback().set(FUNC(coco12_state::horizontal_sync));
	m_vdg->fsync_wr_callback().set(FUNC(coco12_state::field_sync));
	m_vdg->input_callback().set(FUNC(coco12_state::sam_read));

	// internal ram
	RAM(config, m_ram).set_default_size("64K").set_extra_options("4K,16K,32K");

	// floating space
	coco_floating(config);

	// software lists
	SOFTWARE_LIST(config, "coco_cart_list").set_original("coco_cart").set_filter("COCO");
	SOFTWARE_LIST(config, "coco_flop_list").set_original("coco_flop").set_filter("COCO");
	SOFTWARE_LIST(config, "dragon_cart_list").set_compatible("dragon_cart");
}

void coco12_state::cocoh(machine_config &config)
{
	coco(config);
	HD6309E(config.replace(), m_maincpu, DERIVED_CLOCK(1, 1));
	m_maincpu->set_addrmap(AS_PROGRAM, &coco12_state::coco_mem);
	m_ram->set_default_size("64K");
}

void coco12_state::cocoe(machine_config &config)
{
	coco(config);
	cococart_slot_device &cartslot(COCOCART_SLOT(config.replace(), CARTRIDGE_TAG, DERIVED_CLOCK(1, 1), coco_cart, "fdc"));
	cartslot.cart_callback().set([this] (int state) { cart_w(state != 0); }); // lambda because name is overloaded
	cartslot.nmi_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);
	cartslot.halt_callback().set_inputline(m_maincpu, INPUT_LINE_HALT);
	COCO_VHD(config, m_vhd_0, 0, m_maincpu);
	COCO_VHD(config, m_vhd_1, 0, m_maincpu);
}

void coco12_state::cocoeh(machine_config &config)
{
	cocoe(config);
	HD6309E(config.replace(), m_maincpu, DERIVED_CLOCK(1, 1));
	m_maincpu->set_addrmap(AS_PROGRAM, &coco12_state::coco_mem);
	m_ram->set_default_size("64K");
}

void coco12_state::coco2(machine_config &config)
{
	coco(config);
	cococart_slot_device &cartslot(COCOCART_SLOT(config.replace(), CARTRIDGE_TAG, DERIVED_CLOCK(1, 1), coco_cart, "fdcv11"));
	cartslot.cart_callback().set([this] (int state) { cart_w(state != 0); }); // lambda because name is overloaded
	cartslot.nmi_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);
	cartslot.halt_callback().set_inputline(m_maincpu, INPUT_LINE_HALT);
	COCO_VHD(config, m_vhd_0, 0, m_maincpu);
	COCO_VHD(config, m_vhd_1, 0, m_maincpu);
}

void coco12_state::coco2h(machine_config &config)
{
	coco2(config);
	HD6309E(config.replace(), m_maincpu, DERIVED_CLOCK(1, 1));
	m_maincpu->set_addrmap(AS_PROGRAM, &coco12_state::coco_mem);
	m_ram->set_default_size("64K");
}

void coco12_state::coco2b(machine_config &config)
{
	coco2(config);
	MC6847T1_NTSC(config.replace(), m_vdg, XTAL(14'318'181) / 4);
	m_vdg->set_screen(SCREEN_TAG);
	m_vdg->hsync_wr_callback().set(FUNC(coco12_state::horizontal_sync));
	m_vdg->fsync_wr_callback().set(FUNC(coco12_state::field_sync));
	m_vdg->input_callback().set(FUNC(coco12_state::sam_read));
}

void coco12_state::coco2bh(machine_config &config)
{
	coco2b(config);
	HD6309E(config.replace(), m_maincpu, DERIVED_CLOCK(1, 1));
	m_maincpu->set_addrmap(AS_PROGRAM, &coco12_state::coco_mem);
	m_ram->set_default_size("64K");
}

void coco12_state::cp400(machine_config &config)
{
	coco(config);
	cococart_slot_device &cartslot(COCOCART_SLOT(config.replace(), CARTRIDGE_TAG, DERIVED_CLOCK(1, 1), coco_cart, "cp450_fdc"));
	cartslot.cart_callback().set([this] (int state) { cart_w(state != 0); }); // lambda because name is overloaded
	cartslot.nmi_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);
	cartslot.halt_callback().set_inputline(m_maincpu, INPUT_LINE_HALT);
}

void coco12_state::t4426(machine_config &config)
{
	coco2(config);
	cococart_slot_device &cartslot(COCOCART_SLOT(config.replace(), CARTRIDGE_TAG, DERIVED_CLOCK(1, 1), t4426_cart, "t4426"));
	cartslot.cart_callback().set([this] (int state) { cart_w(state != 0); }); // lambda because name is overloaded
	cartslot.nmi_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);
	cartslot.halt_callback().set_inputline(m_maincpu, INPUT_LINE_HALT);
	cartslot.set_fixed(true); // This cart is fixed so no way to change it
}

void coco12_state::cd6809(machine_config &config)
{
	coco(config);
	cococart_slot_device &cartslot(COCOCART_SLOT(config.replace(), CARTRIDGE_TAG, DERIVED_CLOCK(1, 1), coco_cart, "cd6809_fdc"));
	cartslot.cart_callback().set([this] (int state) { cart_w(state != 0); }); // lambda because name is overloaded
	cartslot.nmi_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);
	cartslot.halt_callback().set_inputline(m_maincpu, INPUT_LINE_HALT);
}

//**************************************************************************
//  ROMS
//**************************************************************************

ROM_START(coco)
	ROM_REGION(0x8000,MAINCPU_TAG,0)
	ROM_LOAD("bas10.rom",   0x2000, 0x2000, CRC(00b50aaa) SHA1(1f08455cd48ce6a06132aea15c4778f264e19539))
ROM_END

ROM_START(trsvidtx)
	ROM_REGION(0x8000,MAINCPU_TAG,0)
	ROM_LOAD( "8041716-1.1.u13", 0x2000, 0x0800, BAD_DUMP CRC(7844793c) SHA1(4595f1ff9140974356fa60781caf201e5c464258) )
	ROM_RELOAD(0x2800, 0x0800)
	ROM_RELOAD(0x3000, 0x0800)
	ROM_RELOAD(0x3800, 0x0800)
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
	ROM_LOAD("soft4426-u13-1.2.bin", 0x2000, 0x2000, CRC(3c1af94a) SHA1(1dc57b3e4a6ef6a743ca21d8f111a74b1ea9d54e))
	ROM_LOAD("soft4426-u14-1.2.bin", 0x0000, 0x2000, CRC(e031d076) SHA1(7275f1e3f165ff6a4657e4e5e24cb8b817239f54))
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
	ROMX_LOAD("cd6809bas83.rom",    0x2000, 0x2000, CRC(f8e64142) SHA1(c0fd689119e2619ec226a2d67aeeb32070c14e38), ROM_BIOS(0))
	ROMX_LOAD("cd6809extbas83.rom", 0x0000, 0x2000, CRC(e5d5aa15) SHA1(0cd4a3d9e4af1d0176964e35e3d15a9fa0e68ac4), ROM_BIOS(0))

	ROM_SYSTEM_BIOS( 1, "84", "1984" )
	ROMX_LOAD("cd6809bas84.rom",    0x2000, 0x2000, CRC(8a9971da) SHA1(5cb5f1ffc983a85ba92af68b1d571b270f6db559), ROM_BIOS(1))
	ROMX_LOAD("cd6809extbas84.rom", 0x0000, 0x2000, CRC(8dc853e2) SHA1(d572ce4497c115af53d2b0feeb52d3c7a7fec175), ROM_BIOS(1))
ROM_END

#define rom_cocoh rom_coco
#define rom_cocoeh rom_cocoe
#define rom_coco2h rom_coco2
#define rom_coco2bh rom_coco2b

//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR   NAME       PARENT  COMPAT  MACHINE  INPUT    CLASS         INIT        COMPANY                         FULLNAME                               FLAGS
COMP( 1980,  coco,      0,      0,      coco,    coco,    coco12_state, empty_init, "Tandy Radio Shack",            "Color Computer",                      0 )
COMP( 1981,  trsvidtx,  coco,   0,      coco,    coco,    coco12_state, empty_init, "Tandy Radio Shack",            "Videotex", MACHINE_NOT_WORKING )
COMP( 19??,  cocoh,     coco,   0,      cocoh,   coco,    coco12_state, empty_init, "Tandy Radio Shack",            "Color Computer (HD6309)",             MACHINE_UNOFFICIAL )
COMP( 1981,  cocoe,     coco,   0,      cocoe,   coco,    coco12_state, empty_init, "Tandy Radio Shack",            "Color Computer (Extended BASIC 1.0)", 0 )
COMP( 19??,  cocoeh,    coco,   0,      cocoeh,  coco,    coco12_state, empty_init, "Tandy Radio Shack",            "Color Computer (Extended BASIC 1.0; HD6309)", MACHINE_UNOFFICIAL )
COMP( 1983,  coco2,     coco,   0,      coco2,   coco,    coco12_state, empty_init, "Tandy Radio Shack",            "Color Computer 2",                    0 )
COMP( 19??,  coco2h,    coco,   0,      coco2h,  coco,    coco12_state, empty_init, "Tandy Radio Shack",            "Color Computer 2 (HD6309)",           MACHINE_UNOFFICIAL )
COMP( 1985?, coco2b,    coco,   0,      coco2b,  coco,    coco12_state, empty_init, "Tandy Radio Shack",            "Color Computer 2B",                   0 )
COMP( 19??,  coco2bh,   coco,   0,      coco2bh, coco,    coco12_state, empty_init, "Tandy Radio Shack",            "Color Computer 2B (HD6309)",          MACHINE_UNOFFICIAL )
COMP( 1983,  cp400,     coco,   0,      cp400,   coco,    coco12_state, empty_init, "Prológica",                    "CP400",                               0 )
COMP( 1985,  cp400c2,   coco,   0,      cp400,   cp400c2, coco12_state, empty_init, "Prológica",                    "CP400 Color II",                      0 )
COMP( 1983,  lzcolor64, coco,   0,      coco,    coco,    coco12_state, empty_init, "Novo Tempo / LZ Equipamentos", "Color64",                             0 )
COMP( 1983,  cd6809,    coco,   0,      cd6809,  coco,    coco12_state, empty_init, "Codimex",                      "CD-6809",              0 )
COMP( 1984,  mx1600,    coco,   0,      coco,    coco,    coco12_state, empty_init, "Dynacom",                      "MX-1600",                             0 )
COMP( 1986,  t4426,     coco,   0,      t4426,   coco,    coco12_state, empty_init, "Terco AB",                     "Terco 4426 CNC Programming station",  0 )
