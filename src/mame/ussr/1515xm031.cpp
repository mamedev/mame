// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/**********************************************************************

    1515XM1-031 Gate Array emulation

    Keyboard controller and programmable timer for UKNC

**********************************************************************/

#include "emu.h"
#include "1515xm031.h"
#include "machine/keyboard.ipp"

//#define VERBOSE (LOG_GENERAL)
#include "logmacro.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(K1515XM031_PRI, k1515xm031_pri_device, "1515xm1_031_pri", "1515XM1-031 keyboard")
DEFINE_DEVICE_TYPE(K1515XM031_SUB, k1515xm031_sub_device, "1515xm1_031_sub", "1515XM1-031 timer")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

INPUT_PORTS_START(ms7007)
       PORT_START("Y4")
       PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("<STOP>") PORT_CODE(KEYCODE_F10) PORT_CHAR(UCHAR_MAMEKEY(F10))
       PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
       PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
       PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
       PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
       PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
       PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
       PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

       PORT_START("Y5")
       PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad ,") PORT_CODE(KEYCODE_ASTERISK) PORT_CHAR(UCHAR_MAMEKEY(ASTERISK))
       PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad -") PORT_CODE(KEYCODE_MINUS_PAD) PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD))
       PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
       PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
       PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Left Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
       PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad 7") PORT_CODE(KEYCODE_7_PAD) PORT_CHAR(UCHAR_MAMEKEY(7_PAD))
       PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad 8") PORT_CODE(KEYCODE_8_PAD) PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
       PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad 9") PORT_CODE(KEYCODE_9_PAD) PORT_CHAR(UCHAR_MAMEKEY(9_PAD))

       PORT_START("Y6")
       PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("AR2 (Esc)") PORT_CODE(KEYCODE_ESC) PORT_CHAR(UCHAR_MAMEKEY(ESC))
       PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Tab") PORT_CODE(KEYCODE_TAB) PORT_CHAR('\t')
       PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Ctrl") PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
       PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Graf") PORT_CODE(KEYCODE_LALT) PORT_CHAR(UCHAR_MAMEKEY(LALT))
       PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Alf") PORT_CODE(KEYCODE_RALT) PORT_CHAR(UCHAR_MAMEKEY(RALT))
       PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad 0") PORT_CODE(KEYCODE_0_PAD) PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
       PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad .") PORT_CODE(KEYCODE_DEL_PAD) PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))
       PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad Enter") PORT_CODE(KEYCODE_ENTER_PAD) PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD))

       PORT_START("Y7")
       PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR('+')
       PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J') PORT_CHAR(0x0a) PORT_CHAR(0x0a)
       PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F') PORT_CHAR(0x06) PORT_CHAR(0x06)
       PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q') PORT_CHAR(0x11) PORT_CHAR(0x11)
       PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Lock") PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
       PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad 1") PORT_CODE(KEYCODE_1_PAD) PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
       PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad 2") PORT_CODE(KEYCODE_2_PAD) PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
       PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad 3") PORT_CODE(KEYCODE_3_PAD) PORT_CHAR(UCHAR_MAMEKEY(3_PAD))

       PORT_START("Y8")
       PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("K1") PORT_CODE(KEYCODE_F1) PORT_CHAR(UCHAR_MAMEKEY(F1))
       PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
       PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C') PORT_CHAR(0x03) PORT_CHAR(0x03)
       PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y') PORT_CHAR(0x19) PORT_CHAR(0x19)
       PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Ch")
       PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad 4") PORT_CODE(KEYCODE_4_PAD) PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
       PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad 5") PORT_CODE(KEYCODE_5_PAD) PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
       PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad 6") PORT_CODE(KEYCODE_6_PAD) PORT_CHAR(UCHAR_MAMEKEY(6_PAD))

       PORT_START("Y9")
       PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("K2") PORT_CODE(KEYCODE_F2) PORT_CHAR(UCHAR_MAMEKEY(F2))
       PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('@')
       PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U') PORT_CHAR(0x15) PORT_CHAR(0x15)
       PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W') PORT_CHAR(0x17) PORT_CHAR(0x17)
       PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S') PORT_CHAR(0x13) PORT_CHAR(0x13)
       PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad +") PORT_CODE(KEYCODE_PLUS_PAD) PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD))
       PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("<ISP> (Go)") PORT_CODE(KEYCODE_F8) PORT_CHAR(UCHAR_MAMEKEY(F6))
       PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("<SBROS> (Reset)") PORT_CODE(KEYCODE_F9) PORT_CHAR(UCHAR_MAMEKEY(F7))

       PORT_START("Y10")
       PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("K3") PORT_CODE(KEYCODE_F3) PORT_CHAR(UCHAR_MAMEKEY(F3))
       PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
       PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K') PORT_CHAR(0x0b) PORT_CHAR(0x0b)
       PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A') PORT_CHAR(0x01) PORT_CHAR(0x01)
       PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M') PORT_CHAR(0x0d) PORT_CHAR(0x0d)
       PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("UTF8_LEFT") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
       PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("<UST> (Setup)") PORT_CODE(KEYCODE_F7) PORT_CHAR(UCHAR_MAMEKEY(F8))
       PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("<POM> (Help)") PORT_CODE(KEYCODE_F6) PORT_CHAR(UCHAR_MAMEKEY(F9))

       PORT_START("Y11")
       PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
       PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E') PORT_CHAR(0x05) PORT_CHAR(0x05)
       PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P') PORT_CHAR(0x10) PORT_CHAR(0x10)
       PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I') PORT_CHAR(0x09) PORT_CHAR(0x09)
       PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
       PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Cursor right") PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
       PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Enter") PORT_CODE(KEYCODE_ENTER) PORT_CHAR('\r')
       PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')

       PORT_START("Y12")
       PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("K4") PORT_CODE(KEYCODE_F4) PORT_CHAR(UCHAR_MAMEKEY(F4))
       PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
       PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N') PORT_CHAR(0x0e) PORT_CHAR(0x0e)
       PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R') PORT_CHAR(0x12) PORT_CHAR(0x12)
       PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T') PORT_CHAR(0x14) PORT_CHAR(0x14)
       PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Cursor down") PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
       PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Cursor up") PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
       PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(':') PORT_CHAR('*')

       PORT_START("Y13")
       PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("K5") PORT_CODE(KEYCODE_F5) PORT_CHAR(UCHAR_MAMEKEY(F5))
       PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('^')
       PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G') PORT_CHAR(0x07) PORT_CHAR(0x07)
       PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O') PORT_CHAR(0x0f) PORT_CHAR(0x0f)
       PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X') PORT_CHAR(0x18) PORT_CHAR(0x18)
       PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
       PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(": hardsign") PORT_CODE(KEYCODE_TILDE) PORT_CHAR('`') PORT_CHAR('~')
       PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('_')

       PORT_START("Y14")
       PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('&')
       PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[') PORT_CHAR('{')
       PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L') PORT_CHAR(0x0c) PORT_CHAR(0x0c)
       PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B') PORT_CHAR(0x02) PORT_CHAR(0x02)
       PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Cursor left") PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
       PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\\') PORT_CHAR('|')
       PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H') PORT_CHAR(0x08) PORT_CHAR(0x08)
       PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR(')')

       PORT_START("Y15")
       PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('*')
       PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}')
       PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D') PORT_CHAR(0x04) PORT_CHAR(0x04)
       PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(": @") PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=') PORT_CHAR('+')
       PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
       PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V') PORT_CHAR(0x16) PORT_CHAR(0x16)
       PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z') PORT_CHAR(0x1a) PORT_CHAR(0x1a)
       PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR('(')
INPUT_PORTS_END

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor k1515xm031_pri_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(ms7007);
}

