// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    IBM Model F PC/XT 83-key keyboard emulation

*********************************************************************/

/*

    Part No     Layout
    -------------------
    1501100     US
    1501102     Germany
    1501105     UK

*/

#include "pcxt83.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define I8048_TAG       "i8048"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type PC_KBD_IBM_PC_XT_83 = &device_creator<ibm_pc_xt_83_keyboard_device>;


//-------------------------------------------------
//  ROM( ibm_pc_xt_83_keyboard )
//-------------------------------------------------

ROM_START( ibm_pc_xt_83_keyboard )
	ROM_REGION( 0x400, I8048_TAG, 0 )
	/*
	Keyboard Part No. 1501105

	MOI 74 01
	PN 4584751
	GX 344231

	i 4429745
	ZO P 379297
	8143 P
	(C) INTEL 76
	*/
	ROM_LOAD( "4584751.m1", 0x000, 0x400, CRC(c59aa9d1) SHA1(4f5b2a075c68f6493310ec1e2a24271ceea330df) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *ibm_pc_xt_83_keyboard_device::device_rom_region() const
{
	return ROM_NAME( ibm_pc_xt_83_keyboard );
}


//-------------------------------------------------
//  ADDRESS_MAP( kb_io )
//-------------------------------------------------

static ADDRESS_MAP_START( ibm_pc_xt_83_keyboard_io, AS_IO, 8, ibm_pc_xt_83_keyboard_device )
	AM_RANGE(MCS48_PORT_BUS, MCS48_PORT_BUS) AM_READWRITE(bus_r, bus_w)
	AM_RANGE(MCS48_PORT_P1, MCS48_PORT_P1) AM_WRITE(p1_w)
	AM_RANGE(MCS48_PORT_P2, MCS48_PORT_P2) AM_WRITE(p2_w)
	AM_RANGE(MCS48_PORT_T0, MCS48_PORT_T0) AM_READ(t0_r)
	AM_RANGE(MCS48_PORT_T1, MCS48_PORT_T1) AM_READ(t1_r)
ADDRESS_MAP_END


//-------------------------------------------------
//  MACHINE_DRIVER( ibm_pc_xt_83_keyboard )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( ibm_pc_xt_83_keyboard )
	MCFG_CPU_ADD(I8048_TAG, I8048, MCS48_LC_CLOCK(IND_U(47), CAP_P(20.7)))
	MCFG_CPU_IO_MAP(ibm_pc_xt_83_keyboard_io)
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor ibm_pc_xt_83_keyboard_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( ibm_pc_xt_83_keyboard );
}


//-------------------------------------------------
//  INPUT_PORTS( ibm_pc_xt_83_keyboard )
//-------------------------------------------------

INPUT_PORTS_START( ibm_pc_xt_83_keyboard )
	PORT_START("MD00")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_PLUS_PAD) PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 3 PgDn") PORT_CODE(KEYCODE_3_PAD) PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 6 " UTF8_RIGHT) PORT_CODE(KEYCODE_6_PAD) PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS_PAD) PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 9 PgUp") PORT_CODE(KEYCODE_9_PAD) PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Scroll Lock Break") PORT_CODE(KEYCODE_SCRLOCK) PORT_CHAR(UCHAR_MAMEKEY(SCRLOCK))

	PORT_START("MD01")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad . Del") PORT_CODE(KEYCODE_DEL_PAD) PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 2 " UTF8_DOWN) PORT_CODE(KEYCODE_2_PAD) PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 1 End") PORT_CODE(KEYCODE_1_PAD) PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5_PAD) PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 4 " UTF8_LEFT) PORT_CODE(KEYCODE_4_PAD) PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 7 Home") PORT_CODE(KEYCODE_7_PAD) PORT_CHAR(UCHAR_MAMEKEY(7_PAD))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 8 " UTF8_UP) PORT_CODE(KEYCODE_8_PAD) PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_NUMLOCK) PORT_CHAR(UCHAR_MAMEKEY(NUMLOCK))

	PORT_START("MD02")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 0 Ins") PORT_CODE(KEYCODE_0_PAD) PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad * PrtSc") PORT_CODE(KEYCODE_ASTERISK) PORT_CHAR(UCHAR_MAMEKEY(ASTERISK))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_TILDE) PORT_CHAR('`') PORT_CHAR('~')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)

	PORT_START("MD03")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('\'') PORT_CHAR('"')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=') PORT_CHAR('+')

	PORT_START("MD04")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR(')')

	PORT_START("MD05")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR('(')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('*')

	PORT_START("MD06")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('&')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('^')

	PORT_START("MD07")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')

	PORT_START("MD08")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('@')

	PORT_START("MD09")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_TAB) PORT_CHAR(UCHAR_MAMEKEY(TAB))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ESC) PORT_CHAR(UCHAR_MAMEKEY(ESC))

	PORT_START("MD10")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LALT) PORT_CHAR(UCHAR_MAMEKEY(LALT))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F10) PORT_CHAR(UCHAR_MAMEKEY(F10))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F8) PORT_CHAR(UCHAR_MAMEKEY(F8))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F6) PORT_CHAR(UCHAR_MAMEKEY(F6))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F4) PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F2) PORT_CHAR(UCHAR_MAMEKEY(F2))

	PORT_START("MD11")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F9) PORT_CHAR(UCHAR_MAMEKEY(F9))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F7) PORT_CHAR(UCHAR_MAMEKEY(F7))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F5) PORT_CHAR(UCHAR_MAMEKEY(F5))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F3) PORT_CHAR(UCHAR_MAMEKEY(F3))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SPECIAL ) // 76 "Clear"
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F1) PORT_CHAR(UCHAR_MAMEKEY(F1))
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor ibm_pc_xt_83_keyboard_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( ibm_pc_xt_83_keyboard );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  ibm_pc_xt_83_keyboard_device - constructor
//-------------------------------------------------

