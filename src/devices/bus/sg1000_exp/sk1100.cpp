// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Sega SK-1100 keyboard emulation


Release data from the Sega Retro project:

  Year: 1984    Country/region: JP    Model code: SK-1100


TODO:
- SP-400 serial printer


**********************************************************************/

#include "emu.h"
#include "sk1100.h"
#include "softlist_dev.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(SEGA_SK1100, sega_sk1100_device, "sega_sk1100", "Sega SK-1100 Keyboard")


/*-------------------------------------------------
    INPUT_PORTS( sk1100_keys )
-------------------------------------------------*/

static INPUT_PORTS_START( sk1100_keys )
	PORT_START("PA0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q') PORT_CHAR('q')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('A') PORT_CHAR('a')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z') PORT_CHAR('z')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("ENG DIER'S") PORT_CODE(KEYCODE_RALT) PORT_CHAR(UCHAR_MAMEKEY(RALT))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('K') PORT_CHAR('k')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('I') PORT_CHAR('i')

	PORT_START("PA1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('W') PORT_CHAR('w')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('S') PORT_CHAR('s')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('X') PORT_CHAR('x')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SPC") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('L') PORT_CHAR('l')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('O') PORT_CHAR('o')

	PORT_START("PA2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('E') PORT_CHAR('e')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('D') PORT_CHAR('d')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('C') PORT_CHAR('c')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("HOME CLR") PORT_CODE(KEYCODE_HOME) PORT_CHAR(UCHAR_MAMEKEY(HOME))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('P') PORT_CHAR('p')

	PORT_START("PA3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('R') PORT_CHAR('r')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('F') PORT_CHAR('f')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('V') PORT_CHAR('v')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("INS DEL") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_SMALL_PI) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR(0x03c0)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('@') PORT_CHAR('`')

	PORT_START("PA4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('T') PORT_CHAR('t')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('G') PORT_CHAR('g')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('B') PORT_CHAR('b')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_DOWN) PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('[') PORT_CHAR('{')

	PORT_START("PA5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y') PORT_CHAR('y')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('H') PORT_CHAR('h')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('N') PORT_CHAR('n')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_LEFT) PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CR") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('U') PORT_CHAR('u')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('J') PORT_CHAR('j')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('M') PORT_CHAR('m')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_RIGHT) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_UP) PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA7")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED ) // keyboard disabled

	PORT_START("PB0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT( 0x0e, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PB1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT( 0x0e, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PB2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT( 0x0e, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PB3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT( 0x0e, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PB4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH2) PORT_CHAR('^')
	PORT_BIT( 0x0e, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PB5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("\xc2\xa5") PORT_CODE(KEYCODE_TILDE) PORT_CHAR(0x00a5)
	PORT_BIT( 0x06, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("FUNC") PORT_CODE(KEYCODE_TAB)

	PORT_START("PB6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("BREAK") PORT_CODE(KEYCODE_ESC) PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("GRAPH") PORT_CODE(KEYCODE_LALT) PORT_CHAR(UCHAR_MAMEKEY(LALT))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CTRL") PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)

	PORT_START("PB7")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED ) // keyboard disabled
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor sega_sk1100_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( sk1100_keys );
}


void sega_sk1100_device::device_add_mconfig(machine_config &config)
{
	/* devices */
	I8255(config, m_ppi);
	m_ppi->in_pa_callback().set(FUNC(sega_sk1100_device::ppi_pa_r));
	m_ppi->in_pb_callback().set(FUNC(sega_sk1100_device::ppi_pb_r));
	m_ppi->out_pc_callback().set(FUNC(sega_sk1100_device::ppi_pc_w));

	CASSETTE(config, m_cassette);
	m_cassette->set_formats(sc3000_cassette_formats);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	m_cassette->set_interface("sc3000_cass");

	SK1100_PRINTER_PORT(config, m_printer_port, sk1100_printer_port_devices, nullptr);

	/* software lists */
	SOFTWARE_LIST(config, "sc3k_cart_list").set_original("sc3000_cart");
	SOFTWARE_LIST(config, "cass_list").set_original("sc3000_cass");
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  sega_sk1100_device - constructor
//-------------------------------------------------

sega_sk1100_device::sega_sk1100_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, SEGA_SK1100, tag, owner, clock),
	device_sg1000_expansion_slot_interface(mconfig, *this),
	m_cassette(*this, "cassette"),
	m_ppi(*this, UPD9255_0_TAG),
	m_printer_port(*this, "printer"),
	m_pa(*this, "PA%u", 0U),
	m_pb(*this, "PB%u", 0U),
	m_keylatch(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sega_sk1100_device::device_start()
{
	/* register for state saving */
	save_item(NAME(m_keylatch));
}


//-------------------------------------------------
//  peripheral_r - keyboard read
//-------------------------------------------------

uint8_t sega_sk1100_device::peripheral_r(offs_t offset)
{
	return m_ppi->read(offset & 0x03);
}


//-------------------------------------------------
//  peripheral_w - keyboard write
//-------------------------------------------------

void sega_sk1100_device::peripheral_w(offs_t offset, uint8_t data)
{
	m_ppi->write(offset & 0x03, data);
}


bool sega_sk1100_device::is_readable(uint8_t offset)
{
	return (m_keylatch != 0x07 ? true : false);
}


/*-------------------------------------------------
    I8255 INTERFACE
-------------------------------------------------*/

uint8_t sega_sk1100_device::ppi_pa_r()
{
	/*
	    Signal  Description

	    PA0     Keyboard input
	    PA1     Keyboard input
	    PA2     Keyboard input
	    PA3     Keyboard input
	    PA4     Keyboard input
	    PA5     Keyboard input
	    PA6     Keyboard input
	    PA7     Keyboard input
	*/

	return m_pa[m_keylatch]->read();
}

uint8_t sega_sk1100_device::ppi_pb_r()
{
	/*
	    Signal  Description

	    PB0     Keyboard input
	    PB1     Keyboard input
	    PB2     Keyboard input
	    PB3     Keyboard input
	    PB4     /CONT input from cartridge terminal B-11
	    PB5     /FAULT input from printer
	    PB6     BUSY input from printer
	    PB7     Cassette tape input
	*/

	/* keyboard */
	uint8_t data = m_pb[m_keylatch]->read();

	/* cartridge contact */
	data |= 0x10;

	/* printer port */
	data |= m_printer_port->fault_r() << 5;
	data |= m_printer_port->busy_r() << 6;

	/* tape input */
	if (m_cassette->input() > +0.0) data |= 0x80;

	return data;
}

void sega_sk1100_device::ppi_pc_w(uint8_t data)
{
	/*
	    Signal  Description

	    PC0     Keyboard raster output
	    PC1     Keyboard raster output
	    PC2     Keyboard raster output
	    PC3     not connected
	    PC4     Cassette tape output
	    PC5     DATA to printer
	    PC6     /RESET to printer
	    PC7     /FEED to printer
	*/

	/* keyboard */
	m_keylatch = data & 0x07;

	/* cassette */
	m_cassette->output(BIT(data, 4) ? +1.0 : -1.0);

	/* printer port */
	m_printer_port->data_w(BIT(data, 5));
	m_printer_port->reset_w(BIT(data, 6));
	m_printer_port->feed_w(BIT(data, 7));
}