//-------------------------------------------------
//  k1515xm031_pri_device - constructor
//-------------------------------------------------

k1515xm031_pri_device::k1515xm031_pri_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, K1515XM031_PRI, tag, owner, clock)
	, device_z80daisy_interface(mconfig, *this)
	, device_matrix_keyboard_interface(mconfig, *this, "Y4", "Y5", "Y6", "Y7", "Y8", "Y9", "Y10", "Y11", "Y12", "Y13", "Y14", "Y15")
	, m_write_virq(*this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void k1515xm031_pri_device::device_start()
{
	// register for state saving
	save_item(NAME(m_csr));
	save_item(NAME(m_buf));

	m_buf = 0;
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void k1515xm031_pri_device::device_reset()
{
	m_csr = 0;

	reset_key_state();
	start_processing(attotime::from_hz(16'000));
}

//-------------------------------------------------
//  daisy chained interrupts
//-------------------------------------------------

int k1515xm031_pri_device::z80daisy_irq_state()
{
	if (m_rxrdy == ASSERT_LINE)
		return Z80_DAISY_INT;
	else
		return 0;
}

int k1515xm031_pri_device::z80daisy_irq_ack()
{
	int vec = -1;

	if (m_rxrdy == ASSERT_LINE)
	{
		m_rxrdy = CLEAR_LINE;
		vec = 0300;
	}

	return vec;
}


//-------------------------------------------------
//  read - register read
//-------------------------------------------------

uint16_t k1515xm031_pri_device::read(offs_t offset)
{
	uint16_t data = 0;

	switch (offset)
	{
	case 0:
		data = m_csr & KBDCSR_RD;
		break;

	case 1:
		data = m_buf;
		if (!machine().side_effects_disabled())
		{
			m_csr &= ~CSR_DONE;
			clear_virq(m_write_virq, m_csr, CSR_IE, m_rxrdy);
			start_processing(attotime::from_hz(16'000));
		}
		break;

	case 2: // KBD_BUS
		data = 010000; // indicate that 80-track floppy drive is installed
		break;
	}

	if (offset)
	LOG("%s R %06o == %06o\n", machine().describe_context(), 0177700 + (offset << 1), data);

	return data;
}


//-------------------------------------------------
//  write - register write
//-------------------------------------------------

void k1515xm031_pri_device::write(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	switch (offset & 3)
	{
	case 0:
		if ((data & CSR_IE) == 0)
			clear_virq(m_write_virq, 1, 1, m_rxrdy);
		else if ((m_csr & (CSR_DONE + CSR_IE)) == CSR_DONE)
			raise_virq(m_write_virq, 1, 1, m_rxrdy);
		m_csr = ((m_csr & ~KBDCSR_WR) | (data & KBDCSR_WR));
		break;
	}
}

void k1515xm031_pri_device::key_make(uint8_t row, uint8_t column)
{
	m_buf = column * 16 + row + 4;
	m_csr |= CSR_DONE;
	raise_virq(m_write_virq, m_csr, CSR_IE, m_rxrdy);
	stop_processing();
}

void k1515xm031_pri_device::key_break(uint8_t row, uint8_t column)
{
	m_buf = 128 + row + 4;
	m_csr |= CSR_DONE;
	raise_virq(m_write_virq, m_csr, CSR_IE, m_rxrdy);
	stop_processing();
}


//-------------------------------------------------
//  k1515xm031_sub_device - constructor
//-------------------------------------------------

k1515xm031_sub_device::k1515xm031_sub_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, K1515XM031_SUB, tag, owner, clock)
	, device_z80daisy_interface(mconfig, *this)
	, m_write_virq(*this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void k1515xm031_sub_device::device_start()
{
	// register for state saving
	save_item(NAME(m_csr));
	save_item(NAME(m_buf));
	save_item(NAME(m_value));
	save_item(NAME(m_counter));

	m_timer = timer_alloc(FUNC(k1515xm031_sub_device::timer_tick), this);
	m_timer->adjust(attotime::never, 0, attotime::never);

	m_buf = m_csr = m_value = m_counter = 0;
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void k1515xm031_sub_device::device_reset()
{
	m_csr &= ~(TMRCSR_MODE | TMRCSR_EVNT);
}

//-------------------------------------------------
//  daisy chained interrupts
//-------------------------------------------------

int k1515xm031_sub_device::z80daisy_irq_state()
{
	if (m_rxrdy == ASSERT_LINE)
		return Z80_DAISY_INT;
	else
		return 0;
}

int k1515xm031_sub_device::z80daisy_irq_ack()
{
	int vec = -1;

	if (m_rxrdy == ASSERT_LINE)
	{
		m_rxrdy = CLEAR_LINE;
		vec = 0304;
	}

	return vec;
}


//-------------------------------------------------
//  read - register read
//-------------------------------------------------

uint16_t k1515xm031_sub_device::read(offs_t offset)
{
	uint16_t data = 0;

	switch (offset)
	{
	case 0:
		data = m_csr & TMRCSR_RD;
		if (!machine().side_effects_disabled())
			m_csr &= ~TMRCSR_OVF;
		break;

	case 2:
		data = m_value & TMRBUF_RDWR;
		if (!machine().side_effects_disabled())
		{
			m_csr &= ~CSR_DONE;
			clear_virq(m_write_virq, m_csr, CSR_IE, m_rxrdy);
		}
		break;
	}

	return data;
}

//-------------------------------------------------
//  write - register write
//-------------------------------------------------

void k1515xm031_sub_device::write(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOG("%s W %06o <- %06o & %06o\n", machine().describe_context(), 0177710 + (offset << 1), data, mem_mask);

	switch (offset)
	{
	case 0:
		if (BIT(data, 0))
		{
			int period = (data >> 1) & 3;
			m_timer->adjust(attotime::from_usec(2 << period), 0, attotime::from_usec(2 << period)); // FIXME 1.92, not 2 usec
			LOG("timer set %d usec\n", 2 << period);
		}
		else
		{
			m_timer->adjust(attotime::never, 0, attotime::never);
			LOG("timer stopped\n");
		}

		if ((data & CSR_IE) == 0)
			clear_virq(m_write_virq, 1, 1, m_rxrdy);
		else if ((m_csr & (CSR_DONE + CSR_IE)) == CSR_DONE)
			raise_virq(m_write_virq, 1, 1, m_rxrdy);

		m_csr = ((m_csr & ~TMRCSR_WR) | (data & TMRCSR_WR));
		break;

	case 1:
		m_buf = data & TMRBUF_RDWR;
		if (!BIT(m_csr, 0))
			m_value = m_counter = m_buf;
		break;
	}
}

TIMER_CALLBACK_MEMBER(k1515xm031_sub_device::timer_tick)
{
	if (m_counter)
	{
		if (--m_counter == 0)
		{
			if (m_csr & CSR_DONE)
			{
				m_csr |= TMRCSR_OVF;
			}
			m_csr |= CSR_DONE;
			LOG("timer done ovf %d irq %d\n", m_csr & TMRCSR_OVF, m_csr & CSR_IE);
			raise_virq(m_write_virq, m_csr, CSR_IE, m_rxrdy);
			m_value = m_counter = m_buf;
		}
		else
		{
			m_value = m_counter;
		}
	}
}
