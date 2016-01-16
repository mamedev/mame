// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    Amiga Keyboard

    We currently emulate the Amiga 500 keyboard controller, which was
    also used in later Amiga 2000 keyboards.

    TODO: - Natural keyboard mode doesn't work with shifted characters,
            they get sent in the wrong order (core bug?)
          - Move 6500/1 to its own CPU core so that it can be shared with
            other systems
          - Add support for more keyboard controllers (pending on them
            getting dumped)

    Amiga 1000 keyboard part numbers (manufactured by Mitsumi):

    - 327063-01  R56-2144  English
    - 327063-02            British
    - 327063-03  R56-2153  German
    - 327063-04  R56-2152  French
    - 327063-05  R56-2154  Italian

***************************************************************************/

#include "amigakbd.h"
#include "machine/rescap.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type AMIGAKBD = &device_creator<amigakbd_device>;

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

static ADDRESS_MAP_START( mpu6500_map, AS_PROGRAM, 8, amigakbd_device )
	ADDRESS_MAP_GLOBAL_MASK(0xfff)
	AM_RANGE(0x000, 0x03f) AM_RAM
	AM_RANGE(0x080, 0x080) AM_READWRITE(port_a_r, port_a_w)
	AM_RANGE(0x081, 0x081) AM_READ_PORT("special") AM_WRITE(port_b_w)
	AM_RANGE(0x082, 0x082) AM_WRITE(port_c_w)
	AM_RANGE(0x083, 0x083) AM_WRITE(port_d_w)
	AM_RANGE(0x084, 0x085) AM_WRITE(latch_w)
	AM_RANGE(0x086, 0x087) AM_READ(counter_r)
	AM_RANGE(0x088, 0x088) AM_WRITE(transfer_latch_w)
	AM_RANGE(0x089, 0x089) AM_WRITE(clear_pa0_detect)
	AM_RANGE(0x08a, 0x08a) AM_WRITE(clear_pa1_detect)
	AM_RANGE(0x08f, 0x08f) AM_READWRITE(control_r, control_w)
	AM_RANGE(0x090, 0x0ff) AM_NOP
	AM_RANGE(0x800, 0xfff) AM_ROM AM_REGION("mos6570_036", 0)
ADDRESS_MAP_END

static MACHINE_CONFIG_FRAGMENT( a500_keyboard )
	MCFG_CPU_ADD("mos6570_036", M6502, XTAL_3MHz / 2)
	MCFG_CPU_PROGRAM_MAP(mpu6500_map)
MACHINE_CONFIG_END

machine_config_constructor amigakbd_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( a500_keyboard );
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

ROM_START( mos6570_036 )
	ROM_REGION(0x800, "mos6570_036", 0)
	ROM_LOAD("328191-02.ic1", 0x000, 0x800, CRC(4a3fc332) SHA1(83b21d0c8b93fc9b9b3b287fde4ec8f3badac5a2))
ROM_END

const rom_entry *amigakbd_device::device_rom_region() const
{
	return ROM_NAME( mos6570_036 );
}

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

