// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    coco12.cpp

    TRS-80 Radio Shack Color Computer 1/2

    Mathis Rosenhauer (original driver)
    Nate Woods (current maintainer)
    Tim Lindner (VHD and other work)

    TRS-80 Deluxe Color Computer

    Prototype Color Computer with added MOS6551 and AY-3-8913

***************************************************************************/

#include "emu.h"
#include "coco12.h"

#include "bus/coco/cococart.h"
#include "bus/coco/coco_t4426.h"

#include "cpu/m6809/m6809.h"
#include "cpu/m6809/hd6309.h"
#include "imagedev/cassette.h"

#include "softlist_dev.h"
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

void deluxecoco_state::deluxecoco_rom2(address_map &map)
{
	// $C000-$FEFF
	map(0x0000, 0x3eff).view(m_rom_view);

	m_rom_view[0](0x0000, 0x3eff).rw(m_cococart, FUNC(cococart_slot_device::cts_read), FUNC(cococart_slot_device::cts_write));
	m_rom_view[1](0x0000, 0x3eff).rom().region(MAINCPU_TAG, 0x4000).nopw();
}

void coco12_state::coco_io0(address_map &map)
{
	// $FF00-$FF1F
	map(0x00, 0x03).mirror(0x1c).rw(m_pia_0, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
}

void coco12_state::coco_io1(address_map &map)
{
	// $FF20-$FF3F
	map(0x00, 0x03).mirror(0x1c).r(m_pia_1, FUNC(pia6821_device::read)).w(FUNC(coco12_state::ff20_write));
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

void coco12_state::ms1600_rom2(address_map &map)
{
	// $C000-$FEFF
	map(0x0000, 0x2fff).rw(m_cococart, FUNC(cococart_slot_device::cts_read), FUNC(cococart_slot_device::cts_write));
	map(0x3000, 0x3eff).rom().region(MAINCPU_TAG, 0x7000).nopw();
}

void deluxecoco_state::deluxecoco_io1(address_map &map)
{
	// $FF20-$FF3F
	map(0x00, 0x03).r(m_pia_1, FUNC(pia6821_device::read)).w(FUNC(coco12_state::ff20_write));
	map(0x10, 0x10).w(FUNC(deluxecoco_state::ff30_write));
	map(0x18, 0x19).w(m_psg, FUNC(ay8913_device::data_address_w));
	map(0x1c, 0x1f).rw(m_acia, FUNC(mos6551_device::read), FUNC(mos6551_device::write));
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
	PORT_BIT( 0x3ff, 0x200,  IPT_AD_STICK_X) PORT_NAME("Right Joystick X") PORT_SENSITIVITY(JOYSTICK_SENSITIVITY) PORT_KEYDELTA(JOYSTICK_DELTA) PORT_MINMAX(0,1023) PORT_CODE_DEC(KEYCODE_4_PAD) PORT_CODE_INC(KEYCODE_6_PAD) PORT_CODE_DEC(JOYCODE_X_LEFT_SWITCH) PORT_CODE_INC(JOYCODE_X_RIGHT_SWITCH) PORT_PLAYER(1) PORT_CONDITION(CTRL_SEL_TAG, 0x0f, EQUALS, 0x01)
	PORT_START(JOYSTICK_RY_TAG)
	PORT_BIT( 0x3ff, 0x200,  IPT_AD_STICK_Y) PORT_NAME("Right Joystick Y") PORT_SENSITIVITY(JOYSTICK_SENSITIVITY) PORT_KEYDELTA(JOYSTICK_DELTA) PORT_MINMAX(0,1023) PORT_CODE_DEC(KEYCODE_8_PAD) PORT_CODE_INC(KEYCODE_2_PAD) PORT_CODE_DEC(JOYCODE_Y_UP_SWITCH)   PORT_CODE_INC(JOYCODE_Y_DOWN_SWITCH)  PORT_PLAYER(1) PORT_CONDITION(CTRL_SEL_TAG, 0x0f, EQUALS, 0x01)
	PORT_START(JOYSTICK_LX_TAG)
	PORT_BIT( 0x3ff, 0x200,  IPT_AD_STICK_X) PORT_NAME("Left Joystick X") PORT_SENSITIVITY(JOYSTICK_SENSITIVITY) PORT_KEYDELTA(JOYSTICK_DELTA) PORT_MINMAX(0,1023) PORT_CODE_DEC(KEYCODE_4_PAD) PORT_CODE_INC(KEYCODE_6_PAD) PORT_CODE_DEC(JOYCODE_X_LEFT_SWITCH) PORT_CODE_INC(JOYCODE_X_RIGHT_SWITCH) PORT_PLAYER(2) PORT_CONDITION(CTRL_SEL_TAG, 0xf0, EQUALS, 0x10)
	PORT_START(JOYSTICK_LY_TAG)
	PORT_BIT( 0x3ff, 0x200,  IPT_AD_STICK_Y) PORT_NAME("Left Joystick Y") PORT_SENSITIVITY(JOYSTICK_SENSITIVITY) PORT_KEYDELTA(JOYSTICK_DELTA) PORT_MINMAX(0,1023) PORT_CODE_DEC(KEYCODE_8_PAD) PORT_CODE_INC(KEYCODE_2_PAD) PORT_CODE_DEC(JOYCODE_Y_UP_SWITCH)   PORT_CODE_INC(JOYCODE_Y_DOWN_SWITCH)  PORT_PLAYER(2) PORT_CONDITION(CTRL_SEL_TAG, 0xf0, EQUALS, 0x10)
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
	PORT_INCLUDE( coco_beckerport )
INPUT_PORTS_END


//-------------------------------------------------
//  INPUT_PORTS( deluxecoco )
//-------------------------------------------------

static INPUT_PORTS_START( deluxecoco )
	PORT_INCLUDE( coco )
	PORT_MODIFY("row6")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_NAME("ALT") PORT_CODE(KEYCODE_LALT) PORT_CHAR(UCHAR_MAMEKEY(LALT))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_NAME("CTRL") PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_MAMEKEY(LCONTROL), UCHAR_SHIFT_2)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_F1) PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, 0) PORT_CODE(KEYCODE_F2) PORT_CHAR(UCHAR_MAMEKEY(F2))
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
	ADDRESS_MAP_BANK(config, m_floating).set_map(&coco_state::coco_floating_map).set_options(ENDIANNESS_BIG, 8, 16);
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
//  DEVICE_INPUT_DEFAULTS_START( acia )
//-------------------------------------------------