ibm_pc_xt_83_keyboard_device::ibm_pc_xt_83_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, PC_KBD_IBM_PC_XT_83, "IBM PC/XT Keyboard", tag, owner, clock, "kb_pcxt83", __FILE__),
		device_pc_kbd_interface(mconfig, *this),
		m_maincpu(*this, I8048_TAG),
		m_md00(*this, "MD00"),
		m_md01(*this, "MD01"),
		m_md02(*this, "MD02"),
		m_md03(*this, "MD03"),
		m_md04(*this, "MD04"),
		m_md05(*this, "MD05"),
		m_md06(*this, "MD06"),
		m_md07(*this, "MD07"),
		m_md08(*this, "MD08"),
		m_md09(*this, "MD09"),
		m_md10(*this, "MD10"),
		m_md11(*this, "MD11"),
		m_bus(0xff),
		m_p1(0xff),
		m_p2(0xff),
		m_sense(0),
		m_q(1)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ibm_pc_xt_83_keyboard_device::device_start()
{
	set_pc_kbdc_device();

	// state saving
	save_item(NAME(m_bus));
	save_item(NAME(m_p1));
	save_item(NAME(m_p2));
	save_item(NAME(m_sense));
	save_item(NAME(m_q));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void ibm_pc_xt_83_keyboard_device::device_reset()
{
}


//-------------------------------------------------
//  clock_write -
//-------------------------------------------------

WRITE_LINE_MEMBER( ibm_pc_xt_83_keyboard_device::clock_write )
{
}


//-------------------------------------------------
//  data_write -
//-------------------------------------------------

WRITE_LINE_MEMBER( ibm_pc_xt_83_keyboard_device::data_write )
{
	m_maincpu->set_input_line(MCS48_INPUT_IRQ, state ? CLEAR_LINE : ASSERT_LINE);
}


//-------------------------------------------------
//  bus_r -
//-------------------------------------------------

READ8_MEMBER( ibm_pc_xt_83_keyboard_device::bus_r )
{
	// HACK this should be handled in mcs48.c
	return m_bus;
}


//-------------------------------------------------
//  bus_w -
//-------------------------------------------------

WRITE8_MEMBER( ibm_pc_xt_83_keyboard_device::bus_w )
{
	/*

	    bit     description

	    0
	    1
	    2
	    3
	    4
	    5       DATAOUT
	    6       -REQOUT
	    7       Z1 RESET

	*/

	m_pc_kbdc->data_write_from_kb(BIT(data, 5));
	m_pc_kbdc->clock_write_from_kb(BIT(data, 6));

	if (!BIT(m_bus, 7) && BIT(data, 7))
	{
		UINT8 data = 0xff;

		if (BIT(m_p1, 0)) data &= m_md00->read();
		if (BIT(m_p1, 1)) data &= m_md01->read();
		if (BIT(m_p1, 2)) data &= m_md02->read();
		if (BIT(m_p1, 3)) data &= m_md03->read();
		if (BIT(m_p1, 4)) data &= m_md04->read();
		if (BIT(m_p1, 5)) data &= m_md05->read();
		if (BIT(m_p1, 6)) data &= m_md06->read();
		if (BIT(m_p1, 7)) data &= m_md07->read();
		if (BIT(m_p2, 4)) data &= m_md08->read();
		if (BIT(m_p2, 5)) data &= m_md09->read();
		if (BIT(m_p2, 6)) data &= m_md10->read();
		if (BIT(m_p2, 7)) data &= m_md11->read();

		m_q = BIT(data, m_sense);
	}

	m_bus = data;
}


//-------------------------------------------------
//  p1_w -
//-------------------------------------------------

WRITE8_MEMBER( ibm_pc_xt_83_keyboard_device::p1_w )
{
	/*

	    bit     description

	    0       MD00
	    1       MD01
	    2       MD02
	    3       MD03
	    4       MD04
	    5       MD05
	    6       MD06
	    7       MD07

	*/

	m_p1 = data;
}


//-------------------------------------------------
//  p2_w -
//-------------------------------------------------

WRITE8_MEMBER( ibm_pc_xt_83_keyboard_device::p2_w )
{
	/*

	    bit     description

	    0       SELECT 2
	    1       SELECT 1
	    2       SELECT 0
	    3       SA CLOSED
	    4       MD08
	    5       MD09
	    6       MD10
	    7       MD11

	*/

	if (!BIT(m_p2, 3) && BIT(data, 3))
	{
		m_sense = data & 0x07;
	}

	m_p2 = data;
}


//-------------------------------------------------
//  t0_r -
//-------------------------------------------------

READ8_MEMBER( ibm_pc_xt_83_keyboard_device::t0_r )
{
	return clock_signal();
}


//-------------------------------------------------
//  t1_r -
//-------------------------------------------------

READ8_MEMBER( ibm_pc_xt_83_keyboard_device::t1_r )
{
	return BIT(m_p2, 3) && m_q;
}
