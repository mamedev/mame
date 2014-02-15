#include "terminal.h"

static INPUT_PORTS_START(serial_terminal)
	PORT_INCLUDE(generic_terminal)
	PORT_START("TERM_TXBAUD")
	PORT_CONFNAME(0xff, 0x06, "TX Baud") PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, serial_terminal_device, update_serial)
	PORT_CONFSETTING( 0x0d, "110")
	PORT_CONFSETTING( 0x00, "150")
	PORT_CONFSETTING( 0x01, "300")
	PORT_CONFSETTING( 0x02, "600")
	PORT_CONFSETTING( 0x03, "1200")
	PORT_CONFSETTING( 0x04, "2400")
	PORT_CONFSETTING( 0x05, "4800")
	PORT_CONFSETTING( 0x06, "9600")
	PORT_CONFSETTING( 0x07, "14400")
	PORT_CONFSETTING( 0x08, "19200")
	PORT_CONFSETTING( 0x09, "28800")
	PORT_CONFSETTING( 0x0a, "38400")
	PORT_CONFSETTING( 0x0b, "57600")
	PORT_CONFSETTING( 0x0c, "115200")

	PORT_START("TERM_RXBAUD")
	PORT_CONFNAME(0xff, 0x06, "RX Baud") PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, serial_terminal_device, update_serial)
	PORT_CONFSETTING( 0x0d, "110")
	PORT_CONFSETTING( 0x00, "150")
	PORT_CONFSETTING( 0x01, "300")
	PORT_CONFSETTING( 0x02, "600")
	PORT_CONFSETTING( 0x03, "1200")
	PORT_CONFSETTING( 0x04, "2400")
	PORT_CONFSETTING( 0x05, "4800")
	PORT_CONFSETTING( 0x06, "9600")
	PORT_CONFSETTING( 0x07, "14400")
	PORT_CONFSETTING( 0x08, "19200")
	PORT_CONFSETTING( 0x09, "28800")
	PORT_CONFSETTING( 0x0a, "38400")
	PORT_CONFSETTING( 0x0b, "57600")
	PORT_CONFSETTING( 0x0c, "115200")

	PORT_START("TERM_STARTBITS")
	PORT_CONFNAME(0xff, 0x01, "Start Bits") PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, serial_terminal_device, update_serial)
	PORT_CONFSETTING( 0x00, "0")
	PORT_CONFSETTING( 0x01, "1")

	PORT_START("TERM_DATABITS")
	PORT_CONFNAME(0xff, 0x03, "Data Bits") PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, serial_terminal_device, update_serial)
	PORT_CONFSETTING( 0x00, "5")
	PORT_CONFSETTING( 0x01, "6")
	PORT_CONFSETTING( 0x02, "7")
	PORT_CONFSETTING( 0x03, "8")
	PORT_CONFSETTING( 0x04, "9")

	PORT_START("TERM_PARITY")
	PORT_CONFNAME(0xff, 0x00, "Parity") PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, serial_terminal_device, update_serial)
	PORT_CONFSETTING( 0x00, "None")
	PORT_CONFSETTING( 0x01, "Odd")
	PORT_CONFSETTING( 0x02, "Even")
	PORT_CONFSETTING( 0x03, "Mark")
	PORT_CONFSETTING( 0x04, "Space")

	PORT_START("TERM_STOPBITS")
	PORT_CONFNAME(0xff, 0x01, "Stop Bits") PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, serial_terminal_device, update_serial)
	PORT_CONFSETTING( 0x00, "0")
	PORT_CONFSETTING( 0x01, "1")
	PORT_CONFSETTING( 0x02, "1.5")
	PORT_CONFSETTING( 0x03, "2")
INPUT_PORTS_END

ioport_constructor serial_terminal_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(serial_terminal);
}

serial_terminal_device::serial_terminal_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: generic_terminal_device(mconfig, SERIAL_TERMINAL, "Serial Terminal", tag, owner, clock, "serial_terminal", __FILE__),
	device_serial_interface(mconfig, *this),
	device_rs232_port_interface(mconfig, *this),
	m_io_term_txbaud(*this, "TERM_TXBAUD"),
	m_io_term_rxbaud(*this, "TERM_RXBAUD"),
	m_io_term_startbits(*this, "TERM_STARTBITS"),
	m_io_term_databits(*this, "TERM_DATABITS"),
	m_io_term_parity(*this, "TERM_PARITY"),
	m_io_term_stopbits(*this, "TERM_STOPBITS")
{
}

void serial_terminal_device::device_start()
{
}

WRITE_LINE_MEMBER(serial_terminal_device::update_serial)
{
	static const int m_baud[] = {150, 300, 600, 1200, 2400, 4800, 9600, 14400, 19200, 28800, 38400, 57600, 115200, 110};
	static const int m_startbits[] = {0, 1};
	static const int m_databits[] = {5, 6, 7, 8, 9};
	static const parity_t m_parity[] = {PARITY_NONE, PARITY_ODD, PARITY_EVEN, PARITY_MARK, PARITY_SPACE};
	static const stop_bits_t m_stopbits[] = {STOP_BITS_0, STOP_BITS_1, STOP_BITS_1_5, STOP_BITS_2};

	UINT8 startbits = m_io_term_startbits->read();
	UINT8 databits = m_io_term_databits->read();
	UINT8 parity = m_io_term_parity->read();
	UINT8 stopbits = m_io_term_stopbits->read();

	set_data_frame(m_startbits[startbits], m_databits[databits], m_parity[parity], m_stopbits[stopbits]);

	UINT8 txbaud = m_io_term_txbaud->read();
	set_tra_rate(m_baud[txbaud]);

	UINT8 rxbaud = m_io_term_rxbaud->read();
	set_rcv_rate(m_baud[rxbaud]);
}

void serial_terminal_device::device_reset()
{
	generic_terminal_device::device_reset();

	output_rxd(1);

	update_serial(0);
}

void serial_terminal_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	device_serial_interface::device_timer(timer, id, param, ptr);
}

void serial_terminal_device::send_key(UINT8 code)
{
	if(is_transmit_register_empty())
	{
		transmit_register_setup(code);
		return;
	}
	m_key_valid = true;
	m_curr_key = code;
}

void serial_terminal_device::tra_callback()
{
	output_rxd(transmit_register_get_data_bit());
}

void serial_terminal_device::tra_complete()
{
	if(m_key_valid)
	{
		transmit_register_setup(m_curr_key);
		m_key_valid = false;
	}
}

void serial_terminal_device::rcv_complete()
{
	receive_register_extract();
	term_write(get_received_char());
}

const device_type SERIAL_TERMINAL = &device_creator<serial_terminal_device>;
