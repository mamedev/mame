// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/***************************************************************************

    K1801VP1-014 gate array (keyboard controller for the BK) and MS7008
    keyboard emulation.

    https://github.com/1801BM1/k1801/blob/master/014/
    http://www.bk001x.ru/For_Forum/Files/Pereferia/Bkshka_MC7008_shema_klava_ext.pdf
    https://retrocomputing.stackexchange.com/questions/27062/

    "Stop" button is handled by a dedicated circuit.

***************************************************************************/

#include "emu.h"
#include "1801vp014.h"

#include "machine/keyboard.ipp"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(K1801VP014, k1801vp014_device, "1801vp1-014", "1801VP1-014")


//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

INPUT_PORTS_START(ms7008)
	PORT_START("LINEC")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Ctrl") PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("АР2 / Alt") PORT_CODE(KEYCODE_RCONTROL)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("СТОП / Stop") PORT_CODE(KEYCODE_ESC) PORT_CHAR(UCHAR_MAMEKEY(ESC)) PORT_CHANGED_MEMBER(DEVICE_SELF, k1801vp014_device, stop_button, 0)

	PORT_START("LINE0") // X0
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ШАГ / Step") PORT_CODE(KEYCODE_F8) PORT_CHAR(UCHAR_MAMEKEY(F8)) // alt vec
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ПОВТ / Repeat") PORT_CODE(KEYCODE_F1) PORT_CHAR(UCHAR_MAMEKEY(F1)) // alt vec
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ИНД СУ / Display controls") PORT_CODE(KEYCODE_F6) PORT_CHAR(UCHAR_MAMEKEY(F6)) // alt vec
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("КТ / Escape") PORT_CODE(KEYCODE_F2) PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("БЛОК РЕД / Edit lock") PORT_CODE(KEYCODE_F7) PORT_CHAR(UCHAR_MAMEKEY(F7)) // alt vec
	PORT_BIT(0x60, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("LINE1") // X1
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB) PORT_CHAR(UCHAR_MAMEKEY(TAB)) // alt vec
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Delete to EOL") PORT_CODE(KEYCODE_F3) PORT_CHAR(UCHAR_MAMEKEY(F3)) // alt vec
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("СБР / Clear") PORT_CODE(KEYCODE_F9) PORT_CHAR(UCHAR_MAMEKEY(F9))
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("РУС / Rus") PORT_CODE(KEYCODE_LALT)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ЛАТ / Lat") PORT_CODE(KEYCODE_RALT)

	PORT_START("LINE2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Home") PORT_CODE(KEYCODE_HOME) PORT_CHAR(UCHAR_MAMEKEY(HOME))
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Delete") PORT_CODE(KEYCODE_F4) PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Insert") PORT_CODE(KEYCODE_F5) PORT_CHAR(UCHAR_MAMEKEY(F5))

	PORT_START("LINE3") // X3
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("LINE4") // not in the matrix
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0xFE, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("LINE5") // not in the matrix
	PORT_BIT(0xFF, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("LINE6") // X4
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('\"')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')

	PORT_START("LINE7") // X5
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_CHAR('<') PORT_CHAR(',')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('=') PORT_CHAR('-')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP) PORT_CHAR('>') PORT_CHAR('.')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('?') PORT_CHAR('/')

	PORT_START("LINE8") // X6
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_END) PORT_CHAR('@') PORT_CHAR('`')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_CHAR('A') PORT_CHAR('a')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B) PORT_CHAR('B') PORT_CHAR('b')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C) PORT_CHAR('C') PORT_CHAR('c')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_CHAR('D') PORT_CHAR('d')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_CHAR('E') PORT_CHAR('e')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F) PORT_CHAR('F') PORT_CHAR('f')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G) PORT_CHAR('G') PORT_CHAR('g')

	PORT_START("LINE9") // X7
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H) PORT_CHAR('H') PORT_CHAR('h')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I) PORT_CHAR('I') PORT_CHAR('i')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J) PORT_CHAR('J') PORT_CHAR('j')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K) PORT_CHAR('K') PORT_CHAR('k')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_CHAR('L') PORT_CHAR('l')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M) PORT_CHAR('M') PORT_CHAR('m')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N) PORT_CHAR('N') PORT_CHAR('n')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O) PORT_CHAR('O') PORT_CHAR('o')

	PORT_START("LINE10") // X8
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P) PORT_CHAR('P') PORT_CHAR('p')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q') PORT_CHAR('q')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_CHAR('R') PORT_CHAR('r')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_CHAR('S') PORT_CHAR('s')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T) PORT_CHAR('T') PORT_CHAR('t')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_CHAR('U') PORT_CHAR('u')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V) PORT_CHAR('V') PORT_CHAR('v')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_CHAR('W') PORT_CHAR('w')

	PORT_START("LINE11") // X9
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X) PORT_CHAR('X') PORT_CHAR('x')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y') PORT_CHAR('y')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z') PORT_CHAR('z')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE) PORT_CHAR('^') PORT_CHAR('~')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('_')
INPUT_PORTS_END