static INPUT_PORTS_START( a500_us_keyboard )
	PORT_START("special")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LWIN)      PORT_CHAR(UCHAR_MAMEKEY(LWIN))      PORT_NAME("Left Amiga")  PORT_CHANGED_MEMBER(DEVICE_SELF, amigakbd_device, check_reset, NULL)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LALT)      PORT_CHAR(UCHAR_MAMEKEY(LALT))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT)    PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL)  PORT_CHAR(UCHAR_SHIFT_2)            PORT_NAME("Ctrl")        PORT_CHANGED_MEMBER(DEVICE_SELF, amigakbd_device, check_reset, NULL)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RWIN)      PORT_CHAR(UCHAR_MAMEKEY(RWIN))      PORT_NAME("Right Amiga") PORT_CHANGED_MEMBER(DEVICE_SELF, amigakbd_device, check_reset, NULL)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RALT)      PORT_CHAR(UCHAR_MAMEKEY(RALT))
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RSHIFT)    PORT_CHAR(UCHAR_SHIFT_1)

	PORT_START("row_d6")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ASTERISK)   PORT_CHAR(UCHAR_MAMEKEY(ASTERISK))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_END)                                           PORT_NAME("Unused")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CAPSLOCK)   PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK)) PORT_NAME("Caps Lock")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB)        PORT_CHAR(9)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE)      PORT_CHAR('`') PORT_CHAR('~')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC)        PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("row_d5")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_PLUS_PAD)   PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)          PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)          PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)          PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)          PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_HOME)                                          PORT_NAME("(")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("row_d4")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9_PAD)      PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)          PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)          PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)          PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)          PORT_CHAR('2') PORT_CHAR('@')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F1)         PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("row_d3")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6_PAD)      PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)          PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)          PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)          PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)          PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F2)         PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("row_d2")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3_PAD)      PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)          PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)          PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)          PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)          PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F3)         PORT_CHAR(UCHAR_MAMEKEY(F3))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("row_d1")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL_PAD)    PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)          PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)          PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)          PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)          PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F4)         PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("row_d0")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8_PAD)      PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)          PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)          PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)          PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)          PORT_CHAR('6') PORT_CHAR('^')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F5)         PORT_CHAR(UCHAR_MAMEKEY(F5))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("row_c7")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5_PAD)      PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)          PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)          PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)          PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)          PORT_CHAR('7') PORT_CHAR('&')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_PGUP)                                          PORT_NAME(")")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("row_c6")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2_PAD)      PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)      PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)          PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)          PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)          PORT_CHAR('8') PORT_CHAR('*')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F6)         PORT_CHAR(UCHAR_MAMEKEY(F6))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("row_c5")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER_PAD)  PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)       PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)          PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)          PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)          PORT_CHAR('9') PORT_CHAR('(')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH_PAD)  PORT_CHAR('/')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("row_c4")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7_PAD)      PORT_CHAR('7') PORT_CHAR('&')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)      PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)      PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)          PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)          PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F7)         PORT_CHAR(UCHAR_MAMEKEY(F7))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("row_c3")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4_PAD)      PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)      PORT_CHAR('\'') PORT_CHAR('"')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR('[')  PORT_CHAR('{')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)      PORT_CHAR('-')  PORT_CHAR('_')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F8)         PORT_CHAR(UCHAR_MAMEKEY(F8))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("row_c2")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1_PAD)      PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)      PORT_CHAR(' ')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_PGDN)                                          PORT_NAME("Unused")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR('=') PORT_CHAR('+')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F9)         PORT_CHAR(UCHAR_MAMEKEY(F9))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("row_c1")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0_PAD)     PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL)       PORT_CHAR(UCHAR_MAMEKEY(DEL))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER)     PORT_CHAR(13)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F10)       PORT_CHAR(UCHAR_MAMEKEY(F10))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("row_c0")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS_PAD) PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)      PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT)     PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT)      PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)        PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F11)                                           PORT_NAME("Help")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END

ioport_constructor amigakbd_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( a500_us_keyboard );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  amigakbd_device - constructor
//-------------------------------------------------