static DEVICE_INPUT_DEFAULTS_START(acia)
	DEVICE_INPUT_DEFAULTS("RS232_TXBAUD", 0xff, RS232_BAUD_300)
	DEVICE_INPUT_DEFAULTS("RS232_RXBAUD", 0xff, RS232_BAUD_300)
	DEVICE_INPUT_DEFAULTS("RS232_DATABITS", 0xff, RS232_DATABITS_8)
	DEVICE_INPUT_DEFAULTS("RS232_PARITY", 0xff, RS232_PARITY_NONE)
	DEVICE_INPUT_DEFAULTS("RS232_STOPBITS", 0xff, RS232_STOPBITS_1)
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
	COCO_DWSOCK(config, m_beckerport, 0);

	// sound hardware
	coco_sound(config);

	CASSETTE(config, m_cassette);
	m_cassette->set_formats(coco_cassette_formats);
	m_cassette->set_default_state(CASSETTE_PLAY | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_ENABLED);

	rs232_port_device &rs232(RS232_PORT(config, RS232_TAG, default_rs232_devices, "rs_printer"));
	rs232.dcd_handler().set(m_pia_1, FUNC(pia6821_device::ca1_w));
	rs232.set_option_device_input_defaults("rs_printer", DEVICE_INPUT_DEFAULTS_NAME(rs_printer));

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);

	MC6847_NTSC(config, m_vdg, XTAL(14'318'181) / 4); // VClk output from MC6883
	m_vdg->set_screen(m_screen);
	m_vdg->hsync_wr_callback().set(FUNC(coco12_state::horizontal_sync));
	m_vdg->fsync_wr_callback().set(FUNC(coco12_state::field_sync));
	m_vdg->input_callback().set(FUNC(coco12_state::sam_read));

	// internal ram
	RAM(config, m_ram).set_default_size("64K").set_extra_options("4K,16K,32K");

	// floating space
	coco_floating(config);

	// cartridge
	COCOCART_SLOT(config, m_cococart, DERIVED_CLOCK(1, 1), coco_cart, "fdc");
	m_cococart->cart_callback().set([this] (int state) { cart_w(state != 0); }); // lambda because name is overloaded
	m_cococart->nmi_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);
	m_cococart->halt_callback().set_inputline(m_maincpu, INPUT_LINE_HALT);

	// software lists
	SOFTWARE_LIST(config, "coco_cart_list").set_original("coco_cart").set_filter("COCO");
	SOFTWARE_LIST(config, "coco_cass_list").set_original("coco_cass").set_filter("COCO");
	SOFTWARE_LIST(config, "coco_flop_list").set_original("coco_flop").set_filter("COCO");
	SOFTWARE_LIST(config, "dragon_cart_list").set_compatible("dragon_cart");

	// virtual hard disks
	COCO_VHD(config, m_vhd_0, 0, m_maincpu);
	COCO_VHD(config, m_vhd_1, 0, m_maincpu);
}

