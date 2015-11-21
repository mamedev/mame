// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    coco12.c

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

#include "includes/coco12.h"
#include "imagedev/cassette.h"
#include "cpu/m6809/m6809.h"
#include "bus/coco/coco_232.h"
#include "bus/coco/coco_orch90.h"
#include "bus/coco/coco_pak.h"
#include "bus/coco/coco_fdc.h"
#include "bus/coco/coco_multi.h"
#include "bus/coco/coco_dwsock.h"
#include "formats/coco_cas.h"
#include "softlist.h"

//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

//-------------------------------------------------
//  ADDRESS_MAP( coco_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( coco_mem, AS_PROGRAM, 8, coco_state )
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

static INPUT_PORTS_START( coco_joystick )
	PORT_START(JOYSTICK_RX_TAG)
	PORT_BIT( 0xff, 0x80,  IPT_AD_STICK_X) PORT_SENSITIVITY(JOYSTICK_SENSITIVITY) PORT_KEYDELTA(JOYSTICK_DELTA) PORT_MINMAX(0x00,0xFF) PORT_CODE_DEC(KEYCODE_4_PAD) PORT_CODE_INC(KEYCODE_6_PAD) PORT_CODE_DEC(JOYCODE_X_LEFT_SWITCH) PORT_CODE_INC(JOYCODE_X_RIGHT_SWITCH) PORT_PLAYER(1) PORT_CONDITION(CTRL_SEL_TAG, 0x0f, EQUALS, 0x01)
	PORT_START(JOYSTICK_RY_TAG)
	PORT_BIT( 0xff, 0x80,  IPT_AD_STICK_Y) PORT_SENSITIVITY(JOYSTICK_SENSITIVITY) PORT_KEYDELTA(JOYSTICK_DELTA) PORT_MINMAX(0x00,0xFF) PORT_CODE_DEC(KEYCODE_8_PAD) PORT_CODE_INC(KEYCODE_2_PAD) PORT_CODE_DEC(JOYCODE_Y_UP_SWITCH)   PORT_CODE_INC(JOYCODE_Y_DOWN_SWITCH)  PORT_PLAYER(1) PORT_CONDITION(CTRL_SEL_TAG, 0x0f, EQUALS, 0x01)
	PORT_START(JOYSTICK_LX_TAG)
	PORT_BIT( 0xff, 0x80,  IPT_AD_STICK_X) PORT_SENSITIVITY(JOYSTICK_SENSITIVITY) PORT_KEYDELTA(JOYSTICK_DELTA) PORT_MINMAX(0x00,0xFF) PORT_CODE_DEC(KEYCODE_4_PAD) PORT_CODE_INC(KEYCODE_6_PAD) PORT_CODE_DEC(JOYCODE_X_LEFT_SWITCH) PORT_CODE_INC(JOYCODE_X_RIGHT_SWITCH) PORT_PLAYER(2) PORT_CONDITION(CTRL_SEL_TAG, 0xf0, EQUALS, 0x10)
	PORT_START(JOYSTICK_LY_TAG)
	PORT_BIT( 0xff, 0x80,  IPT_AD_STICK_Y) PORT_SENSITIVITY(JOYSTICK_SENSITIVITY) PORT_KEYDELTA(JOYSTICK_DELTA) PORT_MINMAX(0x00,0xFF) PORT_CODE_DEC(KEYCODE_8_PAD) PORT_CODE_INC(KEYCODE_2_PAD) PORT_CODE_DEC(JOYCODE_Y_UP_SWITCH)   PORT_CODE_INC(JOYCODE_Y_DOWN_SWITCH)  PORT_PLAYER(2) PORT_CONDITION(CTRL_SEL_TAG, 0xf0, EQUALS, 0x10)
	PORT_START(JOYSTICK_BUTTONS_TAG)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Right Button") PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, NULL) PORT_CODE(KEYCODE_0_PAD) PORT_CODE(JOYCODE_BUTTON1) PORT_CODE(MOUSECODE_BUTTON1) PORT_PLAYER(1) PORT_CONDITION(CTRL_SEL_TAG, 0x0f, EQUALS, 0x01)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Left Button")  PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, NULL) PORT_CODE(KEYCODE_0_PAD) PORT_CODE(JOYCODE_BUTTON1) PORT_CODE(MOUSECODE_BUTTON1) PORT_PLAYER(2) PORT_CONDITION(CTRL_SEL_TAG, 0xf0, EQUALS, 0x10)
INPUT_PORTS_END



//-------------------------------------------------
//  INPUT_PORTS( coco_cart_autostart )
//-------------------------------------------------