amigakbd_device::amigakbd_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, AMIGAKBD, "Amiga 500 Keyboard with 6570-036 MPU", tag, owner, clock, "amigakbd", __FILE__),
	m_write_kclk(*this),
	m_write_kdat(*this),
	m_write_krst(*this),
	m_mpu(*this, "mos6570_036"),
	m_special(*this, "special"),
	m_row_d6(*this, "row_d6"),
	m_row_d5(*this, "row_d5"),
	m_row_d4(*this, "row_d4"),
	m_row_d3(*this, "row_d3"),
	m_row_d2(*this, "row_d2"),
	m_row_d1(*this, "row_d1"),
	m_row_d0(*this, "row_d0"),
	m_row_c7(*this, "row_c7"),
	m_row_c6(*this, "row_c6"),
	m_row_c5(*this, "row_c5"),
	m_row_c4(*this, "row_c4"),
	m_row_c3(*this, "row_c3"),
	m_row_c2(*this, "row_c2"),
	m_row_c1(*this, "row_c1"),
	m_row_c0(*this, "row_c0"),
	m_timer(nullptr),
	m_watchdog(nullptr),
	m_reset(nullptr),
	m_kdat(1),
	m_kclk(1),
	m_port_c(0xff),
	m_port_d(0xff),
	m_latch(0xffff),
	m_counter(0xffff),
	m_control(0x00)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void amigakbd_device::device_start()
{
	// resolve callbacks
	m_write_kclk.resolve_safe();
	m_write_kdat.resolve_safe();
	m_write_krst.resolve_safe();

	// allocate timers
	m_timer = timer_alloc(0, nullptr);
	m_watchdog = timer_alloc(1, nullptr);
	m_reset = timer_alloc(2, nullptr);

	// register for save states
	save_item(NAME(m_kdat));
	save_item(NAME(m_kclk));
	save_item(NAME(m_port_c));
	save_item(NAME(m_port_d));
	save_item(NAME(m_latch));
	save_item(NAME(m_counter));
	save_item(NAME(m_control));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void amigakbd_device::device_reset()
{
	// stack starts 0
	m_mpu->set_state_int(M6502_S, 0);

	m_kdat = 1;
	m_kclk = 1;
	m_port_c = 0xff;
	m_port_d = 0xff;
	m_latch = 0xffff;   // not initialized by hardware
	m_counter = 0xffff; // not initialized by hardware
	m_control = 0x00;

	m_timer->adjust(attotime::zero, 0, attotime::from_hz(XTAL_3MHz / 2));
	m_watchdog->adjust(attotime::from_msec(54));
}

void amigakbd_device::device_timer(emu_timer &timer, device_timer_id tid, int param, void *ptr)
{
	switch (tid)
	{
	// 6500/1 internal timer
	case 0:
		switch (m_control & 0x03)
		{
		// interval timer, pulse width measurement (connected to gnd here)
		case 0:
		case 3:
			if (m_counter-- == 0)
			{
				// counter overflow
				m_control |= COUNTER_OVERFLOW;
				m_counter = m_latch;

				// generate interrupt?
				update_irqs();
			}
			break;

		// pulse generator
		case 1:
			break;

		// event counter
		case 2:
			break;
		}
		break;

	// watchdog
	case 1:
		m_mpu->reset();
		m_watchdog->adjust(attotime::from_msec(54));
		break;

	// keyboard reset timer
	case 2:
		m_write_krst(1);
		break;
	}
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

INPUT_CHANGED_MEMBER( amigakbd_device::check_reset )
{
	UINT8 keys = m_special->read();

	// ctrl-amiga-amiga pressed?
	if (!BIT(keys, 6) && !BIT(keys, 3) && !BIT(keys, 2))
	{
		m_write_krst(0);
		m_reset->adjust(PERIOD_OF_555_MONOSTABLE(RES_K(47), CAP_U(10)));
	}
}

void amigakbd_device::update_irqs()
{
	if ((m_control & PA1_INT_ENABLED) && (m_control & PA1_NEGATIVE_EDGE))
		m_mpu->set_input_line(M6502_IRQ_LINE, ASSERT_LINE);

	else if ((m_control & PA0_INT_ENABLED) && (m_control & PA0_POSITIVE_EDGE))
		m_mpu->set_input_line(M6502_IRQ_LINE, ASSERT_LINE);

	else if ((m_control & COUNTER_INT_ENABLED) && (m_control & COUNTER_OVERFLOW))
		m_mpu->set_input_line(M6502_IRQ_LINE, ASSERT_LINE);

	else
		m_mpu->set_input_line(M6502_IRQ_LINE, CLEAR_LINE);
}

READ8_MEMBER( amigakbd_device::port_a_r )
{
	UINT8 data = 0xfc;

	// kdat & kclk
	data |= m_kdat << 0;
	data |= m_kclk << 1;

	// port d rows
	if (!BIT(m_port_d, 6)) data &= m_row_d6->read();
	if (!BIT(m_port_d, 5)) data &= m_row_d5->read();
	if (!BIT(m_port_d, 4)) data &= m_row_d4->read();
	if (!BIT(m_port_d, 3)) data &= m_row_d3->read();
	if (!BIT(m_port_d, 2)) data &= m_row_d2->read();
	if (!BIT(m_port_d, 1)) data &= m_row_d1->read();
	if (!BIT(m_port_d, 0)) data &= m_row_d0->read();

	// port c rows
	if (!BIT(m_port_c, 7)) data &= m_row_c7->read();
	if (!BIT(m_port_c, 6)) data &= m_row_c6->read();
	if (!BIT(m_port_c, 5)) data &= m_row_c5->read();
	if (!BIT(m_port_c, 4)) data &= m_row_c4->read();
	if (!BIT(m_port_c, 3)) data &= m_row_c3->read();
	if (!BIT(m_port_c, 2)) data &= m_row_c2->read();
	if (!BIT(m_port_c, 1)) data &= m_row_c1->read();
	if (!BIT(m_port_c, 0)) data &= m_row_c0->read();

	return data;
}

WRITE8_MEMBER( amigakbd_device::port_a_w )
{
	// look for pa0 edge
	if (!m_kdat && BIT(data, 0))
	{
		m_control |= PA0_POSITIVE_EDGE;
		update_irqs();
	}

	// and pa1 edge
	if (m_kclk && !BIT(data, 1))
	{
		m_control |= PA1_NEGATIVE_EDGE;
		update_irqs();
	}

	// update with new values and output
	if (m_kdat != BIT(data, 0))
	{
		m_kdat = BIT(data, 0);
		m_write_kdat(m_kdat);
	}

	if (m_kclk != BIT(data, 1))
	{
		m_kclk = BIT(data, 1);
		m_write_kclk(m_kclk);
	}
}

WRITE8_MEMBER( amigakbd_device::port_b_w )
{
	// caps lock led
	machine().output().set_value("led0", BIT(data, 7));
}

WRITE8_MEMBER( amigakbd_device::port_c_w )
{
	m_port_c = data;
}

WRITE8_MEMBER( amigakbd_device::port_d_w )
{
	// reset watchdog on 0 -> 1 transition
	if (!BIT(m_port_d, 7) && BIT(data, 7))
		m_watchdog->adjust(attotime::from_msec(54));

	m_port_d = data;
}

WRITE8_MEMBER( amigakbd_device::latch_w )
{
	if (offset == 0)
	{
		m_latch &= 0x00ff;
		m_latch |= data << 8;
	}
	else
	{
		m_latch &= 0xff00;
		m_latch |= data << 0;
	}
}

READ8_MEMBER( amigakbd_device::counter_r )
{
	if (!space.debugger_access())
	{
		m_control &= ~COUNTER_OVERFLOW;
		update_irqs();
	}

	if (offset == 0)
		return m_counter >> 8;
	else
		return m_counter >> 0;
}

WRITE8_MEMBER( amigakbd_device::transfer_latch_w )
{
	m_control &= ~COUNTER_OVERFLOW;
	update_irqs();

	m_latch &= 0x00ff;
	m_latch |= data << 8;

	m_counter = m_latch;
}

WRITE8_MEMBER( amigakbd_device::clear_pa0_detect )
{
	m_control &= ~PA0_POSITIVE_EDGE;
	update_irqs();
}

WRITE8_MEMBER( amigakbd_device::clear_pa1_detect )
{
	m_control &= ~PA1_NEGATIVE_EDGE;
	update_irqs();
}

READ8_MEMBER( amigakbd_device::control_r )
{
	return m_control;
}

WRITE8_MEMBER( amigakbd_device::control_w )
{
	m_control = data;
	update_irqs();
}

WRITE_LINE_MEMBER( amigakbd_device::kdat_w )
{
	// detect positive edge
	if (state && !m_kdat)
	{
		m_control |= PA0_POSITIVE_EDGE;
		update_irqs();
	}

	m_kdat = state;
}