void coco12_state::cocoh(machine_config &config)
{
	coco(config);

	HD6309E(config.replace(), m_maincpu, DERIVED_CLOCK(1, 1));
	m_maincpu->set_addrmap(AS_PROGRAM, &coco12_state::coco_mem);
}

void deluxecoco_state::deluxecoco(machine_config &config)
{
	coco2b(config);

	// Asynchronous Communications Interface Adapter
	MOS6551(config, m_acia, 0);
	m_acia->set_xtal(1.8432_MHz_XTAL);
	m_acia->irq_handler().set(m_irqs, FUNC(input_merger_device::in_w<2>));
	m_acia->txd_handler().set(ACIA_TAG, FUNC(rs232_port_device::write_txd));
	m_acia->rts_handler().set(ACIA_TAG, FUNC(rs232_port_device::write_rts));
	m_acia->dtr_handler().set(ACIA_TAG, FUNC(rs232_port_device::write_dtr));

	rs232_port_device &rs232(RS232_PORT(config, ACIA_TAG, default_rs232_devices, nullptr));
	rs232.set_option_device_input_defaults("null_modem", DEVICE_INPUT_DEFAULTS_NAME(acia));
	rs232.set_option_device_input_defaults("pty", DEVICE_INPUT_DEFAULTS_NAME(acia));
	rs232.set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(acia));
	rs232.rxd_handler().set(m_acia, FUNC(mos6551_device::write_rxd));
	rs232.dcd_handler().set(m_acia, FUNC(mos6551_device::write_dcd));
	rs232.dsr_handler().set(m_acia, FUNC(mos6551_device::write_dsr));
	rs232.cts_handler().set(m_acia, FUNC(mos6551_device::write_cts));

	// Programable Sound Generator
	AY8913(config, m_psg, DERIVED_CLOCK(2, 1));
	m_psg->set_flags(AY8910_SINGLE_OUTPUT);
	m_psg->add_route(ALL_OUTPUTS, "speaker", 1.0);

	// Adjust Memory Map
	m_sam->set_addrmap(3, &deluxecoco_state::deluxecoco_rom2);
	m_sam->set_addrmap(5, &deluxecoco_state::deluxecoco_io1);

	// Configure Timer
	TIMER(config, m_timer).configure_generic(FUNC(deluxecoco_state::perodic_timer));
}

void coco12_state::coco2b(machine_config &config)
{
	coco(config);

	MC6847T1_NTSC(config.replace(), m_vdg, XTAL(14'318'181) / 4);
	m_vdg->set_screen(m_screen);
	m_vdg->hsync_wr_callback().set(FUNC(coco12_state::horizontal_sync));
	m_vdg->fsync_wr_callback().set(FUNC(coco12_state::field_sync));
	m_vdg->input_callback().set(FUNC(coco12_state::sam_read));
}

void coco12_state::coco2bh(machine_config &config)
{
	coco2b(config);

	HD6309E(config.replace(), m_maincpu, DERIVED_CLOCK(1, 1));
	m_maincpu->set_addrmap(AS_PROGRAM, &coco12_state::coco_mem);
}

void coco12_state::cp400(machine_config &config)
{
	coco(config);

	m_cococart->set_default_option("cp450_fdc");
}

void coco12_state::t4426(machine_config &config)
{
	coco(config);

	m_cococart->option_reset();
	t4426_cart(*m_cococart);
	m_cococart->set_default_option("t4426");
	m_cococart->set_fixed(true); // This cart is fixed so no way to change it
}

void coco12_state::cd6809(machine_config &config)
{
	coco(config);

	m_cococart->set_default_option("cd6809_fdc");
}

void coco12_state::ms1600(machine_config &config)
{
	coco(config);

	m_sam->set_addrmap(3, &coco12_state::ms1600_rom2);
}

//**************************************************************************
//  ROMS
//**************************************************************************