INPUT_PORTS_START( coco_cart_autostart )
	PORT_START(CART_AUTOSTART_TAG)
	PORT_CONFNAME( 0x01, 0x01, "Cart Auto-Start" )
	PORT_CONFSETTING(    0x00, DEF_STR( Off ))
	PORT_CONFSETTING(    0x01, DEF_STR( On ))
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
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, NULL) PORT_CODE(KEYCODE_ASTERISK) PORT_CHAR('@')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, NULL) PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, NULL) PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, NULL) PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, NULL) PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, NULL) PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, NULL) PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, NULL) PORT_CODE(KEYCODE_G) PORT_CHAR('G')

	PORT_START("row1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, NULL) PORT_CODE(KEYCODE_H) PORT_CHAR('H')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, NULL) PORT_CODE(KEYCODE_I) PORT_CHAR('I')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, NULL) PORT_CODE(KEYCODE_J) PORT_CHAR('J')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, NULL) PORT_CODE(KEYCODE_K) PORT_CHAR('K')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, NULL) PORT_CODE(KEYCODE_L) PORT_CHAR('L')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, NULL) PORT_CODE(KEYCODE_M) PORT_CHAR('M')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, NULL) PORT_CODE(KEYCODE_N) PORT_CHAR('N')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, NULL) PORT_CODE(KEYCODE_O) PORT_CHAR('O')

	PORT_START("row2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, NULL) PORT_CODE(KEYCODE_P) PORT_CHAR('P')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, NULL) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, NULL) PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, NULL) PORT_CODE(KEYCODE_S) PORT_CHAR('S')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, NULL) PORT_CODE(KEYCODE_T) PORT_CHAR('T')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, NULL) PORT_CODE(KEYCODE_U) PORT_CHAR('U')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, NULL) PORT_CODE(KEYCODE_V) PORT_CHAR('V')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, NULL) PORT_CODE(KEYCODE_W) PORT_CHAR('W')

	PORT_START("row3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, NULL) PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, NULL) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, NULL) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, NULL) PORT_NAME("UP") PORT_CODE(KEYCODE_UP) PORT_CHAR('^')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, NULL) PORT_NAME("DOWN") PORT_CODE(KEYCODE_DOWN) PORT_CHAR(10)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, NULL) PORT_NAME("LEFT") PORT_CODE(KEYCODE_LEFT) PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, NULL) PORT_NAME("RIGHT") PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(9)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, NULL) PORT_NAME("SPACE") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')

	PORT_START("row4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, NULL) PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, NULL) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, NULL) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('\"')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, NULL) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, NULL) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, NULL) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, NULL) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, NULL) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')

	PORT_START("row5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, NULL) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, NULL) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, NULL) PORT_CODE(KEYCODE_COLON) PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, NULL) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, NULL) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, NULL) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, NULL) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, NULL) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')

	PORT_START("row6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, NULL) PORT_NAME("ENTER") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, NULL) PORT_NAME("CLEAR") PORT_CODE(KEYCODE_HOME) PORT_CHAR(12)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, NULL) PORT_NAME("BREAK") PORT_CODE(KEYCODE_END) PORT_CODE(KEYCODE_ESC) PORT_CHAR(27)
	PORT_BIT(0x78, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, coco12_state, coco_state::keyboard_changed, NULL) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
INPUT_PORTS_END



//-------------------------------------------------
//  INPUT_PORTS( coco )
//-------------------------------------------------

static INPUT_PORTS_START( coco )
	PORT_INCLUDE( coco_keyboard )
	PORT_INCLUDE( coco_joystick )
	PORT_INCLUDE( coco_analog_control )
	PORT_INCLUDE( coco_cart_autostart )
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
	SLOT_INTERFACE("cp400_fdc", CP400_FDC)
	SLOT_INTERFACE("rs232", COCO_232)
	SLOT_INTERFACE("orch90", COCO_ORCH90)
	SLOT_INTERFACE("banked_16k", COCO_PAK_BANKED)
	SLOT_INTERFACE("pak", COCO_PAK)
	SLOT_INTERFACE("multi", COCO_MULTIPAK)
SLOT_INTERFACE_END



//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( coco_sound )
//-------------------------------------------------

MACHINE_CONFIG_FRAGMENT( coco_sound )
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(DAC_TAG, DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
	MCFG_SOUND_WAVE_ADD(WAVE_TAG, "cassette")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END


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

static MACHINE_CONFIG_START( coco, coco12_state )
	// basic machine hardware
	MCFG_CPU_ADD(MAINCPU_TAG, M6809E, XTAL_3_579545MHz)
	MCFG_CPU_PROGRAM_MAP(coco_mem)

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

	MCFG_SAM6883_ADD(SAM_TAG, XTAL_3_579545MHz, MAINCPU_TAG, AS_PROGRAM)
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

	MCFG_DEVICE_ADD(VDG_TAG, MC6847_NTSC, XTAL_3_579545MHz)
	MCFG_MC6847_HSYNC_CALLBACK(WRITELINE(coco12_state, horizontal_sync))
	MCFG_MC6847_FSYNC_CALLBACK(WRITELINE(coco12_state, field_sync))
	MCFG_MC6847_INPUT_CALLBACK(DEVREAD8(SAM_TAG, sam6883_device, display_read))

	// sound hardware
	MCFG_FRAGMENT_ADD( coco_sound )

	// internal ram
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("64K")
	MCFG_RAM_EXTRA_OPTIONS("4K,16K,32K")

	// software lists
	MCFG_SOFTWARE_LIST_ADD("cart_list","coco_cart")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( cocoe, coco )
	MCFG_COCO_CARTRIDGE_REMOVE(CARTRIDGE_TAG)
	MCFG_COCO_CARTRIDGE_ADD(CARTRIDGE_TAG, coco_cart, "fdc")
	MCFG_COCO_CARTRIDGE_CART_CB(WRITELINE(coco_state, cart_w))
	MCFG_COCO_CARTRIDGE_NMI_CB(INPUTLINE(MAINCPU_TAG, INPUT_LINE_NMI))
	MCFG_COCO_CARTRIDGE_HALT_CB(INPUTLINE(MAINCPU_TAG, INPUT_LINE_HALT))
	MCFG_COCO_VHD_ADD(VHD0_TAG)
	MCFG_COCO_VHD_ADD(VHD1_TAG)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( coco2, coco )
	MCFG_COCO_CARTRIDGE_REMOVE(CARTRIDGE_TAG)
	MCFG_COCO_CARTRIDGE_ADD(CARTRIDGE_TAG, coco_cart, "fdcv11")
	MCFG_COCO_CARTRIDGE_CART_CB(WRITELINE(coco_state, cart_w))
	MCFG_COCO_CARTRIDGE_NMI_CB(INPUTLINE(MAINCPU_TAG, INPUT_LINE_NMI))
	MCFG_COCO_CARTRIDGE_HALT_CB(INPUTLINE(MAINCPU_TAG, INPUT_LINE_HALT))
	MCFG_COCO_VHD_ADD(VHD0_TAG)
	MCFG_COCO_VHD_ADD(VHD1_TAG)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( coco2b, coco2 )
	MCFG_DEVICE_REMOVE(VDG_TAG)
	MCFG_DEVICE_ADD(VDG_TAG, MC6847T1_NTSC, XTAL_3_579545MHz)
	MCFG_MC6847_HSYNC_CALLBACK(WRITELINE(coco12_state, horizontal_sync))
	MCFG_MC6847_FSYNC_CALLBACK(WRITELINE(coco12_state, field_sync))
	MCFG_MC6847_INPUT_CALLBACK(DEVREAD8(SAM_TAG, sam6883_device, display_read))
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( cp400, coco )
	MCFG_COCO_CARTRIDGE_REMOVE(CARTRIDGE_TAG)
	MCFG_COCO_CARTRIDGE_ADD(CARTRIDGE_TAG, coco_cart, "cp400_fdc")
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

ROM_START(mx1600 )
	ROM_REGION(0x8000,MAINCPU_TAG,0)
	ROM_LOAD("mx1600bas.rom",    0x2000, 0x2000, CRC(d918156e) SHA1(70a464edf3a654ed4ffe687e6dee4f0d2acc758b))
	ROM_LOAD("mx1600extbas.rom", 0x0000, 0x2000, CRC(322a3d58) SHA1(9079a477c3f22e46cebb1e68b61df5bd607c71a4))
ROM_END

ROM_START(lzcolor64 )
	ROM_REGION(0x8000,MAINCPU_TAG,0)
	ROM_LOAD("color64bas.rom",    0x2000, 0x2000, CRC(b0717d71) SHA1(ad1beef9d6f095ada69f91d0b8ad75985172d86f))
	ROM_LOAD("color64extbas.rom", 0x0000, 0x2000, CRC(d1b1560d) SHA1(7252de9df405ade453282e992eb1f1910adc8e50))
ROM_END

//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

/*     YEAR     NAME        PARENT  COMPAT  MACHINE    INPUT      INIT    COMPANY                 FULLNAME */
COMP(  1980,    coco,       0,      0,      coco,      coco, driver_device,      0,      "Tandy Radio Shack",    "Color Computer", 0)
COMP(  1981,    cocoe,      coco,   0,      cocoe,     coco, driver_device,      0,      "Tandy Radio Shack",    "Color Computer (Extended BASIC 1.0)", 0)
COMP(  1983,    coco2,      coco,   0,      coco2,     coco, driver_device,      0,      "Tandy Radio Shack",     "Color Computer 2", 0)
COMP(  1985?,   coco2b,     coco,   0,      coco2b,    coco, driver_device,      0,      "Tandy Radio Shack",     "Color Computer 2B", 0)
COMP(  1984,    cp400,      coco,   0,      cp400,     coco, driver_device,      0,      "Prologica",            "CP400", 0)
COMP(  1984,    lzcolor64,  coco,   0,      coco,      coco, driver_device,      0,      "Digiponto",            "LZ Color64", 0)
COMP(  1984,    mx1600,     coco,   0,      coco,      coco, driver_device,      0,      "Dynacom",              "MX-1600", 0)
