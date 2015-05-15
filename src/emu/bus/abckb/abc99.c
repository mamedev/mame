// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Luxor ABC-99 keyboard and mouse emulation

*********************************************************************/

/*

Keyboard PCB Layout
-------------------

|-----------------------------------------------------------------------|
|   CN1         CN2             CPU1        ROM1                    SW1 |
|                                                                       |
|   6MHz    CPU0        ROM0        GI                                  |
|                                                                       |
|                                                                       |
|                                                                       |
|                                                                       |
|                                                                       |
|                                                                       |
|                                                                       |
|                                                                       |
|-----------------------------------------------------------------------|

Notes:
    Relevant IC's shown.

    CPU0        - STMicro ET8035N-6 8035 CPU
    CPU1        - STMicro ET8035N-6 8035 CPU
    ROM0        - Texas Instruments TMS2516JL-16 2Kx8 ROM "107268-16"
    ROM1        - STMicro ET2716Q-1 2Kx8 ROM "107268-17"
    GI          - General Instruments 321239007 keyboard chip "4=7"
    CN1         - DB15 connector, Luxor ABC R8 (3 button mouse)
    CN2         - 12 pin PCB header, keyboard data cable
    SW1         - reset switch?

*/

/*

    TODO:

    - verify cursor keys
    - language DIP
    - mouse
    - MCS-48 PC:01DC - Unimplemented opcode = 75
        - 75 = ENT0 CLK : enable CLK (unscaled_clock/3) output on T0
        - halt Z2 when Z5 is reset, resume Z2 when Z5 executes ENT0 CLK instruction

*/

#include "abc99.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define I8035_Z2_TAG        "z2"
#define I8035_Z5_TAG        "z5"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type ABC99 = &device_creator<abc99_device>;


//-------------------------------------------------
//  ROM( abc99 )
//-------------------------------------------------

