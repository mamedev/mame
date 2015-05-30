// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Luxor ABC-55/77 keyboard emulation

*********************************************************************/

/*

PCB Layout
----------

KTC A65-02486-232

|-----------------------------------------------------------------------|
|   SW1       CN1       LS393                                           |
|   4020    7406    LS132       7407  LS02  7407        NE556    LS1    |
|                                                                       |
|   22-950-3B   XTAL    CPU     ROM0    ROM1    LS373  LS240  22-908-03 |
|                                                                       |
|                                                                       |
|                                                                       |
|                                                                       |
|                                                                       |
|                                                                       |
|                                                                       |
|-----------------------------------------------------------------------|

Notes:
    All IC's shown.

    CPU         - Signetics SCN8035A 8035 CPU
    ROM0        - NEC D2716D 2Kx8 ROM "-78"
    ROM1        - not populated
    22-950-3B   - Exar Semiconductor XR22-950-3B keyboard matrix row driver with 4 to 12 decoder/demultiplexer
    22-908-03   - Exar Semiconductor XR22-908-03 keyboard matrix capacitive readout latch
    CN1         - 1x12 PCB header
    LS1         - loudspeaker
    SW1         - reset switch

*/

#include "abc77.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define I8035_TAG       "z16"
#define DISCRETE_TAG    "discrete"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type ABC77 = &device_creator<abc77_device>;
const device_type ABC55 = &device_creator<abc55_device>;


//-------------------------------------------------
//  ROM( abc77 )
//-------------------------------------------------

ROM_START( abc77 )
	ROM_REGION( 0x1000, I8035_TAG, 0 )
	ROM_LOAD( "-78.z10", 0x000, 0x800, CRC(635986ce) SHA1(04a30141ac611d0544bbb786061515040c23480c) )
//  ROM_LOAD( "keyboard.z14", 0x0800, 0x0800, NO_DUMP ) // non-Swedish keyboard encoding ROM
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *abc77_device::device_rom_region() const
{
	return ROM_NAME( abc77 );
}


//-------------------------------------------------
//  ADDRESS_MAP( abc77_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( abc77_map, AS_PROGRAM, 8, abc77_device )
	AM_RANGE(0x000, 0xfff) AM_ROM AM_REGION("z16", 0)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( abc77_io )
//-------------------------------------------------

static ADDRESS_MAP_START( abc77_io, AS_IO, 8, abc77_device )
	AM_RANGE(0x00, 0x00) AM_MIRROR(0xff) AM_WRITE(j3_w)
	AM_RANGE(0x00, 0x00) AM_MIRROR(0xff) AM_READ_PORT("DSW")
	AM_RANGE(MCS48_PORT_P1, MCS48_PORT_P1) AM_READ(p1_r)
	AM_RANGE(MCS48_PORT_P2, MCS48_PORT_P2) AM_WRITE(p2_w)
	AM_RANGE(MCS48_PORT_T1, MCS48_PORT_T1) AM_READ(t1_r)
	AM_RANGE(MCS48_PORT_PROG, MCS48_PORT_PROG) AM_WRITE(prog_w)
ADDRESS_MAP_END


//-------------------------------------------------
//  DISCRETE_SOUND( abc77 )
//-------------------------------------------------

static const discrete_555_desc abc77_ne556_a =
{
	DISC_555_OUT_SQW | DISC_555_OUT_DC,
	5,      // B+ voltage of 555
	DEFAULT_555_VALUES
};


static DISCRETE_SOUND_START( abc77 )
	DISCRETE_INPUT_LOGIC(NODE_01)
	DISCRETE_555_ASTABLE(NODE_02, NODE_01, (int) RES_K(2.7), (int) RES_K(15), (int) CAP_N(22), &abc77_ne556_a)
	DISCRETE_OUTPUT(NODE_02, 5000)
DISCRETE_SOUND_END


//-------------------------------------------------
//  MACHINE_DRIVER( abc77 )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( abc77 )
	// keyboard cpu
	MCFG_CPU_ADD(I8035_TAG, I8035, XTAL_4_608MHz)
	MCFG_CPU_PROGRAM_MAP(abc77_map)
	MCFG_CPU_IO_MAP(abc77_io)

	// watchdog
	MCFG_WATCHDOG_TIME_INIT(attotime::from_hz(XTAL_4_608MHz/3/5/4096))

	// discrete sound
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(DISCRETE_TAG, DISCRETE, 0)
	MCFG_DISCRETE_INTF(abc77)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor abc77_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( abc77 );
}


//-------------------------------------------------
//  INPUT_CHANGED_MEMBER( keyboard_reset )
//-------------------------------------------------

INPUT_CHANGED_MEMBER( abc77_device::keyboard_reset )
{
	if (oldval && !newval)
	{
		device_reset();
	}
}


//-------------------------------------------------
//  INPUT_PORTS( abc55 )
//-------------------------------------------------