ROM_START(coco)
	ROM_REGION(0x8000,MAINCPU_TAG,0)
	ROM_DEFAULT_BIOS("b12e11")
	ROM_SYSTEM_BIOS(0, "b10", "Color BASIC v1.0")
	ROMX_LOAD("bas10.rom",    0x2000, 0x2000, CRC(00b50aaa) SHA1(1f08455cd48ce6a06132aea15c4778f264e19539), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "b11", "Color BASIC v1.1")
	ROMX_LOAD("bas11.rom",    0x2000, 0x2000, CRC(6270955a) SHA1(cecb7c24ff1e0ab5836e4a7a8eb1b8e01f1fded3), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "b12", "Color BASIC v1.2")
	ROMX_LOAD("bas12.rom",    0x2000, 0x2000, CRC(54368805) SHA1(0f14dc46c647510eb0b7bd3f53e33da07907d04f), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(3, "b10e10", "Color BASIC v1.0 / Extended Color BASIC v1.0")
	ROMX_LOAD("bas10.rom",    0x2000, 0x2000, CRC(00b50aaa) SHA1(1f08455cd48ce6a06132aea15c4778f264e19539), ROM_BIOS(3))
	ROMX_LOAD("extbas10.rom", 0x0000, 0x2000, CRC(6111a086) SHA1(8aa58f2eb3e8bcfd5470e3e35e2b359e9a72848e), ROM_BIOS(3))
	ROM_SYSTEM_BIOS(4, "b10e11", "Color BASIC v1.0 / Extended Color BASIC v1.1")
	ROMX_LOAD("bas10.rom",    0x2000, 0x2000, CRC(00b50aaa) SHA1(1f08455cd48ce6a06132aea15c4778f264e19539), ROM_BIOS(4))
	ROMX_LOAD("extbas11.rom", 0x0000, 0x2000, CRC(a82a6254) SHA1(ad927fb4f30746d820cb8b860ebb585e7f095dea), ROM_BIOS(4))
	ROM_SYSTEM_BIOS(5, "b11e10", "Color BASIC v1.1 / Extended Color BASIC v1.0")
	ROMX_LOAD("bas11.rom",    0x2000, 0x2000, CRC(6270955a) SHA1(cecb7c24ff1e0ab5836e4a7a8eb1b8e01f1fded3), ROM_BIOS(5))
	ROMX_LOAD("extbas10.rom", 0x0000, 0x2000, CRC(6111a086) SHA1(8aa58f2eb3e8bcfd5470e3e35e2b359e9a72848e), ROM_BIOS(5))
	ROM_SYSTEM_BIOS(6, "b11e11", "Color BASIC v1.1 / Extended Color BASIC v1.1")
	ROMX_LOAD("bas11.rom",    0x2000, 0x2000, CRC(6270955a) SHA1(cecb7c24ff1e0ab5836e4a7a8eb1b8e01f1fded3), ROM_BIOS(6))
	ROMX_LOAD("extbas11.rom", 0x0000, 0x2000, CRC(a82a6254) SHA1(ad927fb4f30746d820cb8b860ebb585e7f095dea), ROM_BIOS(6))
	ROM_SYSTEM_BIOS(7, "b12e10", "Color BASIC v1.2 / Extended Color BASIC v1.0")
	ROMX_LOAD("bas12.rom",    0x2000, 0x2000, CRC(54368805) SHA1(0f14dc46c647510eb0b7bd3f53e33da07907d04f), ROM_BIOS(7))
	ROMX_LOAD("extbas10.rom", 0x0000, 0x2000, CRC(6111a086) SHA1(8aa58f2eb3e8bcfd5470e3e35e2b359e9a72848e), ROM_BIOS(7))
	ROM_SYSTEM_BIOS(8, "b12e11", "Color BASIC v1.2 / Extended Color BASIC v1.1")
	ROMX_LOAD("bas12.rom",    0x2000, 0x2000, CRC(54368805) SHA1(0f14dc46c647510eb0b7bd3f53e33da07907d04f), ROM_BIOS(8))
	ROMX_LOAD("extbas11.rom", 0x0000, 0x2000, CRC(a82a6254) SHA1(ad927fb4f30746d820cb8b860ebb585e7f095dea), ROM_BIOS(8))
ROM_END

ROM_START(deluxecoco)
	ROM_REGION(0x8000,MAINCPU_TAG,0)
	ROM_LOAD("adv070_u24.rom", 0x0000, 0x2000, CRC(827fe698) SHA1(70052321688bbc9583b3e017957cc2085bb8d0ae))
	ROM_LOAD("adv071_u24.rom", 0x2000, 0x2000, CRC(0a3942e4) SHA1(2f3d67efd80c36533d0220324c254fbeea364aaa))
	ROM_LOAD("adv072_u24.rom", 0x4000, 0x2000, CRC(c0118da5) SHA1(feea48b6b7070f0ac0acb132ad85087c5ad79e3b))
	ROM_LOAD("adv073-2_u24.rom", 0x6000, 0x2000, CRC(61411227) SHA1(c3aba0eb359f7f40d40d7947f72c9ecae6e0525d))
ROM_END

ROM_START(coco2b)
	ROM_REGION(0x8000,MAINCPU_TAG,0)
	ROM_LOAD("bas13.rom",    0x2000, 0x2000, CRC(d8f4d15e) SHA1(28b92bebe35fa4f026a084416d6ea3b1552b63d3))
	ROM_LOAD("extbas11.rom", 0x0000, 0x2000, CRC(a82a6254) SHA1(ad927fb4f30746d820cb8b860ebb585e7f095dea))
ROM_END

ROM_START(cp400)
	ROM_REGION(0x8000,MAINCPU_TAG,0)
	ROM_LOAD("cp400bas.rom", 0x0000, 0x4000, CRC(878396a5) SHA1(292c545da3c77978e043b00a3dbc317201d18c3b))
ROM_END

ROM_START(cp400c2)
	ROM_REGION(0x8000,MAINCPU_TAG,0)
	ROM_LOAD("cp400bas.rom", 0x0000, 0x4000, CRC(878396a5) SHA1(292c545da3c77978e043b00a3dbc317201d18c3b))
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

ROM_START(ms1600)
	ROM_REGION(0x8000,MAINCPU_TAG,0)
	ROM_LOAD("ms1600.rom", 0x0000, 0x8000, CRC(66f0fafc) SHA1(6e709b8b44aa34a3235a889eb1c3615c0235c3d0))
ROM_END

#define rom_cocoh rom_coco
#define rom_coco2bh rom_coco2b

//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR   NAME        PARENT  COMPAT  MACHINE     INPUT       CLASS             INIT        COMPANY                         FULLNAME                               FLAGS
COMP( 1980,  coco,       0,      0,      coco,       coco,       coco12_state,     empty_init, "Tandy Radio Shack",            "Color Computer 1/2",                  MACHINE_SUPPORTS_SAVE )
COMP( 19??,  cocoh,      coco,   0,      cocoh,      coco,       coco12_state,     empty_init, "Tandy Radio Shack",            "Color Computer 1/2 (HD6309)",         MACHINE_SUPPORTS_SAVE | MACHINE_UNOFFICIAL )
COMP( 1983,  deluxecoco, coco,   0,      deluxecoco, deluxecoco, deluxecoco_state, empty_init, "Tandy Radio Shack",            "Deluxe Color Computer",               MACHINE_SUPPORTS_SAVE )
COMP( 1985?, coco2b,     coco,   0,      coco2b,     coco,       coco12_state,     empty_init, "Tandy Radio Shack",            "Color Computer 2B",                   MACHINE_SUPPORTS_SAVE )
COMP( 19??,  coco2bh,    coco,   0,      coco2bh,    coco,       coco12_state,     empty_init, "Tandy Radio Shack",            "Color Computer 2B (HD6309)",          MACHINE_SUPPORTS_SAVE | MACHINE_UNOFFICIAL )
COMP( 1983,  cp400,      coco,   0,      cp400,      coco,       coco12_state,     empty_init, "Prológica",                    "CP400",                               MACHINE_SUPPORTS_SAVE )
COMP( 1985,  cp400c2,    coco,   0,      cp400,      cp400c2,    coco12_state,     empty_init, "Prológica",                    "CP400 Color II",                      MACHINE_SUPPORTS_SAVE )
COMP( 1984,  mx1600,     coco,   0,      coco,       coco,       coco12_state,     empty_init, "Dynacom",                      "MX-1600",                             MACHINE_SUPPORTS_SAVE )
COMP( 1986,  t4426,      coco,   0,      t4426,      coco,       coco12_state,     empty_init, "Terco AB",                     "Terco 4426 CNC Programming station",  MACHINE_SUPPORTS_SAVE )
COMP( 1983,  lzcolor64,  coco,   0,      coco,       coco,       coco12_state,     empty_init, "Novo Tempo / LZ Equipamentos", "Color64",                             MACHINE_SUPPORTS_SAVE )
COMP( 1983,  cd6809,     coco,   0,      cd6809,     coco,       coco12_state,     empty_init, "Codimex",                      "CD-6809",                             MACHINE_SUPPORTS_SAVE )
COMP( 1987,  ms1600,     coco,   0,      ms1600,     coco,       coco12_state,     empty_init, "ILCE / SEP",                   "Micro-SEP 1600",                      MACHINE_SUPPORTS_SAVE )