INPUT_CHANGED_MEMBER(k1801vp014_device::stop_button)
{
	m_write_halt(newval ? ASSERT_LINE : CLEAR_LINE);
}


//-------------------------------------------------
//  k1801vp014_device - constructor
//-------------------------------------------------

k1801vp014_device::k1801vp014_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, K1801VP014, tag, owner, clock)
	, device_matrix_keyboard_interface(mconfig, *this, "LINE0", "LINE1", "LINE2", "LINE3", "LINE4", "LINE5", "LINE6", "LINE7", "LINE8", "LINE9", "LINE10", "LINE11")
	, device_z80daisy_interface(mconfig, *this)
	, m_io_kbdc(*this, "LINEC")
	, m_write_virq(*this)
	, m_write_keydown(*this)
	, m_write_halt(*this)
	, m_write_data(*this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void k1801vp014_device::device_start()
{
	// save state
	save_item(NAME(m_key_code));
	save_item(NAME(m_kbd_state));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void k1801vp014_device::device_reset()
{
	m_key_code = 0;
	m_kbd_state = 0;
	m_key_irq_vector = 0;
	m_rxrdy = CLEAR_LINE;

	m_write_virq(m_rxrdy);

	reset_key_state();
	start_processing(attotime::from_hz(2'400)); // FIXME real device does not use polling
}

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor k1801vp014_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(ms7008);
}

int k1801vp014_device::z80daisy_irq_state()
{
	if (m_rxrdy == ASSERT_LINE)
		return Z80_DAISY_INT;
	else
		return 0;
}

int k1801vp014_device::z80daisy_irq_ack()
{
	int vec = -1;

	if (m_rxrdy == ASSERT_LINE)
	{
		m_rxrdy = CLEAR_LINE;
		vec = m_key_irq_vector;
	}

	return vec;
}


//-------------------------------------------------
//  read - register read
//-------------------------------------------------

uint16_t k1801vp014_device::read(offs_t offset)
{
	uint16_t data = 0;

	switch (offset & 3)
	{
	case 0:
		data = m_kbd_state;
		break;

	case 1:
		data = m_key_code;
		if (!machine().side_effects_disabled())
		{
			m_kbd_state &= ~CSR_DONE;
			clear_virq(m_write_virq, (m_kbd_state ^ CSR_IE), CSR_IE, m_rxrdy);
		}
	}

	return data;
}


//-------------------------------------------------
//  write - register write
//-------------------------------------------------

void k1801vp014_device::write(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	switch (offset & 3)
	{
	case 0:
		m_kbd_state = (m_kbd_state & ~CSR_IE) | (data & CSR_IE);
		break;

	case 1:
		// FIXME accessing this offset should cause bus error on BK0010
		m_write_data(data >> 8);
		break;
	}
}

void k1801vp014_device::key_make(uint8_t row, uint8_t column)
{
	uint16_t key_code = column + row * 8;
	int shift = BIT(m_io_kbdc->read(), 0);
	int ctrl  = BIT(m_io_kbdc->read(), 1);
	int alt   = BIT(m_io_kbdc->read(), 2);

	if (shift)
	{
		if (row == 6 || row == 7)
			key_code -= 16;
		else if (row > 7 && row <= 11)
			key_code += 32;
	}
	else if (ctrl)
	{
		if (row > 7 && row <= 11)
			key_code -= 64;
	}

	m_kbd_state |= CSR_DONE;
	m_key_code = key_code;

	if (alt)
		m_key_irq_vector = 0274;
	else
		m_key_irq_vector = 060;

	if (key_code == 0 || key_code == 1 || key_code == 2 || key_code == 4 || key_code == 011 || key_code == 013)
		m_key_irq_vector = 0274;

	raise_virq(m_write_virq, (m_kbd_state ^ CSR_IE), CSR_IE, m_rxrdy);
	m_write_keydown(ASSERT_LINE);
}

void k1801vp014_device::key_break(uint8_t row, uint8_t column)
{
	m_write_keydown(CLEAR_LINE);
}

