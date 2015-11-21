// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Luxor ABC-800 keyboard emulation

**********************************************************************/

/*

PCB Layout
----------

KTC A65-02201-201K

    |---------------|
|---|      CN1      |---------------------------------------------------|
|                                                                       |
|       LS193       LS374                       8048        22-008-03   |
|   LS13    LS193               22-050-3B   5.9904MHz                   |
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
    All IC's shown.

    8048        - National Semiconductor INS8048 MCU "8048-132"
    22-008-03   - Exar Semiconductor XR22-008-03 keyboard matrix capacitive readout latch
    22-050-3B   - Exar Semiconductor XR22-050-3B keyboard matrix row driver with 4 to 12 decoder/demultiplexer
    CN1         - keyboard data connector


XR22-008-03 Pinout
------------------
            _____   _____
    D0   1 |*    \_/     | 20  Vcc
    D1   2 |             | 19  _CLR
    D2   3 |             | 18  Q0
    D3   4 |             | 17  Q1
   HYS   5 |  22-008-03  | 16  Q2
    D4   6 |             | 15  Q3
    D5   7 |             | 14  Q4
    D6   8 |             | 13  Q5
    D7   9 |             | 12  Q6
   GND  10 |_____________| 11  Q7


XR22-050-3B Pinout
------------------
            _____   _____
    Y8   1 |*    \_/     | 20  Vcc
    Y9   2 |             | 19  Y7
   Y10   3 |             | 18  Y6
   Y11   4 |             | 17  Y5
  _STB   5 |  22-050-3B  | 16  Y4
    A0   6 |             | 15  Y3
    A1   7 |             | 14  Y2
    A2   8 |             | 13  Y1
    A3   9 |             | 12  Y0
   GND  10 |_____________| 11  OE?

*/

#include "abc800kb.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define I8048_TAG       "i8048"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type ABC800_KEYBOARD = &device_creator<abc800_keyboard_device>;


//-------------------------------------------------
//  ROM( abc800_keyboard )
//-------------------------------------------------

ROM_START( abc800_keyboard )
	ROM_REGION( 0x400, I8048_TAG, 0 )
	ROM_LOAD( "8048-132.z9", 0x0000, 0x0400, CRC(05c4dce5) SHA1(1824c5d304bbd09f97056cfa408e1b18b5219ba2) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *abc800_keyboard_device::device_rom_region() const
{
	return ROM_NAME( abc800_keyboard );
}


//-------------------------------------------------
//  ADDRESS_MAP( abc800_keyboard_io )
//-------------------------------------------------

static ADDRESS_MAP_START( abc800_keyboard_io, AS_IO, 8, abc800_keyboard_device )
	AM_RANGE(MCS48_PORT_P1, MCS48_PORT_P1) AM_READWRITE(kb_p1_r, kb_p1_w)
	AM_RANGE(MCS48_PORT_P2, MCS48_PORT_P2) AM_WRITE(kb_p2_w)
	AM_RANGE(MCS48_PORT_T1, MCS48_PORT_T1) AM_READ(kb_t1_r)
ADDRESS_MAP_END


//-------------------------------------------------
//  MACHINE_DRIVER( abc800_keyboard )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( abc800_keyboard )
	MCFG_CPU_ADD(I8048_TAG, I8048, XTAL_5_9904MHz)
	MCFG_CPU_IO_MAP(abc800_keyboard_io)
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor abc800_keyboard_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( abc800_keyboard );
}


//-------------------------------------------------
//  INPUT_PORTS( abc800_keyboard )
//-------------------------------------------------