ROM_START( abc99 )
	ROM_REGION( 0x1000, I8035_Z2_TAG, 0 )
	ROM_DEFAULT_BIOS("107268")
	ROM_SYSTEM_BIOS( 0, "107268", "107268-17" )
	ROMX_LOAD( "107268-17.z3", 0x0000, 0x0800, CRC(2f60cc35) SHA1(ebc6af9cd0a49a0d01698589370e628eebb6221c), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "106819", "106819-09" )
	ROMX_LOAD( "106819-09.z3", 0x0000, 0x1000, CRC(ffe32a71) SHA1(fa2ce8e0216a433f9bbad0bdd6e3dc0b540f03b7), ROM_BIOS(2) ) // ABC 99 6490423-01

	ROM_REGION( 0x800, I8035_Z5_TAG, 0 )
	ROMX_LOAD( "107268-16.z6", 0x0000, 0x0800, CRC(785ec0c6) SHA1(0b261beae20dbc06fdfccc50b19ea48b5b6e22eb), ROM_BIOS(1) )
	ROMX_LOAD( "107268-64.z6", 0x0000, 0x0800, CRC(e33683ae) SHA1(0c1d9e320f82df05f4804992ef6f6f6cd20623f3), ROM_BIOS(2) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *abc99_device::device_rom_region() const
{
	return ROM_NAME( abc99 );
}


//-------------------------------------------------
//  ADDRESS_MAP( abc99_z2_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( abc99_z2_mem, AS_PROGRAM, 8, abc99_device )
	AM_RANGE(0x0000, 0x0fff) AM_ROM AM_REGION(I8035_Z2_TAG, 0)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( abc99_z2_io )
//-------------------------------------------------

static ADDRESS_MAP_START( abc99_z2_io, AS_IO, 8, abc99_device )
	AM_RANGE(0x21, 0x21) AM_WRITE(z2_led_w)
	AM_RANGE(0x30, 0x30) AM_READ_PORT("X0") AM_WRITENOP
	AM_RANGE(0x31, 0x31) AM_READ_PORT("X1") AM_WRITENOP
	AM_RANGE(0x32, 0x32) AM_READ_PORT("X2") AM_WRITENOP
	AM_RANGE(0x33, 0x33) AM_READ_PORT("X3") AM_WRITENOP
	AM_RANGE(0x34, 0x34) AM_READ_PORT("X4") AM_WRITENOP
	AM_RANGE(0x35, 0x35) AM_READ_PORT("X5") AM_WRITENOP
	AM_RANGE(0x36, 0x36) AM_READ_PORT("X6") AM_WRITENOP
	AM_RANGE(0x37, 0x37) AM_READ_PORT("X7") AM_WRITENOP
	AM_RANGE(0x38, 0x38) AM_READ_PORT("X8") AM_WRITENOP
	AM_RANGE(0x39, 0x39) AM_READ_PORT("X9") AM_WRITENOP
	AM_RANGE(0x3a, 0x3a) AM_READ_PORT("X10") AM_WRITENOP
	AM_RANGE(0x3b, 0x3b) AM_READ_PORT("X11") AM_WRITENOP
	AM_RANGE(0x3c, 0x3c) AM_READ_PORT("X12") AM_WRITENOP
	AM_RANGE(0x3d, 0x3d) AM_READ_PORT("X13") AM_WRITENOP
	AM_RANGE(0x3e, 0x3e) AM_READ_PORT("X14") AM_WRITENOP
	AM_RANGE(0x3f, 0x3f) AM_READ_PORT("X15") AM_WRITENOP
	AM_RANGE(MCS48_PORT_P1, MCS48_PORT_P1) AM_WRITE(z2_p1_w)
	AM_RANGE(MCS48_PORT_P2, MCS48_PORT_P2) AM_READ(z2_p2_r)
	AM_RANGE(MCS48_PORT_T0, MCS48_PORT_T0) AM_READ(z2_t0_r)
	AM_RANGE(MCS48_PORT_T1, MCS48_PORT_T1) AM_READ(z2_t1_r)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( abc99_z5_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( abc99_z5_mem, AS_PROGRAM, 8, abc99_device )
	AM_RANGE(0x0000, 0x07ff) AM_ROM AM_REGION(I8035_Z5_TAG, 0)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( abc99_z5_io )
//-------------------------------------------------

static ADDRESS_MAP_START( abc99_z5_io, AS_IO, 8, abc99_device )
/*  AM_RANGE(MCS48_PORT_P1, MCS48_PORT_P1) AM_READ(z5_p1_r)
    AM_RANGE(MCS48_PORT_P2, MCS48_PORT_P2) AM_WRITE(z5_p2_w)
    AM_RANGE(MCS48_PORT_T0, MCS48_PORT_T0) AM_WRITENOP // Z2 CLK
    AM_RANGE(MCS48_PORT_T1, MCS48_PORT_T1) AM_READ(z5_t1_r)*/
ADDRESS_MAP_END


//-------------------------------------------------
//  MACHINE_DRIVER( abc99 )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( abc99 )
	// keyboard CPU
	MCFG_CPU_ADD(I8035_Z2_TAG, I8035, XTAL_6MHz/3) // from Z5 T0 output
	MCFG_CPU_PROGRAM_MAP(abc99_z2_mem)
	MCFG_CPU_IO_MAP(abc99_z2_io)

	// mouse CPU
	MCFG_CPU_ADD(I8035_Z5_TAG, I8035, XTAL_6MHz)
	MCFG_CPU_PROGRAM_MAP(abc99_z5_mem)
	MCFG_CPU_IO_MAP(abc99_z5_io)
	MCFG_DEVICE_DISABLE() // HACK fix for broken serial I/O

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor abc99_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( abc99 );
}


//-------------------------------------------------
//  INPUT_CHANGED_MEMBER( keyboard_reset )
//-------------------------------------------------

INPUT_CHANGED_MEMBER( abc99_device::keyboard_reset )
{
	if (newval)
	{
		m_mousecpu->reset();
	}
}


//-------------------------------------------------
//  INPUT_PORTS( abc99 )
//-------------------------------------------------

INPUT_PORTS_START( abc99 )
	PORT_START("X0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("PF13") PORT_CODE(KEYCODE_PRTSCR)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(UTF8_RIGHT"|") PORT_CODE(KEYCODE_TAB) PORT_CHAR('\t')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("RETURN") PORT_CODE(KEYCODE_ENTER) PORT_CHAR('\r')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("LF") PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_MAMEKEY(RCONTROL))
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH2) PORT_CHAR('<') PORT_CHAR('>')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("X1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("ALT") PORT_CODE(KEYCODE_LALT) PORT_CHAR(UCHAR_MAMEKEY(LALT))
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad CE") PORT_CODE(KEYCODE_MINUS_PAD) PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD))
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad RETURN") PORT_CODE(KEYCODE_ENTER_PAD) PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD))
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("X2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("PRINT") PORT_CODE(KEYCODE_PGUP)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad 9") PORT_CODE(KEYCODE_9_PAD) PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad 6") PORT_CODE(KEYCODE_6_PAD) PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad 3") PORT_CODE(KEYCODE_3_PAD) PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad .") PORT_CODE(KEYCODE_DEL_PAD) PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Left SHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("X3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("HELP") PORT_CODE(KEYCODE_HOME)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad 7") PORT_CODE(KEYCODE_7_PAD) PORT_CHAR(UCHAR_MAMEKEY(7_PAD))
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad 4") PORT_CODE(KEYCODE_4_PAD) PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad 1") PORT_CODE(KEYCODE_1_PAD) PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad 0") PORT_CODE(KEYCODE_0_PAD) PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("X4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("PF15") PORT_CODE(KEYCODE_PAUSE)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("DEL") PORT_CODE(KEYCODE_DEL) PORT_CHAR(UCHAR_MAMEKEY(DEL))
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(UTF8_RIGHT) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT)) // cursor B
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(UTF8_LEFT) PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT)) // cursor D
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("X5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("PF12") PORT_CODE(KEYCODE_F12) PORT_CHAR(UCHAR_MAMEKEY(F12))
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("BS") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(0x00FC) PORT_CHAR(0x00DC)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\'') PORT_CHAR('*')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Right SHIFT") PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')

	PORT_START("X6")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("PF14") PORT_CODE(KEYCODE_SCRLOCK)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("INS") PORT_CODE(KEYCODE_INSERT) PORT_CHAR(UCHAR_MAMEKEY(INSERT))
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(UTF8_UP) PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP)) // cursor A
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(UTF8_DOWN) PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN)) // cursor C
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("CTRL") PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("CAPS LOCK") PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))

	PORT_START("X7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("PF11") PORT_CODE(KEYCODE_F11) PORT_CHAR(UCHAR_MAMEKEY(F11))
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR(0x00E9) PORT_CHAR(0x00C9)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR(0x00E5) PORT_CHAR(0x00C5)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(0x00E4) PORT_CHAR(0x00C4)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')

	PORT_START("X8")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("STOP") PORT_CODE(KEYCODE_END)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad 8") PORT_CODE(KEYCODE_8_PAD) PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad 5") PORT_CODE(KEYCODE_5_PAD) PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad 2") PORT_CODE(KEYCODE_2_PAD) PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("|" UTF8_LEFT) PORT_CODE(KEYCODE_RALT) PORT_CHAR(UCHAR_MAMEKEY(RALT))

	PORT_START("X9")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('+') PORT_CHAR('?')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON) PORT_CHAR(0x00F6) PORT_CHAR(0x00D6)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR(':')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')

	PORT_START("X10")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("PF10") PORT_CODE(KEYCODE_F10) PORT_CHAR(UCHAR_MAMEKEY(F10))
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR('=')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR(';')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("PF1") PORT_CODE(KEYCODE_F1) PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("ESC") PORT_CODE(KEYCODE_ESC)

	PORT_START("X11")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("PF9") PORT_CODE(KEYCODE_F9) PORT_CHAR(UCHAR_MAMEKEY(F9))
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("PF2") PORT_CODE(KEYCODE_F2) PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')

	PORT_START("X12")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("SPACE") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')

	PORT_START("X13")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("PF6") PORT_CODE(KEYCODE_F6) PORT_CHAR(UCHAR_MAMEKEY(F6))
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("PF5") PORT_CODE(KEYCODE_F5) PORT_CHAR(UCHAR_MAMEKEY(F5))
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("4 \xC2\xA4") PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR(0x00A4)

	PORT_START("X14")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("PF7") PORT_CODE(KEYCODE_F7) PORT_CHAR(UCHAR_MAMEKEY(F7))
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('/')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("PF4") PORT_CODE(KEYCODE_F4) PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')

	PORT_START("X15")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("PF8") PORT_CODE(KEYCODE_F8) PORT_CHAR(UCHAR_MAMEKEY(F8))
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("PF3") PORT_CODE(KEYCODE_F3) PORT_CHAR(UCHAR_MAMEKEY(F3))
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"')

	PORT_START("Z14")
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Language ) ) PORT_DIPLOCATION("Z14:1,2,3")
	PORT_DIPSETTING(    0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x05, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x06, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x07, DEF_STR( Unknown ) )
	PORT_DIPNAME( 0x08, 0x08, "Keyboard Program" ) PORT_DIPLOCATION("Z14:4")
	PORT_DIPSETTING(    0x00, "Internal (8048)" )
	PORT_DIPSETTING(    0x08, "External PROM" )

	PORT_START("MOUSEB")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_NAME("Left Mouse Button") PORT_CODE(MOUSECODE_BUTTON1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_NAME("Middle Mouse Button") PORT_CODE(MOUSECODE_BUTTON3)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_NAME("Right Mouse Button") PORT_CODE(MOUSECODE_BUTTON2)

	PORT_START("MOUSEX")
	PORT_BIT( 0xff, 0x00, IPT_MOUSE_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(5) PORT_MINMAX(0, 255) PORT_PLAYER(1)

	PORT_START("MOUSEY")
	PORT_BIT( 0xff, 0x00, IPT_MOUSE_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(5) PORT_MINMAX(0, 255) PORT_PLAYER(1)

	PORT_START("J4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_NAME("Keyboard Reset") PORT_CHANGED_MEMBER(DEVICE_SELF, abc99_device, keyboard_reset, 0)
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor abc99_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( abc99 );
}



//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  serial_input -
//-------------------------------------------------

inline void abc99_device::serial_input()
{
	m_maincpu->set_input_line(MCS48_INPUT_IRQ, (m_si || m_si_en) ? CLEAR_LINE : ASSERT_LINE);
	m_mousecpu->set_input_line(MCS48_INPUT_IRQ, m_si ? CLEAR_LINE : ASSERT_LINE);
}


//-------------------------------------------------
//  serial_output -
//-------------------------------------------------

inline void abc99_device::serial_output(int state)
{
	if (m_txd != state)
	{
		m_txd = state;

		m_slot->write_rx(m_txd);
	}
}


//-------------------------------------------------
//  serial_clock -
//-------------------------------------------------

inline void abc99_device::serial_clock()
{
	m_slot->trxc_w(1);
	m_slot->trxc_w(0);
}


//-------------------------------------------------
//  keydown -
//-------------------------------------------------

inline void abc99_device::key_down(int state)
{
	if (m_keydown != state)
	{
		m_slot->keydown_w(state);
		m_keydown = state;
	}
}


//-------------------------------------------------
//  scan_mouse -
//-------------------------------------------------

inline void abc99_device::scan_mouse()
{
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  abc99_device - constructor
//-------------------------------------------------

abc99_device::abc99_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, ABC99, "Luxor ABC 99", tag, owner, clock, "abc99", __FILE__),
	abc_keyboard_interface(mconfig, *this),
	m_maincpu(*this, I8035_Z2_TAG),
	m_mousecpu(*this, I8035_Z5_TAG),
	m_speaker(*this, "speaker"),
	m_z14(*this, "Z14"),
	m_mouseb(*this, "MOUSEB"),
	m_si(1),
	m_si_en(1),
	m_so_z2(1),
	m_so_z5(1),
	m_keydown(0),
	m_t1_z2(0),
	m_t1_z5(0),
	m_led_en(0),
	m_reset(1),
	m_txd(1)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void abc99_device::device_start()
{
	// allocate timers
	m_serial_timer = timer_alloc(TIMER_SERIAL);
	m_serial_timer->adjust(MCS48_ALE_CLOCK(XTAL_6MHz/3), 0, MCS48_ALE_CLOCK(XTAL_6MHz/3));

	m_mouse_timer = timer_alloc(TIMER_MOUSE);

	// state saving
	save_item(NAME(m_si));
	save_item(NAME(m_si_en));
	save_item(NAME(m_so_z2));
	save_item(NAME(m_so_z5));
	save_item(NAME(m_keydown));
	save_item(NAME(m_t1_z2));
	save_item(NAME(m_t1_z5));
	save_item(NAME(m_led_en));
	save_item(NAME(m_reset));
	save_item(NAME(m_txd));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void abc99_device::device_reset()
{
	// set EA lines
	m_maincpu->set_input_line(MCS48_INPUT_EA, ASSERT_LINE);
	m_mousecpu->set_input_line(MCS48_INPUT_EA, ASSERT_LINE);

	m_slot->write_rx(1);
}


//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void abc99_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_SERIAL:
		serial_clock();
		break;

	case TIMER_MOUSE:
		scan_mouse();
		break;
	}
}


//-------------------------------------------------
//  txd_w -
//-------------------------------------------------

void abc99_device::txd_w(int state)
{
	if (m_si != state)
	{
		m_si = state;
		serial_input();
	}
}


//-------------------------------------------------
//  z2_bus_w -
//-------------------------------------------------

WRITE8_MEMBER( abc99_device::z2_led_w )
{
	if (m_led_en) return;

	output_set_led_value(LED_1, BIT(data, 0));
	output_set_led_value(LED_2, BIT(data, 1));
	output_set_led_value(LED_3, BIT(data, 2));
	output_set_led_value(LED_4, BIT(data, 3));
	output_set_led_value(LED_5, BIT(data, 4));
	output_set_led_value(LED_6, BIT(data, 5));
	output_set_led_value(LED_7, BIT(data, 6));
	output_set_led_value(LED_8, BIT(data, 7));
}


//-------------------------------------------------
//  z2_p1_w -
//-------------------------------------------------

WRITE8_MEMBER( abc99_device::z2_p1_w )
{
	/*

	    bit     description

	    P10     serial output
	    P11     KEY DOWN
	    P12     transmit -> Z5 T1
	    P13     INS led
	    P14     ALT led
	    P15     CAPS LOCK led
	    P16     speaker output
	    P17     Z8 enable

	*/

	// serial output
	m_so_z2 = BIT(data, 0);
	serial_output(m_so_z2 && m_so_z5);

	// key down
	key_down(!BIT(data, 1));

	// master T1
	m_t1_z5 = BIT(data, 2);

	// key LEDs
	output_set_led_value(LED_INS, BIT(data, 3));
	output_set_led_value(LED_ALT, BIT(data, 4));
	output_set_led_value(LED_CAPS_LOCK, BIT(data, 5));

	// speaker output
	m_speaker->level_w(!BIT(data, 6));

	// Z8 enable
	m_led_en = BIT(data, 7);
}


//-------------------------------------------------
//  z2_p2_r -
//-------------------------------------------------

READ8_MEMBER( abc99_device::z2_p2_r )
{
	/*

	    bit     description

	    P20
	    P21
	    P22
	    P23
	    P24
	    P25     DIP0
	    P26     DIP1
	    P27     DIP2

	*/

	UINT8 data = m_z14->read() << 5;

	return data;
}


//-------------------------------------------------
//  z2_t0_r -
//-------------------------------------------------

READ8_MEMBER( abc99_device::z2_t0_r )
{
	return 1; // 0=mouse connected, 1=no mouse
}


//-------------------------------------------------
//  z2_t1_r -
//-------------------------------------------------

READ8_MEMBER( abc99_device::z2_t1_r )
{
	return m_t1_z2;
}


//-------------------------------------------------
//  z5_p1_r -
//-------------------------------------------------

READ8_MEMBER( abc99_device::z5_p1_r )
{
	/*

	    bit     description

	    P10     XA
	    P11     XB
	    P12     YA
	    P13     YB
	    P14     LB
	    P15     MB
	    P16     RB
	    P17     input from host

	*/

	UINT8 data = 0;

	// mouse buttons
	data |= (m_mouseb->read() & 0x07) << 4;

	// serial input
	data |= m_si << 7;

	return data;
}


//-------------------------------------------------
//  z5_p2_w -
//-------------------------------------------------

WRITE8_MEMBER( abc99_device::z5_p2_w )
{
	/*

	    bit     description

	    P20
	    P21
	    P22
	    P23
	    P24     Z2 serial input enable
	    P25     Z2 RESET
	    P26     serial output
	    P27     Z2 T1

	*/

	// serial input enable
	int si_en = BIT(data, 4);

	if (m_si_en != si_en)
	{
		m_si_en = si_en;
		serial_input();
	}

	// Z2 reset
	int reset = BIT(data, 5);

	if (!m_reset && reset)
	{
		m_maincpu->reset();
	}

	m_reset = reset;

	// serial output
	m_so_z5 = BIT(data, 6);
	serial_output(m_so_z2 && m_so_z5);

	// keyboard CPU T1
	m_t1_z2 = BIT(data, 7);
}


//-------------------------------------------------
//  z5_t1_r -
//-------------------------------------------------

READ8_MEMBER( abc99_device::z5_t1_r )
{
	return m_t1_z5;
}