INPUT_PORTS_START( abc55 )
	PORT_START("X0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CAPS LOCK") PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CTRL") PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RIGHT SHIFT") PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("LEFT SHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)

	PORT_START("X1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH2) PORT_CHAR('<') PORT_CHAR('>')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RETURN") PORT_CODE(KEYCODE_ENTER) PORT_CHAR('\r')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("\xE2\x86\x90") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("\xE2\x86\x92") PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))

	PORT_START("X3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR(0x00E9) PORT_CHAR(0x00C9)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('+') PORT_CHAR('?')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR(0x00E5) PORT_CHAR(0x00C5)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(0x00FC) PORT_CHAR(0x00DC)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(0x00E4) PORT_CHAR(0x00C4)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\'') PORT_CHAR('*')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SPACE") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')

	PORT_START("X4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR('=')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON) PORT_CHAR(0x00F6) PORT_CHAR(0x00D6)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR(':')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('-') PORT_CHAR('_')

	PORT_START("X5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('/')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR(';')

	PORT_START("X6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')

	PORT_START("X7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("4 \xC2\xA4") PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR(0x00A4)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')

	PORT_START("X8")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')

	PORT_START("X9")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X10")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X11")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("PF1") PORT_CODE(KEYCODE_F1) PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("PF2") PORT_CODE(KEYCODE_F2) PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("PF3") PORT_CODE(KEYCODE_F3) PORT_CHAR(UCHAR_MAMEKEY(F3))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("PF4") PORT_CODE(KEYCODE_F4) PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("PF5") PORT_CODE(KEYCODE_F5) PORT_CHAR(UCHAR_MAMEKEY(F5))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("PF6") PORT_CODE(KEYCODE_F6) PORT_CHAR(UCHAR_MAMEKEY(F6))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("PF7") PORT_CODE(KEYCODE_F7) PORT_CHAR(UCHAR_MAMEKEY(F7))
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("PF8") PORT_CODE(KEYCODE_F8) PORT_CHAR(UCHAR_MAMEKEY(F8))

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, "Keyboard Program" )
	PORT_DIPSETTING(    0x00, "Internal (8048)" )
	PORT_DIPSETTING(    0x01, "External PROM" ) // @ Z10
	PORT_DIPNAME( 0x02, 0x02, "Character Set" )
	PORT_DIPSETTING(    0x02, "Swedish" )
	PORT_DIPSETTING(    0x00, "US ASCII" )
	PORT_DIPNAME( 0x04, 0x04, "External Encoding PROM" ) // @ Z14
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x18, "Keyboard Language" ) PORT_CONDITION("DSW", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(    0x00, "Danish" )
	PORT_DIPSETTING(    0x10, DEF_STR( French ) )
	PORT_DIPSETTING(    0x08, DEF_STR( German ) )
	PORT_DIPSETTING(    0x18, DEF_STR( Spanish ) )
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("SW1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_NAME("Keyboard Reset") PORT_CHANGED_MEMBER(DEVICE_SELF, abc77_device, keyboard_reset, 0)
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor abc55_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( abc55 );
}


//-------------------------------------------------
//  INPUT_PORTS( abc77 )
//-------------------------------------------------

INPUT_PORTS_START( abc77 )
	PORT_INCLUDE( abc55 )

	PORT_MODIFY("X9")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 9") PORT_CODE(KEYCODE_9_PAD) PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad +") PORT_CODE(KEYCODE_PLUS_PAD) PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 6") PORT_CODE(KEYCODE_6_PAD) PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad -") PORT_CODE(KEYCODE_MINUS_PAD) PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 3") PORT_CODE(KEYCODE_3_PAD) PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad RETURN") PORT_CODE(KEYCODE_ENTER_PAD) PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad .") PORT_CODE(KEYCODE_DEL_PAD) PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))

	PORT_MODIFY("X10")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 7") PORT_CODE(KEYCODE_7_PAD) PORT_CHAR(UCHAR_MAMEKEY(7_PAD))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 8") PORT_CODE(KEYCODE_8_PAD) PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 4") PORT_CODE(KEYCODE_4_PAD) PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 5") PORT_CODE(KEYCODE_5_PAD) PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 1") PORT_CODE(KEYCODE_1_PAD) PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 2") PORT_CODE(KEYCODE_2_PAD) PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 0") PORT_CODE(KEYCODE_0_PAD) PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor abc77_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( abc77 );
}



//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  serial_output -
//-------------------------------------------------

inline void abc77_device::serial_output(int state)
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

inline void abc77_device::serial_clock()
{
	m_clock = !m_clock;

	m_slot->trxc_w(!m_clock);
}


//-------------------------------------------------
//  keydown -
//-------------------------------------------------

inline void abc77_device::key_down(int state)
{
	if (m_keydown != state)
	{
		m_slot->keydown_w(state);
		m_keydown = state;
	}
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  abc77_device - constructor
//-------------------------------------------------

abc77_device::abc77_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, ABC77, "Luxor ABC 77", tag, owner, clock, "abc77", __FILE__),
	abc_keyboard_interface(mconfig, *this),
	m_maincpu(*this, I8035_TAG),
	m_discrete(*this, DISCRETE_TAG),
	m_x0(*this, "X0"),
	m_x1(*this, "X1"),
	m_x2(*this, "X2"),
	m_x3(*this, "X3"),
	m_x4(*this, "X4"),
	m_x5(*this, "X5"),
	m_x6(*this, "X6"),
	m_x7(*this, "X7"),
	m_x8(*this, "X8"),
	m_x9(*this, "X9"),
	m_x10(*this, "X10"),
	m_x11(*this, "X11"),
	m_dsw(*this, "DSW"),
	m_txd(1),
	m_keydown(1),
	m_clock(0),
	m_stb(1)
{
}