INPUT_PORTS_START( abc800_keyboard )
	PORT_START("X0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CAPS LOCK") PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CTRL") PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Left SHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED ) // 80
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED ) // 81
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED ) // 82
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED ) // 83

	PORT_START("X1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad -") PORT_CODE(KEYCODE_MINUS_PAD) PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad RETURN") PORT_CODE(KEYCODE_ENTER_PAD) PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED ) // 84
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED ) // 85

	PORT_START("X2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED ) // 18
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 6") PORT_CODE(KEYCODE_6_PAD) PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 3") PORT_CODE(KEYCODE_3_PAD) PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED ) // 86

	PORT_START("X3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("4 \xC2\xA4") PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR(0x00A4)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 9") PORT_CODE(KEYCODE_9_PAD) PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 5") PORT_CODE(KEYCODE_5_PAD) PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 2") PORT_CODE(KEYCODE_2_PAD) PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SPACE") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')

	PORT_START("X4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 8") PORT_CODE(KEYCODE_8_PAD) PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 4") PORT_CODE(KEYCODE_4_PAD) PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 1") PORT_CODE(KEYCODE_1_PAD) PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad .") PORT_CODE(KEYCODE_DEL_PAD) PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))

	PORT_START("X5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 7") PORT_CODE(KEYCODE_7_PAD) PORT_CHAR(UCHAR_MAMEKEY(7_PAD))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("PF4") PORT_CODE(KEYCODE_F4) PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("PF6") PORT_CODE(KEYCODE_F6) PORT_CHAR(UCHAR_MAMEKEY(F6))
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 0") PORT_CODE(KEYCODE_0_PAD) PORT_CHAR(UCHAR_MAMEKEY(0_PAD))

	PORT_START("X6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('/')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("PF2") PORT_CODE(KEYCODE_F2) PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("PF3") PORT_CODE(KEYCODE_F3) PORT_CHAR(UCHAR_MAMEKEY(F3))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("PF5") PORT_CODE(KEYCODE_F5) PORT_CHAR(UCHAR_MAMEKEY(F5))
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("PF8") PORT_CODE(KEYCODE_F8) PORT_CHAR(UCHAR_MAMEKEY(F8))

	PORT_START("X7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("PF1") PORT_CODE(KEYCODE_F1) PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RETURN") PORT_CODE(KEYCODE_ENTER) PORT_CHAR('\r')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_LEFT) PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("PF7") PORT_CODE(KEYCODE_F7) PORT_CHAR(UCHAR_MAMEKEY(F7))

	PORT_START("X8")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR(';')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('<') PORT_CHAR('>')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(u_UMLAUT " " U_UMLAUT) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(0x00FC) PORT_CHAR(0x00DC)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH2) PORT_CHAR('\'') PORT_CHAR('*')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_RIGHT) PORT_CODE(KEYCODE_TAB) PORT_CHAR(UCHAR_MAMEKEY(TAB))

	PORT_START("X9")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR('=')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR(':')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(e_ACUTE " " E_ACUTE) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR(0x00E9) PORT_CHAR(0x00C9)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(a_RING " " A_RING) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR(0x00E5) PORT_CHAR(0x00C5)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(a_UMLAUT " " A_UMLAUT) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(0x00E4) PORT_CHAR(0x00C4)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Right SHIFT") PORT_CODE(KEYCODE_RSHIFT)

	PORT_START("X10")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('+') PORT_CHAR('?')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(o_UMLAUT " " O_UMLAUT) PORT_CODE(KEYCODE_COLON) PORT_CHAR(0x00F6) PORT_CHAR(0x00D6)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED ) // 87
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED ) // 88
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED ) // 89
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED ) // 8a

	PORT_START("X11")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED ) // 03
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED ) // 23
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED ) // 96
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED ) // 8b
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED ) // 8c
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED ) // 8d
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED ) // 8e
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED ) // 8f
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor abc800_keyboard_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( abc800_keyboard );
}



//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  serial_output -
//-------------------------------------------------

inline void abc800_keyboard_device::serial_output(int state)
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

inline void abc800_keyboard_device::serial_clock()
{
	m_clk = !m_clk;

	m_slot->trxc_w(!m_clk);
}


//-------------------------------------------------
//  keydown -
//-------------------------------------------------

inline void abc800_keyboard_device::key_down(int state)
{
	if (m_keydown != state)
	{
		m_keydown = state;

		m_slot->keydown_w(state);
	}
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  abc800_keyboard_device - constructor
//-------------------------------------------------

abc800_keyboard_device::abc800_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, ABC800_KEYBOARD, "ABC-800 Keyboard", tag, owner, clock, "abc800kb", __FILE__),
		abc_keyboard_interface(mconfig, *this),
		m_maincpu(*this, I8048_TAG),
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
		m_row(0),
		m_txd(1),
		m_clk(0),
		m_stb(1),
		m_keydown(1), m_serial_timer(nullptr)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void abc800_keyboard_device::device_start()
{
	// allocate timers
	m_serial_timer = timer_alloc();
	m_serial_timer->adjust(attotime::from_hz(XTAL_5_9904MHz/(3*5)/20), 0, attotime::from_hz(XTAL_5_9904MHz/(3*5)/20)); // ???

	// state saving
	save_item(NAME(m_row));
	save_item(NAME(m_clk));
	save_item(NAME(m_txd));
	save_item(NAME(m_stb));
	save_item(NAME(m_keydown));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void abc800_keyboard_device::device_reset()
{
}


//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void abc800_keyboard_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	serial_clock();
}


//-------------------------------------------------
//  txd_w -
//-------------------------------------------------

void abc800_keyboard_device::txd_w(int state)
{
	m_maincpu->set_input_line(MCS48_INPUT_IRQ, state ? CLEAR_LINE : ASSERT_LINE);
}


//-------------------------------------------------
//  kb_p1_r - keyboard column data read
//-------------------------------------------------

READ8_MEMBER( abc800_keyboard_device::kb_p1_r )
{
	UINT8 data = 0xff;

	if (m_stb)
	{
		switch (m_row)
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
//  kb_p1_w - keyboard row write
//-------------------------------------------------

WRITE8_MEMBER( abc800_keyboard_device::kb_p1_w )
{
	/*

	    bit     description

	    0       A0
	    1       A1
	    2       A2
	    3       A3
	    4       ?
	    5       ?
	    6
	    7

	*/

	// keyboard row
	if (!m_stb)
	{
		m_row = data & 0x0f;
	}
}


//-------------------------------------------------
//  kb_p2_w - keyboard control write
//-------------------------------------------------

WRITE8_MEMBER( abc800_keyboard_device::kb_p2_w )
{
	/*

	    bit     description

	    0
	    1
	    2
	    3
	    4       TxD
	    5       ?
	    6       22-008-03 CLR, 22-050-3B STB
	    7       22-008-03 HYS

	*/

	// TxD
	serial_output(!BIT(data, 4));

	// keydown
	key_down(!BIT(data, 5));

	// strobe
	m_stb = BIT(data, 6);
}


//-------------------------------------------------
//  kb_t1_r - keyboard T1 timer read
//-------------------------------------------------

READ8_MEMBER( abc800_keyboard_device::kb_t1_r )
{
	return m_clk;
}