abc77_device::abc77_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source) :
	device_t(mconfig, type, name, tag, owner, clock, shortname, source),
	abc_keyboard_interface(mconfig, *this),
	m_maincpu(*this, I8035_TAG),
	m_discrete(*this, DISCRETE_TAG),
	m_x0(*this, "X0"),
	m_x1(*this, "X1"),
	m_x2(*this, "X2"),
	m_x3(*this, "X3"),
	m_x4(*this, "X4"),
	m_x5(*this, "X5"),
	m_x6(*this, "X6"),
	m_x7(*this, "X7"),
	m_x8(*this, "X8"),
	m_x9(*this, "X9"),
	m_x10(*this, "X10"),
	m_x11(*this, "X11"),
	m_dsw(*this, "DSW"),
	m_txd(1),
	m_keydown(1),
	m_clock(0),
	m_stb(1)
{
}

abc55_device::abc55_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	abc77_device(mconfig, ABC55, "Luxor ABC 55", tag, owner, clock, "abc55", __FILE__) { }


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void abc77_device::device_start()
{
	// allocate timers
	m_serial_timer = timer_alloc(TIMER_SERIAL);
	m_serial_timer->adjust(attotime::from_hz(19200), 0, attotime::from_hz(19200)); // ALE/32

	m_reset_timer = timer_alloc(TIMER_RESET);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void abc77_device::device_reset()
{
	int t = 1.1 * RES_K(100) * CAP_N(100) * 1000; // t = 1.1 * R1 * C1
	int ea = BIT(m_dsw->read(), 7);

	// trigger reset
	m_maincpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	m_reset_timer->adjust(attotime::from_msec(t));

	m_maincpu->set_input_line(MCS48_INPUT_EA, ea ? CLEAR_LINE : ASSERT_LINE);

	m_slot->write_rx(1);
}


//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void abc77_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_SERIAL:
		serial_clock();
		break;

	case TIMER_RESET:
		m_maincpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
		break;
	}
}


//-------------------------------------------------
//  txd_w -
//-------------------------------------------------

void abc77_device::txd_w(int state)
{
	m_maincpu->set_input_line(MCS48_INPUT_IRQ, state ? CLEAR_LINE : ASSERT_LINE);
}


//-------------------------------------------------
//  p1_r -
//-------------------------------------------------

READ8_MEMBER( abc77_device::p1_r )
{
	/*

	    bit     description

	    P10     Z17 Y0
	    P11     Z17 Y1
	    P12     Z17 Y2
	    P13     Z17 Y3
	    P14     Z17 Y4
	    P15     Z17 Y5
	    P16     Z17 Y6
	    P17     Z17 Y7

	*/

	UINT8 data = 0xff;

	if (m_stb)
	{
		switch (m_keylatch)
		{
		case 0: data = m_x0->read(); break;
		case 1: data = m_x1->read(); break;
		case 2: data = m_x2->read(); break;
		case 3: data = m_x3->read(); break;
		case 4: data = m_x4->read(); break;
		case 5: data = m_x5->read(); break;
		case 6: data = m_x6->read(); break;
		case 7: data = m_x7->read(); break;
		case 8: data = m_x8->read(); break;
		case 9: data = m_x9->read(); break;
		case 10: data = m_x10->read(); break;
		case 11: data = m_x11->read(); break;
		}
	}

	return data;
}


//-------------------------------------------------
//  p2_w -
//-------------------------------------------------

WRITE8_MEMBER( abc77_device::p2_w )
{
	/*

	    bit     description

	    P20     Z2 A0
	    P21     Z2 A1
	    P22     Z2 A2
	    P23     Z2 A3
	    P24     NE556 2,6
	    P25     TxD
	    P26     _KEYDOWN
	    P27     Z17 HYS

	*/

	if (!m_stb)
	{
		m_keylatch = data & 0x0f;

		if (m_keylatch == 1)
		{
			machine().watchdog_reset();
		}
	}

	// beep
	m_discrete->write(space, NODE_01, BIT(data, 4));

	// transmit data
	serial_output(BIT(data, 5));

	// key down
	key_down(BIT(data, 6));

	// hysteresis
	m_hys = BIT(data, 7);
}


//-------------------------------------------------
//  t1_r -
//-------------------------------------------------

READ8_MEMBER( abc77_device::t1_r )
{
	return m_clock;
}


//-------------------------------------------------
//  prog_w -
//-------------------------------------------------

WRITE8_MEMBER( abc77_device::prog_w )
{
	m_stb = BIT(data, 0);
}


//-------------------------------------------------
//  j3_w -
//-------------------------------------------------

WRITE8_MEMBER( abc77_device::j3_w )
{
	m_j3 = data;
}
